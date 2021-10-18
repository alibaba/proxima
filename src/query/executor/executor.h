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

#include "task.h"

namespace proxima {
namespace be {
namespace query {

//! Predefine class
class Executor;
//! Alias for Executor
using ExecutorPtr = std::shared_ptr<Executor>;

/*!
 * Task executor
 */
class Executor {
 public:
  //! Destructor
  virtual ~Executor() = default;

 public:
  //! Execute one task
  virtual int execute_task(const TaskPtr &task) = 0;

  //! Execute tasks
  virtual int execute_tasks(const TaskPtrList &tasks) = 0;
};


}  // namespace query
}  // namespace be
}  // namespace proxima
