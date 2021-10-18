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
 *   \brief    Bin log event implementation for proxima search engine
 */

//!!! undef VERSION must be after include my_global.h
#include "binlog_event.h"
#include <mysql.h>
#include <my_global.h>
#include "repository_common/logger.h"
#include "log_context.h"

namespace proxima {
namespace be {
namespace repository {

static unsigned long GetFieldLength(const char **packet) {
  static constexpr int ONE_BYTE_THRESHOLD = 251;
  static constexpr int TWO_BYTE_THRESHOLD = 252;
  static constexpr int THREE_BYTE_THRESHOLD = 253;

  const unsigned char *pos = reinterpret_cast<const unsigned char *>(*packet);
  if (*pos < ONE_BYTE_THRESHOLD) {
    (*packet)++;
    return *pos;
  }
  if (*pos == ONE_BYTE_THRESHOLD) {
    (*packet)++;
    return ((unsigned long)~0);  // NULL_LENGTH;
  }
  if (*pos == TWO_BYTE_THRESHOLD) {
    (*packet) += 3;
    return (unsigned long)uint2korr(pos + 1);
  }
  if (*pos == THREE_BYTE_THRESHOLD) {
    (*packet) += 4;
    return (unsigned long)uint3korr(pos + 1);
  }

  (*packet) += 9; /* Must be 254 when here */
  return (unsigned long)uint4korr(pos + 1);
}

static uint8_t GetUint8(const void *buf) {
  const uint8_t *ptr = static_cast<const uint8_t *>(buf);
  return *ptr;
}

/**
 * BasicEvent Description
 * 4              timestamp
 * 1              event type
 * 4              server-id
 * 4              event-size
 * if binlog-version > 1:
 * 4              log pos
 * 2              flags
 */
BasicEvent::BasicEvent(const char *event_buf, unsigned int length) {
  data_.assign(event_buf, length);
  buffer_ = const_cast<char *>(data_.data());
  buffer_len_ = length;
  if (buffer_len_ < LOG_EVENT_HEADER_LEN) {
    LOG_ERROR("Event length check failed. len[%u]", length);
    is_valid_ = false;
  } else {
    type_ = (EventType)buffer_[EVENT_TYPE_OFFSET];
    timestamp_ = uint4korr(buffer_);
    server_id_ = uint4korr(buffer_ + SERVER_ID_OFFSET);
    log_pos_ = uint4korr(buffer_ + LOG_POS_OFFSET);
    is_valid_ = true;
  }
}

/**
 * QueryEvent Description
 * 4              slave_proxy_id
 * 4              execution time
 * 1              schema length
 * 2              error-code
 * if binlog-version ≥ 4:
 * 2              status-vars length
 * string[$len]   status-vars
 * string[$len]   schema
 * 1              [00]
 * string[EOF]    query
 */
QueryEvent::QueryEvent(const char *buf, unsigned int length)
    : BasicEvent(buf, length) {
  if (length < LOG_EVENT_HEADER_LEN + QE_POST_HEADER_SCHEMA_LEN_OFFSET + 1) {
    LOG_ERROR("Query event length check failed. len[%u]", length);
    is_valid_ = false;
    return;
  }
  unsigned int offset = LOG_EVENT_HEADER_LEN + QE_POST_HEADER_SCHEMA_LEN_OFFSET;
  unsigned int db_len = (unsigned int)buf[offset];
  unsigned short status_var_len;
  offset = LOG_EVENT_HEADER_LEN + QE_POST_HEADER_STATUS_LEN_OFFSET;
  status_var_len = (unsigned short)uint2korr(buf + offset);

  offset = LOG_EVENT_HEADER_LEN + QE_POST_HEADER_LEN + status_var_len;
  db_name_.assign(buf + offset, db_len);
  offset = offset + db_len + 1;
  query_.assign(buf + offset, length - offset - CRC_LEN);
  is_valid_ = true;
}

/**
   RotateEvent Description
   if binlog-version > 1 {
   8              position
     }
   string[p]      name of the next binlog
 */
RotateEvent::RotateEvent(const char *buf, unsigned int length, bool has_crc)
    : BasicEvent(buf, length) {
  if (length < LOG_EVENT_HEADER_LEN + RE_POST_HEADER_LEN) {
    LOG_ERROR("Rotate event length check failed. len[%u]", length);
    is_valid_ = false;
    return;
  }
  const char *ptr = buf + LOG_EVENT_HEADER_LEN;

  position_ = (uint64_t)uint8korr(ptr);
  unsigned int file_len = length - LOG_EVENT_HEADER_LEN - RE_POST_HEADER_LEN;
  if (has_crc) {
    file_len -= CRC_LEN;
  }
  ptr += sizeof(uint64_t);
  next_binlog_name_.assign(ptr, file_len);
  is_valid_ = true;
}

/**
 * TableMapEvent Description
 * post-header:
 *    if post_header_len == 6 {
 *  4              table id
 *    } else {
 *  6              table id
 *    }
 *  2              flags
 *
 * payload:
 *  1              schema name length
 *  string         schema name
 *  1              [00]
 *  1              table name length
 *  string         table name
 *  1              [00]
 *  lenenc-int     column-count
 *  string.var_len [length=$column-count] column-def
 *  lenenc-str     column-meta-def
 *  n              NULL-bitmask, length: (column-count + 8) / 7
 */
TableMapEvent::TableMapEvent(const char *buf, unsigned int length)
    : BasicEvent(buf, length) {
  if (length < LOG_EVENT_HEADER_LEN + TME_POST_HEADER_LEN) {
    LOG_ERROR("TableMap event length check failed. len[%u]", length);
    is_valid_ = false;
    return;
  }

  const char *ptr = buffer_ + LOG_EVENT_HEADER_LEN;
  // table id
  table_id_ = (unsigned long)uint6korr(ptr);
  ptr += 6;
  // flags
  ptr += 2;
  // schema
  size_t db_len = *ptr;
  ptr += 1;
  database_name_.assign(ptr, db_len);
  ptr += db_len + 1;
  // table
  size_t table_len = *ptr;
  ptr += 1;
  table_name_.assign(ptr, table_len);
  ptr += table_len + 1;
  // column
  column_count_ = GetFieldLength(&ptr);
  column_info_.resize(column_count_);
  for (unsigned long i = 0; i < column_count_; ++i) {
    column_info_[i].type = (int32_t)GetUint8(ptr + i);
  }
  ptr += column_count_;

  unsigned int bytes_read = ptr - buffer_;
  if (bytes_read < length) {
    field_metadata_size_ = GetFieldLength(&ptr);
    if (field_metadata_size_ > column_count_ * 2) {
      return;
    }
    decode_meta_data(ptr);
    ptr += field_metadata_size_;
    uint8_t *null_bits = (uint8_t *)ptr;
    for (unsigned long i = 0; i < column_count_; ++i) {
      if (*(null_bits + i / 8) & (0x01 << (i % 8))) {
        column_info_[i].nullable = true;
      } else {
        column_info_[i].nullable = false;
      }
    }
  }
  is_valid_ = true;
}

/**
 * RowsEvent Description
 * header:
 *  if post_header_len == 6 {
 * 4                    table id
 *  } else {
 * 6                    table id
 *  }
 * 2                    flags
 *  if version == 2 {
 * 2                    extra-data-length
 * string.var_len       extra-data
 *  }
 *
 * body:
 * lenenc_int           number of columns
 * string.var_len       columns-present-bitmap1, length: (num of columns+7)/8
 *  if UPDATE_ROWS_EVENTv1 or v2 {
 * string.var_len       columns-present-bitmap2, length: (num of columns+7)/8
 *  }
 *
 * rows:
 * string.var_len       nul-bitmap, length (bits set in
 * 'columns-present-bitmap1'+7)/8 string.var_len       value of each field as
 * defined in table-map if UPDATE_ROWS_EVENTv1 or v2 { string.var_len
 * nul-bitmap, length (bits set in 'columns-present-bitmap2'+7)/8 string.var_len
 * value of each field as defined in table-map
 *   }
 *   ... repeat rows until event-end
 */
RowsEvent::RowsEvent(const char *buf, unsigned int length)
    : BasicEvent(buf, length) {
  if (length < LOG_EVENT_HEADER_LEN + ROWS_EVENT_POST_HEADER_LEN) {
    LOG_ERROR("Row event length check failed. len[%u]", length);
    is_valid_ = false;
    return;
  }

  const char *ptr = buffer_ + LOG_EVENT_HEADER_LEN;
  // post header
  table_id_ = (unsigned long)uint6korr(ptr);
  ptr += ROWS_EVENT_POST_HEADER_LEN;

  uint16_t var_header_len = (uint16_t)uint2korr(ptr);
  if (var_header_len < 2) {
    LOG_ERROR("RowsEvent var_header_len check failed. len[%u]", var_header_len);
    is_valid_ = false;
  }
  ptr += var_header_len;

  // body
  column_count_ = GetFieldLength(&ptr);
  bits_length_ = (column_count_ + 7) / 8;
  present_columns_ = (uint8_t *)ptr;
  ptr += bits_length_;
  if (type_ == UPDATE_ROWS_EVENT || type_ == UPDATE_ROWS_EVENT_V1) {
    present_columns_update_ = (uint8_t *)ptr;
    ptr += bits_length_;
  }

  unsigned int bytes_read = ptr - buffer_;
  if (bytes_read > length) {
    LOG_ERROR("RowsEvent read bytes check failed. bytes[%u]", bytes_read);
    is_valid_ = false;
  }
  rows_buf_ = ptr;
  rows_end_ = buffer_ + length - CRC_LEN;
  cur_buf_ = rows_buf_;
  is_valid_ = true;
}

void TableMapEvent::decode_meta_data(const char *ptr) {
  for (unsigned long i = 0; i < column_count_; ++i) {
    ColumnInfo &info = column_info_[i];
    switch (info.type) {
      case MYSQL_TYPE_TINY_BLOB:
      case MYSQL_TYPE_BLOB:
      case MYSQL_TYPE_MEDIUM_BLOB:
      case MYSQL_TYPE_LONG_BLOB:
      case MYSQL_TYPE_DOUBLE:
      case MYSQL_TYPE_FLOAT:
      case MYSQL_TYPE_GEOMETRY:
      case MYSQL_TYPE_JSON:
        info.meta = (int32_t)GetUint8(ptr);
        ++ptr;
        break;
      case MYSQL_TYPE_SET:
      case MYSQL_TYPE_ENUM:
        LOG_WARN("This type cannot exist in binlog.");
        break;
      case MYSQL_TYPE_STRING:
        info.meta = ((int32_t)GetUint8(ptr)) << 8;  // real type
        info.meta += (int32_t)GetUint8(ptr + 1);    // field length
        ptr += 2;
        break;
      case MYSQL_TYPE_BIT:
        info.meta = (int32_t)uint2korr(ptr);
        ptr += 2;
        break;
      case MYSQL_TYPE_VARCHAR:
        info.meta = (int32_t)uint2korr(ptr);
        ptr += 2;
        break;
      case MYSQL_TYPE_NEWDECIMAL:
        info.meta = ((int32_t)GetUint8(ptr)) << 8;  // precision
        info.meta += (int32_t)GetUint8(ptr + 1);    // decimals
        ptr += 2;
        break;
      case MYSQL_TYPE_TIME2:
      case MYSQL_TYPE_DATETIME2:
      case MYSQL_TYPE_TIMESTAMP2: {
        info.meta = (int32_t)GetUint8(ptr);
        ++ptr;
        break;
      }
      default:
        info.meta = 0;
        break;
    }
  }
}

void RowsEvent::fill_table_map(const LogContext &context) {
  table_map_ = context.table_map();
  start_position_ = table_map_->log_pos() - table_map_->buffer_length();
}

}  // end namespace repository
}  // namespace be
}  // namespace proxima
