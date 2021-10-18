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
 *   \brief    interface of metrics collector
 */

#pragma once

#include <string>
#include <ailego/pattern/factory.h>
#include <ailego/utility/time_helper.h>
#include "proto/config.pb.h"
#include "proto/proxima_be.pb.h"

namespace proxima {
namespace be {
namespace metrics {

enum class ProtocolType { kHttp = 0, kGrpc, kProtocolSize };
constexpr const size_t kProtocolTypeSize =
    static_cast<size_t>(ProtocolType::kProtocolSize);
constexpr const char *kProtocolName[] = {"http", "grpc"};

//! Metrics Collector interface
class MetricsCollector {
 public:
  static int CreateAndInitMetrics(
      const proxima::be::proto::MetricsConfig &config);

  static MetricsCollector &GetInstance();

  virtual int init(const proxima::be::proto::MetricsConfig & /*config*/) {
    return 0;
  }

  virtual ~MetricsCollector() = default;

  //! report rt per vector
  virtual void report_query_rt(ProtocolType /*type*/, uint32_t /*batch*/,
                               uint64_t /*us*/) {}

  virtual void report_query_success_count(uint32_t /*batch*/) {}

  virtual void report_query_failure_count(uint32_t /*batch*/) {}

  virtual void report_query_batch(uint32_t /*batch*/) {}

  virtual void report_query_count_by_type(
      proto::QueryRequest::QueryType /*query_type*/, uint32_t /*batch*/) {}

  virtual void report_get_document_rt(ProtocolType /*type*/, uint64_t /*us*/) {}

  virtual void report_get_document_success_count() {}

  virtual void report_get_document_failure_count() {}

  virtual void report_write_rt(ProtocolType /*type*/, uint32_t /*batch*/,
                               uint64_t /*us*/) {}

  virtual void report_write_success_count(uint32_t /*batch*/) {}

  virtual void report_write_failure_count(uint32_t /*batch*/) {}

  virtual void report_write_doc_count_by_operation_type(
      proto::OperationType /*type*/, size_t /*doc_count*/) {}

  virtual void report_write_batch(uint32_t /*batch*/) {}

 private:
  static std::string metrics_name_;
};

#define METRICS_REGISTER(__NAME__, __IMPL__, ...) \
  AILEGO_FACTORY_REGISTER(__NAME__, MetricsCollector, __IMPL__, ##__VA_ARGS__)

}  // namespace metrics
}  // namespace be
}  // namespace proxima
