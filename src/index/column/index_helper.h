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
 *   \date     Jun 2021
 *   \brief    Helper class for transforming types
 */

#include "../typedef.h"

namespace proxima {
namespace be {
namespace index {

/*
 * QuantizeTypes support FP16/INT8/INT4
 */
enum class QuantizeTypes : uint32_t {
  UNDEFINED = 0,
  VECTOR_FP16 = 1,
  VECTOR_INT8 = 2,
  VECTOR_INT4 = 3
};

/*
 * Helper calss for transforming some inner types.
 */
class IndexHelper {
 public:
  //! Transform data type to proxima feature type
  static FeatureTypes GetProximaFeatureType(DataTypes data_type);

  //! Tranform str to quantize type
  static QuantizeTypes GetQuantizeType(const std::string &quantize_type);
};

}  // end namespace index
}  // namespace be
}  // end namespace proxima
