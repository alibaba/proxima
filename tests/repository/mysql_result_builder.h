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
 *   \brief    Mysql result builder interface definition for bilin engine
 */

#pragma once

#include <gmock/gmock.h>
#include "proto/proxima_be.pb.h"
#include "event_builder.h"
#include "mock_mysql_connector.h"

using namespace proxima::be::repository;
using namespace ::testing;

namespace proxima {
namespace be {
namespace repository {

class MysqlResultBuilder;
using MysqlResultBuilderPtr = std::shared_ptr<MysqlResultBuilder>;

/*!
 */
class MysqlResultBuilder {
 public:
  //! Constructor
  MysqlResultBuilder() {
    connection_uri_ = "mysql://root:root@1.0.0.1:3306/mytest";
    user_ = "root";
    password_ = "root";
    ailego::Uri::Parse(connection_uri_.c_str(), &uri_);
    table_name_ = "table";
    db_ = "mytest";
    table_id_ = 1000;
  }

  //! Destructor
  ~MysqlResultBuilder() = default;

 public:
  void BuildCollectionConfig() {
    auto *repo = config_.mutable_repository_config();
    repo->set_repository_type(
        proto::CollectionConfig::RepositoryConfig::RT_DATABASE);
    repo->set_repository_name(table_name_);
    auto *database = repo->mutable_database();
    database->set_connection_uri(connection_uri_);
    database->set_table_name(table_name_);
    database->set_user(user_);
    database->set_password(password_);

    config_.add_forward_column_names("name");
    config_.add_forward_column_names("age");
    auto *index1 = config_.add_index_column_params();
    index1->set_column_name("vector1");
    auto *index2 = config_.add_index_column_params();
    index2->set_column_name("vector2");
  }

  MockMysqlResultWrapperPtr BuildSelectVersionResult() {
    MockMysqlResultWrapperPtr result =
        std::make_shared<MockMysqlResultWrapper>();
    result->append_field_meta("VERSION()");
    std::vector<std::string> values1 = {"5.7.10-log"};
    result->append_row_values(values1);
    return result;
  }

  MockMysqlResultWrapperPtr BuildShowBinlogResult() {
    MockMysqlResultWrapperPtr result =
        std::make_shared<MockMysqlResultWrapper>();
    result->append_field_meta("Variable_name");
    result->append_field_meta("Value");
    std::vector<std::string> values1 = {"binlog_format", "ROW"};
    result->append_row_values(values1);
    return result;
  }

  MockMysqlResultWrapperPtr BuildShowBinaryLogsResult() {
    MockMysqlResultWrapperPtr result =
        std::make_shared<MockMysqlResultWrapper>();
    result->append_field_meta("Log_name");
    result->append_field_meta("File_size");
    std::vector<std::string> values1 = {"binlog.000000", "12345"};
    result->append_row_values(values1);
    std::vector<std::string> values2 = {"binlog.000004", "12345"};
    result->append_row_values(values2);
    return result;
  }

  std::string BuildTableMapEventStr() {
    std::vector<bool> column_nulls(column_types_.size(), false);
    column_nulls[column_types_.size() - 1] = true;
    std::string table_map = EventBuilder::BuildTableMapEvent(
        table_id_, db_, table_name_, column_types_, column_metas_,
        column_nulls);
    return " " + table_map;
  }

  TableMapEventPtr BuildTableMapEvent() {
    std::string event_str = BuildTableMapEventStr();
    return std::make_shared<TableMapEvent>(event_str.substr(1).c_str(),
                                           event_str.size() - 1);
  }

  std::string BuildWriteRowsEventStr(std::vector<std::string> &column_values,
                                     size_t rows_count = 1) {
    TableMapEventPtr table_map = BuildTableMapEvent();
    std::vector<bool> column_nulls(column_types_.size(), false);
    std::string rows_str = EventBuilder::BuildWriteRowsEvent(
        table_id_, column_nulls, column_types_, column_values, table_map,
        WRITE_ROWS_EVENT_V1, rows_count);
    return " " + rows_str;
  }

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

  MockMysqlResultWrapperPtr BuildScanTableResult() {
    MockMysqlResultWrapperPtr result =
        std::make_shared<MockMysqlResultWrapper>();
    result->append_field_meta("id", MYSQL_TYPE_LONG, 11, 0,
                              AUTO_INCREMENT_FLAG);
    result->append_field_meta("name", MYSQL_TYPE_VAR_STRING, 100);
    result->append_field_meta("age", MYSQL_TYPE_LONG, 11);
    result->append_field_meta("vector1", MYSQL_TYPE_VAR_STRING, 1024);
    result->append_field_meta("vector2", MYSQL_TYPE_VAR_STRING, 1024);

    std::vector<std::string> values1 = {"1", "1,2,3,4", "1,2,3,5", "name1",
                                        "18"};
    result->append_row_values(values1);
    std::vector<std::string> values2 = {"2", "2,2,3,4", "2,2,3,5", "name2",
                                        "19"};
    result->append_row_values(values2);

    return result;
  }

  MockMysqlResultWrapperPtr BuildSelectDbResult() {
    MockMysqlResultWrapperPtr result =
        std::make_shared<MockMysqlResultWrapper>();
    result->append_field_meta("id", MYSQL_TYPE_LONG, 11, 0,
                              AUTO_INCREMENT_FLAG);

    std::vector<std::string> values1 = {"1"};
    result->append_row_values(values1);

    return result;
  }

 public:
  proto::CollectionConfig config_;
  // agent::proto::CollectionConfig config_;
  std::string connection_uri_{};
  std::string user_{};
  std::string password_{};
  ailego::Uri uri_{};
  std::string table_name_{};
  std::string db_{};

  uint64_t table_id_{0};
  std::vector<enum_field_types> column_types_{};
  std::vector<int32_t> column_metas_{};
};

}  // namespace repository
}  // namespace be
}  // namespace proxima
