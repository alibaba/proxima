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
 *   \brief    protobuf helper
 */

#pragma once

#include <string>

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
#include <google/protobuf/message.h>
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

namespace proxima {
namespace be {

//! See data type mapping:
//! https://developers.google.com/protocol-buffers/docs/proto3#json
class ProtobufHelper {
 public:
  struct PrintOptions {
    // Whether to always print primitive fields.
    bool always_print_primitive_fields{true};
  };

  struct JsonParseOptions {
    // Whether to ignore unknown JSON fields during parsing
    bool ignore_unknown_fields{false};
  };

  //! Perform json serialization.
  static bool MessageToJson(const google::protobuf::Message &message,
                            std::string *json) {
    return MessageToJson(message, PrintOptions(), json);
  }

  //! Perform json serialization.
  static bool MessageToJson(const google::protobuf::Message &message,
                            const PrintOptions &options, std::string *json);

  //! Perform json deserialization.
  static bool JsonToMessage(const std::string &json,
                            google::protobuf::Message *message) {
    return JsonToMessage(json, JsonParseOptions{}, message);
  }

  //! Perform json deserialization.
  static bool JsonToMessage(const std::string &json,
                            const JsonParseOptions &options,
                            google::protobuf::Message *message);
};

}  // namespace be
}  // namespace proxima
