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

#include "collection_query.h"
#include "equal_task.h"

namespace proxima {
namespace be {
namespace query {

/*!
 * EqualQuery
 */
class EqualQuery : public ContextImpl, public QueryKeyContext {
 public:
  //! Constructor
  EqualQuery(uint64_t traceID, const proto::GetDocumentRequest *req,
             index::IndexServicePtr index, MetaWrapperPtr meta_wrapper,
             ExecutorPtr executor_ptr, ProfilerPtr profiler_ptr,
             proto::GetDocumentResponse *resp);

  //! Destructor
  ~EqualQuery() override;

 public:
  //! Validate query object, 0 for valid, otherwise non zero returned
  int validate() const override;

  //! Retrieve IOMode of query
  IOMode mode() const override;

  //! Retrieve the type of query, Readonly
  QueryType type() const override;

  //! Prepare resources, 0 for success, otherwise failed
  int prepare() override;

  //! Evaluate query, and collection feedback
  int evaluate() override;

  //! Finalize query object
  int finalize() override;

  //! Retrieve collection name
  const std::string &collection() const override;

  //! Retrieve primary_key
  uint64_t primary_key() const override;

 private:
  //! Equal tasks
  EqualTaskPtrList tasks_{};

  // Request pointer readonly field
  const proto::GetDocumentRequest *request_{nullptr};

  // Response pointer
  proto::GetDocumentResponse *response_{nullptr};
};


}  // namespace query
}  // namespace be
}  // namespace proxima
