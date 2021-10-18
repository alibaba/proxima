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
 */

#include "index/id_map.h"
#include <gtest/gtest.h>
using namespace proxima::be;
using namespace proxima::be::index;


class IDMapTest : public testing::Test {
 protected:
  void SetUp() {
    char cmd_buf[100];
    snprintf(cmd_buf, 100, "rm -rf ./data.id");
    system(cmd_buf);
  }

  void TearDown() {}
};

TEST_F(IDMapTest, TestGeneral) {
  auto id_map = IDMap::Create("collection_test", "./");
  ASSERT_NE(id_map, nullptr);

  ReadOptions read_options;
  read_options.use_mmap = true;
  read_options.create_new = true;
  int ret = id_map->open(read_options);
  ASSERT_EQ(ret, 0);

  for (size_t i = 0; i < 20000; i++) {
    ret = id_map->insert(i, i);
    ASSERT_EQ(ret, 0);
  }

  for (size_t i = 0; i < 20000; i++) {
    ASSERT_EQ(id_map->has(i), true);
    idx_t doc_id = id_map->get_mapping_id(i);
    ASSERT_EQ(doc_id, i);
  }

  ret = id_map->close();
  ASSERT_EQ(ret, 0);

  ret = id_map->open(read_options);
  ASSERT_EQ(ret, 0);

  for (size_t i = 0; i < 20000; i++) {
    ASSERT_EQ(id_map->has(i), true);
    idx_t doc_id = id_map->get_mapping_id(i);
    ASSERT_EQ(doc_id, i);
  }

  for (size_t i = 0; i < 10000; i++) {
    id_map->remove(i);
  }

  for (size_t i = 0; i < 10000; i++) {
    ASSERT_EQ(id_map->has(i), false);
  }

  ret = id_map->close();
  ASSERT_EQ(ret, 0);

  ret = id_map->open(read_options);
  ASSERT_EQ(ret, 0);

  for (size_t i = 0; i < 10000; i++) {
    ASSERT_EQ(id_map->has(i), false);
  }

  for (size_t i = 10000; i < 20000; i++) {
    ASSERT_EQ(id_map->has(i), true);
    idx_t doc_id = id_map->get_mapping_id(i);
    ASSERT_EQ(doc_id, i);
  }
}
