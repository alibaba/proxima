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
#include "meta_wrapper.h"
#include "query.h"

namespace proxima {
namespace be {
namespace query {


/*!
 * Create query object from pb.request
 */
class QueryFactory {
 public:
  //! Factory method, build one Query Object from PB Request
  // Valid Query Object returned on success, otherwise return DUMMY OBJECT
  static QueryPtr Create(const proto::QueryRequest *request,
                         index::IndexServicePtr index_service,
                         MetaWrapperPtr meta_service, ExecutorPtr executor,
                         ProfilerPtr profiler, proto::QueryResponse *response);

  //! Factory method, build one Query Object
  static QueryPtr Create(const proto::GetDocumentRequest *request,
                         index::IndexServicePtr index_service,
                         MetaWrapperPtr meta_service, ExecutorPtr executor,
                         ProfilerPtr profiler,
                         proto::GetDocumentResponse *response);
};


}  // namespace query
}  // namespace be
}  // namespace proxima
