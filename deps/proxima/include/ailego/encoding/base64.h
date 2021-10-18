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

 *   \author   Jiliang.ljl
 *   \date     Mar 2021
 *   \brief    Interface of Base64 encode & decode
 */

#ifndef __AILEGO_ENCODING_BASE64_H__
#define __AILEGO_ENCODING_BASE64_H__

#include <cstring>
#include <string>

namespace ailego {

/*! Base64 encode & decode
 */
struct Base64 {
  //! Encode the bytes in base64 encoding
  static size_t Encode(const uint8_t *src, size_t len, uint8_t *dst);

  //! Decode the bytes with base64 encoding
  static size_t Decode(const uint8_t *src, size_t len, uint8_t *dst);

  //! Encode the bytes in base64 encoding
  static std::string Encode(const uint8_t *src, size_t len) {
    std::string out;
    out.resize((len + 2) / 3 * 4);
    Encode(src, len, (uint8_t *)out.data());
    return out;
  }

  //! Encode a stl-string in base64 encoding
  static std::string Encode(const std::string &src) {
    return Encode(reinterpret_cast<const uint8_t *>(src.c_str()), src.size());
  }

  //! Encode a c-string in base64 encoding
  static std::string Encode(const char *src) {
    return Encode(reinterpret_cast<const uint8_t *>(src), std::strlen(src));
  }

  //! Decode the bytes with base64 encoding
  static std::string Decode(const uint8_t *src, size_t len) {
    std::string out;
    out.resize((len + 3) / 4 * 3);
    out.resize(Decode(src, len, (uint8_t *)out.data()));
    return out;
  }

  //! Decode a c-string with base64 encoding
  static std::string Decode(const char *src) {
    return Decode(reinterpret_cast<const uint8_t *>(src), std::strlen(src));
  }

  //! Decode a stl-string with base64 encoding
  static std::string Decode(const std::string &src) {
    return Decode(reinterpret_cast<const uint8_t *>(src.c_str()), src.size());
  }
};

}  // namespace ailego

#endif  // __AILEGO_ENCODING_BASE64_H__
