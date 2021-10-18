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
class Scheduler;
//! Alias SchedulerPtr
using SchedulerPtr = std::shared_ptr<Scheduler>;

/*!
 * Schedule task to execution queue
 */
class Scheduler {
 public:
  //! Static helper function, get default scheduler
  static SchedulerPtr Default();

  //! Default system concurrency
  static uint32_t HostConcurrency();

 public:
  //! Destructor
  virtual ~Scheduler() = default;

 public:
  //! Dispatch task to execution queue, return 0 for success, otherwise failed
  virtual int schedule(TaskPtr task) = 0;

  //! Retrieve concurrency of scheduler
  virtual uint32_t concurrency() const = 0;

  //! Set concurrency
  virtual uint32_t concurrency(uint32_t concurrency) = 0;
};


}  // namespace query
}  // namespace be
}  // namespace proxima
