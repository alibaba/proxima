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

#include "index/index_service.h"
#include <gtest/gtest.h>

using namespace proxima::be;
using namespace proxima::be::index;

class IndexServiceTest : public testing::Test {
 protected:
  void SetUp() {
    char cmd_buf[100];
    snprintf(cmd_buf, 100, "rm -rf ./teachers/");
    system(cmd_buf);

    snprintf(cmd_buf, 100, "rm -rf ./students/");
    system(cmd_buf);

    FillSchema();
  }

  void TearDown() {}

  void FillSchema() {
    schema_ = std::make_shared<meta::CollectionMeta>();
    meta::ColumnMetaPtr column_meta = std::make_shared<meta::ColumnMeta>();
    column_meta->set_name("face");
    column_meta->set_index_type(IndexTypes::PROXIMA_GRAPH_INDEX);
    column_meta->set_data_type(DataTypes::VECTOR_FP32);
    column_meta->set_dimension(16);
    column_meta->mutable_parameters()->set("metric_type", "SquaredEuclidean");
    schema_->append(column_meta);
    schema_->set_name("teachers");

    schema1_ = std::make_shared<meta::CollectionMeta>();
    meta::ColumnMetaPtr column_meta1 = std::make_shared<meta::ColumnMeta>();
    column_meta1->set_name("face");
    column_meta1->set_index_type(IndexTypes::PROXIMA_GRAPH_INDEX);
    column_meta1->set_data_type(DataTypes::VECTOR_FP32);
    column_meta1->set_dimension(16);
    column_meta1->mutable_parameters()->set("metric_type", "SquaredEuclidean");
    schema1_->append(column_meta1);
    schema1_->set_name("students");
  }

  meta::CollectionMetaPtr CreateSchema(const std::string &name) {
    auto schema = std::make_shared<meta::CollectionMeta>();
    meta::ColumnMetaPtr column_meta = std::make_shared<meta::ColumnMeta>();
    column_meta->set_name("column_test");
    column_meta->set_index_type(IndexTypes::PROXIMA_GRAPH_INDEX);
    column_meta->set_data_type(DataTypes::VECTOR_FP32);
    column_meta->set_dimension(16);
    column_meta->mutable_parameters()->set("metric_type", "SquaredEuclidean");
    schema->append(column_meta);
    schema->set_name(name);
    return schema;
  }

 protected:
  meta::CollectionMetaPtr schema_{};
  meta::CollectionMetaPtr schema1_{};
};

TEST_F(IndexServiceTest, TestGeneral) {
  IndexService index_service;
  int ret = index_service.init();
  ASSERT_EQ(ret, 0);

  ret = index_service.start();
  ASSERT_EQ(ret, 0);

  ret = index_service.create_collection("teachers", schema_);
  ASSERT_EQ(ret, 0);

  ASSERT_EQ(index_service.has_collection("teachers"), true);

  std::vector<std::string> names;
  ret = index_service.list_collections(&names);
  ASSERT_EQ(ret, 0);
  ASSERT_EQ(names.size(), 1);
  ASSERT_EQ(names[0], "teachers");

  std::vector<SegmentPtr> segments;
  ret = index_service.list_segments("teachers", &segments);
  ASSERT_EQ(ret, 0);
  ASSERT_EQ(segments.size(), 1);

  index_service.stop();
}

TEST_F(IndexServiceTest, TestLoadCollection) {
  IndexService index_service;

  ASSERT_EQ(index_service.init(), 0);
  ASSERT_EQ(index_service.start(), 0);

  int ret = index_service.create_collection("teachers", schema_);
  ASSERT_EQ(ret, 0);

  ret = index_service.create_collection("students", schema1_);
  ASSERT_EQ(ret, 0);

  ASSERT_EQ(index_service.stop(), 0);

  ret = index_service.start();
  std::vector<std::string> names;
  names.emplace_back("teachers");
  names.emplace_back("students");

  std::vector<meta::CollectionMetaPtr> schemas;
  schemas.emplace_back(schema_);
  schemas.emplace_back(schema1_);

  ret = index_service.load_collections(names, schemas);
  ASSERT_EQ(ret, 0);

  index_service.stop();
}

void do_hybrid_collection_operations(IndexService *service,
                                     const std::string &name,
                                     const meta::CollectionMetaPtr &schema) {
  int ret = service->create_collection(name, schema);
  ASSERT_EQ(ret, 0);

  ret = service->drop_collection(name);
  ASSERT_EQ(ret, 0);
}

TEST_F(IndexServiceTest, TestMultiThread) {
  IndexService index_service;

  ASSERT_EQ(index_service.init(), 0);
  ASSERT_EQ(index_service.start(), 0);

  ailego::ThreadPool pool(3);
  for (int i = 0; i < 100; i++) {
    std::string name = "collection_" + std::to_string(i);
    auto schema = CreateSchema(name);
    pool.execute(&do_hybrid_collection_operations, &index_service, name,
                 schema);
  }
  pool.wait_finish();
  index_service.stop();
}
