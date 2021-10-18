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
 *   \date     Jan 2021
 *   \brief
 */

#pragma once

#include <bthread/bthread.h>
#include "task.h"

namespace proxima {
namespace be {
namespace query {

/*!
 * Bthread Task interface
 */
class BthreadTask : public Task {
 public:
  //! Constructor
  explicit BthreadTask(std::string name);

  //! Destructor
  ~BthreadTask() override;

 public:
  //! Retrieve task name
  const std::string &name() const override;

  int exit_code() const override;

  //! Run task, 0 for success otherwise failed
  int run() override;

  //! Retrieve the status of task, readonly
  Status status() const override;

  //! Update status
  void status(Status status) override;

  //! true for task in running stage, otherwise return false
  bool running() const override;

  //! true for task has been finished, otherwise return false
  bool finished() const override;

  //! Executes the task object exactly once, even if called concurrently, from
  // several threads. return immediately if invoke run method after run_once.
  int run_once() override;

  //! Wait until task has been finished， return value same with finished()
  bool wait_finish() override;

 private:
  //! Run interface, all derived class should implement this function
  virtual int do_run() = 0;

 private:
  //! task name
  std::string name_{};

  //! task status
  std::atomic<Status> status_{Status::INITIALIZED};

  //! Return code of run interface
  int exit_code_{0};

  //! Status mutex
  bthread_mutex_t mutex_;

  //! Finished event condition
  bthread_cond_t cond_;
};


}  // namespace query
}  // namespace be
}  // namespace proxima
