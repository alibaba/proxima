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
 *   \brief    Implementation of memory segment
 */

#include "memory_segment.h"
#include <chrono>
#include <ailego/utility/time_helper.h>
#include "common/error_code.h"
#include "../file_helper.h"
#include "../typedef.h"

namespace proxima {
namespace be {
namespace index {


MemorySegmentPtr MemorySegment::Create(const std::string &collection_name,
                                       const std::string &collection_path,
                                       const SegmentMeta &segment_meta,
                                       const meta::CollectionMeta *schema,
                                       const DeleteStore *delete_store,
                                       const IDMap *id_map,
                                       uint32_t concurrency) {
  return std::make_shared<MemorySegment>(collection_name, collection_path,
                                         segment_meta, schema, delete_store,
                                         id_map, concurrency);
}

int MemorySegment::CreateAndOpen(
    const std::string &collection_name, const std::string &collection_path,
    const SegmentMeta &segment_meta, const meta::CollectionMeta *schema,
    const DeleteStore *delete_store, const IDMap *id_map, uint32_t concurrency,
    const ReadOptions &read_options, MemorySegmentPtr *memory_segment) {
  *memory_segment = Create(collection_name, collection_path, segment_meta,
                           schema, delete_store, id_map, concurrency);

  return (*memory_segment)->open(read_options);
}


MemorySegment::~MemorySegment() {
  if (opened_) {
    if (segment_meta_.state == SegmentState::PERSIST) {
      close_and_remove_files();
    } else {
      close();
    }
  }
}

int MemorySegment::open(const ReadOptions &read_options) {
  CHECK_STATUS(opened_, false);

  int ret = open_forward_indexer(read_options);
  CHECK_RETURN(ret, 0);

  ret = open_column_indexers(read_options);
  CHECK_RETURN(ret, 0);

  segment_meta_.index_file_count = this->get_index_file_count();
  segment_meta_.index_file_size = this->get_index_file_size();

  opened_ = true;
  SLOG_INFO("Opened memory segment.");
  return 0;
}

int MemorySegment::close() {
  CHECK_STATUS(opened_, true);

  // try to ensure active insert requests finished
  uint32_t retry = 0;
  while (retry < MAX_WAIT_RETRY_COUNT &&
         (active_insert_count_ > 0 || active_search_count_ > 0)) {
    LOG_INFO(
        "Try to wait active request finished. active_insert_count[%zu] "
        "active_search_count[%zu] retry[%d]",
        (size_t)active_insert_count_.load(),
        (size_t)active_search_count_.load(), retry);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    retry++;
  }

  forward_indexer_->close();
  for (auto &it : column_indexers_) {
    it.second->close();
  }
  column_indexers_.clear();

  opened_ = false;
  SLOG_DEBUG("Closed memory segment.");
  return 0;
}


int MemorySegment::flush() {
  CHECK_STATUS(opened_, true);

  forward_indexer_->flush();
  for (auto &it : column_indexers_) {
    it.second->flush();
  }

  segment_meta_.index_file_count = this->get_index_file_count();
  segment_meta_.index_file_size = this->get_index_file_size();
  return 0;
}

int MemorySegment::dump() {
  CHECK_STATUS(opened_, true);

  // try to ensure active insert requests finished
  uint32_t retry = 0;
  while (retry < MAX_WAIT_RETRY_COUNT && active_insert_count_ > 0) {
    LOG_INFO(
        "Try to wait active request finished. active_insert_count[%zu] "
        "retry[%d]",
        (size_t)active_insert_count_.load(), retry);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    retry++;
  }

  auto dumper = aitheta2::IndexFactory::CreateDumper("FileDumper");
  if (!dumper) {
    SLOG_ERROR("Create dumper failed.");
    return ErrorCode_RuntimeError;
  }

  std::string segment_file_path = FileHelper::MakeFilePath(
      collection_path_, FileID::SEGMENT_FILE, segment_meta_.segment_id);

  int ret = dumper->create(segment_file_path);
  CHECK_RETURN_WITH_CLOG(ret, 0, "Create dumper file failed.");

  ret = dump_forward_indexer(dumper);
  CHECK_RETURN(ret, 0);

  ret = dump_column_indexers(dumper);
  CHECK_RETURN(ret, 0);

  dumper->close();

  segment_meta_.index_file_count = 1U;
  segment_meta_.index_file_size = FileHelper::FileSize(segment_file_path);
  return 0;
}

int MemorySegment::close_and_remove_files() {
  CHECK_STATUS(opened_, true);

  forward_indexer_->close();
  FileHelper::RemoveFile(forward_indexer_->index_file_path());

  for (auto &it : column_indexers_) {
    it.second->close();
    FileHelper::RemoveFile(it.second->index_file_path());
  }
  column_indexers_.clear();

  opened_ = false;
  SLOG_DEBUG("Closed memory segment and remove index files.");
  return 0;
}

int MemorySegment::insert(const Record &record, idx_t *doc_id) {
  CHECK_STATUS(opened_, true);

  AutoCounter ac(active_insert_count_);

  // 1. insert into forward indexer first
  ForwardData fwd_data;
  fwd_data.header.primary_key = record.primary_key;
  fwd_data.header.timestamp = record.timestamp;
  fwd_data.header.lsn = record.lsn;
  fwd_data.header.revision = record.revision;
  fwd_data.data = std::move(record.forward_data);

  int ret = forward_indexer_->insert(fwd_data, doc_id);
  CHECK_RETURN_WITH_SLOG(ret, 0, "Insert into forward indexer failed. key[%zu]",
                         (size_t)record.primary_key);

  // 2. insert into column indexers
  for (size_t i = 0; i < record.column_datas.size(); i++) {
    auto &column_data = record.column_datas[i];
    std::string column_name = column_data.column_name;

    // Skip not-exist column
    if (!column_indexers_.has(column_name)) {
      SLOG_ERROR("Not find column indexer. column[%s]", column_name.c_str());
      continue;
    }

    auto &column_indexer = column_indexers_.get(column_name);
    ret = column_indexer->insert(*doc_id, column_data);
    CHECK_RETURN_WITH_SLOG(
        ret, 0, "Insert into column indexer failed. key[%zu] column[%s]",
        (size_t)record.primary_key, column_name.c_str());
  }

  // 3. update segment stats
  update_stats(record, *doc_id);
  return 0;
}

int MemorySegment::remove(idx_t doc_id) {
  CHECK_STATUS(opened_, true);

  ailego::ElapsedTime timer;
  int ret = 0;

  // No need to remove forward data.
#if 0
  ret = forward_indexer_->remove(doc_id);
  CHECK_RETURN_WITH_SLOG(ret, 0,
                         "Remove from forward indexer failed. doc_id[%zu]",
                         (size_t)doc_id);
#endif

  for (auto &column_meta : schema_->index_columns()) {
    std::string column_name = column_meta->name();
    if (column_indexers_.has(column_name)) {
      ret = column_indexers_.get(column_name)->remove(doc_id);
      if (ret != 0) {
        SLOG_WARN(
            "Remove from column indexer failed. column_name[%s] doc_id[%zu]",
            column_name.c_str(), (size_t)doc_id);
        continue;
      }
    }
  }

  SLOG_DEBUG("Remove from memory segment success. doc_id[%lu] cost[%zuus]",
             (size_t)doc_id, timer.micro_seconds());
  return 0;
}

int MemorySegment::optimize(ThreadPoolPtr pool) {
  CHECK_STATUS(opened_, true);

  int ret = 0;
  for (auto &column_meta : schema_->index_columns()) {
    std::string column_name = column_meta->name();
    if (column_indexers_.has(column_name)) {
      ret = column_indexers_.get(column_name)->optimize(pool);
      if (ret != 0) {
        SLOG_WARN("Optimize column indexer failed. column_name[%s]",
                  column_name.c_str());
        continue;
      }
    }
  }

  return 0;
}

#if 0
int MemorySegment::update(idx_t doc_id, const Record &record) {
  CHECK_STATUS(opened_, true);

  AutoCounter ac(active_insert_count_);

  // 1. insert into forward indexer first
  ForwardData fwd_data;
  fwd_data.header.primary_key = record.primary_key;
  fwd_data.header.timestamp = record.timestamp;
  fwd_data.header.lsn = record.lsn;
  fwd_data.header.revision = record.revision;
  fwd_data.data = std::move(record.forward_data);

  int ret = forward_indexer_->update(doc_id, fwd_data);
  CHECK_RETURN_WITH_SLOG(ret, 0, "Update forward indexer failed. key[%zu]",
                         (size_t)record.primary_key);

  // 2. insert into column indexers
  for (size_t i = 0; i < record.column_datas.size(); i++) {
    auto &column_data = record.column_datas[i];
    auto &column_name = column_data.column_name;

    // Skip not-exist column
    if (!column_indexers_.has(column_name)) {
      SLOG_ERROR("Not find column indexer. column[%s]", column_name.c_str());
      continue;
    }

    auto &column_indexer = column_indexers_.get(column_name);
    ret = column_indexer->update(doc_id, column_data);
    CHECK_RETURN_WITH_SLOG(
        ret, 0, "Update column indexer failed. key[%zu] column[%s]",
        (size_t)record.primary_key, column_name.c_str());
  }

  return 0;
}
#endif

int MemorySegment::knn_search(const std::string &column_name,
                              const std::string &query,
                              const QueryParams &query_params,
                              QueryResultList *result) {
  CHECK_STATUS(opened_, true);

  std::vector<QueryResultList> batch_results;
  int ret =
      this->knn_search(column_name, query, query_params, 1, &batch_results);
  CHECK_RETURN(ret, 0);

  (*result) = batch_results[0];
  return 0;
}

int MemorySegment::knn_search(const std::string &column_name,
                              const std::string &query,
                              const QueryParams &query_params,
                              uint32_t batch_count,
                              std::vector<QueryResultList> *batch_results) {
  CHECK_STATUS(opened_, true);

  AutoCounter as(active_search_count_);

  ailego::ElapsedTime timer;
  uint64_t query_id = query_params.query_id;
  if (!column_indexers_.has(column_name)) {
    SLOG_ERROR("Column not exist. query_id[%zu] column[%s]", (size_t)query_id,
               column_name.c_str());
    return ErrorCode_InexistentColumn;
  }

  // check query format
  auto &column_indexer = column_indexers_.get(column_name);

  // search columns
  std::vector<IndexDocumentList> batch_search_results;
  FilterFunction filter = nullptr;
  // If user choose to use deep delete, then we don't need to pass filter
  // to column indexer.
  if (delete_store_ && delete_store_->count() > 0) {
    filter = [this](idx_t doc_id) { return delete_store_->has(doc_id); };
  }

  int ret = column_indexer->search(query, query_params, batch_count, filter,
                                   &batch_search_results);
  CHECK_RETURN_WITH_SLOG(
      ret, 0, "Column indexer search failed. query_id[%zu] column[%s]",
      (size_t)query_id, column_name.c_str());

  // fill results
  uint32_t res_num = 0U;
  for (size_t i = 0; i < batch_search_results.size(); i++) {
    auto &search_results = batch_search_results[i];
    QueryResultList output_result_list;
    for (size_t j = 0; j < search_results.size(); j++) {
      idx_t doc_id = search_results[j].key();
      ForwardData fwd_data;
      ret = forward_indexer_->seek(doc_id, &fwd_data);
      if (ret != 0) {
        SLOG_WARN(
            "Forward data not exist. query_id[%zu] doc_id[%zu] column[%s]",
            (size_t)query_id, (size_t)doc_id, column_name.c_str());
        continue;
      }
      QueryResult res;
      res.primary_key = fwd_data.header.primary_key;
      res.score = search_results[j].score();
      res.revision = fwd_data.header.revision;
      res.forward_data = std::move(fwd_data.data);
      res.lsn = fwd_data.header.lsn;
      output_result_list.emplace_back(res);
    }
    res_num += search_results.size();
    batch_results->emplace_back(output_result_list);
  }

  SLOG_DEBUG(
      "Knn search query success. query_id[%zu] "
      "batch_count[%u] topk[%u] res_num[%u] cost[%zuus] column[%s]",
      (size_t)query_id, batch_count, query_params.topk, res_num,
      (size_t)timer.micro_seconds(), column_name.c_str());

  return 0;
}

int MemorySegment::kv_search(uint64_t primary_key, QueryResult *result) {
  CHECK_STATUS(opened_, true);

  idx_t doc_id = id_map_->get_mapping_id(primary_key);
  bool found = false;
  result->primary_key = INVALID_KEY;

  if (!delete_store_->has(doc_id)) {
    if (doc_id >= segment_meta_.min_doc_id &&
        doc_id <= segment_meta_.max_doc_id) {
      ForwardData fwd_data;
      int ret = forward_indexer_->seek(doc_id, &fwd_data);
      if (ret == 0 && fwd_data.header.primary_key != INVALID_KEY) {
        result->primary_key = fwd_data.header.primary_key;
        result->revision = fwd_data.header.revision;
        result->forward_data = std::move(fwd_data.data);
        result->lsn = fwd_data.header.lsn;
        found = true;
      }
    }
  }

  SLOG_DEBUG("Kv search query success. key[%zu] found[%d]", (size_t)primary_key,
             found);
  return 0;
}

int MemorySegment::remove_column(const std::string &column_name) {
  CHECK_STATUS(opened_, true);

  if (!column_indexers_.has(column_name)) {
    SLOG_WARN("Column not exist, remove failed. column[%s]",
              column_name.c_str());
    return 0;
  }

  column_indexers_.get(column_name)->close();
  column_indexers_.erase(column_name);

  SLOG_INFO("Remove column done. column[%s]", column_name.c_str());
  return 0;
}

int MemorySegment::add_column(const meta::ColumnMetaPtr &column_meta) {
  CHECK_STATUS(opened_, true);

  std::string column_name = column_meta->name();
  if (column_indexers_.has(column_name)) {
    SLOG_WARN("Column already exist, add failed. column[%s]",
              column_name.c_str());
    return 0;
  }

  ReadOptions read_options;
  read_options.use_mmap = true;
  read_options.create_new = true;

  ColumnIndexerPtr column_indexer = ColumnIndexer::Create(
      collection_name_, collection_path_, segment_meta_.segment_id, column_name,
      column_meta->index_type());

  if (!column_indexer) {
    SLOG_ERROR("Create column indexer failed. index_type[%d] column[%s]",
               column_meta->index_type(), column_name.c_str());
    return ErrorCode_RuntimeError;
  }

  column_indexer->set_concurrency(concurrency_);
  int ret = column_indexer->open(*column_meta.get(), read_options);
  CHECK_RETURN_WITH_SLOG(
      ret, 0, "Create and open column indexer failed. ret[%d] column[%s]", ret,
      column_name.c_str());

  column_indexers_.emplace(column_name, column_indexer);
  SLOG_INFO("Add column success.column[%s]", column_name.c_str());

  return 0;
}

int MemorySegment::open_forward_indexer(const ReadOptions &read_options) {
  forward_indexer_ = ForwardIndexer::Create(collection_name_, collection_path_,
                                            segment_meta_.segment_id);
  if (!forward_indexer_) {
    SLOG_ERROR("Create forward indexer failed.");
    return ErrorCode_RuntimeError;
  }

  forward_indexer_->set_start_doc_id(segment_meta_.min_doc_id);
  int ret = forward_indexer_->open(read_options);
  CHECK_RETURN_WITH_SLOG(ret, 0, "Open forward indexer failed.");

  SLOG_DEBUG("Opened forward indexer. min_doc_id[%zu] forward_count[%zu]",
             (size_t)segment_meta_.min_doc_id,
             (size_t)forward_indexer_->doc_count());
  return 0;
}

int MemorySegment::open_column_indexers(const ReadOptions &read_options) {
  for (auto &column_meta : schema_->index_columns()) {
    std::string column_name = column_meta->name();

    ColumnIndexerPtr column_indexer = ColumnIndexer::Create(
        collection_name_, collection_path_, segment_meta_.segment_id,
        column_name, column_meta->index_type());
    if (!column_indexer) {
      SLOG_ERROR("Create column indexer failed. index_type[%d] column[%s]",
                 column_meta->index_type(), column_name.c_str());
      return ErrorCode_RuntimeError;
    }

    int ret = column_indexer->open(*column_meta.get(), read_options);
    CHECK_RETURN_WITH_SLOG(
        ret, 0, "Create and open column indexer failed. ret[%d] column[%s]",
        ret, column_name.c_str());

    column_indexers_.emplace(column_name, column_indexer);
    SLOG_DEBUG("Opened column indexer. column[%s]", column_name.c_str());
  }

  return 0;
}

int MemorySegment::dump_forward_indexer(const IndexDumperPtr &dumper) {
  IndexDumperPtr fwd_dumper =
      std::make_shared<IndexSegmentDumper>(dumper, FORWARD_DUMP_BLOCK);
  int ret = forward_indexer_->dump(fwd_dumper);
  CHECK_RETURN_WITH_SLOG(ret, 0, "Dump forward indexer failed.");

  fwd_dumper->close();
  return 0;
}

int MemorySegment::dump_column_indexers(const IndexDumperPtr &dumper) {
  for (auto &it : column_indexers_) {
    std::string column_name = it.first;
    auto &column_indexer = it.second;
    IndexDumperPtr index_dumper = std::make_shared<IndexSegmentDumper>(
        dumper, COLUMN_DUMP_BLOCK + column_name);
    int ret = column_indexer->dump(index_dumper);
    CHECK_RETURN_WITH_SLOG(ret, 0, "Dump column indexer failed. column[%s]",
                           column_name.c_str());
    index_dumper->close();
  }
  return 0;
}

void MemorySegment::update_stats(const Record &record, idx_t doc_id) {
  std::lock_guard<std::mutex> lock(mutex_);
  segment_meta_.doc_count++;

  if (doc_id > segment_meta_.max_doc_id) {
    segment_meta_.max_doc_id = doc_id;
  }

  if (record.primary_key < segment_meta_.min_primary_key) {
    segment_meta_.min_primary_key = record.primary_key;
  }

  if (record.primary_key > segment_meta_.max_primary_key) {
    segment_meta_.max_primary_key = record.primary_key;
  }

  if (record.timestamp < segment_meta_.min_timestamp) {
    segment_meta_.min_timestamp = record.timestamp;
  }

  if (record.timestamp > segment_meta_.max_timestamp) {
    segment_meta_.max_timestamp = record.timestamp;
  }

  if (record.lsn > segment_meta_.max_lsn) {
    segment_meta_.max_lsn = record.lsn;
  }

  if (record.lsn < segment_meta_.min_lsn) {
    segment_meta_.min_lsn = record.lsn;
  }
}

size_t MemorySegment::get_index_file_count() {
  return column_indexers_.size() + 1;
}

size_t MemorySegment::get_index_file_size() {
  size_t file_size = 0U;
  for (auto &it : column_indexers_) {
    file_size += FileHelper::FileSize(it.second->index_file_path());
  }

  file_size += FileHelper::FileSize(forward_indexer_->index_file_path());
  return file_size;
}


}  // end namespace index
}  // namespace be
}  // end namespace proxima
