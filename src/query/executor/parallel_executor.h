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

#include "executor.h"
#include "scheduler.h"

namespace proxima {
namespace be {
namespace query {

/*!
 * Parallel Executor
 */
class ParallelExecutor : public Executor {
 public:
  //! Constructor
  explicit ParallelExecutor(SchedulerPtr scheduler);

  //! Destructor
  ~ParallelExecutor() override;

 public:
  //! Execute one task
  int execute_task(const TaskPtr &task) override;

  //! Execute tasks
  int execute_tasks(const TaskPtrList &tasks) override;

 private:
  //! Wait all task finish and collect execution status,
  //! return value 0 for success, otherwise failed
  int wait_finish(const TaskPtrList &tasks);

 private:
  //! Scheduler handle
  SchedulerPtr scheduler_{nullptr};
};


}  // namespace query
}  // namespace be
}  // namespace proxima
