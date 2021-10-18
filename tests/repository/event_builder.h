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
 *   \brief    Mysql Mysql interface definition for bilin engine
 */

#pragma once

//!!! undef VERSION must be after include my_global.h
#include <myisampack.h>
#include <mysql.h>
#include <my_global.h>
#undef VERSION

#include <ailego/utility/string_helper.h>
#include "repository/binlog/binlog_event.h"

using namespace proxima::be::repository;
using namespace ::testing;

namespace proxima {
namespace be {
namespace repository {

/*! EventBuilder
 */
class EventBuilder {
 public:
  static void BuildBasicEvent(EventType type, uchar **data) {
    uchar *buf = *data;
    uint32_t timestamp = (uint32_t)std::time(nullptr);
    int4store(buf, timestamp);
    buf += 4;
    *buf = (uchar)type;
    buf += 1;
    uint32_t server_id = 10000;
    int4store(buf, server_id);
    buf += 4;
    uint32_t event_size = 0;
    int4store(buf, event_size);
    buf += 4;
    uint32_t log_pos = 0;
    int4store(buf, log_pos);
    buf += 4;
    uint16_t flags = 0;
    int2store(buf, flags);
    buf += 2;

    *data = buf;
  }

  static std::string BuildQueryEvent(const std::string &db,
                                     const std::string &query) {
    std::string buffer(10240, 0);
    uchar *data = (uchar *)(&(buffer[0]));
    uchar *start = data;
    BuildBasicEvent(QUERY_EVENT, &data);
    data += 8;  // slave_proxy_id + execution time
    uint8_t schema_length = (uint8_t)db.size();
    *data = (uchar)schema_length;
    data += 1;
    data += 2;  // error-code
    uint16_t status_var_len = 0;
    int2store(data, status_var_len);
    data += 2;
    data += status_var_len;  // status vars
    memcpy(data, db.data(), db.size());
    data += schema_length;
    *data = '\0';
    data += 1;
    memcpy(data, query.data(), query.size());
    data += query.size();
    data += 4;  // crc
    size_t len = data - start;
    buffer.resize(len);
    return buffer;
  }

  static std::string BuildRotateEvent(const std::string &file_name,
                                      uint64_t position, bool has_crc = true) {
    std::string buffer(10240, 0);
    uchar *data = (uchar *)(&(buffer[0]));
    uchar *start = data;
    BuildBasicEvent(ROTATE_EVENT, &data);
    int8store(data, position);
    data += 8;
    memcpy(data, file_name.data(), file_name.size());
    data += file_name.size();
    if (has_crc) {
      data += 4;
    }
    size_t len = data - start;
    buffer.resize(len);
    return buffer;
  }

  static void SaveColumnMeta(uchar **data,
                             const std::vector<enum_field_types> &column_types,
                             const std::vector<int32_t> &column_metas) {
    std::string buffer(1024, 0);
    uchar *ptr = (uchar *)(&(buffer[0]));
    uchar *start = ptr;
    for (size_t i = 0; i < column_types.size(); ++i) {
      switch (column_types[i]) {
        case MYSQL_TYPE_TINY_BLOB:
        case MYSQL_TYPE_BLOB:
        case MYSQL_TYPE_MEDIUM_BLOB:
        case MYSQL_TYPE_LONG_BLOB:
        case MYSQL_TYPE_DOUBLE:
        case MYSQL_TYPE_FLOAT:
        case MYSQL_TYPE_GEOMETRY:
        case MYSQL_TYPE_JSON:
          *ptr = (uchar)column_metas[i];
          ptr += 1;
          break;
        case MYSQL_TYPE_STRING:
          *(ptr + 1) = (uchar)column_metas[i];
          ptr += 2;
          break;
        case MYSQL_TYPE_BIT:
          int2store(ptr, (uint16_t)column_metas[i]);
          ptr += 2;
          break;
        case MYSQL_TYPE_VARCHAR:
        case MYSQL_TYPE_VAR_STRING:
          int2store(ptr, (uint16_t)column_metas[i]);
          ptr += 2;
          break;
        case MYSQL_TYPE_NEWDECIMAL:
          int2store(ptr, (uint16_t)column_metas[i]);
          ptr += 2;
          break;
        case MYSQL_TYPE_TIME2:
        case MYSQL_TYPE_DATETIME2:
        case MYSQL_TYPE_TIMESTAMP2:
          *ptr = (uchar)column_metas[i];
          ptr += 1;
          break;
        default:
          break;
      }
    }
    uint32_t meta_len = ptr - start;
    uchar *cur = *data;
    *cur = (uchar)meta_len;
    cur += 1;
    memcpy(cur, start, meta_len);
    *data = cur + meta_len;
  }

