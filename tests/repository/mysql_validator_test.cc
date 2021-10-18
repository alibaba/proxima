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
#include "repository/binlog/mysql_validator.h"
#include "mock_mysql_connector.h"
#undef private

#include "repository/repository_common/error_code.h"

using namespace ::proxima::be;
using namespace proxima::be::repository;

class MysqlValidatorTest : public testing::Test {
 protected:
  void SetUp() {
    mgr_ = std::make_shared<MysqlConnectorManager>();
    ASSERT_TRUE(mgr_);
    connector_ = std::make_shared<MockMysqlConnector>();
    ASSERT_TRUE(connector_);
    mgr_->put(connector_);
  }

  void TearDown() {}

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

  MockMysqlResultWrapperPtr BuildSelectDbResult() {
    MockMysqlResultWrapperPtr result =
        std::make_shared<MockMysqlResultWrapper>();
    result->append_field_meta("SCHEMA_NAME");
    std::vector<std::string> values1 = {"mytest"};
    result->append_row_values(values1);
    return result;
  }

 protected:
  MysqlConnectorManagerPtr mgr_{};
  MockMysqlConnectorPtr connector_{};
};

TEST_F(MysqlValidatorTest, TestSimple) {
  MysqlValidator validator(mgr_);
  int ret = validator.init();
  ASSERT_EQ(ret, 0);

  MockMysqlResultWrapperPtr result = BuildSelectVersionResult();
  EXPECT_CALL(*connector_, execute_query(_, _, _))
      .WillOnce(Invoke([&result](const std::string &,
                                 MysqlResultWrapperPtr *out, bool) -> int {
        *out = result;
        return 0;
      }))
      .RetiresOnSaturation();
  ASSERT_TRUE(validator.validate_version());

  MockMysqlResultWrapperPtr result1 = BuildShowBinlogResult();
  EXPECT_CALL(*connector_, execute_query(_, _, _))
      .WillOnce(Invoke([&result1](const std::string &,
                                  MysqlResultWrapperPtr *out, bool) -> int {
        *out = result1;
        return 0;
      }))
      .RetiresOnSaturation();
  ASSERT_TRUE(validator.validate_binlog_format());

  const std::string connection_uri = "mysql://root:root@127.0.0.1:3306/mytest";
  ailego::Uri uri;
  ailego::Uri::Parse(connection_uri.c_str(), &uri);

  MockMysqlResultWrapperPtr result2 = BuildSelectDbResult();
  EXPECT_CALL(*connector_, uri())
      .WillOnce(Invoke([&uri]() -> const ailego::Uri & { return uri; }))
      .RetiresOnSaturation();
  EXPECT_CALL(*connector_, execute_query(_, _, _))
      .WillOnce(Invoke([&result2](const std::string &,
                                  MysqlResultWrapperPtr *out, bool) -> int {
        *out = result2;
        return 0;
      }))
      .RetiresOnSaturation();
  ASSERT_TRUE(validator.validate_database_exist());
}
