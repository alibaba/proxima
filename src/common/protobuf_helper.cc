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

 *   \author   jiliang.ljl
 *   \date     Feb 2021
 *   \brief    implementation of protobuf helper
 */

#include "protobuf_helper.h"
#include <ailego/encoding/base64.h>
#include <ailego/encoding/json.h>
#include "common/logger.h"

using google::protobuf::Descriptor;
using google::protobuf::EnumDescriptor;
using google::protobuf::EnumValueDescriptor;
using google::protobuf::FieldDescriptor;
using google::protobuf::Message;
using google::protobuf::Reflection;

namespace proxima {
namespace be {

#define RETURN_ON_NULL(ptr, msg) \
  if (!ptr) {                    \
    LOG_ERROR(msg);              \
    return false;                \
  }

static bool ParseMessage(const Message &msg, ailego::JsonValue *json,
                         const ProtobufHelper::PrintOptions &options);
static bool ParseJson(const ailego::JsonObject &json,
                      const ProtobufHelper::JsonParseOptions &options,
                      google::protobuf::Message *msg);

static bool FieldToJson(const Message &msg, const FieldDescriptor *field,
                        const ProtobufHelper::PrintOptions &options,
                        ailego::JsonValue *json) {
  const Reflection *ref = msg.GetReflection();
  bool repeated = field->is_repeated();

  ailego::JsonArray json_array;
  size_t array_size = 0;
  if (repeated) {
    array_size = ref->FieldSize(msg, field);
    json_array.reserve(array_size);
  }
  switch (field->cpp_type()) {
    case FieldDescriptor::CPPTYPE_DOUBLE:
      if (repeated) {
        for (size_t i = 0; i != array_size; ++i) {
          double value = ref->GetRepeatedDouble(msg, field, i);
          ailego::JsonValue v(value);
          json_array.push(v);
        }
      } else {
        *json = ref->GetDouble(msg, field);
      }
      break;
    case FieldDescriptor::CPPTYPE_FLOAT:
      if (repeated) {
        for (size_t i = 0; i != array_size; ++i) {
          float value = ref->GetRepeatedFloat(msg, field, i);
          ailego::JsonValue v(value);
          json_array.push(v);
        }
      } else {
        *json = ref->GetFloat(msg, field);
      }
      break;
    case FieldDescriptor::CPPTYPE_INT64:
      if (repeated) {
        for (size_t i = 0; i != array_size; ++i) {
          int64_t value = ref->GetRepeatedInt64(msg, field, i);
          ailego::JsonValue v(std::to_string(value));
          json_array.push(v);
        }
      } else {
        *json = std::to_string(ref->GetInt64(msg, field));
      }
      break;
    case FieldDescriptor::CPPTYPE_UINT64:
      if (repeated) {
        for (size_t i = 0; i != array_size; ++i) {
          uint64_t value = ref->GetRepeatedUInt64(msg, field, i);
          ailego::JsonValue v(std::to_string(value));
          json_array.push(v);
        }
      } else {
        *json = std::to_string(ref->GetUInt64(msg, field));
      }
      break;
    case FieldDescriptor::CPPTYPE_INT32:
      if (repeated) {
        for (size_t i = 0; i != array_size; ++i) {
          int32_t value = ref->GetRepeatedInt32(msg, field, i);
          ailego::JsonValue v(value);
          json_array.push(v);
        }
      } else {
        *json = ref->GetInt32(msg, field);
      }
      break;
    case FieldDescriptor::CPPTYPE_UINT32:
      if (repeated) {
        for (size_t i = 0; i != array_size; ++i) {
          uint32_t value = ref->GetRepeatedUInt32(msg, field, i);
          ailego::JsonValue v(value);
          json_array.push(v);
        }
      } else {
        *json = ref->GetUInt32(msg, field);
      }
      break;
    case FieldDescriptor::CPPTYPE_BOOL:
      if (repeated) {
        for (size_t i = 0; i != array_size; ++i) {
          bool value = ref->GetRepeatedBool(msg, field, i);
          ailego::JsonValue v(value);
          json_array.push(v);
        }
      } else {
        *json = ref->GetBool(msg, field);
      }
      break;
    case FieldDescriptor::CPPTYPE_STRING: {
      bool is_binary = field->type() == FieldDescriptor::TYPE_BYTES;
      if (repeated) {
        for (size_t i = 0; i != array_size; ++i) {
          std::string value = ref->GetRepeatedString(msg, field, i);
          if (is_binary) {
            value = ailego::Base64::Encode(value);
          }
          ailego::JsonValue v(ailego::JsonString(value).encode());
          json_array.push(v);
        }
      } else {
        std::string value = ref->GetString(msg, field);
        if (is_binary) {
          value = ailego::Base64::Encode(value);
        }
        *json = ailego::JsonString(value).encode();
      }
      break;
    }
    case FieldDescriptor::CPPTYPE_MESSAGE:
      if (repeated) {
        for (size_t i = 0; i != array_size; ++i) {
          const Message &value = ref->GetRepeatedMessage(msg, field, i);
          ailego::JsonValue v;
          bool ret = ParseMessage(value, &v, options);
          if (!ret) {
            return ret;
          }
          json_array.push(v);
        }
      } else {
        const Message &value = ref->GetMessage(msg, field);
        return ParseMessage(value, json, options);
      }
      break;
    case FieldDescriptor::CPPTYPE_ENUM:
      if (repeated) {
        for (size_t i = 0; i != array_size; ++i) {
          const EnumValueDescriptor *value =
              ref->GetRepeatedEnum(msg, field, i);
          ailego::JsonValue v(value->name());
          json_array.push(v);
        }
      } else {
        auto enum_desp = ref->GetEnum(msg, field);
        *json = enum_desp->name();
      }
      break;
    default:
      LOG_ERROR("Unexpected type: %d", static_cast<int>(field->cpp_type()));
      return false;
  }
  if (repeated) {
    *json = json_array;
  }
  return true;
}

#define RETURN_ON_INVALID_JSON(ERROR_MSG)                  \
  LOG_ERROR("Json is " ERROR_MSG ". json[%s] field[%s]",   \
            json.as_json_string().as_stl_string().c_str(), \
            field->full_name().c_str());                   \
  return false

static bool JsonToField(const ailego::JsonValue &json,
                        const ProtobufHelper::JsonParseOptions &options,
                        google::protobuf::Message *msg,
                        const FieldDescriptor *field) {
  const Reflection *ref = msg->GetReflection();
  bool repeated = field->is_repeated();
  switch (field->cpp_type()) {
#define CONVERT(type, ctype, checkfun, asfun, sfunc, afunc, error) \
  case FieldDescriptor::type: {                                    \
    if (!json.checkfun()) {                                        \
      RETURN_ON_INVALID_JSON(error);                               \
    }                                                              \
    if (repeated) {                                                \
      ref->afunc(msg, field, static_cast<ctype>(json.asfun()));    \
    } else {                                                       \
      ref->sfunc(msg, field, static_cast<ctype>(json.asfun()));    \
    }                                                              \
    break;                                                         \
  }

#define CONVERT_FLOAT(type, ctype, sfunc, afunc)                 \
  case FieldDescriptor::type: {                                  \
    ctype val = 0.0;                                             \
    if (json.is_integer()) {                                     \
      val = static_cast<ctype>(json.as_integer());               \
    } else if (json.is_float()) {                                \
      val = static_cast<ctype>(json.as_float());                 \
    } else if (json.is_string()) {                               \
      const char *value = json.as_c_string();                    \
      if (strncmp(value, "NaN", 3) == 0) {                       \
        val = std::numeric_limits<ctype>::quiet_NaN();           \
      } else if (strncmp(value, "Infinity", 8) == 0) {           \
        val = std::numeric_limits<ctype>::infinity();            \
      } else if (strncmp(value, "-Infinity", 9) == 0) {          \
        val = -std::numeric_limits<ctype>::infinity();           \
      } else {                                                   \
        RETURN_ON_INVALID_JSON(                                  \
            "float string only allow Nan, Infinity, -Infinity"); \
      }                                                          \
    } else {                                                     \
      RETURN_ON_INVALID_JSON("not integer, float or string");    \
    }                                                            \
    if (repeated) {                                              \
      ref->afunc(msg, field, val);                               \
    } else {                                                     \
      ref->sfunc(msg, field, val);                               \
    }                                                            \
    break;                                                       \
  }

#define CONVERT_LONG(type, ctype, sfunc, afunc, strtofun)                  \
  case FieldDescriptor::type: {                                            \
    ctype val = 0;                                                         \
    if (json.is_integer()) {                                               \
      val = static_cast<ctype>(json.as_integer());                         \
    } else if (json.is_string()) {                                         \
      val = static_cast<ctype>(strtofun(json.as_c_string(), nullptr, 10)); \
    } else {                                                               \
      RETURN_ON_INVALID_JSON("not integer or string");                     \
    }                                                                      \
    if (repeated) {                                                        \
      ref->afunc(msg, field, val);                                         \
    } else {                                                               \
      ref->sfunc(msg, field, val);                                         \
    }                                                                      \
    break;                                                                 \
  }


    CONVERT(CPPTYPE_INT32, int32_t, is_integer, as_integer, SetInt32, AddInt32,
            "not number");

    CONVERT(CPPTYPE_UINT32, uint32_t, is_integer, as_integer, SetUInt32,
            AddUInt32, "not number");

    CONVERT_FLOAT(CPPTYPE_FLOAT, float, SetFloat, AddFloat);

    CONVERT_FLOAT(CPPTYPE_DOUBLE, double, SetDouble, AddDouble);

    CONVERT(CPPTYPE_BOOL, bool, is_boolean, as_bool, SetBool, AddBool,
            "not bool");

    CONVERT_LONG(CPPTYPE_INT64, int64_t, SetInt64, AddInt64, strtoll);

    CONVERT_LONG(CPPTYPE_UINT64, uint64_t, SetUInt64, AddUInt64, strtoull);

    case FieldDescriptor::CPPTYPE_STRING: {
      if (!json.is_string()) {
        RETURN_ON_INVALID_JSON("not string");
      }
      const char *value = json.as_c_string();
      bool is_bytes = field->type() == FieldDescriptor::TYPE_BYTES;
      if (repeated) {
        ref->AddString(
            msg, field,
            is_bytes ? ailego::Base64::Decode(value) : json.as_stl_string());
      } else {
        ref->SetString(
            msg, field,
            is_bytes ? ailego::Base64::Decode(value) : json.as_stl_string());
      }
      break;
    }
    case FieldDescriptor::CPPTYPE_MESSAGE: {
      if (!json.is_object()) {
        RETURN_ON_INVALID_JSON("not object");
      }
      Message *mf = (repeated) ? ref->AddMessage(msg, field)
                               : ref->MutableMessage(msg, field);
      return ParseJson(json.as_object(), options, mf);
    }
    case FieldDescriptor::CPPTYPE_ENUM: {
      const EnumDescriptor *ed = field->enum_type();
      const EnumValueDescriptor *ev = 0;
      if (json.is_integer()) {
        ev = ed->FindValueByNumber(json.as_integer());
      } else if (json.is_string()) {
        ev = ed->FindValueByName(json.as_stl_string());
      } else {
        RETURN_ON_INVALID_JSON("not integer or string");
      }
      if (!ev) {
        RETURN_ON_INVALID_JSON("Enum value not found");
      }
      if (repeated) {
        ref->AddEnum(msg, field, ev);
      } else {
        ref->SetEnum(msg, field, ev);
      }
      break;
    }
    default:
      LOG_ERROR("Unexpected type: %d", static_cast<int>(field->cpp_type()));
      return false;
  }
  return true;
}

static bool ParseJson(const ailego::JsonObject &json,
                      const ProtobufHelper::JsonParseOptions &options,
                      google::protobuf::Message *msg) {
  const Descriptor *d = msg->GetDescriptor();
  const Reflection *ref = msg->GetReflection();
  RETURN_ON_NULL(d, "null descriptor");
  RETURN_ON_NULL(ref, "null reflection");
  for (auto it = json.cbegin(); it != json.cend(); ++it) {
    const char *name = it->key().c_str();
    const FieldDescriptor *field = d->FindFieldByName(name);
    if (!field) {
      field = d->FindFieldByCamelcaseName(name);
      if (!field) {
        field = ref->FindKnownExtensionByName(name);
      }
    }
    if (!field) {
      if (options.ignore_unknown_fields) {
        continue;
      }
      LOG_ERROR("Unknown field in protobuf. json key[%s] message[%s]", name,
                d->full_name().c_str());
      return false;
    }
    auto &value = it->value();

    if (value.is_null()) {
      ref->ClearField(msg, field);
      // no need check required as protobuf3 is all optional
      continue;
    }
    if (field->is_repeated()) {
      if (!value.is_array()) {
        LOG_ERROR("Input json is not array. key[%s] value[%s] field[%s]", name,
                  value.as_json_string().as_stl_string().c_str(),
                  field->full_name().c_str());
        return false;
      }
      ref->ClearField(msg, field);
      auto &array = value.as_array();
      for (auto ait = array.cbegin(); ait != array.cend(); ++ait) {
        if (!JsonToField(*ait, options, msg, field)) {
          return false;
        }
      }
    } else {
      if (!JsonToField(value, options, msg, field)) {
        return false;
      }
    }
  }
  return true;
}

static bool ParseMessage(const Message &msg, ailego::JsonValue *json,
                         const ProtobufHelper::PrintOptions &options) {
  const Descriptor *d = msg.GetDescriptor();
  RETURN_ON_NULL(d, "null descriptor")
  const Reflection *ref = msg.GetReflection();
  RETURN_ON_NULL(ref, "null reflection")
  size_t count = d->field_count();
  ailego::JsonObject obj;
  for (size_t i = 0; i != count; ++i) {
    const FieldDescriptor *field = d->field(i);
    RETURN_ON_NULL(field, "null field descriptor")
    if (field->is_optional() && !ref->HasField(msg, field)) {
      // do not print empty message
      if (field->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE) {
        continue;
      }
      if (!options.always_print_primitive_fields) {
        continue;
      }
    }
    auto oneof_des = field->containing_oneof();
    if (oneof_des) {
      bool has_oneof = false;
      for (int fi = 0; fi < oneof_des->field_count(); fi++) {
        if (ref->HasField(msg, oneof_des->field(fi))) {
          has_oneof = true;
          break;
        }
      }
      if (!has_oneof || !ref->HasField(msg, field)) {
        continue;
      }
    }
    ailego::JsonValue field_json;
    bool ret = FieldToJson(msg, field, options, &field_json);
    if (!ret) {
      return ret;
    }
    ailego::JsonString field_name(field->name());
    obj.set(field_name.encode(), field_json);
  }
  *json = obj;
  return true;
}

bool ProtobufHelper::MessageToJson(const google::protobuf::Message &message,
                                   const ProtobufHelper::PrintOptions &options,
                                   std::string *json) {
  ailego::JsonValue root;
  bool ret = ParseMessage(message, &root, options);
  if (!ret) {
    return ret;
  }
  *json = root.as_json_string().as_stl_string();
  return true;
}

bool ProtobufHelper::JsonToMessage(
    const std::string &json, const ProtobufHelper::JsonParseOptions &options,
    google::protobuf::Message *message) {
  ailego::JsonValue node;
  if (!node.parse(json.c_str())) {
    LOG_ERROR("Parse json value failed. json[%s]", json.c_str());
    return false;
  }
  if (!node.is_object()) {
    LOG_ERROR("Json is not object. json[%s]", json.c_str());
    return false;
  }
  return ParseJson(node.as_object(), options, message);
}

#undef RETURN_ON_NULL
#undef RETURN_ON_INVALID_JSON
#undef CONVERT
#undef CONVERT_FLOAT
#undef CONVERT_LONG

}  // namespace be
}  // namespace proxima
