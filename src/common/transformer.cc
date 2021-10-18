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
 *   \date     Mar 2021
 *   \brief
 */

#include "transformer.h"

namespace proxima {
namespace be {

bool Transformer::NeedTransform(DataTypes in_type, DataTypes out_type) {
  return in_type != out_type;
}

int Transformer::SupportTransform(DataTypes in_type, DataTypes out_type) {
  if ((in_type == out_type) || (in_type == DataTypes::VECTOR_FP32 &&
                                out_type == DataTypes::VECTOR_FP16)) {
    return 0;
  }
  return PROXIMA_BE_ERROR_CODE(MismatchedDataType);
}

int Transformer::Transform(DataTypes in_type, const std::string &in,
                           DataTypes out_type, std::string *out) {
  if (Transformer::NeedTransform(in_type, out_type)) {
    if (in_type == DataTypes::VECTOR_FP32 &&
        out_type == DataTypes::VECTOR_FP16) {
      if (in.empty() || in.length() % sizeof(float) != 0) {
        return PROXIMA_BE_ERROR_CODE(InvalidFeature);
      }
      // counts the elements of query
      auto elements = in.length() / sizeof(float);
      // resize fp32(4 bytes) -> fp16(2 bytes)
      out->resize(elements << 1);

      ailego::FloatHelper::ToFP16(
          reinterpret_cast<const float *>(in.c_str()), elements,
          reinterpret_cast<uint16_t *>(const_cast<char *>(out->c_str())));
      return 0;
    }
    return PROXIMA_BE_ERROR_CODE(MismatchedDataType);
  }
  *out = in;
  return 0;
}

//! Transform double of JsonValue to float
template <>
float Json2Primary::Primary<float>(const ailego::JsonValue &object) {
  return float(object.as_float());
}

//! Transform double of JsonValue
template <>
double Json2Primary::Primary<double>(const ailego::JsonValue &object) {
  return object.as_float();
}


template <>
void Primary2Bytes::Bytes<int8_t, DataTypes::VECTOR_INT4>(
    const std::vector<int8_t> &values, std::string *bytes) {
  bytes->resize(values.size() >> 1);
  uint8_t *out = reinterpret_cast<uint8_t *>(&(*bytes)[0]);
  for (size_t i = 0; i < values.size(); i += 2) {
    out[i >> 1] = (static_cast<uint8_t>(values[i + 1]) << 4) |
                  (static_cast<uint8_t>(values[i]) & 0xF);
  }
}

template <>
void Primary2Bytes::Bytes<float, DataTypes::VECTOR_FP16>(
    const std::vector<float> &values, std::string *bytes) {
  bytes->resize(values.size() * sizeof(float) >> 1);
  ailego::FloatHelper::ToFP16(reinterpret_cast<const float *>(values.data()),
                              values.size(),
                              reinterpret_cast<uint16_t *>(&(*bytes)[0]));
}

}  // namespace be
}  // namespace proxima
