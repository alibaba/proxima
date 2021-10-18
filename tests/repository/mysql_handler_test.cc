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
#include "repository/binlog/mysql_handler.h"
#include "repository/binlog/table_reader.h"
#include "repository/repository_common/error_code.h"
#include "event_builder.h"
#include "mock_mysql_connector.h"
#include "mysql_result_builder.h"

#undef private

using namespace proxima::be;
using namespace proxima::be::repository;

class MysqlHandlerTest : public testing::Test {
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
    ctx_.seq_id = 1;
    InitHandler();
  }

  void TearDown() {}

  void InitHandler() {
    builder_.BuildCollectionConfig();
    handler_ = std::make_shared<MysqlHandler>(builder_.config_, mgr_);
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

 protected:
  MysqlHandlerPtr handler_{};
  MysqlConnectorManagerPtr mgr_{};
  MockMysqlConnectorPtr connector1_{};
  MockMysqlConnectorPtr connector2_{};
  InfoFetcherPtr fetcher_{};
  std::vector<enum_field_types> column_types_{};
  std::vector<int32_t> column_metas_{};
  LsnContext ctx_;
  MysqlResultBuilder builder_{};
};

TEST_F(MysqlHandlerTest, TestGeneral) {
  ScanMode mode = ScanMode::FULL;

  // for validator
  MockMysqlResultWrapperPtr version_result =
      builder_.BuildSelectVersionResult();

  MockMysqlResultWrapperPtr binlog_result = builder_.BuildShowBinlogResult();
  MockMysqlResultWrapperPtr schema_result = builder_.BuildQuerySchemaResult();
  MockMysqlResultWrapperPtr result1 = builder_.BuildQueryCollationResult();
  MockMysqlResultWrapperPtr db_result = builder_.BuildSelectDbResult();

  MockMysqlResultWrapperPtr scan_result = builder_.BuildScanTableResult();
  // for fetcher
  ailego::Uri test_uri = builder_.uri_;
  EXPECT_CALL(*connector1_, execute_query(_, _, _))
      .Times(3)
      .WillOnce(
          Invoke([&version_result](const std::string &,
                                   MysqlResultWrapperPtr *out, bool) -> int {
            *out = version_result;
            return 0;
          }))
      .WillOnce(
          Invoke([&binlog_result](const std::string &,
                                  MysqlResultWrapperPtr *out, bool) -> int {
            *out = binlog_result;
            return 0;
          }))
      .WillOnce(Invoke([&db_result](const std::string &,
                                    MysqlResultWrapperPtr *out, bool) -> int {
        *out = db_result;
        return 0;
      }))
      .RetiresOnSaturation();

  EXPECT_CALL(*connector1_, uri())
      .WillOnce(
          Invoke([&test_uri]() -> const ailego::Uri & { return test_uri; }))
      .RetiresOnSaturation();

  EXPECT_CALL(*connector2_, uri())
      .WillOnce(
          Invoke([&test_uri]() -> const ailego::Uri & { return test_uri; }))
      .RetiresOnSaturation();

  EXPECT_CALL(*connector2_, execute_query(_, _, _))
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

  int ret = handler_->init(mode);
  ASSERT_EQ(ret, 0);

  EXPECT_CALL(*connector1_, execute_query(_, _, _))
      .Times(1)
      .WillOnce(Invoke([&scan_result](const std::string &,
                                      MysqlResultWrapperPtr *out, bool) -> int {
        *out = scan_result;
        return 0;
      }))
      .RetiresOnSaturation();
  ret = handler_->start(ctx_);
  ASSERT_EQ(ret, 0);

  // RowData row_data;
  proto::WriteRequest::Row row_data;
  LsnContext ctx;
  ret = handler_->get_next_row_data(&row_data, &ctx);
  ASSERT_EQ(ret, 0);
  ASSERT_EQ(row_data.primary_key(), (uint64_t)1);
  ASSERT_EQ(ctx.status, RowDataStatus::NORMAL);
  row_data.Clear();
  ret = handler_->get_next_row_data(&row_data, &ctx);
  ASSERT_EQ(ret, 0);
  ASSERT_EQ(row_data.primary_key(), (uint64_t)2);
  ASSERT_EQ(ctx.status, RowDataStatus::NORMAL);
  row_data.Clear();
  ret = handler_->get_next_row_data(&row_data, &ctx);
  ASSERT_EQ(ret, 0);
  ASSERT_EQ(ctx.status, RowDataStatus::NO_MORE_DATA);
  row_data.Clear();

  // proxima::be::agent::proto::CollectionDataset dataset;
  proto::WriteRequest::RowMeta meta;
  ret = handler_->get_fields_meta(&meta);
  ASSERT_EQ(ret, 0);
  // ASSERT_EQ(dataset.index_tuples_size(), 2);
  // ASSERT_EQ(dataset.index_tuples(0).field_name(), "vector1");
  // ASSERT_EQ(dataset.index_tuples(0).field_type(),
  // GenericValueMeta::FT_STRING);
  // ASSERT_EQ(dataset.index_tuples(1).field_name(), "vector2");
  // ASSERT_EQ(dataset.index_tuples(1).field_type(),
  // GenericValueMeta::FT_STRING); ASSERT_EQ(dataset.forward_tuples_size(), 2);
  // ASSERT_EQ(dataset.forward_tuples(0).field_name(), "name");
  // ASSERT_EQ(dataset.forward_tuples(1).field_name(), "age");
  ASSERT_EQ(meta.forward_column_names(0), "name");
  ASSERT_EQ(meta.forward_column_names(1), "age");
  ASSERT_EQ(meta.index_column_metas(0).column_name(), "vector1");
  ASSERT_EQ(meta.index_column_metas(1).column_name(), "vector2");
  // reset status
  EXPECT_CALL(*connector2_, uri())
      .WillOnce(
          Invoke([&test_uri]() -> const ailego::Uri & { return test_uri; }))
      .RetiresOnSaturation();

  result1->reset();
  EXPECT_CALL(*connector2_, execute_query(_, _, _))
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
  // first set check sum and update lsn info
  EXPECT_CALL(*connector1_, execute_query(_, _, _))
      .Times(2)
      .WillOnce(::testing::Return(0))
      .WillOnce(::testing::Return(0))
      .RetiresOnSaturation();
  // second request dump
  EXPECT_CALL(*connector1_, execute_simple_command(_, _, _))
      .WillOnce(::testing::Return(0))
      .RetiresOnSaturation();

  mode = ScanMode::INCREMENTAL;
  ctx_.file_name = "binlog.000003";
  ctx_.position = 4;
  ret = handler_->reset_status(mode, builder_.config_, ctx_);
  ASSERT_EQ(ret, 0);

  // fetch data
  std::string table_map_str = builder_.BuildTableMapEventStr();
  std::vector<std::string> values = {"1",       "name1",   "30",     "123.456",
                                     "1,2,3,4", "1,2,3,5", "1,2,3,6"};
  std::string write_rows_str = builder_.BuildWriteRowsEventStr(values);
  EXPECT_CALL(*connector1_, client_safe_read(_))
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

  EXPECT_CALL(*connector1_, data())
      .Times(2)
      .WillOnce(Invoke([&table_map_str]() -> const void * {
        return (const void *)table_map_str.c_str();
      }))
      .WillOnce(Invoke([&write_rows_str]() -> const void * {
        return (const void *)write_rows_str.c_str();
      }))
      .RetiresOnSaturation();

  row_data.Clear();
  ret = handler_->get_next_row_data(&row_data, &ctx);
  ASSERT_EQ(ret, 0);
  ASSERT_EQ(ctx.status, RowDataStatus::NORMAL);
  ASSERT_EQ(row_data.primary_key(), (uint64_t)1);
  ASSERT_EQ(row_data.forward_column_values().values(0).string_value(), "name1");
  ASSERT_EQ(row_data.forward_column_values().values(1).int32_value(), 30);
  ASSERT_EQ(row_data.index_column_values().values(0).string_value(), "1,2,3,4");
  ASSERT_EQ(row_data.index_column_values().values(1).string_value(), "1,2,3,5");
}

