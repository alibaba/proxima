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

#include "index/persist_hash_map.h"
#include <ailego/utility/time_helper.h>
#include <gtest/gtest.h>
#include "index/snapshot.h"
using namespace proxima::be;
using namespace proxima::be::index;


class PersistHashMapTest : public testing::Test {
 protected:
  void SetUp() {
    char cmd_buf[100];
    snprintf(cmd_buf, 100, "rm -rf ./idmap");
    system(cmd_buf);
  }

  void TearDown() {}
};

TEST_F(PersistHashMapTest, TestGeneral) {
  ReadOptions read_options;
  read_options.use_mmap = true;
  read_options.create_new = true;

  SnapshotPtr snapshot;
  int ret = Snapshot::CreateAndOpen("./idmap", FileID::ID_FILE, read_options,
                                    &snapshot);
  ASSERT_EQ(ret, 0);

  PersistHashMap<uint64_t, idx_t> id_map;

  ret = id_map.mount(snapshot->data());
  ASSERT_EQ(ret, 0);

  ASSERT_EQ(0, id_map.reserve(50000));

  for (size_t i = 0; i < 20000; i++) {
    ret = id_map.emplace(i, i);
    ASSERT_EQ(ret, 0);
  }

  size_t size = id_map.size();
  ASSERT_EQ(size, 20000);

  for (size_t i = 0; i < 20000; i++) {
    ASSERT_EQ(id_map.has(i), true);
    idx_t doc_id;
    ASSERT_EQ(0, id_map.get(i, &doc_id));
    ASSERT_EQ(doc_id, i);
  }

  id_map.unmount();

  ret = id_map.mount(snapshot->data());
  ASSERT_EQ(ret, 0);

  size = id_map.size();
  ASSERT_EQ(size, 20000);

  for (size_t i = 0; i < 20000; i++) {
    ASSERT_EQ(id_map.has(i), true);
    idx_t doc_id;
    ASSERT_EQ(0, id_map.get(i, &doc_id));
    ASSERT_EQ(doc_id, i);
  }

  for (size_t i = 0; i < 10000; i++) {
    ret = id_map.erase(i);
    ASSERT_EQ(ret, 0);
  }

  for (size_t i = 0; i < 10000; i++) {
    ASSERT_EQ(id_map.has(i), false);
  }

  for (size_t i = 20000; i < 50000; i++) {
    ret = id_map.emplace(i, i);
    ASSERT_EQ(ret, 0);
  }

  id_map.unmount();

  ret = id_map.mount(snapshot->data());
  ASSERT_EQ(ret, 0);

  size = id_map.size();
  ASSERT_EQ(size, 40000);

  for (size_t i = 0; i < 10000; i++) {
    ASSERT_EQ(id_map.has(i), false);
  }

  for (size_t i = 10000; i < 50000; i++) {
    ASSERT_EQ(id_map.has(i), true);
    idx_t doc_id;
    ASSERT_EQ(0, id_map.get(i, &doc_id));
    ASSERT_EQ(doc_id, i);
  }

  for (size_t i = 40000; i < 50000; i++) {
    int ret = id_map.emplace_or_assign(i, i + 1);
    ASSERT_EQ(ret, 0);
  }

  for (size_t i = 40000; i < 50000; i++) {
    idx_t doc_id;
    ASSERT_EQ(0, id_map.get(i, &doc_id));
    ASSERT_EQ(doc_id, i + 1);
  }
}
