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
 *   \brief    ColumnIndexer can streamly process column
 *             data, building index and dump to persist index.
 */

#pragma once

#include "column_reader.h"

namespace proxima {
namespace be {
namespace index {

class ColumnIndexer;
using ColumnIndexerPtr = std::shared_ptr<ColumnIndexer>;

/*
 * ColumnIndexer can process column data streamly. After
 * accumulating to a certain amount, it will dump to
 * a full index type.
 */
class ColumnIndexer : public ColumnReader {
 public:
  //! Destructor
  virtual ~ColumnIndexer() = default;

  //! Create instance and return a shared ptr
  static ColumnIndexerPtr Create(const std::string &collection_name,
                                 const std::string &collection_path,
                                 SegmentID segment_id,
                                 const std::string &column_name,
                                 IndexTypes index_type);

 public:
  //! Flush snapshot
  virtual int flush() = 0;

  //! Dump to another kind of index
  virtual int dump(IndexDumperPtr dumper) = 0;

  //! Insert column data
  virtual int insert(idx_t doc_id, const ColumnData &column_data) = 0;

#if 0
  //! Update column data by doc_id
  virtual int update(idx_t doc_id, const ColumnData &column_data) = 0;
#endif

  //! Optimize index structure
  virtual int optimize(ThreadPoolPtr /*pool */) {
    return 0;
  }

  //! Remove column data by doc_id
  virtual int remove(idx_t /*doc_id */) {
    return 0;
  }
};


}  // end namespace index
}  // namespace be
}  // end namespace proxima
