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
 *   \brief    Field interface definition for proxima search engine
 */

#pragma once

#include <sstream>
#include <string>
#include <m_ctype.h>
#include "binlog_common.h"
#include "binlog_event.h"
#include "mysql_connector.h"

namespace proxima {
namespace be {
namespace repository {

class Field;
using FieldPtr = std::shared_ptr<Field>;

/*! Field Attribute
 */
class FieldAttr {
 public:
  //! Constructor
  FieldAttr(bool index, bool forward, const std::string &collat,
            FieldMetaPtr field_meta)
      : is_index_(index),
        is_forward_(forward),
        is_selected_(index || forward),
        collation_(collat),
        meta_(std::move(field_meta)) {}

  //! Destructor
  ~FieldAttr() = default;

  //! Is index field
  bool is_index() const {
    return is_index_;
  }

  //! Is forward field
  bool is_forward() const {
    return is_forward_;
  }

  //! Is selected field
  bool is_selected() const {
    return is_selected_;
  }

  //! Get field meta
  const FieldMetaPtr &meta() const {
    return meta_;
  }

  //! Get collation
  const std::string &collation() const {
    return collation_;
  }

 private:
  //! is index field
  bool is_index_;
  //! is forward field
  bool is_forward_;
  //! is selected field
  bool is_selected_;
  // collation
  std::string collation_;
  //! Field Meta
  FieldMetaPtr meta_;
};

/*! Field Factory
 */
class FieldFactory {
 public:
  //! Create Field
  static FieldPtr Create(const std::string &field_name, const FieldAttr &attr);
};

/*! Field
 */
class Field {
 public:
  //! Constructor
  Field(const std::string &name, const FieldAttr &attr);

  //! Destructor
  virtual ~Field() = default;

  //! Unpack binary
  virtual const void *unpack_binary(const void *data, const void *data_end,
                                    const ColumnInfo &info,
                                    GenericValue *value) = 0;

  //! Unpack text
  virtual bool unpack_text(const void *data, unsigned long length,
                           GenericValue *value) = 0;

  //! Get field name
  const std::string &field_name() const {
    return field_name_;
  }

  //! Get select field name
  const std::string &select_field() const {
    return select_field_;
  }

  //! Get field type
  enum_field_types field_type() const {
    return field_type_;
  }

  //! Get dst field type
  FieldType dst_field_type() const {
    return dst_field_type_;
  }

  //! Get field decimals
  uint32_t field_decimals() const {
    return field_decimals_;
  }

  //! Is index field
  bool is_index() const {
    return is_index_;
  }

  //! Is forward field
  bool is_forward() const {
    return is_forward_;
  }

  //! Is selected field
  bool is_selected() const {
    return is_selected_;
  }

  //! Is auto increment field
  bool is_auto_increment() const {
    return flags_ & AUTO_INCREMENT_FLAG;
  }

  //! Is unsigned type
  bool is_unsigned() const {
    return flags_ & UNSIGNED_FLAG;
  }

 private:
  //! Convert field type
  FieldType convert_field_type(enum_field_types types);

 protected:
  //! Field name
  std::string field_name_{};
  //! Select field name for full scan
  std::string select_field_{};
  //! Field collation
  std::string collation_{};
  //! Field type
  enum_field_types field_type_{};
  //! Dst field type
  FieldType dst_field_type_{};
  //! Field length
  uint32_t field_length_{0};
  //! Field decimals
  uint32_t field_decimals_{0};
  //! Field flags
  uint32_t flags_{0};
  //! Is index field
  bool is_index_{false};
  //! Is forward field
  bool is_forward_{false};
  //! Is selected field
  bool is_selected_{false};
  //! Field src collation
  CHARSET_INFO *src_cs_{nullptr};
  //! Field dst collation
  CHARSET_INFO *dst_cs_{nullptr};

  //! UTF8 charset name
  const static std::string UTF8_CHARSET_NAME;
};

/*! Field Tiny
 */
class FieldInteger : public Field {
 public:
  //! Constructor
  FieldInteger(const std::string &name, const FieldAttr &attr)
      : Field(name, attr) {}

  //! Destructor
  virtual ~FieldInteger() = default;

  //! Unpack binary
  const void *unpack_binary(const void *data, const void *data_end,
                            const ColumnInfo &info,
                            GenericValue *value) override = 0;

  //! Unpack text
  bool unpack_text(const void *data, unsigned long length,
                   GenericValue *value) override;
};

/*! Field Tiny
 */
class FieldTiny : public FieldInteger {
 public:
  //! Constructor
  FieldTiny(const std::string &name, const FieldAttr &attr)
      : FieldInteger(name, attr) {}

  //! Destructor
  virtual ~FieldTiny() = default;

  //! Unpack binary
  const void *unpack_binary(const void *data, const void *data_end,
                            const ColumnInfo &info,
                            GenericValue *value) override;
};

/*! Field Short
 */
class FieldShort : public FieldInteger {
 public:
  //! Constructor
  FieldShort(const std::string &name, const FieldAttr &attr)
      : FieldInteger(name, attr) {}

