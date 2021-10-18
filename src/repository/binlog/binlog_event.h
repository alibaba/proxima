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
 *   \brief    Bin log event
 */

#pragma once

#include <memory>
#include <string>
#include <vector>

namespace proxima {
namespace be {
namespace repository {

class LogContext;

/*! Event Type
 */
enum EventType {
  UNKNOWN_EVENT = 0,
  START_EVENT_V3 = 1,
  QUERY_EVENT = 2,  // describe alter table statement
  STOP_EVENT = 3,
  ROTATE_EVENT = 4,  // describe the end of binlog file
  INTVAR_EVENT = 5,
  LOAD_EVENT = 6,
  SLAVE_EVENT = 7,
  CREATE_FILE_EVENT = 8,
  APPEND_BLOCK_EVENT = 9,
  EXEC_LOAD_EVENT = 10,
  DELETE_FILE_EVENT = 11,
  NEW_LOAD_EVENT = 12,
  RAND_EVENT = 13,
  USER_VAR_EVENT = 14,
  FORMAT_DESCRIPTION_EVENT =
      15,  // the first event in binlog file describe event version
  XID_EVENT = 16,
  BEGIN_LOAD_QUERY_EVENT = 17,
  EXECUTE_LOAD_QUERY_EVENT = 18,

  TABLE_MAP_EVENT = 19,  // describe the rows event's meta

  PRE_GA_WRITE_ROWS_EVENT = 20,
  PRE_GA_UPDATE_ROWS_EVENT = 21,
  PRE_GA_DELETE_ROWS_EVENT = 22,

  WRITE_ROWS_EVENT_V1 = 23,   // describe insert statement
  UPDATE_ROWS_EVENT_V1 = 24,  // describe update statement
  DELETE_ROWS_EVENT_V1 = 25,  // describe delete statement

  INCIDENT_EVENT = 26,

  HEARTBEAT_LOG_EVENT = 27,

  // 5.6 new events

  IGNORABLE_LOG_EVENT = 28,
  ROWS_QUERY_LOG_EVENT = 29,

  WRITE_ROWS_EVENT = 30,   // describe insert statement
  UPDATE_ROWS_EVENT = 31,  // describe update statement
  DELETE_ROWS_EVENT = 32,  // describe delete statement

  GTID_LOG_EVENT = 33,
  ANONYMOUS_GTID_LOG_EVENT = 34,

  PREVIOUS_GTIDS_LOG_EVENT = 35,

  // 5.7 new events

  TRANSACTION_CONTEXT_EVENT = 36,

  VIEW_CHANGE_EVENT = 37,

  XA_PREPARE_LOG_EVENT = 38,

  ENUM_END_EVENT
};

class BasicEvent;
class QueryEvent;
struct RotateEvent;
struct TableMapEvent;
struct RowsEvent;

using BasicEventPtr = std::shared_ptr<BasicEvent>;
using QueryEventPtr = std::shared_ptr<QueryEvent>;
using RotateEventPtr = std::shared_ptr<RotateEvent>;
using TableMapEventPtr = std::shared_ptr<TableMapEvent>;
using RowsEventPtr = std::shared_ptr<RowsEvent>;

/* Basic Event
 */
class BasicEvent {
 public:
  //! Constructor
  BasicEvent(const char *buf, unsigned int length);

  //! Destructor
  virtual ~BasicEvent() = default;

  //! Get is valid
  bool is_valid() const {
    return is_valid_;
  }

  //! Get event type
  EventType type() const {
    return type_;
  }

  //! Get log pos
  size_t log_pos() const {
    return log_pos_;
  }

  //! Get timestamp
  time_t timestamp() const {
    return timestamp_;
  }

  //! Get server id
  unsigned int server_id() const {
    return server_id_;
  }

  //! Get buffer
  const char *buffer() const {
    return buffer_;
  }

  //! Get event length
  unsigned int buffer_length() const {
    return buffer_len_;
  }

  //! Get event type offset
  static int EventTypeOffset() {
    return EVENT_TYPE_OFFSET;
  }

 protected:
  // Bin log event header, detail
  // see: https://dev.mysql.com/doc/internals/en/binlog-event-header.html
  // 4              timestamp
  // 1              event type
  // 4              server-id
  // 4              event-size
  // 4              log pos
  // 2              flags
  static constexpr uint32_t EVENT_TYPE_OFFSET = 4;
  static constexpr uint32_t SERVER_ID_OFFSET = 5;
  static constexpr uint32_t EVENT_LEN_OFFSET = 9;
  static constexpr uint32_t LOG_POS_OFFSET = 13;
  static constexpr uint32_t LOG_EVENT_HEADER_LEN = 19;
  static constexpr uint32_t CRC_LEN = 4;

 protected:
  //! Members
  bool is_valid_{false};
  EventType type_{UNKNOWN_EVENT};
  size_t log_pos_{0};
  time_t timestamp_{0};
  unsigned int server_id_{0};
  char *buffer_{nullptr};
  unsigned int buffer_len_{0};
  std::string data_{};
};

/* Query event
 */
class QueryEvent : public BasicEvent {
 public:
  //! Constructor
  QueryEvent(const char *buf, unsigned int length);

