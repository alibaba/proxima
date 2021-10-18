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
 *   \brief    SimpleForwardReader can open a persist forward index, and
 *             provides search ability.
 */
#pragma once

#include <memory>
#include "common/macro_define.h"
#include "common/types.h"
#include "forward_reader.h"
#include "../collection_dataset.h"
#include "../typedef.h"

namespace proxima {
namespace be {
namespace index {

class SimpleForwardReader;
using SimpleForwardReaderPtr = std::shared_ptr<SimpleForwardReader>;

/*
 * SimpleForwardReader can open persist forward index, load into memory,
 * and provides search ability.
 */
class SimpleForwardReader : public ForwardReader {
 public:
  PROXIMA_DISALLOW_COPY_AND_ASSIGN(SimpleForwardReader);

  //! Constructor
  SimpleForwardReader(const std::string &coll_name,
                      const std::string &coll_path, SegmentID seg_id) {
    this->set_collection_name(coll_name);
    this->set_collection_path(coll_path);
    this->set_segment_id(seg_id);
  }

  //! Destructor
  ~SimpleForwardReader();

 public:
  //! Open persist storage
  int open(const ReadOptions &read_options) override;

  //! Close persist storage
  int close() override;

  //! Get forward by doc id
  int seek(idx_t doc_id, ForwardData *forward) override;

 public:
  //! Return index path
  std::string index_file_path() const override {
    return index_file_path_;
  }

  //! Return doc count
  size_t doc_count() const override {
    if (forward_searcher_) {
      return forward_searcher_->count();
    } else {
      return 0U;
    }
  }

 private:
  int open_proxima_container(const ReadOptions &read_options);

  int open_forward_searcher();

 private:
  IndexContainerPtr container_{};
  IndexImmutableClosetPtr forward_searcher_{};

  std::string index_file_path_{};
  bool opened_{false};
};


}  // end namespace index
}  // namespace be
}  // end namespace proxima
