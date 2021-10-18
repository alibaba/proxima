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

#include "forward_serializer.h"
#include "common/error_code.h"
#include "common/logger.h"

namespace proxima {
namespace be {
namespace query {

bool ForwardSerializer::Deserialize(const std::string &buf,
                                    proto::GenericValueList *values) {
  return values->ParseFromString(buf);
}

int ForwardSerializer::FillForward(const index::QueryResult &forward,
                                   const ColumnNameList &columns,
                                   proto::Document *doc) {
  proto::GenericValueList values;
  ForwardSerializer::Deserialize(forward.forward_data, &values);

  if (static_cast<size_t>(values.values_size()) != columns.size()) {
    LOG_DEBUG("Mismatch forwards. buf_size[%lu], values[%d], forwards[%lu]",
              forward.forward_data.size(), values.values_size(),
              columns.size());
    return PROXIMA_BE_ERROR_CODE(MismatchedForward);
  }

  for (size_t pos = 0; pos < columns.size(); pos++) {
    proto::GenericKeyValue *kv = doc->add_forward_column_values();
    kv->set_key(columns[pos]);
    kv->mutable_value()->Swap(values.mutable_values(pos));
  }

  return 0;
}

}  // namespace query
}  // namespace be
}  // namespace proxima
