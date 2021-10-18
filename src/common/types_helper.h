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

#pragma once

#include "proto/common.pb.h"
#include "error_code.h"
#include "logger.h"
#include "types.h"

namespace proxima {
namespace be {

struct DataTypeCodeBook {
  static DataTypes Get(proto::DataType type) {
    DataTypes data_types = DataTypes::UNDEFINED;
    switch (type) {
      case proto::DataType::DT_BINARY:
        data_types = DataTypes::BINARY;
        break;
      case proto::DataType::DT_STRING:
        data_types = DataTypes::STRING;
        break;
      case proto::DataType::DT_BOOL:
        data_types = DataTypes::BOOL;
        break;
      case proto::DataType::DT_INT32:
        data_types = DataTypes::INT32;
        break;
      case proto::DataType::DT_INT64:
        data_types = DataTypes::INT64;
        break;
      case proto::DataType::DT_UINT32:
        data_types = DataTypes::UINT32;
        break;
      case proto::DataType::DT_UINT64:
        data_types = DataTypes::UINT64;
        break;
      case proto::DataType::DT_FLOAT:
        data_types = DataTypes::FLOAT;
        break;
      case proto::DataType::DT_DOUBLE:
        data_types = DataTypes::DOUBLE;
        break;
      case proto::DataType::DT_VECTOR_BINARY32:
        data_types = DataTypes::VECTOR_BINARY32;
        break;
      case proto::DataType::DT_VECTOR_BINARY64:
        data_types = DataTypes::VECTOR_BINARY64;
        break;
      case proto::DataType::DT_VECTOR_FP16:
        data_types = DataTypes::VECTOR_FP16;
        break;
      case proto::DataType::DT_VECTOR_FP32:
        data_types = DataTypes::VECTOR_FP32;
        break;
      case proto::DataType::DT_VECTOR_FP64:
        data_types = DataTypes::VECTOR_FP64;
        break;
      case proto::DataType::DT_VECTOR_INT4:
        data_types = DataTypes::VECTOR_INT4;
        break;
      case proto::DataType::DT_VECTOR_INT8:
        data_types = DataTypes::VECTOR_INT8;
        break;
      case proto::DataType::DT_VECTOR_INT16:
        data_types = DataTypes::VECTOR_INT16;
      default:
        break;
    }
    return data_types;
  }

  static proto::DataType Get(DataTypes type) {
    proto::DataType data_type = proto::DataType::DT_UNDEFINED;
    switch (type) {
      case DataTypes::BINARY:
        data_type = proto::DataType::DT_BINARY;
        break;
      case DataTypes::STRING:
        data_type = proto::DataType::DT_STRING;
        break;
      case DataTypes::BOOL:
        data_type = proto::DataType::DT_BOOL;
        break;
      case DataTypes::INT32:
        data_type = proto::DataType::DT_INT32;
        break;
      case DataTypes::INT64:
        data_type = proto::DataType::DT_INT64;
        break;
      case DataTypes::UINT32:
        data_type = proto::DataType::DT_UINT32;
        break;
      case DataTypes::UINT64:
        data_type = proto::DataType::DT_UINT64;
        break;
      case DataTypes::FLOAT:
        data_type = proto::DataType::DT_FLOAT;
        break;
      case DataTypes::DOUBLE:
        data_type = proto::DataType::DT_DOUBLE;
        break;
      case DataTypes::VECTOR_BINARY32:
        data_type = proto::DataType::DT_VECTOR_BINARY32;
        break;
      case DataTypes::VECTOR_BINARY64:
        data_type = proto::DataType::DT_VECTOR_BINARY64;
        break;
      case DataTypes::VECTOR_FP16:
        data_type = proto::DataType::DT_VECTOR_FP16;
        break;
      case DataTypes::VECTOR_FP32:
        data_type = proto::DataType::DT_VECTOR_FP32;
        break;
      case DataTypes::VECTOR_FP64:
        data_type = proto::DataType::DT_VECTOR_FP64;
        break;
      case DataTypes::VECTOR_INT4:
        data_type = proto::DataType::DT_VECTOR_INT4;
        break;
      case DataTypes::VECTOR_INT8:
        data_type = proto::DataType::DT_VECTOR_INT8;
        break;
      case DataTypes::VECTOR_INT16:
        data_type = proto::DataType::DT_VECTOR_INT16;
      default:
        break;
    }

    return data_type;
  }
};

struct IndexParamsHelper {
  static int Append(const proto::GenericKeyValue &kv,
                    aitheta2::IndexParams *params) {
    if (!kv.IsInitialized()) {
      return PROXIMA_BE_ERROR_CODE(InvalidArgument);
    }
    switch (kv.value().value_oneof_case()) {
      case proto::GenericValue::ValueOneofCase::kStringValue:
        params->insert<char *>(
            kv.key(), const_cast<char *>(kv.value().string_value().c_str()));
        break;
      case proto::GenericValue::ValueOneofCase::kBoolValue:
        params->insert<bool>(kv.key(), kv.value().bool_value());
        break;
      case proto::GenericValue::ValueOneofCase::kInt32Value:
        params->insert<int32_t>(kv.key(), kv.value().int32_value());
        break;
      case proto::GenericValue::ValueOneofCase::kInt64Value:
        params->insert<int64_t>(kv.key(), kv.value().int64_value());
        break;
      case proto::GenericValue::ValueOneofCase::kUint32Value:
        params->insert<uint32_t>(kv.key(), kv.value().uint32_value());
        break;
      case proto::GenericValue::ValueOneofCase::kUint64Value:
        params->insert<uint64_t>(kv.key(), kv.value().uint64_value());
        break;
      case proto::GenericValue::ValueOneofCase::kFloatValue:
        params->insert<float>(kv.key(), kv.value().float_value());
        break;
      case proto::GenericValue::ValueOneofCase::kDoubleValue:
        params->insert<double>(kv.key(), kv.value().double_value());
        break;
      default:
        LOG_DEBUG("None value or kBytes has been set, skip this");
        break;
    }
    return 0;
  }

  static int Append(const proto::KeyValuePair &kv,
                    aitheta2::IndexParams *params) {
    if (!kv.IsInitialized()) {
      return PROXIMA_BE_ERROR_CODE(InvalidArgument);
    }
    params->set(kv.key(), kv.value());
    return 0;
  }

  static int SerializeToParams(
      const google::protobuf::RepeatedPtrField<be::proto::KeyValuePair> &maps,
      aitheta2::IndexParams *params) {
    int count = maps.size();
    while (count--) {
      Append(maps.at(count), params);
    }
    return 0;
  }

  static int SerializeToPB(
      const aitheta2::IndexParams &params,
      google::protobuf::RepeatedPtrField<be::proto::KeyValuePair> *maps) {
    for (const auto &it : params.hypercube().cubes()) {
      auto *kv = maps->Add();
      kv->set_key(it.first);
      kv->set_value(it.second.cast<std::string>());
    }
    return 0;
  }
};

//! Code book for OperationTypes
struct OperationTypesCodeBook {
  static OperationTypes Get(proto::OperationType type) {
    if (type == proto::OP_INSERT) {
      return OperationTypes::INSERT;
    } else if (type == proto::OP_UPDATE) {
      return OperationTypes::UPDATE;
    } else {
      return OperationTypes::DELETE;
    }
  }
};

}  // namespace be
}  // namespace proxima
