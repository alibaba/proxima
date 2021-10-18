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
#include "repository/binlog/event_fetcher.h"
#include "event_builder.h"
#include "mock_mysql_connector.h"
#include "mysql_result_builder.h"
#undef private
#include "repository/repository_common/error_code.h"
#include "event_builder.h"

using namespace ::proxima::be;
using namespace proxima::be::repository;

class EventFetcherTest : public testing::Test {
 protected:
  void SetUp() {
    mgr_ = std::make_shared<MysqlConnectorManager>();
    ASSERT_TRUE(mgr_);
    connector_ = std::make_shared<MockMysqlConnector>();
    ASSERT_TRUE(connector_);
    mgr_->put(connector_);
    connection_uri_ = "mysql://root:root@127.0.0.1:3306/mytest";
    ASSERT_TRUE(ailego::Uri::Parse(connection_uri_.c_str(), &uri_));
    table_name_ = "table";
    db_ = "mytest";
    file_name_ = "binlog.000004";
    BuildSchemaInfo();
    table_id_ = 1000;
  }

  void TearDown() {}

  void BuildSchemaInfo() {
    column_types_.push_back(MYSQL_TYPE_LONG);
    column_types_.push_back(MYSQL_TYPE_VAR_STRING);
    column_types_.push_back(MYSQL_TYPE_LONG);
    column_types_.push_back(MYSQL_TYPE_FLOAT);
    column_types_.push_back(MYSQL_TYPE_VAR_STRING);
    column_types_.push_back(MYSQL_TYPE_VAR_STRING);
    column_types_.push_back(MYSQL_TYPE_VAR_STRING);
    column_metas_.push_back(0);
    column_metas_.push_back(2);
    column_metas_.push_back(0);
    column_metas_.push_back(0);
    column_metas_.push_back(2);
    column_metas_.push_back(2);
    column_metas_.push_back(2);
  }

  void InitFetcher() {
    // init
    fetcher_ = std::make_shared<EventFetcher>(mgr_);
    // set checksum
    EXPECT_CALL(*connector_, execute_query(_, _, _))
        .Times(2)
        .WillOnce(testing::Return(0))
        .WillOnce(testing::Return(0))
        .RetiresOnSaturation();

    // request dump
    EXPECT_CALL(*connector_, execute_simple_command(_, _, _))
        .WillOnce(testing::Return(0))
        .RetiresOnSaturation();

    int ret = fetcher_->init(file_name_, 4);
    ASSERT_EQ(ret, 0);
  }

  std::string BuildTableMapEventStr() {
    std::vector<bool> column_nulls(column_types_.size(), false);
    column_nulls[column_types_.size() - 1] = true;
    std::string table_map = EventBuilder::BuildTableMapEvent(
        table_id_, db_, table_name_, column_types_, column_metas_,
        column_nulls);
    return " " + table_map;
  }

  std::string BuildNoMoreDataEvent() {
    std::string str(1, (char)254);
    return str;
  }

  std::string BuildOtherEventStr(EventType type) {
    return " " + EventBuilder::BuildOtherEvent(type);
  }

  std::string BuildQueryEventStr(const std::string &query) {
    return " " + EventBuilder::BuildQueryEvent(db_, query);
  }

  std::string BuildRotateEventStr(const std::string &file, bool has_crc) {
    return " " + EventBuilder::BuildRotateEvent(file, 4, has_crc);
  }

  std::string BuildWriteRowsEventStr(std::vector<std::string> &column_values) {
    std::string event_str = BuildTableMapEventStr();
    TableMapEventPtr table_map = std::make_shared<TableMapEvent>(
        event_str.substr(1).c_str(), event_str.size() - 1);
    std::vector<bool> column_nulls(column_types_.size(), false);
    std::string rows_str = EventBuilder::BuildWriteRowsEvent(
        table_id_, column_nulls, column_types_, column_values, table_map);
    return " " + rows_str;
  }

