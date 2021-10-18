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

#include <list>
#include <memory>
#include <mutex>
#include <vector>

namespace proxima {
namespace be {
namespace query {

//! Predefined class
class Task;
//! Alias for TaskPtr
using TaskPtr = std::shared_ptr<Task>;
using TaskPtrList = std::list<TaskPtr>;

/**
 * Task interface, most of API's should be readonly, except execute &
 * wait_finish the priority of updating status grant to TaskController, which
 * derived by Schedule]
 */
class Task {
 public:
  /*!
   * Status indicated the stage of task
   */
  enum Status {
    //! Initialized, wait to schedule.
    INITIALIZED,
    //! Has been scheduled, not running yet.
    SCHEDULED,
    //! Running, not finished yet.
    RUNNING,
    //! Finished,
    FINISHED,
  };

 public:
  //! Destructor
  virtual ~Task() = default;

 public:
  //! Retrieve task name
  virtual const std::string &name() const = 0;

  //! Retrieve exit code off task
  virtual int exit_code() const = 0;

  //! Run task, 0 for success otherwise failed
  virtual int run() = 0;

  //! Retrieve the status of task, readonly
  virtual Status status() const = 0;

  //! Update status
  virtual void status(Status status) = 0;

  //! true for task in running stage, otherwise return false
  virtual bool running() const = 0;

  //! true for task has been finished, otherwise return false
  virtual bool finished() const = 0;

  //! Executes the task object exactly once, even if called concurrently, from
  // several threads. return immediately if invoke run method after run_once.
  virtual int run_once() = 0;

  //! Wait until task has been finished， return value same with finished()
  virtual bool wait_finish() = 0;
};


}  // namespace query
}  // namespace be
}  // namespace proxima
