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

//!!! undef VERSION must be after include my_global.h
#include <my_global.h>
#undef VERSION

extern "C" {
#include <decimal.h>
}

#define private public
#include "repository/binlog/field.h"
#undef private
#include "proto/common.pb.h"
#include "repository/repository_common/error_code.h"

using namespace ::proxima::be;
using namespace proxima::be::repository;

class FieldTest : public testing::Test {
 protected:
  void SetUp() {}

  void TearDown() {}

  FieldAttr *CreateFieldAttr(const char *field_name,
                             enum_field_types field_type,
                             unsigned int field_length,
                             unsigned int field_decimals,
                             unsigned int field_flags, bool is_gbk = false) {
    auto meta = std::make_shared<FieldMeta>(
        field_name, field_type, field_length, field_decimals, field_flags);
    std::string collation("utf8_general_ci");
    if (is_gbk) {
      collation = "gbk_chinese_ci";
    }
    FieldAttr *attr = new FieldAttr(true, true, collation, meta);
    return attr;
  }

 protected:
  std::string field_name_{"f1"};
};


#define TEST_CREATE_FIELD_TYPE(field_type, field_length, field_decimals, \
                               field_flags)                              \
  {                                                                      \
    FieldAttr *attr =                                                    \
        CreateFieldAttr(field_name_.c_str(), field_type, field_length,   \
                        field_decimals, field_flags);                    \
    auto field = FieldFactory::Create(field_name_, *attr);               \
    delete attr;                                                         \
    ASSERT_TRUE(field != nullptr);                                       \
  }

#define CREATE_FIELD(field_type, field_length, field_decimals, field_flags) \
  {                                                                         \
    FieldAttr *attr =                                                       \
        CreateFieldAttr(field_name_.c_str(), field_type, field_length,      \
                        field_decimals, field_flags);                       \
    field = FieldFactory::Create(field_name_, *attr);                       \
    delete attr;                                                            \
    ASSERT_TRUE(field != nullptr);                                          \
  }

#define CREATE_FIELD2(field_type, field_length, field_decimals, field_flags, \
                      gbk)                                                   \
  {                                                                          \
    FieldAttr *attr =                                                        \
        CreateFieldAttr(field_name_.c_str(), field_type, field_length,       \
                        field_decimals, field_flags, gbk);                   \
    field = FieldFactory::Create(field_name_, *attr);                        \
    delete attr;                                                             \
    ASSERT_TRUE(field != nullptr);                                           \
  }

TEST_F(FieldTest, TestCreateField) {
  TEST_CREATE_FIELD_TYPE(MYSQL_TYPE_TINY, 1, 0, 0);
  TEST_CREATE_FIELD_TYPE(MYSQL_TYPE_SHORT, 2, 0, 0);
  TEST_CREATE_FIELD_TYPE(MYSQL_TYPE_LONG, 4, 0, 0);
  TEST_CREATE_FIELD_TYPE(MYSQL_TYPE_FLOAT, 4, 0, 0);
  TEST_CREATE_FIELD_TYPE(MYSQL_TYPE_DOUBLE, 8, 0, 0);
  TEST_CREATE_FIELD_TYPE(MYSQL_TYPE_TIMESTAMP, 4, 0, 0);
  TEST_CREATE_FIELD_TYPE(MYSQL_TYPE_TIMESTAMP2, 4, 0, 0);
  TEST_CREATE_FIELD_TYPE(MYSQL_TYPE_LONGLONG, 8, 0, 0);
  TEST_CREATE_FIELD_TYPE(MYSQL_TYPE_INT24, 3, 0, 0);
  TEST_CREATE_FIELD_TYPE(MYSQL_TYPE_DATE, 4, 0, 0);
  TEST_CREATE_FIELD_TYPE(MYSQL_TYPE_TIME, 3, 0, 0);
  TEST_CREATE_FIELD_TYPE(MYSQL_TYPE_TIME2, 3, 0, 0);
  TEST_CREATE_FIELD_TYPE(MYSQL_TYPE_DATETIME, 8, 0, 0);
  TEST_CREATE_FIELD_TYPE(MYSQL_TYPE_DATETIME2, 8, 0, 0);
  TEST_CREATE_FIELD_TYPE(MYSQL_TYPE_YEAR, 1, 0, 0);
  TEST_CREATE_FIELD_TYPE(MYSQL_TYPE_BIT, 8, 0, 0);
  TEST_CREATE_FIELD_TYPE(MYSQL_TYPE_JSON, 16, 0, 0);
  TEST_CREATE_FIELD_TYPE(MYSQL_TYPE_NEWDECIMAL, 20, 0, 0);
  TEST_CREATE_FIELD_TYPE(MYSQL_TYPE_BLOB, 256, 0, 0);
  TEST_CREATE_FIELD_TYPE(MYSQL_TYPE_VARCHAR, 16, 0, 0);
  TEST_CREATE_FIELD_TYPE(MYSQL_TYPE_VAR_STRING, 10, 0, 0);
  TEST_CREATE_FIELD_TYPE(MYSQL_TYPE_STRING, 10, 0, 0);
  TEST_CREATE_FIELD_TYPE(MYSQL_TYPE_GEOMETRY, 16, 0, 0);
}

TEST_F(FieldTest, TestFieldTiny) {
  FieldPtr field;
  // signed text
  {
    unsigned int flags = 0;
    proto::GenericValue value;
    CREATE_FIELD(MYSQL_TYPE_TINY, 1, 0, flags);
    ASSERT_FALSE(field->unpack_text(nullptr, 0, &value));
    ASSERT_TRUE(field->unpack_text("-127", 4, &value));
    ASSERT_EQ(value.int32_value(), -127);
    ASSERT_TRUE(field->unpack_text("127", 3, &value));
    ASSERT_EQ(value.int32_value(), 127);
  }
  // unsigned text
  {
    unsigned int flags = UNSIGNED_FLAG;
    proto::GenericValue value;
    CREATE_FIELD(MYSQL_TYPE_TINY, 1, 0, flags);
    ASSERT_TRUE(field->unpack_text("6", 1, &value));
    ASSERT_EQ(value.uint32_value(), 6);
    ASSERT_TRUE(field->unpack_text("255", 3, &value));
    ASSERT_EQ(value.uint32_value(), 255);
  }
  // signed binary
  {
    unsigned int flags = 0;
    proto::GenericValue value;
    ColumnInfo info;
    uint8_t data[2] = {(uint8_t)(-10)};
    CREATE_FIELD(MYSQL_TYPE_TINY, 1, 0, flags);
    ASSERT_FALSE(field->unpack_binary(data, data, info, &value));
    ASSERT_TRUE(field->unpack_binary(data, data + 1, info, &value));
    ASSERT_EQ(value.int32_value(), -10);
  }
  // unsigned binary
  {
    unsigned int flags = UNSIGNED_FLAG;
    proto::GenericValue value;
    ColumnInfo info;
    uint8_t data[2] = {255};
    CREATE_FIELD(MYSQL_TYPE_TINY, 1, 0, flags);
    ASSERT_FALSE(field->unpack_binary(data, data, info, &value));
    ASSERT_TRUE(field->unpack_binary(data, data + 1, info, &value));
    ASSERT_EQ(value.uint32_value(), 255);
  }
}

