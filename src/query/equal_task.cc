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
 *
 *   \author   guonix
 *   \date     Nov 2020
 *   \brief
 */

#include "equal_task.h"
#include "common/error_code.h"

namespace proxima {
namespace be {
namespace query {

EqualTask::EqualTask(index::SegmentPtr segment, QueryKeyContext *context)
    : BthreadTask("EqualTask"),
      hit_count_(0),
      segment_(std::move(segment)),
      context_(context) {}

EqualTask::~EqualTask() = default;

uint32_t EqualTask::hit() const {
  return hit_count_;
}

const index::QueryResult &EqualTask::forward() const {
  return forward_;
}

int EqualTask::do_run() {
  if (!segment_ || !context_) {
    return PROXIMA_BE_ERROR_CODE(InvalidSegment);
  }
  // Invoke kv_search with segment
  int code = segment_->kv_search(context_->primary_key(), &forward_);
  if (code == 0 && forward_.primary_key != index::INVALID_KEY) {
    hit_count_ = 1;
  }

  return code;
}

}  // namespace query
}  // namespace be
}  // namespace proxima