 protected:
  MysqlConnectorManagerPtr mgr_{};
  MockMysqlConnectorPtr connector_{};
  std::string connection_uri_{};
  ailego::Uri uri_{};
  std::string table_name_{};
  EventFetcherPtr fetcher_{};
  std::string file_name_{};
  std::string db_{};
  uint64_t table_id_{0};
  std::vector<enum_field_types> column_types_{};
  std::vector<int32_t> column_metas_{};
};

TEST_F(EventFetcherTest, TestGeneral) {
  InitFetcher();

  std::string table_map_str = BuildTableMapEventStr();
  BasicEventPtr event;
  EXPECT_CALL(*connector_, client_safe_read(_))
      .WillOnce(Invoke([&table_map_str](unsigned long *len) -> int {
        *len = table_map_str.size();
        return 0;
      }))
      .RetiresOnSaturation();

  EXPECT_CALL(*connector_, data())
      .WillOnce(Invoke([&table_map_str]() -> unsigned char * {
        return (unsigned char *)table_map_str.c_str();
      }))
      .RetiresOnSaturation();

  int ret = fetcher_->fetch(&event);
  ASSERT_EQ(ret, 0);
  ASSERT_EQ(event->type(), TABLE_MAP_EVENT);
}

// TEST_F(EventFetcherTest, TestFetchWithReadDataFailed) {
//   InitFetcher();
//   fetcher_->need_reconnect_ = true;

//   BasicEventPtr event;
//   EXPECT_CALL(*connector_, reconnect())
//       .Times(1)
//       .WillOnce(testing::Return(false))
//       .RetiresOnSaturation();

//   int ret = fetcher_->fetch(&event);
//   ASSERT_EQ(ret, ErrorCode_ConnectMysql);
// }

// TEST_F(EventFetcherTest, TestFetchWithNoMoreData) {
//   InitFetcher();

//   std::string event_str = BuildNoMoreDataEvent();
//   BasicEventPtr event;
//   EXPECT_CALL(*connector_, client_safe_read(_))
//       .WillOnce(Invoke([&event_str](unsigned long *len) -> int {
//         *len = event_str.size();
//         return 0;
//       }))
//       .RetiresOnSaturation();

//   EXPECT_CALL(*connector_, data())
//       .WillOnce(Invoke([&event_str]() -> unsigned char * {
//         return (unsigned char *)event_str.c_str();
//       }))
//       .RetiresOnSaturation();

//   int ret = fetcher_->fetch(&event);
//   ASSERT_EQ(ret, ErrorCode_BinlogNoMoreData);
// }

// TEST_F(EventFetcherTest, TestFetchWithQueryEvent) {
//   InitFetcher();

//   std::string event_str = BuildQueryEventStr("test query");
//   BasicEventPtr event;
//   EXPECT_CALL(*connector_, client_safe_read(_))
//       .WillOnce(Invoke([&event_str](unsigned long *len) -> int {
//         *len = event_str.size();
//         return 0;
//       }))
//       .RetiresOnSaturation();

//   EXPECT_CALL(*connector_, data())
//       .WillOnce(Invoke([&event_str]() -> unsigned char * {
//         return (unsigned char *)event_str.c_str();
//       }))
//       .RetiresOnSaturation();

//   int ret = fetcher_->fetch(&event);
//   ASSERT_EQ(ret, 0);
//   ASSERT_EQ(event->type(), QUERY_EVENT);
// }

// TEST_F(EventFetcherTest, TestFetchWithRotateEvent) {
//   InitFetcher();

//   std::string event_str = BuildRotateEventStr(file_name_, true);
//   BasicEventPtr event;
//   EXPECT_CALL(*connector_, client_safe_read(_))
//       .WillOnce(Invoke([&event_str](unsigned long *len) -> int {
//         *len = event_str.size();
//         return 0;
//       }))
//       .RetiresOnSaturation();

//   EXPECT_CALL(*connector_, data())
//       .WillOnce(Invoke([&event_str]() -> unsigned char * {
//         return (unsigned char *)event_str.c_str();
//       }))
//       .RetiresOnSaturation();

//   int ret = fetcher_->fetch(&event);
//   ASSERT_EQ(ret, 0);
//   ASSERT_EQ(event->type(), ROTATE_EVENT);
// }

// TEST_F(EventFetcherTest, TestFetchWithTableMapEvent) {
//   InitFetcher();

