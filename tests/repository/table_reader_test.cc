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
 *
 *   \author   hongqing.hu
 *   \date     Dec 2020
 *   \brief
 */

#include <gtest/gtest.h>

#define private public
#include "repository/binlog/table_reader.h"
#include "mock_mysql_connector.h"
#include "mysql_result_builder.h"
#undef private

#include "repository/repository_common/error_code.h"

using namespace ::proxima::be;
using namespace proxima::be::repository;

class TableReaderTest : public testing::Test {
 protected:
  void SetUp() {
    mgr_ = std::make_shared<MysqlConnectorManager>();
    ASSERT_TRUE(mgr_);
    connector1_ = std::make_shared<MockMysqlConnector>();
    ASSERT_TRUE(connector1_);
    mgr_->put(connector1_);
    connector2_ = std::make_shared<MockMysqlConnector>();
    ASSERT_TRUE(connector2_);
    mgr_->put(connector2_);
    table_name_ = builder_.table_name_;
    ctx_.seq_id = 1;
    InitFetcher();
  }

  void TearDown() {}

  void InitFetcher() {
    builder_.BuildCollectionConfig();

    // init
    ailego::Uri test_uri = builder_.uri_;
    EXPECT_CALL(*connector1_, uri())
        .WillOnce(
            Invoke([&test_uri]() -> const ailego::Uri & { return test_uri; }))
        .RetiresOnSaturation();
    fetcher_ = std::make_shared<InfoFetcher>(builder_.config_, mgr_);
    int ret = fetcher_->init();
    ASSERT_EQ(ret, 0);
    ASSERT_EQ(fetcher_->database(), "mytest");
  }

 protected:
  MysqlConnectorManagerPtr mgr_{};
  MockMysqlConnectorPtr connector1_{};
  MockMysqlConnectorPtr connector2_{};
  std::string table_name_{};
  InfoFetcherPtr fetcher_{};
  LsnContext ctx_;
  MysqlResultBuilder builder_{};
};

TEST_F(TableReaderTest, TestGeneral) {
  TableReader reader(table_name_, fetcher_, mgr_);

  // build schema result
  MockMysqlResultWrapperPtr schema_result = builder_.BuildQuerySchemaResult();
  MockMysqlResultWrapperPtr result1 = builder_.BuildQueryCollationResult();
  EXPECT_CALL(*connector1_, execute_query(_, _, _))
      .Times(2)
      .WillOnce(Invoke([&result1](const std::string &,
                                  MysqlResultWrapperPtr *out, bool) -> int {
        *out = result1;
        return 0;
      }))
      .WillOnce(
          Invoke([&schema_result](const std::string &,
                                  MysqlResultWrapperPtr *out, bool) -> int {
            *out = schema_result;
            return 0;
          }))
      .RetiresOnSaturation();
  // init table reader
  int ret = reader.init();
  ASSERT_EQ(ret, 0);

  // build scan table result
  MockMysqlResultWrapperPtr scan_result = builder_.BuildScanTableResult();
  EXPECT_CALL(*connector2_, execute_query(_, _, _))
      .WillOnce(Invoke([&scan_result](const std::string &,
                                      MysqlResultWrapperPtr *out, bool) -> int {
        *out = scan_result;
        return 0;
      }))
      .RetiresOnSaturation();
  ret = reader.start(ctx_);
  ASSERT_EQ(ret, 0);

  // get next row data
  WriteRequest::Row row_data;
  LsnContext ctx;
  ret = reader.get_next_row_data(&row_data, &ctx);
  ASSERT_EQ(ret, 0);
  ASSERT_EQ(ctx.seq_id, (uint64_t)1);
  ASSERT_EQ(ctx.status, RowDataStatus::NORMAL);
  ASSERT_EQ(row_data.primary_key(), (uint64_t)1);
  ASSERT_EQ(row_data.operation_type(), ::proxima::be::proto::OP_INSERT);
  ASSERT_EQ(row_data.forward_column_values().values(0).string_value(), "name1");
  ASSERT_EQ(row_data.forward_column_values().values(1).int32_value(), 18);
  ASSERT_EQ(row_data.index_column_values().values(0).string_value(), "1,2,3,4");
  ASSERT_EQ(row_data.index_column_values().values(1).string_value(), "1,2,3,5");

  row_data.Clear();
  ret = reader.get_next_row_data(&row_data, &ctx);
  ASSERT_EQ(ret, 0);
  ASSERT_EQ(ctx.status, RowDataStatus::NORMAL);
  ASSERT_EQ(ctx.seq_id, (uint64_t)2);
  ASSERT_EQ(row_data.primary_key(), (uint64_t)2);
  ASSERT_EQ(row_data.operation_type(), ::proxima::be::proto::OP_INSERT);
  ASSERT_EQ(row_data.forward_column_values().values(0).string_value(), "name2");
  ASSERT_EQ(row_data.forward_column_values().values(1).int32_value(), 19);
  ASSERT_EQ(row_data.index_column_values().values(0).string_value(), "2,2,3,4");
  ASSERT_EQ(row_data.index_column_values().values(1).string_value(), "2,2,3,5");

  row_data.Clear();
  ret = reader.get_next_row_data(&row_data, &ctx);
  ASSERT_EQ(ret, 0);
  ASSERT_EQ(ctx.status, RowDataStatus::NO_MORE_DATA);
}

