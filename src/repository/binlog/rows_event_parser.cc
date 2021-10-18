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
 *   \brief    Rows event parser interface implementation for proxima search
 *             engine
 */

#include "rows_event_parser.h"
#include "repository/repository_common/error_code.h"
#include "repository_common/config.h"
#include "repository_common/logger.h"

namespace proxima {
namespace be {
namespace repository {

RowsEventParser::RowsEventParser(TableSchemaPtr schema)
    : schema_(std::move(schema)) {}

int RowsEventParser::parse(RowsEvent *event, WriteRequest::Row *row_data,
                           LsnContext *context) {
  if (!event) {
    LOG_ERROR("Rows event is nullptr.");
    return ErrorCode_RuntimeError;
  }

  if (schema_->fields().size() != event->column_count()) {
    LOG_ERROR("Bin log row column count mismatched. actual[%lu] expected[%zu]",
              event->column_count(), schema_->fields().size());
    return ErrorCode_InvalidRowData;
  }

  std::vector<GenericValue> values;
  const void *buf = parse_row_data(event->cur_buf(), event->present_columns(),
                                   *event, values);
  if (event->type() == UPDATE_ROWS_EVENT_V1 ||
      event->type() == UPDATE_ROWS_EVENT) {
    buf = parse_row_data(buf, event->present_columns_update(), *event, values);
  }
  if (!buf) {
    LOG_ERROR("Parse row data failed.");
    return ErrorCode_InvalidRowData;
  }
  int ret = fill_row_data(values, event->type(), row_data);
  if (ret != 0) {
    LOG_ERROR("Fill row data failed.");
    return ret;
  }

  // previous table map event position
  context->position = event->start_position();

  event->set_cur_buf(buf);

  return 0;
}

int RowsEventParser::fill_row_data(const std::vector<GenericValue> &values,
                                   EventType type,
                                   WriteRequest::Row *row_data) {
  uint64_t seq_id = get_auto_increment_id(values[schema_->auto_increment_id()]);
  if (seq_id == INVALID_PRIMARY_KEY) {
    LOG_ERROR("Get auto increment id failed.");
    return ErrorCode_RuntimeError;
  }
  row_data->set_primary_key(seq_id);

  switch (type) {
    case WRITE_ROWS_EVENT:
    case WRITE_ROWS_EVENT_V1: {
      fill_selected_columns(values, row_data);
      row_data->set_operation_type(proto::OP_INSERT);
      break;
    }
    case UPDATE_ROWS_EVENT:
    case UPDATE_ROWS_EVENT_V1: {
      fill_selected_columns(values, row_data);
      row_data->set_operation_type(proto::OP_UPDATE);
      break;
    }
    case DELETE_ROWS_EVENT:
    case DELETE_ROWS_EVENT_V1: {
      row_data->set_operation_type(proto::OP_DELETE);
      break;
    }
    default:
      LOG_ERROR("Unsupported event type %d.", (int32_t)type);
      return ErrorCode_RuntimeError;
  }
  return 0;
}

void RowsEventParser::fill_selected_columns(
    const std::vector<GenericValue> &values, WriteRequest::Row *row_data) {
  auto &index_ids = schema_->selected_index_ids();
  for (size_t i = 0; i < index_ids.size(); ++i) {
    auto *index_column = row_data->mutable_index_column_values()->add_values();
    *index_column = values[index_ids[i]];
  }

  auto &forward_ids = schema_->selected_forward_ids();
  for (size_t i = 0; i < forward_ids.size(); ++i) {
    auto *forward_column =
        row_data->mutable_forward_column_values()->add_values();
    *forward_column = values[forward_ids[i]];
  }
}

const void *RowsEventParser::parse_row_data(const void *buf,
                                            const uint8_t *present_columns,
                                            const RowsEvent &event,
                                            std::vector<GenericValue> &values) {
  if (!buf) {
    return nullptr;
  }
  unsigned long column_count = event.column_count();
  values.resize(column_count);
  const uint8_t *null_bits = static_cast<const uint8_t *>(buf);
  const void *row_data = null_bits + event.bits_length();
  TableMapEventPtr table_map = event.table_map();
  if (!table_map) {
    LOG_ERROR("Table map event is nullptr.");
    return nullptr;
  }
  const std::vector<FieldPtr> &fields = schema_->fields();
  for (unsigned long c = 0; c < column_count; ++c) {
    // Can ignore, row mode always present
    if (!(present_columns[c / 8] & (0x1 << (c % 8)))) {
      continue;
    }
    if (null_bits[c / 8] & (0x1 << (c % 8))) {
      continue;
    }
    row_data = fields[c]->unpack_binary(row_data, event.rows_end(),
                                        table_map->column_info(c), &values[c]);
    if (!row_data) {
      LOG_ERROR("Unpack column data failed. id[%lu] field_name[%s]", c,
                fields[c]->field_name().c_str());
      return nullptr;
    }
  }

  return row_data;
}

uint64_t RowsEventParser::get_auto_increment_id(const GenericValue &value) {
  switch (value.value_oneof_case()) {
    case GenericValue::kInt32Value:
      return (uint64_t)value.int32_value();
    case GenericValue::kInt64Value:
      return (uint64_t)value.int64_value();
    case GenericValue::kUint32Value:
      return (uint64_t)value.uint32_value();
    case GenericValue::kUint64Value:
      return (uint64_t)value.uint64_value();
    default:
      LOG_ERROR("Unsupported auto_increment data type %d.",
                value.value_oneof_case());
      return INVALID_PRIMARY_KEY;
  }
}

}  // end namespace repository
}  // namespace be
}  // end namespace proxima
