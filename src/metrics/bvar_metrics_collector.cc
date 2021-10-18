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

 *   \author   jiliang.ljl
 *   \date     Feb 2021
 *   \brief    bvar metrics header
 */

#include "metrics/bvar_metrics_collector.h"
#include <ailego/utility/string_helper.h>

namespace proxima {
namespace be {
namespace metrics {

int BvarMetricsCollector::init(
    const ::proxima::be::proto::MetricsConfig & /*config*/) {
  auto query_type_enum_descriptor = proto::QueryRequest_QueryType_descriptor();
  query_type_counter_.resize(proto::QueryRequest_QueryType_QueryType_ARRAYSIZE);
  query_type_counter_second_.resize(
      proto::QueryRequest_QueryType_QueryType_ARRAYSIZE);
  for (int i = proto::QueryRequest_QueryType_QueryType_MIN;
       i <= proto::QueryRequest_QueryType_QueryType_MAX; i++) {
    if (proto::QueryRequest_QueryType_IsValid(i)) {
      query_type_counter_[i].reset(new LongAdder{});
      query_type_counter_second_[i].reset(new WindowedLongAdder{
          MODULE_QUERY,
          ailego::StringHelper::Concat(
              "type", query_type_enum_descriptor->FindValueByNumber(i)->name(),
              "_count_second"),
          query_type_counter_[i].get(), 1});
    }
  }

  query_latency_by_protocol_.resize(kProtocolTypeSize);
  get_document_latency_by_protocol_.resize(kProtocolTypeSize);
  write_latency_by_protocol_.resize(kProtocolTypeSize);
  for (size_t i = 0; i < kProtocolTypeSize; i++) {
    query_latency_by_protocol_[i].reset(new LatencyRecorder{
        MODULE_QUERY,
        ailego::StringHelper::Concat(kProtocolName[i], "_request")});

    get_document_latency_by_protocol_[i].reset(new LatencyRecorder{
        MODULE_GET_DOCUMENT,
        ailego::StringHelper::Concat(kProtocolName[i], "_request")});

    write_latency_by_protocol_[i].reset(new LatencyRecorder{
        MODULE_WRITE,
        ailego::StringHelper::Concat(kProtocolName[i], "_request")});
  }

  auto operation_type_enum_descriptor = proto::OperationType_descriptor();
  write_doc_count_by_operation_type_.resize(proto::OperationType_ARRAYSIZE);
  write_doc_count_by_operation_type_second_.resize(
      proto::OperationType_ARRAYSIZE);
  for (int i = proto::OperationType_MIN; i <= proto::OperationType_MAX; i++) {
    if (proto::OperationType_IsValid(i)) {
      write_doc_count_by_operation_type_[i].reset(new LongAdder{});
      write_doc_count_by_operation_type_second_[i].reset(new WindowedLongAdder{
          MODULE_WRITE,
          ailego::StringHelper::Concat(
              operation_type_enum_descriptor->FindValueByNumber(i)->name(),
              "_count_second"),
          write_doc_count_by_operation_type_[i].get(), 1});
    }
  }


  return 0;
}

void BvarMetricsCollector::report_query_count_by_type(
    proto::QueryRequest::QueryType query_type, uint32_t batch) {
  if (query_type < 0 ||
      query_type >= static_cast<int>(query_type_counter_.size()) ||
      !query_type_counter_[query_type]) {
    return;
  }
  (*query_type_counter_[query_type]) << batch;
}

void BvarMetricsCollector::report_write_doc_count_by_operation_type(
    proto::OperationType type, size_t doc_count) {
  if (type < 0 ||
      type >= static_cast<int>(write_doc_count_by_operation_type_.size()) ||
      !write_doc_count_by_operation_type_[type]) {
    return;
  }
  (*write_doc_count_by_operation_type_[type]) << doc_count;
}

METRICS_REGISTER(bvar, BvarMetricsCollector);

}  // namespace metrics
}  // namespace be
}  // namespace proxima
