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
 *   \date     Dec 2020
 *   \brief
 */

#include "query_agent.h"
#include "common/config.h"
#include "common/profiler.h"
#include "query_service_builder.h"

namespace proxima {
namespace be {
namespace query {

/*!
 * QueryAgent implementation
 */
class QueryAgentImpl : public QueryAgent {
 public:
  //! Constructor
  explicit QueryAgentImpl(QueryServicePtr query_service)
      : query_service_(std::move(query_service)), stopped_(false) {}

  //! Destructor
  ~QueryAgentImpl() override = default;

 public:
  //! Get Query Service Instance
  QueryServicePtr get_service() const override {
    return query_service_;
  }

 public:
  //! Query Service
  int search(const proto::QueryRequest *query,
             proto::QueryResponse *response) override {
    int error_code = PROXIMA_BE_ERROR_CODE(StoppedService);
    if (!is_running()) {
      LOG_WARN("QueryAgent stopped, invoke start and try again.");
      return error_code;
    }
    ProfilerPtr profiler = std::make_shared<Profiler>(query->debug_mode());
    profiler->start();
    error_code = query_service_->search(query, response, profiler);
    profiler->stop();
    if (profiler->enabled()) {
      response->set_debug_info(profiler->as_json_string());
    }
    return error_code;
  }

  //! Query Service
  int search_by_key(const proto::GetDocumentRequest *query,
                    proto::GetDocumentResponse *response) override {
    if (!is_running()) {
      LOG_WARN("QueryAgent stopped, invoke start and try again.");
      return PROXIMA_BE_ERROR_CODE(StoppedService);
    }
    ProfilerPtr profiler = std::make_shared<Profiler>(query->debug_mode());
    profiler->start();
    int error_code = query_service_->search_by_key(query, response, profiler);
    profiler->stop();
    if (profiler->enabled()) {
      response->set_debug_info(profiler->as_json_string());
    }
    return error_code;
  }

 public:
  //! Init Query Agent
  int init() override {
    LOG_INFO("QueryAgent initialize complete.");
    return 0;
  }

  //! Cleanup Query Agent
  int cleanup() override {
    int error_code = query_service_->cleanup();
    if (error_code == 0) {
      LOG_INFO("QueryAgent cleanup complete.");
    } else {
      LOG_ERROR("QueryAgent cleanup failed. code[%d], what[%s]", error_code,
                ErrorCode::What(error_code));
    }
    return error_code;
  }

  //! Start background service
  int start() override {
    stopped_.store(false);
    LOG_INFO("QueryAgent start complete.");
    return 0;
  }

  //! Stop background service
  int stop() override {
    LOG_INFO("QueryAgent stopped.");
    stopped_.store(true);
    return 0;
  }

  //! Check QueryAgent is running
  int is_running() override {
    return !stopped_;
  }

 private:
  //! Handler for query service
  QueryServicePtr query_service_{nullptr};

  //! Stopped flag
  std::atomic_bool stopped_{false};
};

//! Create one QueryAgent instance
//! @param: concurrency: the buckets of execution queue, equal 0, means
// using hardware concurrency
QueryAgentPtr QueryAgent::Create(index::IndexServicePtr index_service,
                                 meta::MetaServicePtr meta_service,
                                 uint32_t concurrency) {
  return std::make_shared<QueryAgentImpl>(QueryServiceBuilder::Create(
      std::move(index_service), std::move(meta_service), concurrency));
}

}  // namespace query
}  // namespace be
}  // namespace proxima
