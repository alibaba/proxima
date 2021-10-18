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
 *   \brief    Provides management of collection version meta info
 */

#pragma once

#include <memory>
#include "common/macro_define.h"
#include "common/types.h"
#include "snapshot.h"
#include "typedef.h"
#include "version_store.h"

namespace proxima {
namespace be {
namespace index {

class VersionManager;
using VersionManagerPtr = std::shared_ptr<VersionManager>;

/*
 * Version edit of one segment change
 */
struct VersionEdit {
  std::vector<SegmentID> add_segments;
  std::vector<SegmentID> delete_segments;
};

/*
 * VersionManager is reponsible for recording the stats the segments of
 * collection. The meta info of collection will snapshot to persist
 * storage at the same time.
 */
class VersionManager {
 public:
  PROXIMA_DISALLOW_COPY_AND_ASSIGN(VersionManager);

  //! Constructor
  VersionManager(const std::string &coll_name, const std::string &coll_path)
      : collection_name_(coll_name), collection_path_(coll_path) {}

  //! Destructor
  ~VersionManager();

  //! Create an instance
  static VersionManagerPtr Create(const std::string &collection_name,
                                  const std::string &collection_path);

  //! Create an instance and open
  static int CreateAndOpen(const std::string &collection_name,
                           const std::string &collection_path,
                           const ReadOptions &options,
                           VersionManagerPtr *version_manager);

 public:
  //! Open persist storage and open
  int open(const ReadOptions &options);

  //! Flush memory to persist storage
  int flush();

  //! Close persist storage and cleanup
  int close();

 public:
  //! Apply a version edit
  int apply(const VersionEdit &edit);

  //! Return current version segment metas
  const std::vector<SegmentMeta> &current_version() const {
    return current_version_;
  }

  //! Allocate new segment meta
  int alloc_segment_meta(SegmentMeta *segment_meta) {
    CHECK_STATUS(opened_, true);
    return version_store_.alloc_segment_meta(segment_meta);
  }

  //! Get segment meta
  int get_segment_meta(SegmentID segment_id, SegmentMeta *segment_meta) const {
    CHECK_STATUS(opened_, true);
    return version_store_.get_segment_meta(segment_id, segment_meta);
  }

  //! Get all segment metas
  int get_segment_metas(SegmentState state,
                        std::vector<SegmentMeta> *segment_metas) const {
    CHECK_STATUS(opened_, true);
    for (uint32_t i = 0; i < version_store_.total_segment_count(); i++) {
      SegmentMeta segment_meta;
      int ret = version_store_.get_segment_meta(i, &segment_meta);
      CHECK_RETURN(ret, 0);
      if (segment_meta.state == state) {
        segment_metas->emplace_back(segment_meta);
      }
    }
    return 0;
  }

  //! Update segment meta
  int update_segment_meta(const SegmentMeta &segment_meta) {
    CHECK_STATUS(opened_, true);
    return version_store_.update_segment_meta(segment_meta);
  }

  //! Get summary
  int get_collection_summary(CollectionSummary *summary) const {
    CHECK_STATUS(opened_, true);
    return version_store_.get_collection_summary(summary);
  }

  //! Update summary
  int update_collection_summary(const CollectionSummary &summary) {
    CHECK_STATUS(opened_, true);
    return version_store_.update_collection_summary(summary);
  }

 public:
  //! Return belonged collection name
  const std::string &collection_name() const {
    return collection_name_;
  }

  //! Return index file path
  const std::string &file_path() const {
    return snapshot_->file_path();
  }

  //! Return total segment count
  uint32_t total_segment_count() const {
    return version_store_.total_segment_count();
  }

 private:
  std::string collection_name_{};
  std::string collection_path_{};

  SnapshotPtr snapshot_{};
  VersionStore version_store_{};
  std::vector<SegmentMeta> current_version_{};
  std::mutex mutex_{};

  bool opened_{false};
};


}  // end namespace index
}  // namespace be
}  // end namespace proxima
