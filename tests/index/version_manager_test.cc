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

#include "index/version_manager.h"
#include <gtest/gtest.h>

using namespace proxima::be;
using namespace proxima::be::index;


class VersionManagerTest : public testing::Test {
 protected:
  void SetUp() {
    char cmd_buf[100];
    snprintf(cmd_buf, 100, "rm -rf ./data.manifest");
    system(cmd_buf);
  }

  void TearDown() {}
};

TEST_F(VersionManagerTest, TestGeneral) {
  auto version_manager = VersionManager::Create("collection_test", "./");
  ASSERT_NE(version_manager, nullptr);

  ReadOptions read_options;
  read_options.use_mmap = true;
  read_options.create_new = true;
  int ret = version_manager->open(read_options);
  ASSERT_EQ(ret, 0);

  ASSERT_EQ(version_manager->total_segment_count(), 1);

  for (size_t i = 1; i < 100; i++) {
    SegmentMeta segment_meta;
    int ret = version_manager->alloc_segment_meta(&segment_meta);
    ASSERT_EQ(ret, 0);
    segment_meta.state = SegmentState::WRITING;
    ret = version_manager->update_segment_meta(segment_meta);
    ASSERT_EQ(ret, 0);
    ASSERT_EQ(segment_meta.segment_id, i);
    ASSERT_EQ(version_manager->total_segment_count(), i + 1);
  }

  for (size_t i = 0; i < 100; i++) {
    VersionEdit edit;
    edit.add_segments.emplace_back(i);
    ret = version_manager->apply(edit);
    ASSERT_EQ(ret, 0);
    ASSERT_EQ(version_manager->current_version().size(), i + 1);
  }

  for (size_t i = 0; i < 100; i++) {
    SegmentMeta segment_meta;
    segment_meta.segment_id = i;
    segment_meta.state = SegmentState::WRITING;
    segment_meta.doc_count = i;
    segment_meta.min_primary_key = i;
    segment_meta.max_primary_key = i;
    segment_meta.min_doc_id = i;
    segment_meta.max_doc_id = i;
    segment_meta.min_timestamp = i;
    segment_meta.max_timestamp = i;

    ret = version_manager->update_segment_meta(segment_meta);
    ASSERT_EQ(ret, 0);
  }

  for (size_t i = 0; i < 100; i++) {
    SegmentMeta segment_meta;
    int ret = version_manager->get_segment_meta(i, &segment_meta);
    ASSERT_EQ(ret, 0);
    ASSERT_EQ(segment_meta.segment_id, i);
    ASSERT_EQ(segment_meta.state, SegmentState::WRITING);
    ASSERT_EQ(segment_meta.doc_count, i);
    ASSERT_EQ(segment_meta.min_primary_key, i);
    ASSERT_EQ(segment_meta.max_primary_key, i);
    ASSERT_EQ(segment_meta.min_doc_id, i);
    ASSERT_EQ(segment_meta.max_doc_id, i);
    ASSERT_EQ(segment_meta.min_timestamp, i);
    ASSERT_EQ(segment_meta.max_timestamp, i);
  }
  version_manager->close();

  read_options.create_new = false;
  ret = version_manager->open(read_options);
  ASSERT_EQ(ret, 0);
  for (size_t i = 0; i < 100; i++) {
    SegmentMeta segment_meta;
    int ret = version_manager->get_segment_meta(i, &segment_meta);
    ASSERT_EQ(ret, 0);
    ASSERT_EQ(segment_meta.segment_id, i);
    ASSERT_EQ(segment_meta.state, SegmentState::WRITING);
    ASSERT_EQ(segment_meta.doc_count, i);
    ASSERT_EQ(segment_meta.min_doc_id, i);
    ASSERT_EQ(segment_meta.max_doc_id, i);
    ASSERT_EQ(segment_meta.min_primary_key, i);
    ASSERT_EQ(segment_meta.max_primary_key, i);
    ASSERT_EQ(segment_meta.min_timestamp, i);
    ASSERT_EQ(segment_meta.max_timestamp, i);
  }

  ASSERT_EQ(version_manager->current_version().size(), 100);
  for (size_t i = 0; i < 100; i++) {
    const SegmentMeta &segment_meta = version_manager->current_version()[i];
    ASSERT_EQ(segment_meta.segment_id, i);
    ASSERT_EQ(segment_meta.doc_count, i);
    ASSERT_EQ(segment_meta.min_primary_key, i);
    ASSERT_EQ(segment_meta.max_primary_key, i);
    ASSERT_EQ(segment_meta.min_doc_id, i);
    ASSERT_EQ(segment_meta.max_doc_id, i);
    ASSERT_EQ(segment_meta.min_timestamp, i);
    ASSERT_EQ(segment_meta.max_timestamp, i);
  }

  // test alloc unused segment meta
  // check if it will be reused
  for (size_t i = 0; i < 100; i++) {
    SegmentMeta segment_meta;
    int ret = version_manager->alloc_segment_meta(&segment_meta);
    ASSERT_EQ(ret, 0);
    ASSERT_EQ(segment_meta.segment_id, 100);
  }
}

TEST_F(VersionManagerTest, TestVersionEdit) {
  auto version_manager = VersionManager::Create("collection_test", "./");
  ASSERT_NE(version_manager, nullptr);

  ReadOptions read_options;
  read_options.use_mmap = true;
  read_options.create_new = true;
  int ret = version_manager->open(read_options);
  ASSERT_EQ(ret, 0);

  for (size_t i = 0; i < 100; i++) {
    SegmentMeta segment_meta;
    ret = version_manager->alloc_segment_meta(&segment_meta);
    ASSERT_EQ(ret, 0);
    segment_meta.state = SegmentState::WRITING;
    ret = version_manager->update_segment_meta(segment_meta);
    ASSERT_EQ(ret, 0);
    ASSERT_EQ(segment_meta.segment_id, i + 1);
  }

  VersionEdit edit1;
  edit1.add_segments.emplace_back(10);
  edit1.add_segments.emplace_back(11);

  ret = version_manager->apply(edit1);
  ASSERT_EQ(ret, 0);
  ASSERT_EQ(version_manager->current_version().size(), 2);
  ASSERT_EQ(version_manager->current_version()[0].segment_id, 10);
  ASSERT_EQ(version_manager->current_version()[1].segment_id, 11);

  VersionEdit edit2;
  edit2.add_segments.emplace_back(12);
  edit2.add_segments.emplace_back(13);
  edit2.delete_segments.emplace_back(10);
  ret = version_manager->apply(edit2);
  ASSERT_EQ(ret, 0);
  ASSERT_EQ(version_manager->current_version().size(), 3);
  ASSERT_EQ(version_manager->current_version()[0].segment_id, 11);
  ASSERT_EQ(version_manager->current_version()[1].segment_id, 12);
  ASSERT_EQ(version_manager->current_version()[2].segment_id, 13);
}
