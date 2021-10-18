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
 *   \brief    Storage of key->doc_id mappings
 */

#pragma once

#include <ailego/parallel/lock.h>
#include "common/macro_define.h"
#include "common/types.h"
#include "persist_hash_map.h"
#include "snapshot.h"

namespace proxima {
namespace be {
namespace index {

class IDMap;
using IDMapPtr = std::shared_ptr<IDMap>;

/*
 * IDMap is responsible for recording pk->doc_id pair in the collection.
 */
class IDMap {
 public:
  PROXIMA_DISALLOW_COPY_AND_ASSIGN(IDMap);

  //! Constructor
  IDMap(const std::string &coll_name, const std::string &coll_path)
      : collection_name_(coll_name), collection_path_(coll_path) {}

  //! Destructor
  ~IDMap();

  //! Create an instance
  static IDMapPtr Create(const std::string &collection_name,
                         const std::string &collection_path);

  //! Create an instance and initialize
  static int CreateAndOpen(const std::string &collection_name,
                           const std::string &collection_path,
                           const ReadOptions &read_options, IDMapPtr *id_map);

 public:
  //! Open persist storage and initialize
  int open(const ReadOptions &read_options);

  //! Flush memory to persist storage
  int flush();

  //! Close persist storage
  int close();

 public:
  //! Insert <key, doc_id> pair
  int insert(uint64_t key, idx_t doc_id);

  //! Remove <key, doc_id> pair
  void remove(uint64_t key);

  //! Check doc primary key exist
  bool has(uint64_t key) const;

  //! Get doc id by primary key
  idx_t get_mapping_id(uint64_t key) const;

 public:
  //! Return belonged collection name
  const std::string &collection_name() const {
    return collection_name_;
  }

  //! Return persist storage file path
  const std::string &file_path() const {
    return snapshot_->file_path();
  }

  //! Return key-id mapping pair count
  size_t count() const {
    return key_map_.size();
  }

 private:
  std::string collection_name_{};
  std::string collection_path_{};

  SnapshotPtr snapshot_{};
  PersistHashMap<uint64_t, idx_t> key_map_{};

  bool opened_{false};
};


}  // end namespace index
}  // namespace be
}  // end namespace proxima
