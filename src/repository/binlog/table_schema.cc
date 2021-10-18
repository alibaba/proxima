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
 *   \date     Nov 2020
 *   \brief    Table schema interface implementation for proxima search engine
 */

#include "table_schema.h"

namespace proxima {
namespace be {
namespace repository {

void TableSchema::fill_fields_meta(proto::WriteRequest::RowMeta *meta) {
  // fill index tuples
  for (auto id : selected_index_ids_) {
    auto *index_meta = meta->add_index_column_metas();
    index_meta->set_column_name(fields_[id]->field_name());
  }

  // fill forward tuples
  for (auto id : selected_forward_ids_) {
    auto *forward_name = meta->add_forward_column_names();
    *forward_name = fields_[id]->field_name();
  }
}

}  // end namespace repository
}  // namespace be
}  // end namespace proxima
