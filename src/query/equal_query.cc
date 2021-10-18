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

#include "equal_query.h"
#include "common/error_code.h"
#include "common/logger.h"

namespace proxima {
namespace be {
namespace query {

//! Constructor
EqualQuery::EqualQuery(uint64_t traceID, const proto::GetDocumentRequest *req,
                       index::IndexServicePtr index,
                       MetaWrapperPtr meta_wrapper, ExecutorPtr executor_ptr,
                       ProfilerPtr profiler_ptr,
                       proto::GetDocumentResponse *resp)
    : ContextImpl(traceID, index, meta_wrapper, profiler_ptr, executor_ptr),
      request_(req),
      response_(resp) {}

//! Destructor
EqualQuery::~EqualQuery() = default;

//! Validate query object, 0 for valid, otherwise non zero returned
int EqualQuery::validate() const {
  // Avoid core dump resulted by nullptr request
  if (!request_) {
    return PROXIMA_BE_ERROR_CODE(InvalidArgument);
  }

  int code = ContextImpl::validate();
  if (code == 0) {
    if (valid_executor()) {
      code = meta()->validate_collection(collection());
    } else {
      LOG_WARN("Invalid response or executor passed to EqualQuery");
      code = PROXIMA_BE_ERROR_CODE(InvalidArgument);
    }
  }
  return code;
}

//! Retrieve IOMode of query
IOMode EqualQuery::mode() const {
  return IOMode::READONLY;
}

//! Retrieve the type of query, Readonly
QueryType EqualQuery::type() const {
  return QueryType::EQUAL;
}

//! Prepare resources, 0 for success, otherwise failed
int EqualQuery::prepare() {
  index::SegmentPtrList segments;
  int code = list_segments(&segments);
  if (code == 0) {
    for (auto &segment : segments) {
      tasks_.emplace_back(std::make_shared<EqualTask>(segment, this));
    }
  } else {
    LOG_ERROR("Failed to build query param from request");
  }
  return code;
}

//! Evaluate query, and collection feedback
int EqualQuery::evaluate() {
  TaskPtrList tasks(tasks_.begin(), tasks_.end());
  int code = executor()->execute_tasks(tasks);
  if (code == 0) {
    // one result or nothing
    for (auto &task : tasks_) {
      if (task->hit()) {  // Pick first hit document, drop others
        proto::Document *doc = response_->mutable_document();
        doc->set_primary_key(primary_key());
        code = fill_forward(task->forward(), doc);
        if (code != 0) {
          LOG_WARN("Fill forward failed. code[%d] what[%s]", code,
                   ErrorCode::What(code));
        }
        break;
      }
    }
  }
  return code;
}

int EqualQuery::finalize() {
  return 0;
}

//! Retrieve collection name
const std::string &EqualQuery::collection() const {
  return request_->collection_name();
}

//! Retrieve primary_key
uint64_t EqualQuery::primary_key() const {
  return request_->primary_key();
}

}  // namespace query
}  // namespace be
}  // namespace proxima
