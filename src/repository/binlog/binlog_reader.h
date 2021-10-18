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
 *   \brief    Binlog reader interface definition for proxima search engine
 */

#pragma once

#include "event_fetcher.h"
#include "log_context.h"
#include "mysql_reader.h"
#include "rows_event_parser.h"

namespace proxima {
namespace be {
namespace repository {

/*! Bin Log Handler
 */
class BinlogReader : public MysqlReader {
 public:
  //! Constructor
  BinlogReader(const std::string &table_name, InfoFetcherPtr info_fetcher,
               MysqlConnectorManagerPtr mgr);

  //! Destructor
  ~BinlogReader() = default;

  //! Init Binlog reader
  int init() override;

  // Start Binlog reader
  int start(const LsnContext &context) override;

  //! Get row data from binlog
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
  //! Process query event
  int process_query_event(BasicEventPtr event);

  //! Process rotate event
  void process_rotate_event(BasicEventPtr event);

  //! Process table map event
  void process_table_map_event(BasicEventPtr event);

  //! Process rows event
  int process_rows_event(BasicEventPtr event, WriteRequest::Row *row_data,
                         LsnContext *context);

  //! Process remain rows event
  int process_remain_rows(WriteRequest::Row *row_data, LsnContext *context);

  //! Process remain query event
  int process_remain_query_event();

  //! Process remain events
  int process_remain_events(WriteRequest::Row *row_data, LsnContext *context);

  //! Judge query is alter table statement
  bool is_alter_table_statement(const std::string &query);

 private:
  //! Table name
  std::string table_name_{};
  //! Skipped event
  bool skipped_event_{false};
  //! Stop fetch next event
  bool stop_fetch_{false};
  //! Current schema changed
  bool schema_changed_{false};
  //! Binlog reader suspended
  bool suspended_{false};
  //! Table schema
  TableSchemaPtr table_schema_{};
  //! Info fetcher
  InfoFetcherPtr info_fetcher_{};
  //! Mysql connector manager
  MysqlConnectorManagerPtr connector_mgr_{};
  //! Event fetcher
  EventFetcherPtr event_fetcher_{};
  //! Rows Event
  RowsEventPtr rows_event_{};
  //! Query Event
  QueryEventPtr query_event_{};
  //! Rows event parser
  RowsEventParserPtr parser_{};
  //! Log context
  LogContext log_context_{};
};

}  // end namespace repository
}  // namespace be
}  // end namespace proxima
