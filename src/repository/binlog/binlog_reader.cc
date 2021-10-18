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
 *   \brief    Binlog reader interface implementation for proxima search
 *             engine
 */

#include "binlog_reader.h"
#include <memory>
#include <ailego/utility/string_helper.h>
#include "repository/repository_common/error_code.h"
#include "repository_common/logger.h"
#include "binlog_common.h"

namespace proxima {
namespace be {
namespace repository {

BinlogReader::BinlogReader(const std::string &table_name,
                           InfoFetcherPtr info_fetcher,
                           MysqlConnectorManagerPtr mgr)
    : table_name_(table_name),
      info_fetcher_(std::move(info_fetcher)),
      connector_mgr_(std::move(mgr)) {}

int BinlogReader::init() {
  RLOG_INFO("Begin init Binlog reader.");

  // get table schema
  int ret = info_fetcher_->get_table_schema(table_name_, &table_schema_);
  if (ret != 0) {
    RLOG_ERROR("Info fetcher get table schema failed.");
    return ret;
  }

  // create parser
  parser_ = std::make_shared<RowsEventParser>(table_schema_);

  // create event fetcher
  event_fetcher_ = std::make_shared<EventFetcher>(connector_mgr_);

  suspended_ = false;

  RLOG_INFO("Binlog reader init success.");
  return 0;
}

int BinlogReader::start(const LsnContext &context) {
  RLOG_INFO("Begin start binlog reader. file[%s] position[%zu]",
            context.file_name.c_str(), (size_t)context.position);
  int ret = event_fetcher_->init(context.file_name, context.position);
  if (ret != 0) {
    RLOG_ERROR("Init event fetcher failed.");
    return ret;
  }

  log_context_.update_lsn(context.file_name, context.position);

  RLOG_INFO("End start binlog reader.");

  return 0;
}

int BinlogReader::get_next_row_data(WriteRequest::Row *row_data,
                                    LsnContext *context) {
  if (suspended_) {
    return ErrorCode_Suspended;
  }
  stop_fetch_ = false;
  context->status = RowDataStatus::NORMAL;

  int ret = process_remain_events(row_data, context);
  if (ret != 0) {
    RLOG_ERROR("Process remain events failed.");
    return ret;
  }

  while (!stop_fetch_) {
    // process alter table schema
    if (schema_changed_) {
      context->status = RowDataStatus::SCHEMA_CHANGED;
      schema_changed_ = false;
      break;
    }

    BasicEventPtr event;
    ret = event_fetcher_->fetch(&event);
    if (ret == 0) {
      if (!event->is_valid()) {
        continue;
      }
      switch (event->type()) {
        case QUERY_EVENT:
          ret = process_query_event(event);
          break;
        case TABLE_MAP_EVENT:
          process_table_map_event(event);
          break;
        case WRITE_ROWS_EVENT:
        case UPDATE_ROWS_EVENT:
        case DELETE_ROWS_EVENT:
        case WRITE_ROWS_EVENT_V1:
        case UPDATE_ROWS_EVENT_V1:
        case DELETE_ROWS_EVENT_V1:
          ret = process_rows_event(event, row_data, context);
          break;
        case ROTATE_EVENT:
          process_rotate_event(event);
        default:
          break;
      }
    } else if (ret == ErrorCode_BinlogNoMoreData) {
      context->status = RowDataStatus::NO_MORE_DATA;
      break;
    }
    if (ret != 0) {
      RLOG_ERROR("Fetch next event failed.");
      return ret;
    }
  }
  context->file_name = log_context_.file_name();

  // Debug
  if (context->status != RowDataStatus::NO_MORE_DATA) {
    RLOG_DEBUG("primary_key[%zu] status[%d]", (size_t)row_data->primary_key(),
               context->status);
  }

  return 0;
}

int BinlogReader::process_query_event(BasicEventPtr event) {
  QueryEventPtr query_event = std::dynamic_pointer_cast<QueryEvent>(event);
  if (!query_event) {
    RLOG_ERROR("Dynamic cast to query event failed.");
    return ErrorCode_LogicError;
  }

  if (!is_alter_table_statement(query_event->query())) {
    return 0;
  }

  // TODO need parse the alter statement next version
  int ret = info_fetcher_->get_table_schema(table_name_, &table_schema_);
  if (ret == ErrorCode_InvalidCollectionConfig) {
    suspended_ = true;
    RLOG_ERROR(
        "Table schema and collection config mismatched, "
        "reader enter suspended status.");
    return ret;
  } else if (ret != 0) {
    query_event_ = std::move(query_event);
    RLOG_ERROR("Info fetcher get table schema failed.");
    return ret;
  }

  parser_->update_schema(table_schema_);
  schema_changed_ = true;

  return 0;
}

void BinlogReader::process_rotate_event(BasicEventPtr event) {
  RotateEventPtr rotate_event = std::dynamic_pointer_cast<RotateEvent>(event);
  if (!rotate_event) {
    RLOG_ERROR("Dynamic pointer cast to RotateEvent failed.");
    return;
  }

  log_context_.update_lsn(rotate_event->next_binlog_name(),
                          rotate_event->position());

  RLOG_DEBUG("Rotate event info. file[%s] position[%zu]",
             rotate_event->next_binlog_name().c_str(),
             (size_t)rotate_event->position());
}

void BinlogReader::process_table_map_event(BasicEventPtr event) {
  TableMapEventPtr map_event = std::dynamic_pointer_cast<TableMapEvent>(event);
  if (!map_event) {
    RLOG_ERROR("Dynamic pointer cast to TableMapEvent failed.");
    skipped_event_ = true;
    return;
  }

  // Is current table's table_map
  bool is_current_table =
      (ailego::StringHelper::CompareIgnoreCase(map_event->database_name(),
                                               info_fetcher_->database())) &&
      (ailego::StringHelper::CompareIgnoreCase(map_event->table_name(),
                                               table_name_));
  if (is_current_table) {
    log_context_.update_table_map(map_event);
    skipped_event_ = false;
  } else {
    skipped_event_ = true;
  }
}

int BinlogReader::process_rows_event(BasicEventPtr event,
                                     WriteRequest::Row *row_data,
                                     LsnContext *context) {
  if (skipped_event_) {
    return 0;
  }
  rows_event_ = std::dynamic_pointer_cast<RowsEvent>(event);
  if (!rows_event_) {
    RLOG_ERROR("Dynamic pointer cast to RowsEvent failed.");
    return ErrorCode_RuntimeError;
  }

  rows_event_->fill_table_map(log_context_);
  int ret = parser_->parse(rows_event_.get(), row_data, context);
  if (ret != 0) {
    rows_event_.reset();
    return ret;
  }

  if (rows_event_->is_finished()) {
    rows_event_.reset();
  }
  stop_fetch_ = true;

  return 0;
}

int BinlogReader::process_remain_rows(WriteRequest::Row *row_data,
                                      LsnContext *context) {
  if (rows_event_ && rows_event_->is_finished()) {
    return 0;
  }

  int ret = parser_->parse(rows_event_.get(), row_data, context);
  if (ret != 0) {
    rows_event_.reset();
    RLOG_ERROR("Parse rows event failed.");
    return ret;
  }
  if (rows_event_->is_finished()) {
    rows_event_.reset();
  }
  stop_fetch_ = true;

  return 0;
}

int BinlogReader::process_remain_query_event() {
  int ret = info_fetcher_->get_table_schema(table_name_, &table_schema_);
  if (ret != 0) {
    RLOG_ERROR("Info fetcher get table schema failed.");
    return ret;
  }

  parser_->update_schema(table_schema_);
  schema_changed_ = true;
  query_event_.reset();

  return 0;
}

int BinlogReader::process_remain_events(WriteRequest::Row *row_data,
                                        LsnContext *context) {
  // process multiply rows in a rows event
  if (rows_event_) {
    int ret = process_remain_rows(row_data, context);
    if (ret != 0) {
      RLOG_ERROR("Process remain rows event failed.");
    }
    stop_fetch_ = true;
    return ret;
  }

  // repeated process query event
  if (query_event_) {
    int ret = process_remain_query_event();
    if (ret != 0) {
      stop_fetch_ = true;
      RLOG_ERROR("Process remain query event failed.");
    }
    return ret;
  }

  return 0;
}

bool BinlogReader::is_alter_table_statement(const std::string &query) {
  enum State { SP0, A0, L0, T0, E0, R, SP1, T1, A1, B, L1, E1 };
  State state = SP0;
  for (auto i = query.begin(); i != query.end(); ++i) {
    if (state == SP0 && (*i == ' ' || *i == '\t' || *i == '\r' || *i == '\n')) {
    } else if (state == SP0 && (*i == 'a' || *i == 'A')) {
      state = A0;
    } else if (state == A0 && (*i == 'l' || *i == 'L')) {
      state = L0;
    } else if (state == L0 && (*i == 't' || *i == 'T')) {
      state = T0;
    } else if (state == T0 && (*i == 'e' || *i == 'E')) {
      state = E0;
    } else if (state == E0 && (*i == 'r' || *i == 'R')) {
      state = R;
    } else if (state == R &&
               (*i == ' ' || *i == '\t' || *i == '\r' || *i == '\n')) {
      state = SP1;
    } else if (state == SP1 &&
               (*i == ' ' || *i == '\t' || *i == '\r' || *i == '\n')) {
    } else if (state == SP1 && (*i == 't' || *i == 'T')) {
      state = T1;
    } else if (state == T1 && (*i == 'a' || *i == 'A')) {
      state = A1;
    } else if (state == A1 && (*i == 'b' || *i == 'B')) {
      state = B;
    } else if (state == B && (*i == 'l' || *i == 'L')) {
      state = L1;
    } else if (state == L1 && (*i == 'e' || *i == 'E')) {
      state = E1;
    } else if (state == E1) {
      return true;
    } else {
      return false;
    }
  }
  return false;
}

}  // end namespace repository
}  // namespace be
}  // end namespace proxima
