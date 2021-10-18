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

#include "index/column/forward_indexer.h"
#include <memory>
#include <unordered_map>
#include <gtest/gtest.h>
#include "index/file_helper.h"
using namespace proxima::be;
using namespace proxima::be::index;


class ForwardIndexerTest : public testing::Test {
 protected:
  void SetUp() {
    FileHelper::RemoveFile("./data.fwd.0");
  }

  void TearDown() {}
};

TEST_F(ForwardIndexerTest, TestGeneral) {
  auto forward_indexer = ForwardIndexer::Create("test_collection", "./", 0);
  ASSERT_NE(forward_indexer, nullptr);

  ReadOptions read_options;
  read_options.use_mmap = true;
  read_options.create_new = true;

  forward_indexer->set_start_doc_id(0U);
  int ret = forward_indexer->open(read_options);
  ASSERT_EQ(ret, 0);

  // insert 1000 records
  for (size_t i = 0; i < 1000; i++) {
    ForwardData forward;
    forward.header.primary_key = i;
    forward.header.lsn = i;
    forward.header.revision = i;
    forward.data = "hello";

    idx_t doc_id = 0U;
    ret = forward_indexer->insert(forward, &doc_id);
    ASSERT_EQ(ret, 0);
    ASSERT_EQ(doc_id, i);
  }

  // seek 1000 records
  for (size_t i = 0; i < 1000; i++) {
    ForwardData forward;
    ret = forward_indexer->seek(i, &forward);
    ASSERT_EQ(ret, 0);
    ASSERT_EQ(forward.header.primary_key, i);
    ASSERT_EQ(forward.header.lsn, i);
    ASSERT_EQ(forward.header.revision, i);
    ASSERT_EQ(forward.data, "hello");
  }

  ret = forward_indexer->flush();
  ASSERT_EQ(ret, 0);

  ret = forward_indexer->close();
  ASSERT_EQ(ret, 0);

  // test reopen
  read_options.create_new = false;
  ret = forward_indexer->open(read_options);
  ASSERT_EQ(ret, 0);

  // test seek 1000 records again
  for (size_t i = 0; i < 1000; i++) {
    ForwardData forward;
    ret = forward_indexer->seek(i, &forward);
    ASSERT_EQ(ret, 0);
    ASSERT_EQ(forward.header.primary_key, i);
    ASSERT_EQ(forward.header.lsn, i);
    ASSERT_EQ(forward.header.revision, i);
    ASSERT_EQ(forward.data, "hello");
  }

  // Test remove
  for (size_t i = 0; i < 1000; i++) {
    ret = forward_indexer->remove(i);
    ASSERT_EQ(ret, 0);

    ForwardData forward;
    ret = forward_indexer->seek(i, &forward);
    ASSERT_NE(ret, 0);
  }
}

std::mutex g_mutex;
std::unordered_map<uint64_t, uint64_t> g_key_id_map;

void do_insert_forward(ForwardIndexer *forward_indexer, size_t number) {
  ForwardData forward;
  forward.header.primary_key = number;
  forward.header.lsn = number;
  forward.header.revision = number;
  forward.data = "hello";

  idx_t doc_id = 0U;
  int ret = forward_indexer->insert(forward, &doc_id);
  ASSERT_EQ(ret, 0);

  std::lock_guard<std::mutex> lock(g_mutex);
  g_key_id_map[number] = doc_id;
}

void do_seek_forward(ForwardIndexer *forward_indexer, size_t number) {
  std::lock_guard<std::mutex> lock(g_mutex);
  ForwardData forward;
  int ret = forward_indexer->seek(g_key_id_map[number], &forward);
  ASSERT_EQ(ret, 0);
  ASSERT_EQ(forward.header.primary_key, number);
  ASSERT_EQ(forward.header.lsn, number);
  ASSERT_EQ(forward.header.revision, number);
  ASSERT_EQ(forward.data, "hello");
}

void do_hybrid_operations(ForwardIndexer *forward_indexer, size_t number) {
  do_insert_forward(forward_indexer, number);
  do_seek_forward(forward_indexer, number);
}

TEST_F(ForwardIndexerTest, TestMultiThread) {
  auto forward_indexer = ForwardIndexer::Create("test_collection", "./", 0);
  ASSERT_TRUE(forward_indexer != nullptr);

  ReadOptions read_options;
  read_options.use_mmap = true;
  read_options.create_new = true;

  forward_indexer->set_start_doc_id(0U);
  int ret = forward_indexer->open(read_options);
  ASSERT_EQ(ret, 0);

  ailego::ThreadPool pool(3);
  for (size_t i = 0; i < 2000; i++) {
    pool.execute(&do_hybrid_operations, forward_indexer.get(), i);
  }
  pool.wait_finish();
}
