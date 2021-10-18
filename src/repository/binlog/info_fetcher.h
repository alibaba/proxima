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
 *   \brief    Table info fetcher interface definition for proxima search engine
 */

#pragma once

#include "mysql_connector.h"
#include "table_schema.h"

namespace proxima {
namespace be {
namespace repository {

class SelectedFields;
class InfoFetcher;

using SelectedFieldsPtr = std::shared_ptr<SelectedFields>;
using InfoFetcherPtr = std::shared_ptr<InfoFetcher>;

/*! Selected Fields
 */
class SelectedFields {
 public:
  //! Constructor
  SelectedFields() = default;

  //! Destructor
  ~SelectedFields() = default;

  //! Get selected fields
  const std::vector<std::string> &fields() const {
    return fields_;
  }

  //! Get index fields
  const std::vector<std::string> &index_fields() const {
    return index_fields_;
  }

  //! Get forward fields
  const std::vector<std::string> &forward_fields() const {
    return forward_fields_;
  }

  //! Add selected field
  void add_field(const std::string &field) {
    fields_.emplace_back(field);
  }

  //! Add selected index field
  void add_index_field(const std::string &field) {
    index_fields_.emplace_back(field);
  }

  //! Add selected forward field
  void add_forward_field(const std::string &field) {
    forward_fields_.emplace_back(field);
  }

  //! Is index field
  bool is_index(const std::string &field_name) {
    auto it = std::find(index_fields_.begin(), index_fields_.end(), field_name);
    return it != index_fields_.end();
  }

  //! Is forward field
  bool is_forward(const std::string &field_name) {
    auto it =
        std::find(forward_fields_.begin(), forward_fields_.end(), field_name);
    return it != forward_fields_.end();
  }

  //! Is selected field
  bool is_selected(const std::string &field_name) {
    auto it = std::find(fields_.begin(), fields_.end(), field_name);
    return it != fields_.end();
  }

 private:
  //! All selected fields
  std::vector<std::string> fields_;
  //! Selected index fields
  std::vector<std::string> index_fields_;
  //! Selected forward fields
  std::vector<std::string> forward_fields_;
};

/*! Info Fetcher
 */
class InfoFetcher : public MysqlConnectorProxy {
 public:
  //! Constructor
  InfoFetcher(const CollectionConfig &config, MysqlConnectorManagerPtr mgr);

  //! Destructor
  ~InfoFetcher() = default;

  //! Init info fetcher
  int init();

  //! Get table schema
  int get_table_schema(const std::string &table_name, TableSchemaPtr *schema);

  //! Get table Snapshot
  int get_table_snapshot(const std::string &table_name, std::string *file_name,
                         uint64_t *position);

  //! Get database name
  const std::string &database() const {
    return database_;
  }

 private:
  //! Build selected fields
  int build_selected_fields(const CollectionConfig &config);

  //! Get table snapshot internal
  int get_table_snapshot_internal(std::string *file_name, uint64_t *position);

  //! Lock read table
  int lock_table(const std::string &table_name);

  //! Unlock read table
  int unlock_table();

  //! Parse mysql table schema result
  int parse_table_schema(const MysqlResultWrapperPtr &result,
                         const std::map<std::string, std::string> &collation,
                         TableSchemaPtr *result_schema);

  //! Get collation info for fields
  int get_collation_info(const std::string &table_name,
                         std::map<std::string, std::string> &collation);

  //! Fill selected fields
  int fill_selected_fields(TableSchemaPtr *table_schema);

  //! Fill index fields
  int fill_index_fields(const std::vector<std::string> &selected_fields,
                        TableSchemaPtr *table_schema);

  //! Fill forward fields
  int fill_forward_fields(const std::vector<std::string> &selected_fields,
                          TableSchemaPtr *table_schema);

 private:
  //! Database
  std::string database_{};
  //! Collection config
  CollectionConfig collection_config_{};
  //! Selected fields
  SelectedFieldsPtr selected_fields_{};
};

}  // end namespace repository
}  // namespace be
}  // end namespace proxima
