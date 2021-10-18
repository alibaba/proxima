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

#pragma once

#include <ailego/encoding/json.h>
#include <ailego/utility/float_helper.h>
#include "error_code.h"
#include "logger.h"
#include "types.h"

namespace proxima {
namespace be {

/*! Transformer
 */
class Transformer {
 public:
  //! Check transform action
  static bool NeedTransform(DataTypes in, DataTypes out);

  //! Check support current transform
  static int SupportTransform(DataTypes in_type, DataTypes out_type);

  //! Transform input features to output features
  // return 0 for success otherwise failed
  static int Transform(DataTypes, const std::string &, DataTypes,
                       std::string *);

  //! Transform json array to std::vector, return count of element has been
  // transformed
  template <typename T>
  static size_t Transform(const ailego::JsonArray &array,
                          std::vector<T> *values);

  //! Interpret json value to vector, param matrix should contains valid
  // matrix or array object.
  template <typename T>
  static size_t Transform(const ailego::JsonValue &matrix,
                          std::vector<T> *values);

  //! Interpret json string to vector, param json should contains valid
  // matrix or array object.
  template <typename T>
  static size_t Transform(
      const std::string &json,
      std::function<int(const ailego::JsonValue &)> *validator,
      std::vector<T> *values);

  //! transform vector to bytes
  template <typename T, DataTypes>
  static size_t Transform(const std::vector<T> &values, std::string *bytes);
};

/*!
 * Transform json object to primary data
 */
struct Json2Primary {
  template <class T>
  static T Primary(const ailego::JsonValue &object) {
    return static_cast<T>(object.as_integer());
  }
};

//! Transform double of JsonValue to float
template <>
float Json2Primary::Primary<float>(const ailego::JsonValue &object);

//! Transform double of JsonValue
template <>
double Json2Primary::Primary<double>(const ailego::JsonValue &object);

template <typename T>
size_t Transformer::Transform(const ailego::JsonArray &array,
                              std::vector<T> *values) {
  for (auto it = array.begin(); it != array.end(); ++it) {
    values->emplace_back(Json2Primary::Primary<T>(*it));
  }
  return size_t(array.size());
}

template <typename T>
size_t Transformer::Transform(const ailego::JsonValue &matrix,
                              std::vector<T> *values) {
  try {
    auto &array = matrix.as_array();
    size_t size = 0;
    if (!array.empty() && array.begin()->is_array()) {
      for (auto it = array.begin(); it != array.end(); ++it) {
        size_t temp = Transformer::Transform(it->as_array(), values);
        if (temp < 0) {
          return temp;
        }
        size += temp;
      }
    } else {
      size = Transformer::Transform(matrix.as_array(), values);
    }
    return size;
  } catch (const std::exception &e) {
    return PROXIMA_BE_ERROR_CODE(InvalidVectorFormat);
  }
}

template <typename T>
size_t Transformer::Transform(
    const std::string &json,
    std::function<int(const ailego::JsonValue &)> *validator,
    std::vector<T> *values) {
  ailego::JsonValue node;
  if (!node.parse(json.c_str())) {
    LOG_ERROR("Parse index json value failed.");
    return PROXIMA_BE_ERROR_CODE(InvalidVectorFormat);
  }

  int error_code = !validator ? 0 : (*validator)(node);
  if (error_code != 0) {
    return size_t(error_code);
  }
  values->clear();
  return Transformer::Transform(node, values);
}

/*!
 * Serialize primary array into bytes
 */
struct Primary2Bytes {
  template <class T, DataTypes>
  static void Bytes(const std::vector<T> &values, std::string *bytes) {
    auto capacity = values.size() * sizeof(T);
    bytes->resize(capacity);
    std::memcpy(&((*bytes)[0]), values.data(), capacity);
  }
};

template <>
void Primary2Bytes::Bytes<int8_t, DataTypes::VECTOR_INT4>(
    const std::vector<int8_t> &values, std::string *bytes);

template <>
void Primary2Bytes::Bytes<float, DataTypes::VECTOR_FP16>(
    const std::vector<float> &values, std::string *bytes);

template <typename T, DataTypes DT>
size_t Transformer::Transform(const std::vector<T> &values,
                              std::string *bytes) {
  Primary2Bytes::Bytes<T, DT>(values, bytes);
  return values.size();
}

}  // namespace be
}  // namespace proxima
