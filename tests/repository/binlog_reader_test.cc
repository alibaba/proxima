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
#include "repository/binlog/binlog_reader.h"
#include "repository/binlog/info_fetcher.h"
#include "repository/binlog/rows_event_parser.h"
#include "mock_mysql_connector.h"
#undef private
#include "repository/repository_common/error_code.h"
#include "event_builder.h"
#include "mysql_result_builder.h"

using namespace ::proxima::be;
using namespace proxima::be::repository;

class BinlogReaderTest : public testing::Test {
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
    InitTableSchema();

    ctx_.position = 4;
    ctx_.file_name = "binlog.000004";

    table_name_ = builder_.table_name_;
  }

  void TearDown() {}

  void InitTableSchema() {
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
  }

  std::string BuildNoMoreDataEvent() {
    std::string str(1, (char)254);
    return str;
  }

  std::string BuildQueryEventStr(const std::string &query) {
    return " " + EventBuilder::BuildQueryEvent(builder_.db_, query);
  }

  std::string BuildRotateEventStr(const std::string &file, bool has_crc) {
    return " " + EventBuilder::BuildRotateEvent(file, 4, has_crc);
  }

 protected:
  MysqlConnectorManagerPtr mgr_{};
  MockMysqlConnectorPtr connector1_{};
  MockMysqlConnectorPtr connector2_{};
  std::string table_name_{};
  InfoFetcherPtr fetcher_{};
  TableSchemaPtr schema_;
  LsnContext ctx_;
  MysqlResultBuilder builder_{};
};

TEST_F(BinlogReaderTest, TestSimple) {
  BinlogReader reader(table_name_, fetcher_, mgr_);
  MockMysqlResultWrapperPtr result = builder_.BuildQuerySchemaResult();
  MockMysqlResultWrapperPtr result1 = builder_.BuildQueryCollationResult();
  // init table schema
  EXPECT_CALL(*connector1_, execute_query(_, _, _))
      .Times(2)
      .WillOnce(Invoke([&result1](const std::string &,
                                  MysqlResultWrapperPtr *out, bool) -> int {
        *out = result1;
        return 0;
      }))
      .WillOnce(Invoke([&result](const std::string &,
                                 MysqlResultWrapperPtr *out, bool) -> int {
        *out = result;
        return 0;
      }))
      .RetiresOnSaturation();
  int ret = reader.init();
  ASSERT_EQ(ret, 0);

  // first set check sum
  EXPECT_CALL(*connector2_, execute_query(_, _, _))
      .Times(2)
      .WillOnce(::testing::Return(0))
      .WillOnce(::testing::Return(0))
      .RetiresOnSaturation();
  // second request dump
  EXPECT_CALL(*connector2_, execute_simple_command(_, _, _))
      .WillOnce(::testing::Return(0))
      .RetiresOnSaturation();
  ret = reader.start(ctx_);
  ASSERT_EQ(ret, 0);

  // fetch data
  std::string table_map_str = builder_.BuildTableMapEventStr();
  std::vector<std::string> values = {"1",       "name1",   "30",     "123.456",
                                     "1,2,3,4", "1,2,3,5", "1,2,3,6"};
  std::string write_rows_str = builder_.BuildWriteRowsEventStr(values);
  EXPECT_CALL(*connector2_, client_safe_read(_))
      .Times(2)
      .WillOnce(Invoke([&table_map_str](unsigned long *len) -> int {
        *len = table_map_str.size();
        return 0;
      }))
      .WillOnce(Invoke([&write_rows_str](unsigned long *len) -> int {
        *len = write_rows_str.size();
        return 0;
      }))
      .RetiresOnSaturation();

  EXPECT_CALL(*connector2_, data())
      .Times(2)
      .WillOnce(Invoke([&table_map_str]() -> unsigned char * {
        return (unsigned char *)table_map_str.c_str();
      }))
      .WillOnce(Invoke([&write_rows_str]() -> unsigned char * {
        return (unsigned char *)write_rows_str.c_str();
      }))
      .RetiresOnSaturation();

  WriteRequest::Row row_data;
  LsnContext context;
  ret = reader.get_next_row_data(&row_data, &context);
  ASSERT_EQ(ret, 0);
  ASSERT_EQ(context.status, RowDataStatus::NORMAL);
  ASSERT_EQ(row_data.primary_key(), (uint64_t)1);
  ASSERT_EQ(row_data.forward_column_values().values(0).string_value(), "name1");
  ASSERT_EQ(row_data.forward_column_values().values(1).int32_value(), 30);
  ASSERT_EQ(row_data.index_column_values().values(0).string_value(), "1,2,3,4");
  ASSERT_EQ(row_data.index_column_values().values(1).string_value(), "1,2,3,5");
}

