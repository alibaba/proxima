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
 *   \brief    Table reader interface implementation for proxima search engine
 */

#include "table_reader.h"
#include "repository_common/error_code.h"
#include "repository_common/logger.h"
#include "binlog_common.h"
#include "sql_builder.h"

namespace proxima {
namespace be {
namespace repository {

TableReader::TableReader(const std::string &table_name,
                         InfoFetcherPtr info_fetcher,
                         MysqlConnectorManagerPtr mgr)
    : MysqlConnectorProxy(std::move(mgr)),
      table_name_(table_name),
      info_fetcher_(std::move(info_fetcher)) {}

int TableReader::init() {
  RLOG_INFO("Begin init TableReader.");

  int ret = init_connector();
  if (ret != 0) {
    LOG_ERROR("Mysql connector proxy init failed.");
    return ret;
  }

  ret = info_fetcher_->get_table_schema(table_name_, &table_schema_);
  if (ret != 0) {
    RLOG_ERROR("Info fetcher get table schema failed.");
    return ret;
  }

  RLOG_INFO("Table reader init success.");
  return 0;
}

int TableReader::start(const LsnContext &context) {
  RLOG_INFO("Begin start TableReader. seq_id[%zu]", (size_t)context.seq_id);
  int ret = prepare_reader(context.seq_id);
  if (ret != 0) {
    RLOG_ERROR("Prepare table reader failed.");
    return ret;
  }

  sequence_id_ = context.seq_id;

  RLOG_INFO("End start TableReader success.");

  return 0;
}

int TableReader::get_next_row_data(WriteRequest::Row *row_data,
                                   LsnContext *context) {
  if (need_reconnect_) {
    if (connector_->reconnect() && prepare_reader(sequence_id_) == 0) {
      need_reconnect_ = false;
    } else {
      return ErrorCode_ConnectMysql;
    }
  }
  MysqlRow *row = result_wrappeer_->next();
  if (!row) {
    if (!result_wrappeer_->has_error()) {
      context->status = RowDataStatus::NO_MORE_DATA;
      return 0;
    } else {
      need_reconnect_ = true;
      return ErrorCode_FetchMysqlResult;
    }
  }
  build_row_data(row, row_data, context);

  context->status = RowDataStatus::NORMAL;
  sequence_id_ = context->seq_id;

  // Debug
  RLOG_INFO("primary_key: %zu", (size_t)row_data->primary_key());

  return 0;
}

int TableReader::build_row_data(MysqlRow *row, WriteRequest::Row *row_data,
                                LsnContext *context) {
  // set operation type
  row_data->set_operation_type(proto::OP_INSERT);

  const std::vector<FieldPtr> &fields = table_schema_->selected_fields();
  uint32_t max_index_id = table_schema_->max_index_id();
  unsigned int fields_num = result_wrappeer_->fields_num();
  for (unsigned int i = 1; i < fields_num; ++i) {
    GenericValue *forward_value = nullptr;
    if (i - 1 >= max_index_id && fields[i - 1]->is_forward()) {
      forward_value = row_data->mutable_forward_column_values()->add_values();
      fields[i - 1]->unpack_text(row->field_value(i), row->field_length(i),
                                 forward_value);
    }

    if (i - 1 < max_index_id && fields[i - 1]->is_index()) {
      GenericValue *index_value =
          row_data->mutable_index_column_values()->add_values();
      fields[i - 1]->unpack_text(row->field_value(i), row->field_length(i),
                                 index_value);
    }
  }

  // process primary key
  GenericValue value;
  const FieldPtr &auto_increment_field = table_schema_->auto_increment_field();
  auto_increment_field->unpack_text(row->field_value(0), row->field_length(0),
                                    &value);
  uint64_t auto_increment_id = get_auto_increment_id(value);
  if (auto_increment_id == INVALID_PRIMARY_KEY) {
    RLOG_ERROR("Get auto increment id failed.");
    return ErrorCode_RuntimeError;
  }
  row_data->set_primary_key(auto_increment_id);

  // process auto increment field
  context->seq_id = auto_increment_id;

  return 0;
}

int TableReader::prepare_reader(uint64_t seq_id) {
  // Get select fields
  std::vector<std::string> selected_fields;
  const std::vector<FieldPtr> &fields = table_schema_->selected_fields();
  for (auto it = fields.begin(); it != fields.end(); ++it) {
    selected_fields.emplace_back((*it)->select_field());
  }

  // Build scan table sql
  const std::string &auto_inc_field =
      table_schema_->auto_increment_field()->field_name();
  const std::string &db = info_fetcher_->database();
  std::string select_sql = SqlBuilder::BuildScanTableSql(
      db, table_name_, auto_inc_field, selected_fields, seq_id);

  // Execute select sql
  int ret = connector_->execute_query(select_sql, &result_wrappeer_, false);
  if (ret != 0) {
    RLOG_ERROR("Connector execute query failed. sql[%s]", select_sql.c_str());
    return ret;
  }

  return 0;
}

uint64_t TableReader::get_auto_increment_id(const GenericValue &value) {
  switch (value.value_oneof_case()) {
    case GenericValue::kInt32Value:
      return (uint64_t)value.int32_value();
    case GenericValue::kInt64Value:
      return (uint64_t)value.int64_value();
    case GenericValue::kUint32Value:
      return (uint64_t)value.uint32_value();
    case GenericValue::kUint64Value:
      return (uint64_t)value.uint64_value();
    default:
      RLOG_ERROR("Unsupported auto_increment data type %d.",
                 value.value_oneof_case());
      return INVALID_PRIMARY_KEY;
  }
}

}  // end namespace repository
}  // namespace be
}  // end namespace proxima
