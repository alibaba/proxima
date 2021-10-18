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

#include <bthread/execution_queue.h>
#include "task_queue.h"

namespace proxima {
namespace be {
namespace query {

/*!
 * Implementation of TaskQueue interface, which based on Execution
 * Queue in brpc
 */
class BThreadQueue : public TaskQueue {
 public:
  //! Disable copy and assigment operator on BThreadQueue instance
  // DISALLOW_COPY_AND_ASSIGN(BThreadQueue);

  //! Constructor
  BThreadQueue();

  //! Destructor
  ~BThreadQueue() override;

 public:
  //! Retrieve id of queue
  uint64_t id() const;

  //! Start task queue, return value 0 for success, otherwise failed
  int start() override;

  //! Stop task queue, return value 0 for success, otherwise failed
  int stop() override;

  //! Join task queue, return value 0 for success, otherwise failed
  int join() override;

  //! Put the task in queue
  //! @return,     0: succeed, mark status of task as Scheduled
  //!          other: failed
  int put(const TaskPtr &task) override;

  //! Retrieve start flags
  bool started() const override;

 private:
  //! Release all resourced
  void clear_context();

 private:
  //! Identifier of execution queue
  bthread::ExecutionQueueId<TaskPtr> queue_id_ = {0};

  //! Default options for execution queue
  bthread::ExecutionQueueOptions options_{};

  //! Queue status
  TaskQueue::Status status_{TaskQueue::Status::INITIALIZED};
};


}  // namespace query
}  // namespace be
}  // namespace proxima