//   std::string event_str = BuildTableMapEventStr();
//   BasicEventPtr event;
//   EXPECT_CALL(*connector_, client_safe_read(_))
//       .WillOnce(Invoke([&event_str](unsigned long *len) -> int {
//         *len = event_str.size();
//         return 0;
//       }))
//       .RetiresOnSaturation();

//   EXPECT_CALL(*connector_, data())
//       .WillOnce(Invoke([&event_str]() -> unsigned char * {
//         return (unsigned char *)event_str.c_str();
//       }))
//       .RetiresOnSaturation();

//   int ret = fetcher_->fetch(&event);
//   ASSERT_EQ(ret, 0);
//   ASSERT_EQ(event->type(), TABLE_MAP_EVENT);
// }

// TEST_F(EventFetcherTest, TestFetchWithWriteRowsEvent) {
//   InitFetcher();

//   std::vector<std::string> values = {"1",       "name1",   "30", "123.456",
//                                      "1,2,3,4", "1,2,3,5", "1,2,3,6"};
//   std::string event_str = BuildWriteRowsEventStr(values);
//   BasicEventPtr event;
//   EXPECT_CALL(*connector_, client_safe_read(_))
//       .WillOnce(Invoke([&event_str](unsigned long *len) -> int {
//         *len = event_str.size();
//         return 0;
//       }))
//       .RetiresOnSaturation();

//   EXPECT_CALL(*connector_, data())
//       .WillOnce(Invoke([&event_str]() -> unsigned char * {
//         return (unsigned char *)event_str.c_str();
//       }))
//       .RetiresOnSaturation();

//   int ret = fetcher_->fetch(&event);
//   ASSERT_EQ(ret, 0);
//   ASSERT_EQ(event->type(), WRITE_ROWS_EVENT_V1);
// }

// TEST_F(EventFetcherTest, TestFetchWithOtherEvent) {
//   InitFetcher();

//   std::string event_str = BuildOtherEventStr(HEARTBEAT_LOG_EVENT);
//   BasicEventPtr event;
//   EXPECT_CALL(*connector_, client_safe_read(_))
//       .WillOnce(Invoke([&event_str](unsigned long *len) -> int {
//         *len = event_str.size();
//         return 0;
//       }))
//       .RetiresOnSaturation();

//   EXPECT_CALL(*connector_, data())
//       .WillOnce(Invoke([&event_str]() -> unsigned char * {
//         return (unsigned char *)event_str.c_str();
//       }))
//       .RetiresOnSaturation();

//   int ret = fetcher_->fetch(&event);
//   ASSERT_EQ(ret, 0);
//   ASSERT_EQ(event->type(), HEARTBEAT_LOG_EVENT);
// }

// TEST_F(EventFetcherTest, TestUpdateLsnInfo) {
//   InitFetcher();

//   std::string file_name("binlog.000001");
//   uint64_t position = 4;

//   // lsn valid
//   EXPECT_CALL(*connector_, execute_query(_, _, _))
//       .Times(1)
//       .WillOnce(testing::Return(0))
//       .RetiresOnSaturation();

//   int ret = fetcher_->update_lsn_info(file_name, position);
//   ASSERT_EQ(ret, 0);

//   ASSERT_EQ(fetcher_->file_name_, file_name);
//   ASSERT_EQ(fetcher_->position_, position);

//   MysqlResultBuilder builder;
//   MysqlResultWrapperPtr result = builder.BuildShowBinaryLogsResult();
//   // lsn invalid
//   EXPECT_CALL(*connector_, execute_query(_, _, _))
//       .Times(2)
//       .WillOnce(testing::Return(1))
//       .WillOnce(Invoke([&result](const std::string &,
//                                  MysqlResultWrapperPtr *out, bool) -> int {
//         *out = result;
//         return 0;
//       }))
//       .RetiresOnSaturation();


//   ret = fetcher_->update_lsn_info(file_name, position);
//   ASSERT_EQ(ret, 0);
//   ASSERT_EQ(fetcher_->file_name_, "binlog.000004");
//   ASSERT_EQ(fetcher_->position_, 4);
// }
