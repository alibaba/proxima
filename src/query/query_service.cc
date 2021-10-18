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
 *
 *   \author   guonix
 *   \date     Nov 2020
 *   \brief
 */

#include "query_service.h"
#include <ailego/utility/time_helper.h>
#include "common/error_code.h"
#include "common/logger.h"
#include "executor/parallel_executor.h"
#include "meta_wrapper.h"
#include "query_factory.h"
#include "query_service_builder.h"

namespace proxima {
namespace be {
namespace query {

/*!
 * QueryService Implementation
 */
class QueryServiceImpl : public QueryService {
 public:
  //! Destructor
  QueryServiceImpl(index::IndexServicePtr index_service,
                   MetaWrapperPtr meta_service, ExecutorPtr executor)
      : index_service_(std::move(index_service)),
        meta_service_(std::move(meta_service)),
        executor_(std::move(executor)) {}

  //! Destructor
  ~QueryServiceImpl() override = default;

 public:
  //! Initialized flag
  bool initialized() const override {
    return index_service_ && meta_service_ && executor_;
  }

  //! Query Service
  int search(const proto::QueryRequest *request, proto::QueryResponse *response,
             ProfilerPtr profiler) override {
    if (!initialized() || !request || !response || !profiler) {
      return PROXIMA_BE_ERROR_CODE(RuntimeError);
    }

    // Do not change the sequence of following statements, we need more
    // specific profiling data with debug mode enabled.
    // Step1: Create Query
    profiler->open_stage("before_process_query");
    ailego::ElapsedTime timer;
    auto query = QueryFactory::Create(request, index_service_, meta_service_,
                                      executor_, profiler, response);
    profiler->close_stage();

    // Step2: Process Query
    int code = process_query(query, profiler);
    if (code != 0) {
      LOG_ERROR("Process query failed. code[%d] what[%s]", code,
                ErrorCode::What(code));
      return code;
    }

    // Stage3: After Process Query
    profiler->open_stage("after_process_query");
    uint32_t result_counts = 0;
    for (int i = 0; i < response->results_size(); i++) {
      result_counts += response->results(i).documents_size();
    }
    LOG_INFO(
        "Knn search success. query_id[%zu] batch_count[%u] topk[%u] "
        "is_linear[%d] "
        "resnum[%u] rt[%zuus] collection[%s]",
        (size_t)query->id(), request->knn_param().batch_count(),
        request->knn_param().topk(), request->knn_param().is_linear(),
        result_counts, (size_t)timer.micro_seconds(),
        request->collection_name().c_str());
    profiler->close_stage();
    return code;
  }

  //! Query Service
  int search_by_key(const proto::GetDocumentRequest *request,
                    proto::GetDocumentResponse *response,
                    ProfilerPtr profiler) override {
    if (!initialized() || !request || !response || !profiler) {
      return PROXIMA_BE_ERROR_CODE(RuntimeError);
    }

    ailego::ElapsedTime timer;
    auto query = QueryFactory::Create(request, index_service_, meta_service_,
                                      executor_, profiler, response);

    int code = process_query(query, profiler);
    if (code != 0) {
      LOG_ERROR("Process query failed. code[%d] what[%s]", code,
                ErrorCode::What(code));
      return code;
    }

    uint32_t result_counts = response->has_document() ? 1 : 0;
    LOG_INFO(
        "Kv search success. query_id[%zu] pk[%zu] resnum[%u] rt[%zuus] "
        "collection[%s]",
        (size_t)query->id(), (size_t)request->primary_key(), result_counts,
        (size_t)timer.micro_seconds(), request->collection_name().c_str());
    return code;
  }

  //! Cleanup QueryService
  int cleanup() override {
    index_service_.reset();
    meta_service_.reset();
    executor_.reset();
    return 0;
  }

 private:
  // Process query
  int process_query(QueryPtr query, ProfilerPtr profiler) {
    profiler->add("query_id", query->id());
    profiler->open_stage("query");

    int code = query->validate();
    if (code == 0) {
      code = query->prepare();
      if (code == 0) {
        code = query->evaluate();
      } else {
        LOG_ERROR(
            "Failed to prepare resource for query. trace_id[%zu], code[%d]",
            (size_t)query->id(), code);
      }
    } else {
      LOG_ERROR("Can't validate query, skip it and continue");
    }

    query->finalize();
    LOG_DEBUG("Query [%zu] have been finished", (size_t)query->id());
    profiler->close_stage();

    return code;
  }

 private:
  //! IndexService Handler
  index::IndexServicePtr index_service_{nullptr};

  //! MetaService Handler
  MetaWrapperPtr meta_service_{nullptr};

  //! Executor
  ExecutorPtr executor_{nullptr};
};

QueryServicePtr QueryServiceBuilder::Create(
    index::IndexServicePtr index_service, meta::MetaServicePtr meta_service,
    uint32_t concurrency) {
  if (!index_service || !meta_service) {
    LOG_ERROR(
        "Create QueryService failed, invalid arguments index_service "
        "or meta_service");
    return nullptr;
  }

  if (concurrency == 0) {  // Default host concurrency
    concurrency = Scheduler::HostConcurrency();
  }

  // Share one scheduler between multiple instance of QueryService
  auto scheduler = Scheduler::Default();
  if (scheduler->concurrency() == 0) {
    LOG_INFO("Set concurrency of query service [%u]", concurrency);
    scheduler->concurrency(concurrency);
  }

  auto meta_wrapper = std::make_shared<MetaWrapper>(meta_service);
  auto executor = std::make_shared<ParallelExecutor>(scheduler);

  LOG_INFO("QueryService created with parallel executor");
  return std::make_shared<QueryServiceImpl>(index_service, meta_wrapper,
                                            executor);
}

}  // namespace query
}  // namespace be
}  // namespace proxima
