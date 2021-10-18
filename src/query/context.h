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

#include "index/collection_query.h"
#include "index/index_service.h"
#include "proto/proxima_be.pb.h"

namespace proxima {
namespace be {
namespace query {

/*!
 * QueryContext which store request and response
 */
class QueryContext {
 public:
  //! Destructor
  virtual ~QueryContext() = default;

 public:
  //! Retrieve the PB request of query
  virtual const proto::QueryRequest *request() const = 0;

  //! Retrieve the PB response of query
  virtual const proto::QueryResponse *response() const = 0;

  //! Retrieve the PB response of query
  virtual proto::QueryResponse *mutable_response() = 0;
};


/*!
 * CollectionQueryContext provide the collection of query
 */
class CollectionQueryContext {
 public:
  //! Destructor
  virtual ~CollectionQueryContext() = default;

 public:
  //! Retrieve collection name
  virtual const std::string &collection() const = 0;
};


/*!
 * KNNQueryContext: Provide all the params needed for invoke
 * segment.knn_search
 */
class KNNQueryContext {
 public:
  //! Destructor
  virtual ~KNNQueryContext() = default;

 public:
  //! Retrieve column name
  virtual const std::string &column() const = 0;

  //! Retrieve features field
  virtual const std::string &features() const = 0;

  //! Retrieve batch_count field
  virtual uint32_t batch_count() const = 0;

  //! Retrieve QueryParams
  virtual const index::QueryParams &query_params() const = 0;
};


/*!
 * QueryKeyContext
 */
class QueryKeyContext {
 public:
  //! Destructor
  virtual ~QueryKeyContext() = default;

 public:
  //! Retrieve primary_key
  virtual uint64_t primary_key() const = 0;
};


}  // namespace query
}  // namespace be
}  // namespace proxima
