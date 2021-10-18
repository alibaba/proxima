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
 *   \author   hongqing.hu
 *   \date     Dec 2020
 *   \brief
 */


#include <gtest/gtest.h>
#define private public
#define protected public
#include "agent/collection_counter.h"
#include "agent/index_agent.h"

#undef protected
#undef private

using namespace proxima::be;
using namespace proxima::be::agent;

class CollectionCounterTest : public testing::Test {
 protected:
  // Sets up the test fixture.
  void SetUp() override {}

  // Tears down the test fixture.
  void TearDown() override {}
};

TEST_F(CollectionCounterTest, TestGeneral) {
  CollectionCounterMap map;
  std::string name("test1");
  map.add_counter(name);

  auto counter = map.get_counter(name);
  ASSERT_TRUE(counter != nullptr);

  uint32_t count = 100;
  counter->add_active_count(count);

  counter->sub_active_count(count - 1);

  counter->dec_active_count();

  ASSERT_EQ(counter->active_count(), (uint32_t)0);

  map.remove_counter(name);

  counter = map.get_counter(name);
  ASSERT_TRUE(counter == nullptr);
}
