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

 *   \author   Haichao.chc
 *   \date     Jun 2021
 *   \brief    ForwardData describes the format of forward data
 */


#pragma once

#include <string>
#include "../constants.h"

namespace proxima {
namespace be {
namespace index {

/*
 * ForwardData packed some meta info with forward data.
 * It provides serialize and deserilize ability.
 */
struct ForwardData {
 public:
  /*
   * ForwardHeader contains some header fields.
   */
  struct ForwardHeader {
    uint64_t primary_key{0U};
    uint64_t timestamp{0U};
    uint32_t revision{0U};
    uint64_t lsn{0U};

    ForwardHeader() {
      primary_key = INVALID_KEY;
    }
  };

  //! Serialize to bytes string
  void serialize(std::string *buf) const {
    buf->clear();
    buf->append((const char *)&header, sizeof(ForwardHeader));
    buf->append(data);
  }

  //! Deserialize from bytes string
  void deserialize(const std::string &buf) {
    header = *((ForwardHeader *)buf.data());
    data.assign(buf.data() + sizeof(ForwardHeader),
                buf.size() - sizeof(ForwardHeader));
  }

  ForwardHeader header{};
  std::string data{};
};


}  // end namespace index
}  // namespace be
}  // end namespace proxima
