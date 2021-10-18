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
 *   \brief    Provides storage & search ability of lsn context information
 */

#pragma once

#include <ailego/container/heap.h>
#include <ailego/parallel/lock.h>
#include "common/macro_define.h"
#include "snapshot.h"
#include "typedef.h"

namespace proxima {
namespace be {
namespace index {

class LsnStore;
using LsnStorePtr = std::shared_ptr<LsnStore>;

/*
 * LsnStore is mainly for storage of mysql log sequence number and context.
 */
class LsnStore {
 public:
  /*
   * Header info
   */
  struct Header {
    uint64_t tail_block_index{0U};
    uint64_t lsn_count{0U};
    uint64_t reserved_[6];
  };

  static_assert(sizeof(Header) % 64 == 0,
                "Header must be aligned with 64 bytes");

  /*
   * LSN struct, includes lsn and context
   */
  struct LSN {
    uint64_t lsn{0U};
    std::string lsn_context;

    LSN() = default;

    LSN(uint64_t lsn_val, const std::string &lsn_context_val) {
      lsn = lsn_val;
      lsn_context = lsn_context_val;
    }

    bool operator<(const LSN &other) const {
      return lsn < other.lsn;
    }

    bool operator>(const LSN &other) const {
      return lsn > other.lsn;
    }
  };

 public:
  PROXIMA_DISALLOW_COPY_AND_ASSIGN(LsnStore);

  //! Constructor
  LsnStore(const std::string &coll_name, const std::string &coll_path)
      : collection_name_(coll_name), collection_path_(coll_path) {}

  //! Destructor
  ~LsnStore();

  //! Create instance
  static LsnStorePtr Create(const std::string &collection_name,
                            const std::string &collection_path);

  //! Create instance and open
  static int CreateAndOpen(const std::string &collection_name,
                           const std::string &collection_path,
                           const ReadOptions &read_options,
                           LsnStorePtr *lsn_store);

 public:
  //! Open persist storage and initialize
  int open(const ReadOptions &read_options);

  //! Flush memory to persist storage
  int flush();

  //! Close persist storage
  int close();

 public:
  //! Append <lsn, lsn_context> pair
  int append(uint64_t lsn, const std::string &lsn_context);

  //! Shift inner segment
  int shift();

  //! Get latest lsn and context
  int get_latest_lsn(uint64_t *lsn, std::string *lsn_context);

 public:
  //! Return collection name
  const std::string &collection_name() const {
    return collection_name_;
  }

  //! Return index file path
  const std::string &file_path() const {
    return file_path_;
  }

  //! Return stored lsn count
  size_t count() const {
    return header_.lsn_count;
  }

 private:
  int mount();

  void unmount();

  int update_header();

 private:
  static constexpr uint64_t kWindowSize = 2000;
  static constexpr uint64_t kDataBlockCount = 3;
  static constexpr uint64_t kDataBlockSize = 1UL * 1024UL * 1024UL;

 private:
  std::string collection_name_{};
  std::string collection_path_{};
  std::string file_path_{};

  SnapshotPtr snapshot_{};
  IndexBlockPtr header_block_{};
  std::vector<IndexBlockPtr> data_blocks_{};
  Header header_;
  ailego::SharedMutex mutex_{};

  bool opened_{false};
};


}  // end namespace index
}  // namespace be
}  // end namespace proxima
