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

#include "query/executor/scheduler.h"
#include <gtest/gtest.h>
#include "task-inl.h"

using namespace proxima::be::query;
using namespace proxima::be::query::test;

const std::string kName("task name");
const int kCode = 0;
const int kMillSeconds = 100;

TEST(SchedulerTest, TestScheduler) {
  SchedulerPtr scheduler = Scheduler::Default();
  TaskPtr task = CreateTask(kName, kCode, kMillSeconds);

  ASSERT_EQ(scheduler->concurrency(), 0);

  // Can't schedule an task before set concurrency
  ASSERT_TRUE(scheduler->schedule(task) != 0);

  uint32_t concurrency = Scheduler::HostConcurrency();
  ASSERT_EQ(scheduler->concurrency(concurrency), concurrency);

  ASSERT_EQ(scheduler->schedule(task), 0);

  ASSERT_TRUE(task->wait_finish());
}
