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

#pragma once

#include "executor/bthread_task.h"
#include "context.h"

namespace proxima {
namespace be {
namespace query {

//! Predefine class
class KNNTask;

// Alias knn task pointer
using KNNTaskPtr = std::shared_ptr<KNNTask>;
using KNNTaskPtrList = std::list<KNNTaskPtr>;


/*!
 * KNNTask
 */
class KNNTask : public BthreadTask {
 public:
  //! Constructor
  KNNTask(index::SegmentPtr segment, KNNQueryContext *context);

  //! Constructor
  KNNTask(const std::string &name, index::SegmentPtr segment,
          KNNQueryContext *context);

  //! Destructor
  ~KNNTask() override;

 public:
  //! Retrieve result of knn_search
  const std::vector<index::QueryResultList> &result() const;

 private:
  //! Run search task
  int do_run() override;

 private:
  //! Segment handle
  index::SegmentPtr segment_{nullptr};

  //! KnnQueryContext handler
  KNNQueryContext *context_{nullptr};

  //! Query Result, corresponding to the field of proto::QueryResponse
  std::vector<index::QueryResultList> result_{};
};


}  // namespace query
}  // namespace be
}  // namespace proxima
