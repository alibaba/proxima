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
 *   \brief    Table reader interface definition for proxima search engine
 */

#pragma once

#include "mysql_connector.h"
#include "mysql_reader.h"

namespace proxima {
namespace be {
namespace repository {

/*! Table Handler
 */
class TableReader : public MysqlReader, MysqlConnectorProxy {
 public:
  //! Constructor
  TableReader(const std::string &table_name, InfoFetcherPtr info_fetcher,
              MysqlConnectorManagerPtr mgr);

  //! Destructor
  ~TableReader() = default;

  //! Init reader
  int init() override;

  // Start reader
  int start(const LsnContext &context) override;

  //! Get row data from table
  int get_next_row_data(WriteRequest::Row *row_data,
                        LsnContext *context) override;

  //! Get Table Schema
  TableSchemaPtr get_table_schema() const override {
    return table_schema_;
  }

  //! Get Info Fetcher
  InfoFetcherPtr get_info_fetcher() const override {
    return info_fetcher_;
  }

 private:
  //! Prepare table reader
  int prepare_reader(uint64_t seq_id);

  //! Build row data
  int build_row_data(MysqlRow *row, WriteRequest::Row *row_data,
                     LsnContext *context);

  //! Get sequence id
  uint64_t get_auto_increment_id(const GenericValue &value);

 private:
  //! Table name
  std::string table_name_{};
  //! Current seq id
  uint64_t sequence_id_{0};
  //! Connector need reconnect
  bool need_reconnect_{false};
  //! Table schema
  TableSchemaPtr table_schema_{};
  //! Info fetcher
  InfoFetcherPtr info_fetcher_{};
  //! Mysql result
  MysqlResultWrapperPtr result_wrappeer_{};
  //! Selected fields
  SelectedFieldsPtr selected_fields_{};
};

}  // end namespace repository
}  // namespace be
}  // end namespace proxima
