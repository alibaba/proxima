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
class EqualTask;

//! Alias for EqualTask
using EqualTaskPtr = std::shared_ptr<EqualTask>;
using EqualTaskPtrList = std::list<EqualTaskPtr>;


/*!
 * EqualTask
 */
class EqualTask : public BthreadTask {
 public:
  //! Constructor
  EqualTask(index::SegmentPtr segment, QueryKeyContext *context);

  //! Destructor
  ~EqualTask() override;

 public:
  //! Retrieve hit count
  uint32_t hit() const;

  //! Get the forward
  const index::QueryResult &forward() const;

 private:
  // Do search
  int do_run() override;

 private:
  //! Hit count
  uint32_t hit_count_{0};

  //! Segment handler
  index::SegmentPtr segment_{nullptr};

  //! Query Param
  QueryKeyContext *context_{nullptr};

  //! Forward
  index::QueryResult forward_{};
};


}  // namespace query
}  // namespace be
}  // namespace proxima
