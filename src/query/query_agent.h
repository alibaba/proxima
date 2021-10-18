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

#pragma once

#include "query_service.h"

namespace proxima {
namespace be {
namespace query {

//! Predefine class
class QueryAgent;
//! Alias Pointer for QueryAgent
using QueryAgentPtr = std::shared_ptr<QueryAgent>;

/*!
 * QueryAgent interface
 */
class QueryAgent {
 public:
  //! Create one QueryAgent instance
  //! @param: concurrency: the buckets of execution queue, equal 0, means
  // using hardware concurrency
  static QueryAgentPtr Create(index::IndexServicePtr index_service,
                              meta::MetaServicePtr meta_service,
                              uint32_t concurrency);

 public:
  //! Destructor
  virtual ~QueryAgent() = default;

  //! Get Query Service Instance
  virtual QueryServicePtr get_service() const = 0;

 public:
  //! Query Service
  virtual int search(const proto::QueryRequest *query,
                     proto::QueryResponse *response) = 0;

  //! Query document by key
  virtual int search_by_key(const proto::GetDocumentRequest *query,
                            proto::GetDocumentResponse *response) = 0;

 public:
  //! Init Query Agent
  virtual int init() = 0;

  //! Cleanup Query Agent
  virtual int cleanup() = 0;

  //! Start background service
  virtual int start() = 0;

  //! Stop background service
  virtual int stop() = 0;

  //! Return running status
  virtual int is_running() = 0;
};


}  // namespace query
}  // namespace be
}  // namespace proxima
