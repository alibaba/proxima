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

#define private public
#define protected public
#include "index/lsn_store.h"
#undef private
#undef protected

#include <ailego/parallel/thread_pool.h>
#include <ailego/utility/time_helper.h>
#include <gtest/gtest.h>

using namespace proxima::be;
using namespace proxima::be::index;


class LsnStoreTest : public testing::Test {
 protected:
  void SetUp() {
    char cmd_buf[100];
    snprintf(cmd_buf, 100, "rm -rf ./data.lsn");
    system(cmd_buf);
  }

  void TearDown() {}
};

TEST_F(LsnStoreTest, TestGeneral) {
  LsnStorePtr lsn_store = LsnStore::Create("teachers", "./");
  ASSERT_TRUE(lsn_store != nullptr);

  ReadOptions read_options;
  read_options.use_mmap = true;
  read_options.create_new = true;
  int ret = lsn_store->open(read_options);
  ASSERT_EQ(ret, 0);

  for (size_t i = 0; i < 40000; i++) {
    std::string lsn_context = "JDBC://hello";
    ret = lsn_store->append(i, lsn_context + std::to_string(i));
    ASSERT_EQ(ret, 0);
  }

  ASSERT_EQ(lsn_store->header_.tail_block_index, 1);
  ASSERT_GT(lsn_store->data_blocks_[0]->data_size(), 0);
  LOG_INFO("data_size: %lu", lsn_store->data_blocks_[0]->data_size());
  ASSERT_GT(lsn_store->data_blocks_[1]->data_size(), 0);
  ASSERT_EQ(lsn_store->data_blocks_[2]->data_size(), 0);

  for (size_t i = 30000; i < 70000; i++) {
    std::string lsn_context = "JDBC://hello";
    ret = lsn_store->append(i, lsn_context + std::to_string(i));
    ASSERT_EQ(ret, 0);
  }

  ASSERT_EQ(lsn_store->header_.tail_block_index, 0);
  ASSERT_GT(lsn_store->data_blocks_[1]->data_size(), 0);
  LOG_INFO("data_size: %lu", lsn_store->data_blocks_[0]->data_size());
  ASSERT_EQ(lsn_store->data_blocks_[2]->data_size(), 0);

  ret = lsn_store->shift();
  ASSERT_EQ(ret, 0);
  ASSERT_EQ(lsn_store->data_blocks_[2]->data_size(),
            lsn_store->data_blocks_[0]->data_size());

  for (size_t i = 70000; i < 71000; i++) {
    std::string lsn_context = "JDBC://hello";
    ret = lsn_store->append(i, lsn_context + std::to_string(i));
    ASSERT_EQ(ret, 0);

    ailego::ElapsedTime timer;
    uint64_t lsn;
    std::string lsn_context_val;
    ret = lsn_store->get_latest_lsn(&lsn, &lsn_context_val);
    ASSERT_EQ(ret, 0);
    ASSERT_EQ(lsn, i);
    ASSERT_EQ(lsn_context_val, lsn_context + std::to_string(i));
  }

  ret = lsn_store->close();
  ASSERT_EQ(ret, 0);

  read_options.use_mmap = true;
  read_options.create_new = false;
  ret = lsn_store->open(read_options);
  ASSERT_EQ(ret, 0);

  uint64_t lsn;
  std::string lsn_context_val;
  lsn_store->get_latest_lsn(&lsn, &lsn_context_val);
  ASSERT_EQ(lsn, 70999);
  ASSERT_EQ(lsn_context_val,
            std::string("JDBC://hello") + std::to_string(70999));
}

void do_insert(LsnStore *lsn_store, size_t number) {
  std::string lsn_context = "JDBC://hello";
  int ret = lsn_store->append(number, lsn_context + std::to_string(number));
  ASSERT_EQ(ret, 0);
}

TEST_F(LsnStoreTest, TestMultiThread) {
  LsnStorePtr lsn_store = LsnStore::Create("teachers", "./");
  ASSERT_TRUE(lsn_store != nullptr);

  ReadOptions read_options;
  read_options.use_mmap = true;
  read_options.create_new = true;
  int ret = lsn_store->open(read_options);
  ASSERT_EQ(ret, 0);

  ailego::ThreadPool pool(10, false);
  for (size_t i = 0; i < 10000; i++) {
    pool.execute(do_insert, lsn_store.get(), i);
  }
  pool.wait_finish();

  LOG_INFO("data_size0: %lu", lsn_store->data_blocks_[0]->data_size());
  LOG_INFO("data_size1: %lu", lsn_store->data_blocks_[1]->data_size());
  LOG_INFO("data_size2: %lu", lsn_store->data_blocks_[2]->data_size());

  uint64_t lsn;
  std::string lsn_context_val;
  ret = lsn_store->get_latest_lsn(&lsn, &lsn_context_val);
  ASSERT_EQ(ret, 0);
  ASSERT_GE(lsn, 9999);
  ASSERT_EQ(lsn_context_val,
            std::string("JDBC://hello") + std::to_string(9999));
}
