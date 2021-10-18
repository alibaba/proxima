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
 *   \brief    VectorColumnReader provides search ability for column
 *             vector data
 */

#pragma once

#include "common/macro_define.h"
#include "common/types.h"
#include "column_reader.h"
#include "context_pool.h"
#include "index_helper.h"
#include "../typedef.h"

namespace proxima {
namespace be {
namespace index {

/*
 * VectorColumnReader is an implementation of ColumnReader, mainly
 * for persist column index.
 */
class VectorColumnReader : public ColumnReader {
 public:
  PROXIMA_DISALLOW_COPY_AND_ASSIGN(VectorColumnReader);

  //! Constructor
  VectorColumnReader(const std::string &coll_name, const std::string &coll_path,
                     SegmentID seg_id, const std::string &col_name) {
    this->set_collection_name(coll_name);
    this->set_collection_path(coll_path);
    this->set_segment_id(seg_id);
    this->set_column_name(col_name);
  }

  //! Destructor
  ~VectorColumnReader();

 public:
  //! Open index
  int open(const meta::ColumnMeta &column_meta,
           const ReadOptions &read_options) override;

  //! Close index
  int close() override;

  //! Search similar results with query
  int search(const std::string &query, const QueryParams &query_params,
             FilterFunction filter, IndexDocumentList *results) override;

  //! Batch search similar results with query
  int search(const std::string &query, const QueryParams &query_params,
             uint32_t batch_count, FilterFunction filter,
             std::vector<IndexDocumentList> *batch_result_list) override;

 public:
  //! Return index file path
  std::string index_file_path() const override {
    return index_file_path_;
  }

  //! Return document count
  size_t doc_count() const override {
    if (proxima_searcher_) {
      return proxima_searcher_->stats().loaded_count();
    } else {
      return 0U;
    }
  }

 private:
  bool check_column_meta(const meta::ColumnMeta &column_meta);

  int open_proxima_container(const ReadOptions &read_options);

  int open_proxima_searcher();

 private:
  IndexContainerPtr container_{};
  IndexParams proxima_params_{};
  IndexSearcherPtr proxima_searcher_{};
  IndexMeta proxima_meta_{};

  ContextPool context_pool_{};
  QuantizeTypes quantize_type_{QuantizeTypes::UNDEFINED};
  IndexReformerPtr reformer_{};
  IndexMeasurePtr measure_{};

  std::string index_file_path_{};
  bool opened_{false};
};


}  // end namespace index
}  // namespace be
}  // end namespace proxima
