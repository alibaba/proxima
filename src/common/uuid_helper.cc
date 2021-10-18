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
 *   \date     Nov 2020
 *   \brief
 */

#include "uuid_helper.h"
#include <iostream>
#include <random>
#include <sstream>
#include <ailego/utility/string_helper.h>

namespace proxima {
namespace be {

static uint32_t random_char() {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dis(0, 255);
  return dis(gen);
}

std::string gen_uuid(const std::string &sep) {
  return ailego::StringHelper::Concat(generate_hex(4), sep, generate_hex(2),
                                      sep, generate_hex(2), sep,
                                      generate_hex(8));
}

bool valid_uuid(const std::string &uuid) {
  return uuid.length() == 32;
}

std::string generate_hex(const uint32_t len) {
  std::stringstream ss;
  for (uint32_t i = 0; i < len; i++) {
    const auto rc = random_char();
    std::stringstream stream;
    stream << std::hex << rc;
    auto hex = stream.str();
    ss << (hex.length() < 2 ? '0' + hex : hex);
  }
  return ss.str();
}

}  // namespace be
}  // namespace proxima