TEST_F(BinlogReaderTest, TestGetNextRowData) {
  BinlogReader reader(table_name_, fetcher_, mgr_);
  MockMysqlResultWrapperPtr result = builder_.BuildQuerySchemaResult();
  MockMysqlResultWrapperPtr result1 = builder_.BuildQueryCollationResult();
  // init table schema
  EXPECT_CALL(*connector1_, execute_query(_, _, _))
      .Times(2)
      .WillOnce(Invoke([&result1](const std::string &,
                                  MysqlResultWrapperPtr *out, bool) -> int {
        *out = result1;
        return 0;
      }))
      .WillOnce(Invoke([&result](const std::string &,
                                 MysqlResultWrapperPtr *out, bool) -> int {
        *out = result;
        return 0;
      }))
      .RetiresOnSaturation();

  int ret = reader.init();
  ASSERT_EQ(ret, 0);

  // first set check sum
  EXPECT_CALL(*connector2_, execute_query(_, _, _))
      .Times(2)
      .WillOnce(::testing::Return(0))
      .WillOnce(::testing::Return(0))
      .RetiresOnSaturation();
  // second request dump
  EXPECT_CALL(*connector2_, execute_simple_command(_, _, _))
      .WillOnce(::testing::Return(0))
      .RetiresOnSaturation();

  ret = reader.start(ctx_);
  ASSERT_EQ(ret, 0);

  // fetch data
  std::string rotate_event_str = BuildRotateEventStr(ctx_.file_name, false);
  std::string rotate_event_str1 = BuildRotateEventStr(ctx_.file_name, true);
  std::string query_event_str = BuildQueryEventStr("query event");
  std::string query_event_str1 = BuildQueryEventStr("alter table mytest.");
  std::string table_map_str = builder_.BuildTableMapEventStr();
  std::vector<std::string> values = {"1",       "name1",   "30",     "123.456",
                                     "1,2,3,4", "1,2,3,5", "1,2,3,6"};
  size_t rows_count = 2;
  std::string write_rows_str =
      builder_.BuildWriteRowsEventStr(values, rows_count);
  EXPECT_CALL(*connector2_, client_safe_read(_))
      .Times(5)
      .WillOnce(Invoke([&rotate_event_str](unsigned long *len) -> int {
        *len = rotate_event_str.size();
        return 0;
      }))
      .WillOnce(Invoke([&query_event_str](unsigned long *len) -> int {
        *len = query_event_str.size();
        return 0;
      }))
      .WillOnce(Invoke([&rotate_event_str1](unsigned long *len) -> int {
        *len = rotate_event_str1.size();
        return 0;
      }))
      .WillOnce(Invoke([&table_map_str](unsigned long *len) -> int {
        *len = table_map_str.size();
        return 0;
      }))
      .WillOnce(Invoke([&write_rows_str](unsigned long *len) -> int {
        *len = write_rows_str.size();
        return 0;
      }))
      .RetiresOnSaturation();

  EXPECT_CALL(*connector2_, data())
      .Times(5)
      .WillOnce(Invoke([&rotate_event_str]() -> unsigned char * {
        return (unsigned char *)rotate_event_str.c_str();
      }))
      .WillOnce(Invoke([&query_event_str]() -> unsigned char * {
        return (unsigned char *)query_event_str.c_str();
      }))
      .WillOnce(Invoke([&rotate_event_str1]() -> unsigned char * {
        return (unsigned char *)rotate_event_str1.c_str();
      }))
      .WillOnce(Invoke([&table_map_str]() -> unsigned char * {
        return (unsigned char *)table_map_str.c_str();
      }))
      .WillOnce(Invoke([&write_rows_str]() -> unsigned char * {
        return (unsigned char *)write_rows_str.c_str();
      }))
      .RetiresOnSaturation();


  for (size_t i = 0; i < rows_count; ++i) {
    WriteRequest::Row row_data;
    LsnContext context;
    ret = reader.get_next_row_data(&row_data, &context);
    ASSERT_EQ(ret, 0);
    ASSERT_EQ(context.status, RowDataStatus::NORMAL);
    ASSERT_EQ(row_data.primary_key(), (uint64_t)1);
    ASSERT_EQ(row_data.forward_column_values().values(0).string_value(),
              "name1");
    ASSERT_EQ(row_data.forward_column_values().values(1).int32_value(), 30);
    ASSERT_EQ(row_data.index_column_values().values(0).string_value(),
              "1,2,3,4");
    ASSERT_EQ(row_data.index_column_values().values(1).string_value(),
              "1,2,3,5");
  }

  EXPECT_CALL(*connector2_, client_safe_read(_))
      .WillOnce(Invoke([&query_event_str1](unsigned long *len) -> int {
        *len = query_event_str1.size();
        return 0;
      }))
      .RetiresOnSaturation();

  EXPECT_CALL(*connector2_, data())
      .WillOnce(Invoke([&query_event_str1]() -> unsigned char * {
        return (unsigned char *)query_event_str1.c_str();
      }))
      .RetiresOnSaturation();

  result1->reset();
  EXPECT_CALL(*connector1_, execute_query(_, _, _))
      .Times(2)
      .WillOnce(Invoke([&result1](const std::string &,
                                  MysqlResultWrapperPtr *out, bool) -> int {
        *out = result1;
        return 0;
      }))
      .WillOnce(Invoke([&result](const std::string &,
                                 MysqlResultWrapperPtr *out, bool) -> int {
        *out = result;
        return ErrorCode_ExecuteMysql;
      }))
      .RetiresOnSaturation();

  WriteRequest::Row row_data;
  LsnContext context;
  ret = reader.get_next_row_data(&row_data, &context);
  ASSERT_EQ(ret, ErrorCode_ExecuteMysql);

  result1->reset();
  EXPECT_CALL(*connector1_, execute_query(_, _, _))
      .Times(2)
      .WillOnce(Invoke([&result1](const std::string &,
                                  MysqlResultWrapperPtr *out, bool) -> int {
        *out = result1;
        return 0;
      }))
      .WillOnce(Invoke([&result](const std::string &,
                                 MysqlResultWrapperPtr *out, bool) -> int {
        *out = result;
        return 0;
      }))
      .RetiresOnSaturation();

  ret = reader.get_next_row_data(&row_data, &context);
  ASSERT_EQ(ret, 0);
  ASSERT_EQ(context.status, RowDataStatus::SCHEMA_CHANGED);
}

