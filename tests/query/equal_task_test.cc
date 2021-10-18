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
 *   \date     Dec 2020
 *   \brief
 */

#include "query/equal_task.h"
#include <gtest/gtest.h>
#include "index/mock_segment.h"  // for MockSegment
#include "mock_query_context.h"  // for Mock*Context

TEST(EqualTaskTest, TestTaskRun) {
  {  // Test Invalid arguments
    MockEqualQueryContext context;
    EqualTask task(nullptr, &context);
    ASSERT_TRUE(task.run() != 0);
    EqualTask task1(nullptr, nullptr);
    ASSERT_TRUE(task1.run() != 0);
    EqualTask task2(std::make_shared<MockSegment>(), nullptr);
    ASSERT_TRUE(task2.run() != 0);
  }

  {
    MockEqualQueryContext context;
    EXPECT_CALL(context, primary_key()).WillRepeatedly(Return(1u));

    auto segment = std::make_shared<MockSegment>();

    {  // Failed
      EXPECT_CALL(*segment, kv_search(_, _))
          .WillOnce(Return(1))
          .RetiresOnSaturation();

      EqualTask task(segment, &context);
      task.status(Task::Status::SCHEDULED);
      EXPECT_EQ(task.run(), 1);
      EXPECT_EQ(task.exit_code(), 1);
    }

    {
      EXPECT_CALL(*segment, kv_search(_, _))
          .WillOnce(Return(0))
          .RetiresOnSaturation();

      EqualTask task(segment, &context);
      task.status(Task::Status::SCHEDULED);
      EXPECT_EQ(task.run(), 0);
      EXPECT_EQ(task.exit_code(), 0);
      EXPECT_TRUE(task.wait_finish());
      EXPECT_TRUE(task.finished());
      // EXPECT_TRUE(task.forward());
    }

    {
      EXPECT_CALL(*segment, kv_search(_, _))
          .WillOnce(Invoke([](uint64_t primary_key, QueryResult *result) {
            EXPECT_EQ(primary_key, 1);
            result->primary_key = 1;
            result->revision = 2;
            return 0;
          }))
          .RetiresOnSaturation();

      EqualTask task(segment, &context);
      task.status(Task::Status::SCHEDULED);
      EXPECT_EQ(task.run(), 0);
      EXPECT_EQ(task.exit_code(), 0);
      EXPECT_TRUE(task.wait_finish());
      EXPECT_TRUE(task.finished());
      EXPECT_EQ(task.hit(), 1);

      EXPECT_EQ(task.forward().primary_key, 1);
      EXPECT_EQ(task.forward().revision, 2);
    }
  }
}
