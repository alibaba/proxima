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

#include "index/column/column_indexer.h"
#include <gtest/gtest.h>
#include "index/file_helper.h"

using namespace proxima::be;
using namespace proxima::be::index;

class ColumnIndexerTest : public testing::Test {
 protected:
  void SetUp() {
    FileHelper::RemoveFile("./data.pxa.test_column.0");
  }

  void TearDown() {}
};

TEST_F(ColumnIndexerTest, TestGeneral) {
  auto column_indexer =
      ColumnIndexer::Create("test_collection", "./", 0, "test_column",
                            IndexTypes::PROXIMA_GRAPH_INDEX);

  meta::ColumnMeta meta;
  meta.set_name("test_column");
  meta.set_index_type(IndexTypes::PROXIMA_GRAPH_INDEX);
  meta.set_data_type(DataTypes::VECTOR_FP32);
  meta.set_dimension(16);

  ReadOptions read_options;
  read_options.use_mmap = true;
  read_options.create_new = true;
  int ret = column_indexer->open(meta, read_options);
  ASSERT_EQ(ret, 0);

  // Test insert
  for (size_t i = 0; i < 1000; i++) {
    std::vector<float> fvec(16U);
    for (size_t j = 0; j < 16U; j++) {
      fvec[j] = i * 1.0f;
    }

    std::string vector((char *)fvec.data(), fvec.size() * sizeof(float));
    ColumnData column_data;
    column_data.column_name = "test_column";
    column_data.data_type = DataTypes::VECTOR_FP32;
    column_data.dimension = 16;
    column_data.data = vector;
    ret = column_indexer->insert(i, column_data);
    ASSERT_EQ(ret, 0);
  }

  // Test search
  for (size_t i = 0; i < 1000; i++) {
    std::vector<float> fvec(16U);
    for (size_t j = 0; j < 16U; j++) {
      fvec[j] = i * 1.0f;
    }

    IndexDocumentList result_list;
    std::string query((char *)fvec.data(), fvec.size() * sizeof(float));
    QueryParams query_params;
    query_params.topk = 10;
    ret = column_indexer->search(query, query_params, nullptr, &result_list);
    ASSERT_EQ(ret, 0);
    ASSERT_NEAR(result_list[0].score(), 0.0f, 0.1f);
    ASSERT_EQ(result_list[0].key(), i);
  }
}

TEST_F(ColumnIndexerTest, TestQuantizeFP16) {
  auto column_indexer =
      ColumnIndexer::Create("test_collection", "./", 0, "test_column",
                            IndexTypes::PROXIMA_GRAPH_INDEX);

  meta::ColumnMeta meta;
  meta.set_name("test_column");
  meta.set_index_type(IndexTypes::PROXIMA_GRAPH_INDEX);
  meta.set_data_type(DataTypes::VECTOR_FP32);
  meta.set_dimension(16);
  meta.mutable_parameters()->set("quantize_type", "DT_VECTOR_FP16");

  ReadOptions read_options;
  read_options.use_mmap = true;
  read_options.create_new = true;
  int ret = column_indexer->open(meta, read_options);
  ASSERT_EQ(ret, 0);

  for (size_t i = 0; i < 1000; i++) {
    std::vector<float> fvec(16U);
    for (size_t j = 0; j < 16U; j++) {
      fvec[j] = i * 1.0f;
    }

    std::string vector((char *)fvec.data(), fvec.size() * sizeof(float));
    ColumnData column_data;
    column_data.column_name = "test_column";
    column_data.data_type = DataTypes::VECTOR_FP32;
    column_data.dimension = 16;
    column_data.data = vector;
    ret = column_indexer->insert(i, column_data);
    ASSERT_EQ(ret, 0);
  }

  for (size_t i = 0; i < 1000; i++) {
    std::vector<float> fvec(16U);
    for (size_t j = 0; j < 16U; j++) {
      fvec[j] = i * 1.0f;
    }

    IndexDocumentList result_list;
    std::string query((char *)fvec.data(), fvec.size() * sizeof(float));
    QueryParams query_params;
    query_params.topk = 10;
    ret = column_indexer->search(query, query_params, nullptr, &result_list);
    ASSERT_EQ(ret, 0);
    ASSERT_NEAR(result_list[0].score(), 0.0f, 0.1f);
    ASSERT_EQ(result_list[0].key(), i);
  }
}

