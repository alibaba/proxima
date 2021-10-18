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

#include "query/executor/bthread_queue.h"
#include <chrono>
#include <ratio>
#include <gtest/gtest.h>
#include "common/error_code.h"
#include "task-inl.h"

using namespace proxima::be::query;
using namespace proxima::be::query::test;

const std::string kName("task name");
const int kCode = 0;
const int kMillSeconds = 1000;

TEST(BThreadQueueTest, TestDefaultContructor) {
  BThreadQueue queue;
  ASSERT_EQ(queue.start(), 0);
  ASSERT_TRUE(queue.started());
  ASSERT_EQ(queue.stop(), 0);
  ASSERT_EQ(queue.join(), 0);
  ASSERT_FALSE(queue.started());
}

TEST(BThreadQueueTest, TestPutOperation) {
  BThreadQueue queue;
  ASSERT_EQ(queue.start(), 0);
  ASSERT_TRUE(queue.started());
  TaskPtr task = CreateTask(kName, kCode, kMillSeconds);
  ASSERT_EQ(queue.put(task), 0);
  ASSERT_TRUE(task->wait_finish());
  ASSERT_EQ(queue.stop(), 0);
  ASSERT_EQ(queue.join(), 0);
  ASSERT_FALSE(queue.started());
}

TEST(BThreadQueueTest, TestFalseOperation) {
  BThreadQueue queue;
  ASSERT_EQ(queue.start(), 0);
  ASSERT_TRUE(queue.started());
  TaskPtr task = CreateTask(kName, kCode, kMillSeconds);
  ASSERT_TRUE(queue.start() != 0);
  ASSERT_TRUE(queue.join() != 0);

  ASSERT_EQ(queue.put(task), 0);
  // Task should sleep for 100ms,
  ASSERT_TRUE(static_cast<int>(task->status()) >=
              static_cast<int>(Task::Status::SCHEDULED));

  ASSERT_EQ(queue.stop(), 0);
  ASSERT_TRUE(queue.start() != 0);

  ASSERT_EQ(queue.join(), 0);

  // Can't enqueue task
  ASSERT_EQ(queue.put(task), PROXIMA_BE_ERROR_CODE(RuntimeError));

  ASSERT_FALSE(queue.started());
}
