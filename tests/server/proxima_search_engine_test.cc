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
#include "common/config.h"
#undef private
#undef protected

#include <ailego/utility/file_helper.h>
#include <gtest/gtest.h>
#include "common/defer.h"
#include "server/proxima_search_engine.h"
#include "port_helper.h"
#include "proxima_search_client.h"

#ifdef proxima_search_engine_test_VERSION
#define PROXIMA_BE_VERSION_STRING proxima_search_engine_test_VERSION
#else
#define PROXIMA_BE_VERSION_STRING "unknown"
#endif

using namespace proxima::be;
using namespace proxima::be::server;

class ProximaSearchEngineTest : public ::testing::Test {
 protected:
  virtual void SetUp() {
    ailego::FileHelper::RemoveDirectory("./test_proxima_be/");

    int pid;
    PortHelper::GetPort(&grpc_port_, &pid);
    PortHelper::GetPort(&http_port_, &pid);
    PortHelper::RemovePortFile(pid);

    auto &config = Config::Instance();
    config.config_.mutable_common_config()->set_logger_type("ConsoleLogger");
    config.config_.mutable_common_config()->set_log_directory(
        "./test_proxima_be/log/");
    config.config_.mutable_common_config()->set_protocol("grpc");
    config.config_.mutable_common_config()->set_grpc_listen_port(grpc_port_);
    config.config_.mutable_common_config()->set_http_listen_port(http_port_);
    config.config_.mutable_index_config()->set_index_directory(
        "./test_proxima_be/index_data/");
    std::string work_directory;
    ailego::FileHelper::GetWorkingDirectory(&work_directory);
    std::string meta_uri = "sqlite://" + work_directory +
                           "/test_proxima_be/proxima_be_meta.sqlite";
    config.config_.mutable_meta_config()->set_meta_uri(meta_uri);
  }

  virtual void TearDown() {}

 protected:
  int grpc_port_;
  int http_port_;
};

