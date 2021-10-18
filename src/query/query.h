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
 *   \date     Oct 2020
 *   \brief
 */

#pragma once

#include <memory>
#include "common/profiler.h"
#include "query_types.h"

namespace proxima {
namespace be {
namespace query {

//! Predefine class
class Query;
//! Alias for QueryPtr
using QueryPtr = std::shared_ptr<Query>;


/*!
 * Query Interface
 */
class Query {
 public:
  //! Destructor
  virtual ~Query() = default;

 public:
  //! Unique request id, using trace all relevant information
  virtual uint64_t id() const = 0;

  //! Validate query object, 0 for valid, otherwise non zero returned
  virtual int validate() const = 0;

  //! Retrieve IOMode of query
  virtual IOMode mode() const = 0;

  //! Retrieve the type of query, Readonly
  virtual QueryType type() const = 0;

  //! Prepare resources, 0 for success, otherwise failed
  virtual int prepare() = 0;

  //! Evaluate query, and collection feedback
  virtual int evaluate() = 0;

  //! Finalize query object
  virtual int finalize() = 0;

  /* Common usage of query
   * 1. get query object
   * 2. invoke invalidate
   * 3. invoke prepare
   * 4. invoke evaluate
   * 5. collect all the feedback from query
   * 6. Handle query finished
   */

  //! Get Profiler
  virtual ProfilerPtr profiler() const = 0;
};


}  // namespace query
}  // namespace be
}  // namespace proxima
