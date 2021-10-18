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
 *   \brief    Mysql validator interface definition for proxima search engine
 */

#pragma once

#include "mysql_connector.h"

namespace proxima {
namespace be {
namespace repository {

/*! Mysql Validator
 */
class MysqlValidator : public MysqlConnectorProxy {
 public:
  //! Constructor
  MysqlValidator(MysqlConnectorManagerPtr mgr) : MysqlConnectorProxy(mgr) {}

  //! Destructor
  ~MysqlValidator() = default;

  //! Init Mysql validator
  int init();

  //! Validate mysql version
  bool validate_version();

  //! Validate mysql row_mode
  bool validate_binlog_format();

  //! Validate database exist
  bool validate_database_exist();

 private:
  //! Mysql version separator
  static const std::string MYSQL_VERSION_SEPARATOR;
  //! Mysql major version
  static const std::string MYSQL_MAJOR_VERSION;
  //! Mysql minor version
  static const std::string MYSQL_MINOR_VERSION;
};

}  // end namespace repository
}  // namespace be
}  // end namespace proxima
