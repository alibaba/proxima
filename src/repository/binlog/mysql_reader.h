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
 *   \brief    Mysql reader interface definition for proxima search engine
 */

#pragma once

#include "info_fetcher.h"
#include "table_schema.h"

namespace proxima {
namespace be {
namespace repository {

class MysqlReader;
using MysqlReaderPtr = std::shared_ptr<MysqlReader>;

#define READER_FORMAT " table[%s] "

#define RLOG_DEBUG(format, ...) \
  LOG_DEBUG(format READER_FORMAT, ##__VA_ARGS__, table_name_.c_str())

#define RLOG_INFO(format, ...) \
  LOG_INFO(format READER_FORMAT, ##__VA_ARGS__, table_name_.c_str())

#define RLOG_ERROR(format, ...) \
  LOG_ERROR(format READER_FORMAT, ##__VA_ARGS__, table_name_.c_str())

/*! Mysql Reader
 */
class MysqlReader {
 public:
  //! Destructor
  virtual ~MysqlReader() {}

  //! Init reader
  virtual int init() = 0;

  //! Start reader
  virtual int start(const LsnContext &context) = 0;

  //! Get row data from table
  virtual int get_next_row_data(WriteRequest::Row *row_data,
                                LsnContext *context) = 0;

  //! Get Table Schema
  virtual TableSchemaPtr get_table_schema() const = 0;

  //! Get Info Fetcher
  virtual InfoFetcherPtr get_info_fetcher() const = 0;
};

}  // end namespace repository
}  // namespace be
}  // end namespace proxima
