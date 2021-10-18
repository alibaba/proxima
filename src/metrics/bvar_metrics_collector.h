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

#pragma once

#include <memory>
#include <vector>
#include <bvar/bvar.h>
#include "metrics/metrics_collector.h"

namespace proxima {
namespace be {
namespace metrics {

//! Brpc bvar base Metrics
class BvarMetricsCollector : public MetricsCollector {
 public:
  int init(const proxima::be::proto::MetricsConfig &config) override;

  ~BvarMetricsCollector() override = default;

  void report_query_rt(ProtocolType type, uint32_t batch,
                       uint64_t us) override {
    for (size_t i = 0; i < batch; i++) {
      *(query_latency_by_protocol_[static_cast<int>(type)]) << us;
    }
  }

  void report_query_success_count(uint32_t batch) override {
    query_success_count_ << batch;
  }

  void report_query_failure_count(uint32_t batch) override {
    query_failure_count_ << batch;
  }

  void report_query_batch(uint32_t batch) override {
    query_batch_ << batch;
  }

  void report_query_count_by_type(proto::QueryRequest::QueryType query_type,
                                  uint32_t batch) override;

  void report_get_document_rt(ProtocolType type, uint64_t us) override {
    *get_document_latency_by_protocol_[static_cast<int>(type)] << us;
  }

  void report_get_document_success_count() override {
    get_document_success_count_ << 1;
  }

  void report_get_document_failure_count() override {
    get_document_failure_count_ << 1;
  }

  void report_write_rt(ProtocolType type, uint32_t batch,
                       uint64_t us) override {
    for (uint32_t i = 0; i < batch; i++) {
      *write_latency_by_protocol_[static_cast<int>(type)] << us;
    }
  }

  void report_write_success_count(uint32_t batch) override {
    write_success_count_ << batch;
  }

  void report_write_failure_count(uint32_t batch) override {
    write_failure_count_ << batch;
  }

  void report_write_doc_count_by_operation_type(proto::OperationType type,
                                                size_t doc_count) override;

  void report_write_batch(uint32_t batch) override {
    write_batch_ << batch;
  }

 private:
  using IntRecorder = bvar::IntRecorder;
  using LatencyRecorder = bvar::LatencyRecorder;
  using LatencyRecorderUPtr = std::unique_ptr<LatencyRecorder>;
  using LongAdder = bvar::Adder<int64_t>;
  using LongAdderUPtr = std::unique_ptr<LongAdder>;
  using WindowedLongAdder = bvar::Window<LongAdder>;
  using WindowedLongAdderUPtr = std::unique_ptr<WindowedLongAdder>;
  using WindowedIntRecorder = bvar::Window<IntRecorder>;

  static constexpr const char *MODULE_QUERY = "se_query";
  static constexpr const char *MODULE_GET_DOCUMENT = "se_get_document";
  static constexpr const char *MODULE_WRITE = "se_write";

  //! query metrics
  // query single vector request and rt
  std::vector<LatencyRecorderUPtr> query_latency_by_protocol_;
  // query success request count
  LongAdder query_success_count_;
  WindowedLongAdder query_success_count_second_{
      MODULE_QUERY, "success_count_second", &query_success_count_, 1};
  // query failure request count
  LongAdder query_failure_count_;
  WindowedLongAdder query_failure_count_second_{
      MODULE_QUERY, "failure_count_second", &query_failure_count_, 1};
  // average query batch per seconds
  IntRecorder query_batch_;
  WindowedIntRecorder query_batch_second_{MODULE_QUERY, "batch_second",
                                          &query_batch_, 1};
  // query count per query_type
  std::vector<std::unique_ptr<LongAdder>> query_type_counter_;
  std::vector<std::unique_ptr<WindowedLongAdder>> query_type_counter_second_;

  //! get document metrics
  // get document request and rt
  std::vector<LatencyRecorderUPtr> get_document_latency_by_protocol_;
  // get document success request count
  LongAdder get_document_success_count_;
  WindowedLongAdder get_document_success_count_second_{
      MODULE_GET_DOCUMENT, "success_count_second", &get_document_success_count_,
      1};
  // get_document failure request count
  LongAdder get_document_failure_count_;
  WindowedLongAdder get_document_failure_count_second_{
      MODULE_GET_DOCUMENT, "failure_count_second", &get_document_failure_count_,
      1};

  //! write metrics
  // write request and rt
  std::vector<LatencyRecorderUPtr> write_latency_by_protocol_;
  // write success request count
  LongAdder write_success_count_;
  WindowedLongAdder write_success_count_second_{
      MODULE_WRITE, "success_count_second", &write_success_count_, 1};
  // write failure request count
  LongAdder write_failure_count_;
  WindowedLongAdder write_failure_count_second_{
      MODULE_WRITE, "failure_count_second", &write_failure_count_, 1};
  // write document count by operation type
  std::vector<LongAdderUPtr> write_doc_count_by_operation_type_;
  std::vector<WindowedLongAdderUPtr> write_doc_count_by_operation_type_second_;
  // average write batch per seconds
  IntRecorder write_batch_;
  WindowedIntRecorder write_batch_second_{MODULE_WRITE, "batch_second",
                                          &write_batch_, 1};
};

}  // namespace metrics
}  // namespace be
}  // namespace proxima
