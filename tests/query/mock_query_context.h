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

#include <gmock/gmock.h>
#include "query/context.h"

using namespace proxima::be;
using namespace proxima::be::query;

using namespace ::testing;  // for testing::*

using QueryRequest = proxima::be::proto::QueryRequest;
using QueryResponse = proxima::be::proto::QueryResponse;

//! QueryContext which store request and response
class MockQueryContext : public QueryContext {
 public:
  ~MockQueryContext() override = default;

  //! Retrieve the PB request of query
  MOCK_METHOD(const QueryRequest *, request, (), (const, override));

  //! Retrieve the PB response of query
  MOCK_METHOD(const QueryResponse *, response, (), (const, override));

  //! Retrieve the PB response of query
  MOCK_METHOD(QueryResponse *, mutable_response, (), (override));
};

//! CollectionQueryContext provide the collection of query
class MockCollectionQueryContext : public CollectionQueryContext {
 public:
  //! Destructor
  ~MockCollectionQueryContext() override = default;

 public:
  //! Retrieve collection name
  MOCK_METHOD(const std::string &, collection, (), (const, override));
};

//! KNNQueryContext: Provide all the params needed for invoke segment.knn_search
class MockKNNQueryContext : public KNNQueryContext {
 public:
  //! Destructor
  ~MockKNNQueryContext() override = default;

 public:
  //! Retrieve column name
  MOCK_METHOD(const std::string &, column, (), (const, override));

  //! Retrieve features field
  MOCK_METHOD(const std::string &, features, (), (const, override));

  //! Retrieve batch_count field
  MOCK_METHOD(uint32_t, batch_count, (), (const, override));

  //! Retrieve QueryParams
  MOCK_METHOD(const index::QueryParams &, query_params, (), (const, override));
};

//! EqualQueryContext
class MockEqualQueryContext : public QueryKeyContext {
 public:
  //! Destructor
  ~MockEqualQueryContext() override = default;

 public:
  //! Retrieve primary_key
  MOCK_METHOD(uint64_t, primary_key, (), (const, override));
};