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
 *   \brief    Reserving delete records of collection
 */

#pragma once

#include <memory>
#include <ailego/container/bitmap.h>
#include "common/macro_define.h"
#include "common/types.h"
#include "concurrent_bitmap.h"
#include "delta_store.h"
#include "snapshot.h"
#include "typedef.h"

namespace proxima {
namespace be {
namespace index {

class DeleteStore;
using DeleteStorePtr = std::shared_ptr<DeleteStore>;

/*
 * DeleteStore is responsible for storage of delete docs.
 * It will store in memory and disk at the same time, and
 * also provides quick search whether one doc exists ability.
 */
class DeleteStore {
 public:
  PROXIMA_DISALLOW_COPY_AND_ASSIGN(DeleteStore);

  //! Constructor
  DeleteStore(const std::string &coll_name, const std::string &coll_path)
      : collection_name_(coll_name), collection_path_(coll_path) {}

  //! Destructor
  ~DeleteStore();

  //! Create an instance
  static DeleteStorePtr Create(const std::string &collection_name,
                               const std::string &collection_path);

  //! Create an instance and open
  static int CreateAndOpen(const std::string &collection_name,
                           const std::string &collection_path,
                           const ReadOptions &options,
                           DeleteStorePtr *delete_store);

 public:
  //! Open persist storage and initialize
  int open(const ReadOptions &options);

  //! Flush memory to persist storage
  int flush();

  //! Close persist storage and cleanup
  int close();

 public:
  //! Insert a doc id
  int insert(idx_t doc_id);

  //! Check if exist a doc id
  bool has(idx_t doc_id) const;

 public:
  //! Return belonged collection name
  const std::string &collection_name() const {
    return collection_name_;
  }

  //! Return persist storage file path
  const std::string &file_path() const {
    return snapshot_->file_path();
  }

  //! Return delete count
  size_t count() const {
    return delta_store_.count();
  }

 private:
  std::string collection_name_{};
  std::string collection_path_{};

  SnapshotPtr snapshot_{};
  DeltaStore<idx_t> delta_store_{};
  ConcurrentBitmap bitmap_{};

  bool opened_{false};
};


}  // end namespace index
}  // namespace be
}  // end namespace proxima
