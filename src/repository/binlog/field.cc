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
 *   \brief    Field interface implementation for proxima search engine
 */

#include "field.h"
#include <iomanip>
//!!! undef VERSION must be after include my_global.h
#include <my_global.h>
#include <my_sys.h>

extern "C" {
#include <decimal.h>
}
#include <myisampack.h>
#include "common/logger.h"

namespace proxima {
namespace be {
namespace repository {

static std::string UsecondsToStr(int32_t frac, uint32_t decimals) {
  std::string sec = std::to_string(frac);
  if (sec.size() < 6) {
    std::string tmp = std::string(6 - sec.size(), '0');
    tmp.append(sec);
    sec = tmp;
  }
  return sec.substr(0, decimals);
}

static void FormatNumber(std::ostringstream &oss, int32_t value,
                         uint32_t precision) {
  if (precision == 2) {
    value %= 100;
  } else if (precision == 4) {
    value %= 10000;
  }
  oss << std::setw(precision) << std::setfill('0') << value;
}

static uint8_t HexToValue(char ch) {
  if (ch >= '0' && ch <= '9') {
    return ch - '0';
  } else if (ch >= 'a' && ch <= 'f') {
    return ch - 'a' + 10;
  } else if (ch >= 'A' && ch <= 'F') {
    return ch - 'A' + 10;
  } else {
    return 0;
  }
}
static void HexToBinary(const char *data, unsigned long length,
                        std::string *mutable_bytes) {
  mutable_bytes->resize(length / 2);
  uint8_t *ptr = (uint8_t *)(&((*mutable_bytes)[0]));
  for (size_t i = 0; i + 1 < length; i += 2) {
    ptr[i >> 1] = (HexToValue(data[i]) << 4) + HexToValue(data[i + 1]);
  }
}

FieldPtr FieldFactory::Create(const std::string &field_name,
                              const FieldAttr &attr) {
  FieldPtr field;
  const FieldMetaPtr &meta = attr.meta();
  switch (meta->type()) {
    case MYSQL_TYPE_TINY:
      field = std::make_shared<FieldTiny>(field_name, attr);
      break;
    case MYSQL_TYPE_SHORT:
      field = std::make_shared<FieldShort>(field_name, attr);
      break;
    case MYSQL_TYPE_LONG:
      field = std::make_shared<FieldLong>(field_name, attr);
      break;
    case MYSQL_TYPE_FLOAT:
      field = std::make_shared<FieldFloat>(field_name, attr);
      break;
    case MYSQL_TYPE_DOUBLE:
      field = std::make_shared<FieldDouble>(field_name, attr);
      break;
    case MYSQL_TYPE_TIMESTAMP:
    case MYSQL_TYPE_TIMESTAMP2:
      field = std::make_shared<FieldTimestamp>(field_name, attr);
      break;
    case MYSQL_TYPE_LONGLONG:
      field = std::make_shared<FieldLongLong>(field_name, attr);
      break;
    case MYSQL_TYPE_INT24:
      field = std::make_shared<FieldInt24>(field_name, attr);
      break;
    case MYSQL_TYPE_DATE:
      field = std::make_shared<FieldDate>(field_name, attr);
      break;
    case MYSQL_TYPE_TIME:
    case MYSQL_TYPE_TIME2:
      field = std::make_shared<FieldTime>(field_name, attr);
      break;
    case MYSQL_TYPE_DATETIME:
    case MYSQL_TYPE_DATETIME2:
      field = std::make_shared<FieldDatetime>(field_name, attr);
      break;
    case MYSQL_TYPE_YEAR:
      field = std::make_shared<FieldYear>(field_name, attr);
      break;
    case MYSQL_TYPE_BIT:
      field = std::make_shared<FieldBit>(field_name, attr);
      break;
    case MYSQL_TYPE_JSON:
      field = std::make_shared<FieldJson>(field_name, attr);
      break;
    case MYSQL_TYPE_NEWDECIMAL:
      field = std::make_shared<FieldDecimal>(field_name, attr);
      break;
    case MYSQL_TYPE_BLOB:
      field = std::make_shared<FieldBlob>(field_name, attr);
      break;
    case MYSQL_TYPE_VARCHAR:
    case MYSQL_TYPE_VAR_STRING:
      field = std::make_shared<FieldVarString>(field_name, attr);
      break;
    case MYSQL_TYPE_STRING:
      field = std::make_shared<FieldString>(field_name, attr);
      break;
    case MYSQL_TYPE_GEOMETRY:
      field = std::make_shared<FieldGeometry>(field_name, attr);
      break;
    default:
      LOG_ERROR("Unsupported field type %d.", meta->type());
      break;
  }

  return field;
}

const std::string Field::UTF8_CHARSET_NAME = "utf8_general_ci";

Field::Field(const std::string &name, const FieldAttr &attr)
    : field_name_(name) {
  auto &meta = attr.meta();
  field_type_ = meta->type();
  dst_field_type_ = convert_field_type(field_type_);
  field_length_ = meta->length();
  field_decimals_ = meta->decimals();
  flags_ = meta->flags();
  is_index_ = attr.is_index();
  is_forward_ = attr.is_forward();
  is_selected_ = attr.is_selected();
  select_field_ = name;

  collation_ = attr.collation();
  if (!collation_.empty()) {
    dst_cs_ = get_charset_by_name(UTF8_CHARSET_NAME.c_str(), myf(0));
    src_cs_ = get_charset_by_name(collation_.c_str(), myf(0));
  }
}

FieldType Field::convert_field_type(enum_field_types types) {
  switch (types) {
    case MYSQL_TYPE_TINY:
    case MYSQL_TYPE_SHORT:
    case MYSQL_TYPE_INT24:
    case MYSQL_TYPE_LONG:
      return GenericValueMeta::FT_INT32;
    case MYSQL_TYPE_FLOAT:
      return GenericValueMeta::FT_FLOAT;
    case MYSQL_TYPE_DOUBLE:
      return GenericValueMeta::FT_DOUBLE;
    case MYSQL_TYPE_LONGLONG:
      return GenericValueMeta::FT_INT64;
    case MYSQL_TYPE_TIMESTAMP:
    case MYSQL_TYPE_TIMESTAMP2:
    case MYSQL_TYPE_DATE:
    case MYSQL_TYPE_TIME:
    case MYSQL_TYPE_TIME2:
    case MYSQL_TYPE_DATETIME:
    case MYSQL_TYPE_DATETIME2:
    case MYSQL_TYPE_YEAR:
      return GenericValueMeta::FT_STRING;
    case MYSQL_TYPE_BIT:
      return GenericValueMeta::FT_UINT64;
    case MYSQL_TYPE_JSON:
      return GenericValueMeta::FT_STRING;
    case MYSQL_TYPE_NEWDECIMAL:
      return GenericValueMeta::FT_STRING;
    case MYSQL_TYPE_BLOB:
      return GenericValueMeta::FT_BYTES;
    case MYSQL_TYPE_VARCHAR:
    case MYSQL_TYPE_VAR_STRING:
      return GenericValueMeta::FT_BYTES;
    case MYSQL_TYPE_STRING:
      return GenericValueMeta::FT_STRING;
    case MYSQL_TYPE_GEOMETRY:
      return GenericValueMeta::FT_BYTES;
    default:
      return GenericValueMeta::FT_BYTES;
  }
}

bool FieldInteger::unpack_text(const void *data, unsigned long length,
                               GenericValue *value) {
  if (!data || !length) {
    return false;
  }

  if (!is_unsigned()) {
    value->set_int32_value(static_cast<int32_t>(
        std::strtol(static_cast<const char *>(data), nullptr, 0)));
  } else {
    value->set_uint32_value(static_cast<uint32_t>(
        std::strtoul(static_cast<const char *>(data), nullptr, 0)));
  }

  return true;
}

const void *FieldTiny::unpack_binary(const void *data, const void *data_end,
                                     const ColumnInfo & /*info*/,
                                     GenericValue *value) {
  const int8_t *ptr = static_cast<const int8_t *>(data);
  uint32_t data_len = sizeof(int8_t);
  if (ptr + data_len > data_end) {
    return nullptr;
  }
  if (!is_unsigned()) {
    int32_t tmp = (int32_t)ptr[0];
    value->set_int32_value(tmp);
  } else {
    uint32_t tmp = (uint32_t)(uint8_t)ptr[0];
    value->set_uint32_value(tmp);
  }

  return ptr + data_len;
}

const void *FieldShort::unpack_binary(const void *data, const void *data_end,
                                      const ColumnInfo & /*info*/,
                                      GenericValue *value) {
  const uint8_t *ptr = static_cast<const uint8_t *>(data);
  uint32_t data_len = sizeof(int16_t);
  if (ptr + data_len > data_end) {
    return nullptr;
  }

  if (!is_unsigned()) {
    int32_t tmp = (int32_t)sint2korr(ptr);
    value->set_int32_value(tmp);
  } else {
    uint32_t tmp = (uint32_t)uint2korr(ptr);
    value->set_uint32_value(tmp);
  }

  return ptr + data_len;
}

const void *FieldInt24::unpack_binary(const void *data, const void *data_end,
                                      const ColumnInfo & /*info*/,
                                      GenericValue *value) {
  const uint8_t *ptr = static_cast<const uint8_t *>(data);
  uint32_t data_len = 3;  // sizeof(int24)
  if (ptr + data_len > data_end) {
    return nullptr;
  }

  if (!is_unsigned()) {
    int32_t tmp = (int32_t)sint3korr(ptr);
    value->set_int32_value(tmp);
  } else {
    uint32_t tmp = (uint32_t)uint3korr(ptr);
    value->set_uint32_value(tmp);
  }

  return ptr + data_len;
}

const void *FieldLong::unpack_binary(const void *data, const void *data_end,
                                     const ColumnInfo & /*info*/,
                                     GenericValue *value) {
  const uint8_t *ptr = static_cast<const uint8_t *>(data);
  uint32_t data_len = sizeof(int32_t);
  if (ptr + data_len > data_end) {
    return nullptr;
  }
  if (!is_unsigned()) {
    int32_t tmp = (int32_t)sint4korr(ptr);
    value->set_int32_value(tmp);
  } else {
    uint32_t tmp = (uint32_t)uint4korr(ptr);
    value->set_uint32_value(tmp);
  }

  return ptr + data_len;
}

const void *FieldLongLong::unpack_binary(const void *data, const void *data_end,
                                         const ColumnInfo & /*info*/,
                                         GenericValue *value) {
  const uint8_t *ptr = static_cast<const uint8_t *>(data);
  uint32_t data_len = sizeof(int64_t);
  if (ptr + data_len > data_end) {
    return nullptr;
  }

  if (!is_unsigned()) {
    int64_t tmp = (int64_t)sint8korr(ptr);
    value->set_int64_value(tmp);
  } else {
    uint64_t tmp = (uint64_t)uint8korr(ptr);
    value->set_uint64_value(tmp);
  }

  return ptr + data_len;
}

bool FieldLongLong::unpack_text(const void *data, unsigned long length,
                                GenericValue *value) {
  if (!data || !length) {
    return false;
  }

  if (!is_unsigned()) {
    value->set_int64_value(static_cast<int64_t>(
        std::strtoll(static_cast<const char *>(data), nullptr, 0)));
  } else {
    value->set_uint64_value(static_cast<uint64_t>(
        std::strtoull(static_cast<const char *>(data), nullptr, 0)));
  }

  return true;
}

const void *FieldFloat::unpack_binary(const void *data, const void *data_end,
                                      const ColumnInfo & /*info*/,
                                      GenericValue *value) {
  const uint8_t *ptr = static_cast<const uint8_t *>(data);
  uint32_t data_len = sizeof(float);
  if (ptr + data_len > data_end) {
    return nullptr;
  }
  value->set_float_value(*(static_cast<const float *>(data)));

  return ptr + data_len;
}

bool FieldFloat::unpack_text(const void *data, unsigned long length,
                             GenericValue *value) {
  if (!data || !length) {
    return false;
  }

  value->set_float_value(std::strtof(static_cast<const char *>(data), nullptr));
  return true;
}

const void *FieldDouble::unpack_binary(const void *data, const void *data_end,
                                       const ColumnInfo & /*info*/,
                                       GenericValue *value) {
  const uint8_t *ptr = static_cast<const uint8_t *>(data);
  uint32_t data_len = sizeof(double);
  if (ptr + data_len > data_end) {
    return nullptr;
  }
  double tmp = *(static_cast<const double *>(data));
  value->set_double_value(tmp);

  return ptr + data_len;
}

bool FieldDouble::unpack_text(const void *data, unsigned long length,
                              GenericValue *value) {
  if (!data || !length) {
    return false;
  }

  value->set_double_value(
      std::strtod(static_cast<const char *>(data), nullptr));
  return true;
}

const void *FieldDecimal::unpack_binary(const void *data, const void *data_end,
                                        const ColumnInfo &info,
                                        GenericValue *value) {
  uint32_t precision = (info.meta >> 8);
  uint32_t decimals = (info.meta & 0xff);
  int data_len = decimal_bin_size(precision, decimals);
  const uint8_t *ptr = static_cast<const uint8_t *>(data);
  if (ptr + data_len > data_end) {
    return nullptr;
  }

  int32_t decimal_count = decimal_size(precision, decimals);
  std::vector<decimal_digit_t> decimal_buf(decimal_count, 0);
  decimal_t decimal;
  decimal.buf = &(decimal_buf[0]);
  decimal.len = decimal_count;
  int ret = bin2decimal(ptr, &decimal, precision, decimals);
  if (ret != 0) {
    LOG_ERROR("Execute bin2decimal failed. ret[%d]", ret);
    return nullptr;
  }

  int result_size = decimal_string_size(&decimal);
  std::string decimal_str;
  decimal_str.resize(result_size);
  ret =
      decimal2string(&decimal, &decimal_str[0], &result_size, 0, decimals, '0');
  if (ret != 0) {
    LOG_ERROR("Execute decimal2string failed. ret[%d]", ret);
    return nullptr;
  }
  decimal_str.resize(result_size);

  std::string *mutable_value = value->mutable_string_value();
  mutable_value->swap(decimal_str);

  return ptr + data_len;
}

bool FieldDecimal::unpack_text(const void *data, unsigned long length,
                               GenericValue *value) {
  if (!data || !length) {
    return false;
  }

  value->set_string_value(static_cast<const char *>(data), length);
  return true;
}

const void *FieldBit::unpack_binary(const void *data, const void *data_end,
                                    const ColumnInfo &info,
                                    GenericValue *value) {
  uint32_t bits = ((info.meta >> 8) * 8) + (info.meta & 0xff);
  uint32_t data_len = (bits + 7) / 8;
  const uint8_t *ptr = static_cast<const uint8_t *>(data);
  if (ptr + data_len > data_end) {
    return nullptr;
  }
  uint64_t bit_value = 0;
  switch (data_len) {
    case 1:
      bit_value = (uint64_t)mi_uint1korr(ptr);
      break;
    case 2:
      bit_value = (uint64_t)mi_uint2korr(ptr);
      break;
    case 3:
      bit_value = (uint64_t)mi_uint3korr(ptr);
      break;
    case 4:
      bit_value = (uint64_t)mi_uint4korr(ptr);
      break;
    case 5:
      bit_value = (uint64_t)mi_uint5korr(ptr);
      break;
    case 6:
      bit_value = (uint64_t)mi_uint6korr(ptr);
      break;
    case 7:
      bit_value = (uint64_t)mi_uint7korr(ptr);
      break;
    case 8:
      bit_value = (uint64_t)mi_uint8korr(ptr);
      break;
    default:
      return nullptr;
  }

  value->set_uint64_value(bit_value);

  return ptr + data_len;
}

bool FieldBit::unpack_text(const void *data, unsigned long length,
                           GenericValue *value) {
  if (!data || !length) {
    return false;
  }

  const char *ptr = static_cast<const char *>(data);
  value->set_uint64_value((uint64_t)std::strtoul(ptr, nullptr, 0));
  return true;
}

const void *FieldDatetime::unpack_binary(const void *data, const void *data_end,
                                         const ColumnInfo &info,
                                         GenericValue *value) {
  const uint8_t *ptr = static_cast<const uint8_t *>(data);
  unsigned fixed_len = 5;
  unsigned int total_length = fixed_len + (info.meta + 1) / 2;
  if (ptr + total_length > data_end) {
    return nullptr;
  }

  // big endian
  int64_t int_part = (int64_t)mi_uint5korr(ptr) - DATETIMEF_INT_OFS;
  int32_t frac = 0;
  switch (info.meta) {
    case 1:
    case 2:
      frac = (int32_t)(*(ptr + fixed_len)) * 10000;
      break;
    case 3:
    case 4:
      frac = (int32_t)mi_uint2korr(ptr + fixed_len) * 100;
      break;
    case 5:
    case 6:
      frac = (int32_t)mi_uint3korr(ptr + fixed_len);
      break;
    default:
      frac = 0;
      break;
  }
  std::ostringstream oss;
  if (int_part == 0) {
    oss << "0000-00-00 00:00:00";
  } else {
    int64_t ymd = int_part >> 17;
    int64_t ym = ymd >> 5;
    int64_t hms = int_part % (1 << 17);
    FormatNumber(oss, ym / 13, 4);
    oss << "-";
    FormatNumber(oss, ym % 13, 2);
    oss << "-";
    FormatNumber(oss, (ymd % (1 << 5)), 2);
    oss << " ";
    FormatNumber(oss, hms >> 12, 2);
    oss << ":";
    FormatNumber(oss, ((hms >> 6) % (1 << 6)), 2);
    oss << ":";
    FormatNumber(oss, (hms % (1 << 6)), 2);
  }
  if (frac >= 1) {
    oss << "." << UsecondsToStr(frac, info.meta);
  }
  value->set_string_value(oss.str());

  return ptr + total_length;
}

bool FieldDatetime::unpack_text(const void *data, unsigned long length,
                                GenericValue *value) {
  if (!data || !length) {
    return false;
  }

  const char *ptr = static_cast<const char *>(data);
  value->set_string_value(ptr, length);
  return true;
}

const void *FieldTimestamp::unpack_binary(const void *data,
                                          const void *data_end,
                                          const ColumnInfo &info,
                                          GenericValue *value) {
  const uint8_t *ptr = static_cast<const uint8_t *>(data);
  unsigned int seconds_len = 4;
  unsigned int total_len = seconds_len + (info.meta + 1) / 2;
  if (ptr + total_len > data_end) {
    return nullptr;
  }
  uint64_t tv_seconds = (uint64_t)mi_uint4korr(ptr);
  int32_t tv_usec = 0;
  switch (info.meta) {
    case 1:
    case 2:
      tv_usec = (int32_t)(*(ptr + seconds_len)) * 10000;
      break;
    case 3:
    case 4:
      tv_usec = (int32_t)mi_uint2korr(ptr + seconds_len) * 100;
      break;
    case 5:
    case 6:
      tv_usec = (int32_t)mi_uint3korr(ptr + seconds_len);
      break;
    default:
      tv_usec = 0;
      break;
  }
  std::ostringstream oss;
  if (tv_seconds == 0) {
    oss << "0000-00-00 00:00:00";
  } else {
    time_t time = (time_t)tv_seconds;
    tm *local = std::localtime(&time);
    char buf[32];
    strftime(buf, 32, "%Y-%m-%d %H:%M:%S", local);
    oss << buf;
  }

  if (info.meta >= 1) {
    oss << "." << UsecondsToStr(tv_usec, info.meta);
  }
  value->set_string_value(oss.str());

  return ptr + total_len;
}

bool FieldTimestamp::unpack_text(const void *data, unsigned long length,
                                 GenericValue *value) {
  if (!data || !length) {
    return false;
  }

  const char *ptr = static_cast<const char *>(data);
  value->set_string_value(ptr, length);
  return true;
}

const void *FieldTime::unpack_binary(const void *data, const void *data_end,
                                     const ColumnInfo &info,
                                     GenericValue *value) {
  const uint8_t *ptr = static_cast<const uint8_t *>(data);
  unsigned int total_len = 3 + (info.meta + 1) / 2;
  if (ptr + total_len > data_end) {
    return nullptr;
  }
  int64_t int_part = 0;
  int32_t frac = 0;
  int64_t ltime = 0;
  switch (info.meta) {
    case 1:
    case 2:
      int_part = (int64_t)mi_uint3korr(ptr) - TIME_INT_OFS;
      frac = (int32_t)(*(ptr + 3));
      if (int_part < 0 && frac > 0) {
        int_part++;
        frac -= 0x100;
      }
      frac = frac * 10000;
      ltime = int_part << 24;
      break;
    case 3:
    case 4:
      int_part = (int64_t)mi_uint3korr(ptr) - TIME_INT_OFS;
      frac = (int32_t)mi_uint2korr(ptr + 3);
      if (int_part < 0 && frac > 0) {
        int_part++;
        frac -= 0x10000;
      }
      frac = frac * 100;
      ltime = int_part << 24;
      break;
    case 5:
    case 6:
      int_part = (int64_t)mi_uint6korr(ptr) - TIME_OFS;
      ltime = int_part;
      frac = (int32_t)(int_part % (1L << 24));
      break;
    default:
      int_part = (int64_t)mi_uint3korr(ptr) - TIME_INT_OFS;
      ltime = int_part << 24;
      break;
  }
  std::ostringstream oss;
  if (int_part == 0) {
    oss << "00:00:00";
  } else {
    int64_t ultime = labs(ltime);
    int_part = ultime >> 24;
    if (ltime < 0) {
      oss << "-";
    }
    int32_t d = (int32_t)((int_part >> 12) % (1 << 10));
    if (d >= 100) {
      oss << d;
    } else {
      FormatNumber(oss, d, 2);
    }
    oss << ":";
    FormatNumber(oss, (int32_t)((int_part >> 6) % (1 << 6)), 2);
    oss << ":";
    FormatNumber(oss, (int32_t)(int_part % (1 << 6)), 2);
  }
  if (info.meta >= 1) {
    oss << ".";
    oss << UsecondsToStr(abs(frac), info.meta);
  }
  value->set_string_value(oss.str());

  return ptr + total_len;
}

bool FieldTime::unpack_text(const void *data, unsigned long length,
                            GenericValue *value) {
  if (!data || !length) {
    return false;
  }

  const char *ptr = static_cast<const char *>(data);
  value->set_string_value(ptr, length);
  return true;
}

const void *FieldDate::unpack_binary(const void *data, const void *data_end,
                                     const ColumnInfo & /*info*/,
                                     GenericValue *value) {
  const uint8_t *ptr = static_cast<const uint8_t *>(data);
  uint32_t data_len = 3;
  if (ptr + data_len > data_end) {
    return nullptr;
  }
  uint32_t tmp = (uint32_t)uint3korr(ptr);

  std::ostringstream oss;
  if (tmp == 0) {
    oss << "0000-00-00";
  } else {
    FormatNumber(oss, tmp / (16 * 32), 4);
    oss << "-";
    FormatNumber(oss, tmp / 32 % 16, 2);
    oss << "-";
    FormatNumber(oss, tmp % 32, 2);
  }
  value->set_string_value(oss.str());

  return ptr + data_len;
}

bool FieldDate::unpack_text(const void *data, unsigned long length,
                            GenericValue *value) {
  if (!data || !length) {
    return false;
  }

  const char *ptr = static_cast<const char *>(data);
  value->set_string_value(ptr, length);
  return true;
}

const void *FieldYear::unpack_binary(const void *data, const void *data_end,
                                     const ColumnInfo & /*info*/,
                                     GenericValue *value) {
  const uint8_t *ptr = static_cast<const uint8_t *>(data);
  uint32_t data_len = sizeof(uint8_t);
  if (ptr + data_len > data_end) {
    return nullptr;
  }
  int32_t tmp = (int32_t)(*ptr);
  std::ostringstream oss;
  if (tmp == 0) {
    oss << "0000";
  } else {
    oss << tmp + 1900;
  }
  value->set_string_value(oss.str());

  return ptr + data_len;
}

bool FieldYear::unpack_text(const void *data, unsigned long length,
                            GenericValue *value) {
  if (!data || !length) {
    return false;
  }

  const char *ptr = static_cast<const char *>(data);
  value->set_string_value(ptr, length);
  return true;
}

FieldBlob::FieldBlob(const std::string &name, const FieldAttr &attr)
    : Field(name, attr) {
  is_binary_ = flags_ & BINARY_FLAG;
  if (!is_binary_ && dst_cs_ && src_cs_) {
    need_convert_ = !my_charset_same(src_cs_, dst_cs_);
  } else {
    select_field_ = "HEX(" + name + ")";
  }
}

const void *FieldBlob::unpack_binary(const void *data, const void *data_end,
                                     const ColumnInfo &info,
                                     GenericValue *value) {
  uint32_t length = 0;
  uint32_t meta = info.meta;
  const uint8_t *ptr = static_cast<const uint8_t *>(data);
  if (ptr + meta > data_end) {
    return nullptr;
  }
  switch (meta) {
    case 1:
      length = (uint32_t)(*ptr);
      break;
    case 2:
      length = (uint32_t)uint2korr(ptr);
      break;
    case 3:
      length = (uint32_t)uint3korr(ptr);
      break;
    case 4:
      length = (uint32_t)uint4korr(ptr);
      break;
    default:
      return nullptr;
  }
  ptr += meta;
  if (ptr + length > data_end) {
    return nullptr;
  }

  if (!is_binary_) {
    if (!need_convert_) {
      value->set_string_value((const char *)ptr, length);
    } else {
      std::string buffer;
      size_t buffer_len = length * dst_cs_->mbmaxlen + 1;
      buffer.resize(buffer_len);
      uint errors = 0;
      size_t actual_length =
          my_convert(&(buffer[0]), buffer_len, dst_cs_, (const char *)ptr,
                     length, src_cs_, &errors);
      if (errors != 0) {
        LOG_ERROR("Convert charset failed. error_no[%u]", errors);
        return nullptr;
      }
      value->set_string_value(buffer.data(), actual_length);
    }
  } else {
    value->set_bytes_value(ptr, length);
  }

  return ptr + length;
}

bool FieldBlob::unpack_text(const void *data, unsigned long length,
                            GenericValue *value) {
  if (!data) {
    return false;
  }

  const char *ptr = static_cast<const char *>(data);
  if (is_binary_) {
    std::string *mutable_bytes = value->mutable_bytes_value();
    HexToBinary(ptr, length, mutable_bytes);
  } else {
    value->set_string_value((const char *)data, length);
  }

  return true;
}

FieldVarString::FieldVarString(const std::string &name, const FieldAttr &attr)
    : Field(name, attr) {
  is_binary_ = (flags_ & BINARY_FLAG);
  length_bytes_ = (field_length_ < 256) ? 1 : 2;
  if (!is_binary_ && dst_cs_ && src_cs_) {
    need_convert_ = !my_charset_same(src_cs_, dst_cs_);
  } else {
    select_field_ = "HEX(" + name + ")";
  }
}

const void *FieldVarString::unpack_binary(const void *data,
                                          const void *data_end,
                                          const ColumnInfo &info,
                                          GenericValue *value) {
  uint32_t bytes_count;
  const uint8_t *ptr = static_cast<const uint8_t *>(data);
  int32_t len = info.meta;
  if (len < 256) {
    if (ptr + 1 > data_end) {
      return nullptr;
    }
    bytes_count = *ptr;
    ptr += 1;
  } else {
    if (ptr + 2 > data_end) {
      return nullptr;
    }
    bytes_count = (uint32_t)uint2korr(ptr);
    ptr += 2;
  }

  if (ptr + bytes_count > data_end) {
    return nullptr;
  }

  if (!is_binary_) {
    if (!need_convert_) {
      value->set_string_value((const char *)ptr, bytes_count);
    } else {
      std::string buffer;
      size_t buffer_len = field_length_ * dst_cs_->mbmaxlen + 1;
      buffer.resize(buffer_len);
      uint errors = 0;
      size_t actual_length =
          my_convert(&(buffer[0]), buffer_len, dst_cs_, (const char *)ptr,
                     bytes_count, src_cs_, &errors);
      if (errors != 0) {
        LOG_ERROR("Convert charset failed. error_no[%u]", errors);
        return nullptr;
      }
      value->set_string_value(buffer.data(), actual_length);
    }
  } else {
    value->set_bytes_value(ptr, bytes_count);
  }

  return ptr + bytes_count;
}

bool FieldVarString::unpack_text(const void *data, unsigned long length,
                                 GenericValue *value) {
  if (!data) {
    return false;
  }

  const char *ptr = static_cast<const char *>(data);
  if (is_binary_) {
    std::string *mutable_bytes = value->mutable_bytes_value();
    HexToBinary(ptr, length, mutable_bytes);
  } else {
    value->set_string_value(ptr, length);
  }

  return true;
}

FieldString::FieldString(const std::string &name, const FieldAttr &attr)
    : Field(name, attr) {
  is_binary_ = flags_ & BINARY_FLAG;
  is_enum_ = flags_ & ENUM_FLAG;
  is_set_ = flags_ & SET_FLAG;
  if (!is_binary_ && !is_enum_ && !is_set_ && dst_cs_ && src_cs_) {
    need_convert_ = !my_charset_same(src_cs_, dst_cs_);
  } else if (is_enum_ || is_set_) {
    select_field_ = name + "+0";
  } else {
    select_field_ = "HEX(" + name + ")";
  }
}

const void *FieldString::unpack_binary(const void *data, const void *data_end,
                                       const ColumnInfo &info,
                                       GenericValue *value) {
  int32_t len = 0;
  int32_t meta = info.meta;
  enum_field_types parsed_type;
  if (meta < 256) {
    len = meta;
    parsed_type = MYSQL_TYPE_STRING;
  } else {
    int32_t byte0 = meta >> 8;
    int32_t byte1 = meta & 0xFF;
    if ((byte0 & 0x30) != 0x30) {
      len = byte1 | (((byte0 & 0x30) ^ 0x30) << 4);
      parsed_type = (enum_field_types)(byte0 | 0x30);
    } else {
      if (byte0 == MYSQL_TYPE_STRING || byte0 == MYSQL_TYPE_SET ||
          byte0 == MYSQL_TYPE_ENUM) {
        parsed_type = (enum_field_types)byte0;
        len = byte1;
      } else {
        return nullptr;
      }
    }
  }

  if (parsed_type == MYSQL_TYPE_STRING) {
    return parse_string_value(data, data_end, len, value);
  } else if (parsed_type == MYSQL_TYPE_SET) {
    return parse_set_value(data, data_end, meta, value);
  } else if (parsed_type == MYSQL_TYPE_ENUM) {
    return parse_enum_value(data, data_end, len, value);
  } else {
    return nullptr;
  }
}

bool FieldString::unpack_text(const void *data, unsigned long length,
                              GenericValue *value) {
  if (!data) {
    return false;
  }

  const char *ptr = static_cast<const char *>(data);
  if (is_enum_) {
    value->set_int32_value((int32_t)std::strtol(ptr, nullptr, 0));
  } else if (is_set_) {
    value->set_uint64_value((uint64_t)std::strtoul(ptr, nullptr, 0));
  } else {
    if (!is_binary_) {
      value->set_string_value((const char *)data, length);
    } else {
      std::string *mutable_bytes = value->mutable_bytes_value();
      HexToBinary(ptr, length, mutable_bytes);
    }
  }

  return true;
}

const void *FieldString::parse_string_value(const void *data,
                                            const void *data_end, int32_t len,
                                            GenericValue *value) {
  const uint8_t *ptr = static_cast<const uint8_t *>(data);
  uint32_t bytes_count;
  if (len < 256) {
    if (ptr + 1 > data_end) {
      return nullptr;
    }
    bytes_count = (uint32_t)(*ptr);
    ptr++;
  } else {
    bytes_count = (uint32_t)uint2korr(ptr);
    if (ptr + 2 > data_end) {
      return nullptr;
    }
    ptr += 2;
  }
  if (ptr + bytes_count > data_end) {
    return nullptr;
  }

  if (!is_binary_) {
    if (!need_convert_) {
      value->set_string_value((const char *)ptr, bytes_count);
    } else {
      std::string buffer;
      size_t buffer_len = field_length_ * dst_cs_->mbmaxlen + 1;
      buffer.resize(buffer_len);
      uint errors = 0;
      size_t actual_length =
          my_convert(&(buffer[0]), buffer_len, dst_cs_, (const char *)ptr,
                     bytes_count, src_cs_, &errors);
      if (errors != 0) {
        LOG_ERROR("Convert charset failed. error_no[%u]", errors);
        return nullptr;
      }
      value->set_string_value(buffer.data(), actual_length);
    }
  } else {
    value->set_bytes_value(ptr, bytes_count);
    std::string *mutable_bytes = value->mutable_bytes_value();
    mutable_bytes->append(field_length_ - bytes_count, (char)0);
  }

  return ptr + bytes_count;
}

const void *FieldString::parse_set_value(const void *data, const void *data_end,
                                         int32_t meta, GenericValue *value) {
  const uint8_t *ptr = static_cast<const uint8_t *>(data);
  int32_t bits = (meta & 0xFF) * 8;
  int32_t len = (bits + 7) / 8;
  if (ptr + len > data_end) {
    return nullptr;
  }
  uint64_t bit_value;
  switch (len) {
    case 1:
      bit_value = (uint64_t)(*ptr);
      break;
    case 2:
      bit_value = (uint64_t)uint2korr(ptr);
      break;
    case 3:
      bit_value = (uint64_t)uint3korr(ptr);
      break;
    case 4:
      bit_value = (uint64_t)uint4korr(ptr);
      break;
    case 5:
      bit_value = (uint64_t)uint5korr(ptr);
      break;
    case 6:
      bit_value = (uint64_t)uint6korr(ptr);
      break;
    case 7:
      bit_value =
          (uint64_t)(uint4korr(ptr) + (((uint64_t)uint3korr(ptr + 4)) << 32));
      break;
    case 8:
      bit_value = (uint64_t)uint8korr(ptr);
      break;
    default:
      return nullptr;
  }
  value->set_uint64_value(bit_value);
  return ptr + len;
}

const void *FieldString::parse_enum_value(const void *data,
                                          const void *data_end, int32_t len,
                                          GenericValue *value) {
  const uint8_t *ptr = static_cast<const uint8_t *>(data);
  if (ptr + len > data_end) {
    return nullptr;
  }
  int32_t enum_value;
  switch (len) {
    case 1:
      enum_value = (int32_t)(*ptr);
      break;
    case 2:
      enum_value = (int32_t)uint2korr(ptr);
      break;
    default:
      return nullptr;
  }
  value->set_int32_value(enum_value);
  return ptr + len;
}

const void *FieldJson::unpack_binary(const void *data, const void *data_end,
                                     const ColumnInfo &info,
                                     GenericValue *value) {
  uint32_t length = 0;
  uint32_t meta = info.meta;
  const uint8_t *ptr = static_cast<const uint8_t *>(data);
  if (ptr + meta > data_end) {
    return nullptr;
  }
  switch (meta) {
    case 1:
      length = (uint32_t)(*ptr);
      break;
    case 2:
      length = (uint32_t)uint2korr(ptr);
      break;
    case 3:
      length = (uint32_t)uint3korr(ptr);
      break;
    case 4:
      length = (uint32_t)uint4korr(ptr);
      break;
    default:
      return nullptr;
  }
  ptr += meta;

  if (ptr + length > data_end) {
    return nullptr;
  }

  // TODO parse json
  value->set_bytes_value((const char *)ptr, length);

  return ptr + length;
}

bool FieldJson::unpack_text(const void *data, unsigned long length,
                            GenericValue *value) {
  if (!data || !length) {
    return false;
  }
  value->set_bytes_value(static_cast<const char *>(data), length);
  return true;
}

const void *FieldGeometry::unpack_binary(const void *data, const void *data_end,
                                         const ColumnInfo &info,
                                         GenericValue *value) {
  uint32_t length = 0;
  uint32_t meta = info.meta;
  const uint8_t *ptr = static_cast<const uint8_t *>(data);
  if (ptr + meta > data_end) {
    return nullptr;
  }
  switch (meta) {
    case 1:
      length = (uint32_t)(*ptr);
      break;
    case 2:
      length = (uint32_t)uint2korr(ptr);
      break;
    case 3:
      length = (uint32_t)uint3korr(ptr);
      break;
    case 4:
      length = (uint32_t)uint4korr(ptr);
      break;
    default:
      return nullptr;
  }
  ptr += meta;

  if (ptr + length > data_end) {
    return nullptr;
  }

  value->set_bytes_value(ptr, length);

  return ptr + length;
}

bool FieldGeometry::unpack_text(const void *data, unsigned long length,
                                GenericValue *value) {
  if (!data || !length) {
    return false;
  }
  value->set_bytes_value(data, length);
  return true;
}

}  // end namespace repository
}  // namespace be
}  // end namespace proxima
