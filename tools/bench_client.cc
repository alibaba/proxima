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

 *   \author   Haichao.chc
 *   \date     Oct 2020
 *   \brief    Client tool that can send benchmark requests
 *             to remote proxima be server
 */

#include <fstream>
#include <iostream>
#include <map>
#include <ailego/parallel/thread_pool.h>
#include <ailego/utility/string_helper.h>
#include <ailego/utility/time_helper.h>
#include <bvar/latency_recorder.h>
#include <gflags/gflags.h>
#include <google/protobuf/util/json_util.h>
#include "common/logger.h"
#include "common/protobuf_helper.h"
#include "common/version.h"
#include "proxima_search_client.h"
#include "vecs_reader.h"

using namespace proxima::be;

DEFINE_string(command, "", "Command type: search/insert/delete/update");
DEFINE_string(host, "", "The host of proxima search engine");
DEFINE_string(collection, "", "Collection name");
DEFINE_string(column, "", "Column name");
DEFINE_string(file, "", "Input file path");
DEFINE_string(protocol, "grpc", "Protocol http or grpc");
DEFINE_string(data_type, "float", "Input data type");
DEFINE_uint32(concurrency, 10, "Concurrency connection to server(default 10)");
DEFINE_uint32(topk, 10, "Query topk results(default 10)");
DEFINE_bool(perf, false, "Output perf result");
DEFINE_uint32(rows, 0, "Limit loaded records count");

static bool ValidateNotEmpty(const char *flagname, const std::string &value) {
  return !value.empty();
}

DEFINE_validator(command, ValidateNotEmpty);
DEFINE_validator(host, ValidateNotEmpty);
DEFINE_validator(collection, ValidateNotEmpty);
DEFINE_validator(file, ValidateNotEmpty);

static inline void PrintUsage() {
  std::cout << "Usage:" << std::endl;
  std::cout << " bench_client <args>" << std::endl << std::endl;
  std::cout << "Args: " << std::endl;
  std::cout
      << " --command        Command type: search|insert|delete|update|recall"
      << std::endl;
  std::cout << " --host           The host of proxima be" << std::endl;
  std::cout << " --collection     Specify collection name" << std::endl;
  std::cout << " --column         Specify column name" << std::endl;
  std::cout << " --file           Read input data from file" << std::endl;
  std::cout << " --protocol       Protocol http or grpc" << std::endl;
  std::cout << " --data_type      Support float/binary now (default float)"
            << std::endl;
  std::cout << " --concurrency    Send concurrency (default 10)" << std::endl;
  std::cout << " --topk           Topk results (default 10)" << std::endl;
  std::cout << " --perf           Output perf result (default false)"
            << std::endl;
  std::cout << " --rows           Limit loaded rows count" << std::endl;
  std::cout << " --help, -h       Display help info" << std::endl;
  std::cout << " --version, -v    Display version info" << std::endl;
}

// Global storage
struct Record {
  uint64_t key;
  std::string vector;
  std::string attributes;
  uint32_t dimension;
};

// Global varibles for recall statistics
static std::atomic<uint64_t> g_top1_total_count(0U);
static std::atomic<uint64_t> g_top1_hit_count(0U);
static std::atomic<uint64_t> g_top10_total_count(0U);
static std::atomic<uint64_t> g_top10_hit_count(0U);
static std::atomic<uint64_t> g_top50_total_count(0U);
static std::atomic<uint64_t> g_top50_hit_count(0U);
static std::atomic<uint64_t> g_top100_total_count(0U);
static std::atomic<uint64_t> g_top100_hit_count(0U);
static std::atomic<uint64_t> g_topk_total_count(0U);
static std::atomic<uint64_t> g_topk_hit_count(0U);

// Global varibles for qps statistics
static std::atomic<bool> g_running(false);
static uint64_t g_min_insert_qps(-1U);
static uint64_t g_max_insert_qps(0U);
static uint64_t g_min_search_qps(-1U);
static uint64_t g_max_search_qps(0U);
static uint64_t g_min_update_qps(-1U);
static uint64_t g_max_update_qps(0U);
static uint64_t g_min_delete_qps(-1U);
static uint64_t g_max_delete_qps(0U);
static std::vector<Record> g_record_list;
static ProximaSearchClientPtr g_client;
static bvar::LatencyRecorder g_insert_latency_recorder;
static bvar::LatencyRecorder g_search_latency_recorder;
static bvar::LatencyRecorder g_update_latency_recorder;
static bvar::LatencyRecorder g_delete_latency_recorder;

