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
 *   \date     Oct 2020
 *   \brief    Mysql Mysql interface definition for bilin engine
 */

#pragma once

#include "binlog_common.h"
#include "mysql_connector.h"
#include "mysql_reader.h"
#include "table_schema.h"

namespace proxima {
namespace be {
namespace repository {

class MysqlHandler;
using MysqlHandlerPtr = std::shared_ptr<MysqlHandler>;

/*! MysqlHandler
 */
class MysqlHandler {
 public:
  //! Constructor
  MysqlHandler(const CollectionConfig &config);

  //! Constructor
  MysqlHandler(const CollectionConfig &config, MysqlConnectorManagerPtr mgr);

  //! Destructor
  virtual ~MysqlHandler();

  //! Init Mysql Handler
  virtual int init(ScanMode mode);

  //! Init Mysql Handler
  virtual int start(const LsnContext &context);

  //! Get row data from binlog
  virtual int get_next_row_data(WriteRequest::Row *row_data,
                                LsnContext *context);

  //! Reset binlog reader status
  virtual int reset_status(ScanMode mode, const CollectionConfig &config,
                           const LsnContext &context);

  //! Get table snapshot
  virtual int get_table_snapshot(std::string *binlog_file, uint64_t *position);

  //! Get table snapshot
  virtual int get_fields_meta(WriteRequest::RowMeta *meta);

 private:
  //! Validate mysql information
  int validate_mysql();

 private:
  //! Table name
  std::string table_name_{};
  //! Collection name
  std::string collection_name_{};
  //! Collection config
  CollectionConfig collection_config_{};
  //! Mysql reader
  MysqlReaderPtr mysql_reader_{};
  //! Is already inited
  bool inited_{false};
  //! Mysql connector manager
  MysqlConnectorManagerPtr connector_mgr_{};
};

}  // namespace repository
}  // namespace be
}  // namespace proxima
