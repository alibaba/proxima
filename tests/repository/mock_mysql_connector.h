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

 *   \author   hongqing.hu
 *   \date     Dec 2020
 *   \brief    Mock mysql connector interface definition for proxima search
 engine
 */

#pragma once

#include <gmock/gmock.h>
#include "repository/binlog/mysql_connector.h"

using namespace proxima::be::repository;
using namespace ::testing;

namespace proxima {
namespace be {
namespace repository {

/*! Mock Mysql Connector
 */
class MockMysqlConnector : public MysqlConnector {
 public:
  //! Constructor
  MockMysqlConnector(){};

  //! Destructor
  virtual ~MockMysqlConnector() {}

  //! Init Mysql Connector
  MOCK_METHOD3(init, int(const ailego::Uri &uri, const std::string &user,
                         const std::string &password));

  //! Reconnect
  MOCK_METHOD0(reconnect, bool());

  //! Get uri
  MOCK_CONST_METHOD0(uri, const ailego::Uri &());

  //! Get data
  MOCK_CONST_METHOD0(data, const void *());

  //! Execute query
  MOCK_METHOD3(execute_query,
               int(const std::string &sql, MysqlResultWrapperPtr *result,
                   bool sync_fetch));

  //! Execute simple command
  MOCK_METHOD3(execute_simple_command,
               int(enum_server_command command, const unsigned char *arg,
                   size_t arg_length));

  //! Client safe read
  MOCK_METHOD1(client_safe_read, int(unsigned long *len));
};
using MockMysqlConnectorPtr = std::shared_ptr<MockMysqlConnector>;

class MockMysqlResultWrapper : public MysqlResultWrapper {
 public:
  //! Constructor
  MockMysqlResultWrapper() : MysqlResultWrapper(nullptr, nullptr) {}

  //! Destructor
  virtual ~MockMysqlResultWrapper() {
    for (size_t i = 0; i < buffers_.size(); ++i) {
      delete[] lengths_[i];
      for (size_t j = 0; j < fields_num_; ++j) {
        delete[] buffers_[i][j];
      }
      delete[] buffers_[i];
    }
    buffers_.clear();
  }

  int init() override {
    return init_value_;
  }

  MysqlRow *next() override {
    if (cur_idx_ < rows_.size()) {
      MysqlRow *row = &(rows_[cur_idx_]);
      cur_idx_++;
      return row;
    }
    return nullptr;
  }

  bool has_error() override {
    return has_error_;
  }

  void set_has_error(bool has_error) {
    has_error_ = has_error;
  }

  uint32_t fields_num() const override {
    return fields_num_;
  }

  uint32_t rows_num() const override {
    return (uint32_t)rows_.size();
  }

  const FieldMetaPtr &field_meta(unsigned int i) const override {
    return fields_[i];
  }

  void append_field_meta(const char *field_name,
                         enum_field_types field_type = MYSQL_TYPE_VAR_STRING,
                         unsigned int field_length = 0,
                         unsigned int field_decimals = 0,
                         unsigned int field_flags = 0) {
    FieldMetaPtr meta = std::make_shared<FieldMeta>(
        field_name, field_type, field_length, field_decimals, field_flags);
    fields_.emplace_back(meta);
    fields_num_ = (unsigned int)fields_.size();
  }

  void append_row_values(std::vector<std::string> &values) {
    MYSQL_ROW row = new char *[fields_num_];
    unsigned long *length = new unsigned long[fields_num_];
    for (size_t i = 0; i < values.size(); ++i) {
      row[i] = new char[values[i].size() + 1];
      length[i] = values[i].size();
      memcpy(row[i], values[i].data(), length[i]);
      row[i][length[i]] = '\0';
    }
    MysqlRow mysql_row(fields_num_);
    mysql_row.reset(row, length);
    rows_.emplace_back(mysql_row);

    buffers_.emplace_back(row);
    lengths_.emplace_back(length);
  }

  void reset() {
    cur_idx_ = 0;
  }

 private:
  bool has_error_{false};
  int init_value_{0};
  size_t cur_idx_{0};
  std::vector<MysqlRow> rows_{};
  std::vector<MYSQL_ROW> buffers_{};
  std::vector<unsigned long *> lengths_{};
};

using MockMysqlResultWrapperPtr = std::shared_ptr<MockMysqlResultWrapper>;

}  // namespace repository
}  // namespace be
}  // namespace proxima
