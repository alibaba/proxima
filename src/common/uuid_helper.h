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

#pragma once

#include <string>

namespace proxima {
namespace be {

//! Generate uuid(represent by 32 hex numbers)
//! param sep: seperator between groups, 0 means do not split into groups
std::string gen_uuid(const std::string &sep = "");

//! Check uuid
bool valid_uuid(const std::string &uuid);

//! Generate hex string, length indicated by param len
std::string generate_hex(uint32_t len);

}  // namespace be
}  // namespace proxima
