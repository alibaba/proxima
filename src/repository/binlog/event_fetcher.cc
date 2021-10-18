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
 *   \brief    Event fetcher interface implementation for proxima search engine
 */

#include <thread>

//!!! undef VERSION must be after include my_global.h
#include <my_global.h>
#include <ailego/utility/string_helper.h>
#include <ailego/utility/time_helper.h>
#include "repository_common/error_code.h"
#include "repository_common/logger.h"
#include "event_fetcher.h"
#include "sql_builder.h"

namespace proxima {
namespace be {
namespace repository {

EventFetcher::EventFetcher(MysqlConnectorManagerPtr mgr)
    : MysqlConnectorProxy(mgr) {}

int EventFetcher::init(const std::string &file_name, uint64_t position) {
  int ret = init_connector();
  if (ret != 0) {
    LOG_ERROR("Mysql connector proxy init failed.");
    return ret;
  }

  ret = turnoff_checksum();
  if (ret != 0) {
    LOG_ERROR("Turnoff checksum failed.");
    return ret;
  }

  ret = update_lsn_info(file_name, position);
  if (ret != 0) {
    LOG_ERROR("Update lsn info failed.");
    return ret;
  }

  ret = request_dump(file_name_, position_);
  if (ret != 0) {
    LOG_ERROR("Send dump request to master failed. file[%s] position[%zu]",
              file_name_.c_str(), (size_t)position_);
    return ret;
  }

  return 0;
}

int EventFetcher::turnoff_checksum() {
  std::string sql = SqlBuilder::BuildTurnoffCheckSumSql();
  int ret = connector_->execute_query(sql, nullptr, true);
  if (ret != 0) {
    LOG_ERROR("Connector xecute query failed. sql[%s]", sql.c_str());
    return ret;
  }
  return 0;
}

int EventFetcher::update_lsn_info(const std::string &file_name,
                                  uint64_t position) {
  // check file_name+position is valid lsn
  std::string sql = SqlBuilder::BuildSelectEventsSql(file_name, position);
  MysqlResultWrapperPtr events_result;
  int ret = connector_->execute_query(sql, &events_result, true);
  if (ret == 0) {
    file_name_ = file_name;
    position_ = position;
    return 0;
  }

  LOG_ERROR("Current lsn info is invalid, skipped. file_name[%s] position[%zu]",
            file_name.c_str(), (size_t)position);

  // get latest file_name+position
  sql = SqlBuilder::BuildShowBinaryLogsSql();
  MysqlResultWrapperPtr result;
  ret = connector_->execute_query(sql, &result, true);
  if (ret != 0) {
    LOG_ERROR("Show binary logs failed. sql[%s]", sql.c_str());
    return ret;
  }
  if (!result) {
    LOG_ERROR("Mysql result wrapper is nullptr.");
    return ErrorCode_ExecuteMysql;
  }

  ret = get_latest_lsn(file_name, *result);
  if (ret != 0) {
    return ret;
  }

  return 0;
}

int EventFetcher::get_latest_lsn(const std::string &file_name,
                                 MysqlResultWrapper &result) {
  if (result.fields_num() != 2) {
    LOG_ERROR("Mysql result's fields num mismatched. fields[%u]",
              result.fields_num());
    return ErrorCode_InvalidMysqlResult;
  }

  MysqlRow *row = result.next();
  std::vector<std::string> files;
  while (row) {
    std::string key(row->field_value(0), row->field_length(0));
    if (file_name.compare(key) < 0) {
      file_name_ = key;
      position_ = 4;  // binlog file start position
      LOG_INFO("Bin log lsn updated. file_name[%s] position[%zu]",
               file_name_.c_str(), (size_t)position_);
      break;
    }
    row = result.next();
  }

  if (row != nullptr) {
    return 0;
  } else {
    return ErrorCode_RuntimeError;
  }
}

/* dump request format
 * binlog-pos(4)|flags(2)|server_id(4)|binlog-filename(EOF)|
 */
int EventFetcher::request_dump(const std::string &file_name,
                               uint64_t position) {
  unsigned char buf[512];
  unsigned char *ptr_buf = buf;
  int4store(ptr_buf, (uint32)position);
  ptr_buf += 4;
  int2store(ptr_buf, BINLOG_DUMP_NON_BLOCK);
  ptr_buf += 2;
  if (server_id_ == 0) {
    server_id_ = generate_server_id();
  }
  int4store(ptr_buf, server_id_);
  ptr_buf += 4;
  memcpy(ptr_buf, file_name.c_str(), file_name.size());
  ptr_buf += file_name.size();

  size_t command_size = ptr_buf - buf;
  int ret =
      connector_->execute_simple_command(COM_BINLOG_DUMP, buf, command_size);
  if (ret != 0) {
    LOG_ERROR("Execute COM_BINLOG_DUMP command failed. code[%d]", ret);
    return ret;
  }

  return 0;
}

uint32_t EventFetcher::generate_server_id() {
  uint32_t server_id = (uint32_t)ailego::Monotime::MicroSeconds();
  // protect server id not match the real slave id
  while (server_id <= 10000) {
    server_id = (uint32_t)ailego::Monotime::MicroSeconds();
  }
  return server_id;
}

int EventFetcher::read_data(unsigned long *len) {
  int ret = 0;
  // reconnect mysql
  if (need_reconnect_) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    if (!connector_->reconnect()) {
      return ErrorCode_ConnectMysql;
    }
    ret = turnoff_checksum();
    if (ret != 0) {
      return ret;
    }
    ret = request_dump(file_name_, position_);
    if (ret != 0) {
      return ret;
    }
    need_reconnect_ = false;
    event_idx_ = 0;
  }