TEST_F(MysqlHandlerTest, TestInit) {
  ScanMode mode = ScanMode::FULL;

  // connection manager init failed
  {
    auto config1 = builder_.config_;
    config1.mutable_repository_config()->mutable_database()->set_connection_uri(
        "invalid");
    MysqlHandlerPtr handler = std::make_shared<MysqlHandler>(config1);
    int ret = handler->init(mode);
    ASSERT_EQ(ret, ErrorCode_InvalidArgument);
  }

  // validate mysql failed
  {
    MysqlHandlerPtr handler = std::make_shared<MysqlHandler>(builder_.config_);
    int ret = handler->init(mode);
    ASSERT_EQ(ret, ErrorCode_RuntimeError);
  }

  // success
  {
    MysqlHandlerPtr handler =
        std::make_shared<MysqlHandler>(builder_.config_, mgr_);

    // for validator
    MockMysqlResultWrapperPtr version_result =
        builder_.BuildSelectVersionResult();

    MockMysqlResultWrapperPtr binlog_result = builder_.BuildShowBinlogResult();
    MockMysqlResultWrapperPtr db_result = builder_.BuildSelectDbResult();
    MockMysqlResultWrapperPtr schema_result = builder_.BuildQuerySchemaResult();
    MockMysqlResultWrapperPtr result1 = builder_.BuildQueryCollationResult();
    // for fetcher
    ailego::Uri test_uri = builder_.uri_;
    EXPECT_CALL(*connector1_, execute_query(_, _, _))
        .Times(3)
        .WillOnce(
            Invoke([&version_result](const std::string &,
                                     MysqlResultWrapperPtr *out, bool) -> int {
              *out = version_result;
              return 0;
            }))
        .WillOnce(
            Invoke([&binlog_result](const std::string &,
                                    MysqlResultWrapperPtr *out, bool) -> int {
              *out = binlog_result;
              return 0;
            }))
        .WillOnce(Invoke([&db_result](const std::string &,
                                      MysqlResultWrapperPtr *out, bool) -> int {
          *out = db_result;
          return 0;
        }))
        .RetiresOnSaturation();

    EXPECT_CALL(*connector1_, uri())
        .WillOnce(
            Invoke([&test_uri]() -> const ailego::Uri & { return test_uri; }))
        .RetiresOnSaturation();

    EXPECT_CALL(*connector2_, uri())
        .WillOnce(
            Invoke([&test_uri]() -> const ailego::Uri & { return test_uri; }))
        .RetiresOnSaturation();

    EXPECT_CALL(*connector2_, execute_query(_, _, _))
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

    int ret = handler_->init(mode);
    ASSERT_EQ(ret, 0);

    ret = handler_->init(mode);
    ASSERT_EQ(ret, ErrorCode_RepeatedInitialized);
  }
}