TEST_F(FieldTest, TestFieldShort) {
  FieldPtr field;
  // signed text
  {
    unsigned int flags = 0;
    proto::GenericValue value;
    CREATE_FIELD(MYSQL_TYPE_SHORT, 2, 0, flags);
    ASSERT_FALSE(field->unpack_text(nullptr, 0, &value));
    ASSERT_TRUE(field->unpack_text("-128", 4, &value));
    ASSERT_EQ(value.int32_value(), -128);
    ASSERT_TRUE(field->unpack_text("32765", 5, &value));
    ASSERT_EQ(value.int32_value(), 32765);
  }
  // unsigned text
  {
    unsigned int flags = UNSIGNED_FLAG;
    proto::GenericValue value;
    CREATE_FIELD(MYSQL_TYPE_SHORT, 2, 0, flags);
    ASSERT_TRUE(field->unpack_text("6", 1, &value));
    ASSERT_EQ(value.uint32_value(), 6);
    ASSERT_TRUE(field->unpack_text("65535", 5, &value));
    ASSERT_EQ(value.uint32_value(), 65535);
  }
  // signed binary
  {
    unsigned int flags = 0;
    proto::GenericValue value;
    ColumnInfo info;
    uint8_t data[3] = {1, 2};
    CREATE_FIELD(MYSQL_TYPE_SHORT, 2, 0, flags);
    ASSERT_FALSE(field->unpack_binary(data, data, info, &value));
    ASSERT_TRUE(field->unpack_binary(data, data + 2, info, &value));
    ASSERT_EQ(value.int32_value(), 513);
  }
  // unsigned binary
  {
    unsigned int flags = UNSIGNED_FLAG;
    proto::GenericValue value;
    ColumnInfo info;
    uint8_t data[3] = {2, 1};
    CREATE_FIELD(MYSQL_TYPE_SHORT, 2, 0, flags);
    ASSERT_FALSE(field->unpack_binary(data, data, info, &value));
    ASSERT_TRUE(field->unpack_binary(data, data + 2, info, &value));
    ASSERT_EQ(value.uint32_value(), 258);
  }
}

TEST_F(FieldTest, TestFieldInt24) {
  FieldPtr field;
  // signed text
  {
    unsigned int flags = 0;
    proto::GenericValue value;
    CREATE_FIELD(MYSQL_TYPE_INT24, 3, 0, flags);
    ASSERT_FALSE(field->unpack_text(nullptr, 0, &value));
    ASSERT_TRUE(field->unpack_text("-65536", 6, &value));
    ASSERT_EQ(value.int32_value(), -65536);
    ASSERT_TRUE(field->unpack_text("100000", 6, &value));
    ASSERT_EQ(value.int32_value(), 100000);
  }
  // unsigned text
  {
    unsigned int flags = UNSIGNED_FLAG;
    proto::GenericValue value;
    CREATE_FIELD(MYSQL_TYPE_INT24, 3, 0, flags);
    ASSERT_TRUE(field->unpack_text("100", 3, &value));
    ASSERT_EQ(value.uint32_value(), 100);
    ASSERT_TRUE(field->unpack_text("1234567", 7, &value));
    ASSERT_EQ(value.uint32_value(), 1234567);
  }
  // signed binary
  {
    unsigned int flags = 0;
    proto::GenericValue value;
    ColumnInfo info;
    uint8_t data[3] = {1, 2, 0};
    CREATE_FIELD(MYSQL_TYPE_INT24, 3, 0, flags);
    ASSERT_FALSE(field->unpack_binary(data, data, info, &value));
    ASSERT_TRUE(field->unpack_binary(data, data + 3, info, &value));
    ASSERT_EQ(value.int32_value(), 513);
  }
  // unsigned binary
  {
    unsigned int flags = UNSIGNED_FLAG;
    proto::GenericValue value;
    ColumnInfo info;
    uint8_t data[3] = {2, 1, 0};
    CREATE_FIELD(MYSQL_TYPE_INT24, 3, 0, flags);
    ASSERT_FALSE(field->unpack_binary(data, data, info, &value));
    ASSERT_TRUE(field->unpack_binary(data, data + 3, info, &value));
    ASSERT_EQ(value.uint32_value(), 258);
  }
}

TEST_F(FieldTest, TestFieldLong) {
  FieldPtr field;
  // signed text
  {
    unsigned int flags = 0;
    proto::GenericValue value;
    CREATE_FIELD(MYSQL_TYPE_LONG, 4, 0, flags);
    ASSERT_FALSE(field->unpack_text(nullptr, 0, &value));
    ASSERT_TRUE(field->unpack_text("-65536", 6, &value));
    ASSERT_EQ(value.int32_value(), -65536);
    ASSERT_TRUE(field->unpack_text("2000000000", 10, &value));
    ASSERT_EQ(value.int32_value(), 2000000000);
  }
  // unsigned text
  {
    unsigned int flags = UNSIGNED_FLAG;
    proto::GenericValue value;
    CREATE_FIELD(MYSQL_TYPE_LONG, 4, 0, flags);
    ASSERT_TRUE(field->unpack_text("100", 3, &value));
    ASSERT_EQ(value.uint32_value(), 100);
    ASSERT_TRUE(field->unpack_text("4000000000", 10, &value));
    ASSERT_EQ(value.uint32_value(), 4000000000);
  }
  // signed binary
  {
    unsigned int flags = 0;
    proto::GenericValue value;
    ColumnInfo info;
    uint8_t data[4] = {1, 2, 0, 1};
    CREATE_FIELD(MYSQL_TYPE_LONG, 4, 0, flags);
    ASSERT_FALSE(field->unpack_binary(data, data, info, &value));
    ASSERT_TRUE(field->unpack_binary(data, data + 4, info, &value));
    ASSERT_EQ(value.int32_value(), 16777729);
  }
  // unsigned binary
  {
    unsigned int flags = UNSIGNED_FLAG;
    proto::GenericValue value;
    ColumnInfo info;
    uint8_t data[4] = {2, 1, 0, 255};
    CREATE_FIELD(MYSQL_TYPE_LONG, 4, 0, flags);
    ASSERT_FALSE(field->unpack_binary(data, data, info, &value));
    ASSERT_TRUE(field->unpack_binary(data, data + 4, info, &value));
    ASSERT_EQ(value.uint32_value(), 4278190338);
  }
}

