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

#include "index/column/forward_reader.h"
#include <memory>
#include <unordered_map>
#include <gtest/gtest.h>
#include "index/column/forward_indexer.h"
#include "index/file_helper.h"

using namespace proxima::be;
using namespace proxima::be::index;

class ForwardReaderTest : public testing::Test {
 protected:
  void SetUp() {
    FileHelper::RemoveFile("./data.fwd.0");
    FileHelper::RemoveFile("data.seg.0");
  }

  void TearDown() {}
};

void do_search(ForwardReader *reader, size_t number) {
  ForwardData forward;
  int ret = reader->seek(number, &forward);

  ASSERT_EQ(ret, 0);
  ASSERT_EQ(forward.header.primary_key, number);
  ASSERT_EQ(forward.header.lsn, number);
  ASSERT_EQ(forward.header.revision, number);
  ASSERT_EQ(forward.data, "hello");
}

TEST_F(ForwardReaderTest, TestGeneral) {
  auto forward_indexer = ForwardIndexer::Create("test_collection", "./", 0);
  ASSERT_TRUE(forward_indexer != nullptr);

  ReadOptions read_options;
  read_options.use_mmap = true;
  read_options.create_new = true;

  forward_indexer->set_start_doc_id(0);
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

  auto dumper = aitheta2::IndexFactory::CreateDumper("FileDumper");
  ASSERT_NE(dumper, nullptr);

  ret = dumper->create("data.seg.0");
  ASSERT_EQ(ret, 0);

  IndexDumperPtr fwd_dumper =
      std::make_shared<IndexSegmentDumper>(dumper, FORWARD_DUMP_BLOCK);

  ret = forward_indexer->dump(fwd_dumper);
  ASSERT_EQ(ret, 0);

  fwd_dumper->close();
  dumper->close();
  ret = forward_indexer->close();
  ASSERT_EQ(ret, 0);

  auto forward_reader = ForwardReader::Create("test_collection", "./", 0);
  read_options.create_new = false;
  ret = forward_reader->open(read_options);
  ASSERT_EQ(ret, 0);

  for (size_t i = 0; i < 1000; i++) {
    ForwardData forward;
    ret = forward_reader->seek(i, &forward);

    ASSERT_EQ(ret, 0);
    ASSERT_EQ(forward.header.primary_key, i);
    ASSERT_EQ(forward.header.lsn, i);
    ASSERT_EQ(forward.header.revision, i);
    ASSERT_EQ(forward.data, "hello");
  }

  ailego::ThreadPool pool(3);
  for (size_t i = 0; i < 1000; i++) {
    pool.execute(&do_search, forward_reader.get(), i);
  }
  pool.wait_finish();

  forward_reader->close();
}
