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

 *   \author   DianZhang.Chen
 *   \date     Oct 2020
 *   \brief    Mysql Mysql interface definition for bilin engine
 */

#pragma once

#include <gmock/gmock.h>
#include "repository/binlog/mysql_handler.h"

using namespace proxima::be::repository;
using namespace ::testing;

namespace proxima {
namespace be {
namespace repository {

class MockMysqlHandler;
using MockMysqlHandlerPtr = std::shared_ptr<MockMysqlHandler>;

/*! Mysql Handler
 */
class MockMysqlHandler : public MysqlHandler {
 public:
  //! Constructor
  MockMysqlHandler(const CollectionConfig &config) : MysqlHandler(config){};

  //! Destructor
  virtual ~MockMysqlHandler() = default;

  //! Init Mysql Handler
  MOCK_METHOD1(init, int(ScanMode mode));

  //! Start Mysql Handler
  MOCK_METHOD1(start, int(const LsnContext &context));

  //! Reset binlog reader status
  MOCK_METHOD3(reset_status, int(ScanMode mode, const CollectionConfig &config,
                                 const LsnContext &context));

  //! Get fields meta
  MOCK_METHOD2(get_fields_meta, int(GenericValueMetaList *index_tuples,
                                    GenericValueMetaList *forward_tuples));

  //! Get row data from binlog
  MOCK_METHOD2(get_next_row_data,
               int(WriteRequest::Row *row_data, LsnContext *context));

  //! Get fields meta
  MOCK_METHOD1(get_fields_meta, int(WriteRequest::RowMeta *meta));

  //! Get table snapshot
  MOCK_METHOD2(get_table_snapshot,
               int(std::string *binlog_file, uint64_t *position));
};

}  // namespace repository
}  // namespace be
}  // namespace proxima
