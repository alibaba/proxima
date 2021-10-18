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
 *   \date     Jun 2021
 *   \brief    ColumnReader load column index data and provides read interfaces.
 */

#pragma once

#include <thread>
#include "index_provider.h"
#include "../collection_dataset.h"
#include "../collection_query.h"
#include "../snapshot.h"

namespace proxima {
namespace be {
namespace index {

class ColumnReader;
using ColumnReaderPtr = std::shared_ptr<ColumnReader>;

using FilterFunction = std::function<bool(idx_t doc_id)>;

/*
 * ColumnReader load persist column index, and provides search interfaces.
 */
class ColumnReader : public IndexProvider {
 public:
  //! Constructor
  ColumnReader() {
    concurrency_ = std::thread::hardware_concurrency();
  }

  //! Destructor
  virtual ~ColumnReader() = default;

  //! Create an instance and return shared ptr
  static ColumnReaderPtr Create(const std::string &collection_name,
                                const std::string &collection_path,
                                SegmentID segment_id,
                                const std::string &column_name,
                                IndexTypes index_type);

 public:
  //! Open snapshot on persist storage
  virtual int open(const meta::ColumnMeta &column_meta,
                   const ReadOptions &read_options) = 0;

  //! Close persist storage
  virtual int close() = 0;


  //! Search similar results with query
  virtual int search(const std::string &query, const QueryParams &query_params,
                     FilterFunction filter, IndexDocumentList *result_list) = 0;

  //! Batch search similar results with query
  virtual int search(const std::string &query, const QueryParams &query_params,
                     uint32_t batch_count, FilterFunction filter,
                     std::vector<IndexDocumentList> *batch_result_list) = 0;

 public:
  //! Set concurrency
  void set_concurrency(uint32_t val) {
    concurrency_ = val;
  }

  //! Return concurrency
  uint32_t concurrency() {
    return concurrency_;
  }

 private:
  uint32_t concurrency_{0U};
};


}  // end namespace index
}  // namespace be
}  // end namespace proxima
