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
 *   \brief    Abstract class for module which needs storage snapshot
 */

#pragma once

#include <memory>
#include "common/macro_define.h"
#include "constants.h"
#include "file_helper.h"
#include "typedef.h"

namespace proxima {
namespace be {
namespace index {

/*
 * Snapshot read options:
 * use_mmap: whether use mmap storage
 * create_new: whether force create new file
 */
struct ReadOptions {
  bool use_mmap{false};
  bool create_new{false};
};

class Snapshot;
using SnapshotPtr = std::shared_ptr<Snapshot>;

/*
 * Snapshot represents a kind of storage which
 * will sync data to disk automatically or routinely.
 */
class Snapshot {
 public:
  PROXIMA_DISALLOW_COPY_AND_ASSIGN(Snapshot);

  //! Constructor
  Snapshot(const std::string &dir_path_val, FileID file_id_val,
           uint32_t suffix_id_val, const std::string &suffix_name_val)
      : dir_path_(dir_path_val),
        file_id_(file_id_val),
        suffix_id_(suffix_id_val),
        suffix_name_(suffix_name_val) {}

  //! Destructor
  ~Snapshot();

  //! Constructor
  static SnapshotPtr Create(const std::string &dir_path, FileID file_id,
                            uint32_t suffix_id, const std::string &suffix_name);

  //! Create and open an instance
  static int CreateAndOpen(const std::string &dir_path, FileID file_id,
                           const ReadOptions &options, SnapshotPtr *snapshot);

  //! Create and open an instance
  static int CreateAndOpen(const std::string &dir_path, FileID file_id,
                           uint32_t suffix_id, const ReadOptions &options,
                           SnapshotPtr *snapshot);

  //! Create and open an instance
  static int CreateAndOpen(const std::string &dir_path, FileID file_id,
                           uint32_t suffix_id, const std::string &suffix_name,
                           const ReadOptions &options, SnapshotPtr *snapshot);

 public:
  //! Open persist storage
  int open(const ReadOptions &read_options);

  //! Flush memory data to persist storage
  int flush();

  //! Close persist storage
  int close();

 public:
  //! Return if opened
  bool is_open() const {
    return opened_;
  }

  //! Return directory path
  const std::string &dir_path() const {
    return dir_path_;
  }

  //! Return file id
  FileID file_id() const {
    return file_id_;
  }

  //! Return suffix id
  uint32_t suffix_id() const {
    return suffix_id_;
  }

  //! Return suffix name
  const std::string &suffix_name() const {
    return suffix_name_;
  }

  //! Return file path
  const std::string &file_path() const {
    return file_path_;
  }

  //! Return storage data ptr
  const IndexStoragePtr &data() const {
    return storage_;
  }

 private:
  std::string dir_path_{};
  FileID file_id_{FileID::UNDEFINED};
  uint32_t suffix_id_{0U};
  std::string suffix_name_{};

  std::string file_path_{};
  IndexStoragePtr storage_{};
  bool opened_{false};
};


}  // end namespace index
}  // namespace be
}  // end namespace proxima
