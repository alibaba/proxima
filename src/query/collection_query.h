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

#pragma once

#include "executor/executor.h"
#include "context.h"
#include "forward_serializer.h"
#include "meta_wrapper.h"
#include "query.h"

namespace proxima {
namespace be {
namespace query {

/*!
 * Implementation of Context
 */
class ContextImpl : public Query, public CollectionQueryContext {
 public:
  //! Constructor
  ContextImpl(uint64_t traceID, index::IndexServicePtr index_service,
              MetaWrapperPtr meta, ProfilerPtr profiler, ExecutorPtr executor);

  //! Destructor
  ~ContextImpl() override = default;

 public:
  //! Unique request id, using trace all relevant information
  uint64_t id() const override;

  //! Validate query object, 0 for valid, otherwise non zero returned
  int validate() const override;

  //! Get Profiler
  ProfilerPtr profiler() const override;

 protected:
  //! Retrieve meta handler
  MetaWrapperPtr meta() const;

  //! Retrieve executor
  ExecutorPtr executor() const;

  //! List segment under current collection
  int list_segments(index::SegmentPtrList *segments);

  //! Fill forward field
  int fill_forward(const index::QueryResult &forward, proto::Document *doc);

  //! Get forward columns
  ColumnNameList *get_forward_columns(const index::QueryResult &forward);

  //! Check valid executor
  bool valid_executor() const {
    return executor_ != nullptr;
  }

 private:
  //! Trace ID
  uint64_t trace_id_{0};

  //! Index Service Handler
  index::IndexServicePtr index_service_{nullptr};

  //! Schema Validator Handler
  MetaWrapperPtr meta_{nullptr};

  //! Executor
  ExecutorPtr executor_{nullptr};

  //! Profiler handler
  ProfilerPtr profiler_{nullptr};

  //! revision map
  std::unordered_map<uint64_t, ColumnNameList> revision_to_forward_columns_;
};


/*!
 * Implementation of CollectionQuery
 */
class CollectionQuery : public ContextImpl, public QueryContext {
 public:
  //! Constructor
  CollectionQuery(uint64_t traceID, const proto::QueryRequest *request,
                  index::IndexServicePtr index_service, MetaWrapperPtr meta,
                  ExecutorPtr executor, ProfilerPtr profiler,
                  proto::QueryResponse *response);

  //! Destructor
  ~CollectionQuery() override;

 public:
  //! Validate query object, 0 for valid, otherwise non zero returned
  int validate() const override;

  //! Retrieve the PB request of query
  const proto::QueryRequest *request() const override;

  //! Retrieve the PB response of query
  const proto::QueryResponse *response() const override;

  //! Retrieve the mutable PB response of query
  proto::QueryResponse *mutable_response() override;

  //! Retrieve collection name
  const std::string &collection() const override;

 protected:
  //! Check valid response
  bool valid_response() const {
    return response_;
  }

 private:
  //! Readonly QueryRequest Handler
  const proto::QueryRequest *request_{nullptr};

  //! Response handler
  proto::QueryResponse *response_{nullptr};
};


}  // namespace query
}  // namespace be
}  // namespace proxima