  //! Unpack binary
  const void *unpack_binary(const void *data, const void *data_end,
                            const ColumnInfo &info,
                            GenericValue *value) override;
};

/*! Field Int24
 */
class FieldInt24 : public FieldInteger {
 public:
  //! Constructor
  FieldInt24(const std::string &name, const FieldAttr &attr)
      : FieldInteger(name, attr) {}

  //! Destructor
  virtual ~FieldInt24() = default;

  //! Unpack binary
  const void *unpack_binary(const void *data, const void *data_end,
                            const ColumnInfo &info,
                            GenericValue *value) override;
};

/*! Field Long
 */
class FieldLong : public FieldInteger {
 public:
  //! Constructor
  FieldLong(const std::string &name, const FieldAttr &attr)
      : FieldInteger(name, attr) {}

  //! Destructor
  virtual ~FieldLong() = default;

  //! Unpack binary
  const void *unpack_binary(const void *data, const void *data_end,
                            const ColumnInfo &info,
                            GenericValue *value) override;
};

/*! Field Long Long
 */
class FieldLongLong : public FieldInteger {
 public:
  //! Constructor
  FieldLongLong(const std::string &name, const FieldAttr &attr)
      : FieldInteger(name, attr) {}

  //! Destructor
  virtual ~FieldLongLong() = default;

  //! Unpack binary
  const void *unpack_binary(const void *data, const void *data_end,
                            const ColumnInfo &info,
                            GenericValue *value) override;

  //! Unpack text
  bool unpack_text(const void *data, unsigned long length,
                   GenericValue *value) override;
};

/*! Field Float
 */
class FieldFloat : public Field {
 public:
  //! Constructor
  FieldFloat(const std::string &name, const FieldAttr &attr)
      : Field(name, attr) {}

  //! Destructor
  virtual ~FieldFloat() = default;

  //! Unpack binary
  const void *unpack_binary(const void *data, const void *data_end,
                            const ColumnInfo &info,
                            GenericValue *value) override;

  //! Unpack text
  bool unpack_text(const void *data, unsigned long length,
                   GenericValue *value) override;
};

/*! Field Double
 */
class FieldDouble : public Field {
 public:
  //! Constructor
  FieldDouble(const std::string &name, const FieldAttr &attr)
      : Field(name, attr) {}

  //! Destructor
  virtual ~FieldDouble() = default;

  //! Unpack binary
  const void *unpack_binary(const void *data, const void *data_end,
                            const ColumnInfo &info,
                            GenericValue *value) override;

  //! Unpack text
  bool unpack_text(const void *data, unsigned long length,
                   GenericValue *value) override;
};

/*! Field Decimal
 */
class FieldDecimal : public Field {
 public:
  //! Constructor
  FieldDecimal(const std::string &name, const FieldAttr &attr)
      : Field(name, attr) {}

  //! Destructor
  virtual ~FieldDecimal() = default;

  //! Unpack binary
  const void *unpack_binary(const void *data, const void *data_end,
                            const ColumnInfo &info,
                            GenericValue *value) override;

  //! Unpack text
  bool unpack_text(const void *data, unsigned long length,
                   GenericValue *value) override;
};

/*! Field Bit
 */
class FieldBit : public Field {
 public:
  //! Constructor
  FieldBit(const std::string &name, const FieldAttr &attr) : Field(name, attr) {
    select_field_ = name + "+0";
  }

  //! Destructor
  virtual ~FieldBit() = default;

  //! Unpack binary
  const void *unpack_binary(const void *data, const void *data_end,
                            const ColumnInfo &info,
                            GenericValue *value) override;

  //! Unpack text
  bool unpack_text(const void *data, unsigned long length,
                   GenericValue *value) override;
};

/*! Field Datetime
 */
class FieldDatetime : public Field {
 public:
  //! Constructor
  FieldDatetime(const std::string &name, const FieldAttr &attr)
      : Field(name, attr) {}

  //! Destructor
  virtual ~FieldDatetime() = default;

  //! Unpack binary
  const void *unpack_binary(const void *data, const void *data_end,
                            const ColumnInfo &info,
                            GenericValue *value) override;

  //! Unpack text
  bool unpack_text(const void *data, unsigned long length,
                   GenericValue *value) override;

 private:
  const static uint64_t DATETIMEF_INT_OFS = 0x8000000000;
};

/*! Field Timestamp
 */
class FieldTimestamp : public Field {
 public:
  //! Constructor
  FieldTimestamp(const std::string &name, const FieldAttr &attr)
      : Field(name, attr) {}

  //! Destructor
  virtual ~FieldTimestamp() = default;

  //! Unpack binary
  const void *unpack_binary(const void *data, const void *data_end,
                            const ColumnInfo &info,
                            GenericValue *value) override;

  //! Unpack text
  bool unpack_text(const void *data, unsigned long length,
                   GenericValue *value) override;
};

/*! Field Time
 */
class FieldTime : public Field {
 public:
  //! Constructor
  FieldTime(const std::string &name, const FieldAttr &attr)
      : Field(name, attr) {}

