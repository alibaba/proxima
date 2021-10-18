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
#include <gmock/gmock.h>
#include "index/index_service.h"

using namespace proxima::be;
using namespace proxima::be::index;

class MockSegment : public Segment {
 public:
  ~MockSegment() override = default;

  MOCK_METHOD(size_t, doc_count, (), (const, override));

  MOCK_METHOD(ForwardReaderPtr, get_forward_reader, (), (const, override));

  MOCK_METHOD(ColumnReaderPtr, get_column_reader,
              (const std::string &column_name), (const, override));

  MOCK_METHOD(int, remove_column, (const std::string &column_name), (override));

  MOCK_METHOD(int, add_column, (const meta::ColumnMetaPtr &column_meta),
              (override));

  MOCK_METHOD(int, knn_search,
              (const std::string &column_name, const std::string &query,
               const QueryParams &query_params, QueryResultList *results),
              (override));
  MOCK_METHOD(int, knn_search,
              (const std::string &column_name, const std::string &query,
               const QueryParams &query_params, uint32_t batch_count,
               std::vector<QueryResultList> *results),
              (override));
  MOCK_METHOD(int, kv_search, (uint64_t primary_key, QueryResult *result),
              (override));
};
