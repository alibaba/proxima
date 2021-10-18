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
 *   \brief    ForwardIndexer can streamly process multiple column
 *             data, building forward index and dump to persist index.
 */

#pragma once

#include "forward_reader.h"

namespace proxima {
namespace be {
namespace index {

class ForwardIndexer;
using ForwardIndexerPtr = std::shared_ptr<ForwardIndexer>;

/*
 * ForwardIndexer process forward data streamly. After
 * accumulating to a certain amount, it will dump to
 * full index, which is persist on disk.
 *
 */
class ForwardIndexer : public ForwardReader {
 public:
  //! Destructor
  virtual ~ForwardIndexer() = default;

  //! Create an instance and return a shared ptr
  static ForwardIndexerPtr Create(const std::string &collection_name,
                                  const std::string &collection_path,
                                  SegmentID segment_id);

 public:
  //! Flush to persist storage
  virtual int flush() = 0;

  //! Dump to full index type
  virtual int dump(IndexDumperPtr dumper) = 0;

  //! Insert a forward data, and return its local index.
  virtual int insert(const ForwardData &forward_data, idx_t *doc_id) = 0;

#if 0
  //! Update forward data by doc_id
  virtual int update(idx_t doc_id, const ForwardData &forward_data) = 0;
#endif

  //! Remove forward data by doc_id
  virtual int remove(idx_t doc_id) = 0;
};


}  // end namespace index
}  // namespace be
}  // end namespace proxima
