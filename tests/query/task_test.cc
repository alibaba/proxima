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

#include <chrono>
#include <gtest/gtest.h>
#include "task-inl.h"

using namespace proxima::be::query;
using namespace proxima::be::query::test;

const std::string kName("task name");
const int kCode = 0;
const int kMillSeconds = 100;

TEST(TaskTest, TestDefaultContructor) {
  TaskImpl task(kName, kCode);
  ASSERT_EQ(task.name(), kName);
  ASSERT_EQ(task.status(), Task::Status::INITIALIZED);
  ASSERT_FALSE(task.running());
  ASSERT_FALSE(task.finished());
}

TEST(TaskTest, TestExitCode) {
  TaskImpl task(kName, kCode);
  task.status(Task::Status::SCHEDULED);
  ASSERT_EQ(task.run(), kCode);
  ASSERT_EQ(task.exit_code(), kCode);
  ASSERT_TRUE(task.finished());
  ASSERT_TRUE(task.wait_finish());
}

TEST(TaskTest, TestAsyncRun) {
  TaskPtr task = CreateTask(kName, kCode, kMillSeconds);

  ASSERT_EQ(task->status(), Task::Status::INITIALIZED);

  {
    auto begin = std::chrono::steady_clock::now();
    // Async run task
    std::thread runner(
        [](TaskPtr task) {
          task->status(Task::Status::SCHEDULED);
          task->run();
        },
        task);

    // Wait finish
    ASSERT_TRUE(task->wait_finish());
    auto end = std::chrono::steady_clock::now();
    auto comsuption =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);

    ASSERT_TRUE(comsuption.count() >= kMillSeconds);
    ASSERT_TRUE(task->finished());
    runner.join();
  }
}
