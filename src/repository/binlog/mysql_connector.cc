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
 *   \brief    Mysql connector interface implementation for proxima search
 *             engine
 */

#include "mysql_connector.h"
#include <errmsg.h>
#include <sql_common.h>
#include "repository_common/error_code.h"
#include "repository_common/logger.h"

namespace proxima {
namespace be {
namespace repository {

MysqlResultWrapper::MysqlResultWrapper(MYSQL *mysql, MYSQL_RES *res)
    : mysql_(mysql), result_(res) {}

MysqlResultWrapper::~MysqlResultWrapper() {
  if (result_) {
    mysql_free_result(result_);
    result_ = nullptr;
  }
}

int MysqlResultWrapper::init() {
  fields_num_ = mysql_num_fields(result_);
  mysql_row_.reset(new (std::nothrow) MysqlRow(fields_num_));
  if (!mysql_row_) {
    LOG_ERROR("New MysqlRow failed.");
    return ErrorCode_RuntimeError;
  }
  MYSQL_FIELD *mysql_fields = mysql_fetch_fields(result_);
  if (!mysql_fields) {
    LOG_ERROR("Fetch mysql fields failed. errno[%d] reason[%s]",
              mysql_errno(mysql_), mysql_error(mysql_));
    return ErrorCode_ExecuteMysql;
  }
  fields_.clear();
  for (unsigned int i = 0; i < fields_num_; ++i) {
    fields_.emplace_back(std::make_shared<FieldMeta>(
        mysql_fields[i].name, mysql_fields[i].type, mysql_fields[i].length,
        mysql_fields[i].decimals, mysql_fields[i].flags));
  }

  return 0;
}

MysqlRow *MysqlResultWrapper::next() {
  MYSQL_ROW row = mysql_fetch_row(result_);
  if (row) {
    unsigned long *lengths = mysql_fetch_lengths(result_);
    if (lengths) {
      mysql_row_->reset(row, lengths);
      return mysql_row_.get();
    }
  }
  return nullptr;
}

bool MysqlResultWrapper::has_error() {
  if (mysql_errno(mysql_)) {
    LOG_ERROR("Mysql error. code[%d] reason[%s]", mysql_errno(mysql_),
              mysql_error(mysql_));
    return true;
  }
  return false;
}

std::mutex MysqlConnector::global_mutex_;

MysqlConnector::MysqlConnector() {}

MysqlConnector::~MysqlConnector() {
  if (mysql_) {
    mysql_close(mysql_);
    mysql_ = nullptr;
  }
}

int MysqlConnector::init(const ailego::Uri &connection_uri,
                         const std::string &user, const std::string &password) {
  uri_ = connection_uri;
  user_ = user;
  password_ = password;

  if (!reconnect()) {
    LOG_ERROR("Mysql reconnect failed.");
    return ErrorCode_ConnectMysql;
  }

  return 0;
}

bool MysqlConnector::reconnect(void) {
  if (mysql_) {
    mysql_close(mysql_);
    mysql_ = nullptr;
  }
  {
    std::lock_guard<std::mutex> lock(global_mutex_);
    mysql_ = mysql_init(nullptr);
  }
  if (!mysql_) {
    LOG_ERROR("Mysql init failed code[%u] reason[%s]", mysql_errno(mysql_),
              mysql_error(mysql_));
    return false;
  }

  my_bool reconnect_opt = 1;
  mysql_options(mysql_, MYSQL_OPT_RECONNECT, &reconnect_opt);
  unsigned int connect_timeout = 3;
  mysql_options(mysql_, MYSQL_OPT_CONNECT_TIMEOUT, &connect_timeout);
  mysql_options(mysql_, MYSQL_SET_CHARSET_NAME, "utf8");

  if (!mysql_real_connect(mysql_, uri_.host().c_str(), user_.c_str(),
                          password_.c_str(), 0, uri_.port(), nullptr, 0)) {
    LOG_ERROR(
        "Mysql real connect failed. host[%s] port[%d] code[%d] reason[%s]",
        uri_.host().c_str(), uri_.port(), mysql_errno(mysql_),
        mysql_error(mysql_));
    return false;
  }

  return true;
}

int MysqlConnector::execute_query(const std::string &sql,
                                  MysqlResultWrapperPtr *result,
                                  bool sync_fetch) {
  if (need_reconnect_) {
    if (reconnect()) {
      need_reconnect_ = false;
    } else {
      return ErrorCode_ConnectMysql;
    }
  }

  if (mysql_query(mysql_, sql.c_str())) {
    need_reconnect_ = need_reconnect();
    LOG_ERROR("Execute mysql query failed. code[%d] reason[%s] sql[%s]",
              mysql_errno(mysql_), mysql_error(mysql_), sql.c_str());
    return ErrorCode_ExecuteMysql;
  }
  if (!result) {
    return 0;
  }
  MYSQL_RES *res;
  if (sync_fetch) {
    res = mysql_store_result(mysql_);
  } else {
    res = mysql_use_result(mysql_);
  }
  if (!res) {
    if (mysql_errno(mysql_)) {
      need_reconnect_ = need_reconnect();
      LOG_ERROR("Get mysql result failed. code[%d], reason[%s]",
                mysql_errno(mysql_), mysql_error(mysql_));
      return ErrorCode_ExecuteMysql;
    } else {
      return 0;
    }
  }
  *result = std::make_shared<MysqlResultWrapper>(mysql_, res);
  int ret = (*result)->init();
  if (ret != 0) {
    need_reconnect_ = need_reconnect();
    LOG_ERROR("Init MysqlResultWrapper failed.");
    return ret;
  }

  return 0;
}

int MysqlConnector::execute_simple_command(enum enum_server_command command,
                                           const unsigned char *arg,
                                           size_t arg_length) {
  if (simple_command(mysql_, command, arg, arg_length, 1)) {
    need_reconnect_ = need_reconnect();
    LOG_ERROR("Execute simple command failed. command[%d] code[%d] reason[%s]",
              command, mysql_errno(mysql_), mysql_error(mysql_));
    return ErrorCode_ExecuteSimpleCommand;
  }
  return 0;
}

int MysqlConnector::client_safe_read(unsigned long *len) {
  // TODO judge mysql_ closed
  *len = cli_safe_read(mysql_, NULL);
  if (*len == packet_error) {
    need_reconnect_ = need_reconnect();
    LOG_ERROR("Reading packet from server failed. code[%d] reason[%s]",
              mysql_errno(mysql_), mysql_error(mysql_));
    return ErrorCode_FetchMysqlResult;
  }
  return 0;
}

bool MysqlConnector::need_reconnect() {
  return (mysql_errno(mysql_) == CR_SERVER_GONE_ERROR ||
          mysql_errno(mysql_) == CR_SERVER_LOST);
}

int MysqlConnectorManager::init(const std::string &connection_uri,
                                const std::string &user,
                                const std::string &password) {
  if (!ailego::Uri::Parse(connection_uri.c_str(), &uri_)) {
    LOG_ERROR("Parse uri failed. uri[%s]", connection_uri.c_str());
    return ErrorCode_InvalidArgument;
  }
  if (!validate_paramters(uri_)) {
    LOG_ERROR("Validate mysql parameters failed. uri[%s]",
              connection_uri.c_str());
    return ErrorCode_InvalidArgument;
  }
  if (user.empty() || password.empty()) {
    LOG_ERROR("User name for password is empty. uri[%s]",
              connection_uri.c_str());
    return ErrorCode_InvalidArgument;
  }
  user_ = user;
  password_ = password;

  return 0;
}

MysqlConnectorPtr MysqlConnectorManager::get() {
  if (!connectors_.empty()) {
    MysqlConnectorPtr connector = connectors_.front();
    connectors_.pop();
    return connector;
  } else {
    MysqlConnectorPtr connector = std::make_shared<MysqlConnector>();
    int ret = connector->init(uri_, user_, password_);
    if (ret != 0) {
      LOG_ERROR("Init MysqlConnector failed. code[%d]", ret);
      return nullptr;
    }
    return connector;
  }
}

void MysqlConnectorManager::put(const MysqlConnectorPtr connector) {
  connectors_.push(std::move(connector));
}

bool MysqlConnectorManager::validate_paramters(const ailego::Uri &cur_uri) {
  if (cur_uri.host().empty() || cur_uri.port() <= 0 ||
      cur_uri.path().size() <= 1) {
    return false;
  }
  return true;
}

MysqlConnectorProxy::MysqlConnectorProxy(MysqlConnectorManagerPtr mgr)
    : connector_mgr_(std::move(mgr)) {}

MysqlConnectorProxy::~MysqlConnectorProxy() {
  if (connector_) {
    connector_mgr_->put(connector_);
    connector_.reset();
  }
}

int MysqlConnectorProxy::init_connector() {
  if (connector_mgr_) {
    connector_ = connector_mgr_->get();
    if (!connector_) {
      LOG_ERROR("Connector manager get connector failed.");
      return ErrorCode_RuntimeError;
    }
    return 0;
  }
  return ErrorCode_RuntimeError;
}

}  // namespace repository
}  // namespace be
}  // end namespace proxima
