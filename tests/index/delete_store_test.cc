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

#include "index/delete_store.h"
#include <gtest/gtest.h>

using namespace proxima::be;
using namespace proxima::be::index;


class DeleteStoreTest : public testing::Test {
 protected:
  void SetUp() {
    char cmd_buf[100];
    snprintf(cmd_buf, 100, "rm -rf ./data.del");
    system(cmd_buf);
  }

  void TearDown() {}
};

TEST_F(DeleteStoreTest, TestGeneral) {
  DeleteStorePtr delete_store = DeleteStore::Create("collection_test", "./");
  ASSERT_NE(delete_store, nullptr);

  ReadOptions read_options;
  read_options.use_mmap = true;
  read_options.create_new = true;

  int ret = delete_store->open(read_options);
  ASSERT_EQ(ret, 0);

  for (size_t i = 0; i < 10000; i++) {
    ASSERT_EQ(delete_store->insert(i), 0);
  }

  for (size_t i = 0; i < 10000; i++) {
    ASSERT_EQ(delete_store->has(i), true);
  }
  ASSERT_EQ(delete_store->close(), 0);

  ret = delete_store->open(read_options);
  ASSERT_EQ(ret, 0);

  for (size_t i = 0; i < 10000; i++) {
    ASSERT_EQ(delete_store->has(i), true);
  }
}