TEST_F(ProximaSearchEngineTest, TestClient) {
  auto &engine = ProximaSearchEngine::Instance();

  int ret = engine.init(false, "");
  ASSERT_EQ(ret, 0);

  Defer defer([&engine] {
    engine.stop();
    engine.cleanup();
  });

  engine.set_version(PROXIMA_BE_VERSION_STRING);
  ret = engine.start();
  ASSERT_EQ(ret, 0);

  // Create a client
  ProximaSearchClientPtr client = ProximaSearchClient::Create();
  ASSERT_TRUE(client != nullptr);

  // Connect to server
  ChannelOptions options(std::string("127.0.0.1:") +
                         std::to_string(grpc_port_));
  options.timeout_ms = 60000U;
  Status status = client->connect(options);
  ASSERT_EQ(status.reason, "Success");
  ASSERT_EQ(status.code, 0);

  // Create collection
  CollectionConfig config;
  config.collection_name = "test_collection";
  config.forward_columns = {"fwd_column1", "fwd_column2", "fwd_column3",
                            "fwd_column4"};
  config.index_columns = {
      IndexColumnParam("test_column", DataType::VECTOR_FP32, 8)};
  status = client->create_collection(config);
  ASSERT_EQ(status.reason, "Success");
  ASSERT_EQ(status.code, 0);

  // Describe collection
  CollectionInfo collection_info;
  status = client->describe_collection("test_collection", &collection_info);
  ASSERT_EQ(status.reason, "Success");
  ASSERT_EQ(status.code, 0);
  ASSERT_EQ(collection_info.collection_name, "test_collection");
  ASSERT_EQ(collection_info.forward_columns.size(), 4);
  ASSERT_EQ(collection_info.forward_columns[0], "fwd_column1");
  ASSERT_EQ(collection_info.forward_columns[1], "fwd_column2");
  ASSERT_EQ(collection_info.forward_columns[2], "fwd_column3");
  ASSERT_EQ(collection_info.forward_columns[3], "fwd_column4");
  ASSERT_EQ(collection_info.index_columns.size(), 1);
  ASSERT_EQ(collection_info.index_columns[0].column_name, "test_column");
  ASSERT_EQ(collection_info.index_columns[0].index_type,
            IndexType::PROXIMA_GRAPH_INDEX);
  ASSERT_EQ(collection_info.index_columns[0].data_type, DataType::VECTOR_FP32);
  ASSERT_EQ(collection_info.index_columns[0].dimension, 8);

  // Insert records
  WriteRequestPtr write_request = WriteRequest::Create();
  write_request->set_collection_name("test_collection");
  write_request->add_forward_columns(
      {"fwd_column1", "fwd_column2", "fwd_column3", "fwd_column4"});
  write_request->add_index_column("test_column", DataType::VECTOR_FP32, 8);

  for (int i = 0; i < 10; i++) {
    WriteRequest::RowPtr row = write_request->add_row();
    row->set_primary_key(i);
    row->set_operation_type(OperationType::INSERT);
    row->add_index_value({i + 0.1f, i + 0.2f, i + 0.3f, i + 0.4f, i + 0.5f,
                          i + 0.6f, i + 0.7f, i + 0.8f});  // "test_column"
    row->add_forward_value("hello" + std::to_string(i));   // "fwd_column1"
    row->add_forward_value((int64_t)i);                    // "fwd_column2"
    row->add_forward_value((float)i);                      // "fwd_column3"
    row->add_forward_value((double)i);                     // "fwd_column4"
  }
  status = client->write(*write_request);
  ASSERT_EQ(status.reason, "Success");
  ASSERT_EQ(status.code, 0);

  // Stats collection
  CollectionStats collection_stats;
  status = client->stats_collection("test_collection", &collection_stats);
  ASSERT_EQ(status.reason, "Success");
  ASSERT_EQ(status.code, 0);

  ASSERT_EQ(collection_stats.collection_name, "test_collection");
  ASSERT_EQ(collection_stats.total_doc_count, 10);
  ASSERT_EQ(collection_stats.total_segment_count, 1);
  ASSERT_EQ(collection_stats.segment_stats.size(), 1);
  ASSERT_EQ(collection_stats.segment_stats[0].doc_count, 10);
  ASSERT_EQ(collection_stats.segment_stats[0].min_primary_key, 0);
  ASSERT_EQ(collection_stats.segment_stats[0].max_primary_key, 9);

  // Query records
  QueryRequestPtr query_request = QueryRequest::Create();
  QueryResponsePtr query_response = QueryResponse::Create();

  query_request->set_collection_name("test_collection");
  QueryRequest::KnnQueryParamPtr knn_param =
      query_request->add_knn_query_param();
  knn_param->set_column_name("test_column");
  knn_param->set_topk(10);
  knn_param->set_features({0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8});

  status = client->query(*query_request, query_response.get());
  ASSERT_EQ(status.reason, "Success");
  ASSERT_EQ(status.code, 0);

  ASSERT_EQ(query_response->result_count(), 1);
  auto result = query_response->result(0);
  ASSERT_EQ(result->document_count(), 10);
  for (size_t i = 0; i < result->document_count(); i++) {
    auto doc = result->document(i);
    ASSERT_EQ(doc->primary_key(), i);
    std::string fwd_val1;
    int64_t fwd_val2;
    float fwd_val3;
    double fwd_val4;
    doc->get_forward_value("fwd_column1", &fwd_val1);
    doc->get_forward_value("fwd_column2", &fwd_val2);
    doc->get_forward_value("fwd_column3", &fwd_val3);
    doc->get_forward_value("fwd_column4", &fwd_val4);
    ASSERT_EQ(fwd_val1, "hello" + std::to_string(i));
    ASSERT_EQ(fwd_val2, (int64_t)i);
    ASSERT_EQ(fwd_val3, (float)i);
    ASSERT_EQ(fwd_val4, (double)i);
  }

  // test wrong forward
  auto doc = result->document(3);
  uint32_t wrong_type_fwd_value1;
  doc->get_forward_value("fwd_column4", &wrong_type_fwd_value1);
  ASSERT_EQ(wrong_type_fwd_value1, 0);

  uint64_t wrong_type_fwd_value2;
  doc->get_forward_value("fwd_column3", &wrong_type_fwd_value2);
  ASSERT_EQ(wrong_type_fwd_value2, 0);

  bool wrong_type_fwd_value3;
  doc->get_forward_value("fwd_column2", &wrong_type_fwd_value3);
  ASSERT_EQ(wrong_type_fwd_value3, false);

  // test insert json format
  WriteRequestPtr write_request2 = WriteRequest::Create();
  write_request2->set_collection_name("test_collection");
  write_request2->add_forward_columns(
      {"fwd_column1", "fwd_column2", "fwd_column3", "fwd_column4"});
  write_request2->add_index_column("test_column", DataType::VECTOR_FP32, 8);
  WriteRequest::RowPtr row = write_request2->add_row();
  row->set_primary_key(10);
  row->set_operation_type(OperationType::INSERT);
  row->add_index_value_by_json(
      "[10.1, 10.2, 10.3, 10.4, 10.5, 10.6, 10.7, 10.8]");
  row->add_forward_value("hello" + std::to_string(10));
  row->add_forward_value((int64_t)10);
  row->add_forward_value((float)10);
  row->add_forward_value((double)10);
  status = client->write(*write_request2);
  ASSERT_EQ(status.reason, "Success");
  ASSERT_EQ(status.code, 0);

  // test query json format
  QueryRequestPtr query_request2 = QueryRequest::Create();
  QueryResponsePtr query_response2 = QueryResponse::Create();
  query_request2->set_collection_name("test_collection");
  auto knn_param2 = query_request2->add_knn_query_param();
  knn_param2->set_column_name("test_column");
  knn_param2->set_topk(10);
  knn_param2->set_features_by_json(
      "[10.1, 10.2, 10.3, 10.4, 10.5, 10.6, 10.7, 10.8]");
  knn_param2->set_data_type(DataType::VECTOR_FP32);
  knn_param2->set_dimension(8);
  status = client->query(*query_request2, query_response2.get());
  ASSERT_EQ(status.reason, "Success");
  ASSERT_EQ(status.code, 0);

  ASSERT_EQ(query_response2->result_count(), 1);
  auto result2 = query_response2->result(0);
  ASSERT_EQ(result2->document_count(), 10);
  {
    auto doc = result2->document(0);
    ASSERT_EQ(doc->primary_key(), 10);
    ASSERT_EQ(doc->score(), 0.0f);
    std::string fwd_val1;
    int64_t fwd_val2;
    float fwd_val3;
    double fwd_val4;
    doc->get_forward_value("fwd_column1", &fwd_val1);
    doc->get_forward_value("fwd_column2", &fwd_val2);
    doc->get_forward_value("fwd_column3", &fwd_val3);
    doc->get_forward_value("fwd_column4", &fwd_val4);
    ASSERT_EQ(fwd_val1, "hello10");
    ASSERT_EQ(fwd_val2, (int64_t)10);
    ASSERT_EQ(fwd_val3, (float)10);
    ASSERT_EQ(fwd_val4, (double)10);
  }

  // Drop collection
  status = client->drop_collection("test_collection");
  ASSERT_EQ(status.reason, "Success");
  ASSERT_EQ(status.code, 0);

  ret = engine.stop();
  ASSERT_EQ(ret, 0);

  ret = engine.cleanup();
  ASSERT_EQ(ret, 0);
}