static bool LoadFromTextFile() {
  std::ifstream file_stream(FLAGS_file);
  if (!file_stream.is_open()) {
    LOG_ERROR("Can't open input file %s", FLAGS_file.c_str());
    return false;
  }

  std::string line;
  while (std::getline(file_stream, line)) {
    line.erase(line.find_last_not_of('\n') + 1);
    if (line.empty()) {
      continue;
    }
    std::vector<std::string> res;
    ailego::StringHelper::Split(line, ';', &res);
    if (res.size() < 1) {
      LOG_ERROR("Bad input line");
      continue;
    }

    Record record;
    // Parse key
    uint64_t key = std::stoull(res[0]);
    record.key = key;
    // Parse feature
    if (res.size() >= 2) {
      if (FLAGS_data_type == "binary") {
        std::vector<uint8_t> vec;
        ailego::StringHelper::Split(res[1], ' ', &vec);
        if (vec.size() == 0 || vec.size() % 32 != 0) {
          LOG_ERROR("Bad feature field");
          continue;
        }

        std::vector<uint8_t> tmp;
        for (size_t i = 0; i < vec.size(); i += 8) {
          uint8_t v = 0;
          v |= (vec[i] & 0x01) << 7;
          v |= (vec[i + 1] & 0x01) << 6;
          v |= (vec[i + 2] & 0x01) << 5;
          v |= (vec[i + 3] & 0x01) << 4;
          v |= (vec[i + 4] & 0x01) << 3;
          v |= (vec[i + 5] & 0x01) << 2;
          v |= (vec[i + 6] & 0x01) << 1;
          v |= (vec[i + 7] & 0x01) << 0;
          tmp.push_back(v);
        }

        record.vector =
            std::string((const char *)tmp.data(), tmp.size() * sizeof(uint8_t));
        record.dimension = vec.size();
      } else {
        std::vector<float> feature;
        ailego::StringHelper::Split(res[1], ' ', &feature);
        if (feature.size() == 0) {
          LOG_ERROR("Bad feature field");
          continue;
        }
        record.vector = std::string((const char *)feature.data(),
                                    feature.size() * sizeof(float));
        record.dimension = feature.size();
      }
    }
    // Parse attributes
    if (res.size() >= 3) {
      record.attributes = res[2];
    }
    g_record_list.emplace_back(record);

    // Limit rows count
    if (FLAGS_rows > 0 && g_record_list.size() >= FLAGS_rows) {
      break;
    }
  }
  file_stream.close();

  return true;
}

static bool LoadFromVecsFile() {
  tools::VecsReader reader;
  if (!reader.load(FLAGS_file)) {
    LOG_ERROR("Load vecs file failed.");
    return false;
  }

  for (uint32_t i = 0; i < reader.num_vecs(); i++) {
    Record new_record;
    uint64_t key = reader.get_key(i);
    const char *feature = (const char *)reader.get_vector(i);

    new_record.key = key;
    new_record.vector.append(feature, reader.index_meta().element_size());
    new_record.dimension = reader.index_meta().dimension();
    g_record_list.emplace_back(new_record);

    // Limit rows count
    if (FLAGS_rows > 0 && g_record_list.size() >= FLAGS_rows) {
      break;
    }
  }

  return true;
}

static bool LoadRecords() {
  bool ret;
  if (FLAGS_file.find(".vecs") != std::string::npos) {
    ret = LoadFromVecsFile();
  } else {
    ret = LoadFromTextFile();
  }

  return ret;
}

static bool InitClient() {
  if (FLAGS_protocol == "http") {
    g_client = ProximaSearchClient::Create("HttpClient");
  } else if (FLAGS_protocol == "grpc") {
    g_client = ProximaSearchClient::Create("GrpcClient");
  } else {
    LOG_ERROR("Unknown protocol, only support http or grpc now. protocol[%s]",
              FLAGS_protocol.c_str());
    return false;
  }

  proxima::be::ChannelOptions options(FLAGS_host);
  options.connection_count = FLAGS_concurrency;
  options.timeout_ms = 60000;
  Status status = g_client->connect(options);
  if (status.code != 0) {
    LOG_ERROR("Connect failed. code[%d] reason[%s]", status.code,
              status.reason.c_str());
    return false;
  }
  return true;
}

