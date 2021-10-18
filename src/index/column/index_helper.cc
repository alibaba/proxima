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

#include "index_helper.h"

namespace proxima {
namespace be {
namespace index {

FeatureTypes IndexHelper::GetProximaFeatureType(DataTypes data_type) {
  switch (data_type) {
    case DataTypes::VECTOR_BINARY32:
      return FeatureTypes::FT_BINARY32;
    case DataTypes::VECTOR_BINARY64:
      return FeatureTypes::FT_BINARY64;
    case DataTypes::VECTOR_FP16:
      return FeatureTypes::FT_FP16;
    case DataTypes::VECTOR_FP32:
      return FeatureTypes::FT_FP32;
    case DataTypes::VECTOR_FP64:
      return FeatureTypes::FT_FP64;
    case DataTypes::VECTOR_INT4:
      return FeatureTypes::FT_INT4;
    case DataTypes::VECTOR_INT8:
      return FeatureTypes::FT_INT8;
    case DataTypes::VECTOR_INT16:
      return FeatureTypes::FT_INT16;
    default:
      return FeatureTypes::FT_UNDEFINED;
  }
}

QuantizeTypes IndexHelper::GetQuantizeType(const std::string &quantize_type) {
  if (quantize_type == "DT_VECTOR_FP16") {
    return QuantizeTypes::VECTOR_FP16;
  } else if (quantize_type == "DT_VECTOR_INT8") {
    return QuantizeTypes::VECTOR_INT8;
  } else if (quantize_type == "DT_VECTOR_INT4") {
    return QuantizeTypes::VECTOR_INT4;
  } else {
    return QuantizeTypes::UNDEFINED;
  }
}

}  // end namespace index
}  // namespace be
}  // end namespace proxima