TEST_F(MysqlHandlerTest, TestStart) {
  ScanMode mode = ScanMode::FULL;

  // connection manager init failed
  {
    auto config1 = builder_.config_;
    config1.mutable_repository_config()->mutable_database()->set_connection_uri(
        "invalid");
    MysqlHandlerPtr handler = std::make_shared<MysqlHandler>(config1);
    int ret = handler->init(mode);
    ASSERT_EQ(ret, ErrorCode_InvalidArgument);
  }

  // validate mysql failed
  {
    MysqlHandlerPtr handler = std::make_shared<MysqlHandler>(builder_.config_);
    int ret = handler->init(mode);
    ASSERT_EQ(ret, ErrorCode_RuntimeError);
  }

  // success
  {
    MysqlHandlerPtr handler =
        std::make_shared<MysqlHandler>(builder_.config_, mgr_);

    // for validator
    MockMysqlResultWrapperPtr version_result =
        builder_.BuildSelectVersionResult();

    MockMysqlResultWrapperPtr binlog_result = builder_.BuildShowBinlogResult();
    MockMysqlResultWrapperPtr db_result = builder_.BuildSelectDbResult();
    MockMysqlResultWrapperPtr schema_result = builder_.BuildQuerySchemaResult();
    MockMysqlResultWrapperPtr result1 = builder_.BuildQueryCollationResult();
    MockMysqlResultWrapperPtr scan_result = builder_.BuildScanTableResult();

    // for fetcher
    ailego::Uri test_uri = builder_.uri_;
    EXPECT_CALL(*connector1_, execute_query(_, _, _))
        .Times(3)
        .WillOnce(
            Invoke([&version_result](const std::string &,
                                     MysqlResultWrapperPtr *out, bool) -> int {
              *out = version_result;
              return 0;
            }))
        .WillOnce(
            Invoke([&binlog_result](const std::string &,
                                    MysqlResultWrapperPtr *out, bool) -> int {
              *out = binlog_result;
              return 0;
            }))
        .WillOnce(Invoke([&db_result](const std::string &,
                                      MysqlResultWrapperPtr *out, bool) -> int {
          *out = db_result;
          return 0;
        }))
        .RetiresOnSaturation();

    EXPECT_CALL(*connector1_, uri())
        .WillOnce(
            Invoke([&test_uri]() -> const ailego::Uri & { return test_uri; }))
        .RetiresOnSaturation();

    EXPECT_CALL(*connector2_, uri())
        .WillOnce(
            Invoke([&test_uri]() -> const ailego::Uri & { return test_uri; }))
        .RetiresOnSaturation();

    EXPECT_CALL(*connector2_, execute_query(_, _, _))
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

    int ret = handler_->init(mode);
    ASSERT_EQ(ret, 0);

    EXPECT_CALL(*connector1_, execute_query(_, _, _))
        .Times(1)
        .WillOnce(
            Invoke([&scan_result](const std::string &,
                                  MysqlResultWrapperPtr *out, bool) -> int {
              *out = scan_result;
              return 0;
            }))
        .RetiresOnSaturation();
    ret = handler_->start(ctx_);
    ASSERT_EQ(ret, 0);
  }
}

