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


#include "query/query_factory.h"
#include <gtest/gtest.h>


using namespace proxima::be::query;

using QueryRequest = proxima::be::proto::QueryRequest;
using QueryResponse = proxima::be::proto::QueryResponse;
using GetDocumentRequest = proxima::be::proto::GetDocumentRequest;
using GetDocumentResponse = proxima::be::proto::GetDocumentResponse;

TEST(QueryBuilerTest, TestDummyQuery) {
  QueryRequest request;
  request.set_query_type(
      proxima::be::proto::
          QueryRequest_QueryType_QueryRequest_QueryType_INT_MIN_SENTINEL_DO_NOT_USE_);

  // Dummy query
  auto query = QueryFactory::Create(&request, nullptr, nullptr, nullptr,
                                    nullptr, nullptr);

  EXPECT_EQ(query->mode(), IOMode::READONLY);
  EXPECT_EQ(query->type(), QueryType::UNDEFINED);

  EXPECT_TRUE(query->validate() != 0);
  EXPECT_TRUE(query->prepare() != 0);
  EXPECT_TRUE(query->evaluate() != 0);
  EXPECT_TRUE(query->finalize() != 0);
}

TEST(QueryBuilerTest, TestValidQuery) {
  {
    QueryRequest request;
    request.set_query_type(proxima::be::proto::QueryRequest_QueryType_QT_KNN);
    // KNN query
    auto query = QueryFactory::Create(&request, nullptr, nullptr, nullptr,
                                      nullptr, nullptr);
    EXPECT_EQ(query->mode(), IOMode::READONLY);
    EXPECT_EQ(query->type(), QueryType::KNN);
    EXPECT_EQ(query->id(), 1);
  }

  {
    GetDocumentRequest request;
    // Equal query
    auto query = QueryFactory::Create(&request, nullptr, nullptr, nullptr,
                                      nullptr, nullptr);
    EXPECT_EQ(query->mode(), IOMode::READONLY);
    EXPECT_EQ(query->type(), QueryType::EQUAL);
    EXPECT_EQ(query->id(), 2);
  }
}