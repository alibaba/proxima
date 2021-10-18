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

#include "index/segment/persist_segment.h"
#include <gtest/gtest.h>
#include "index/segment/memory_segment.h"

using namespace proxima::be;
using namespace proxima::be::index;

class PersistSegmentTest : public testing::Test {
 protected:
  void SetUp() {
    char cmd_buf[100];
    snprintf(cmd_buf, 100, "rm -rf ./teachers/");
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
  }

 protected:
  meta::CollectionMetaPtr schema_{};
};

TEST_F(PersistSegmentTest, TestGeneral) {
  DeleteStore delete_store("teachers", "./teachers/");
  ReadOptions read_options;
  read_options.use_mmap = true;
  read_options.create_new = true;
  int ret = delete_store.open(read_options);
  ASSERT_EQ(ret, 0);

  IDMap id_map("teachers", "./teachers/");
  ret = id_map.open(read_options);
  ASSERT_EQ(ret, 0);

  SegmentMeta segment_meta;
  segment_meta.segment_id = 0;

  MemorySegmentPtr memory_segment =
      MemorySegment::Create("teachers", "./teachers/", segment_meta,
                            schema_.get(), &delete_store, &id_map, 5);
  ASSERT_TRUE(memory_segment != nullptr);

  ret = memory_segment->open(read_options);
  ASSERT_EQ(ret, 0);

  for (size_t i = 0; i < 1000; i++) {
    Record record;
    record.primary_key = i;
    record.lsn = i;
    record.forward_data = "hello";

    CollectionDataset::ColumnData new_column;
    new_column.column_name = "face";
    new_column.data_type = DataTypes::VECTOR_FP32;
    new_column.dimension = 16U;

    std::vector<float> fvec(16U);
    for (size_t j = 0; j < 16U; j++) {
      fvec[j] = i * 1.0f;
    }
    std::string vector((char *)fvec.data(), fvec.size() * sizeof(float));
    new_column.data = vector;
    record.column_datas.emplace_back(new_column);

    idx_t doc_id;
    ret = memory_segment->insert(record, &doc_id);
    ASSERT_EQ(ret, 0);
    ASSERT_EQ(doc_id, i);
    id_map.insert(record.primary_key, doc_id);
  }

  ret = memory_segment->dump();
  ASSERT_EQ(ret, 0);

  PersistSegmentPtr persist_segment = PersistSegment::Create(
      "teachers", "./teachers/", memory_segment->segment_meta(), schema_.get(),
      &delete_store, &id_map, 5);
  ASSERT_NE(persist_segment, nullptr);

  read_options.create_new = false;
  ret = persist_segment->load(read_options);
  ASSERT_EQ(ret, 0);

  auto &meta0 = memory_segment->segment_meta();
  auto &meta1 = persist_segment->segment_meta();
  ASSERT_EQ(meta0.segment_id, meta1.segment_id);
  ASSERT_EQ(meta0.state, meta1.state);
  ASSERT_EQ(meta0.index_file_size, meta1.index_file_size);
  ASSERT_EQ(meta0.min_doc_id, meta1.min_doc_id);
  ASSERT_EQ(meta0.max_doc_id, meta1.max_doc_id);
  ASSERT_EQ(meta0.min_primary_key, meta1.min_primary_key);
  ASSERT_EQ(meta0.max_primary_key, meta1.max_primary_key);
  ASSERT_EQ(meta0.min_timestamp, meta1.min_timestamp);
  ASSERT_EQ(meta0.max_timestamp, meta1.max_timestamp);

  for (size_t i = 0; i < 1000; i++) {
    std::vector<float> fvec(16U);
    for (size_t j = 0; j < 16U; j++) {
      fvec[j] = i * 1.0f;
    }
    std::string query((char *)fvec.data(), fvec.size() * sizeof(float));
    QueryParams query_params;
    query_params.topk = 10;
    query_params.data_type = DataTypes::VECTOR_FP32;
    query_params.dimension = 16;

    QueryResultList result_list;
    ret =
        persist_segment->knn_search("face", query, query_params, &result_list);
    ASSERT_EQ(ret, 0);

    ASSERT_EQ(result_list[0].primary_key, i);
    ASSERT_EQ(result_list[0].score, 0.0f);
    ASSERT_EQ(result_list[0].lsn, i);
    ASSERT_EQ(result_list[0].forward_data, "hello");
  }

  for (size_t i = 0; i < 1000; i++) {
    std::vector<float> fvec(16U);
    for (size_t j = 0; j < 16U; j++) {
      fvec[j] = i * 1.0f;
    }
    std::string query((char *)fvec.data(), fvec.size() * sizeof(float));
    QueryParams query_params;
    query_params.topk = 10;
    query_params.data_type = DataTypes::VECTOR_FP32;
    query_params.dimension = 16;
    query_params.radius = 0.1f;

    QueryResultList result_list;
    ret =
        persist_segment->knn_search("face", query, query_params, &result_list);
    ASSERT_EQ(ret, 0);

    ASSERT_EQ(result_list.size(), 1);
    ASSERT_EQ(result_list[0].primary_key, i);
    ASSERT_EQ(result_list[0].score, 0.0f);
    ASSERT_EQ(result_list[0].lsn, i);
    ASSERT_EQ(result_list[0].forward_data, "hello");
  }

  for (size_t i = 0; i < 1000; i++) {
    QueryResult result;
    ret = persist_segment->kv_search(i, &result);
    ASSERT_EQ(ret, 0);
    ASSERT_EQ(result.primary_key, i);
    ASSERT_EQ(result.score, 0.0f);
    ASSERT_EQ(result.lsn, i);
    ASSERT_EQ(result.forward_data, "hello");
  }

  for (size_t i = 0; i < 1000; i++) {
    ret = delete_store.insert(i);
    ASSERT_EQ(ret, 0);
  }

  for (size_t i = 0; i < 1000; i++) {
    std::vector<float> fvec(16U);
    for (size_t j = 0; j < 16U; j++) {
      fvec[j] = i * 1.0f;
    }
    std::string query((char *)fvec.data(), fvec.size() * sizeof(float));
    QueryParams query_params;
    query_params.topk = 10;
    query_params.data_type = DataTypes::VECTOR_FP32;
    query_params.dimension = 16;

    QueryResultList result_list;
    ret =
        persist_segment->knn_search("face", query, query_params, &result_list);
    ASSERT_EQ(ret, 0);
    ASSERT_EQ(result_list.size(), 0);
  }

  for (size_t i = 0; i < 1000; i++) {
    QueryResult result;
    ret = persist_segment->kv_search(i, &result);
    ASSERT_EQ(ret, 0);
    ASSERT_EQ(result.primary_key, INVALID_KEY);
  }
}