TEST_F(TableReaderTest, TestInitSuccess) {
  TableReader reader(table_name_, fetcher_, mgr_);

  // build schema result
  MockMysqlResultWrapperPtr schema_result = builder_.BuildQuerySchemaResult();
  MockMysqlResultWrapperPtr result1 = builder_.BuildQueryCollationResult();
  EXPECT_CALL(*connector1_, execute_query(_, _, _))
      .Times(2)
      .WillOnce(Invoke([&result1](const std::string &,
                                  MysqlResultWrapperPtr *out, bool) -> int {
        *out = result1;
        return 0;
      }))
      .WillOnce(
          Invoke([&schema_result](const std::string &,
                                  MysqlResultWrapperPtr *out, bool) -> int {
            *out = schema_result;
            return 0;
          }))
      .RetiresOnSaturation();
  // init table reader
  int ret = reader.init();
  ASSERT_EQ(ret, 0);

  // build scan table result
  MockMysqlResultWrapperPtr scan_result = builder_.BuildScanTableResult();
  EXPECT_CALL(*connector2_, execute_query(_, _, _))
      .WillOnce(Invoke([&scan_result](const std::string &,
                                      MysqlResultWrapperPtr *out, bool) -> int {
        *out = scan_result;
        return 0;
      }))
      .RetiresOnSaturation();
  ret = reader.start(ctx_);
  ASSERT_EQ(ret, 0);
}

TEST_F(TableReaderTest, TestInitFailedWithGetConnector) {
  MysqlConnectorManagerPtr mgr = std::make_shared<MysqlConnectorManager>();
  MysqlConnectorPtr connector;
  mgr->put(connector);
  TableReader reader(table_name_, fetcher_, mgr);
  int ret = reader.init();
  ASSERT_EQ(ret, ErrorCode_RuntimeError);
}

TEST_F(TableReaderTest, TestInitFailedWithGetTableSchema) {
  TableReader reader(table_name_, fetcher_, mgr_);

  // build schema result
  MockMysqlResultWrapperPtr schema_result = builder_.BuildQuerySchemaResult();
  EXPECT_CALL(*connector1_, execute_query(_, _, _))
      .WillOnce(
          Invoke([&schema_result](const std::string &,
                                  MysqlResultWrapperPtr *out, bool) -> int {
            *out = schema_result;
            return ErrorCode_ExecuteMysql;
          }))
      .RetiresOnSaturation();
  // init table reader
  int ret = reader.init();
  ASSERT_EQ(ret, ErrorCode_ExecuteMysql);
}

