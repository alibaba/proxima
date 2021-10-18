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

 *   \author   Dianzhang.Chen
 *   \date     Apr 2021
 *   \brief
 */

#include "repository/lsn_context_format.h"
#include <gtest/gtest.h>

using namespace proxima::be::repository;

int main(int argc, char *argv[]) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

class LsnContextFormatTest : public ::testing::Test {
 protected:
  LsnContextFormatTest() {}

  ~LsnContextFormatTest() {}
  void SetUp() {}
  void TearDown() {}
};

TEST_F(LsnContextFormatTest, TestGeneral) {
  std::string lsn_str = "binlog;123456789;123;0";
  auto lsn_context = LsnContextFormat();
  lsn_context.parse_from_string(lsn_str);
  auto filename = lsn_context.file_name();
  auto position = lsn_context.position();
  auto seq_id = lsn_context.seq_id();
  auto mode = lsn_context.mode();
  EXPECT_EQ(filename, "binlog");
  EXPECT_EQ(position, 123456789);
  EXPECT_EQ(seq_id, 123);
  EXPECT_EQ(mode, ScanMode::FULL);

  lsn_str = "binlog2;87654321;123;1";
  lsn_context = LsnContextFormat();
  lsn_context.parse_from_string(lsn_str);
  filename = lsn_context.file_name();
  position = lsn_context.position();
  seq_id = lsn_context.seq_id();
  mode = lsn_context.mode();
  EXPECT_EQ(filename, "binlog2");
  EXPECT_EQ(position, 87654321);
  EXPECT_EQ(seq_id, 123);
  EXPECT_EQ(mode, ScanMode::INCREMENTAL);

  lsn_context =
      LsnContextFormat("binlog2", 87654321, 123, ScanMode::INCREMENTAL);
  auto lsn_str2 = lsn_context.convert_to_string();
  EXPECT_EQ(lsn_str, lsn_str2);
}
