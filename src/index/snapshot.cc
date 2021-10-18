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
 *   \brief    Implementation of Snapshot
 */

#include "snapshot.h"
#include <aitheta2/index_factory.h>

namespace proxima {
namespace be {
namespace index {

Snapshot::~Snapshot() {
  if (opened_) {
    this->close();
  }
}

SnapshotPtr Snapshot::Create(const std::string &dir_path, FileID file_id,
                             uint32_t suffix_id,
                             const std::string &suffix_name) {
  return std::make_shared<Snapshot>(dir_path, file_id, suffix_id, suffix_name);
}

int Snapshot::CreateAndOpen(const std::string &dir_path, FileID file_id,
                            const ReadOptions &options, SnapshotPtr *snapshot) {
  *snapshot = Create(dir_path, file_id, INVALID_SEGMENT_ID, "");

  return (*snapshot)->open(options);
}

int Snapshot::CreateAndOpen(const std::string &dir_path, FileID file_id,
                            uint32_t suffix_id, const ReadOptions &options,
                            SnapshotPtr *snapshot) {
  *snapshot = Create(dir_path, file_id, suffix_id, "");

  return (*snapshot)->open(options);
}

int Snapshot::CreateAndOpen(const std::string &dir_path, FileID file_id,
                            uint32_t suffix_id, const std::string &suffix_name,
                            const ReadOptions &options, SnapshotPtr *snapshot) {
  *snapshot = Create(dir_path, file_id, suffix_id, suffix_name);

  return (*snapshot)->open(options);
}

int Snapshot::open(const ReadOptions &read_options) {
  CHECK_STATUS(opened_, false);

  if (read_options.use_mmap) {
    storage_ = IndexFactory::CreateStorage("MMapFileStorage");
  } else {
    storage_ = IndexFactory::CreateStorage("MemoryStorage");
  }

  if (!storage_) {
    LOG_ERROR("Create storage failed.");
    return ErrorCode_RuntimeError;
  }

  if (suffix_id_ != INVALID_SEGMENT_ID) {
    if (suffix_name_.empty()) {
      file_path_ = FileHelper::MakeFilePath(dir_path_, file_id_, suffix_id_);
    } else {
      file_path_ = FileHelper::MakeFilePath(dir_path_, file_id_, suffix_id_,
                                            suffix_name_);
    }
  } else {
    file_path_ = FileHelper::MakeFilePath(dir_path_, file_id_);
  }

  // Default open warmup flag
  IndexParams stg_params;
  stg_params.set("proxima.mmap_file.storage.memory_warmup", true);

  storage_->init(stg_params);

  int ret = storage_->open(file_path_, read_options.create_new);
  CHECK_RETURN_WITH_LOG(ret, 0, "Open storage failed. file_path[%s] ret[%d]",
                        file_path_.c_str(), ret);

  opened_ = true;
  return 0;
}

int Snapshot::flush() {
  CHECK_STATUS(opened_, true);

  return storage_->flush();
}

int Snapshot::close() {
  CHECK_STATUS(opened_, true);

  int ret = storage_->close();
  CHECK_RETURN(ret, 0);

  opened_ = false;
  return 0;
}


}  // end namespace index
}  // namespace be
}  // end namespace proxima
