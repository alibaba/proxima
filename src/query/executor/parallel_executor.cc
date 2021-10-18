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

#include "parallel_executor.h"
#include "common/error_code.h"
#include "common/logger.h"

namespace proxima {
namespace be {
namespace query {

ParallelExecutor::ParallelExecutor(SchedulerPtr scheduler)
    : scheduler_(std::move(scheduler)) {}

ParallelExecutor::~ParallelExecutor() = default;

int ParallelExecutor::execute_task(const TaskPtr &task) {
  if (task) {
    task->status(Task::Status::SCHEDULED);
    return task->run_once();
  }
  return PROXIMA_BE_ERROR_CODE(InvalidArgument);
}

int ParallelExecutor::execute_tasks(const TaskPtrList &tasks) {
  int code = 0;
  if (tasks.empty()) {
    return code;
  }

  auto iter = tasks.begin();
  // Keep the head task, schedule others to other coroutines
  while (++iter != tasks.end()) {
    code = scheduler_->schedule(*iter);
    // Break loop, if schedule task failed，handle schedule error in
    // wait_finish function
    if (code != 0) {
      LOG_ERROR("Can't schedule task to run. code[%d]", code);
      break;
    }
  }

  // Execute first task in queue, if all others have been scheduled
  if (code == 0) {
    execute_task(*tasks.begin());
  }

  // Ignore return code of first task, wait finished event for all tasks.
  return wait_finish(tasks);
}

int ParallelExecutor::wait_finish(const TaskPtrList &tasks) {
  int error_code = 0;

  // Wait task finished, if has been scheduled before
  for (auto &task : tasks) {
    if (task->status() == Task::Status::INITIALIZED) {
      // Task has not been scheduled, set error code to Schedule Error,
      // and continue to reclaim the resources of others
      error_code = PROXIMA_BE_ERROR_CODE(ScheduleError);
      continue;
    }
    // Try run task in current coroutine if possible before waiting it has
    // been done with other coroutine.
    // Optimizing performance of query, The worst case of latency with
    // multi-segments query is equal to sequence-execution, which more
    // helpful when Proxima BE is under extreme high load.
    task->run_once();

    task->wait_finish();
    if (error_code == 0 && task->exit_code() != 0) {
      // Only save the error code of first task which failed
      error_code = task->exit_code();
    }
  }

  return error_code;
}

}  // namespace query
}  // namespace be
}  // namespace proxima
