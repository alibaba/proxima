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
#include "mock_mysql_connector.h"
#undef private

#include "repository/repository_common/error_code.h"

using namespace ::proxima::be;
using namespace proxima::be::repository;

class InfoFetcherTest : public testing::Test {
 protected:
  void SetUp() {
    mgr_ = std::make_shared<MysqlConnectorManager>();
    ASSERT_TRUE(mgr_);
    connector_ = std::make_shared<MockMysqlConnector>();
    ASSERT_TRUE(connector_);
    mgr_->put(connector_);
    connection_uri_ = "mysql://root:root@127.0.0.1:3306/mytest";
    user_ = "root";
    password_ = "root";
    ASSERT_TRUE(ailego::Uri::Parse(connection_uri_.c_str(), &uri_));
    table_name_ = "table";

    InitFetcher();
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

    std::vector<std::string> values1 = {
        "1", "name1", "18", "123.456", "1,2,3,4", "1,2,3,5", "1,2,3,6"};
    result->append_row_values(values1);
    std::vector<std::string> values2 = {
        "2", "name2", "19", "223.456", "2,2,3,4", "2,2,3,5", "2,2,3,6"};
    result->append_row_values(values2);
    std::vector<std::string> values3 = {
        "3", "name3", "29", "323.456", "3,2,3,4", "3,2,3,5", "3,2,3,6"};
    result->append_row_values(values3);

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

  MockMysqlResultWrapperPtr BuildInvalidQueryCollationResult() {
    MockMysqlResultWrapperPtr result =
        std::make_shared<MockMysqlResultWrapper>();
    result->append_field_meta("Field", MYSQL_TYPE_VAR_STRING, 11);
    result->append_field_meta("invalid", MYSQL_TYPE_VAR_STRING, 100);
    result->append_field_meta("Type", MYSQL_TYPE_VAR_STRING, 100);
    result->append_field_meta("Collation", MYSQL_TYPE_VAR_STRING, 11);

    std::vector<std::string> values1 = {"id", "", "", ""};
    result->append_row_values(values1);
    std::vector<std::string> values2 = {"name", "", "", "utf8_general_ci"};
    result->append_row_values(values2);
    std::vector<std::string> values3 = {"age", "", "", ""};
    result->append_row_values(values3);
    std::vector<std::string> values4 = {"score", "", "", "utf8_general_ci"};
    result->append_row_values(values4);
    std::vector<std::string> values5 = {"vector1", "", "", "utf8_general_ci"};
    result->append_row_values(values5);
    std::vector<std::string> values6 = {"vector2", "", "", "utf8_general_ci"};
    result->append_row_values(values6);
    std::vector<std::string> values7 = {"vector3", "", "", "utf8_general_ci"};
    result->append_row_values(values7);

    return result;
  }

  MockMysqlResultWrapperPtr BuildSnapshotResult() {
    MockMysqlResultWrapperPtr result =
        std::make_shared<MockMysqlResultWrapper>();
    result->append_field_meta("File");
    result->append_field_meta("Position");
    result->append_field_meta("Binlog_Do_DB");
    result->append_field_meta("Binlog_Ignore_DB");
    result->append_field_meta("Executed_Gtid_Set");

    std::vector<std::string> values1 = {"binlog.000001", "10240", "", "", ""};
    result->append_row_values(values1);

    return result;
  }

  MockMysqlResultWrapperPtr BuildInvalidRowsSnapshotResult() {
    MockMysqlResultWrapperPtr result =
        std::make_shared<MockMysqlResultWrapper>();
    result->append_field_meta("File");
    result->append_field_meta("Position");
    result->append_field_meta("Binlog_Do_DB");
    result->append_field_meta("Binlog_Ignore_DB");
    result->append_field_meta("Executed_Gtid_Set");

    return result;
  }

  MockMysqlResultWrapperPtr BuildEmptySnapshotResult() {
    MockMysqlResultWrapperPtr result =
        std::make_shared<MockMysqlResultWrapper>();
    result->append_field_meta("File");
    result->append_field_meta("Position");
    result->append_field_meta("Binlog_Do_DB");
    result->append_field_meta("Binlog_Ignore_DB");
    result->append_field_meta("Executed_Gtid_Set");

    return result;
  }

  MockMysqlResultWrapperPtr BuildInvalidFieldSnapshotResult() {
    MockMysqlResultWrapperPtr result =
        std::make_shared<MockMysqlResultWrapper>();
    result->append_field_meta("File");
    result->append_field_meta("Position");
    result->append_field_meta("Binlog_Do_DB");
    result->append_field_meta("Binlog_Ignore_DB");
    // result->append_field_meta("Executed_Gtid_Set");
    std::vector<std::string> values1 = {"binlog.000001", "10240", "", ""};
    result->append_row_values(values1);
    return result;
  }

  MockMysqlResultWrapperPtr BuildInvalidSnapshotResult() {
    MockMysqlResultWrapperPtr result =
        std::make_shared<MockMysqlResultWrapper>();
    result->append_field_meta("Invalid");
    result->append_field_meta("Position");
    result->append_field_meta("Binlog_Do_DB");
    result->append_field_meta("Binlog_Ignore_DB");
    result->append_field_meta("Executed_Gtid_Set");

    std::vector<std::string> values1 = {"binlog.000001", "10240", "", "", ""};
    result->append_row_values(values1);

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

  void InitFetcher() {
    CollectionConfig config;
    BuildCollectionConfig(config);

    // init
    ailego::Uri test_uri = uri_;
    EXPECT_CALL(*connector_, uri())
        .WillOnce(
            Invoke([&test_uri]() -> const ailego::Uri & { return test_uri; }))
        .RetiresOnSaturation();
    fetcher_ = std::make_shared<InfoFetcher>(config, mgr_);
    int ret = fetcher_->init();
    ASSERT_EQ(ret, 0);
    ASSERT_EQ(fetcher_->database(), "mytest");
  }

 protected:
  MysqlConnectorManagerPtr mgr_{};
  MockMysqlConnectorPtr connector_{};
  std::string connection_uri_{};
  std::string user_{};
  std::string password_{};
  ailego::Uri uri_{};
  std::string table_name_{};
  InfoFetcherPtr fetcher_{};
};

TEST_F(InfoFetcherTest, TestSimple) {
  // build schema result
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
  TableSchemaPtr schema;
  int ret = fetcher_->get_table_schema(table_name_, &schema);
  ASSERT_EQ(ret, 0);
  auto &forward_ids = schema->selected_forward_ids();
  ASSERT_EQ(forward_ids.size(), (size_t)2);
  ASSERT_EQ(forward_ids[0], (size_t)1);
  ASSERT_EQ(forward_ids[1], (size_t)2);
  auto &index_ids = schema->selected_index_ids();
  ASSERT_EQ(index_ids.size(), (size_t)2);
  ASSERT_EQ(index_ids[0], (size_t)4);
  ASSERT_EQ(index_ids[1], (size_t)5);

  // get table snapshot
  MysqlResultWrapperPtr snapshot_result = BuildSnapshotResult();
  EXPECT_CALL(*connector_, execute_query(_, _, _))
      .Times(3)
      .WillOnce(testing::Return(0))
      .WillOnce(
          Invoke([&snapshot_result](const std::string &,
                                    MysqlResultWrapperPtr *out, bool) -> int {
            *out = snapshot_result;
            return 0;
          }))
      .WillOnce(testing::Return(0))
      .RetiresOnSaturation();
  std::string file_name;
  uint64_t position;
  ret = fetcher_->get_table_snapshot(table_name_, &file_name, &position);
  ASSERT_EQ(ret, 0);
  ASSERT_EQ(file_name, "binlog.000001");
  ASSERT_EQ(position, (uint64_t)10240);
}

TEST_F(InfoFetcherTest, TestGetTableSchemaWithExecuteQueryFailed) {
  // build schema result
  MockMysqlResultWrapperPtr result1 = BuildQueryCollationResult();
  EXPECT_CALL(*connector_, execute_query(_, _, _))
      .Times(2)
      .WillOnce(Invoke([&result1](const std::string &,
                                  MysqlResultWrapperPtr *out, bool) -> int {
        *out = result1;
        return 0;
      }))
      .WillOnce(Invoke([](const std::string &, MysqlResultWrapperPtr *out,
                          bool) -> int { return 1; }))
      .RetiresOnSaturation();
  // execute query failed
  TableSchemaPtr schema;
  int ret = fetcher_->get_table_schema(table_name_, &schema);
  ASSERT_EQ(ret, 1);
}

TEST_F(InfoFetcherTest, TestGetTableSchemaWithGetCollationInfoFailed) {
  // build schema result
  MockMysqlResultWrapperPtr result1 = BuildQueryCollationResult();
  EXPECT_CALL(*connector_, execute_query(_, _, _))
      .Times(1)
      .WillOnce(Invoke([&result1](const std::string &,
                                  MysqlResultWrapperPtr *out, bool) -> int {
        *out = result1;
        return 1;
      }))
      .RetiresOnSaturation();
  // execute query failed
  TableSchemaPtr schema;
  int ret = fetcher_->get_table_schema(table_name_, &schema);
  ASSERT_EQ(ret, ErrorCode_ExecuteMysql);
}

TEST_F(InfoFetcherTest, TestGetTableSchemaWithParseTableSchemaFailed) {
  MockMysqlResultWrapperPtr result = BuildQuerySchemaResult();
  MockMysqlResultWrapperPtr result1 = BuildQueryCollationResult();
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
  // parse table schema failed
  TableSchemaPtr schema;
  fetcher_->selected_fields_->index_fields_.push_back("invalid_column");
  int ret = fetcher_->get_table_schema(table_name_, &schema);
  ASSERT_EQ(ret, ErrorCode_InvalidCollectionConfig);
}

TEST_F(InfoFetcherTest, TestGetTableSchemaSuccess) {
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
  // success
  TableSchemaPtr schema;
  int ret = fetcher_->get_table_schema(table_name_, &schema);
  ASSERT_EQ(ret, 0);
}

TEST_F(InfoFetcherTest, TestGetCollationInfoSuccess) {
  MockMysqlResultWrapperPtr result1 = BuildQueryCollationResult();
  EXPECT_CALL(*connector_, execute_query(_, _, _))
      .Times(1)
      .WillOnce(Invoke([&result1](const std::string &,
                                  MysqlResultWrapperPtr *out, bool) -> int {
        *out = result1;
        return 0;
      }))
      .RetiresOnSaturation();
  std::map<std::string, std::string> kv;
  int ret = fetcher_->get_collation_info("t1", kv);
  ASSERT_EQ(ret, 0);
  EXPECT_EQ(kv.size(), (size_t)7);
  EXPECT_EQ(kv["id"], "");
  EXPECT_EQ(kv["name"], "utf8_general_ci");
  EXPECT_EQ(kv["age"], "");
  EXPECT_EQ(kv["score"], "utf8_general_ci");
  EXPECT_EQ(kv["vector1"], "utf8_general_ci");
  EXPECT_EQ(kv["vector2"], "utf8_general_ci");
  EXPECT_EQ(kv["vector3"], "utf8_general_ci");
}

TEST_F(InfoFetcherTest, TestGetCollationInfoWithExecuteFailed) {
  MockMysqlResultWrapperPtr result1 = BuildQueryCollationResult();
  EXPECT_CALL(*connector_, execute_query(_, _, _))
      .Times(1)
      .WillOnce(Invoke([&result1](const std::string &,
                                  MysqlResultWrapperPtr *out, bool) -> int {
        *out = result1;
        return 1;
      }))
      .RetiresOnSaturation();
  std::map<std::string, std::string> kv;
  int ret = fetcher_->get_collation_info("t1", kv);
  ASSERT_EQ(ret, ErrorCode_ExecuteMysql);
}

TEST_F(InfoFetcherTest, TestGetCollationInfoWithInvalidResultFailed) {
  MockMysqlResultWrapperPtr result1 = BuildInvalidQueryCollationResult();
  EXPECT_CALL(*connector_, execute_query(_, _, _))
      .Times(1)
      .WillOnce(Invoke([&result1](const std::string &,
                                  MysqlResultWrapperPtr *out, bool) -> int {
        *out = result1;
        return 0;
      }))
      .RetiresOnSaturation();
  std::map<std::string, std::string> kv;
  int ret = fetcher_->get_collation_info("t1", kv);
  ASSERT_EQ(ret, ErrorCode_InvalidMysqlResult);
}

TEST_F(InfoFetcherTest, TestGetTableSnapshotSuccess) {
  // get table snapshot
  MysqlResultWrapperPtr snapshot_result = BuildSnapshotResult();
  EXPECT_CALL(*connector_, execute_query(_, _, _))
      .Times(3)
      .WillOnce(testing::Return(0))
      .WillOnce(
          Invoke([&snapshot_result](const std::string &,
                                    MysqlResultWrapperPtr *out, bool) -> int {
            *out = snapshot_result;
            return 0;
          }))
      .WillOnce(testing::Return(0))
      .RetiresOnSaturation();
  std::string file_name;
  uint64_t position;
  int ret = fetcher_->get_table_snapshot(table_name_, &file_name, &position);
  ASSERT_EQ(ret, 0);
  ASSERT_EQ(file_name, "binlog.000001");
  ASSERT_EQ(position, (uint64_t)10240);
}

TEST_F(InfoFetcherTest, TestGetTableSnapshotWithLockTableFailed) {
  // get table snapshot
  EXPECT_CALL(*connector_, execute_query(_, _, _))
      .Times(1)
      .WillOnce(testing::Return(1))
      .RetiresOnSaturation();
  std::string file_name;
  uint64_t position;
  int ret = fetcher_->get_table_snapshot(table_name_, &file_name, &position);
  ASSERT_EQ(ret, ErrorCode_ExecuteMysql);
}

TEST_F(InfoFetcherTest, TestGetTableSnapshotWithGetInternalFailed) {
  // get table snapshot
  MysqlResultWrapperPtr snapshot_result = BuildSnapshotResult();
  EXPECT_CALL(*connector_, execute_query(_, _, _))
      .Times(3)
      .WillOnce(testing::Return(0))
      .WillOnce(
          Invoke([&snapshot_result](const std::string &,
                                    MysqlResultWrapperPtr *out, bool) -> int {
            *out = snapshot_result;
            return 1;
          }))
      .WillOnce(testing::Return(0))
      .RetiresOnSaturation();
  std::string file_name;
  uint64_t position;
  int ret = fetcher_->get_table_snapshot(table_name_, &file_name, &position);
  ASSERT_EQ(ret, ErrorCode_ExecuteMysql);
}

TEST_F(InfoFetcherTest, TestGetTableSnapshotInternalSuccess) {
  // get table snapshot
  MysqlResultWrapperPtr snapshot_result = BuildSnapshotResult();
  EXPECT_CALL(*connector_, execute_query(_, _, _))
      .Times(1)
      .WillOnce(
          Invoke([&snapshot_result](const std::string &,
                                    MysqlResultWrapperPtr *out, bool) -> int {
            *out = snapshot_result;
            return 0;
          }))
      .RetiresOnSaturation();
  std::string file_name;
  uint64_t position;
  int ret = fetcher_->get_table_snapshot_internal(&file_name, &position);
  ASSERT_EQ(ret, 0);
  ASSERT_EQ(file_name, "binlog.000001");
  ASSERT_EQ(position, (uint64_t)10240);
}

TEST_F(InfoFetcherTest, TestGetTableSnapshotInternalWithExecuteQueryFailed) {
  // get table snapshot
  MysqlResultWrapperPtr snapshot_result = BuildSnapshotResult();
  EXPECT_CALL(*connector_, execute_query(_, _, _))
      .Times(1)
      .WillOnce(
          Invoke([&snapshot_result](const std::string &,
                                    MysqlResultWrapperPtr *out, bool) -> int {
            *out = snapshot_result;
            return 1;
          }))
      .RetiresOnSaturation();
  std::string file_name;
  uint64_t position;
  int ret = fetcher_->get_table_snapshot_internal(&file_name, &position);
  ASSERT_EQ(ret, ErrorCode_ExecuteMysql);
}

TEST_F(InfoFetcherTest, TestGetTableSnapshotInternalWithInvaildRowsResult) {
  // get table snapshot
  MysqlResultWrapperPtr snapshot_result = BuildInvalidRowsSnapshotResult();
  EXPECT_CALL(*connector_, execute_query(_, _, _))
      .Times(1)
      .WillOnce(
          Invoke([&snapshot_result](const std::string &,
                                    MysqlResultWrapperPtr *out, bool) -> int {
            *out = snapshot_result;
            return 0;
          }))
      .RetiresOnSaturation();
  std::string file_name;
  uint64_t position;
  int ret = fetcher_->get_table_snapshot_internal(&file_name, &position);
  ASSERT_EQ(ret, ErrorCode_InvalidMysqlResult);
}

TEST_F(InfoFetcherTest, TestGetTableSnapshotInternalWithEmptyRow) {
  // get table snapshot
  MysqlResultWrapperPtr snapshot_result = BuildEmptySnapshotResult();
  EXPECT_CALL(*connector_, execute_query(_, _, _))
      .Times(1)
      .WillOnce(
          Invoke([&snapshot_result](const std::string &,
                                    MysqlResultWrapperPtr *out, bool) -> int {
            *out = snapshot_result;
            return 0;
          }))
      .RetiresOnSaturation();
  std::string file_name;
  uint64_t position;
  int ret = fetcher_->get_table_snapshot_internal(&file_name, &position);
  ASSERT_EQ(ret, ErrorCode_InvalidMysqlResult);
}

TEST_F(InfoFetcherTest, TestGetTableSnapshotInternalWithInvalidFieldsNum) {
  // get table snapshot
  MysqlResultWrapperPtr snapshot_result = BuildInvalidFieldSnapshotResult();
  EXPECT_CALL(*connector_, execute_query(_, _, _))
      .Times(1)
      .WillOnce(
          Invoke([&snapshot_result](const std::string &,
                                    MysqlResultWrapperPtr *out, bool) -> int {
            *out = snapshot_result;
            return 0;
          }))
      .RetiresOnSaturation();
  std::string file_name;
  uint64_t position;
  int ret = fetcher_->get_table_snapshot_internal(&file_name, &position);
  ASSERT_EQ(ret, ErrorCode_InvalidMysqlResult);
}

TEST_F(InfoFetcherTest, TestGetTableSnapshotInternalWithInvalidResult) {
  // get table snapshot
  MysqlResultWrapperPtr snapshot_result = BuildInvalidSnapshotResult();
  EXPECT_CALL(*connector_, execute_query(_, _, _))
      .Times(1)
      .WillOnce(
          Invoke([&snapshot_result](const std::string &,
                                    MysqlResultWrapperPtr *out, bool) -> int {
            *out = snapshot_result;
            return 0;
          }))
      .RetiresOnSaturation();
  std::string file_name;
  uint64_t position;
  int ret = fetcher_->get_table_snapshot_internal(&file_name, &position);
  ASSERT_EQ(ret, ErrorCode_InvalidMysqlResult);
}
