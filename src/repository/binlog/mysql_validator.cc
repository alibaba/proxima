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
 *   \brief    Mysql validator interface implementation for proxima search
 *             engine
 */

#include "mysql_validator.h"
#include <ailego/utility/string_helper.h>
#include "repository_common/error_code.h"
#include "repository_common/logger.h"
#include "sql_builder.h"

namespace proxima {
namespace be {
namespace repository {

const std::string MysqlValidator::MYSQL_VERSION_SEPARATOR = ".";
const std::string MysqlValidator::MYSQL_MAJOR_VERSION = "5";
const std::string MysqlValidator::MYSQL_MINOR_VERSION = "7";

int MysqlValidator::init() {
  int ret = init_connector();
  if (ret != 0) {
    LOG_ERROR("Mysql connector proxy init failed.");
    return ret;
  }
  return 0;
}

bool MysqlValidator::validate_version() {
  std::string sql = SqlBuilder::BuildSelectVersionSql();
  MysqlResultWrapperPtr result;
  int ret = connector_->execute_query(sql, &result, true);
  if (ret != 0) {
    LOG_ERROR("Execute select version sql failed. sql[%s]", sql.c_str());
    return false;
  }

  uint32_t rows_num = result->rows_num();
  if (rows_num != 1) {
    LOG_ERROR("Mysql result rows num mismatched. rows[%u]", rows_num);
    return false;
  }

  unsigned int fields_num = result->fields_num();
  if (fields_num != 1) {
    LOG_ERROR("Result fields num mismatched. fields[%u]", fields_num);
    return false;
  }
  MysqlRow *row = result->next();
  if (!row) {
    LOG_ERROR("Get next version result failed.");
    return false;
  }
  std::string version(row->field_value(0));
  std::vector<std::string> array;
  ailego::StringHelper::Split(version, MYSQL_VERSION_SEPARATOR, &array);
  if (array.size() < 2) {
    LOG_ERROR("Invalid mysql version. version[%s]", version.c_str());
    return false;
  }

  // Current only support mysql version 5.7
  if (array[0] == MYSQL_MAJOR_VERSION && array[1] == MYSQL_MINOR_VERSION) {
    return true;
  } else {
    LOG_ERROR("Current only support Mysql 5.7. version[%s]", version.c_str());
    return false;
  }
}

bool MysqlValidator::validate_binlog_format() {
  std::string sql = SqlBuilder::BuildShowBinlogFormat();
  MysqlResultWrapperPtr result;
  int ret = connector_->execute_query(sql, &result, true);
  if (ret != 0) {
    LOG_ERROR("Execute show binlog format sql failed. sql[%s]", sql.c_str());
    return false;
  }

  uint32_t rows_num = result->rows_num();
  if (rows_num != 1) {
    LOG_ERROR("Show binlog format result rows mismatched. rows[%u]", rows_num);
    return false;
  }

  unsigned int fields_num = result->fields_num();
  if (fields_num != 2) {
    LOG_ERROR("Binlog format result fields num mismatched. num[%u]",
              fields_num);
    return false;
  }

  MysqlRow *row = result->next();
  if (!row) {
    LOG_ERROR("Get next show binlog format result failed.");
    return false;
  }

  std::string value(row->field_value(1));
  if (value == "ROW") {
    return true;
  } else {
    LOG_ERROR("Only support ROW mysql binlog format. format[%s]",
              value.c_str());
    return false;
  }
}

bool MysqlValidator::validate_database_exist() {
  auto &uri = connector_->uri();
  if (uri.path().empty()) {
    return false;
  }
  std::string db = uri.path().substr(1);
  std::string sql = SqlBuilder::BuildSelectDbSql(db);
  MysqlResultWrapperPtr result;
  int ret = connector_->execute_query(sql, &result, true);
  if (ret != 0) {
    LOG_ERROR("Execute select db sql failed. sql[%s]", sql.c_str());
    return false;
  }

  uint32_t rows_num = result->rows_num();
  if (rows_num != 1) {
    LOG_ERROR("Select db sql rows num mismatched. sql[%s] rows[%u]",
              sql.c_str(), rows_num);
    return false;
  }

  return true;
}

}  // end namespace repository
}  // namespace be
}  // end namespace proxima