static void DoSearchProxima(Record *record) {
  ailego::ElapsedTime timer;
  QueryRequestPtr request = QueryRequest::Create();
  request->set_collection_name(FLAGS_collection);
  auto knn_param = request->add_knn_query_param();
  knn_param->set_column_name(FLAGS_column);
  knn_param->set_topk(FLAGS_topk);
  knn_param->set_features(record->vector.c_str(), record->vector.size(), 1);
  if (FLAGS_data_type == "binary") {
    knn_param->set_data_type(DataType::VECTOR_BINARY32);
  } else {
    knn_param->set_data_type(DataType::VECTOR_FP32);
  }
  knn_param->set_dimension(record->dimension);

  QueryResponsePtr response = QueryResponse::Create();
  Status status = g_client->query(*request, response.get());
  if (status.code != 0) {
    LOG_ERROR("Search records failed. query_id[%zu] code[%d] reason[%s] ",
              (size_t)record->key, status.code, status.reason.c_str());
    return;
  }

  auto result = response->result(0);
  std::string result_str;
  for (size_t i = 0; i < result->document_count(); i++) {
    auto doc = result->document(i);
    std::string attr;
    doc->get_forward_value("forward", &attr);
    if (attr.empty()) {
      ailego::StringHelper::Append(&result_str, " ", doc->primary_key(), ":",
                                   doc->score());
    } else {
      ailego::StringHelper::Append(&result_str, " ", doc->primary_key(), ":",
                                   doc->score(), ":", attr);
    }
  }

  uint64_t latency_us = timer.micro_seconds();
  g_search_latency_recorder << latency_us;

  if (!FLAGS_perf) {
    LOG_INFO(
        "Search records success. query_id[%zu] res_num[%zu] results[%s] "
        "rt[%zuus]",
        (size_t)record->key, result->document_count(), result_str.c_str(),
        (size_t)latency_us);
  }
}

static void DoInsertProxima(Record *record) {
  ailego::ElapsedTime timer;
  WriteRequestPtr request = WriteRequest::Create();
  request->set_collection_name(FLAGS_collection);
  if (FLAGS_data_type == "binary") {
    request->add_index_column(FLAGS_column, DataType::VECTOR_BINARY32,
                              record->dimension);
  } else {
    request->add_index_column(FLAGS_column, DataType::VECTOR_FP32,
                              record->dimension);
  }
  // Support forward column temporarily
  if (!record->attributes.empty()) {
    request->add_forward_column("forward");
  }

  auto row = request->add_row();
  row->set_operation_type(OperationType::INSERT);
  row->set_primary_key(record->key);
  row->add_index_value(record->vector.c_str(), record->vector.size());
  // Support forward column temporarily
  if (!record->attributes.empty()) {
    row->add_forward_value(record->attributes);
  }

  Status status = g_client->write(*request);
  if (status.code != 0) {
    LOG_ERROR("Insert record failed. key[%zu] code[%d] reason[%s]",
              (size_t)record->key, status.code, status.reason.c_str());
    return;
  }

  uint64_t latency_us = timer.micro_seconds();
  g_insert_latency_recorder << latency_us;

  if (!FLAGS_perf) {
    LOG_INFO("Insert record success. key[%zu] rt[%zuus]", (size_t)record->key,
             (size_t)latency_us);
  }
}

static void DoDeleteProxima(Record *record) {
  ailego::ElapsedTime timer;
  WriteRequestPtr request = WriteRequest::Create();
  request->set_collection_name(FLAGS_collection);
  auto row = request->add_row();
  row->set_operation_type(OperationType::DELETE);
  row->set_primary_key(record->key);
  row->add_index_value(record->vector.c_str(), record->vector.size());

  Status status = g_client->write(*request);
  if (status.code != 0) {
    LOG_ERROR("Delete record failed. key[%zu] code[%d] reason[%s]",
              (size_t)record->key, status.code, status.reason.c_str());
    return;
  }

  uint64_t latency_us = timer.micro_seconds();
  g_delete_latency_recorder << latency_us;

  if (!FLAGS_perf) {
    LOG_INFO("Delete record success. key[%zu] rt[%zuus]", (size_t)record->key,
             (size_t)latency_us);
  }
}