TEST_F(TableReaderTest, TestStartFailedWithPrepareReader) {
  TableReader reader(table_name_, fetcher_, mgr_);

  // build schema result
  MockMysqlResultWrapperPtr schema_result = builder_.BuildQuerySchemaResult();
  MockMysqlResultWrapperPtr result1 = builder_.BuildQueryCollationResult();
  EXPECT_CALL(*connector1_, execute_query(_, _, _))
      .Times(2)
      .WillOnce(Invoke([&result1](const std::string &,
                                  MysqlResultWrapperPtr *out, bool) -> int {
        *out = result1;
        return 0;
      }))
      .WillOnce(
          Invoke([&schema_result](const std::string &,
                                  MysqlResultWrapperPtr *out, bool) -> int {
            *out = schema_result;
            return 0;
          }))
      .RetiresOnSaturation();
  // init table reader
  int ret = reader.init();
  ASSERT_EQ(ret, 0);

  // build scan table result
  MockMysqlResultWrapperPtr scan_result = builder_.BuildScanTableResult();
  EXPECT_CALL(*connector2_, execute_query(_, _, _))
      .WillOnce(Invoke([&scan_result](const std::string &,
                                      MysqlResultWrapperPtr *out, bool) -> int {
        *out = scan_result;
        return ErrorCode_ExecuteMysql;
      }))
      .RetiresOnSaturation();
  ret = reader.start(ctx_);
  ASSERT_EQ(ret, ErrorCode_ExecuteMysql);
}

TEST_F(TableReaderTest, TestGetNextRowDataSuccess) {
  TableReader reader(table_name_, fetcher_, mgr_);

  // build schema result
  MockMysqlResultWrapperPtr schema_result = builder_.BuildQuerySchemaResult();
  MockMysqlResultWrapperPtr result1 = builder_.BuildQueryCollationResult();
  EXPECT_CALL(*connector1_, execute_query(_, _, _))
      .Times(2)
      .WillOnce(Invoke([&result1](const std::string &,
                                  MysqlResultWrapperPtr *out, bool) -> int {
        *out = result1;
        return 0;
      }))
      .WillOnce(
          Invoke([&schema_result](const std::string &,
                                  MysqlResultWrapperPtr *out, bool) -> int {
            *out = schema_result;
            return 0;
          }))
      .RetiresOnSaturation();

  // init table reader
  int ret = reader.init();
  ASSERT_EQ(ret, 0);

  // build scan table result
  MockMysqlResultWrapperPtr scan_result = builder_.BuildScanTableResult();
  EXPECT_CALL(*connector2_, execute_query(_, _, _))
      .WillOnce(Invoke([&scan_result](const std::string &,
                                      MysqlResultWrapperPtr *out, bool) -> int {
        *out = scan_result;
        return 0;
      }))
      .RetiresOnSaturation();
  ret = reader.start(ctx_);
  ASSERT_EQ(ret, 0);

  // get next row data
  WriteRequest::Row row_data;
  LsnContext ctx;
  ret = reader.get_next_row_data(&row_data, &ctx);
  ASSERT_EQ(ret, 0);
  ASSERT_EQ(ctx.seq_id, (uint64_t)1);
  ASSERT_EQ(ctx.status, RowDataStatus::NORMAL);
  ASSERT_EQ(row_data.primary_key(), (uint64_t)1);
  ASSERT_EQ(row_data.operation_type(), ::proxima::be::proto::OP_INSERT);
  ASSERT_EQ(row_data.forward_column_values().values(0).string_value(), "name1");
  ASSERT_EQ(row_data.forward_column_values().values(1).int32_value(), 18);
  ASSERT_EQ(row_data.index_column_values().values(0).string_value(), "1,2,3,4");
  ASSERT_EQ(row_data.index_column_values().values(1).string_value(), "1,2,3,5");

  row_data.Clear();
  ret = reader.get_next_row_data(&row_data, &ctx);
  ASSERT_EQ(ret, 0);
  ASSERT_EQ(ctx.status, RowDataStatus::NORMAL);
  ASSERT_EQ(ctx.seq_id, (uint64_t)2);
  ASSERT_EQ(row_data.primary_key(), (uint64_t)2);
  ASSERT_EQ(row_data.operation_type(), ::proxima::be::proto::OP_INSERT);
  ASSERT_EQ(row_data.forward_column_values().values(0).string_value(), "name2");
  ASSERT_EQ(row_data.forward_column_values().values(1).int32_value(), 19);
  ASSERT_EQ(row_data.index_column_values().values(0).string_value(), "2,2,3,4");
  ASSERT_EQ(row_data.index_column_values().values(1).string_value(), "2,2,3,5");

  row_data.Clear();
  ret = reader.get_next_row_data(&row_data, &ctx);
  ASSERT_EQ(ret, 0);
  ASSERT_EQ(ctx.status, RowDataStatus::NO_MORE_DATA);
}