TEST_F(ColumnIndexerTest, TestQuantizeINT8) {
  auto column_indexer =
      ColumnIndexer::Create("test_collection", "./", 0, "test_column",
                            IndexTypes::PROXIMA_GRAPH_INDEX);

  meta::ColumnMeta meta;
  meta.set_name("test_column");
  meta.set_index_type(IndexTypes::PROXIMA_GRAPH_INDEX);
  meta.set_data_type(DataTypes::VECTOR_FP32);
  meta.set_dimension(16);
  meta.mutable_parameters()->set("quantize_type", "DT_VECTOR_INT8");

  ReadOptions read_options;
  read_options.use_mmap = true;
  read_options.create_new = true;
  int ret = column_indexer->open(meta, read_options);
  ASSERT_EQ(ret, 0);

  for (size_t i = 0; i < 1000; i++) {
    std::vector<float> fvec(16U);
    for (size_t j = 0; j < 16U; j++) {
      fvec[j] = i * 1.0f;
    }

    std::string vector((char *)fvec.data(), fvec.size() * sizeof(float));
    ColumnData column_data;
    column_data.column_name = "test_column";
    column_data.data_type = DataTypes::VECTOR_FP32;
    column_data.dimension = 16;
    column_data.data = vector;
    ret = column_indexer->insert(i, column_data);
    ASSERT_EQ(ret, 0);
  }

  for (size_t i = 0; i < 1000; i++) {
    std::vector<float> fvec(16U);
    for (size_t j = 0; j < 16U; j++) {
      fvec[j] = i * 1.0f;
    }

    IndexDocumentList result_list;
    std::string query((char *)fvec.data(), fvec.size() * sizeof(float));
    QueryParams query_params;
    query_params.topk = 10;
    ret = column_indexer->search(query, query_params, nullptr, &result_list);
    ASSERT_EQ(ret, 0);
    ASSERT_NEAR(result_list[0].score(), 0.0f, 0.1f);
    ASSERT_EQ(result_list[0].key(), i);
  }
}

TEST_F(ColumnIndexerTest, TestQuantizeINT8InnerProduct) {
  auto column_indexer =
      ColumnIndexer::Create("test_collection_int8_ip", "./", 0, "test_column",
                            IndexTypes::PROXIMA_GRAPH_INDEX);
  meta::ColumnMeta meta;
  meta.set_name("test_column");
  meta.set_index_type(IndexTypes::PROXIMA_GRAPH_INDEX);
  meta.set_data_type(DataTypes::VECTOR_FP32);
  meta.set_dimension(16);
  meta.mutable_parameters()->set("quantize_type", "DT_VECTOR_INT8");
  meta.mutable_parameters()->set("metric_type", "InnerProduct");
  ReadOptions read_options;
  read_options.use_mmap = true;
  read_options.create_new = true;
  int ret = column_indexer->open(meta, read_options);
  ASSERT_EQ(ret, 0);
  for (size_t i = 0; i <= 1000; i++) {
    std::vector<float> fvec(16U);
    for (size_t j = 0; j < 16U; j++) {
      fvec[j] = i * 0.001f;
    }
    std::string vector((char *)fvec.data(), fvec.size() * sizeof(float));
    ColumnData column_data;
    column_data.column_name = "test_column";
    column_data.data_type = DataTypes::VECTOR_FP32;
    column_data.dimension = 16;
    column_data.data = vector;
    ret = column_indexer->insert(i, column_data);
    ASSERT_EQ(ret, 0);
  }
  std::vector<float> fvec(16U);
  for (size_t j = 0; j < 16U; j++) {
    fvec[j] = 1.0f;
  }
  IndexDocumentList result_list;
  std::string query((char *)fvec.data(), fvec.size() * sizeof(float));
  QueryParams query_params;
  query_params.topk = 10;
  ret = column_indexer->search(query, query_params, nullptr, &result_list);
  ASSERT_EQ(ret, 0);
  ASSERT_EQ(1000, result_list[0].key());
  ASSERT_NEAR(result_list[0].score(), 16.0f, 0.1f);
}

