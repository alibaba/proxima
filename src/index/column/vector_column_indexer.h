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
 *   \brief    VectorColumnIndexer process vector column data streamly, it's
 *             a kind of implementation of ColumnIndexer.
 */

#pragma once

#include <condition_variable>
#include <memory>
#include <queue>
#include "common/error_code.h"
#include "common/macro_define.h"
#include "common/types.h"
#include "meta/meta.h"
#include "column_indexer.h"
#include "context_pool.h"
#include "index_helper.h"
#include "../snapshot.h"
#include "../typedef.h"

namespace proxima {
namespace be {
namespace index {

/*
 * VectorColumnIndexer process vector column data streamly, and provides
 * vector search interfaces.
 */
class VectorColumnIndexer : public ColumnIndexer {
 public:
  enum class EngineTypes : uint32_t {
    PROXIMA_HNSW_STREAMER = 0,
    PROXIMA_OSWG_STREAMER = 1
  };

 public:
  PROXIMA_DISALLOW_COPY_AND_ASSIGN(VectorColumnIndexer);

  //! Constructor
  VectorColumnIndexer(const std::string &coll_name,
                      const std::string &coll_path, SegmentID seg_id,
                      const std::string &col_name) {
    this->set_collection_name(coll_name);
    this->set_collection_path(coll_path);
    this->set_segment_id(seg_id);
    this->set_column_name(col_name);
  }

  //! Destructor
  ~VectorColumnIndexer();

 public:
  //! Open persist storage
  int open(const meta::ColumnMeta &column_meta,
           const ReadOptions &read_options) override;

  //! Flush memory to persist storage
  int flush() override;

  //! Close persist storage
  int close() override;

  //! Dump index to persist storage
  int dump(IndexDumperPtr dumper) override;

 public:
  //! Insert vector
  int insert(idx_t doc_id, const ColumnData &column_data) override;

#if 0
  //! Update column data by doc_id
  int update(idx_t doc_id, const ColumnData &column_data) override;
#endif

  //! Remove column data by doc_id
  int remove(idx_t doc_id) override;

  //! Optimize index structure
  int optimize(ThreadPoolPtr pool) override;

  //! Search similar results with query
  int search(const std::string &query, const QueryParams &query_params,
             FilterFunction filter, IndexDocumentList *result_list) override;

  //! Batch search similar results with query
  int search(const std::string &query, const QueryParams &query_params,
             uint32_t batch_count, FilterFunction filter,
             std::vector<IndexDocumentList> *batch_result_list) override;

 public:
  //! Return index path
  std::string index_file_path() const override {
    if (snapshot_) {
      return snapshot_->file_path();
    } else {
      return "";
    }
  }

  //! Return doc count
  size_t doc_count() const override {
    if (proxima_streamer_) {
      return proxima_streamer_->stats().added_count();
    } else {
      return 0U;
    }
  }

 private:
  bool check_column_meta(const meta::ColumnMeta &column_meta);

  int open_proxima_streamer();

  std::string get_engine_name() {
    if (engine_type_ == EngineTypes::PROXIMA_OSWG_STREAMER) {
      return "OswgStreamer";
    } else {
      return "HnswStreamer";
    }
  }

 private:
  SnapshotPtr snapshot_{};
  IndexParams proxima_params_{};
  IndexStreamerPtr proxima_streamer_{};
  IndexMeta proxima_meta_{};
  ContextPool context_pool_{};

  EngineTypes engine_type_{EngineTypes::PROXIMA_OSWG_STREAMER};

  QuantizeTypes quantize_type_{QuantizeTypes::UNDEFINED};
  IndexReformerPtr reformer_{};
  IndexMeasurePtr measure_{};

  bool opened_{false};
};

}  // end namespace index
}  // namespace be
}  // end namespace proxima
