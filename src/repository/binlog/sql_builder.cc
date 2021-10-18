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
 *   \brief    Sql builder interface implementation for proxima search engine
 */

#include "sql_builder.h"

namespace proxima {
namespace be {
namespace repository {

const std::string SqlBuilder::SHOW_BINARY_LOGS_SQL("SHOW BINARY LOGS");
const std::string SqlBuilder::UNLOCK_TABLE_SQL("UNLOCK TABLES");
const std::string SqlBuilder::SELECT_VERSION_SQL("SELECT VERSION()");
const std::string SqlBuilder::SHOW_BINLOG_FORMAT_SQL(
    "SHOW GLOBAL VARIABLES LIKE 'binlog_format'");
const std::string SqlBuilder::SHOW_MASTER_STATUS_SQL("SHOW MASTER STATUS");
const std::string SqlBuilder::TURN_OFF_CHECKSUM_SQL(
    "SET @master_binlog_checksum='NONE'");

std::string SqlBuilder::BuildScanTableSql(
    const std::string &database, const std::string &table,
    const std::string &auto_inc_field,
    const std::vector<std::string> &select_fields, uint64_t seq_id) {
  std::stringstream sql_format;
  sql_format << "SELECT " << auto_inc_field;
  for (size_t i = 0; i < select_fields.size(); ++i) {
    sql_format << ", " << select_fields[i];
  }
  sql_format << " FROM " << database << "." << table << " WHERE "
             << auto_inc_field << " > " << seq_id;
  return sql_format.str();
}

std::string SqlBuilder::BuildGetSchemaSql(const std::string &database,
                                          const std::string &table) {
  std::stringstream sql_format;
  sql_format << "SELECT * FROM " << database << "." << table << " LIMIT 0";
  return sql_format.str();
}

std::string SqlBuilder::BuildLockTableSql(const std::string &database,
                                          const std::string &table) {
  std::stringstream sql_format;
  sql_format << "LOCK TABLE " << database << "." << table << " READ";
  return sql_format.str();
}

std::string SqlBuilder::BuildSelectEventsSql(const std::string &file_name,
                                             uint64_t position) {
  std::stringstream sql_format;
  sql_format << "SHOW BINLOG EVENTS IN '" << file_name << "' from " << position
             << " LIMIT 1";
  return sql_format.str();
}

std::string SqlBuilder::BuildShowFullColumnsSql(const std::string &database,
                                                const std::string &table) {
  std::stringstream sql_format;
  sql_format << "SHOW FULL COLUMNS FROM " << table << " IN " << database;
  return sql_format.str();
}

const std::string &SqlBuilder::BuildShowBinaryLogsSql() {
  return SHOW_BINARY_LOGS_SQL;
}

const std::string &SqlBuilder::BuildUnlockTablesSql() {
  return UNLOCK_TABLE_SQL;
}

const std::string &SqlBuilder::BuildSelectVersionSql() {
  return SELECT_VERSION_SQL;
}

const std::string &SqlBuilder::BuildShowBinlogFormat() {
  return SHOW_BINLOG_FORMAT_SQL;
}

const std::string &SqlBuilder::BuildShowMasterStatus() {
  return SHOW_MASTER_STATUS_SQL;
}

const std::string &SqlBuilder::BuildTurnoffCheckSumSql() {
  return TURN_OFF_CHECKSUM_SQL;
}

std::string SqlBuilder::BuildSelectDbSql(const std::string &db) {
  std::stringstream sql_format;
  sql_format << "SELECT * FROM INFORMATION_SCHEMA.SCHEMATA WHERE SCHEMA_NAME='"
             << db << "'";
  return sql_format.str();
}

}  // end namespace repository
}  // namespace be
}  // end namespace proxima
