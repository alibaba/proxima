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
 *   \brief    Common types that will be used by all the project
 */

#pragma once

#include <limits>
#include <aitheta2/index_params.h>

namespace proxima {
namespace be {

/*
 * Collection operation types
 */
enum class OperationTypes : uint32_t {
  UNDEFINED = 0,
  INSERT = 1,
  UPDATE = 2,
  DELETE = 3
};

/*
 * Column index types
 */
enum class IndexTypes : uint32_t { UNDEFINED = 0, PROXIMA_GRAPH_INDEX };

/*
 * Column data types
 */
enum class DataTypes : uint32_t {
  UNDEFINED = 0,

  BINARY = 1,
  STRING = 2,
  BOOL = 3,
  INT32 = 4,
  INT64 = 5,
  UINT32 = 6,
  UINT64 = 7,
  FLOAT = 8,
  DOUBLE = 9,

  VECTOR_BINARY32 = 20,
  VECTOR_BINARY64 = 21,
  VECTOR_FP16 = 22,
  VECTOR_FP32 = 23,
  VECTOR_FP64 = 24,
  VECTOR_INT4 = 25,
  VECTOR_INT8 = 26,
  VECTOR_INT16 = 27
};

/*
 * Collection query types
 */
enum class QueryTypes : uint32_t { UNDEFINED = 0, KNN_QUERY, EQUAL_QUERY };

}  // namespace be
}  // end namespace proxima
