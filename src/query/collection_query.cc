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

#include "collection_query.h"
#include "common/error_code.h"
#include "common/logger.h"
#include "forward_serializer.h"

namespace proxima {
namespace be {
namespace query {

ContextImpl::ContextImpl(uint64_t traceID, index::IndexServicePtr index_service,
                         MetaWrapperPtr meta_wrapper, ProfilerPtr profiler_ptr,
                         ExecutorPtr executor_ptr)
    : trace_id_(traceID),
      index_service_(std::move(index_service)),
      meta_(std::move(meta_wrapper)),
      executor_(std::move(executor_ptr)),
      profiler_(std::move(profiler_ptr)) {}

//! Unique request id, using trace all relevant information
uint64_t ContextImpl::id() const {
  return trace_id_;
}

//! Validate query object, 0 for valid, otherwise non zero returned
int ContextImpl::validate() const {
  // Not Every query should have response
  if (!index_service_ || !meta_ || !profiler_) {
    return PROXIMA_BE_ERROR_CODE(RuntimeError);
  }
  return 0;
}

ProfilerPtr ContextImpl::profiler() const {
  return profiler_;
}

MetaWrapperPtr ContextImpl::meta() const {
  return meta_;
}

ExecutorPtr ContextImpl::executor() const {
  return executor_;
}

int ContextImpl::list_segments(index::SegmentPtrList *segments) {
  int code = index_service_->list_segments(collection(), segments);
  if (code != 0) {
    LOG_ERROR("Can't get the segments. collection[%s] code[%d]",
              collection().c_str(), code);
    return code;
  }

  return segments->empty() ? PROXIMA_BE_ERROR_CODE(UnavailableSegment) : 0;
}

ColumnNameList *ContextImpl::get_forward_columns(
    const index::QueryResult &forward) {
  auto it = revision_to_forward_columns_.find(forward.revision);
  if (it != revision_to_forward_columns_.end()) {
    return &it->second;
  }
  ColumnNameList columns;
  int code = meta_->list_columns(collection(), forward.revision, &columns);
  if (code != 0) {
    LOG_ERROR("Can't get the collection meta with specified revision[%zu]",
              (size_t)forward.revision);
    return nullptr;
  }
  LOG_DEBUG("Get the collection meta with specified revision[%zu]",
            (size_t)forward.revision);
  auto iit = revision_to_forward_columns_.emplace(forward.revision,
                                                  std::move(columns));
  return &iit.first->second;
}

int ContextImpl::fill_forward(const index::QueryResult &forward,
                              proto::Document *doc) {
  auto *columns = get_forward_columns(forward);
  int code = PROXIMA_BE_ERROR_CODE(InvalidRevision);
  if (columns) {
    code = ForwardSerializer::FillForward(forward, *columns, doc);
  }
  return code;
}

//! Constructor
CollectionQuery::CollectionQuery(uint64_t traceID,
                                 const proto::QueryRequest *pb_request,
                                 index::IndexServicePtr index_service_ptr,
                                 MetaWrapperPtr meta_wrapper,
                                 ExecutorPtr executor_ptr,
                                 ProfilerPtr profiler_ptr,
                                 proto::QueryResponse *pb_response)
    : ContextImpl(traceID, std::move(index_service_ptr),
                  std::move(meta_wrapper), std::move(profiler_ptr),
                  std::move(executor_ptr)),
      request_(pb_request),
      response_(pb_response) {}

//! Destructor
CollectionQuery::~CollectionQuery() = default;

//! Validate query object, 0 for valid, otherwise non zero returned
int CollectionQuery::validate() const {
  // Not Every query should have response
  int code = ContextImpl::validate();
  if (code != 0) {
    return code;
  }
  return !request_ || !response_ ? PROXIMA_BE_ERROR_CODE(RuntimeError) : 0;
}

//! Retrieve the PB request of query
const proto::QueryRequest *CollectionQuery::request() const {
  return request_;
}

//! Retrieve the PB response of query
const proto::QueryResponse *CollectionQuery::response() const {
  return response_;
}

//! Retrieve the mutable PB response of query
proto::QueryResponse *CollectionQuery::mutable_response() {
  return response_;
}

//! Retrieve collection name
const std::string &CollectionQuery::collection() const {
  return request_->collection_name();
}


}  // namespace query
}  // namespace be
}  // namespace proxima