TEST_F(FieldTest, TestFieldLongLong) {
  FieldPtr field;
  // signed text
  {
    unsigned int flags = 0;
    proto::GenericValue value;
    CREATE_FIELD(MYSQL_TYPE_LONGLONG, 8, 0, flags);
    ASSERT_FALSE(field->unpack_text(nullptr, 0, &value));
    ASSERT_TRUE(field->unpack_text("-65536", 6, &value));
    ASSERT_EQ(value.int64_value(), -65536);
    ASSERT_TRUE(field->unpack_text("8000000000", 10, &value));
    ASSERT_EQ(value.int64_value(), 8000000000);
  }
  // unsigned text
  {
    unsigned int flags = UNSIGNED_FLAG;
    proto::GenericValue value;
    CREATE_FIELD(MYSQL_TYPE_LONGLONG, 8, 0, flags);
    ASSERT_TRUE(field->unpack_text("100", 3, &value));
    ASSERT_EQ(value.uint64_value(), 100);
    ASSERT_TRUE(field->unpack_text("8000000000", 10, &value));
    ASSERT_EQ(value.uint64_value(), 8000000000);
  }
  // signed binary
  {
    unsigned int flags = 0;
    proto::GenericValue value;
    ColumnInfo info;
    uint8_t data[8] = {1, 2, 0, 1, 0, 0, 0, 0};
    CREATE_FIELD(MYSQL_TYPE_LONGLONG, 8, 0, flags);
    ASSERT_FALSE(field->unpack_binary(data, data, info, &value));
    ASSERT_TRUE(field->unpack_binary(data, data + 8, info, &value));
    ASSERT_EQ(value.int64_value(), 16777729);
  }
  // unsigned binary
  {
    unsigned int flags = UNSIGNED_FLAG;
    proto::GenericValue value;
    ColumnInfo info;
    uint8_t data[8] = {2, 1, 0, 255, 0, 0, 0, 0};
    CREATE_FIELD(MYSQL_TYPE_LONGLONG, 8, 0, flags);
    ASSERT_FALSE(field->unpack_binary(data, data, info, &value));
    ASSERT_TRUE(field->unpack_binary(data, data + 8, info, &value));
    ASSERT_EQ(value.uint64_value(), 4278190338);
  }
}

TEST_F(FieldTest, TestFieldFloat) {
  FieldPtr field;
  // text
  {
    unsigned int flags = 0;
    proto::GenericValue value;
    CREATE_FIELD(MYSQL_TYPE_FLOAT, 4, 0, flags);
    ASSERT_FALSE(field->unpack_text(nullptr, 0, &value));
    ASSERT_TRUE(field->unpack_text("-123.456", 8, &value));
    ASSERT_FLOAT_EQ(value.float_value(), -123.456);
    ASSERT_TRUE(field->unpack_text("2000000000", 10, &value));
    ASSERT_EQ(value.float_value(), 2000000000);
  }

  // binary
  {
    unsigned int flags = 0;
    proto::GenericValue value;
    ColumnInfo info;
    float val = 0.123456;
    uint8_t *data = (uint8_t *)(&val);
    CREATE_FIELD(MYSQL_TYPE_FLOAT, 4, 0, flags);
    ASSERT_FALSE(field->unpack_binary(data, data, info, &value));
    ASSERT_TRUE(field->unpack_binary(data, data + 4, info, &value));
    ASSERT_FLOAT_EQ(value.float_value(), 0.123456);
  }
}

TEST_F(FieldTest, TestFieldDouble) {
  FieldPtr field;
  // text
  {
    unsigned int flags = 0;
    proto::GenericValue value;
    CREATE_FIELD(MYSQL_TYPE_DOUBLE, 8, 0, flags);
    ASSERT_FALSE(field->unpack_text(nullptr, 0, &value));
    ASSERT_TRUE(field->unpack_text("-123.456", 8, &value));
    ASSERT_DOUBLE_EQ(value.double_value(), -123.456);
    ASSERT_TRUE(field->unpack_text("2000000000", 10, &value));
    ASSERT_EQ(value.double_value(), 2000000000);
  }

  // binary
  {
    unsigned int flags = 0;
    proto::GenericValue value;
    ColumnInfo info;
    double val = 1234.123456;
    uint8_t *data = (uint8_t *)(&val);
    CREATE_FIELD(MYSQL_TYPE_DOUBLE, 8, 0, flags);
    ASSERT_FALSE(field->unpack_binary(data, data, info, &value));
    ASSERT_TRUE(field->unpack_binary(data, data + 8, info, &value));
    ASSERT_DOUBLE_EQ(value.double_value(), 1234.123456);
  }
}

TEST_F(FieldTest, TestFieldDecimal) {
  FieldPtr field;
  // text
  {
    unsigned int flags = 0;
    proto::GenericValue value;
    CREATE_FIELD(MYSQL_TYPE_NEWDECIMAL, 8, 0, flags);
    ASSERT_FALSE(field->unpack_text(nullptr, 0, &value));
    ASSERT_TRUE(field->unpack_text("111.11", 6, &value));
    ASSERT_EQ(value.string_value(), "111.11");
  }

  // binary
  {
    unsigned int flags = 0;
    proto::GenericValue value;
    ColumnInfo info;
    info.meta = (15 << 8) + 10;
    uint8_t data[12] = {128, 48, 57, 24, 147, 229, 78, 9};
    CREATE_FIELD(MYSQL_TYPE_NEWDECIMAL, 8, 0, flags);
    ASSERT_FALSE(field->unpack_binary(data, data, info, &value));
    ASSERT_TRUE(field->unpack_binary(data, data + 12, info, &value));
    ASSERT_EQ(value.string_value(), "12345.4123456789");
  }
}

