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

#include "common/profiler.h"
#include "index/index_service.h"
#include "meta/meta_service.h"
#include "proto/proxima_be.pb.h"

namespace proxima {
namespace be {
namespace query {

//! Predefine class
class QueryService;
//! Alias for QueryService
using QueryServicePtr = std::shared_ptr<QueryService>;


/*!
 * QueryService interface
 */
class QueryService {
 public:
  //! Destructor
  virtual ~QueryService() = default;

 public:
  //! Initialized flag
  virtual bool initialized() const = 0;

  //! Query Service
  virtual int search(const proto::QueryRequest *query,
                     proto::QueryResponse *response, ProfilerPtr profiler) = 0;

  //! Query document by key
  virtual int search_by_key(const proto::GetDocumentRequest *query,
                            proto::GetDocumentResponse *response,
                            ProfilerPtr profiler) = 0;

  //! Cleanup QueryService
  virtual int cleanup() = 0;
};


}  // namespace query
}  // namespace be
}  // namespace proxima
