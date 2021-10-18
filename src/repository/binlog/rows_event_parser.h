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
 *   \brief    Rows event parser interface definition for proxima search engine
 */

#pragma once

#include "binlog_event.h"
#include "table_schema.h"

namespace proxima {
namespace be {
namespace repository {

class RowsEventParser;
using RowsEventParserPtr = std::shared_ptr<RowsEventParser>;

/*! Rows Event Parser
 */
class RowsEventParser {
 public:
  //! Constructor
  RowsEventParser(TableSchemaPtr schema);

  //! Destructor
  ~RowsEventParser() = default;

  //! Parse event
  int parse(RowsEvent *event, WriteRequest::Row *row_data, LsnContext *context);

  //! Update table schema
  void update_schema(TableSchemaPtr schema) {
    schema_ = std::move(schema);
  }

 private:
  //! Fill proto row data
  int fill_row_data(const std::vector<GenericValue> &values, EventType type,
                    WriteRequest::Row *row_data);

  //! Fill selected columns
  void fill_selected_columns(const std::vector<GenericValue> &values,
                             WriteRequest::Row *row_data);

  //! Parse row data
  const void *parse_row_data(const void *buf, const uint8_t *present_columns,
                             const RowsEvent &event,
                             std::vector<GenericValue> &values);

  //! Get sequence id
  uint64_t get_auto_increment_id(const GenericValue &value);

 private:
  //! Table Schema
  TableSchemaPtr schema_{};
};

}  // end namespace repository
}  // namespace be
}  // end namespace proxima
