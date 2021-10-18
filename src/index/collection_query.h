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

 *   \author   Haichao.chc
 *   \date     Oct 2020
 *   \brief    Format of collection quries
 */

#pragma once

#include <vector>
#include <aitheta2/index_params.h>
#include "common/types.h"
#include "constants.h"

namespace proxima {
namespace be {
namespace index {

struct QueryResult;
using QueryResultList = std::vector<QueryResult>;
using QueryResultListConstIter = std::vector<QueryResult>::const_iterator;

/*
 * QueryParams represents knn query params.
 */
struct QueryParams {
  uint32_t topk{0U};
  DataTypes data_type{DataTypes::UNDEFINED};
  uint32_t dimension{0U};
  float radius{0.0f};
  uint64_t query_id{0U};
  bool is_linear{false};
  aitheta2::IndexParams extra_params{};
};

/*
 * QueryResult describes the format of knn query response
 */
struct QueryResult {
  uint64_t primary_key{0U};
  float score{0.0f};
  uint64_t revision{0U};
  std::string forward_data{};
  uint64_t lsn{0U};
  bool reverse_sort{false};

  QueryResult() {
    primary_key = INVALID_KEY;
  }

  bool operator<(const QueryResult &other) const {
    if (reverse_sort) {
      return score > other.score;
    } else {
      return score < other.score;
    }
  }
};

}  // end namespace index
}  // namespace be
}  // end namespace proxima