static void DoUpdateProxima(Record *record) {
  ailego::ElapsedTime timer;
  WriteRequestPtr request = WriteRequest::Create();
  request->set_collection_name(FLAGS_collection);
  if (FLAGS_data_type == "binary") {
    request->add_index_column(FLAGS_column, DataType::VECTOR_BINARY32,
                              record->dimension);
  } else {
    request->add_index_column(FLAGS_column, DataType::VECTOR_FP32,
                              record->dimension);
  }
  auto row = request->add_row();
  row->set_operation_type(OperationType::UPDATE);
  row->set_primary_key(record->key);
  row->add_index_value(record->vector.c_str(), record->vector.size());

  Status status = g_client->write(*request);
  if (status.code != 0) {
    LOG_ERROR("Update record failed. key[%zu] code[%d] reason[%s]",
              (size_t)record->key, status.code, status.reason.c_str());
    return;
  }

  uint64_t latency_us = timer.micro_seconds();
  g_update_latency_recorder << latency_us;

  if (!FLAGS_perf) {
    LOG_INFO("Update record success. key[%zu] rt[%zuus]", (size_t)record->key,
             (size_t)latency_us);
  }
}

#define ADD_RECALL_COUNT(topk, total_count, hit_count)                   \
  for (size_t i = 0; i < topk && i < result1->document_count(); i++) {   \
    total_count++;                                                       \
    auto doc1 = result1->document(i);                                    \
    for (size_t j = 0; j < topk && j < result2->document_count(); j++) { \
      auto doc2 = result2->document(j);                                  \
      if (doc1->primary_key() == doc2->primary_key() ||                  \
          doc1->score() == doc2->score()) {                              \
        hit_count++;                                                     \
        break;                                                           \
      }                                                                  \
    }                                                                    \
  }


static void DoRecallProxima(Record *record) {
  QueryRequestPtr request = QueryRequest::Create();
  request->set_collection_name(FLAGS_collection);
  auto knn_param = request->add_knn_query_param();
  knn_param->set_column_name(FLAGS_column);
  knn_param->set_topk(FLAGS_topk);
  knn_param->set_features(record->vector.c_str(), record->vector.size(), 1);
  if (FLAGS_data_type == "binary") {
    knn_param->set_data_type(DataType::VECTOR_BINARY32);
  } else {
    knn_param->set_data_type(DataType::VECTOR_FP32);
  }
  knn_param->set_dimension(record->dimension);

  // 1. get knn results
  auto response1 = QueryResponse::Create();
  Status status = g_client->query(*request, response1.get());
  if (status.code != 0) {
    LOG_ERROR("Knn search records failed. query_id[%zu] code[%d] reason[%s] ",
              (size_t)record->key, status.code, status.reason.c_str());
    return;
  }
  auto result1 = response1->result(0);

  // 2. get linear knn results
  knn_param->set_linear(true);
  auto response2 = QueryResponse::Create();
  status = g_client->query(*request, response2.get());
  if (status.code != 0) {
    LOG_ERROR(
        "Linear search records failed. query_id[%zu] code[%d] reason[%s] ",
        (size_t)record->key, status.code, status.reason.c_str());
    return;
  }
  auto result2 = response2->result(0);

  if (result1->document_count() != result2->document_count()) {
    LOG_ERROR(
        "Knn search results count mismatch linear search results. result1[%lu] "
        "result2[%lu]",
        result1->document_count(), result2->document_count());
    return;
  }

  if (FLAGS_topk > 1) {
    ADD_RECALL_COUNT(1, g_top1_total_count, g_top1_hit_count);
  }

  if (FLAGS_topk > 10) {
    ADD_RECALL_COUNT(10, g_top10_total_count, g_top10_hit_count);
  }

  if (FLAGS_topk > 50) {
    ADD_RECALL_COUNT(50, g_top50_total_count, g_top50_hit_count);
  }

  if (FLAGS_topk > 100) {
    ADD_RECALL_COUNT(100, g_top100_total_count, g_top100_hit_count);
  }

  ADD_RECALL_COUNT(FLAGS_topk, g_topk_total_count, g_topk_hit_count);
}

