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
 *   \date     Oct 2020
 *   \brief
 */

#pragma once

#include "task.h"

namespace proxima {
namespace be {
namespace query {

//! Predefine class
class TaskQueue;
//! Alias for TaskQueue
using TaskQueuePtr = std::shared_ptr<TaskQueue>;

/*!
 * TaskQueue Interface,
 */
class TaskQueue {
 public:
  /*!
   * Queue status flag
   */
  enum Status {
    INITIALIZED,
    STARTED,
    STOPPED,
    JOINED,
  };

 public:
  //! Destructor
  virtual ~TaskQueue() = default;

 public:
  //! Start task queue, return value 0 for success, otherwise failed
  virtual int start() = 0;

  //! Stop task queue, return value 0 for success, otherwise failed
  virtual int stop() = 0;

  //! Join task queue, return value 0 for success, otherwise failed
  virtual int join() = 0;

  //! Put the task in queue
  //! @return,     0: succeed, mark status of task as Scheduled
  //!          other: failed
  virtual int put(const TaskPtr &task) = 0;

  //! Retrieve start flags
  virtual bool started() const = 0;
};


}  // namespace query
}  // namespace be
}  // namespace proxima
