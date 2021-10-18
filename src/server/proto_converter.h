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

 *   \author   Hongqing.hu
 *   \date     Oct 2020
 *   \brief    Proto converter interface definition for bilin engine
 */

#pragma once

#include <ailego/encoding/json.h>
#include <ailego/utility/float_helper.h>
#include "common/error_code.h"
#include "common/logger.h"
#include "common/transformer.h"
#include "index/collection_dataset.h"
#include "meta/meta.h"
#include "proto/proxima_be.pb.h"

namespace proxima {
namespace be {
namespace server {

/*! ProtoConverter
 */
class ProtoConverter {
 public:
  //! Convert Index Data
  static int ConvertIndexData(
      const std::string &index_value, const meta::ColumnMeta &column_meta,
      const proto::WriteRequest::IndexColumnMeta &proto_meta, bool is_bytes,
      index::ColumnData *column_value);

 private:
  //! Parse single index column value
  static int ParseJsonIndexColumnValue(
      const std::string &column_value, const meta::ColumnMeta &meta,
      const proto::WriteRequest::IndexColumnMeta &proto_meta,
      std::string *serialized_value);

  //! Parse single index column value
  static int ParseBytesIndexColumnValue(
      const std::string &column_value, const meta::ColumnMeta &meta,
      const proto::WriteRequest::IndexColumnMeta &proto_meta,
      std::string *serialized_value);

  //! Copy bytes index column value
  static int CopyBytesIndexColumnValue(const std::string &column_value,
                                       const meta::ColumnMeta &meta,
                                       std::string *serialized_value);

  //! Parse single typed index column value
  template <typename T>
  static int ParseTypedIndexColumnValue(const std::string &column_value,
                                        const meta::ColumnMeta &meta,
                                        std::string *serialized_value);

  //! Parse single typed index column value
  static int SetTypedIndexColumnValue(const std::string &column_value,
                                      std::string *serialized_value);

  //! Validate single typed index column value
  template <typename T>
  static int ValidateTypedIndexColumnValue(const std::string &column_value,
                                           const meta::ColumnMeta &meta);
};

template <typename T>
int ProtoConverter::ParseTypedIndexColumnValue(const std::string &column_value,
                                               const meta::ColumnMeta &meta,
                                               std::string *serialized_value) {
  std::vector<T> values;
  DataTypes data_type = meta.data_type();
  uint32_t dimension = meta.dimension();
  Transformer::Transform(column_value, nullptr, &values);
  if (data_type == DataTypes::VECTOR_BINARY32) {
    dimension /= 32;
  } else if (data_type == DataTypes::VECTOR_BINARY64) {
    dimension /= 64;
  }
  if (values.size() != dimension) {
    LOG_ERROR("Vector dimension mismatched. expected[%u], actual[%zu]",
              dimension, values.size());
    return ErrorCode_MismatchedDimension;
  }


  if (data_type == DataTypes::VECTOR_INT4) {
    Primary2Bytes::Bytes<T, DataTypes::VECTOR_INT4>(values, serialized_value);
  } else if (data_type == DataTypes::VECTOR_FP16) {
    Primary2Bytes::Bytes<T, DataTypes::VECTOR_FP16>(values, serialized_value);
  } else {
    size_t vector_size = dimension * sizeof(T);
    serialized_value->resize(vector_size);
    memcpy(&((*serialized_value)[0]), values.data(), vector_size);
  }

  return 0;
}

template <typename T>
int ProtoConverter::ValidateTypedIndexColumnValue(
    const std::string &column_value, const meta::ColumnMeta &meta) {
  DataTypes data_type = meta.data_type();
  uint32_t dimension = meta.dimension();

  if (data_type == DataTypes::VECTOR_BINARY32) {
    dimension /= 32;
  } else if (data_type == DataTypes::VECTOR_BINARY64) {
    dimension /= 64;
  }

  size_t except_size = 0;
  if (data_type == DataTypes::VECTOR_INT4 ||
      data_type == DataTypes::VECTOR_FP16) {
    except_size = dimension * sizeof(T) / 2;
  } else {
    except_size = dimension * sizeof(T);
  }

  if (except_size != column_value.size()) {
    LOG_ERROR("Vector size mismatched. expected[%zu], actual[%zu]", except_size,
              column_value.size());
    return ErrorCode_MismatchedDimension;
  }

  return 0;
}

}  // end namespace server
}  // namespace be
}  // end namespace proxima
