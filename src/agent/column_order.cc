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

 *   \author   Dianzhang.Chen
 *   \date     Feb 2021
 *   \brief    Columns order implements for proxima search engine
 */

#include "column_order.h"

namespace proxima {
namespace be {
namespace agent {

void ColumnOrder::init_column_order(const meta::CollectionMeta &meta) {
  auto &forward_columns = meta.forward_columns();
  for (size_t i = 0; i < forward_columns.size(); i++) {
    forward_order_[forward_columns[i]] = i;
  }

  auto index_columns = meta.index_columns();
  for (size_t i = 0; i < index_columns.size(); i++) {
    index_order_[index_columns[i]->name()] = i;
  }
}

void ColumnOrderMap::add_column_order(const meta::CollectionMeta &meta) {
  ColumnOrderPtr column_order = std::make_shared<ColumnOrder>();
  column_order->init_column_order(meta);

  auto &name = meta.name();
  std::lock_guard<std::mutex> lock(mutex_);
  column_order_map_[name] = column_order;
}

void ColumnOrderMap::update_column_order(const meta::CollectionMeta &meta) {
  ColumnOrderPtr column_order = std::make_shared<ColumnOrder>();
  column_order->init_column_order(meta);

  auto &name = meta.name();
  std::lock_guard<std::mutex> lock(mutex_);
  column_order_map_[name] = column_order;
}

void ColumnOrderMap::remove_column_order(const std::string &name) {
  std::lock_guard<std::mutex> lock(mutex_);
  column_order_map_.erase(name);
}

ColumnOrderPtr ColumnOrderMap::get_column_order(const std::string &name) {
  std::lock_guard<std::mutex> lock(mutex_);
  auto it = column_order_map_.find(name);
  if (it != column_order_map_.end()) {
    return it->second;
  }
  return nullptr;
}

}  // end namespace agent
}  // namespace be
}  // end namespace proxima
