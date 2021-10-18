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
 *   \brief    Implementation of persist segment manager
 */

#include "persist_segment_manager.h"
#include "common/error_code.h"
#include "common/logger.h"
#include "typedef.h"

namespace proxima {
namespace be {
namespace index {

PersistSegmentManagerPtr PersistSegmentManager::Create(
    const std::string &collection_name, const std::string &collection_path) {
  PersistSegmentManagerPtr new_mgr =
      std::make_shared<PersistSegmentManager>(collection_name, collection_path);
  return new_mgr;
}

PersistSegmentManager::~PersistSegmentManager() {
  if (segments_.size() > 0) {
    unload_segments();
  }
}

void PersistSegmentManager::add_segment(PersistSegmentPtr persist_segment) {
  segments_.emplace(persist_segment->segment_id(), persist_segment);
}

const PersistSegmentPtr &PersistSegmentManager::get_segment(
    SegmentID segment_id) {
  return segments_.get(segment_id);
}

const PersistSegmentPtr &PersistSegmentManager::get_latest_segment() {
  SegmentID max_segment_id = 0U;
  for (auto &it : segments_) {
    if (it.first > max_segment_id) {
      max_segment_id = it.first;
    }
  }
  return segments_.get(max_segment_id);
}

int PersistSegmentManager::unload_segments() {
  for (auto iter = segments_.begin(); iter != segments_.end(); iter++) {
    iter->second->unload();
  }
  segments_.clear();
  return 0;
}


}  // end namespace index
}  // namespace be
}  // end namespace proxima
