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
 *   \brief    Implementation of delete store
 */

#include "delete_store.h"
#include <limits>
#include "common/error_code.h"
#include "file_helper.h"
#include "typedef.h"

namespace proxima {
namespace be {
namespace index {

DeleteStorePtr DeleteStore::Create(const std::string &collection_name,
                                   const std::string &collection_path) {
  return std::make_shared<DeleteStore>(collection_name, collection_path);
}

int DeleteStore::CreateAndOpen(const std::string &collection_name,
                               const std::string &collection_path,
                               const ReadOptions &options,
                               DeleteStorePtr *delete_store) {
  (*delete_store) = Create(collection_name, collection_path);

  return (*delete_store)->open(options);
}

DeleteStore::~DeleteStore() {
  if (opened_) {
    this->close();
  }
}

int DeleteStore::open(const ReadOptions &read_options) {
  CHECK_STATUS(opened_, false);

  int ret = Snapshot::CreateAndOpen(collection_path_, FileID::DELETE_FILE,
                                    read_options, &snapshot_);
  CHECK_RETURN_WITH_CLOG(ret, 0, "Create and open snapshot failed.");

  ret = delta_store_.mount(snapshot_->data());
  CHECK_RETURN_WITH_CLOG(ret, 0, "Mount snapshot failed.");

  for (size_t i = 0; i < delta_store_.count(); i++) {
    bitmap_.set(*(delta_store_.at(i)));
  }

  // To avoid bitmap resize
  bitmap_.set(std::numeric_limits<uint32_t>::max());

  opened_ = true;
  CLOG_DEBUG("Opened delete store.");
  return 0;
}

int DeleteStore::flush() {
  CHECK_STATUS(opened_, true);

  return snapshot_->flush();
}

int DeleteStore::close() {
  CHECK_STATUS(opened_, true);

  delta_store_.unmount();
  bitmap_.clear();

  int ret = snapshot_->close();
  if (ret != 0) {
    LOG_WARN("Close snapshot failed.");
  }

  opened_ = false;
  CLOG_DEBUG("Closed delete store.");
  return ret;
}

int DeleteStore::insert(idx_t doc_id) {
  CHECK_STATUS(opened_, true);
  bitmap_.set(doc_id);

  return delta_store_.append(doc_id);
}

bool DeleteStore::has(idx_t doc_id) const {
  return bitmap_.test(doc_id);
}

}  // end namespace index
}  // namespace be
}  // end namespace proxima
