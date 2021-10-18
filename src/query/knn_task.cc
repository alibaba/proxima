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

#include "knn_task.h"
#include "common/error_code.h"

namespace proxima {
namespace be {
namespace query {

KNNTask::KNNTask(index::SegmentPtr segment, KNNQueryContext *context)
    : BthreadTask("KNNTask"), segment_(std::move(segment)), context_(context) {}

KNNTask::KNNTask(const std::string &name_val, index::SegmentPtr segment,
                 KNNQueryContext *context)
    : BthreadTask(name_val), segment_(std::move(segment)), context_(context) {}

KNNTask::~KNNTask() = default;

const std::vector<index::QueryResultList> &KNNTask::result() const {
  return result_;
}

int KNNTask::do_run() {
  if (!segment_ || !context_) {
    return PROXIMA_BE_ERROR_CODE(InvalidSegment);
  }

  LOG_DEBUG("KNNTask start to run, query_id[%zu], segment_id[%zu]",
            (size_t)context_->query_params().query_id,
            (size_t)segment_->segment_id());
  return segment_->knn_search(context_->column(), context_->features(),
                              context_->query_params(), context_->batch_count(),
                              &result_);
}

}  // namespace query
}  // namespace be
}  // namespace proxima
