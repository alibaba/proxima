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
 *   \brief    Implementation of id map
 */

#include "id_map.h"
#include "common/error_code.h"
#include "constants.h"
#include "file_helper.h"
#include "typedef.h"

namespace proxima {
namespace be {
namespace index {

IDMapPtr IDMap::Create(const std::string &collection_name,
                       const std::string &collection_path) {
  return std::make_shared<IDMap>(collection_name, collection_path);
}

int IDMap::CreateAndOpen(const std::string &collection_name,
                         const std::string &collection_path,
                         const ReadOptions &read_options, IDMapPtr *id_map) {
  (*id_map) = Create(collection_name, collection_path);

  return (*id_map)->open(read_options);
}

IDMap::~IDMap() {
  if (opened_) {
    this->close();
  }
}

int IDMap::open(const ReadOptions &read_options) {
  CHECK_STATUS(opened_, false);

  int ret = Snapshot::CreateAndOpen(collection_path_, FileID::ID_FILE,
                                    read_options, &snapshot_);
  CHECK_RETURN_WITH_CLOG(ret, 0, "Create and open snapshot failed.");

  ret = key_map_.mount(snapshot_->data());
  CHECK_RETURN_WITH_CLOG(ret, 0, "Mount snapshot failed.");

  opened_ = true;
  CLOG_DEBUG("Opened id map.");
  return 0;
}

int IDMap::flush() {
  CHECK_STATUS(opened_, true);

  return snapshot_->flush();
}

int IDMap::close() {
  CHECK_STATUS(opened_, true);

  key_map_.unmount();

  // Do not break close logic
  int ret = snapshot_->close();
  if (ret != 0) {
    LOG_WARN("Close snapshot failed.");
  }

  opened_ = false;
  CLOG_DEBUG("Closed id map.");
  return ret;
}

int IDMap::insert(uint64_t key, idx_t doc_id) {
  CHECK_STATUS(opened_, true);

  int ret = key_map_.emplace(key, doc_id);
  CHECK_RETURN(ret, 0);
  return 0;
}

bool IDMap::has(uint64_t key) const {
  return key_map_.has(key);
}

idx_t IDMap::get_mapping_id(uint64_t key) const {
  idx_t doc_id = INVALID_DOC_ID;
  if (key_map_.has(key)) {
    key_map_.get(key, &doc_id);
  }
  return doc_id;
}

void IDMap::remove(uint64_t key) {
  if (key_map_.has(key)) {
    key_map_.erase(key);
  }
}

}  // end namespace index
}  // namespace be
}  // end namespace proxima
