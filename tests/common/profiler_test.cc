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

#include "common/profiler.h"
#include <gtest/gtest.h>

using namespace proxima::be;

TEST(ProfilerTest, TestDisabledProfiler) {
  Profiler profiler(false);

  EXPECT_FALSE(profiler.enabled());
  profiler.start();
  EXPECT_EQ(profiler.open_stage("abc"), 0);
  EXPECT_EQ(profiler.close_stage(), 0);
  EXPECT_EQ(profiler.add("abc", 10), 0);
  profiler.stop();
  EXPECT_EQ(profiler.as_json_string(), std::string("{}"));
}

TEST(ProfilerTest, TestEnabledProfiler) {
  Profiler profiler(true);

  EXPECT_TRUE(profiler.enabled());
  profiler.start();
  EXPECT_EQ(profiler.open_stage("abc"), 0);
  EXPECT_EQ(profiler.close_stage(), 0);
  EXPECT_EQ(profiler.add("abc", 10), 0);
  EXPECT_EQ(profiler.close_stage(), 0);
  EXPECT_TRUE(profiler.close_stage() != 0);
  EXPECT_TRUE(profiler.open_stage("def") != 0);
  profiler.start();
  EXPECT_EQ(profiler.open_stage("def"), 0);
  profiler.stop();

  std::string json_str = profiler.as_json_string();
  EXPECT_TRUE(json_str.size() != 0);
  ailego::JsonValue val = ailego::JsonValue();

  ailego::JsonValue tmp;
  EXPECT_TRUE(tmp.parse(json_str));
}