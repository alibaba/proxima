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
 *   \brief    Table schema interface definition for proxima search engine
 */

#pragma once

#include <vector>
#include "proto/common.pb.h"
#include "binlog_common.h"
#include "field.h"

namespace proxima {
namespace be {
namespace repository {

class TableSchema;
using TableSchemaPtr = std::shared_ptr<TableSchema>;

/*! Table Schema
 */
class TableSchema {
 public:
  //! Constructor
  TableSchema() = default;

  //! Destructor
  ~TableSchema() = default;

  //! Get fields
  const std::vector<FieldPtr> &fields() const {
    return fields_;
  }

  //! Get field
  const FieldPtr &fields(size_t id) const {
    return fields_[id];
  }

  //! Get selected fields
  const std::vector<FieldPtr> &selected_fields() const {
    return selected_fields_;
  }

  //! Get selected forward ids
  const std::vector<size_t> &selected_forward_ids() const {
    return selected_forward_ids_;
  }

  //! Get selected index ids
  const std::vector<size_t> &selected_index_ids() const {
    return selected_index_ids_;
  }

  //! Get auto increment id
  size_t auto_increment_id() const {
    return auto_increment_id_;
  }

  //! Get auto increment field
  const FieldPtr auto_increment_field() const {
    return fields_[auto_increment_id_];
  }

  //! Add field
  void add_field(FieldPtr field) {
    fields_.emplace_back(std::move(field));
  }

  //! Add selected field
  void add_selected_field(FieldPtr field) {
    selected_fields_.emplace_back(std::move(field));
  }

  //! Add selected forward id
  void add_selected_forward_id(size_t id) {
    selected_forward_ids_.emplace_back(id);
  }

  //! Add selected index ids
  void add_selected_index_id(size_t id) {
    selected_index_ids_.emplace_back(id);
  }

  //! Set auto increment id
  void set_auto_increment_id(size_t id) {
    auto_increment_id_ = id;
  }

  //! Get selected fields meta
  void fill_fields_meta(proto::WriteRequest::RowMeta *meta);

  //! Set max index id
  void set_max_index_id(uint32_t max_id) {
    max_index_id_ = max_id;
  }

  //! Get max index id
  uint32_t max_index_id() {
    return max_index_id_;
  }

 private:
  //! Max index id
  uint32_t max_index_id_{0};
  //! Table all fields
  std::vector<FieldPtr> fields_{};
  //! Selected fields
  std::vector<FieldPtr> selected_fields_{};
  //! Selected forward ids
  std::vector<size_t> selected_forward_ids_{};
  //! Selected index ids
  std::vector<size_t> selected_index_ids_{};
  //! Auto increment field id
  size_t auto_increment_id_{0};
};

}  // end namespace repository
}  // namespace be
}  // end namespace proxima
