/**
 *   Copyright 2021 Alibaba, Inc. and its affiliates. All Rights Reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.

 *   \author   Haichao.chc
 *   \date     Oct 2020
 *   \brief    Implementation of collection class
 */

#include "collection.h"
#include <chrono>
#include <ailego/container/heap.h>
#include <ailego/utility/time_helper.h>
#include "common/defer.h"
#include "common/error_code.h"
#include "common/logger.h"
#include "constants.h"
#include "file_helper.h"
#include "typedef.h"

namespace proxima {
namespace be {
namespace index {

CollectionPtr Collection::Create(const std::string &collection_name,
                                 const std::string &prefix_path,
                                 meta::CollectionMetaPtr schema,
                                 uint32_t concurrency,
                                 ThreadPool *thread_pool) {
  return std::make_shared<Collection>(collection_name, prefix_path, schema,
                                      concurrency, thread_pool);
}

int Collection::CreateAndOpen(const std::string &collection_name,
                              const std::string &prefix_path,
                              meta::CollectionMetaPtr schema,
                              uint32_t concurrency, ThreadPool *thread_pool,
                              const ReadOptions &read_options,
                              CollectionPtr *collection) {
  *collection = std::make_shared<Collection>(collection_name, prefix_path,
                                             schema, concurrency, thread_pool);

  return (*collection)->open(read_options);
}

Collection::Collection(const std::string &coll_name, const std::string &path,
                       meta::CollectionMetaPtr coll_meta, uint32_t concur,
                       ThreadPool *pool)
    : collection_name_(coll_name),
      prefix_path_(path),
      schema_(std::move(coll_meta)),
      concurrency_(concur),
      thread_pool_(pool) {}

Collection::~Collection() {
  if (opened_) {
    close();
  }
}

int Collection::open(const ReadOptions &read_options) {
  CHECK_STATUS(opened_, false);

  dir_path_ = prefix_path_ + "/" + collection_name_;
  std::string manifest_file_path =
      FileHelper::MakeFilePath(dir_path_, FileID::MANIFEST_FILE);

  // check index data
  if (read_options.create_new) {
    if (FileHelper::DirectoryExists(dir_path_)) {
      CLOG_ERROR("Index directory already exist, create failed. dir_path[%s]",
                 dir_path_.c_str());
      return ErrorCode_DuplicateCollection;
    }
  } else {
    if (!FileHelper::DirectoryExists(dir_path_) ||
        !FileHelper::FileExists(manifest_file_path)) {
      CLOG_ERROR(
          "Index directory or manifest not exist, open failed. dir_path[%s]",
          dir_path_.c_str());
      return ErrorCode_InvalidIndexDataFormat;
    }
  }

  int ret = recover_from_snapshot(read_options);
  if (ret != 0) {
    CLOG_ERROR("Recover from snapshot failed.");

    // if create new collection failed
    // need to cleanup history files
    if (read_options.create_new) {
      this->remove_files();
    }
    return ret;
  }

  opened_ = true;

  CollectionStats stats;
  this->get_stats(&stats);
  CLOG_INFO(
      "Open colletion success. doc_count[%zu] segment_count[%zu] "
      "max_docs_per_segment[%zu]",
      (size_t)stats.total_doc_count, (size_t)stats.total_segment_count,
      (size_t)schema_->max_docs_per_segment());

  return 0;
}

int Collection::close() {
  CHECK_STATUS(opened_, true);

  // Wait until dump ended
  while (is_dumping_) {
    LOG_INFO("Collection is dumping segment, wait until dumped...");
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  // Wait until flush ended
  while (is_flushing_) {
    LOG_INFO("Collection is flushing, wait until flushed...");
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  // Wait until optimize ended
  while (is_optimizing_) {
    LOG_INFO("Collection is optimizing, wait until optimized...");
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  // Close writing segment
  writing_segment_->close();

  // maybe dumping segment process exist error
  // so we still close dumping segment safely
  if (dumping_segment_ != nullptr) {
    dumping_segment_->close();
  }

  persist_segment_mgr_->unload_segments();

  id_map_->close();
  delete_store_->close();
  lsn_store_->close();
  version_manager_->close();

  opened_ = false;
  CLOG_INFO("Close collection success.");

  return 0;
}

int Collection::close_and_cleanup() {
  CHECK_STATUS(opened_, true);

  this->close();
  this->remove_files();

  return 0;
}

int Collection::flush() {
  CHECK_STATUS(opened_, true);

  CLOG_INFO("Start flushing collection.");
  ailego::ElapsedTime timer;
  is_flushing_ = true;

  Defer defer([this] { is_flushing_ = false; });

  int ret = 0;
  ret = writing_segment_->flush();
  CHECK_RETURN_WITH_CLOG(ret, 0, "Flush writing segment failed.");

  ret = id_map_->flush();
  CHECK_RETURN_WITH_CLOG(ret, 0, "Flush id map failed.");

  ret = delete_store_->flush();
  CHECK_RETURN_WITH_CLOG(ret, 0, "Flush delete store failed.");

  ret = lsn_store_->flush();
  CHECK_RETURN_WITH_CLOG(ret, 0, "Flush lsn store failed.");

  version_manager_->update_segment_meta(writing_segment_->segment_meta());
  ret = version_manager_->flush();
  CHECK_RETURN_WITH_CLOG(ret, 0, "Flush version manager failed.");

  CLOG_INFO("Ended flushing collection. cost[%zums]",
            (size_t)timer.milli_seconds());
  return 0;
}

int Collection::dump() {
  CHECK_STATUS(opened_, true);

  return this->drive_dump_segment();
}

int Collection::optimize(ThreadPoolPtr pool) {
  CHECK_STATUS(opened_, true);

  CLOG_INFO("Start optimizing collection.");
  ailego::ElapsedTime timer;
  is_optimizing_ = true;

  Defer defer([this] { is_optimizing_ = false; });

  int ret = writing_segment_->optimize(pool);
  CHECK_RETURN_WITH_CLOG(ret, 0, "Optimize writing segment failed.");

  CLOG_INFO("Ended optimizing collection. cost[%zums]",
            (size_t)timer.milli_seconds());
  return 0;
}

int Collection::remove_files() {
  return FileHelper::RemoveDirectory(dir_path_);
}

int Collection::write_records(const CollectionDataset &records) {
  CHECK_STATUS(opened_, true);

  ailego::ElapsedTime timer;
  int ret = 0;
  int error_code = 0;
  for (size_t i = 0; i < records.size(); i++) {
    auto &record = records.get(i);
    uint64_t primary_key = record.primary_key;
    uint64_t lsn = record.lsn;

    switch (record.operation_type) {
      case OperationTypes::INSERT:
        ret = insert_record(record);
        if (ret != 0) {
          error_code = ret;
          CLOG_ERROR("Insert record failed. key[%zu] lsn[%zu] rt[%zuus]",
                     (size_t)primary_key, (size_t)lsn,
                     (size_t)timer.micro_seconds());
        } else {
          CLOG_INFO("Insert record success. key[%zu] lsn[%zu] rt[%zuus]",
                    (size_t)primary_key, (size_t)lsn,
                    (size_t)timer.micro_seconds());
        }
        break;
      case OperationTypes::UPDATE:
        ret = update_record(record);
        if (ret != 0) {
          error_code = ret;
          CLOG_ERROR("Update record failed. key[%zu] lsn[%zu] rt[%zuus]",
                     (size_t)primary_key, (size_t)lsn,
                     (size_t)timer.micro_seconds());
        } else {
          CLOG_INFO("Update record success. key[%zu] lsn[%zu] rt[%zuus]",
                    (size_t)primary_key, (size_t)lsn,
                    (size_t)timer.micro_seconds());
        }
        break;
      case OperationTypes::DELETE:
        ret = delete_record(primary_key);
        if (ret != 0) {
          error_code = ret;
          CLOG_ERROR("Delete record failed. key[%zu] lsn[%zu] rt[%zuus]",
                     (size_t)primary_key, (size_t)lsn,
                     (size_t)timer.micro_seconds());
        } else {
          CLOG_INFO("Delete record success. key[%zu] lsn[%zu] rt[%zuus]",
                    (size_t)primary_key, (size_t)lsn,
                    (size_t)timer.micro_seconds());
        }
        break;
      default:
        CLOG_ERROR("Unknown operation type. type[%d]", record.operation_type);
    }
  }

  return error_code;
}

int Collection::insert_record(const Record &record) {
  // 1. check if record already exists
  if (this->has_record(record.primary_key)) {
    CLOG_ERROR("Insert duplicate record. key[%zu]", (size_t)record.primary_key);
    return ErrorCode_DuplicateKey;
  }

  // 2. insert into memory segment
  idx_t doc_id = INVALID_DOC_ID;
  int ret = writing_segment_->insert(record, &doc_id);
  CHECK_RETURN_WITH_CLOG(ret, 0, "Insert into memory segment failed. key[%zu]",
                         (size_t)record.primary_key);

  // 3. record key/doc_id mapping in id map
  ret = id_map_->insert(record.primary_key, doc_id);
  CHECK_RETURN_WITH_CLOG(ret, 0, "Insert into id map failed. key[%zu]",
                         (size_t)record.primary_key);

  // 4. record in lsn store
  ret = lsn_store_->append(record.lsn, record.lsn_context);
  if (ret != 0) {
    // do not need to terminate insert process
    CLOG_WARN("Lsn store append failed. key[%zu]", (size_t)record.primary_key);
  }

  // try to drive dump writing segment
  uint64_t max_docs_per_segment = schema_->max_docs_per_segment();
  if (max_docs_per_segment > 0 &&
      writing_segment_->doc_count() >= max_docs_per_segment) {
    drive_dump_segment();
  }

  return 0;
}

int Collection::delete_record(uint64_t primary_key) {
  // 1. check if record exist
  if (!this->has_record(primary_key)) {
    CLOG_ERROR("Record not exist in colletion. key[%zu]", (size_t)primary_key);
    return ErrorCode_InexistentKey;
  }

  // 2. get key/doc_id mapping
  idx_t doc_id = id_map_->get_mapping_id(primary_key);
  if (doc_id == INVALID_DOC_ID) {
    CLOG_ERROR("Get mapping doc-id failed. key[%zu]", (size_t)primary_key);
    return ErrorCode_RuntimeError;
  }

  // 3. insert into delete map
  int ret = delete_store_->insert(doc_id);
  CHECK_RETURN_WITH_CLOG(ret, 0, "Insert into delete map failed.");

  // 4. remove mapping in id_map
  id_map_->remove(primary_key);

  // 5. try to inplace remove in writing segment
  if (writing_segment_->is_in_range(doc_id)) {
    ret = writing_segment_->remove(doc_id);
    CHECK_RETURN_WITH_CLOG(ret, 0, "Remove from writing segment failed.");
  }
  return 0;
}

int Collection::update_record(const Record &record) {
  // 1. check if record exist
  if (!this->has_record(record.primary_key)) {
    CLOG_ERROR("Record not exist in collection. key[%zu]",
               (size_t)record.primary_key);
    return ErrorCode_InexistentKey;
  }

  // 2. check record lsn
  int ret = 0;
  if (record.lsn_check) {
    Record old_record;
    ret = this->search_record(record.primary_key, &old_record);
    if (ret != 0 || old_record.primary_key == INVALID_KEY) {
      CLOG_ERROR("Search record failed. key[%zu]", (size_t)record.primary_key);
      return ret;
    }

    if (record.lsn <= old_record.lsn) {
      CLOG_ERROR("Invalid record lsn. key[%zu] lsn[%zu] last_lsn[%zu]",
                 (size_t)record.primary_key, (size_t)record.lsn,
                 (size_t)old_record.lsn);
      return ErrorCode_InvalidRecord;
    }
  }

  // 3. delete old record
  ret = this->delete_record(record.primary_key);
  CHECK_RETURN(ret, 0);

  // 4. insert new record
  ret = this->insert_record(record);

  return ret;
}

bool Collection::has_record(uint64_t primary_key) {
  return id_map_->has(primary_key);
}

int Collection::search_record(uint64_t primary_key, Record *record) {
  if (!this->has_record(primary_key)) {
    return 0;
  }

  idx_t doc_id = id_map_->get_mapping_id(primary_key);
  SegmentPtr found_segment = SegmentPtr();

  auto &segment_metas = version_manager_->current_version();
  // reverse search for newer segment match
  for (auto it = segment_metas.crbegin(); it != segment_metas.crend(); ++it) {
    if (doc_id >= it->min_doc_id && doc_id <= it->max_doc_id) {
      SegmentID segment_id = it->segment_id;
      found_segment = persist_segment_mgr_->get_segment(segment_id);
      break;
    }
  }

  if (!found_segment && dumping_segment_ != nullptr) {
    if (doc_id >= dumping_segment_->segment_meta().min_doc_id &&
        doc_id <= dumping_segment_->segment_meta().max_doc_id) {
      found_segment = dumping_segment_;
    }
  }

  if (!found_segment && writing_segment_ != nullptr) {
    found_segment = writing_segment_;
  }

  QueryResult result;
  int ret = found_segment->kv_search(primary_key, &result);
  if (ret == 0 && result.primary_key != INVALID_KEY) {
    record->primary_key = result.primary_key;
    record->revision = result.revision;
    record->forward_data = std::move(result.forward_data);
    record->lsn = result.lsn;
  }

  return 0;
}

int Collection::get_latest_lsn(uint64_t *lsn, std::string *lsn_context) {
  CHECK_STATUS(opened_, true);
  return lsn_store_->get_latest_lsn(lsn, lsn_context);
}

int Collection::get_segments(std::vector<SegmentPtr> *segments) {
  CHECK_STATUS(opened_, true);

  auto &segment_metas = version_manager_->current_version();
  for (size_t i = 0; i < segment_metas.size(); i++) {
    SegmentID segment_id = segment_metas[i].segment_id;
    if (persist_segment_mgr_->has_segment(segment_id)) {
      segments->emplace_back(persist_segment_mgr_->get_segment(segment_id));
    } else {
      // Maybe it's pre-loaded fail, and it will be loaded again.
      PersistSegmentPtr persist_segment;
      ReadOptions read_options;
      read_options.use_mmap = true;
      read_options.create_new = false;
      int ret = this->load_persist_segment(segment_metas[i], read_options,
                                           &persist_segment);
      CHECK_RETURN(ret, 0);
      persist_segment_mgr_->add_segment(persist_segment);
      segments->emplace_back(persist_segment);
    }
  }

  if (writing_segment_ != nullptr) {
    segments->emplace_back(writing_segment_);
  }

  if (dumping_segment_ != nullptr &&
      !persist_segment_mgr_->has_segment(dumping_segment_->segment_id())) {
    segments->emplace_back(dumping_segment_);
  }

  return 0;
}

int Collection::get_stats(CollectionStats *stats) {
  stats->collection_name = collection_name_;
  stats->collection_path = dir_path_;
  stats->delete_doc_count = delete_store_->count();

  // collect stats of persist segment
  auto &segment_metas = version_manager_->current_version();
  for (size_t i = 0; i < segment_metas.size(); i++) {
    stats->total_doc_count += segment_metas[i].doc_count;
    stats->total_index_file_count += segment_metas[i].index_file_count;
    stats->total_index_file_size += segment_metas[i].index_file_size;
    stats->total_segment_count++;
    stats->segment_stats.emplace_back(segment_metas[i]);
  }

  // collect stats of memory segment
  if (dumping_segment_ != nullptr &&
      !persist_segment_mgr_->has_segment(dumping_segment_->segment_id())) {
    auto &segment_meta = dumping_segment_->segment_meta();
    stats->total_doc_count += segment_meta.doc_count;
    stats->total_index_file_count += segment_meta.index_file_count;
    stats->total_index_file_size += segment_meta.index_file_size;
    stats->total_segment_count++;
    stats->segment_stats.emplace_back(segment_meta);
  }

  if (writing_segment_ != nullptr) {
    auto &segment_meta = writing_segment_->segment_meta();
    stats->total_doc_count += segment_meta.doc_count;
    stats->total_index_file_count += segment_meta.index_file_count;
    stats->total_index_file_size += segment_meta.index_file_size;
    stats->total_segment_count++;
    stats->segment_stats.emplace_back(segment_meta);
  }

  stats->total_index_file_count += 4;
  stats->total_index_file_size += FileHelper::FileSize(id_map_->file_path());
  stats->total_index_file_size +=
      FileHelper::FileSize(delete_store_->file_path());
  stats->total_index_file_size +=
      FileHelper::FileSize(version_manager_->file_path());
  stats->total_index_file_size += FileHelper::FileSize(lsn_store_->file_path());

  return 0;
}

int Collection::update_schema(meta::CollectionMetaPtr new_schema) {
  CHECK_STATUS(opened_, true);

  std::lock_guard<std::mutex> lock(schema_mutex_);
  if (is_dumping_) {
    CLOG_ERROR("Can't update schema while dumping segment.");
    return ErrorCode_StatusError;
  }

  uint32_t new_revision = new_schema->revision();
  uint32_t current_revision = schema_->revision();
  if (new_revision <= current_revision) {
    CLOG_ERROR(
        "New schema revision less than current schema, update failed. "
        "current_schema[%u] new_schema[%u]",
        current_revision, new_revision);
    return ErrorCode_MismatchedSchema;
  }

  std::vector<meta::ColumnMetaPtr> add_columns;
  std::vector<meta::ColumnMetaPtr> delete_columns;
  this->diff_schema(*new_schema, *schema_, &add_columns, &delete_columns);

  int ret = 0;
  std::vector<SegmentPtr> all_segments;
  ret = this->get_segments(&all_segments);
  CHECK_RETURN_WITH_CLOG(ret, 0, "Get segments failed.");

  for (size_t i = 0; i < add_columns.size(); i++) {
    for (size_t j = 0; j < all_segments.size(); j++) {
      ret = all_segments[j]->add_column(add_columns[i]);
      CHECK_RETURN_WITH_CLOG(
          ret, 0, "Add new column failed. column[%s] segment_id[%zu]",
          add_columns[i]->name().c_str(),
          (size_t)all_segments[j]->segment_id());
    }
  }

  for (size_t i = 0; i < delete_columns.size(); i++) {
    for (size_t j = 0; j < all_segments.size(); j++) {
      ret = all_segments[j]->remove_column(delete_columns[i]->name());
      CHECK_RETURN_WITH_CLOG(ret, 0,
                             "Remove column failed. column[%s] segment_id[%zu]",
                             delete_columns[i]->name().c_str(),
                             (size_t)all_segments[j]->segment_id());
    }
  }

  schema_ = new_schema;
  CLOG_INFO("Update schema success. current_schema[%u] new_schema[%u]",
            current_revision, new_revision);

  return 0;
}

int Collection::drive_dump_segment() {
  if (is_dumping_.exchange(true)) {
    return 0;
  }

  // 1. create a new segment for writing
  SegmentMeta new_segment_meta;
  int ret = version_manager_->alloc_segment_meta(&new_segment_meta);
  if (ret != 0) {
    CLOG_ERROR("Alloc segment meta failed.");
    is_dumping_ = false;
    return ret;
  }
  new_segment_meta.min_doc_id =
      writing_segment_->segment_meta().max_doc_id + DOC_ID_INCREASE_COUNT;

  MemorySegmentPtr new_segment;
  ReadOptions read_options;
  read_options.use_mmap = true;
  read_options.create_new = true;
  ret = open_memory_segment(new_segment_meta, read_options, &new_segment);
  if (ret != 0) {
    is_dumping_ = false;
    return ret;
  }

  // 2. swap writing segment -> flushing segment
  MemorySegmentPtr tmp_segment = writing_segment_;
  writing_segment_ = new_segment;
  dumping_segment_ = std::move(tmp_segment);

  // 3. record segment state change
  writing_segment_->update_state(SegmentState::WRITING);
  version_manager_->update_segment_meta(writing_segment_->segment_meta());

  dumping_segment_->flush();
  dumping_segment_->update_state(SegmentState::DUMPING);
  version_manager_->update_segment_meta(dumping_segment_->segment_meta());

  // 4. dump memory segment
  thread_pool_->submit(
      ailego::Closure::New(this, &Collection::do_dump_segment));

  return 0;
}

int Collection::open_memory_segment(const SegmentMeta &segment_meta,
                                    const ReadOptions &read_options,
                                    MemorySegmentPtr *new_segment) {
  int ret = MemorySegment::CreateAndOpen(
      collection_name_, dir_path_, segment_meta, schema_.get(),
      delete_store_.get(), id_map_.get(), concurrency_, read_options,
      new_segment);

  CHECK_RETURN_WITH_CLOG(
      ret, 0, "Create and open memory segment failed. segment_id[%zu]",
      (size_t)segment_meta.segment_id);

  return 0;
}

int Collection::load_persist_segment(const SegmentMeta &segment_meta,
                                     const ReadOptions &read_options,
                                     PersistSegmentPtr *new_segment) {
  int ret = PersistSegment::CreateAndLoad(
      collection_name_, dir_path_, segment_meta, schema_.get(),
      delete_store_.get(), id_map_.get(), concurrency_, read_options,
      new_segment);

  CHECK_RETURN_WITH_CLOG(
      ret, 0, "Create and load persist segment failed. segment_id[%zu]",
      (size_t)segment_meta.segment_id);

  return 0;
}

int Collection::do_dump_segment() {
  SegmentID segment_id = dumping_segment_->segment_id();
  CLOG_INFO("Start dumping segment. segment_id[%zu]", (size_t)segment_id);

  // dump persist segment with retry
  int ret = 0;
  int retry = 0;
  do {
    ret = dumping_segment_->dump();
    if (ret != 0) {
      CLOG_ERROR("Dumping segment failed. retry[%d] segment_id[%zu]", retry,
                 (size_t)segment_id);
    }
  } while (ret != 0 && retry++ < 2);

  if (ret != 0) {
    CLOG_ERROR("Dumping segment failed. segment_id[%zu]", (size_t)segment_id);
    is_dumping_ = false;
    return ret;
  }

  dumping_segment_->update_state(SegmentState::PERSIST);
  version_manager_->update_segment_meta(dumping_segment_->segment_meta());

  // record in version manager with retry
  VersionEdit edit;
  edit.add_segments.emplace_back(segment_id);
  retry = 0;
  do {
    ret = version_manager_->apply(edit);
    if (ret != 0) {
      CLOG_ERROR("Apply new version edit failed. retry[%d]", retry);
    }
  } while (ret != 0 && retry++ < 2);

  if (ret != 0) {
    CLOG_ERROR("Apply new version edit failed.");
    is_dumping_ = false;
    return ret;
  }

  // try to pre load new persist segment into memory
  PersistSegmentPtr persist_segment;
  ReadOptions read_options;
  read_options.use_mmap = true;
  read_options.create_new = false;
  ret = this->load_persist_segment(dumping_segment_->segment_meta(),
                                   read_options, &persist_segment);
  if (ret == 0) {
    persist_segment_mgr_->add_segment(persist_segment);
  }

  // reduce dumping segment ref
  // if search thread release all the refs
  // it will trigger dumping segment auto destruct
  dumping_segment_.reset();

  // shift lsn store
  ret = lsn_store_->shift();
  if (ret != 0) {
    CLOG_WARN("Shift lsn store failed.");
  }

  is_dumping_ = false;
  CLOG_INFO("Ended dumping segment. segment_id[%zu]", (size_t)segment_id);
  return 0;
}

int Collection::recover_from_snapshot(const ReadOptions &read_options) {
  // init version manager
  int ret = VersionManager::CreateAndOpen(collection_name_, dir_path_,
                                          read_options, &version_manager_);
  CHECK_RETURN_WITH_CLOG(ret, 0, "Create and open version manager failed.");

  // init id map
  ret =
      IDMap::CreateAndOpen(collection_name_, dir_path_, read_options, &id_map_);
  CHECK_RETURN_WITH_CLOG(ret, 0, "Create and open id map failed.");

  // init delete store
  ret = DeleteStore::CreateAndOpen(collection_name_, dir_path_, read_options,
                                   &delete_store_);
  CHECK_RETURN_WITH_CLOG(ret, 0, "Create and open delete store failed.");

  // init lsn store
  ret = LsnStore::CreateAndOpen(collection_name_, dir_path_, read_options,
                                &lsn_store_);
  CHECK_RETURN_WITH_CLOG(ret, 0, "Create and open lsn store failed.");

  // init writing segment
  std::vector<SegmentMeta> writing_segment_metas;
  ret = version_manager_->get_segment_metas(SegmentState::WRITING,
                                            &writing_segment_metas);
  CHECK_RETURN_WITH_CLOG(ret, 0, "Get writing segment meta failed.");

  ret = this->open_memory_segment(writing_segment_metas[0], read_options,
                                  &writing_segment_);
  CHECK_RETURN(ret, 0);

  // init dumping segment
  std::vector<SegmentMeta> dumping_segment_metas;
  ret = version_manager_->get_segment_metas(SegmentState::DUMPING,
                                            &dumping_segment_metas);
  CHECK_RETURN_WITH_CLOG(ret, 0, "Get dumping segment meta failed.");

  if (dumping_segment_metas.size() > 0) {
    ret = this->open_memory_segment(dumping_segment_metas[0], read_options,
                                    &dumping_segment_);
    CHECK_RETURN(ret, 0);
    // continue to drive dumping segment
    thread_pool_->submit(
        ailego::Closure::New(this, &Collection::do_dump_segment));
  }

  // init persist segment manager
  persist_segment_mgr_ =
      PersistSegmentManager::Create(collection_name_, dir_path_);
  if (!persist_segment_mgr_) {
    CLOG_ERROR("Create persist segment manager failed.");
    return ErrorCode_RuntimeError;
  }

  // load persist segment & add into psm
  auto &segment_metas = version_manager_->current_version();
  for (size_t i = 0; i < segment_metas.size(); i++) {
    PersistSegmentPtr persist_segment;
    ReadOptions load_options;
    load_options.use_mmap = true;
    load_options.create_new = false;
    ret = this->load_persist_segment(segment_metas[i], load_options,
                                     &persist_segment);
    CHECK_RETURN(ret, 0);
    persist_segment_mgr_->add_segment(persist_segment);
  }

  return 0;
}

void Collection::diff_schema(const meta::CollectionMeta &new_schema,
                             const meta::CollectionMeta &current_schema,
                             std::vector<meta::ColumnMetaPtr> *add_columns,
                             std::vector<meta::ColumnMetaPtr> *delete_columns) {
  auto &new_columns = new_schema.index_columns();
  auto &current_columns = current_schema.index_columns();

  // search in new schema
  // if column not found in current schema
  // just add into new columns
  for (auto &new_column : new_columns) {
    auto it = std::find_if(current_columns.begin(), current_columns.end(),
                           [new_column](const meta::ColumnMetaPtr &cur_column) {
                             return new_column->name() == cur_column->name();
                           });
    if (it == current_columns.end()) {
      add_columns->emplace_back(new_column);
    }
  }

  // search in current schema
  // if column not found in new schema
  // just mark it into delete columns
  for (auto &cur_column : current_columns) {
    auto it = std::find_if(new_columns.begin(), new_columns.end(),
                           [cur_column](const meta::ColumnMetaPtr &new_column) {
                             return cur_column->name() == new_column->name();
                           });

    if (it == current_columns.end()) {
      delete_columns->emplace_back(cur_column);
    }
  }
}


}  // end namespace index
}  // namespace be
}  // end namespace proxima
