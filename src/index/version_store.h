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
 *   \brief    Format of collection version meta info
 */

#pragma once

#include <mutex>
#include <aitheta2/index_storage.h>
#include "common/macro_define.h"
#include "common/types.h"
#include "segment/segment.h"
#include "typedef.h"

namespace proxima {
namespace be {
namespace index {

/**
 *    |    SummaryBlock      |       VersionBlock         |   SegmentBlock   |
 *    |----------------------|----------------------------|------------------|
 *    |  CollectionSummary   | VersionHeader + VersionSet |  SegmentMeta...  |
 *    |----------------------|----------------------------|------------------|
 **/

struct CollectionSummary {
  uint64_t schema_revision{0U};
  uint64_t total_doc_count{0U};
  uint64_t delete_doc_count{0U};
  uint64_t reserved_[5];
};

static_assert(sizeof(CollectionSummary) % 64 == 0,
              "CollectionSummary must be aligned with 64 bytes");

/*
 * Version info header
 */
struct VersionHeader {
  uint64_t total_version_count{0U};
  uint64_t current_version_offset{0U};
  uint64_t total_segment_count{0U};
  uint64_t reserved_[5];
};

static_assert(sizeof(VersionHeader) % 64 == 0,
              "VersionHeader must be aligned with 64 bytes");

/*
 * A VersionSet includes serveral segment metas
 */
struct VersionSet {
  uint64_t segment_count{0};
  uint64_t reserved_[7];
  uint32_t segment_ids[1024];

  VersionSet() {
    memset(this->segment_ids, 0, sizeof(this->segment_ids));
  }
};

static_assert(sizeof(VersionSet) % 64 == 0,
              "VersionSet must be aligned with 64 bytes");

/*
 * VersionStore describes the struct of version info storage
 */
class VersionStore {
 public:
  PROXIMA_DISALLOW_COPY_AND_ASSIGN(VersionStore);

  //! Constructor
  VersionStore() = default;

  //! Destructor
  ~VersionStore() = default;

 public:
  //! Mount persist storage
  int mount(const IndexStoragePtr &storage);

  //! Unmount persist storage
  void unmount();

  //! Alloc new segment meta
  int alloc_segment_meta(SegmentMeta *segment_meta);

  //! Get segment meta
  int get_segment_meta(SegmentID segment_id, SegmentMeta *segment_meta) const;

  //! Update segment meta
  int update_segment_meta(const SegmentMeta &segment_meta);

  //! Get specified version set
  int get_version_set(VersionSet *version_set) const;

  //! Update new version set
  int update_version_set(const VersionSet &version_set);

  //! Get summary
  int get_collection_summary(CollectionSummary *summary) const;

  //! Update summary
  int update_collection_summary(const CollectionSummary &summary);

 public:
  //! Return total version count
  uint32_t total_version_count() const {
    return header_.total_version_count;
  }

  //! Return total segment count
  uint32_t total_segment_count() const {
    return header_.total_segment_count;
  }

 private:
  int init_storage();

  int load_storage();

 private:
  int get_segment_meta_impl(SegmentID segment_id,
                            SegmentMeta *segment_meta) const {
    size_t offset = segment_id * sizeof(SegmentMeta);
    size_t read_len = segment_block_->fetch(offset, (void *)segment_meta,
                                            sizeof(SegmentMeta));
    if (read_len != sizeof(SegmentMeta)) {
      return ErrorCode_ReadData;
    }
    return 0;
  }

  int update_segment_meta_impl(SegmentID segment_id,
                               const SegmentMeta &segment_meta) {
    size_t offset = segment_id * sizeof(SegmentMeta);
    size_t write_len =
        segment_block_->write(offset, &segment_meta, sizeof(SegmentMeta));
    if (write_len != sizeof(SegmentMeta)) {
      return ErrorCode_WriteData;
    }
    return 0;
  }

  int update_header_impl(const VersionHeader &header) {
    size_t write_len =
        version_block_->write(0U, &header, sizeof(VersionHeader));
    if (write_len != sizeof(VersionHeader)) {
      return ErrorCode_WriteData;
    }
    return 0;
  }

 private:
  constexpr static uint32_t kMaxVersionCount = 5;
  constexpr static uint32_t kMaxSegmentCount = 1024;

 private:
  IndexStoragePtr storage_{};
  IndexBlockPtr summary_block_{};
  IndexBlockPtr version_block_{};
  IndexBlockPtr segment_block_{};
  VersionHeader header_;

  std::mutex mutex_{};
};


}  // end namespace index
}  // namespace be
}  // end namespace proxima