  //! Destructor
  ~QueryEvent() = default;

  //! Get database
  const std::string &db_name() const {
    return db_name_;
  }

  //! Get query string
  const std::string &query() const {
    return query_;
  }

 private:
  // Query event Post-header, detail see:
  // https://dev.mysql.com/doc/internals/en/query-event.html
  // 4              slave_proxy_id
  // 4              execution time
  // 1              schema length
  // 2              error-code
  // 2              status-vars length
  static constexpr uint32_t QE_POST_HEADER_SCHEMA_LEN_OFFSET = 8;
  static constexpr uint32_t QE_POST_HEADER_STATUS_LEN_OFFSET = 11;
  static constexpr uint32_t QE_POST_HEADER_LEN = 13;

 private:
  //! Members
  std::string db_name_{};
  std::string query_{};
};

/* Rotate event
 */
struct RotateEvent : public BasicEvent {
 public:
  //! Constructor
  RotateEvent(const char *buf, unsigned int length, bool has_crc);

  //! Destructor
  ~RotateEvent() = default;

  //! Get next binlog file name
  const std::string &next_binlog_name() const {
    return next_binlog_name_;
  }

  //! Get binlog position
  uint64_t position() const {
    return position_;
  }

 private:
  // Rotate event Post-header, detail see:
  // https://dev.mysql.com/doc/internals/en/rotate-event.html
  // 8              position
  static constexpr uint32_t RE_POST_HEADER_LEN = 8;

 private:
  //! Members
  std::string next_binlog_name_{};
  uint64_t position_{0};
};

/* Column Info
 */
struct ColumnInfo {
  int32_t type;
  int32_t meta;
  bool nullable;
};

/* Table Map Event
 */
struct TableMapEvent : public BasicEvent {
 public:
  //! Constructor
  TableMapEvent(const char *buf, unsigned int length);

  //! Destructor
  ~TableMapEvent() = default;

  //! Get table id
  unsigned long table_id() const {
    return table_id_;
  }

  //! Get Table name
  const std::string &table_name() const {
    return table_name_;
  }

  //! Get database name
  const std::string &database_name() const {
    return database_name_;
  }

  //! Get Column info
  const ColumnInfo &column_info(size_t id) {
    return column_info_[id];
  }

 private:
  //! Decode meta data
  void decode_meta_data(const char *ptr);

 private:
  // Table map event post-header, see detail:
  // https://dev.mysql.com/doc/internals/en/table-map-event.html
  // post-header:
  // 6              table id
  // 2              flags
  static constexpr uint32_t TME_POST_HEADER_LEN = 8;

 private:
  //! Members
  unsigned long table_id_{0};
  std::string table_name_{};
  std::string database_name_{};
  unsigned long column_count_{0};
  unsigned long field_metadata_size_{0};
  std::vector<ColumnInfo> column_info_{};
};

/* Rows event
 */
struct RowsEvent : public BasicEvent {
 public:
  //! Constructor
  RowsEvent(const char *buf, unsigned int length);

  //! Destructor
  ~RowsEvent() = default;

  //! Fill table map
  void fill_table_map(const LogContext &context);

  //! Get table map
  TableMapEventPtr table_map() const {
    return table_map_;
  }

  //! Get table id
  unsigned long table_id() const {
    return table_id_;
  }

  //! Get column count
  unsigned long column_count() const {
    return column_count_;
  }

  //! Get bits length
  unsigned long bits_length() const {
    return bits_length_;
  }

  //! Get present columns
  uint8_t *present_columns() const {
    return present_columns_;
  }

  //! Get present columns update
  uint8_t *present_columns_update() const {
    return present_columns_update_;
  }

  //! Get rows buf
  const void *rows_buf() const {
    return rows_buf_;
  }

  //! Get rows end
  const void *rows_end() const {
    return rows_end_;
  }

  //! Get current buf
  const void *cur_buf() const {
    return cur_buf_;
  }

  //! Set current buf
  void set_cur_buf(const void *buf) {
    cur_buf_ = buf;
  }

  //! Is finished
  bool is_finished() const {
    return cur_buf_ == rows_end_;
  }

  //! Get start position
  uint64_t start_position() const {
    return start_position_;
  }

 private:
  // Rows event post-header, see detail:
  // https://dev.mysql.com/doc/internals/en/rows-event.html
  // header:
  // 6                    table id
  // 2                    flags
  // 2                    extra-data-length
  // string.var_len       extra-data
  static constexpr uint32_t ROWS_EVENT_POST_HEADER_LEN = 8;

 private:
  //! Members
  unsigned long table_id_{0};
  unsigned long column_count_{0};
  unsigned long bits_length_{0};
  uint64_t start_position_{0};
  uint8_t *present_columns_{nullptr};
  uint8_t *present_columns_update_{nullptr};

  const void *rows_buf_{nullptr};
  const void *rows_end_{nullptr};
  const void *cur_buf_{nullptr};

  TableMapEventPtr table_map_{};
};

}  // end namespace repository
}  // namespace be
}  // end namespace proxima
