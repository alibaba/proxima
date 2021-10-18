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

 *   \author   Hongqing.hu
 *   \date     Nov 2020
 *   \brief    Mysql connector interface definition for proxima search engine
 */

#pragma once

//!!! notice: my_global.h must before mysql.h
#include <my_global.h>
//!!! notice: my_global.h must before mysql.h

#include <mysql.h>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <vector>
#include <ailego/encoding/uri.h>

namespace proxima {
namespace be {
namespace repository {

class FieldMeta;
class MysqlConnector;
class MysqlResultWrapper;
class MysqlConnectorManager;

using FieldMetaPtr = std::shared_ptr<FieldMeta>;
using MysqlResultWrapperPtr = std::shared_ptr<MysqlResultWrapper>;
using MysqlConnectorPtr = std::shared_ptr<MysqlConnector>;
using MysqlConnectorManagerPtr = std::shared_ptr<MysqlConnectorManager>;

/*! Field Meta
 */
class FieldMeta {
 public:
  //! Constructor
  FieldMeta(const char *field_name, enum_field_types field_type,
            unsigned int field_length, unsigned int field_decimals,
            unsigned int field_flags)
      : name_(field_name),
        type_(field_type),
        length_(field_length),
        decimals_(field_decimals),
        flags_(field_flags) {}

  //! Destructor
  ~FieldMeta() {}

  //! Get Field name
  const std::string &name() const {
    return name_;
  }

  //! Get Field type
  enum_field_types type() const {
    return type_;
  }

  //! Get Field length
  unsigned int length() const {
    return length_;
  }

  //! Get Field decimals
  unsigned int decimals() const {
    return decimals_;
  }

  //! Get Field flags
  unsigned int flags() const {
    return flags_;
  }

 private:
  //! Members
  std::string name_{};
  enum_field_types type_{};
  unsigned int length_{0};
  unsigned int decimals_{0};
  unsigned int flags_{0};
};

/*! Mysql Row
 */
class MysqlRow {
 public:
  //! Constructor
  MysqlRow(unsigned int fields_num)
      : row_(nullptr), lengths_(nullptr), fields_num_(fields_num) {}

  //! Reset mysql row
  void reset(MYSQL_ROW row, unsigned long *lengths) {
    row_ = row;
    lengths_ = lengths;
  }

  //! Get field value
  char *field_value(unsigned int idx) const {
    if (idx < fields_num_) {
      return row_[idx];
    }
    return nullptr;
  }

  //! Get field length
  unsigned int field_length(unsigned int idx) const {
    if (idx < fields_num_) {
      return lengths_[idx];
    }
    return (unsigned int)-1;
  }

 private:
  //! Members
  MYSQL_ROW row_{nullptr};
  unsigned long *lengths_{nullptr};
  unsigned int fields_num_{0};
};

/*! Mysql Result Wrapper
 */
class MysqlResultWrapper {
 public:
  //! Constructor
  MysqlResultWrapper(MYSQL *mysql, MYSQL_RES *res);

  //! Destructor
  virtual ~MysqlResultWrapper();

  //! Init mysql result wrapper
  virtual int init();

  //! Get next mysql row
  virtual MysqlRow *next();

  //! If current has error
  virtual bool has_error();

  //! Get fields num
  virtual uint32_t fields_num() const {
    return fields_num_;
  }

  //! Get result rows num
  virtual uint32_t rows_num() const {
    return (uint32_t)mysql_num_rows(result_);
  }

  //! Get result field meta
  virtual const FieldMetaPtr &field_meta(uint32_t i) const {
    return fields_[i];
  }

 private:
  //! Members
  uint32_t fields_num_{0};
  MYSQL *mysql_{nullptr};
  MYSQL_RES *result_{nullptr};
  std::unique_ptr<MysqlRow> mysql_row_{};
  std::vector<FieldMetaPtr> fields_{};
};

/*! Mysql Handler
 */
class MysqlConnector {
 public:
  //! Constructor
  MysqlConnector();

  //! Destructor
  virtual ~MysqlConnector();

  //! Init mysql connector
  virtual int init(const ailego::Uri &uri, const std::string &user,
                   const std::string &password);

  //! Reconnect mysql
  virtual bool reconnect(void);

  //! Execute query
  virtual int execute_query(const std::string &sql,
                            MysqlResultWrapperPtr *result,
                            bool sync_fetch = false);

  //! Execute Simple Command
  virtual int execute_simple_command(enum enum_server_command command,
                                     const unsigned char *arg,
                                     size_t arg_length);

  //! Client safe read
  virtual int client_safe_read(unsigned long *len);

  //! Get Uri
  virtual const ailego::Uri &uri() const {
    return uri_;
  }

  //! Get mysql read data
  virtual const void *data() const {
    return (const void *)mysql_->net.read_pos;
  }

 private:
  //! Need reconnect
  bool need_reconnect();

 private:
  //! MYSQL pointer
  MYSQL *mysql_{nullptr};
  //! Need reconnect
  bool need_reconnect_{false};
  //! Uri
  ailego::Uri uri_{};
  //! user name
  std::string user_{};
  //! password
  std::string password_{};
  //! Ensure mysql thread safe
  static std::mutex global_mutex_;
};

/* Mysql Connector Manager
 */
class MysqlConnectorManager {
 public:
  //! Constructor
  MysqlConnectorManager() = default;
  //! Destructor
  ~MysqlConnectorManager() = default;

  //! Init manager
  int init(const std::string &connection_uri, const std::string &user,
           const std::string &password);

  //! get mysql connector
  MysqlConnectorPtr get();

  //! put mysql connector
  void put(MysqlConnectorPtr connector);

 protected:
  //! Validate mysql parameters
  bool validate_paramters(const ailego::Uri &uri);

 private:
  //! Members
  ailego::Uri uri_{};
  std::string user_{};
  std::string password_{};
  std::queue<MysqlConnectorPtr> connectors_{};
};

/* MysqlConnectorProxy
 */
class MysqlConnectorProxy {
 public:
  //! Constructor
  MysqlConnectorProxy(MysqlConnectorManagerPtr connector_mgr);

  //! Destructor
  virtual ~MysqlConnectorProxy();

 protected:
  //! Init connector
  virtual int init_connector();

 protected:
  //! Mysql connector manager
  MysqlConnectorManagerPtr connector_mgr_{};
  //! Mysql connector
  MysqlConnectorPtr connector_{};
};

}  // namespace repository
}  // namespace be
}  // namespace proxima
