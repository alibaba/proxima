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

#include "query/knn_task.h"
#include <gtest/gtest.h>
#include "index/mock_segment.h"  // for MockSegment
#include "mock_query_context.h"  // for Mock*Context

TEST(KNNTaskTest, TestTaskRun) {
  {  // Test Invalid arguments
    MockKNNQueryContext context;
    KNNTask task(nullptr, &context);
    ASSERT_TRUE(task.run() != 0);
    KNNTask task1(nullptr, nullptr);
    ASSERT_TRUE(task1.run() != 0);
    KNNTask task2(std::make_shared<MockSegment>(), nullptr);
    ASSERT_TRUE(task2.run() != 0);
  }

  {
    std::string column{"column"};
    std::string features{"features"};
    QueryParams param;

    MockKNNQueryContext context;
    EXPECT_CALL(context, column()).WillRepeatedly(ReturnRef(column));
    EXPECT_CALL(context, features()).WillRepeatedly(ReturnRef(features));
    EXPECT_CALL(context, query_params()).WillRepeatedly(ReturnRef(param));
    EXPECT_CALL(context, batch_count()).WillRepeatedly(Return(1));

    auto segment = std::make_shared<MockSegment>();
    // Specific batch knn_search
    EXPECT_CALL(*segment, knn_search(_, _, _, _, _))
        .Times(1)
        .WillOnce(Return(1))
        .RetiresOnSaturation();

    {  // Failed
      KNNTask task(segment, &context);
      task.status(Task::Status::SCHEDULED);
      EXPECT_EQ(task.run(), 1);
      EXPECT_EQ(task.exit_code(), 1);
      EXPECT_TRUE(task.result().empty());
    }

    EXPECT_CALL(*segment, knn_search(_, _, _, _, _))
        .Times(1)
        .WillOnce(Return(0))
        .RetiresOnSaturation();

    {
      KNNTask task(segment, &context);
      task.status(Task::Status::SCHEDULED);
      EXPECT_EQ(task.run(), 0);
      EXPECT_EQ(task.exit_code(), 0);
      EXPECT_TRUE(task.wait_finish());
      EXPECT_TRUE(task.finished());
      EXPECT_TRUE(task.result().empty());
    }
  }
}
