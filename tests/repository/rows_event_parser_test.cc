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
#include "repository/binlog/info_fetcher.h"
#include "repository/binlog/rows_event_parser.h"
#include "mock_mysql_connector.h"
#undef private
#include "repository/repository_common/error_code.h"
#include "event_builder.h"

using namespace ::proxima::be;
using namespace proxima::be::repository;

class RowsEventParserTest : public testing::Test {
 protected:
  void SetUp() {
    mgr_ = std::make_shared<MysqlConnectorManager>();
    ASSERT_TRUE(mgr_);
    connector_ = std::make_shared<MockMysqlConnector>();
    ASSERT_TRUE(connector_);
    mgr_->put(connector_);
    connection_uri_ = "mysql://127.0.0.1:3306/mytest";
    user_ = "root";
    password_ = "root";
    table_name_ = "table";
    db_ = "mytest";
    InitTableSchema();

    table_id_ = 1000;
  }

  void TearDown() {}

  MockMysqlResultWrapperPtr BuildQuerySchemaResult() {
    MockMysqlResultWrapperPtr result =
        std::make_shared<MockMysqlResultWrapper>();
    result->append_field_meta("id", MYSQL_TYPE_LONG, 11, 0,
                              AUTO_INCREMENT_FLAG);
    result->append_field_meta("name", MYSQL_TYPE_VAR_STRING, 100);
    result->append_field_meta("age", MYSQL_TYPE_LONG, 11);
    result->append_field_meta("score", MYSQL_TYPE_FLOAT, 12);
    result->append_field_meta("vector1", MYSQL_TYPE_VAR_STRING, 1024);
    result->append_field_meta("vector2", MYSQL_TYPE_VAR_STRING, 1024);
    result->append_field_meta("vector3", MYSQL_TYPE_VAR_STRING, 1024);
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

    return result;
  }

  MockMysqlResultWrapperPtr BuildQueryCollationResult() {
    MockMysqlResultWrapperPtr result =
        std::make_shared<MockMysqlResultWrapper>();
    result->append_field_meta("Field", MYSQL_TYPE_VAR_STRING, 11);
    result->append_field_meta("Type", MYSQL_TYPE_VAR_STRING, 100);
    result->append_field_meta("Collation", MYSQL_TYPE_VAR_STRING, 11);

    std::vector<std::string> values1 = {"id", "", ""};
    result->append_row_values(values1);
    std::vector<std::string> values2 = {"name", "", "utf8_general_ci"};
    result->append_row_values(values2);
    std::vector<std::string> values3 = {"age", "", ""};
    result->append_row_values(values3);
    std::vector<std::string> values4 = {"score", "", "utf8_general_ci"};
    result->append_row_values(values4);
    std::vector<std::string> values5 = {"vector1", "", "utf8_general_ci"};
    result->append_row_values(values5);
    std::vector<std::string> values6 = {"vector2", "", "utf8_general_ci"};
    result->append_row_values(values6);
    std::vector<std::string> values7 = {"vector3", "", "utf8_general_ci"};
    result->append_row_values(values7);

    return result;
  }

  void BuildCollectionConfig(CollectionConfig &config) {
    auto *repo = config.mutable_repository_config();
    repo->set_repository_type(CollectionConfig::RepositoryConfig::RT_DATABASE);
    repo->set_repository_name(table_name_);
    auto *database = repo->mutable_database();
    database->set_connection_uri(connection_uri_);
    database->set_table_name(table_name_);
    database->set_user(user_);
    database->set_password(password_);

    config.add_forward_column_names("name");
    config.add_forward_column_names("age");
    auto *index1 = config.add_index_column_params();
    index1->set_column_name("vector1");
    auto *index2 = config.add_index_column_params();
    index2->set_column_name("vector2");
  }

  void InitTableSchema() {
    CollectionConfig config;
    BuildCollectionConfig(config);

    // init
    ailego::Uri test_uri;
    ASSERT_TRUE(ailego::Uri::Parse(connection_uri_.c_str(), &test_uri));
    EXPECT_CALL(*connector_, uri())
        .WillOnce(
            Invoke([&test_uri]() -> const ailego::Uri & { return test_uri; }))
        .RetiresOnSaturation();
    fetcher_ = std::make_shared<InfoFetcher>(config, mgr_);
    int ret = fetcher_->init();
    ASSERT_EQ(ret, 0);

    MockMysqlResultWrapperPtr result1 = BuildQueryCollationResult();
    MockMysqlResultWrapperPtr result = BuildQuerySchemaResult();
    EXPECT_CALL(*connector_, execute_query(_, _, _))
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

    // get table schema
    ret = fetcher_->get_table_schema(table_name_, &schema_);
    ASSERT_EQ(ret, 0);
  }

