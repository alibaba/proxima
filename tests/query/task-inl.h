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

#include <chrono>
#include <thread>
#include "query/executor/bthread_task.h"

namespace proxima {
namespace be {
namespace query {
namespace test {

class TaskImpl : public query::BthreadTask {
 public:
  TaskImpl(const std::string &name, int code, int millseconds = 0)
      : query::BthreadTask(name), ret_code_(code), sleep_(millseconds) {}

  virtual ~TaskImpl() {}

 private:
  virtual int do_run() {
    if (sleep_) {
      std::this_thread::sleep_for(std::chrono::milliseconds(sleep_));
    }
    return ret_code_;
  }

 private:
  int ret_code_;
  int sleep_;
};

static TaskPtr CreateTask(const std::string &name, int code,
                          int millseconds = 0) {
  return std::make_shared<TaskImpl>(name, code, millseconds);
}

}  // namespace test
}  // namespace query
}  // namespace be
}  // namespace proxima
