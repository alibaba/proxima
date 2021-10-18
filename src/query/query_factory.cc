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

#include "query_factory.h"
#include <atomic>
#include "common/error_code.h"
#include "common/logger.h"
#include "equal_query.h"
#include "knn_query.h"
#include "meta_wrapper.h"

namespace proxima {
namespace be {
namespace query {

namespace {

static uint64_t SequenceTraceId() {
  static std::atomic<uint64_t> kTraceID(0);
  return kTraceID++;
}

/*!
 * DummyQuery implementation
 */
class DummyQuery : public Query {
 public:
  //! Destructor
  ~DummyQuery() override = default;

 public:
  //! Unique request id, using trace all relevant information
  uint64_t id() const override {
    return 0;
  }

  //! Validate query object, 0 for valid, otherwise non zero returned
  int validate() const override {
    return PROXIMA_BE_ERROR_CODE(RuntimeError);
  }

  //! Retrieve IOMode of query
  IOMode mode() const override {
    return IOMode::READONLY;
  }

  //! Retrieve the type of query, Readonly
  QueryType type() const override {
    return QueryType::UNDEFINED;
  }

  //! Prepare resources, 0 for success, otherwise failed
  int prepare() override {
    return PROXIMA_BE_ERROR_CODE(RuntimeError);
  }

  //! Evaluate query, and collection feedback
  int evaluate() override {
    return PROXIMA_BE_ERROR_CODE(RuntimeError);
  }

  int finalize() override {
    return PROXIMA_BE_ERROR_CODE(RuntimeError);
  }

  ProfilerPtr profiler() const override {
    return nullptr;
  }
};

}  // namespace

QueryPtr QueryFactory::Create(const proto::QueryRequest *request,
                              index::IndexServicePtr index_service,
                              MetaWrapperPtr meta_service, ExecutorPtr executor,
                              ProfilerPtr profiler,
                              proto::QueryResponse *response) {
  static QueryPtr kDummyQuery = std::make_shared<DummyQuery>();

  uint64_t trace_id = SequenceTraceId();
  QueryPtr query = kDummyQuery;

  proto::QueryRequest::QueryType type = request->query_type();
  LOG_DEBUG("Received request, trace_id[%zu], type[%d]", (size_t)trace_id,
            type);

  switch (type) {
    case proto::QueryRequest_QueryType_QT_KNN:
      query.reset(new (std::nothrow)
                      KNNQuery(trace_id, request, index_service, meta_service,
                               executor, profiler, response));
      break;
    default:
      LOG_ERROR("Ignore unknown query.");
      break;
  }

  return query;
}

//! Factory method, build one Query Object
QueryPtr QueryFactory::Create(const proto::GetDocumentRequest *request,
                              index::IndexServicePtr index_service,
                              MetaWrapperPtr meta_service, ExecutorPtr executor,
                              ProfilerPtr profiler,
                              proto::GetDocumentResponse *response) {
  uint64_t trace_id = SequenceTraceId();
  auto query =
      std::make_shared<EqualQuery>(trace_id, request, index_service,
                                   meta_service, executor, profiler, response);
  return query;
}

}  // namespace query
}  // namespace be
}  // namespace proxima
