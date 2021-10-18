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
 *   \brief    Sql builder interface definition for proxima search engine
 */

#pragma once

#include <sstream>
#include <string>
#include <vector>

namespace proxima {
namespace be {
namespace repository {

/*! Sql Builder
 */
class SqlBuilder {
 public:
  //! Build scan table sql
  static std::string BuildScanTableSql(
      const std::string &database, const std::string &table,
      const std::string &auto_inc_field,
      const std::vector<std::string> &select_fields, uint64_t seq_id);

  //! Build get table schema sql
  static std::string BuildGetSchemaSql(const std::string &database,
                                       const std::string &table);

  //! Build lock table sql
  static std::string BuildLockTableSql(const std::string &database,
                                       const std::string &table);

  //! Build select events sql
  static std::string BuildSelectEventsSql(const std::string &file_name,
                                          uint64_t position);

  //! Build show full columns sql
  static std::string BuildShowFullColumnsSql(const std::string &database,
                                             const std::string &table);

  //! Build show binary logs
  static const std::string &BuildShowBinaryLogsSql();

  //! Build select database sql
  static std::string BuildSelectDbSql(const std::string &db);

  //! Build unlock tables sql
  static const std::string &BuildUnlockTablesSql();

  //! Build unlock tables sql
  static const std::string &BuildSelectVersionSql();

  //! Build unlock tables sql
  static const std::string &BuildShowBinlogFormat();

  //! Build show master status sql
  static const std::string &BuildShowMasterStatus();

  //! Build set checksum sql
  static const std::string &BuildTurnoffCheckSumSql();

 private:
  static const std::string SHOW_BINARY_LOGS_SQL;
  static const std::string UNLOCK_TABLE_SQL;
  static const std::string SELECT_VERSION_SQL;
  static const std::string SHOW_BINLOG_FORMAT_SQL;
  static const std::string SHOW_MASTER_STATUS_SQL;
  static const std::string TURN_OFF_CHECKSUM_SQL;
};

}  // end namespace repository
}  // namespace be
}  // end namespace proxima
