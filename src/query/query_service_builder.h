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

/*!
 * Query Service Builder
 */
class QueryServiceBuilder {
 public:
  //! Create one QueryService object
  //! @param index_service: index_service handler
  //! @param meta_service: meta_service handler, which used to validate schema
  //! of collection
  //! @param concurrency: The max concurrency of execution queue
  //! @return valid pointer for success, otherwise failed
  static QueryServicePtr Create(index::IndexServicePtr index_service,
                                meta::MetaServicePtr meta_service,
                                uint32_t concurrency);
};


}  // namespace query
}  // namespace be
}  // namespace proxima