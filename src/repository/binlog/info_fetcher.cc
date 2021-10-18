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
 *   \brief    Table info fetcher interface implementation for proxima search
 *             engine
 */

#include "info_fetcher.h"
#include <ailego/utility/string_helper.h>
#include "repository_common/error_code.h"
#include "repository_common/logger.h"
#include "sql_builder.h"

namespace proxima {
namespace be {
namespace repository {

InfoFetcher::InfoFetcher(const CollectionConfig &config,
                         MysqlConnectorManagerPtr mgr)
    : MysqlConnectorProxy(mgr), collection_config_(config) {}

int InfoFetcher::init() {
  int ret = init_connector();
  if (ret != 0) {
    LOG_ERROR("Mysql connector proxy init failed.");
    return ret;
  }

  const std::string &path = connector_->uri().path();
  if (path.size() <= 1) {
    LOG_ERROR("Invalid uri path. path[%s]", path.c_str());
    return ErrorCode_InvalidCollectionConfig;
  }
  database_ = path.substr(1);

  ret = build_selected_fields(collection_config_);
  if (ret != 0) {
    LOG_ERROR("Build selected fields failed.");
    return ret;
  }

  return 0;
}

int InfoFetcher::get_table_schema(const std::string &table_name,
                                  TableSchemaPtr *schema) {
  // get collation info
  std::map<std::string, std::string> field_collation;
  int ret = get_collation_info(table_name, field_collation);
  if (ret != 0) {
    LOG_ERROR("Fill collation info failed. code[%d]", ret);
    return ret;
  }

  std::string select_sql = SqlBuilder::BuildGetSchemaSql(database_, table_name);
  MysqlResultWrapperPtr result;
  ret = connector_->execute_query(select_sql, &result, true);
  if (ret != 0) {
    LOG_ERROR("Execute get schema sql failed. sql[%s].", select_sql.c_str());
    return ret;
  }

  ret = parse_table_schema(result, field_collation, schema);
  if (ret != 0) {
    LOG_ERROR("Parse table schema failed.");
    return ret;
  }

  return 0;
}

int InfoFetcher::get_table_snapshot(const std::string &table_name,
                                    std::string *file_name,
                                    uint64_t *position) {
  int ret = lock_table(table_name);
  if (ret != 0) {
    LOG_ERROR("Lock table failed. table[%s]", table_name.c_str());
    return ret;
  }
  ret = get_table_snapshot_internal(file_name, position);
  if (ret != 0) {
    LOG_ERROR("Get table snapshot internal failed. table[%s]",
              table_name.c_str());
  }
  unlock_table();

  return ret;
}

int InfoFetcher::build_selected_fields(const CollectionConfig &config) {
  selected_fields_ = std::make_shared<SelectedFields>();
  for (int i = 0; i < config.index_column_params_size(); ++i) {
    selected_fields_->add_field(config.index_column_params(i).column_name());
    selected_fields_->add_index_field(
        config.index_column_params(i).column_name());
  }
  for (int i = 0; i < config.forward_column_names_size(); ++i) {
    selected_fields_->add_field(config.forward_column_names(i));
    selected_fields_->add_forward_field(config.forward_column_names(i));
  }

  return 0;
}

int InfoFetcher::get_table_snapshot_internal(std::string *file_name,
                                             uint64_t *position) {
  std::string sql = SqlBuilder::BuildShowMasterStatus();
  MysqlResultWrapperPtr result;
  int ret = connector_->execute_query(sql, &result, true);
  if (ret != 0) {
    LOG_ERROR("Connector execute show master status sql failed. sql[%s]",
              sql.c_str());
    return ErrorCode_ExecuteMysql;
  }

  unsigned int rows_num = result->rows_num();
  if (rows_num != 1) {
    LOG_ERROR("Master status result rows mismatched. rows[%u]", rows_num);
    return ErrorCode_InvalidMysqlResult;
  }
  MysqlRow *row = result->next();
  if (!row) {
    LOG_ERROR("Fetch next result failed.");
    return ErrorCode_InvalidMysqlResult;
  }

  unsigned int fields_num = result->fields_num();
  if (fields_num != 5) {
    LOG_ERROR("Mysql result fields num mismatched. num[%u]", fields_num);
    return ErrorCode_InvalidMysqlResult;
  }

  std::map<std::string, std::string> kv;
  for (unsigned int i = 0; i < fields_num; ++i) {
    kv[result->field_meta(i)->name()] =
        std::string(row->field_value(i), row->field_length(i));
  }
  auto file_it = kv.find("File");
  auto pos_it = kv.find("Position");
  if (file_it == kv.end() || pos_it == kv.end()) {
    LOG_ERROR("Find position or file field in result failed.");
    return ErrorCode_InvalidMysqlResult;
  }

  *file_name = file_it->second;
  ailego::StringHelper::ToUint64(pos_it->second, position);

  return 0;
}

int InfoFetcher::lock_table(const std::string &table_name) {
  std::string sql = SqlBuilder::BuildLockTableSql(database_, table_name);
  int ret = connector_->execute_query(sql, nullptr, true);
  if (ret != 0) {
    LOG_ERROR("Execute lock table sql failed. code[%d] sql[%s]", ret,
              sql.c_str());
    return ErrorCode_ExecuteMysql;
  }

  return 0;
}

int InfoFetcher::unlock_table() {
  std::string sql = SqlBuilder::BuildUnlockTablesSql();
  int ret = connector_->execute_query(sql, nullptr, true);
  if (ret != 0) {
    LOG_ERROR("Unlock tables failed. sql[%s] code[%d]", sql.c_str(), ret);
    return ErrorCode_ExecuteMysql;
  }

  return 0;
}

int InfoFetcher::parse_table_schema(
    const MysqlResultWrapperPtr &result,
    const std::map<std::string, std::string> &collation,
    TableSchemaPtr *result_schema) {
  TableSchemaPtr table_schema = std::make_shared<TableSchema>();
  uint32_t fields_num = result->fields_num();
  for (uint32_t i = 0; i < fields_num; ++i) {
    auto &field_meta = result->field_meta(i);
    const std::string &field_name = field_meta->name();
    auto it = collation.find(field_name);
    if (it == collation.end()) {
      LOG_ERROR("Find field collation failed. field[%s]", field_name.c_str());
      return ErrorCode_InvalidMysqlResult;
    }
    FieldAttr attr(selected_fields_->is_index(field_name),
                   selected_fields_->is_forward(field_name), it->second,
                   field_meta);
    FieldPtr field = FieldFactory::Create(field_name, attr);
    if (!field) {
      LOG_ERROR("Create field failed. field_name[%s]", field_name.c_str());
      return ErrorCode_RuntimeError;
    }
    if (field->is_auto_increment()) {
      table_schema->set_auto_increment_id(table_schema->fields().size());
    }
    table_schema->add_field(field);
  }

  // fill index select fields
  int ret = fill_selected_fields(&table_schema);
  if (ret != 0) {
    LOG_ERROR("Fill selected fields failed. code[%d] reason[%s]", ret,
              ErrorCode::What(ret));
    return ret;
  }
  *result_schema = table_schema;

  return 0;
}

int InfoFetcher::get_collation_info(
    const std::string &table_name,
    std::map<std::string, std::string> &field_collation) {
  std::string sql = SqlBuilder::BuildShowFullColumnsSql(database_, table_name);
  MysqlResultWrapperPtr result;
  int ret = connector_->execute_query(sql, &result, true);
  if (ret != 0) {
    LOG_ERROR("Execute show full columns sql failed. code[%d] sql[%s]", ret,
              sql.c_str());
    return ErrorCode_ExecuteMysql;
  }

  auto &name_meta = result->field_meta(0);
  auto &collation_meta = result->field_meta(2);
  if (name_meta->name() != "Field" || collation_meta->name() != "Collation") {
    LOG_ERROR("Invalid full columns result. field1[%s] field2[%s]",
              name_meta->name().c_str(), collation_meta->name().c_str());
    return ErrorCode_InvalidMysqlResult;
  }

  // get collation kv map
  MysqlRow *row = result->next();
  while (row) {
    std::string key = std::string(row->field_value(0), row->field_length(0));
    std::string value = std::string(row->field_value(2), row->field_length(2));
    field_collation[key] = value;
    row = result->next();
  }

  return 0;
}

int InfoFetcher::fill_selected_fields(TableSchemaPtr *table_schema) {
  auto &index_fields = selected_fields_->index_fields();
  int ret = fill_index_fields(index_fields, table_schema);
  if (ret != 0) {
    LOG_ERROR("Fill index fields failed.");
    return ret;
  }

  (*table_schema)->set_max_index_id(index_fields.size());

  ret = fill_forward_fields(selected_fields_->forward_fields(), table_schema);
  if (ret != 0) {
    LOG_ERROR("Fill forward fields failed.");
    return ret;
  }

  return 0;
}

int InfoFetcher::fill_index_fields(
    const std::vector<std::string> &selected_fields,
    TableSchemaPtr *table_schema) {
  auto &all_fields = (*table_schema)->fields();
  for (auto &field_name : selected_fields) {
    auto it = all_fields.begin();
    for (; it != all_fields.end(); ++it) {
      if (ailego::StringHelper::CompareIgnoreCase(field_name,
                                                  (*it)->field_name())) {
        break;
      }
    }
    if (it == all_fields.end()) {
      LOG_ERROR("Invalid table field. field_name:[%s]", field_name.c_str());
      return ErrorCode_InvalidCollectionConfig;
    }
    (*table_schema)->add_selected_field(*it);
    size_t idx = it - all_fields.begin();
    (*table_schema)->add_selected_index_id(idx);
  }

  return 0;
}

int InfoFetcher::fill_forward_fields(
    const std::vector<std::string> &selected_fields,
    TableSchemaPtr *table_schema) {
  auto &all_fields = (*table_schema)->fields();
  for (auto &field_name : selected_fields) {
    auto it = all_fields.begin();
    for (; it != all_fields.end(); ++it) {
      if (ailego::StringHelper::CompareIgnoreCase(field_name,
                                                  (*it)->field_name())) {
        break;
      }
    }
    if (it == all_fields.end()) {
      LOG_ERROR("Invalid table field. field_name:[%s]", field_name.c_str());
      return ErrorCode_InvalidCollectionConfig;
    }
    (*table_schema)->add_selected_field(*it);
    size_t idx = it - all_fields.begin();
    (*table_schema)->add_selected_forward_id(idx);
  }

  return 0;
}

}  // end namespace repository
}  // namespace be
}  // end namespace proxima
