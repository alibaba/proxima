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

 *   \author   Jiliang.ljl
 *   \date     Mar 2021
 *   \brief    Metrics interface.
 */

#include "proto/proxima_be.pb.h"
#include "metrics_collector.h"

namespace proxima {
namespace be {
namespace metrics {


//! QueryMetrics RAII-style reporter
class QueryMetrics {
 public:
  /**
   * Constructor
   *
   * @param type
   * @param ret REQUIRED to be accessible when QueryMetrics destructs.
   */
  QueryMetrics(ProtocolType type, const int *ret) : type_(type), ret_(ret) {}

  //! Update metrics count with request.
  //! request is not saved and can destruct early than QueryMetrics
  void update_with_query_request(const proto::QueryRequest &request) {
    batch_ = GetBatch(request);
    query_type_ = request.query_type();
  }

  ~QueryMetrics() {
    if (batch_ <= 0) {
      return;
    }
    auto rt_us = timer_.micro_seconds();
    auto single_rt_us = rt_us / batch_;
    auto &metrics_obj = MetricsCollector::GetInstance();
    metrics_obj.report_query_rt(type_, batch_, single_rt_us);
    metrics_obj.report_query_count_by_type(query_type_, batch_);
    metrics_obj.report_query_batch(batch_);
    if (*ret_ == 0) {
      metrics_obj.report_query_success_count(batch_);
    } else {
      metrics_obj.report_query_failure_count(batch_);
    }
  }

 private:
  static size_t GetBatch(const proto::QueryRequest &request) {
    switch (request.query_type()) {
      case proto::QueryRequest_QueryType_QT_KNN:
        return request.knn_param().batch_count();
      default:
        // to fix warning enum not handled in switch
        {}
    }
    LOG_ERROR("Unexpected query type:%d",
              static_cast<int>(request.query_type()));
    return 0;
  };

 private:
  ProtocolType type_{ProtocolType::kGrpc};
  const int *ret_{nullptr};
  size_t batch_{0};
  proto::QueryRequest_QueryType query_type_{
      proto::QueryRequest_QueryType_QT_KNN};
  ailego::ElapsedTime timer_;
};

//! GetDocumentMetrics RAII-style reporter
class GetDocumentMetrics {
 public:
  /**
   * Constructor
   *
   * @param type
   * @param ret REQUIRED to be accessible when QueryMetrics destructs.
   */
  GetDocumentMetrics(ProtocolType type, const int *ret)
      : type_(type), ret_(ret) {}

  ~GetDocumentMetrics() {
    auto rt_us = timer_.micro_seconds();
    auto &metrics_obj = MetricsCollector::GetInstance();
    metrics_obj.report_get_document_rt(type_, rt_us);
    if (*ret_ == 0) {
      metrics_obj.report_get_document_success_count();
    } else {
      metrics_obj.report_get_document_failure_count();
    }
  }

 private:
  ProtocolType type_{ProtocolType::kGrpc};
  const int *ret_{nullptr};
  ailego::ElapsedTime timer_;
};

//! WriteMetrics RAII-style reporter
class WriteMetrics {
 public:
  /**
   * Constructor
   *
   * @param type
   * @param ret REQUIRED to be accessible when QueryMetrics destructs.
   */
  WriteMetrics(ProtocolType type, const int *ret) : type_(type), ret_(ret) {}

  ~WriteMetrics() {
    if (!batch_) {
      return;
    }
    auto rt_us = timer_.micro_seconds() / batch_;
    auto &metrics_obj = MetricsCollector::GetInstance();
    metrics_obj.report_write_rt(type_, batch_, rt_us);
    if (insert_doc_count_ > 0) {
      metrics_obj.report_write_doc_count_by_operation_type(proto::OP_INSERT,
                                                           insert_doc_count_);
    }
    if (update_doc_count_ > 0) {
      metrics_obj.report_write_doc_count_by_operation_type(proto::OP_UPDATE,
                                                           update_doc_count_);
    }
    if (delete_doc_count_ > 0) {
      metrics_obj.report_write_doc_count_by_operation_type(proto::OP_DELETE,
                                                           delete_doc_count_);
    }
    metrics_obj.report_write_batch(batch_);
    if (*ret_ == 0) {
      metrics_obj.report_write_success_count(batch_);
    } else {
      metrics_obj.report_write_failure_count(batch_);
    }
  }

  void update_with_write_request(const proto::WriteRequest &req) {
    size_t insert_count = 0;
    size_t update_count = 0;
    size_t delete_count = 0;
    for (const auto &row : req.rows()) {
      switch (row.operation_type()) {
        case proto::OP_INSERT:
          insert_count++;
          break;
        case proto::OP_UPDATE:
          update_count++;
          break;
        case proto::OP_DELETE:
          delete_count++;
          break;
        default:
          LOG_ERROR("Unknown operation type:%d",
                    static_cast<int>(row.operation_type()));
      }
    }
    insert_doc_count_ = insert_count;
    update_doc_count_ = update_count;
    delete_doc_count_ = delete_count;
    batch_ = req.rows_size();
  }

 private:
  ProtocolType type_{ProtocolType::kGrpc};
  const int *ret_{nullptr};
  ailego::ElapsedTime timer_;
  size_t insert_doc_count_{0};
  size_t update_doc_count_{0};
  size_t delete_doc_count_{0};
  size_t batch_{0};
};


}  // namespace metrics

}  // namespace be
}  // namespace proxima