#define OUTPUT_PERF_RESULT(recorder, max_qps, min_qps)                         \
  std::cout << "====================PERFORMANCE======================"         \
            << std::endl;                                                      \
  std::cout << "Process count  : " << recorder.count() << std::endl;           \
  std::cout << "Average qps    : " << recorder.qps() << "/s" << std::endl;     \
  std::cout << "Maximum qps    : " << max_qps << "/s" << std::endl;            \
  std::cout << "Minimum qps    : " << min_qps << "/s" << std::endl;            \
  std::cout << "Average latency: " << recorder.latency() << "us" << std::endl; \
  std::cout << "Maximum latency: " << recorder.max_latency() << "us"           \
            << std::endl;                                                      \
  std::cout << "Percentile @1  : " << recorder.latency_percentile(0.01)        \
            << "us" << std::endl;                                              \
  std::cout << "Percentile @10 : " << recorder.latency_percentile(0.10)        \
            << "us" << std::endl;                                              \
  std::cout << "Percentile @25 : " << recorder.latency_percentile(0.25)        \
            << "us" << std::endl;                                              \
  std::cout << "Percentile @50 : " << recorder.latency_percentile(0.50)        \
            << "us" << std::endl;                                              \
  std::cout << "Percentile @75 : " << recorder.latency_percentile(0.75)        \
            << "us" << std::endl;                                              \
  std::cout << "Percentile @90 : " << recorder.latency_percentile(0.90)        \
            << "us" << std::endl;                                              \
  std::cout << "Percentile @95 : " << recorder.latency_percentile(0.95)        \
            << "us" << std::endl;                                              \
  std::cout << "Percentile @99 : " << recorder.latency_percentile(0.99)        \
            << "us" << std::endl;

static void SearchRecords() {
  if (FLAGS_column.empty()) {
    LOG_ERROR("Input argument column can't be emtpy");
    return;
  }

  ailego::ThreadPool thread_pool(FLAGS_concurrency, false);
  for (size_t i = 0; i < g_record_list.size(); i++) {
    thread_pool.execute(DoSearchProxima, &g_record_list[i]);

    while (thread_pool.pending_count() > 1000) {
      usleep(1000);
      thread_pool.wake_all();
    }
  }
  thread_pool.wait_finish();

  if (FLAGS_perf) {
    OUTPUT_PERF_RESULT(g_search_latency_recorder, g_max_search_qps,
                       g_min_search_qps);
  }
}

static void InsertRecords() {
  if (FLAGS_column.empty()) {
    LOG_ERROR("Input argument column can't be emtpy");
    return;
  }

  ailego::ThreadPool thread_pool(FLAGS_concurrency, false);
  for (size_t i = 0; i < g_record_list.size(); i++) {
    thread_pool.execute(DoInsertProxima, &g_record_list[i]);
    while (thread_pool.pending_count() > 1000) {
      usleep(1000);
      thread_pool.wake_all();
    }
  }
  thread_pool.wait_finish();

  if (FLAGS_perf) {
    OUTPUT_PERF_RESULT(g_insert_latency_recorder, g_max_insert_qps,
                       g_min_insert_qps);
  }
}

static void DeleteRecords() {
  ailego::ThreadPool thread_pool(FLAGS_concurrency, false);
  for (size_t i = 0; i < g_record_list.size(); i++) {
    thread_pool.execute(DoDeleteProxima, &g_record_list[i]);
    while (thread_pool.pending_count() > 1000) {
      usleep(1000);
      thread_pool.wake_all();
    }
  }
  thread_pool.wait_finish();

  if (FLAGS_perf) {
    OUTPUT_PERF_RESULT(g_delete_latency_recorder, g_max_delete_qps,
                       g_min_delete_qps);
  }
}

static void UpdateRecords() {
  if (FLAGS_column.empty()) {
    LOG_ERROR("Input argument column can't be emtpy");
    return;
  }

  ailego::ThreadPool thread_pool(FLAGS_concurrency, false);
  for (size_t i = 0; i < g_record_list.size(); i++) {
    thread_pool.execute(DoUpdateProxima, &g_record_list[i]);
    while (thread_pool.pending_count() > 1000) {
      usleep(1000);
      thread_pool.wake_all();
    }
  }
  thread_pool.wait_finish();

  if (FLAGS_perf) {
    OUTPUT_PERF_RESULT(g_update_latency_recorder, g_max_update_qps,
                       g_min_update_qps);
  }
}