  static std::string BuildTableMapEvent(
      uint64_t table_id, const std::string &db, const std::string &table,
      std::vector<enum_field_types> &column_types,
      std::vector<int32_t> &column_metas, std::vector<bool> &column_nullables) {
    std::string buffer(10240, 0);
    uchar *data = (uchar *)(&(buffer[0]));
    uchar *start = data;
    BuildBasicEvent(TABLE_MAP_EVENT, &data);
    // table id
    int4store(data, (uint32_t)table_id);
    data += 4;
    int2store(data, (uint16_t)(table_id >> 32));
    data += 2;
    // flags
    data += 2;
    // db
    *data = (uchar)db.size();
    data += 1;
    memcpy(data, db.data(), db.size());
    data += db.size();
    data += 1;
    // table
    *data = (uchar)table.size();
    data += 1;
    memcpy(data, table.data(), table.size());
    data += table.size();
    data += 1;
    // column count
    *data = (uchar)column_types.size();
    data += 1;
    // column types
    memcpy(data, column_types.data(), column_types.size());
    data += column_types.size();
    // column meta
    SaveColumnMeta(&data, column_types, column_metas);
    uint32_t null_bytes = (column_nullables.size() + 7) / 8;
    memset(data, 0, null_bytes);
    for (size_t i = 0; i < column_nullables.size(); ++i) {
      if (column_nullables[i]) {
        *(data + i / 8) = *(data + i / 8) & (0x01 << (i % 8));
      }
    }
    data += null_bytes;
    // crc
    data += 4;
    size_t len = data - start;
    buffer.resize(len);
    return buffer;
  }

  static void BuildFieldsValue(
      const std::vector<bool> &column_null,
      const std::vector<enum_field_types> &column_types,
      const std::vector<std::string> &column_values,
      const TableMapEventPtr &table_map, uchar **data) {
    uchar *ptr = *data;
    // null bits
    uint32_t bytes = (column_null.size() + 7) / 8;
    memset(ptr, 0, bytes);
    uint32_t column_count = column_null.size();
    for (uint32_t i = 0; i < column_count; ++i) {
      if (column_null[i]) {
        *(ptr + i / 8) = *(ptr + i / 8) & (0x01 << (i % 8));
      }
    }
    ptr += bytes;

    for (size_t i = 0; i < column_types.size(); ++i) {
      int32_t meta = table_map->column_info(i).meta;
      if (column_values[i].empty()) {
        continue;
      }
      switch (column_types[i]) {
        case MYSQL_TYPE_TINY: {
          int8_t value;
          ailego::StringHelper::ToInt8(column_values[i], &value);
          *((int8_t *)ptr) = value;
          ptr += 1;
        } break;
        case MYSQL_TYPE_SHORT: {
          int16_t value;
          ailego::StringHelper::ToInt16(column_values[i], &value);
          int2store(ptr, (uint16_t)value);
          ptr += 2;
        } break;
        case MYSQL_TYPE_LONG: {
          int32_t value;
          ailego::StringHelper::ToInt32(column_values[i], &value);
          int4store(ptr, (uint32_t)value);
          ptr += 4;
        } break;
        case MYSQL_TYPE_FLOAT: {
          float value;
          ailego::StringHelper::ToFloat(column_values[i], &value);
          *(float *)ptr = value;
          ptr += 4;
        } break;
        case MYSQL_TYPE_DOUBLE: {
          double value;
          ailego::StringHelper::ToDouble(column_values[i], &value);
          *(double *)ptr = value;
          ptr += 8;
        } break;
        case MYSQL_TYPE_TIMESTAMP:
        case MYSQL_TYPE_TIMESTAMP2: {
          uint32_t value;
          ailego::StringHelper::ToUint32(column_values[i], &value);
          mi_int4store(ptr, value);
          ptr += 4;
        } break;
        case MYSQL_TYPE_LONGLONG: {
          int64_t value;
          ailego::StringHelper::ToInt64(column_values[i], &value);
          int8store(ptr, value);
          ptr += 8;
        } break;
        case MYSQL_TYPE_INT24: {
          int32_t value;
          ailego::StringHelper::ToInt32(column_values[i], &value);
          int2store(ptr, (uint16_t)value);
          *(ptr + 2) = (uchar)(((uint32_t)value) >> 16);
          ptr += 3;
        } break;
        case MYSQL_TYPE_DATE: {
          uint32_t value;
          ailego::StringHelper::ToUint32(column_values[i], &value);
          int2store(ptr, (uint16_t)value);
          *(ptr + 2) = (uchar)(value >> 16);
          ptr += 3;
        } break;
        case MYSQL_TYPE_TIME:
        case MYSQL_TYPE_TIME2: {
          int64_t value;
          ailego::StringHelper::ToInt64(column_values[i], &value);
          value += TIME_INT_OFS;
          mi_int3store(ptr, value);
          ptr += 3;
        } break;
        case MYSQL_TYPE_DATETIME:
        case MYSQL_TYPE_DATETIME2: {
          int64_t value;
          ailego::StringHelper::ToInt64(column_values[i], &value);
          value += DATETIMEF_INT_OFS;
          mi_int5store(ptr, value);
          ptr += 5;
        } break;
        case MYSQL_TYPE_YEAR: {
          uint8_t value;
          ailego::StringHelper::ToUint8(column_values[i], &value);
          *(uint8_t *)ptr = value;
          ptr += 1;
        } break;
        case MYSQL_TYPE_BIT:
          break;
        case MYSQL_TYPE_JSON:
          break;
        case MYSQL_TYPE_NEWDECIMAL:
          break;
        case MYSQL_TYPE_BLOB:
          break;
        case MYSQL_TYPE_VARCHAR:
        case MYSQL_TYPE_VAR_STRING: {
          if (meta < 256) {
            *(uint8_t *)ptr = (uint8_t)column_values[i].size();
            ptr += 1;
          } else {
            int2store(ptr, (uint16_t)column_values[i].size());
            ptr += 2;
          }
          memcpy(ptr, column_values[i].data(), column_values[i].size());
          ptr += column_values[i].size();
        } break;
        case MYSQL_TYPE_STRING: {
          memcpy(ptr, column_values[i].data(), column_values[i].size());
          ptr += column_values[i].size();
        } break;
        case MYSQL_TYPE_GEOMETRY:
          break;
        default:
          break;
      }
    }
    *data = ptr;
  }