TEST_F(ColumnIndexerTest, TestQuantizeINT4) {
  auto column_indexer =
      ColumnIndexer::Create("test_collection", "./", 0, "test_column",
                            IndexTypes::PROXIMA_GRAPH_INDEX);

  meta::ColumnMeta meta;
  meta.set_name("test_column");
  meta.set_index_type(IndexTypes::PROXIMA_GRAPH_INDEX);
  meta.set_data_type(DataTypes::VECTOR_FP32);
  meta.set_dimension(16);
  meta.mutable_parameters()->set("quantize_type", "DT_VECTOR_INT4");

  ReadOptions read_options;
  read_options.use_mmap = true;
  read_options.create_new = true;
  int ret = column_indexer->open(meta, read_options);
  ASSERT_EQ(ret, 0);

  for (size_t i = 0; i < 1000; i++) {
    std::vector<float> fvec(16U);
    for (size_t j = 0; j < 16U; j++) {
      fvec[j] = i * 1.0f;
    }

    std::string vector((char *)fvec.data(), fvec.size() * sizeof(float));
    ColumnData column_data;
    column_data.column_name = "test_column";
    column_data.data_type = DataTypes::VECTOR_FP32;
    column_data.dimension = 16;
    column_data.data = vector;
    ret = column_indexer->insert(i, column_data);
    ASSERT_EQ(ret, 0);
  }

  for (size_t i = 0; i < 1000; i++) {
    std::vector<float> fvec(16U);
    for (size_t j = 0; j < 16U; j++) {
      fvec[j] = i * 1.0f;
    }

    IndexDocumentList result_list;
    std::string query((char *)fvec.data(), fvec.size() * sizeof(float));
    QueryParams query_params;
    query_params.topk = 10;
    ret = column_indexer->search(query, query_params, nullptr, &result_list);
    ASSERT_EQ(ret, 0);
    ASSERT_NEAR(result_list[0].score(), 0.0f, 0.1f);
    ASSERT_EQ(result_list[0].key(), i);
  }
}

TEST_F(ColumnIndexerTest, TestOswgEngine) {
  auto column_indexer =
      ColumnIndexer::Create("test_collection", "./", 0, "test_column",
                            IndexTypes::PROXIMA_GRAPH_INDEX);

  meta::ColumnMeta meta;
  meta.set_name("test_column");
  meta.set_index_type(IndexTypes::PROXIMA_GRAPH_INDEX);
  meta.set_data_type(DataTypes::VECTOR_FP32);
  meta.set_dimension(16);
  meta.mutable_parameters()->set("engine", "OSWG");

  ReadOptions read_options;
  read_options.use_mmap = true;
  read_options.create_new = true;
  int ret = column_indexer->open(meta, read_options);
  ASSERT_EQ(ret, 0);

  for (size_t i = 0; i < 1000; i++) {
    std::vector<float> fvec(16U);
    for (size_t j = 0; j < 16U; j++) {
      fvec[j] = i * 1.0f;
    }

    std::string vector((char *)fvec.data(), fvec.size() * sizeof(float));
    ColumnData column_data;
    column_data.column_name = "test_column";
    column_data.data_type = DataTypes::VECTOR_FP32;
    column_data.dimension = 16;
    column_data.data = vector;
    ret = column_indexer->insert(i, column_data);
    ASSERT_EQ(ret, 0);
  }

  for (size_t i = 0; i < 1000; i++) {
    std::vector<float> fvec(16U);
    for (size_t j = 0; j < 16U; j++) {
      fvec[j] = i * 1.0f;
    }

    IndexDocumentList result_list;
    std::string query((char *)fvec.data(), fvec.size() * sizeof(float));
    QueryParams query_params;
    query_params.topk = 10;
    ret = column_indexer->search(query, query_params, nullptr, &result_list);
    ASSERT_EQ(ret, 0);
    ASSERT_NEAR(result_list[0].score(), 0.0f, 0.1f);
    ASSERT_EQ(result_list[0].key(), i);
  }


  for (size_t i = 0; i < 1000; i++) {
    ret = column_indexer->remove(i);
    ASSERT_EQ(ret, 0);

    std::vector<float> fvec(16U);
    for (size_t j = 0; j < 16U; j++) {
      fvec[j] = i * 1.0f;
    }

    IndexDocumentList result_list;
    std::string query((char *)fvec.data(), fvec.size() * sizeof(float));
    QueryParams query_params;
    query_params.topk = 10;
    ret = column_indexer->search(query, query_params, nullptr, &result_list);
    ASSERT_EQ(ret, 0);
    if (result_list.size() > 0) {
      ASSERT_NE(result_list[0].key(), i);
    }
  }
}