static void RecallRecords() {
  if (FLAGS_column.empty()) {
    LOG_ERROR("Input argument column can't be emtpy");
    return;
  }

  ailego::ThreadPool thread_pool(FLAGS_concurrency, false);
  for (size_t i = 0; i < g_record_list.size(); i++) {
    thread_pool.execute(DoRecallProxima, &g_record_list[i]);
    while (thread_pool.pending_count() > 1000) {
      usleep(1000);
      thread_pool.wake_all();
    }
  }
  thread_pool.wait_finish();

  // Output recall ratio
  if (FLAGS_topk > 1) {
    double top1_hit_ratio = g_top1_total_count > 0
                                ? (double)g_top1_hit_count / g_top1_total_count
                                : 0.0f;
    std::cout << "Recall @1: " << top1_hit_ratio << std::endl;
  }

  if (FLAGS_topk > 10) {
    double top10_hit_ratio =
        g_top10_total_count > 0
            ? (double)g_top10_hit_count / g_top10_total_count
            : 0.0f;
    std::cout << "Recall @10: " << top10_hit_ratio << std::endl;
  }

  if (FLAGS_topk > 50) {
    double top50_hit_ratio =
        g_top50_total_count > 0
            ? (double)g_top50_hit_count / g_top50_total_count
            : 0.0f;
    std::cout << "Recall @50: " << top50_hit_ratio << std::endl;
  }

  if (FLAGS_topk > 100) {
    double top100_hit_ratio =
        g_top100_total_count > 0
            ? (double)g_top100_hit_count / g_top100_total_count
            : 0.0f;
    std::cout << "Recall @100: " << top100_hit_ratio << std::endl;
  }

  double topk_hit_ratio = g_topk_total_count > 0
                              ? (double)g_topk_hit_count / g_topk_total_count
                              : 0.0f;
  std::cout << "Recall @" << FLAGS_topk << ": " << topk_hit_ratio << std::endl;
}

static void Monitor() {
  std::this_thread::sleep_for(std::chrono::seconds(5));
  while (g_running) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    if (FLAGS_command == "search") {
      uint64_t qps = (uint64_t)g_search_latency_recorder.qps(1);
      if (qps > g_max_search_qps) {
        g_max_search_qps = qps;
      }
      if (qps < g_min_search_qps) {
        g_min_search_qps = qps;
      }
    } else if (FLAGS_command == "insert") {
      uint64_t qps = (uint64_t)g_insert_latency_recorder.qps(1);
      if (qps > g_max_insert_qps) {
        g_max_insert_qps = qps;
      }
      if (qps < g_min_insert_qps) {
        g_min_insert_qps = qps;
      }
    } else if (FLAGS_command == "update") {
      uint64_t qps = (uint64_t)g_update_latency_recorder.qps(1);
      if (qps > g_max_update_qps) {
        g_max_update_qps = qps;
      }
      if (qps < g_min_update_qps) {
        g_min_update_qps = qps;
      }
    } else if (FLAGS_command == "delete") {
      uint64_t qps = (uint64_t)g_delete_latency_recorder.qps(1);
      if (qps > g_max_delete_qps) {
        g_max_delete_qps = qps;
      }
      if (qps < g_min_delete_qps) {
        g_min_delete_qps = qps;
      }
    }   
  }
}

int main(int argc, char **argv) {
  // Parse arguments
  for (int i = 1; i < argc; ++i) {
    const char *arg = argv[i];
    if (!strcmp(arg, "-help") || !strcmp(arg, "--help") || !strcmp(arg, "-h")) {
      PrintUsage();
      exit(0);
    } else if (!strcmp(arg, "-version") || !strcmp(arg, "--version") ||
               !strcmp(arg, "-v")) {
      std::cout << proxima::be::Version::Details() << std::endl;
      exit(0);
    }
  }
  gflags::ParseCommandLineNonHelpFlags(&argc, &argv, false);

  // Init client channel
  if (!InitClient()) {
    LOG_ERROR("Init client failed. host[%s]", FLAGS_host.c_str());
    exit(1);
  }

  // Load data from input file
  if (!LoadRecords()) {
    LOG_ERROR("Load data from file failed. file[%s]", FLAGS_file.c_str());
    exit(1);
  }
  std::cout << "Load data complete. num[" << g_record_list.size() << "]"
            << std::endl;
  g_running = true;

  // Add monitor thread
  std::thread *monitor_thread = nullptr;
  if (FLAGS_perf) {
    monitor_thread = new std::thread(Monitor);
  }

  // Register commands
  std::map<std::string, std::function<void(void)>> record_ops = {
      {"search", SearchRecords},
      {"insert", InsertRecords},
      {"update", UpdateRecords},
      {"delete", DeleteRecords},
      {"recall", RecallRecords}};
  if (record_ops.find(FLAGS_command) != record_ops.end()) {
    record_ops[FLAGS_command]();
  } else {
    LOG_ERROR("Unsupported command type: %s", FLAGS_command.c_str());
    exit(1);
  }

  g_running = false;
  if (monitor_thread) {
    monitor_thread->join();
    delete monitor_thread;
  }

  return 0;
}


#undef OUTPUT_PERF_RESULT
