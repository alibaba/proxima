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

#include <mysql.h>
#include <gtest/gtest.h>
#define private public
#include "repository/binlog/binlog_event.h"
#undef private
#include "common/error_code.h"
#include "proto/common.pb.h"
#include "event_builder.h"

using namespace ::proxima::be;
using namespace proxima::be::repository;

class BinlogEventTest : public testing::Test {
 protected:
  void SetUp() {}

  void TearDown() {}

 protected:
};


TEST_F(BinlogEventTest, TestTableMapEvent) {
  uint64_t table_id = 1000;
  std::string db("test_db");
  std::string table_name("table1");
  {
    size_t column_count = 1;
    std::vector<bool> column_nulls(column_count, false);
    std::vector<enum_field_types> column_types(column_count, MYSQL_TYPE_LONG);
    std::vector<int32_t> column_metas(column_count, 0);
    std::string table_map = EventBuilder::BuildTableMapEvent(
        table_id, db, table_name, column_types, column_metas, column_nulls);
    TableMapEventPtr event =
        std::make_shared<TableMapEvent>(table_map.c_str(), table_map.size());
    ASSERT_TRUE(event->is_valid());
  }
}

TEST_F(BinlogEventTest, TestDecodeMetaData) {
  char buf[1024];
  TableMapEvent event(buf, 1);
  std::vector<ColumnInfo> &columns = event.column_info_;
  std::vector<int32_t> types = {
      MYSQL_TYPE_TINY_BLOB, MYSQL_TYPE_BLOB,       MYSQL_TYPE_MEDIUM_BLOB,
      MYSQL_TYPE_LONG_BLOB, MYSQL_TYPE_DOUBLE,     MYSQL_TYPE_FLOAT,
      MYSQL_TYPE_GEOMETRY,  MYSQL_TYPE_JSON,       MYSQL_TYPE_SET,
      MYSQL_TYPE_ENUM,      MYSQL_TYPE_STRING,     MYSQL_TYPE_BIT,
      MYSQL_TYPE_VARCHAR,   MYSQL_TYPE_NEWDECIMAL, MYSQL_TYPE_TIME2,
      MYSQL_TYPE_DATETIME2, MYSQL_TYPE_TIMESTAMP2, MYSQL_TYPE_LONG};
  columns.resize(types.size());
  for (size_t i = 0; i < types.size(); ++i) {
    columns[i].type = types[i];
    columns[i].meta = 0;
  }
  event.column_count_ = columns.size();

  std::vector<uint8_t> values;
  for (size_t i = 0; i < 8; ++i) {
    values.emplace_back(i + 1);
  }
  // string
  values.emplace_back(1);
  values.emplace_back(1);
  // bit
  values.emplace_back(2);
  values.emplace_back(1);
  // varchar
  values.emplace_back(1);
  values.emplace_back(2);
  // new decimal
  values.emplace_back(2);
  values.emplace_back(2);
  // time/datatime/timestamp
  values.emplace_back(1);
  values.emplace_back(1);
  values.emplace_back(1);

  event.decode_meta_data((const char *)values.data());

  for (size_t i = 0; i < 8; ++i) {
    ASSERT_EQ(columns[i].meta, i + 1);
  }
  ASSERT_EQ(columns[8].meta, 0);
  ASSERT_EQ(columns[9].meta, 0);
  ASSERT_EQ(columns[10].meta, 257);
  ASSERT_EQ(columns[11].meta, 258);
  ASSERT_EQ(columns[12].meta, 513);
  ASSERT_EQ(columns[13].meta, 514);
  ASSERT_EQ(columns[14].meta, 1);
  ASSERT_EQ(columns[15].meta, 1);
  ASSERT_EQ(columns[16].meta, 1);
  ASSERT_EQ(columns[17].meta, 0);
}
