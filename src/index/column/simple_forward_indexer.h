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
 *   \brief    SimpleForwardIndexer is simple implementation of ForwardIndexer.
 */

#pragma once

#include <ailego/parallel/lock.h>
#include <aitheta2/index_closet.h>
#include "common/macro_define.h"
#include "common/types.h"
#include "forward_indexer.h"
#include "../snapshot.h"
#include "../typedef.h"

namespace proxima {
namespace be {
namespace index {

class SimpleForwardIndexer;
using SimpleForwardIndexerPtr = std::shared_ptr<SimpleForwardIndexer>;

/*
 * SimpleForwardIndexer implement ForwardIndexer with proxima simple forward
 * module.
 */
class SimpleForwardIndexer : public ForwardIndexer {
 public:
  PROXIMA_DISALLOW_COPY_AND_ASSIGN(SimpleForwardIndexer);

  //! Constructor
  SimpleForwardIndexer(const std::string &coll_name,
                       const std::string &coll_path, SegmentID seg_id) {
    this->set_collection_name(coll_name);
    this->set_collection_path(coll_path);
    this->set_segment_id(seg_id);
  }

  //! Destructor
  ~SimpleForwardIndexer();

 public:
  //! Open persist storage
  int open(const ReadOptions &read_options) override;

  //! Close persist storage
  int close() override;

  //! Flush memory to persist storage
  int flush() override;

  //! Dump forward index
  int dump(IndexDumperPtr dumper) override;

 public:
  //! Insert forward data
  int insert(const ForwardData &fwd_data, idx_t *doc_id) override;

#if 0
  //! Update forward data
  int update(idx_t doc_id, const ForwardData &forward_data) override;
#endif

  //! Remove forward data
  int remove(idx_t doc_id) override;

  //! Seek forward data by doc id
  int seek(idx_t doc_id, ForwardData *fwd_data) override;

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
    if (proxima_forward_) {
      return proxima_forward_->count();
    } else {
      return 0U;
    }
  }

 private:
  int open_proxima_forward();

 private:
  SnapshotPtr snapshot_{};
  IndexClosetPtr proxima_forward_{};

  bool opened_{false};
};


}  // end namespace index
}  // namespace be
}  // end namespace proxima