TEST_F(MysqlHandlerTest, TestValidateMysql) {
  // validator init failed
  MysqlConnectorManagerPtr mgr;
  MysqlHandler handler1(builder_.config_, mgr);
  int ret = handler1.validate_mysql();
  ASSERT_EQ(ret, ErrorCode_RuntimeError);

  MysqlHandler handler(builder_.config_, mgr_);
  MockMysqlResultWrapperPtr version_result =
      builder_.BuildSelectVersionResult();
  MockMysqlResultWrapperPtr binlog_result = builder_.BuildShowBinlogResult();
  MockMysqlResultWrapperPtr db_result = builder_.BuildSelectDbResult();

  // validate version failed
  {
    EXPECT_CALL(*connector1_, execute_query(_, _, _))
        .Times(1)
        .WillOnce(testing::Return(1))
        .RetiresOnSaturation();
    ret = handler.validate_mysql();
    ASSERT_EQ(ret, ErrorCode_UnsupportedMysqlVersion);
  }

  // validate binlog format failed
  {
    EXPECT_CALL(*connector2_, execute_query(_, _, _))
        .Times(2)
        .WillOnce(
            Invoke([&version_result](const std::string &,
                                     MysqlResultWrapperPtr *out, bool) -> int {
              *out = version_result;
              return 0;
            }))
        .WillOnce(
            Invoke([&binlog_result](const std::string &,
                                    MysqlResultWrapperPtr *out, bool) -> int {
              *out = binlog_result;
              return 2;
            }))
        .RetiresOnSaturation();
    ret = handler.validate_mysql();
    ASSERT_EQ(ret, ErrorCode_UnsupportedBinlogFormat);
  }

  // validate database exist failed
  ailego::Uri test_uri = builder_.uri_;
  EXPECT_CALL(*connector1_, uri())
      .WillOnce(
          Invoke([&test_uri]() -> const ailego::Uri & { return test_uri; }))
      .RetiresOnSaturation();
  {
    version_result->reset();
    binlog_result->reset();
    EXPECT_CALL(*connector1_, execute_query(_, _, _))
        .Times(3)
        .WillOnce(
            Invoke([&version_result](const std::string &,
                                     MysqlResultWrapperPtr *out, bool) -> int {
              *out = version_result;
              return 0;
            }))
        .WillOnce(
            Invoke([&binlog_result](const std::string &,
                                    MysqlResultWrapperPtr *out, bool) -> int {
              *out = binlog_result;
              return 0;
            }))
        .WillOnce(Invoke([&db_result](const std::string &,
                                      MysqlResultWrapperPtr *out, bool) -> int {
          *out = db_result;
          return 3;
        }))
        .RetiresOnSaturation();
    ret = handler.validate_mysql();
    ASSERT_EQ(ret, ErrorCode_InvalidCollectionConfig);
  }

  // success
  version_result->reset();
  binlog_result->reset();
  db_result->reset();

  EXPECT_CALL(*connector2_, uri())
      .WillOnce(
          Invoke([&test_uri]() -> const ailego::Uri & { return test_uri; }))
      .RetiresOnSaturation();
  EXPECT_CALL(*connector2_, execute_query(_, _, _))
      .Times(3)
      .WillOnce(
          Invoke([&version_result](const std::string &,
                                   MysqlResultWrapperPtr *out, bool) -> int {
            *out = version_result;
            return 0;
          }))
      .WillOnce(
          Invoke([&binlog_result](const std::string &,
                                  MysqlResultWrapperPtr *out, bool) -> int {
            *out = binlog_result;
            return 0;
          }))
      .WillOnce(Invoke([&db_result](const std::string &,
                                    MysqlResultWrapperPtr *out, bool) -> int {
        *out = db_result;
        return 0;
      }))
      .RetiresOnSaturation();

  ret = handler.validate_mysql();
  ASSERT_EQ(ret, 0);
}