  static std::string BuildWriteRowsEvent(
      uint64_t table_id, const std::vector<bool> &column_null,
      const std::vector<enum_field_types> &column_types,
      const std::vector<std::string> &column_values,
      const TableMapEventPtr &table_map,
      EventType event_type = WRITE_ROWS_EVENT_V1, size_t rows_count = 1) {
    std::string buffer(10240, 0);
    uchar *data = (uchar *)(&(buffer[0]));
    uchar *start = data;
    BuildBasicEvent(event_type, &data);
    // table id
    int4store(data, (uint32_t)table_id);
    data += 4;
    int2store(data, (uint16_t)(table_id >> 32));
    data += 2;
    // flags
    data += 2;
    // extra data
    int2store(data, (uint16_t)2);
    data += 2;
    // columns count
    uint32_t column_count = column_null.size();
    *data = (uchar)column_count;
    data += 1;
    // present columns
    uint32_t bytes = (column_count + 7) / 8;
    for (uint32_t i = 0; i < bytes; ++i) {
      *(data + i) = 0xFF;
    }
    data += bytes;

    // rows buffer
    for (size_t i = 0; i < rows_count; ++i) {
      BuildFieldsValue(column_null, column_types, column_values, table_map,
                       &data);
    }

    // crc
    data += 4;
    size_t len = data - start;
    buffer.resize(len);
    return buffer;
  }

  static std::string BuildDeleteRowsEvent(
      uint64_t table_id, const std::vector<bool> &column_null,
      const std::vector<enum_field_types> &column_types,
      const std::vector<std::string> &values,
      const TableMapEventPtr &table_map) {
    return BuildWriteRowsEvent(table_id, column_null, column_types, values,
                               table_map, DELETE_ROWS_EVENT_V1);
  }

  static std::string BuildUpdateRowsEvent(
      uint64_t table_id, const std::vector<bool> &column_null,
      const std::vector<enum_field_types> &column_types,
      const std::vector<std::string> &old_values,
      const std::vector<std::string> &new_values,
      const TableMapEventPtr &table_map) {
    std::string buffer(10240, 0);
    uchar *data = (uchar *)(&(buffer[0]));
    uchar *start = data;
    BuildBasicEvent(UPDATE_ROWS_EVENT_V1, &data);
    // table id
    int4store(data, (uint32_t)table_id);
    data += 4;
    int2store(data, (uint16_t)(table_id >> 32));
    data += 2;
    // flags
    data += 2;
    // extra data
    int2store(data, (uint16_t)2);
    data += 2;
    // columns count
    uint32_t column_count = column_null.size();
    *data = (uchar)column_count;
    data += 1;
    // present columns update
    uint32_t bytes = (column_count + 7) / 8;
    for (uint32_t i = 0; i < bytes; ++i) {
      *(data + i) = 0xFF;
    }
    data += bytes;
    for (uint32_t i = 0; i < bytes; ++i) {
      *(data + i) = 0xFF;
    }
    data += bytes;

    // rows buffer
    BuildFieldsValue(column_null, column_types, old_values, table_map, &data);

    BuildFieldsValue(column_null, column_types, new_values, table_map, &data);

    // crc
    data += 4;
    size_t len = data - start;
    buffer.resize(len);
    return buffer;
  }

  static std::string BuildOtherEvent(EventType type) {
    std::string buffer(10240, 0);
    uchar *data = (uchar *)(&(buffer[0]));
    uchar *start = data;
    BuildBasicEvent(type, &data);
    data += 4;

    size_t len = data - start;
    buffer.resize(len);
    return buffer;
  }

 private:
  const static int64_t TIME_INT_OFS = 0x800000;
  const static uint64_t DATETIMEF_INT_OFS = 0x8000000000;
};

}  // namespace repository
}  // namespace be
}  // namespace proxima
