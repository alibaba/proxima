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
 *   \brief    Proto converter interface implementation for bilin engine
 */

#include "proto_converter.h"
#include "common/transformer.h"
#include "common/types_helper.h"

namespace proxima {
namespace be {
namespace server {

int ProtoConverter::ConvertIndexData(
    const std::string &index_value, const meta::ColumnMeta &column_meta,
    const proto::WriteRequest::IndexColumnMeta &proto_meta, bool is_bytes,
    index::ColumnData *column_data) {
  IndexTypes index_type = column_meta.index_type();
  if (index_type == IndexTypes::PROXIMA_GRAPH_INDEX) {
    column_data->column_name = column_meta.name();
    column_data->data_type = column_meta.data_type();
    column_data->dimension = column_meta.dimension();
    if (is_bytes) {
      int ret = ParseBytesIndexColumnValue(index_value, column_meta, proto_meta,
                                           &(column_data->data));
      if (ret != 0) {
        LOG_ERROR("Set index column value failed. column[%s].",
                  column_meta.name().c_str());
        return ret;
      }
    } else {
      int ret = ParseJsonIndexColumnValue(index_value, column_meta, proto_meta,
                                          &(column_data->data));
      if (ret != 0) {
        LOG_ERROR("Parse index column value failed. column[%s].",
                  column_meta.name().c_str());
        return ret;
      }
    }
  } else {
    LOG_ERROR("Invalid index type %u.", (uint32_t)index_type);
    return ErrorCode_InvalidIndexType;
  }

  return 0;
}

int ProtoConverter::ParseBytesIndexColumnValue(
    const std::string &column_value, const meta::ColumnMeta &meta,
    const proto::WriteRequest::IndexColumnMeta &proto_meta,
    std::string *serialized_value) {
  DataTypes in_data_type = DataTypeCodeBook::Get(proto_meta.data_type());
  if (Transformer::NeedTransform(in_data_type, meta.data_type())) {
    std::string dst_column_value;
    int ret = Transformer::Transform(in_data_type, column_value,
                                     meta.data_type(), &dst_column_value);
    if (ret != 0) {
      LOG_ERROR("Transform vector failed. in[%d] out[%d]", (int)in_data_type,
                (int)meta.data_type());
      return ret;
    }
    return CopyBytesIndexColumnValue(dst_column_value, meta, serialized_value);
  } else {
    return CopyBytesIndexColumnValue(column_value, meta, serialized_value);
  }
}

int ProtoConverter::CopyBytesIndexColumnValue(const std::string &column_value,
                                              const meta::ColumnMeta &meta,
                                              std::string *serialized_value) {
  int ret = 0;
  switch (meta.data_type()) {
    case DataTypes::VECTOR_FP32:
      ret = ValidateTypedIndexColumnValue<float>(column_value, meta);
      break;
    case DataTypes::VECTOR_FP16:
      ret = ValidateTypedIndexColumnValue<float>(column_value, meta);
      break;
    case DataTypes::VECTOR_INT16:
      ret = ValidateTypedIndexColumnValue<int16_t>(column_value, meta);
      break;
    case DataTypes::VECTOR_INT8:
      ret = ValidateTypedIndexColumnValue<int8_t>(column_value, meta);
      break;
    case DataTypes::VECTOR_INT4:
      ret = ValidateTypedIndexColumnValue<int8_t>(column_value, meta);
      break;
    case DataTypes::VECTOR_BINARY32:
      ret = ValidateTypedIndexColumnValue<uint32_t>(column_value, meta);
      break;
    case DataTypes::VECTOR_BINARY64:
      ret = ValidateTypedIndexColumnValue<uint64_t>(column_value, meta);
      break;
    default:
      LOG_ERROR("Invalid data type %u.", (uint32_t)meta.data_type());
      return ErrorCode_InvalidDataType;
  }

  if (ret != 0) {
    LOG_ERROR("Index value is invalid");
    return ret;
  }

  *serialized_value = column_value;

  return 0;
}

int ProtoConverter::ParseJsonIndexColumnValue(
    const std::string &column_value, const meta::ColumnMeta &meta,
    const proto::WriteRequest::IndexColumnMeta &proto_meta,
    std::string *serialized_value) {
  DataTypes src_data_type = DataTypeCodeBook::Get(proto_meta.data_type());
  int ret = Transformer::SupportTransform(src_data_type, meta.data_type());
  if (ret != 0) {
    LOG_ERROR("Not support current transform. src[%d] dst[%d]",
              (int)src_data_type, (int)meta.data_type());
    return ret;
  }
  switch (src_data_type) {
    case DataTypes::VECTOR_FP32:
      ret = ParseTypedIndexColumnValue<float>(column_value, meta,
                                              serialized_value);
      break;
    case DataTypes::VECTOR_FP16:
      ret = ParseTypedIndexColumnValue<float>(column_value, meta,
                                              serialized_value);
      break;
    case DataTypes::VECTOR_INT16:
      ret = ParseTypedIndexColumnValue<int16_t>(column_value, meta,
                                                serialized_value);
      break;
    case DataTypes::VECTOR_INT8:
      ret = ParseTypedIndexColumnValue<int8_t>(column_value, meta,
                                               serialized_value);
      break;
    case DataTypes::VECTOR_INT4:
      ret = ParseTypedIndexColumnValue<int8_t>(column_value, meta,
                                               serialized_value);
      break;
    case DataTypes::VECTOR_BINARY32:
      ret = ParseTypedIndexColumnValue<uint32_t>(column_value, meta,
                                                 serialized_value);
      break;
    case DataTypes::VECTOR_BINARY64:
      ret = ParseTypedIndexColumnValue<uint64_t>(column_value, meta,
                                                 serialized_value);
      break;
    default:
      LOG_ERROR("Unsupported data type %u.", (uint32_t)src_data_type);
      return ErrorCode_InvalidDataType;
  }

  return ret;
}

}  // end namespace server
}  // namespace be
}  // end namespace proxima
