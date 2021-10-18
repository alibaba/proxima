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
 *   \brief    Columns order interface definition for proxima search engine
 */

#pragma once

#include <map>
#include <mutex>
#include "meta/meta.h"

namespace proxima {
namespace be {
namespace agent {

class ColumnOrder;
class ColumnOrderMap;

using ColumnOrderPtr = std::shared_ptr<ColumnOrder>;
using ColumnOrderMapPtr = std::shared_ptr<ColumnOrderMap>;

/*! Columns order
 */
class ColumnOrder {
 public:
  //! Constructor
  ColumnOrder() = default;

  //! Destructor
  ~ColumnOrder() = default;

  //! Init columns order
  void init_column_order(const meta::CollectionMeta &meta);

  //! Get forward order
  const std::map<std::string, size_t> &get_forward_order() const {
    return forward_order_;
  }

  //! Get index order
  const std::map<std::string, size_t> &get_index_order() const {
    return index_order_;
  }

 protected:
  //! Init columns order
  void init_column_order_impl(const meta::CollectionMeta &meta);

 private:
  std::map<std::string, size_t> forward_order_;
  std::map<std::string, size_t> index_order_;
};

/*! Columns order Map
 */
class ColumnOrderMap {
 public:
  //! Constructor
  ColumnOrderMap() = default;

  //! Destructor
  ~ColumnOrderMap() = default;

  //! Add columns order
  void add_column_order(const meta::CollectionMeta &meta);

  //! Update columns order
  void update_column_order(const meta::CollectionMeta &meta);

  //! Remove columns order
  void remove_column_order(const std::string &name);

  //! Get collection columns order
  ColumnOrderPtr get_column_order(const std::string &name);

 private:
  //! Mutex
  std::mutex mutex_{};
  //! Columns order map
  std::map<std::string, ColumnOrderPtr> column_order_map_{};
};

}  // end namespace agent
}  // namespace be
}  // end namespace proxima
