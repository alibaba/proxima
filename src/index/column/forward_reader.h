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
 *   \brief    ForwardReader load forward index and provides read ability.
 */

#pragma once

#include <memory>
#include "forward_data.h"
#include "index_provider.h"
#include "../snapshot.h"
#include "../typedef.h"

namespace proxima {
namespace be {
namespace index {

class ForwardReader;
using ForwardReaderPtr = std::shared_ptr<ForwardReader>;

/*
 * ForwardReader load persist forward index, and provides
 * seek ability.
 */
class ForwardReader : public IndexProvider {
 public:
  //! Destructor
  virtual ~ForwardReader() = default;

 public:
  //! Create an instance and return a shared ptr
  static ForwardReaderPtr Create(const std::string &collection_name,
                                 const std::string &collection_path,
                                 SegmentID segment_id);

 public:
  //! Open and load forward index file
  virtual int open(const ReadOptions &read_options) = 0;

  //! Close and release forward index file
  virtual int close() = 0;

  //! Seek a specific doc id
  virtual int seek(idx_t doc_id, ForwardData *forward_data) = 0;

 public:
  //! Set min doc id
  void set_start_doc_id(uint32_t val) {
    start_doc_id_ = val;
  }

  //! Get min doc id
  uint32_t start_doc_id() {
    return start_doc_id_;
  }

 private:
  uint32_t start_doc_id_{0U};
};

}  // end namespace index
}  // namespace be
}  // end namespace proxima