  //! Destructor
  virtual ~FieldTime() = default;

  //! Unpack binary
  const void *unpack_binary(const void *data, const void *data_end,
                            const ColumnInfo &info,
                            GenericValue *value) override;

  //! Unpack text
  bool unpack_text(const void *data, unsigned long length,
                   GenericValue *value) override;

 private:
  const static int64_t TIME_INT_OFS = 0x800000;
  const static int64_t TIME_OFS = 0x800000000000;
};

/*! Field Date
 */
class FieldDate : public Field {
 public:
  //! Constructor
  FieldDate(const std::string &name, const FieldAttr &attr)
      : Field(name, attr) {}

  //! Destructor
  virtual ~FieldDate() = default;

  //! Unpack binary
  const void *unpack_binary(const void *data, const void *data_end,
                            const ColumnInfo &info,
                            GenericValue *value) override;

  //! Unpack text
  bool unpack_text(const void *data, unsigned long length,
                   GenericValue *value) override;
};

/*! Field Year
 */
class FieldYear : public Field {
 public:
  //! Constructor
  FieldYear(const std::string &name, const FieldAttr &attr)
      : Field(name, attr) {}

  //! Destructor
  virtual ~FieldYear() = default;

  //! Unpack binary
  const void *unpack_binary(const void *data, const void *data_end,
                            const ColumnInfo &info,
                            GenericValue *value) override;

  //! Unpack text
  bool unpack_text(const void *data, unsigned long length,
                   GenericValue *value) override;
};

/*! Field Blob
 */
class FieldBlob : public Field {
 public:
  //! Constructor
  FieldBlob(const std::string &name, const FieldAttr &attr);

  //! Destructor
  virtual ~FieldBlob() = default;

  //! Unpack binary
  const void *unpack_binary(const void *data, const void *data_end,
                            const ColumnInfo &info,
                            GenericValue *value) override;

  //! Unpack text
  bool unpack_text(const void *data, unsigned long length,
                   GenericValue *value) override;

 private:
  //! Is binary
  bool is_binary_{false};
  //! Need convert charset
  bool need_convert_{false};
};

/*! Field Var String
 */
class FieldVarString : public Field {
 public:
  //! Constructor
  FieldVarString(const std::string &name, const FieldAttr &attr);

  //! Destructor
  virtual ~FieldVarString() = default;

  //! Unpack binary
  const void *unpack_binary(const void *data, const void *data_end,
                            const ColumnInfo &info,
                            GenericValue *value) override;

  //! Unpack text
  bool unpack_text(const void *data, unsigned long length,
                   GenericValue *value) override;

 private:
  //! Need convert charset
  bool need_convert_{false};
  //! Is binary
  bool is_binary_{false};
  //! Length bytes
  uint32_t length_bytes_{0};
};

/*! Field String
 */
class FieldString : public Field {
 public:
  //! Constructor
  FieldString(const std::string &name, const FieldAttr &attr);

  //! Destructor
  virtual ~FieldString() = default;

  //! Unpack binary
  const void *unpack_binary(const void *data, const void *data_end,
                            const ColumnInfo &info,
                            GenericValue *value) override;

  //! Unpack text
  bool unpack_text(const void *data, unsigned long length,
                   GenericValue *value) override;

 private:
  //! Parse string type value
  const void *parse_string_value(const void *data, const void *data_end,
                                 int32_t len, GenericValue *value);
  //! Parse set type value
  const void *parse_set_value(const void *data, const void *data_end,
                              int32_t meta, GenericValue *value);
  //! Parse enum type value
  const void *parse_enum_value(const void *data, const void *data_end,
                               int32_t len, GenericValue *value);

 private:
  // Need convert charset
  bool need_convert_{false};
  //! Is binary
  bool is_binary_{false};
  //! Is enum
  bool is_enum_{false};
  //! Is set
  bool is_set_{false};
};

/*! Field Json
 */
class FieldJson : public Field {
 public:
  //! Constructor
  FieldJson(const std::string &name, const FieldAttr &attr)
      : Field(name, attr) {}

  //! Destructor
  virtual ~FieldJson() = default;

  //! Unpack binary
  const void *unpack_binary(const void *data, const void *data_end,
                            const ColumnInfo &info,
                            GenericValue *value) override;

  //! Unpack text
  bool unpack_text(const void *data, unsigned long length,
                   GenericValue *value) override;
};

/*! Field Geometry
 */
class FieldGeometry : public Field {
 public:
  //! Constructor
  FieldGeometry(const std::string &name, const FieldAttr &attr)
      : Field(name, attr) {}

  //! Destructor
  virtual ~FieldGeometry() = default;

  //! Unpack binary
  const void *unpack_binary(const void *data, const void *data_end,
                            const ColumnInfo &info,
                            GenericValue *value) override;

  //! Unpack text
  bool unpack_text(const void *data, unsigned long length,
                   GenericValue *value) override;
};

}  // end namespace repository
}  // namespace be
}  // end namespace proxima