TEST_F(BinlogReaderTest, TestInitWithGetTableSchemaFailed) {
  BinlogReader reader(table_name_, fetcher_, mgr_);
  // init table schema
  EXPECT_CALL(*connector1_, execute_query(_, _, _))
      .Times(1)
      .WillOnce(testing::Return(1))
      .RetiresOnSaturation();
  int ret = reader.init();
  ASSERT_EQ(ret, ErrorCode_ExecuteMysql);
}

TEST_F(BinlogReaderTest, TestStartWithInitEventFetcherFailed) {
  BinlogReader reader(table_name_, fetcher_, mgr_);

  MockMysqlResultWrapperPtr result = builder_.BuildQuerySchemaResult();
  MockMysqlResultWrapperPtr result1 = builder_.BuildQueryCollationResult();
  // init table schema
  EXPECT_CALL(*connector1_, execute_query(_, _, _))
      .Times(2)
      .WillOnce(Invoke([&result1](const std::string &,
                                  MysqlResultWrapperPtr *out, bool) -> int {
        *out = result1;
        return 0;
      }))
      .WillOnce(Invoke([&result](const std::string &,
                                 MysqlResultWrapperPtr *out, bool) -> int {
        *out = result;
        return 0;
      }))
      .RetiresOnSaturation();
  int ret = reader.init();
  ASSERT_EQ(ret, 0);

  // first set check sum
  EXPECT_CALL(*connector2_, execute_query(_, _, _))
      .WillOnce(::testing::Return(1))
      .RetiresOnSaturation();
  ret = reader.start(ctx_);
  ASSERT_EQ(ret, 1);
}

TEST_F(BinlogReaderTest, TestGetNextRowDataWithNoMoreData) {
  BinlogReader reader(table_name_, fetcher_, mgr_);
  MockMysqlResultWrapperPtr result = builder_.BuildQuerySchemaResult();
  MockMysqlResultWrapperPtr result1 = builder_.BuildQueryCollationResult();
  // init table schema
  EXPECT_CALL(*connector1_, execute_query(_, _, _))
      .Times(2)
      .WillOnce(Invoke([&result1](const std::string &,
                                  MysqlResultWrapperPtr *out, bool) -> int {
        *out = result1;
        return 0;
      }))
      .WillOnce(Invoke([&result](const std::string &,
                                 MysqlResultWrapperPtr *out, bool) -> int {
        *out = result;
        return 0;
      }))
      .RetiresOnSaturation();

  int ret = reader.init();
  ASSERT_EQ(ret, 0);

  // first set check sum
  EXPECT_CALL(*connector2_, execute_query(_, _, _))
      .Times(2)
      .WillOnce(::testing::Return(0))
      .WillOnce(::testing::Return(0))
      .RetiresOnSaturation();
  // second request dump
  EXPECT_CALL(*connector2_, execute_simple_command(_, _, _))
      .WillOnce(::testing::Return(0))
      .RetiresOnSaturation();

  ret = reader.start(ctx_);
  ASSERT_EQ(ret, 0);

  // fetch data
  std::string no_more_event = BuildNoMoreDataEvent();
  EXPECT_CALL(*connector2_, client_safe_read(_))
      .WillOnce(Invoke([&no_more_event](unsigned long *len) -> int {
        *len = no_more_event.size();
        return 0;
      }))
      .RetiresOnSaturation();

  EXPECT_CALL(*connector2_, data())
      .WillOnce(Invoke([&no_more_event]() -> unsigned char * {
        return (unsigned char *)no_more_event.c_str();
      }))
      .RetiresOnSaturation();

  WriteRequest::Row row_data;
  LsnContext context;
  ret = reader.get_next_row_data(&row_data, &context);
  ASSERT_EQ(ret, 0);
  ASSERT_EQ(context.status, RowDataStatus::NO_MORE_DATA);
}