TEST_F(TableReaderTest, TestGetNextRowDataFailed) {
  TableReader reader(table_name_, fetcher_, mgr_);

  // build schema result
  MockMysqlResultWrapperPtr schema_result = builder_.BuildQuerySchemaResult();
  MockMysqlResultWrapperPtr result1 = builder_.BuildQueryCollationResult();
  EXPECT_CALL(*connector1_, execute_query(_, _, _))
      .Times(2)
      .WillOnce(Invoke([&result1](const std::string &,
                                  MysqlResultWrapperPtr *out, bool) -> int {
        *out = result1;
        return 0;
      }))
      .WillOnce(
          Invoke([&schema_result](const std::string &,
                                  MysqlResultWrapperPtr *out, bool) -> int {
            *out = schema_result;
            return 0;
          }))
      .RetiresOnSaturation();

  // init table reader
  int ret = reader.init();
  ASSERT_EQ(ret, 0);

  // build scan table result
  MockMysqlResultWrapperPtr scan_result = builder_.BuildScanTableResult();
  EXPECT_CALL(*connector2_, execute_query(_, _, _))
      .WillOnce(Invoke([&scan_result](const std::string &,
                                      MysqlResultWrapperPtr *out, bool) -> int {
        *out = scan_result;
        return 0;
      }))
      .RetiresOnSaturation();

  ret = reader.start(ctx_);
  ASSERT_EQ(ret, 0);

  // get next row data
  WriteRequest::Row row_data;
  LsnContext ctx;
  ret = reader.get_next_row_data(&row_data, &ctx);
  ASSERT_EQ(ret, 0);
  ASSERT_EQ(ctx.seq_id, (uint64_t)1);
  ASSERT_EQ(ctx.status, RowDataStatus::NORMAL);
  ASSERT_EQ(row_data.primary_key(), (uint64_t)1);
  ASSERT_EQ(row_data.operation_type(), ::proxima::be::proto::OP_INSERT);
  ASSERT_EQ(row_data.forward_column_values().values(0).string_value(), "name1");
  ASSERT_EQ(row_data.forward_column_values().values(1).int32_value(), 18);
  ASSERT_EQ(row_data.index_column_values().values(0).string_value(), "1,2,3,4");
  ASSERT_EQ(row_data.index_column_values().values(1).string_value(), "1,2,3,5");

  row_data.Clear();
  ret = reader.get_next_row_data(&row_data, &ctx);
  ASSERT_EQ(ret, 0);
  ASSERT_EQ(ctx.status, RowDataStatus::NORMAL);
  ASSERT_EQ(ctx.seq_id, (uint64_t)2);
  ASSERT_EQ(row_data.primary_key(), (uint64_t)2);
  ASSERT_EQ(row_data.operation_type(), ::proxima::be::proto::OP_INSERT);
  ASSERT_EQ(row_data.forward_column_values().values(0).string_value(), "name2");
  ASSERT_EQ(row_data.forward_column_values().values(1).int32_value(), 19);
  ASSERT_EQ(row_data.index_column_values().values(0).string_value(), "2,2,3,4");
  ASSERT_EQ(row_data.index_column_values().values(1).string_value(), "2,2,3,5");

  row_data.Clear();
  scan_result->set_has_error(true);
  ret = reader.get_next_row_data(&row_data, &ctx);
  ASSERT_EQ(ret, ErrorCode_FetchMysqlResult);
}