  // read data
  return connector_->client_safe_read(len);
}

int EventFetcher::fetch(BasicEventPtr *next_event) {
  unsigned long len;
  int ret = read_data(&len);
  if (ret != 0) {
    need_reconnect_ = true;
    LOG_ERROR("Read event failed. code[%d]", ret);
    return ret;
  }

  // check end of data
  const void *data = connector_->data();
  uint8_t flag = (static_cast<const uint8_t *>(data))[0];
  if (len < 8 && flag == 254) {
    need_reconnect_ = true;
    return ErrorCode_BinlogNoMoreData;
  }

  ++event_idx_;
  const char *buf = static_cast<const char *>(data) + 1;
  EventType event_type = (EventType)buf[BasicEvent::EventTypeOffset()];
  BasicEventPtr event;
  switch (event_type) {
    case QUERY_EVENT:
      event = std::make_shared<QueryEvent>(buf, len - 1);
      break;
    case ROTATE_EVENT: {
      bool has_crc = (event_idx_ != 1);
      event = std::make_shared<RotateEvent>(buf, len - 1, has_crc);
    } break;
    case TABLE_MAP_EVENT:
      event = std::make_shared<TableMapEvent>(buf, len - 1);
      break;
    case WRITE_ROWS_EVENT:
    case UPDATE_ROWS_EVENT:
    case DELETE_ROWS_EVENT:
    case WRITE_ROWS_EVENT_V1:
    case UPDATE_ROWS_EVENT_V1:
    case DELETE_ROWS_EVENT_V1:
      event = std::make_shared<RowsEvent>(buf, len - 1);
      break;
    default:
      event = std::make_shared<BasicEvent>(buf, len - 1);
      break;
  }
  if (event->log_pos() != 0) {
    position_ = (uint64_t)event->log_pos();
  }

  if (event_type == ROTATE_EVENT) {
    update_rotate_info(event);
  }

  *next_event = event;

  return 0;
}

void EventFetcher::update_rotate_info(BasicEventPtr event) {
  RotateEventPtr rotate_event = std::dynamic_pointer_cast<RotateEvent>(event);
  if (!rotate_event) {
    LOG_ERROR("Dynamic pointer cast to RotateEvent failed.");
    return;
  }

  file_name_ = rotate_event->next_binlog_name();
  position_ = rotate_event->position();
}

}  // end namespace repository
}  // namespace be
}  // end namespace proxima
