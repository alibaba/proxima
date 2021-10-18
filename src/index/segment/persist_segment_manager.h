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
 *   \brief    Provides ability for management of persist segments
 */

#pragma once

#include "common/macro_define.h"
#include "persist_segment.h"
#include "../concurrent_hash_map.h"

namespace proxima {
namespace be {
namespace index {

class PersistSegmentManager;
using PersistSegmentManagerPtr = std::shared_ptr<PersistSegmentManager>;

/*
 * PersistSegmentManager is mainly for managing persist segments.
 */
class PersistSegmentManager {
 public:
  //! Constructor
  PersistSegmentManager(const std::string &coll_name,
                        const std::string &coll_path)
      : collection_name_(coll_name), collection_path_(coll_path) {}

  //! Destructor
  ~PersistSegmentManager();

  //! Create an instance
  static PersistSegmentManagerPtr Create(const std::string &collection_name,
                                         const std::string &collection_path);

 public:
  //! Add persist segment
  void add_segment(PersistSegmentPtr persist_segment);

  //! Get a specific segment
  const PersistSegmentPtr &get_segment(SegmentID segment_id);

  //! Get latest put-in segment
  const PersistSegmentPtr &get_latest_segment();

  //! Upload all segments
  int unload_segments();

  //! Return whether has some segment
  bool has_segment(SegmentID segment_id) {
    return segments_.has(segment_id);
  }

  //! Return loaded persist segment count
  size_t segment_count() {
    return segments_.size();
  }

 private:
  std::string collection_name_{};
  std::string collection_path_{};

  ConcurrentHashMap<SegmentID, PersistSegmentPtr> segments_{};
};


}  // end namespace index
}  // namespace be
}  // end namespace proxima
