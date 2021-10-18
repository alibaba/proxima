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
 *   \brief    Implementation of version manager
 */

#include "version_manager.h"
#include "common/error_code.h"
#include "file_helper.h"

namespace proxima {
namespace be {
namespace index {

VersionManagerPtr VersionManager::Create(const std::string &collection_name,
                                         const std::string &collection_path) {
  return std::make_shared<VersionManager>(collection_name, collection_path);
}

int VersionManager::CreateAndOpen(const std::string &collection_name,
                                  const std::string &collection_path,
                                  const ReadOptions &read_options,
                                  VersionManagerPtr *version_manager) {
  (*version_manager) = Create(collection_name, collection_path);

  return (*version_manager)->open(read_options);
}

VersionManager::~VersionManager() {
  if (opened_) {
    this->close();
  }
}

int VersionManager::open(const ReadOptions &read_options) {
  CHECK_STATUS(opened_, false);

  int ret = Snapshot::CreateAndOpen(collection_path_, FileID::MANIFEST_FILE,
                                    read_options, &snapshot_);
  CHECK_RETURN_WITH_CLOG(ret, 0, "Create and open snapshot failed.");

  ret = version_store_.mount(snapshot_->data());
  CHECK_RETURN_WITH_CLOG(ret, 0, "Mount snapshot failed.");

  // Load current version, segments
  if (version_store_.total_version_count() > 0) {
    VersionSet version_set;
    ret = version_store_.get_version_set(&version_set);
    CHECK_RETURN(ret, 0);

    for (uint32_t i = 0; i < version_set.segment_count; i++) {
      SegmentID segment_id = version_set.segment_ids[i];
      SegmentMeta segment_meta;
      ret = version_store_.get_segment_meta(segment_id, &segment_meta);
      CHECK_RETURN(ret, 0);
      current_version_.emplace_back(segment_meta);
    }
  }

  opened_ = true;
  CLOG_DEBUG("Opened version manager.");
  return 0;
}

int VersionManager::flush() {
  CHECK_STATUS(opened_, true);

  return snapshot_->flush();
}

int VersionManager::close() {
  CHECK_STATUS(opened_, true);

  current_version_.clear();
  version_store_.unmount();

  int ret = snapshot_->close();
  if (ret != 0) {
    CLOG_WARN("Close snapshot failed.");
  }

  opened_ = false;
  CLOG_DEBUG("Closed version manager.");
  return ret;
}

int VersionManager::apply(const VersionEdit &edit) {
  CHECK_STATUS(opened_, true);

  std::lock_guard<std::mutex> lock(mutex_);
  int ret = 0;
  for (size_t i = 0; i < edit.add_segments.size(); i++) {
    SegmentID segment_id = edit.add_segments[i];
    SegmentMeta segment_meta;
    ret = version_store_.get_segment_meta(segment_id, &segment_meta);
    CHECK_RETURN(ret, 0);

    current_version_.emplace_back(segment_meta);
  }

  for (size_t i = 0; i < edit.delete_segments.size(); i++) {
    for (size_t j = 0; j < current_version_.size(); j++) {
      if (edit.delete_segments[i] == current_version_[j].segment_id) {
        current_version_.erase(current_version_.begin() + j);
      }
    }
  }

  VersionSet version_set;
  version_set.segment_count = current_version_.size();
  for (size_t i = 0; i < current_version_.size(); i++) {
    version_set.segment_ids[i] = current_version_[i].segment_id;
  }

  return version_store_.update_version_set(version_set);
}


}  // end namespace index
}  // namespace be
}  // namespace proxima
