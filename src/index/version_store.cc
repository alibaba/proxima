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
 *   \brief    Implementation of version store
 */

#include "version_store.h"
#include "common/error_code.h"
#include "constants.h"

namespace proxima {
namespace be {
namespace index {

int VersionStore::mount(const IndexStoragePtr &stg) {
  if (!stg) {
    LOG_ERROR("Mount null storage");
    return ErrorCode_RuntimeError;
  }

  storage_ = stg;
  IndexBlockPtr summary_block = storage_->get(SUMMARY_BLOCK);
  int ret = 0;

  if (!summary_block) {
    ret = init_storage();
  } else {
    ret = load_storage();
  }

  return ret;
}

void VersionStore::unmount() {
  summary_block_.reset();
  version_block_.reset();
  segment_block_.reset();
  memset(&header_, 0, sizeof(header_));
}

int VersionStore::alloc_segment_meta(SegmentMeta *segment_meta) {
  std::lock_guard<std::mutex> lock(mutex_);

  if (header_.total_segment_count >= kMaxSegmentCount) {
    LOG_ERROR("Exceed max segment meta count limit. total_segment_count[%zu]",
              (size_t)header_.total_segment_count);
    return ErrorCode_ExceedLimit;
  }

  // check if exists unused segment meta
  // just return it to reuse
  if (header_.total_segment_count > 0) {
    SegmentID last_segment_id = header_.total_segment_count - 1;
    SegmentMeta last_segment_meta;
    int ret = this->get_segment_meta_impl(last_segment_id, &last_segment_meta);
    CHECK_RETURN(ret, 0);
    if (last_segment_meta.state == SegmentState::CREATED) {
      *segment_meta = last_segment_meta;
      return 0;
    }
  }

  SegmentMeta new_segment_meta;
  new_segment_meta.segment_id = header_.total_segment_count;

  // update segment meta
  int ret = this->update_segment_meta_impl(new_segment_meta.segment_id,
                                           new_segment_meta);
  CHECK_RETURN_WITH_LOG(ret, 0, "Write segment meta failed.");

  // update version header
  header_.total_segment_count++;
  ret = this->update_header_impl(header_);
  CHECK_RETURN_WITH_LOG(ret, 0, "Write header failed.");

  // copy out
  *segment_meta = new_segment_meta;
  return 0;
}

int VersionStore::get_segment_meta(SegmentID segment_id,
                                   SegmentMeta *segment_meta) const {
  if (segment_id >= header_.total_segment_count) {
    LOG_ERROR("Illegal segment id. segment_id[%zu]", (size_t)segment_id);
    return ErrorCode_ExceedLimit;
  }

  return this->get_segment_meta_impl(segment_id, segment_meta);
}

int VersionStore::update_segment_meta(const SegmentMeta &segment_meta) {
  std::lock_guard<std::mutex> lock(mutex_);

  SegmentID segment_id = segment_meta.segment_id;
  if (segment_id >= header_.total_segment_count) {
    LOG_ERROR("Illegal segment id. segment_id[%zu]", (size_t)segment_id);
    return ErrorCode_ExceedLimit;
  }

  return this->update_segment_meta_impl(segment_id, segment_meta);
}

int VersionStore::get_version_set(VersionSet *version_set) const {
  size_t offset = header_.current_version_offset;
  size_t read_len =
      version_block_->fetch(offset, (void *)version_set, sizeof(VersionSet));
  if (read_len != sizeof(VersionSet)) {
    LOG_ERROR("Read version block failed.");
    return ErrorCode_WriteData;
  }

  return 0;
}

int VersionStore::update_version_set(const VersionSet &version_set) {
  std::lock_guard<std::mutex> lock(mutex_);

  size_t offset = sizeof(VersionHeader);
  size_t write_len = 0U;

  // write version set
  write_len = version_block_->write(offset, &version_set, sizeof(VersionSet));
  if (write_len != sizeof(VersionSet)) {
    LOG_ERROR("Write version block failed. ");
    return ErrorCode_WriteData;
  }

  // update version header
  header_.total_version_count++;
  header_.current_version_offset = offset;

  return this->update_header_impl(header_);
}

int VersionStore::get_collection_summary(CollectionSummary *summary) const {
  size_t read_len =
      summary_block_->fetch(0U, (void *)summary, sizeof(CollectionSummary));
  if (read_len != sizeof(CollectionSummary)) {
    LOG_ERROR("Read summary block failed.");
    return ErrorCode_ReadData;
  }
  return 0;
}

int VersionStore::update_collection_summary(const CollectionSummary &summary) {
  std::lock_guard<std::mutex> lock(mutex_);

  size_t write_len =
      summary_block_->write(0U, &summary, sizeof(CollectionSummary));
  if (write_len != sizeof(CollectionSummary)) {
    LOG_ERROR("Write summary block failed.");
    return ErrorCode_WriteData;
  }
  return 0;
}

int VersionStore::init_storage() {
  int ret = 0;
  size_t alloc_len = 0U;

  alloc_len = sizeof(CollectionSummary);
  ret = storage_->append(SUMMARY_BLOCK, alloc_len);
  CHECK_RETURN_WITH_LOG(ret, 0, "Append summary block failed.");
  summary_block_ = storage_->get(SUMMARY_BLOCK);

  alloc_len = sizeof(VersionHeader) + sizeof(VersionSet);
  ret = storage_->append(VERSION_BLOCK, alloc_len);
  CHECK_RETURN_WITH_LOG(ret, 0, "Append version block failed.");
  version_block_ = storage_->get(VERSION_BLOCK);

  alloc_len = sizeof(SegmentMeta) * kMaxSegmentCount;
  ret = storage_->append(SEGMENT_BLOCK, alloc_len);
  CHECK_RETURN_WITH_LOG(ret, 0, "Append segment block failed.");
  segment_block_ = storage_->get(SEGMENT_BLOCK);

  // init summary block
  CollectionSummary summary;
  ret = update_collection_summary(summary);
  CHECK_RETURN(ret, 0);

  // init writing segment
  SegmentMeta segment_meta;
  ret = this->alloc_segment_meta(&segment_meta);
  CHECK_RETURN(ret, 0);

  segment_meta.state = SegmentState::WRITING;
  segment_meta.min_doc_id = 0U;
  ret = this->update_segment_meta(segment_meta);
  CHECK_RETURN(ret, 0);

  return 0;
}

int VersionStore::load_storage() {
  summary_block_ = storage_->get(SUMMARY_BLOCK);
  if (!summary_block_) {
    LOG_ERROR("Get summary block failed.");
    return ErrorCode_InvalidIndexDataFormat;
  }

  version_block_ = storage_->get(VERSION_BLOCK);
  if (!version_block_) {
    LOG_ERROR("Get version block failed.");
    return ErrorCode_InvalidIndexDataFormat;
  }

  segment_block_ = storage_->get(SEGMENT_BLOCK);
  if (!segment_block_) {
    LOG_ERROR("Get segment block failed.");
    return ErrorCode_InvalidIndexDataFormat;
  }

  size_t read_len =
      version_block_->fetch(0, (void *)&header_, sizeof(VersionHeader));
  if (read_len != sizeof(VersionHeader)) {
    LOG_ERROR("Read header block failed.");
    return ErrorCode_ReadData;
  }

  return 0;
}


}  // end namespace index
}  // namespace be
}  // end namespace proxima