TEST_F(FieldTest, TestFieldBit) {
  FieldPtr field;
  // text
  {
    unsigned int flags = 0;
    proto::GenericValue value;
    CREATE_FIELD(MYSQL_TYPE_BIT, 8, 0, flags);
    ASSERT_FALSE(field->unpack_text(nullptr, 0, &value));
    ASSERT_TRUE(field->unpack_text("65534", 5, &value));
    ASSERT_DOUBLE_EQ(value.uint64_value(), 65534);
  }

  // binary
  {
    unsigned int flags = 0;
    proto::GenericValue value;
    ColumnInfo info;
    uint8_t data[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    info.meta = 4;
    CREATE_FIELD(MYSQL_TYPE_BIT, 8, 0, flags);
    ASSERT_FALSE(field->unpack_binary(data, data, info, &value));
    {
      ASSERT_TRUE(field->unpack_binary(data, data + 1, info, &value));
      ASSERT_DOUBLE_EQ(value.uint64_value(), 1);
    }
    {
      info.meta += 256;
      ASSERT_TRUE(field->unpack_binary(data, data + 2, info, &value));
      ASSERT_DOUBLE_EQ(value.uint64_value(), 258);
    }
    {
      info.meta += 256;
      ASSERT_TRUE(field->unpack_binary(data, data + 3, info, &value));
      ASSERT_DOUBLE_EQ(value.uint64_value(), 66051);
    }
    {
      info.meta += 256;
      ASSERT_TRUE(field->unpack_binary(data, data + 4, info, &value));
      ASSERT_DOUBLE_EQ(value.uint64_value(), 16909060);
    }
    {
      info.meta += 256;
      ASSERT_TRUE(field->unpack_binary(data, data + 5, info, &value));
      ASSERT_DOUBLE_EQ(value.uint64_value(), 4328719365);
    }
    {
      info.meta += 256;
      ASSERT_TRUE(field->unpack_binary(data, data + 6, info, &value));
      ASSERT_DOUBLE_EQ(value.uint64_value(), 1108152157446);
    }
    {
      info.meta += 256;
      ASSERT_TRUE(field->unpack_binary(data, data + 7, info, &value));
      ASSERT_DOUBLE_EQ(value.uint64_value(), 283686952306183);
    }
    {
      info.meta += 256;
      ASSERT_TRUE(field->unpack_binary(data, data + 8, info, &value));
      ASSERT_DOUBLE_EQ(value.uint64_value(), 72623859790382856);
    }
  }
}

TEST_F(FieldTest, TestFieldDatetime) {
  FieldPtr field;
  // text
  {
    unsigned int flags = 0;
    proto::GenericValue value;
    CREATE_FIELD(MYSQL_TYPE_DATETIME, 8, 0, flags);
    ASSERT_FALSE(field->unpack_text(nullptr, 0, &value));
    ASSERT_TRUE(field->unpack_text("2021-01-13 12:12:30.123456", 26, &value));
    ASSERT_EQ(value.string_value(), "2021-01-13 12:12:30.123456");
  }

  // binary
  {
    unsigned int flags = 0;
    proto::GenericValue value;
    ColumnInfo info;
    uint8_t data[8] = {0x99, 0x81, 0x21, 0x01, 0x01, 0x02, 0x01, 0x01};
    info.meta = 0;
    CREATE_FIELD(MYSQL_TYPE_DATETIME, 8, 0, flags);
    ASSERT_FALSE(field->unpack_binary(data, data, info, &value));
    {
      ASSERT_TRUE(field->unpack_binary(data, data + 5, info, &value));
      ASSERT_EQ(value.string_value(), "2008-12-16 16:04:01");
    }
    {
      info.meta = 1;
      ASSERT_TRUE(field->unpack_binary(data, data + 6, info, &value));
      ASSERT_EQ(value.string_value(), "2008-12-16 16:04:01.0");
    }
    {
      info.meta = 2;
      ASSERT_TRUE(field->unpack_binary(data, data + 6, info, &value));
      ASSERT_EQ(value.string_value(), "2008-12-16 16:04:01.02");
    }
    {
      info.meta = 3;
      ASSERT_TRUE(field->unpack_binary(data, data + 7, info, &value));
      ASSERT_EQ(value.string_value(), "2008-12-16 16:04:01.051");
    }
    {
      info.meta = 4;
      ASSERT_TRUE(field->unpack_binary(data, data + 7, info, &value));
      ASSERT_EQ(value.string_value(), "2008-12-16 16:04:01.0513");
    }
    {
      info.meta = 5;
      ASSERT_TRUE(field->unpack_binary(data, data + 8, info, &value));
      ASSERT_EQ(value.string_value(), "2008-12-16 16:04:01.13132");
    }
    {
      info.meta = 6;
      ASSERT_TRUE(field->unpack_binary(data, data + 8, info, &value));
      ASSERT_EQ(value.string_value(), "2008-12-16 16:04:01.131329");
    }

    data[0] = 0x80;
    for (size_t i = 1; i < 8; ++i) {
      data[i] = 0x0;
    }
    {
      info.meta = 6;
      ASSERT_TRUE(field->unpack_binary(data, data + 8, info, &value));
      ASSERT_EQ(value.string_value(), "0000-00-00 00:00:00");
    }
  }
}

TEST_F(FieldTest, TestFieldTimestamp) {
  FieldPtr field;
  // text
  {
    unsigned int flags = 0;
    proto::GenericValue value;
    CREATE_FIELD(MYSQL_TYPE_TIMESTAMP, 8, 0, flags);
    ASSERT_FALSE(field->unpack_text(nullptr, 0, &value));
    ASSERT_TRUE(field->unpack_text("2021-01-13 12:12:30.123456", 26, &value));
    ASSERT_EQ(value.string_value(), "2021-01-13 12:12:30.123456");
  }

  // binary
  {
    unsigned int flags = 0;
    proto::GenericValue value;
    ColumnInfo info;
    uint8_t data[8] = {0x60, 0x81, 0x21, 0x01, 0x01, 0x02, 0x01, 0x0};
    info.meta = 0;
    CREATE_FIELD(MYSQL_TYPE_TIMESTAMP, 7, 0, flags);
    ASSERT_FALSE(field->unpack_binary(data, data, info, &value));
    {
      ASSERT_TRUE(field->unpack_binary(data, data + 4, info, &value));
      ASSERT_EQ(value.string_value(), "2021-04-22 15:08:49");
    }
    {
      info.meta = 1;
      ASSERT_TRUE(field->unpack_binary(data, data + 5, info, &value));
      ASSERT_EQ(value.string_value(), "2021-04-22 15:08:49.0");
    }
    {
      info.meta = 2;
      ASSERT_TRUE(field->unpack_binary(data, data + 5, info, &value));
      ASSERT_EQ(value.string_value(), "2021-04-22 15:08:49.01");
    }
    {
      info.meta = 3;
      ASSERT_TRUE(field->unpack_binary(data, data + 6, info, &value));
      ASSERT_EQ(value.string_value(), "2021-04-22 15:08:49.025");
    }
    {
      info.meta = 4;
      ASSERT_TRUE(field->unpack_binary(data, data + 6, info, &value));
      ASSERT_EQ(value.string_value(), "2021-04-22 15:08:49.0258");
    }
    {
      info.meta = 5;
      ASSERT_TRUE(field->unpack_binary(data, data + 7, info, &value));
      ASSERT_EQ(value.string_value(), "2021-04-22 15:08:49.06604");
    }
    {
      info.meta = 6;
      ASSERT_TRUE(field->unpack_binary(data, data + 7, info, &value));
      ASSERT_EQ(value.string_value(), "2021-04-22 15:08:49.066049");
    }

    data[0] = 0x0;
    for (size_t i = 1; i < 8; ++i) {
      data[i] = 0x0;
    }
    {
      info.meta = 6;
      ASSERT_TRUE(field->unpack_binary(data, data + 7, info, &value));
      ASSERT_EQ(value.string_value(), "0000-00-00 00:00:00.000000");
    }
  }
}

TEST_F(FieldTest, TestFieldTime) {
  FieldPtr field;
  // text
  {
    unsigned int flags = 0;
    proto::GenericValue value;
    CREATE_FIELD(MYSQL_TYPE_TIME, 6, 0, flags);
    ASSERT_FALSE(field->unpack_text(nullptr, 0, &value));
    ASSERT_TRUE(field->unpack_text("12:12:30.123456", 15, &value));
    ASSERT_EQ(value.string_value(), "12:12:30.123456");
  }

  // binary
  {
    unsigned int flags = 0;
    proto::GenericValue value;
    ColumnInfo info;
    uint8_t data[8] = {0x80, 0xe1, 0x21, 0x01, 0x01, 0x02, 0x0, 0x0};
    info.meta = 0;
    CREATE_FIELD(MYSQL_TYPE_TIME, 6, 0, flags);
    ASSERT_FALSE(field->unpack_binary(data, data, info, &value));
    {
      ASSERT_TRUE(field->unpack_binary(data, data + 3, info, &value));
      ASSERT_EQ(value.string_value(), "14:04:33");
    }
    {
      info.meta = 1;
      ASSERT_TRUE(field->unpack_binary(data, data + 4, info, &value));
      ASSERT_EQ(value.string_value(), "14:04:33.0");
    }
    {
      info.meta = 2;
      ASSERT_TRUE(field->unpack_binary(data, data + 4, info, &value));
      ASSERT_EQ(value.string_value(), "14:04:33.01");
    }
    {
      info.meta = 3;
      ASSERT_TRUE(field->unpack_binary(data, data + 5, info, &value));
      ASSERT_EQ(value.string_value(), "14:04:33.025");
    }
    {
      info.meta = 4;
      ASSERT_TRUE(field->unpack_binary(data, data + 5, info, &value));
      ASSERT_EQ(value.string_value(), "14:04:33.0257");
    }
    {
      info.meta = 5;
      ASSERT_TRUE(field->unpack_binary(data, data + 6, info, &value));
      ASSERT_EQ(value.string_value(), "14:04:33.06579");
    }
    {
      info.meta = 6;
      ASSERT_TRUE(field->unpack_binary(data, data + 6, info, &value));
      ASSERT_EQ(value.string_value(), "14:04:33.065794");
    }

    data[0] = 0x80;
    for (size_t i = 1; i < 8; ++i) {
      data[i] = 0x0;
    }
    {
      info.meta = 6;
      ASSERT_TRUE(field->unpack_binary(data, data + 6, info, &value));
      ASSERT_EQ(value.string_value(), "00:00:00.000000");
    }

    data[0] = 0x70;
    {
      info.meta = 6;
      ASSERT_TRUE(field->unpack_binary(data, data + 6, info, &value));
      ASSERT_EQ(value.string_value(), "-256:00:00.000000");
    }
  }
}

TEST_F(FieldTest, TestFieldDate) {
  FieldPtr field;
  // text
  {
    unsigned int flags = 0;
    proto::GenericValue value;
    CREATE_FIELD(MYSQL_TYPE_DATE, 3, 0, flags);
    ASSERT_FALSE(field->unpack_text(nullptr, 0, &value));
    ASSERT_TRUE(field->unpack_text("2021-01-13", 10, &value));
    ASSERT_EQ(value.string_value(), "2021-01-13");
  }

  // binary
  {
    unsigned int flags = 0;
    proto::GenericValue value;
    ColumnInfo info;
    uint8_t data[3] = {0x99, 0x81, 0x10};
    info.meta = 0;
    CREATE_FIELD(MYSQL_TYPE_DATE, 3, 0, flags);
    ASSERT_FALSE(field->unpack_binary(data, data, info, &value));
    {
      ASSERT_TRUE(field->unpack_binary(data, data + 3, info, &value));
      ASSERT_EQ(value.string_value(), "2112-12-25");
    }

    for (size_t i = 0; i < 3; ++i) {
      data[i] = 0x0;
    }
    {
      ASSERT_TRUE(field->unpack_binary(data, data + 3, info, &value));
      ASSERT_EQ(value.string_value(), "0000-00-00");
    }
  }
}

TEST_F(FieldTest, TestFieldYear) {
  FieldPtr field;
  // text
  {
    unsigned int flags = 0;
    proto::GenericValue value;
    CREATE_FIELD(MYSQL_TYPE_YEAR, 1, 0, flags);
    ASSERT_FALSE(field->unpack_text(nullptr, 0, &value));
    ASSERT_TRUE(field->unpack_text("2021", 4, &value));
    ASSERT_EQ(value.string_value(), "2021");
  }

  // binary
  {
    unsigned int flags = 0;
    proto::GenericValue value;
    ColumnInfo info;
    uint8_t data[1] = {111};
    CREATE_FIELD(MYSQL_TYPE_YEAR, 1, 0, flags);
    ASSERT_FALSE(field->unpack_binary(data, data, info, &value));
    {
      ASSERT_TRUE(field->unpack_binary(data, data + 1, info, &value));
      ASSERT_EQ(value.string_value(), "2011");
    }

    for (size_t i = 0; i < 1; ++i) {
      data[i] = 0x0;
    }
    {
      ASSERT_TRUE(field->unpack_binary(data, data + 1, info, &value));
      ASSERT_EQ(value.string_value(), "0000");
    }
  }
}

TEST_F(FieldTest, TestFieldBlob) {
  FieldPtr field;
  // unpack_text
  {
    // blob
    unsigned int flags = BINARY_FLAG;
    proto::GenericValue value;
    CREATE_FIELD(MYSQL_TYPE_BLOB, 8, 0, flags);
    ASSERT_FALSE(field->unpack_text(nullptr, 0, &value));
    ASSERT_TRUE(field->unpack_text("313233343536373839", 18, &value));
    ASSERT_EQ(value.bytes_value(), "123456789");

    // text
    flags = 0;
    CREATE_FIELD(MYSQL_TYPE_BLOB, 8, 0, flags);
    ASSERT_TRUE(field->unpack_text("123456789", 9, &value));
    ASSERT_EQ(value.string_value(), "123456789");
  }

  // unpack_binary (blob)
  {
    unsigned int flags = BINARY_FLAG;
    proto::GenericValue value;
    ColumnInfo info;
    uint8_t data[8] = {0x00, 0x00, 0x00, 0x00, 0x30, 0x31, 0x32, 0x33};
    info.meta = 0;
    CREATE_FIELD(MYSQL_TYPE_BLOB, 8, 0, flags);
    ASSERT_FALSE(field->unpack_binary(data, data, info, &value));
    { ASSERT_FALSE(field->unpack_binary(data, data + 4, info, &value)); }
    {
      info.meta = 1;
      data[3] = 0x04;
      ASSERT_TRUE(field->unpack_binary(data + 3, data + 8, info, &value));
      ASSERT_EQ(value.bytes_value(), "0123");
      data[3] = 0x00;
    }
    {
      info.meta = 2;
      data[2] = 0x04;
      ASSERT_TRUE(field->unpack_binary(data + 2, data + 8, info, &value));
      ASSERT_EQ(value.bytes_value(), "0123");
      data[2] = 0x00;
    }
    {
      info.meta = 3;
      data[1] = 0x04;
      ASSERT_TRUE(field->unpack_binary(data + 1, data + 8, info, &value));
      ASSERT_EQ(value.bytes_value(), "0123");
      data[1] = 0x00;
    }
    {
      info.meta = 4;
      data[0] = 0x04;
      ASSERT_TRUE(field->unpack_binary(data + 0, data + 8, info, &value));
      ASSERT_EQ(value.bytes_value(), "0123");
      data[0] = 0x00;
    }
    {
      info.meta = 4;
      data[0] = 0x04;
      ASSERT_FALSE(field->unpack_binary(data + 0, data + 7, info, &value));
      data[0] = 0x00;
    }
  }

  // unpack_binary (text no need convert)
  {
    unsigned int flags = 0;
    proto::GenericValue value;
    ColumnInfo info;
    uint8_t data[8] = {0x00, 0x00, 0x00, 0x00, 0x30, 0x31, 0x32, 0x33};
    info.meta = 0;
    CREATE_FIELD2(MYSQL_TYPE_BLOB, 8, 0, flags, false);
    {
      info.meta = 1;
      data[3] = 0x04;
      ASSERT_TRUE(field->unpack_binary(data + 3, data + 8, info, &value));
      ASSERT_EQ(value.string_value(), "0123");
      data[3] = 0x00;
    }
  }
  // unpack_binary (text need convert)
  {
    unsigned int flags = 0;
    proto::GenericValue value;
    ColumnInfo info;
    uint8_t data[8] = {0x00, 0x00, 0x00, 0x00, 0xce, 0xd2, 0x32, 0x33};
    info.meta = 0;
    CREATE_FIELD2(MYSQL_TYPE_BLOB, 8, 0, flags, true);
    {
      info.meta = 1;
      data[3] = 0x04;
      ASSERT_TRUE(field->unpack_binary(data + 3, data + 8, info, &value));
      ASSERT_EQ(value.string_value(), "我23");
      data[3] = 0x00;
    }
  }
}

TEST_F(FieldTest, TestFieldVarString) {
  FieldPtr field;
  // unpack_text (varchar)
  {
    unsigned int flags = 0;
    proto::GenericValue value;
    CREATE_FIELD2(MYSQL_TYPE_VAR_STRING, 8, 0, flags, false);
    ASSERT_FALSE(field->unpack_text(nullptr, 0, &value));
    ASSERT_TRUE(field->unpack_text("123456789", 9, &value));
    ASSERT_EQ(value.string_value(), "123456789");
  }

  // unpack_text (varbinary)
  {
    unsigned int flags = BINARY_FLAG;
    proto::GenericValue value;
    CREATE_FIELD2(MYSQL_TYPE_VAR_STRING, 8, 0, flags, false);
    ASSERT_TRUE(field->unpack_text("313233343536373839", 18, &value));
    ASSERT_EQ(value.bytes_value(), "123456789");
  }

  // unpack_binary (varbinary)
  {
    unsigned int flags = BINARY_FLAG;
    proto::GenericValue value;
    ColumnInfo info;
    CREATE_FIELD2(MYSQL_TYPE_VAR_STRING, 8, 0, flags, false);
    {
      info.meta = 1;
      uint8_t data[8] = {0x01, 0x30, 0x00, 0x00, 0x30, 0x31, 0x32, 0x33};
      ASSERT_FALSE(field->unpack_binary(data, data, info, &value));
      ASSERT_TRUE(field->unpack_binary(data, data + 2, info, &value));
      ASSERT_EQ(value.bytes_value(), "0");
    }
    {
      info.meta = 256;
      uint8_t data[258] = {0x00, 0x01};
      std::string result("");
      for (size_t i = 2; i < 258; ++i) {
        data[i] = 0x30;
        result.append("0");
      }
      ASSERT_FALSE(field->unpack_binary(data, data, info, &value));
      ASSERT_FALSE(field->unpack_binary(data, data + 4, info, &value));
      ASSERT_TRUE(field->unpack_binary(data, data + 258, info, &value));
      ASSERT_EQ(value.bytes_value(), result);
    }
  }

  // unpack_binary (varchar no need convert)
  {
    unsigned int flags = 0;
    proto::GenericValue value;
    ColumnInfo info;
    CREATE_FIELD2(MYSQL_TYPE_VAR_STRING, 8, 0, flags, false);
    {
      info.meta = 1;
      uint8_t data[8] = {0x01, 0x30, 0x00, 0x00, 0x30, 0x31, 0x32, 0x33};
      ASSERT_FALSE(field->unpack_binary(data, data, info, &value));
      ASSERT_TRUE(field->unpack_binary(data, data + 2, info, &value));
      ASSERT_EQ(value.string_value(), "0");
    }
  }

  // unpack_binary (varchar need convert)
  {
    unsigned int flags = 0;
    proto::GenericValue value;
    ColumnInfo info;
    CREATE_FIELD2(MYSQL_TYPE_VAR_STRING, 8, 0, flags, true);
    {
      info.meta = 1;
      uint8_t data[8] = {0x03, 0xce, 0xd2, 0x30, 0x00, 0x00, 0x30, 0x31};
      ASSERT_FALSE(field->unpack_binary(data, data, info, &value));
      ASSERT_TRUE(field->unpack_binary(data, data + 4, info, &value));
      ASSERT_EQ(value.string_value(), "我0");
    }
  }
}

TEST_F(FieldTest, TestFieldString) {
  FieldPtr field;
  // unpack_text
  {
    // string (char(xx))
    unsigned int flags = 0;
    proto::GenericValue value;
    CREATE_FIELD(MYSQL_TYPE_STRING, 8, 0, flags);
    ASSERT_FALSE(field->unpack_text(nullptr, 0, &value));
    ASSERT_TRUE(field->unpack_text("123456789", 9, &value));
    ASSERT_EQ(value.string_value(), "123456789");

    // string (binary(xx))
    flags = BINARY_FLAG;
    CREATE_FIELD(MYSQL_TYPE_STRING, 8, 0, flags);
    ASSERT_FALSE(field->unpack_text(nullptr, 0, &value));
    ASSERT_TRUE(field->unpack_text("313233343536373839", 18, &value));
    ASSERT_EQ(value.bytes_value(), "123456789");

    // enum
    flags = ENUM_FLAG;
    CREATE_FIELD(MYSQL_TYPE_STRING, 8, 0, flags);
    ASSERT_TRUE(field->unpack_text("1255", 4, &value));
    ASSERT_EQ(value.int32_value(), 1255);

    // set
    flags = SET_FLAG;
    CREATE_FIELD(MYSQL_TYPE_STRING, 8, 0, flags);
    ASSERT_TRUE(field->unpack_text("255", 9, &value));
    ASSERT_EQ(value.uint64_value(), 255);
  }

  // unpack_binary
  {
    unsigned int flags = 0;
    proto::GenericValue value;
    ColumnInfo info;
    CREATE_FIELD(MYSQL_TYPE_STRING, 8, 0, flags);
    {
      info.meta = 1;
      uint8_t data[8] = {0x01, 0x30, 0x00, 0x00, 0x30, 0x31, 0x32, 0x33};
      ASSERT_FALSE(field->unpack_binary(data, data, info, &value));
      ASSERT_TRUE(field->unpack_binary(data, data + 2, info, &value));
      ASSERT_EQ(value.string_value(), "0");
    }
    // string
    {
      info.meta = 1 | (MYSQL_TYPE_STRING << 8);
      uint8_t data[8] = {0x01, 0x30, 0x00, 0x00, 0x30, 0x31, 0x32, 0x33};
      ASSERT_FALSE(field->unpack_binary(data, data, info, &value));
      ASSERT_TRUE(field->unpack_binary(data, data + 2, info, &value));
      ASSERT_EQ(value.string_value(), "0");
    }
    // set
    {
      info.meta = 1 | (MYSQL_TYPE_SET << 8);
      uint8_t data[2] = {0x01, 0x00};
      ASSERT_TRUE(field->unpack_binary(data, data + 2, info, &value));
      ASSERT_EQ(value.uint64_value(), 1);
    }
    // enum len 1
    {
      info.meta = 1 | (MYSQL_TYPE_ENUM << 8);
      uint8_t data[2] = {0x01, 0x00};
      ASSERT_TRUE(field->unpack_binary(data, data + 2, info, &value));
      ASSERT_EQ(value.int32_value(), 1);
    }
    // enum len 2
    {
      info.meta = 2 | (MYSQL_TYPE_ENUM << 8);
      uint8_t data[2] = {0x01, 0x00};
      ASSERT_TRUE(field->unpack_binary(data, data + 2, info, &value));
      ASSERT_EQ(value.int32_value(), 1);
    }
    // enum len 3
    {
      info.meta = 3 | (MYSQL_TYPE_ENUM << 8);
      uint8_t data[3] = {0x01, 0x00};
      ASSERT_FALSE(field->unpack_binary(data, data + 3, info, &value));
    }
  }
}

TEST_F(FieldTest, TestFieldStringWithParseStringValue) {
  FieldPtr field;
  // binary
  {
    unsigned int flags = BINARY_FLAG;
    proto::GenericValue value;
    CREATE_FIELD(MYSQL_TYPE_STRING, 256, 0, flags);
    FieldString *field1 = (FieldString *)field.get();

    // len < 256
    {
      uint8_t data[8] = {0x01, 0x30, 0x00, 0x00, 0x30, 0x31, 0x32, 0x33};
      ASSERT_FALSE(field1->parse_string_value(data, data, 1, &value));
      ASSERT_TRUE(field1->parse_string_value(data, data + 2, 1, &value));
      std::string result("0");
      result.resize(256);
      for (size_t i = 0; i < 255; ++i) {
        result[i + 1] = '\0';
      }
      ASSERT_EQ(value.bytes_value(), result);
    }
    // len >= 256
    {
      std::string result;
      result.resize(256);
      uint8_t data[258] = {0x00, 0x01};
      for (size_t i = 2; i < 258; ++i) {
        data[i] = 0x30;
        result[i - 2] = '0';
      }
      ASSERT_FALSE(field1->parse_string_value(data, data, 256, &value));
      ASSERT_FALSE(field1->parse_string_value(data, data + 20, 256, &value));
      ASSERT_TRUE(field1->parse_string_value(data, data + 258, 256, &value));
      ASSERT_EQ(value.bytes_value(), result);
    }
  }
  // string (no need convert)
  {
    unsigned int flags = 0;
    proto::GenericValue value;
    CREATE_FIELD2(MYSQL_TYPE_STRING, 256, 0, flags, false);
    FieldString *field1 = (FieldString *)field.get();

    // len < 256
    {
      uint8_t data[8] = {0x01, 0x30, 0x00, 0x00, 0x30, 0x31, 0x32, 0x33};
      ASSERT_FALSE(field1->parse_string_value(data, data, 1, &value));
      ASSERT_TRUE(field1->parse_string_value(data, data + 2, 1, &value));
      std::string result("0");
      ASSERT_EQ(value.string_value(), result);
    }
    // len >= 256
    {
      std::string result;
      result.resize(256);
      uint8_t data[258] = {0x00, 0x01};
      for (size_t i = 2; i < 258; ++i) {
        data[i] = 0x30;
        result[i - 2] = '0';
      }
      ASSERT_FALSE(field1->parse_string_value(data, data, 256, &value));
      ASSERT_FALSE(field1->parse_string_value(data, data + 20, 256, &value));
      ASSERT_TRUE(field1->parse_string_value(data, data + 258, 256, &value));
      ASSERT_EQ(value.string_value(), result);
    }
  }
  // string (need convert)
  {
    unsigned int flags = 0;
    proto::GenericValue value;
    CREATE_FIELD2(MYSQL_TYPE_STRING, 256, 0, flags, true);
    FieldString *field1 = (FieldString *)field.get();

    // len < 256
    {
      uint8_t data[8] = {0x03, 0xce, 0xd2, 0x30, 0x00, 0x00, 0x30, 0x31};
      ASSERT_FALSE(field1->parse_string_value(data, data, 64, &value));
      ASSERT_TRUE(field1->parse_string_value(data, data + 4, 64, &value));
      std::string result("我0");
      ASSERT_EQ(value.string_value(), result);
    }
  }
}

TEST_F(FieldTest, TestFieldStringWithParseSetValue) {
  FieldPtr field;
  {
    unsigned int flags = 0;
    proto::GenericValue value;
    CREATE_FIELD(MYSQL_TYPE_STRING, 8, 0, flags);
    FieldString *field1 = (FieldString *)field.get();

    int32_t meta = 0;
    // len = 1
    {
      meta = 1;
      uint8_t data[8] = {0x01, 0x30, 0x00, 0x00, 0x30, 0x31, 0x32, 0x33};
      ASSERT_FALSE(field1->parse_set_value(data, data, meta, &value));
      ASSERT_TRUE(field1->parse_set_value(data, data + 2, meta, &value));
      ASSERT_EQ(value.uint64_value(), 1);
    }
    // len = 2
    {
      meta = 2;
      uint8_t data[8] = {0x01, 0x30, 0x00, 0x00, 0x30, 0x31, 0x32, 0x33};
      ASSERT_TRUE(field1->parse_set_value(data, data + meta, meta, &value));
      ASSERT_EQ(value.uint64_value(), 0x3001);
    }
    // len = 3
    {
      meta = 3;
      uint8_t data[8] = {0x01, 0x30, 0x00, 0x00, 0x30, 0x31, 0x32, 0x33};
      ASSERT_TRUE(field1->parse_set_value(data, data + meta, meta, &value));
      ASSERT_EQ(value.uint64_value(), 0x3001);
    }
    // len = 4
    {
      meta = 4;
      uint8_t data[8] = {0x01, 0x30, 0x00, 0x00, 0x30, 0x31, 0x32, 0x33};
      ASSERT_TRUE(field1->parse_set_value(data, data + meta, meta, &value));
      ASSERT_EQ(value.uint64_value(), 0x3001);
    }
    // len = 5
    {
      meta = 5;
      uint8_t data[8] = {0x01, 0x30, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04};
      ASSERT_TRUE(field1->parse_set_value(data, data + meta, meta, &value));
      ASSERT_EQ(value.uint64_value(), 0x0100003001);
    }
    // len = 6
    {
      meta = 6;
      uint8_t data[8] = {0x01, 0x30, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04};
      ASSERT_TRUE(field1->parse_set_value(data, data + meta, meta, &value));
      ASSERT_EQ(value.uint64_value(), 0x020100003001);
    }
    // len = 7
    {
      meta = 7;
      uint8_t data[8] = {0x01, 0x30, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04};
      ASSERT_TRUE(field1->parse_set_value(data, data + meta, meta, &value));
      ASSERT_EQ(value.uint64_value(), 0x03020100003001);
    }
    // len = 8
    {
      meta = 8;
      uint8_t data[8] = {0x01, 0x30, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04};
      ASSERT_TRUE(field1->parse_set_value(data, data + meta, meta, &value));
      ASSERT_EQ(value.uint64_value(), 0x0403020100003001);
    }
    // len = 9
    {
      meta = 9;
      uint8_t data[9] = {0x01, 0x30, 0x00, 0x00, 0x01, 0x02, 0x03, 0x04};
      ASSERT_FALSE(field1->parse_set_value(data, data + meta, meta, &value));
    }
  }
}

TEST_F(FieldTest, TestFieldJson) {
  FieldPtr field;
  // text
  {
    unsigned int flags = 0;
    proto::GenericValue value;
    CREATE_FIELD(MYSQL_TYPE_JSON, 8, 0, flags);
    ASSERT_FALSE(field->unpack_text(nullptr, 0, &value));
    ASSERT_TRUE(field->unpack_text("[1,2,3,4]", 9, &value));
    ASSERT_EQ(value.bytes_value(), "[1,2,3,4]");
  }

  // binary
  {
    unsigned int flags = 0;
    proto::GenericValue value;
    ColumnInfo info;
    CREATE_FIELD(MYSQL_TYPE_JSON, 8, 0, flags);
    {
      info.meta = 1;
      uint8_t data[8] = {0x01, 0x30, 0x00, 0x00, 0x30, 0x31, 0x32, 0x33};
      ASSERT_FALSE(field->unpack_binary(data, data, info, &value));
      ASSERT_TRUE(field->unpack_binary(data, data + 2, info, &value));
      ASSERT_EQ(value.bytes_value(), "0");
    }
    {
      info.meta = 2;
      uint8_t data[8] = {0x02, 0x00, 0x30, 0x30, 0x30, 0x31, 0x32, 0x33};
      ASSERT_TRUE(field->unpack_binary(data, data + 8, info, &value));
      ASSERT_EQ(value.bytes_value(), "00");
    }
    {
      info.meta = 3;
      uint8_t data[8] = {0x03, 0x00, 0x00, 0x30, 0x30, 0x30, 0x32, 0x33};
      ASSERT_TRUE(field->unpack_binary(data, data + 8, info, &value));
      ASSERT_EQ(value.bytes_value(), "000");
    }
    {
      info.meta = 4;
      uint8_t data[8] = {0x04, 0x00, 0x00, 0x00, 0x30, 0x30, 0x30, 0x30};
      ASSERT_TRUE(field->unpack_binary(data, data + 8, info, &value));
      ASSERT_EQ(value.bytes_value(), "0000");
    }
    {
      info.meta = 5;
      uint8_t data[8] = {0x04, 0x00, 0x00, 0x00, 0x30, 0x30, 0x30, 0x30};
      ASSERT_FALSE(field->unpack_binary(data, data + 8, info, &value));
    }
  }
}

TEST_F(FieldTest, TestFieldGeometry) {
  FieldPtr field;
  // text
  {
    unsigned int flags = 0;
    proto::GenericValue value;
    CREATE_FIELD(MYSQL_TYPE_GEOMETRY, 8, 0, flags);
    ASSERT_FALSE(field->unpack_text(nullptr, 0, &value));
    ASSERT_TRUE(field->unpack_text("POINT(108.23 34.12)", 19, &value));
    ASSERT_EQ(value.bytes_value(), "POINT(108.23 34.12)");
  }

  // binary
  {
    unsigned int flags = 0;
    proto::GenericValue value;
    ColumnInfo info;
    CREATE_FIELD(MYSQL_TYPE_GEOMETRY, 8, 0, flags);
    {
      info.meta = 1;
      uint8_t data[8] = {0x01, 0x30, 0x00, 0x00, 0x30, 0x31, 0x32, 0x33};
      ASSERT_FALSE(field->unpack_binary(data, data, info, &value));
      ASSERT_TRUE(field->unpack_binary(data, data + 2, info, &value));
      ASSERT_EQ(value.bytes_value(), "0");
    }
    {
      info.meta = 2;
      uint8_t data[8] = {0x02, 0x00, 0x30, 0x30, 0x30, 0x31, 0x32, 0x33};
      ASSERT_TRUE(field->unpack_binary(data, data + 8, info, &value));
      ASSERT_EQ(value.bytes_value(), "00");
    }
    {
      info.meta = 3;
      uint8_t data[8] = {0x03, 0x00, 0x00, 0x30, 0x30, 0x30, 0x32, 0x33};
      ASSERT_TRUE(field->unpack_binary(data, data + 8, info, &value));
      ASSERT_EQ(value.bytes_value(), "000");
    }
    {
      info.meta = 4;
      uint8_t data[8] = {0x04, 0x00, 0x00, 0x00, 0x30, 0x30, 0x30, 0x30};
      ASSERT_TRUE(field->unpack_binary(data, data + 8, info, &value));
      ASSERT_EQ(value.bytes_value(), "0000");
    }
    {
      info.meta = 5;
      uint8_t data[8] = {0x04, 0x00, 0x00, 0x00, 0x30, 0x30, 0x30, 0x30};
      ASSERT_FALSE(field->unpack_binary(data, data + 8, info, &value));
    }
  }
}