TEST_F(MysqlHandlerTest, TestGetTableSnapshot) {
  // not init
  std::string binlog_file;
  uint64_t position;
  int ret = handler_->get_table_snapshot(&binlog_file, &position);
  ASSERT_EQ(ret, ErrorCode_NoInitialized);


  //
  // for validator
  MockMysqlResultWrapperPtr version_result =
      builder_.BuildSelectVersionResult();

  MockMysqlResultWrapperPtr binlog_result = builder_.BuildShowBinlogResult();
  MockMysqlResultWrapperPtr schema_result = builder_.BuildQuerySchemaResult();
  MockMysqlResultWrapperPtr result1 = builder_.BuildQueryCollationResult();
  MockMysqlResultWrapperPtr db_result = builder_.BuildSelectDbResult();

  MockMysqlResultWrapperPtr scan_result = builder_.BuildScanTableResult();
  // for fetcher
  ailego::Uri test_uri = builder_.uri_;
  EXPECT_CALL(*connector1_, execute_query(_, _, _))
      .Times(3)
      .WillOnce(
          Invoke([&version_result](const std::string &,
                                   MysqlResultWrapperPtr *out, bool) -> int {
            *out = version_result;
            return 0;
          }))
      .WillOnce(
          Invoke([&binlog_result](const std::string &,
                                  MysqlResultWrapperPtr *out, bool) -> int {
            *out = binlog_result;
            return 0;
          }))
      .WillOnce(Invoke([&db_result](const std::string &,
                                    MysqlResultWrapperPtr *out, bool) -> int {
        *out = db_result;
        return 0;
      }))
      .RetiresOnSaturation();

  EXPECT_CALL(*connector1_, uri())
      .WillOnce(
          Invoke([&test_uri]() -> const ailego::Uri & { return test_uri; }))
      .RetiresOnSaturation();

  EXPECT_CALL(*connector2_, uri())
      .WillOnce(
          Invoke([&test_uri]() -> const ailego::Uri & { return test_uri; }))
      .RetiresOnSaturation();

  EXPECT_CALL(*connector2_, execute_query(_, _, _))
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

  ret = handler_->init(ScanMode::FULL);
  ASSERT_EQ(ret, 0);

  EXPECT_CALL(*connector1_, execute_query(_, _, _))
      .Times(1)
      .WillOnce(Invoke([&scan_result](const std::string &,
                                      MysqlResultWrapperPtr *out, bool) -> int {
        *out = scan_result;
        return 0;
      }))
      .RetiresOnSaturation();
  ret = handler_->start(ctx_);
  ASSERT_EQ(ret, 0);


  // get table snapshot
  MysqlResultWrapperPtr snapshot_result = BuildSnapshotResult();
  EXPECT_CALL(*connector2_, execute_query(_, _, _))
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

  // success
  ret = handler_->get_table_snapshot(&binlog_file, &position);
  ASSERT_EQ(ret, 0);
  ASSERT_EQ(binlog_file, "binlog.000001");
  ASSERT_EQ(position, (uint64_t)10240);

  // get info fetcher failed
  TableReader *reader =
      (TableReader *)((MysqlHandler *)handler_.get())->mysql_reader_.get();
  reader->info_fetcher_ = nullptr;
  ret = handler_->get_table_snapshot(&binlog_file, &position);
  ASSERT_EQ(ret, ErrorCode_RuntimeError);
}


/////////////////////////////////


