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
 *   \brief    Implementation of persist segment
 */

#include "persist_segment.h"
#include <ailego/utility/time_helper.h>
#include "common/auto_counter.h"
#include "common/error_code.h"
#include "../file_helper.h"

namespace proxima {
namespace be {
namespace index {

PersistSegmentPtr PersistSegment::Create(const std::string &collection_name,
                                         const std::string &collection_path,
                                         const SegmentMeta &segment_meta,
                                         const meta::CollectionMeta *schema,
                                         const DeleteStore *delete_store,
                                         const IDMap *id_map,
                                         uint32_t concurrency) {
  return std::make_shared<PersistSegment>(collection_name, collection_path,
                                          segment_meta, schema, delete_store,
                                          id_map, concurrency);
}

int PersistSegment::CreateAndLoad(
    const std::string &collection_name, const std::string &collection_path,
    const SegmentMeta &segment_meta, const meta::CollectionMeta *schema,
    const DeleteStore *delete_store, const IDMap *id_map, uint32_t concurrency,
    const ReadOptions &read_options, PersistSegmentPtr *persist_segment) {
  (*persist_segment) = Create(collection_name, collection_path, segment_meta,
                              schema, delete_store, id_map, concurrency);

  return (*persist_segment)->load(read_options);
}

PersistSegment::~PersistSegment() {
  if (loaded_) {
    unload();
  }
}

int PersistSegment::load(const ReadOptions &read_options) {
  CHECK_STATUS(loaded_, false);

  // Load forward searcher
  int ret = load_forward_reader(read_options);
  CHECK_RETURN_WITH_SLOG(ret, 0, "Load forward searcher failed.");

  // Load column searchers
  ret = load_column_readers(read_options);
  CHECK_RETURN_WITH_SLOG(ret, 0, "Load column searchers failed.");

  SLOG_DEBUG("Load persist segment success.");
  loaded_ = true;

  return 0;
}

int PersistSegment::unload() {
  CHECK_STATUS(loaded_, true);

  // try to ensure active search requests finished
  uint32_t retry = 0;
  while (retry < MAX_WAIT_RETRY_COUNT && active_search_count_ > 0) {
    LOG_INFO(
        "Try to wait active request finished. active_search_count[%zu] "
        "retry[%d]",
        (size_t)active_search_count_.load(), retry);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    retry++;
  }

  forward_reader_->close();
  for (auto &it : column_readers_) {
    // Check if column searcher is empty searcher
    // because sometimes will add empty column in
    // persist segment.
    if (it.second != nullptr) {
      it.second->close();
    }
  }
  column_readers_.clear();

  loaded_ = false;
  SLOG_DEBUG("Unloaded persist segment.");

  return 0;
}

int PersistSegment::knn_search(const std::string &column_name,
                               const std::string &query,
                               const QueryParams &query_params,
                               QueryResultList *results) {
  CHECK_STATUS(loaded_, true);
  std::vector<QueryResultList> batch_results;
  int ret =
      this->knn_search(column_name, query, query_params, 1, &batch_results);
  CHECK_RETURN(ret, 0);

  (*results) = batch_results[0];
  return 0;
}

int PersistSegment::knn_search(const std::string &column_name,
                               const std::string &query,
                               const QueryParams &query_params,
                               uint32_t batch_count,
                               std::vector<QueryResultList> *batch_results) {
  CHECK_STATUS(loaded_, true);

  AutoCounter as(active_search_count_);

  ailego::ElapsedTime timer;
  uint64_t query_id = query_params.query_id;

  if (!column_readers_.has(column_name)) {
    SLOG_ERROR("Column not exist. query_id[%zu] column_name[%s]",
               (size_t)query_id, column_name.c_str());
    return ErrorCode_InexistentColumn;
  }

  auto &column_reader = column_readers_.get(column_name);
  // check if column searcher is empty searcher
  // it means this column added later by update schema
  // so we just return empty results
  if (!column_reader) {
    SLOG_INFO(
        "Empty column searcher return empty results. query_id[%zu] "
        "batch_count[%u] topk[%u] "
        "res_num[0] cost[%zums] column[%s]",
        (size_t)query_id, batch_count, query_params.topk,
        (size_t)timer.milli_seconds(), column_name.c_str());
    return 0;
  }

  // search columns
  std::vector<IndexDocumentList> batch_search_results;
  FilterFunction filter = nullptr;
  if (delete_store_ && delete_store_->count() > 0) {
    filter = [this](idx_t doc_id) { return delete_store_->has(doc_id); };
  }

  int ret = column_reader->search(query, query_params, batch_count, filter,
                                  &batch_search_results);
  CHECK_RETURN_WITH_SLOG(
      ret, 0, "Column searcher search failed. query_id[%zu] column[%s]",
      (size_t)query_id, column_name.c_str());

  // fill results
  uint32_t res_num = 0U;
  for (size_t i = 0; i < batch_search_results.size(); i++) {
    auto &search_results = batch_search_results[i];
    QueryResultList output_result_list;
    for (size_t j = 0; j < search_results.size(); j++) {
      idx_t doc_id = search_results[j].key();
      ForwardData fwd_data;
      ret = forward_reader_->seek(doc_id, &fwd_data);
      if (ret != 0) {
        SLOG_WARN("Forward not exist. query_id[%zu] doc_id[%zu] column[%s] ",
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

int PersistSegment::kv_search(uint64_t primary_key, QueryResult *result) {
  CHECK_STATUS(loaded_, true);

  idx_t doc_id = id_map_->get_mapping_id(primary_key);
  bool found = false;
  result->primary_key = INVALID_KEY;

  if (!delete_store_->has(doc_id)) {
    if (doc_id >= segment_meta_.min_doc_id &&
        doc_id <= segment_meta_.max_doc_id) {
      ForwardData fwd_data;
      int ret = forward_reader_->seek(doc_id, &fwd_data);
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

int PersistSegment::remove_column(const std::string &column_name) {
  CHECK_STATUS(loaded_, true);
  if (!column_readers_.has(column_name)) {
    SLOG_WARN("Column not exist, remove failed. column[%s]",
              column_name.c_str());
    return 0;
  }

  column_readers_.get(column_name)->close();
  column_readers_.erase(column_name);

  SLOG_INFO("Remove column done. column[%s]", column_name.c_str());
  return 0;
}

int PersistSegment::add_column(const meta::ColumnMetaPtr &column_meta) {
  CHECK_STATUS(loaded_, true);
  std::string column_name = column_meta->name();
  if (column_readers_.has(column_meta->name())) {
    SLOG_WARN("Column already exist, remove failed. column[%s]",
              column_name.c_str());
    return 0;
  }

  // occupy a empty column, it will skip in query process
  column_readers_.emplace(column_name, ColumnReaderPtr());

  SLOG_INFO("Add column success. column[%s]", column_name.c_str());
  return 0;
}

int PersistSegment::load_forward_reader(const ReadOptions &read_options) {
  forward_reader_ = ForwardReader::Create(collection_name_, collection_path_,
                                          segment_meta_.segment_id);
  if (!forward_reader_) {
    SLOG_ERROR("Forward reader create failed.");
    return ErrorCode_RuntimeError;
  }

  forward_reader_->set_start_doc_id(segment_meta_.min_doc_id);
  int ret = forward_reader_->open(read_options);
  CHECK_RETURN_WITH_SLOG(ret, 0, "Open forward reader failed.");

  SLOG_DEBUG("Opened forward reader.");
  return 0;
}

int PersistSegment::load_column_readers(const ReadOptions &read_options) {
  for (auto &column_meta : schema_->index_columns()) {
    std::string column_name = column_meta->name();

    ColumnReaderPtr new_column_reader = ColumnReader::Create(
        collection_name_, collection_path_, segment_meta_.segment_id,
        column_name, column_meta->index_type());
    if (!new_column_reader) {
      SLOG_ERROR("Create column reader failed. index_type[%d] column[%s]",
                 column_meta->index_type(), column_name.c_str());
      return ErrorCode_RuntimeError;
    }

    new_column_reader->set_concurrency(concurrency_);
    int ret = new_column_reader->open(*column_meta.get(), read_options);
    CHECK_RETURN_WITH_SLOG(
        ret, 0, "Open column reader failed. index_type[%d] column[%s]",
        column_meta->index_type(), column_name.c_str());

    column_readers_.emplace(column_name, new_column_reader);
  }

  return 0;
}


}  // end namespace index
}  // namespace be
}  // end namespace proxima