  TableMapEventPtr BuildTableMapEvent() {
    std::vector<bool> column_nulls(column_types_.size(), false);
    column_nulls[column_types_.size() - 1] = true;
    std::string table_map = EventBuilder::BuildTableMapEvent(
        table_id_, db_, table_name_, column_types_, column_metas_,
        column_nulls);
    return std::make_shared<TableMapEvent>(table_map.data(), table_map.size());
  }

  RowsEventPtr BuildWriteRowsEvent(std::vector<std::string> &column_values,
                                   const TableMapEventPtr &table_map) {
    std::vector<bool> column_nulls(column_types_.size(), false);
    std::string rows_str = EventBuilder::BuildWriteRowsEvent(
        table_id_, column_nulls, column_types_, column_values, table_map);
    return std::make_shared<RowsEvent>(rows_str.data(), rows_str.size());
  }

  RowsEventPtr BuildDeleteRowsEvent(std::vector<std::string> &column_values,
                                    const TableMapEventPtr &table_map) {
    std::vector<bool> column_nulls(column_types_.size(), false);
    std::string rows_str = EventBuilder::BuildDeleteRowsEvent(
        table_id_, column_nulls, column_types_, column_values, table_map);
    return std::make_shared<RowsEvent>(rows_str.data(), rows_str.size());
  }

  RowsEventPtr BuildUpdateRowsEvent(std::vector<std::string> &old_values,
                                    std::vector<std::string> &new_values,
                                    const TableMapEventPtr &table_map) {
    std::vector<bool> column_nulls(column_types_.size(), false);
    std::string rows_str = EventBuilder::BuildUpdateRowsEvent(
        table_id_, column_nulls, column_types_, old_values, new_values,
        table_map);
    return std::make_shared<RowsEvent>(rows_str.data(), rows_str.size());
  }

 protected:
  MysqlConnectorManagerPtr mgr_{};
  MockMysqlConnectorPtr connector_{};
  std::string connection_uri_{};
  std::string user_{};
  std::string password_{};
  std::string table_name_{};
  std::string db_{};
  InfoFetcherPtr fetcher_{};
  TableSchemaPtr schema_;
  uint64_t table_id_{0};
  std::vector<enum_field_types> column_types_{};
  std::vector<int32_t> column_metas_{};
};

TEST_F(RowsEventParserTest, TestSimple) {
  RowsEventParser parser(schema_);
  TableMapEventPtr table_map = BuildTableMapEvent();
  std::vector<std::string> values = {"1",       "name1",   "30",     "123.456",
                                     "1,2,3,4", "1,2,3,5", "1,2,3,6"};
  RowsEventPtr event = BuildWriteRowsEvent(values, table_map);
  ASSERT_TRUE(event);
  event->table_map_ = table_map;
  WriteRequest::Row row_data;
  LsnContext ctx;
  int ret = parser.parse(event.get(), &row_data, &ctx);
  ASSERT_EQ(ret, 0);
  ASSERT_EQ(row_data.primary_key(), (uint64_t)1);
  ASSERT_EQ(row_data.forward_column_values().values(0).string_value(), "name1");
  ASSERT_EQ(row_data.forward_column_values().values(1).int32_value(), 30);
  ASSERT_EQ(row_data.index_column_values().values(0).string_value(), "1,2,3,4");
  ASSERT_EQ(row_data.index_column_values().values(1).string_value(), "1,2,3,5");
}

TEST_F(RowsEventParserTest, TestParseWriteEventSuccess) {
  RowsEventParser parser(schema_);
  TableMapEventPtr table_map = BuildTableMapEvent();
  std::vector<std::string> values = {"1",       "name1",   "30",     "123.456",
                                     "1,2,3,4", "1,2,3,5", "1,2,3,6"};
  RowsEventPtr event = BuildWriteRowsEvent(values, table_map);
  ASSERT_TRUE(event);
  event->table_map_ = table_map;
  WriteRequest::Row row_data;
  LsnContext ctx;
  int ret = parser.parse(event.get(), &row_data, &ctx);
  ASSERT_EQ(ret, 0);
  ASSERT_EQ(row_data.primary_key(), (uint64_t)1);
  ASSERT_EQ(row_data.operation_type(), proto::OP_INSERT);
  ASSERT_EQ(row_data.forward_column_values().values(0).string_value(), "name1");
  ASSERT_EQ(row_data.forward_column_values().values(1).int32_value(), 30);
  ASSERT_EQ(row_data.index_column_values().values(0).string_value(), "1,2,3,4");
  ASSERT_EQ(row_data.index_column_values().values(1).string_value(), "1,2,3,5");
}