// TEST_F(MysqlHandlerTest, TestSimple) {
//     bool flag = true;
//     CollectionConfig config;
//     config.set_connection_uri("mysql://root:root@11.139.203.151:3306/mytest");
//     config.set_repository_table("mt933");
//     config.add_forward_columns("name");
//     config.add_forward_columns("age");
//     config.add_forward_columns("score");
//     config.add_forward_columns("f1");
//     config.add_forward_columns("f2");
//     config.add_forward_columns("f3");
//     config.add_forward_columns("f4");
//     config.add_forward_columns("f5");
//     config.add_forward_columns("f6");
//     config.add_forward_columns("f10");
//     config.add_forward_columns("f11");
//     config.add_forward_columns("f12");
//     config.add_forward_columns("f13");
//     if (flag) {
//       config.add_forward_columns("f14");
//     }

//     config.add_index_columns("f9");

//     MysqlHandler handler(config);
//     LsnContext ctx;
//     ctx.seq_id = 0;

//     int ret = handler.init(ScanMode::FULL, ctx);
//     ASSERT_EQ(ret, 0);

//     ret = handler.get_table_snapshot(&(ctx.file_name), &(ctx.position));
//     printf("BinLog: %s %lu\n", ctx.file_name.c_str(), ctx.position);

//     uint64_t idx = 1;
//     while (true) {
//         RowData row_data;
//         ret = handler.get_next_row_data(&row_data, &ctx);
//         ASSERT_EQ(ret, 0);
//         if (ctx.status == RowDataStatus::NORMAL) {
//             EXPECT_EQ(row_data.primary_key(), idx);
//             EXPECT_EQ(ctx.seq_id, idx);
//             printf("seq_id: %lu\n", ctx.seq_id);
//             idx += 1;
//         } else {
//             break;
//         }
//         printf("%s\n", row_data.ShortDebugString().c_str());
//     }

//     ctx.position = 4;
//     ctx.file_name = "binlog.000001";

//     ret = handler.reset_status(ScanMode::INCREMENTAL, config, ctx);
//     ASSERT_EQ(ret, 0);

//     printf("////////////////////////////////////\n");
//     idx = 12;
//     int32_t retry_times = 3;
//     int32_t times = 0;
//     while (true) {
//         RowData row_data;
//         ret = handler.get_next_row_data(&row_data, &ctx);
//         if (ret != 0) {
//             continue;
//         }
//         ASSERT_EQ(ret, 0);
//         if (ctx.status == RowDataStatus::NORMAL) {
//             EXPECT_EQ(row_data.primary_key(), idx);
//             printf("file_name: %s position: %lu\n",
//                    ctx.file_name.c_str(), ctx.position);
//             idx += 1;
//         } else if (ctx.status == RowDataStatus::SCHEMA_CHANGED) {
//             continue;
//         } else {
//             // sleep(1);
//             // printf("sleep...\n");
//             // // if (++times < retry_times) {
//             // continue;
//             // // } else {
//             // //     break;
//             // // }
//             // break;
//             continue;
//         }
//         // printf("%s\n", row_data.ShortDebugString().c_str());
//         // printf("primary_key: %lu, %s %d %f %s %d %lf %s %s %d %s %s %s %s
//         %u %s\n",
//         //        row_data.primary_key(),
//         //        row_data.forward_columns().values(0).bytes_value().c_str(),
//         //        row_data.forward_columns().values(1).int32_value(),
//         //        row_data.forward_columns().values(2).float_value(),
//         //        row_data.forward_columns().values(3).bytes_value().c_str(),
//         //        row_data.forward_columns().values(4).int32_value(),
//         //        row_data.forward_columns().values(5).double_value(),
//         // row_data.forward_columns().values(6).string_value().c_str(),
//         // row_data.forward_columns().values(7).string_value().c_str(),
//         //        row_data.forward_columns().values(8).int32_value(),
//         // row_data.forward_columns().values(9).string_value().c_str(),
//         // row_data.forward_columns().values(10).string_value().c_str(),
//         // row_data.forward_columns().values(11).string_value().c_str(),
//         // row_data.forward_columns().values(12).string_value().c_str(),
//         //        row_data.forward_columns().values(13).uint32_value(),
//         //        row_data.index_columns(0).bytes_value().c_str());
//     }
// }