TEST_F(RowsEventParserTest, TestParseDeleteEventSuccess) {
  RowsEventParser parser(schema_);
  TableMapEventPtr table_map = BuildTableMapEvent();
  std::vector<std::string> values = {"1",       "name1",   "30",     "123.456",
                                     "1,2,3,4", "1,2,3,5", "1,2,3,6"};
  RowsEventPtr event = BuildDeleteRowsEvent(values, table_map);
  ASSERT_TRUE(event);
  event->table_map_ = table_map;
  WriteRequest::Row row_data;
  LsnContext ctx;
  int ret = parser.parse(event.get(), &row_data, &ctx);
  ASSERT_EQ(ret, 0);
  ASSERT_EQ(row_data.primary_key(), (uint64_t)1);
  ASSERT_EQ(row_data.operation_type(), proto::OP_DELETE);
}

TEST_F(RowsEventParserTest, TestParseUpdateEventSuccess) {
  RowsEventParser parser(schema_);
  TableMapEventPtr table_map = BuildTableMapEvent();
  std::vector<std::string> old_values = {
      "1", "name1", "30", "123.456", "1,2,3,4", "1,2,3,5", "1,2,3,6"};
  std::vector<std::string> new_values = {
      "1", "name2", "40", "123.456", "2,2,3,4", "2,2,3,5", "1,2,3,6"};
  RowsEventPtr event = BuildUpdateRowsEvent(old_values, new_values, table_map);
  ASSERT_TRUE(event);
  event->table_map_ = table_map;
  WriteRequest::Row row_data;
  LsnContext ctx;
  int ret = parser.parse(event.get(), &row_data, &ctx);
  ASSERT_EQ(ret, 0);
  ASSERT_EQ(row_data.primary_key(), (uint64_t)1);
  ASSERT_EQ(row_data.forward_column_values().values(0).string_value(), "name2");
  ASSERT_EQ(row_data.forward_column_values().values(1).int32_value(), 40);
  ASSERT_EQ(row_data.index_column_values().values(0).string_value(), "2,2,3,4");
  ASSERT_EQ(row_data.index_column_values().values(1).string_value(), "2,2,3,5");
}

TEST_F(RowsEventParserTest, TestParseFailedWithSchemaMismatched) {
  schema_->add_field(FieldPtr());
  RowsEventParser parser(schema_);
  TableMapEventPtr table_map = BuildTableMapEvent();
  std::vector<std::string> old_values = {
      "1", "name1", "30", "123.456", "1,2,3,4", "1,2,3,5", "1,2,3,6"};
  std::vector<std::string> new_values = {
      "1", "name2", "40", "123.456", "2,2,3,4", "2,2,3,5", "1,2,3,6"};
  RowsEventPtr event = BuildUpdateRowsEvent(old_values, new_values, table_map);
  ASSERT_TRUE(event);
  event->table_map_ = table_map;
  WriteRequest::Row row_data;
  LsnContext ctx;
  int ret = parser.parse(event.get(), &row_data, &ctx);
  ASSERT_EQ(ret, ErrorCode_InvalidRowData);
}

TEST_F(RowsEventParserTest, TestParseFailedWithParseRowData) {
  RowsEventParser parser(schema_);
  TableMapEventPtr table_map = BuildTableMapEvent();
  std::vector<std::string> values = {"1",       "name1",   "30",     "123.456",
                                     "1,2,3,4", "1,2,3,5", "1,2,3,6"};
  RowsEventPtr event = BuildDeleteRowsEvent(values, table_map);
  ASSERT_TRUE(event);
  event->cur_buf_ = nullptr;
  event->table_map_ = table_map;
  WriteRequest::Row row_data;
  LsnContext ctx;
  int ret = parser.parse(event.get(), &row_data, &ctx);
  ASSERT_EQ(ret, ErrorCode_InvalidRowData);
}

TEST_F(RowsEventParserTest, TestGetAutoIncrementId) {
  RowsEventParser parser(schema_);
  uint64_t id;
  GenericValue value;
  {
    value.set_int32_value(100);
    id = parser.get_auto_increment_id(value);
    ASSERT_EQ(id, 100);
  }
  {
    value.set_int64_value(1000);
    id = parser.get_auto_increment_id(value);
    ASSERT_EQ(id, 1000);
  }
  {
    value.set_uint32_value(100);
    id = parser.get_auto_increment_id(value);
    ASSERT_EQ(id, 100);
  }
  {
    value.set_uint64_value(100);
    id = parser.get_auto_increment_id(value);
    ASSERT_EQ(id, 100);
  }
  {
    value.set_bytes_value("100");
    id = parser.get_auto_increment_id(value);
    ASSERT_EQ(id, INVALID_PRIMARY_KEY);
  }
}
