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
 *   \brief    Mysql handler interface implementation for proxima search engine
 */

#include "mysql_handler.h"
#include "repository/repository_common/error_code.h"
#include "repository_common/logger.h"
#include "binlog_reader.h"
#include "mysql_validator.h"
#include "table_reader.h"

namespace proxima {
namespace be {
namespace repository {

#define MYSQL_FORMAT " collection[%s] table[%s] "

#define MLOG_INFO(format, ...)                                           \
  LOG_INFO(format MYSQL_FORMAT, ##__VA_ARGS__, collection_name_.c_str(), \
           table_name_.c_str())

#define MLOG_ERROR(format, ...)                                           \
  LOG_ERROR(format MYSQL_FORMAT, ##__VA_ARGS__, collection_name_.c_str(), \
            table_name_.c_str())

MysqlHandler::MysqlHandler(const CollectionConfig &config)
    : table_name_(config.repository_config().database().table_name()),
      collection_name_(config.collection_name()),
      collection_config_(config) {}

MysqlHandler::MysqlHandler(const CollectionConfig &config,
                           MysqlConnectorManagerPtr mgr)
    : table_name_(config.repository_config().database().table_name()),
      collection_name_(config.collection_name()),
      collection_config_(config),
      connector_mgr_(std::move(mgr)) {}

MysqlHandler::~MysqlHandler() {}

int MysqlHandler::init(ScanMode mode) {
  if (inited_) {
    return ErrorCode_RepeatedInitialized;
  }

  MLOG_INFO("Begin init mysql handler.");

  int ret = 0;
  if (!connector_mgr_) {
    connector_mgr_ = std::make_shared<MysqlConnectorManager>();
    auto &database = collection_config_.repository_config().database();
    ret = connector_mgr_->init(database.connection_uri(), database.user(),
                               database.password());
    if (ret != 0) {
      MLOG_ERROR("Init connector manager failed.");
      return ret;
    }
  }

  ret = validate_mysql();
  if (ret != 0) {
    return ret;
  }

  InfoFetcherPtr info_fetcher =
      std::make_shared<InfoFetcher>(collection_config_, connector_mgr_);
  ret = info_fetcher->init();
  if (ret != 0) {
    MLOG_ERROR("Init info fetcher failed.");
    return ret;
  }

  if (mode == ScanMode::FULL) {
    mysql_reader_ = std::make_shared<TableReader>(table_name_, info_fetcher,
                                                  connector_mgr_);

  } else {
    mysql_reader_ = std::make_shared<BinlogReader>(table_name_, info_fetcher,
                                                   connector_mgr_);
  }
  ret = mysql_reader_->init();
  if (ret != 0) {
    MLOG_ERROR("Init mysql reader failed.");
    return ret;
  }

  inited_ = true;

  MLOG_INFO("End init mysql handler.");
  return 0;
}

int MysqlHandler::start(const LsnContext &context) {
  if (!inited_) {
    return ErrorCode_NoInitialized;
  }

  int ret = mysql_reader_->start(context);
  if (ret != 0) {
    MLOG_ERROR("Start mysql reader failed.");
    return ret;
  }

  return 0;
}

int MysqlHandler::get_next_row_data(WriteRequest::Row *row_data,
                                    LsnContext *context) {
  if (!inited_) {
    return ErrorCode_NoInitialized;
  }

  return mysql_reader_->get_next_row_data(row_data, context);
}

int MysqlHandler::reset_status(ScanMode mode, const CollectionConfig &config,
                               const LsnContext &context) {
  if (!inited_) {
    return ErrorCode_NoInitialized;
  }

  MLOG_INFO("Begin reset mysql handler.");

  mysql_reader_.reset();
  collection_config_ = config;
  InfoFetcherPtr info_fetcher =
      std::make_shared<InfoFetcher>(config, connector_mgr_);
  int ret = info_fetcher->init();
  if (ret != 0) {
    MLOG_ERROR("Init info fetcher failed.");
    return ret;
  }
  if (mode == ScanMode::FULL) {
    mysql_reader_ = std::make_shared<TableReader>(table_name_, info_fetcher,
                                                  connector_mgr_);
  } else {
    mysql_reader_ = std::make_shared<BinlogReader>(table_name_, info_fetcher,
                                                   connector_mgr_);
  }
  ret = mysql_reader_->init();
  if (ret != 0) {
    MLOG_ERROR("Init mysql reader failed.");
    return ret;
  }

  ret = mysql_reader_->start(context);
  if (ret != 0) {
    MLOG_ERROR("Start mysql reader failed.");
    return ret;
  }

  MLOG_INFO("End reset mysql handler.");
  return 0;
}

int MysqlHandler::get_fields_meta(WriteRequest::RowMeta *meta) {
  TableSchemaPtr schema = mysql_reader_->get_table_schema();
  if (!schema) {
    MLOG_ERROR("Get table schema failed.");
    return ErrorCode_RuntimeError;
  }
  schema->fill_fields_meta(meta);

  auto &index_column_params = collection_config_.index_column_params();
  int index_column_size = meta->index_column_metas_size();
  for (int i = 0; i < index_column_size; ++i) {
    auto *column_meta = meta->mutable_index_column_metas(i);
    int j = 0;
    for (; j < index_column_params.size(); ++j) {
      if (column_meta->column_name() == index_column_params[j].column_name()) {
        column_meta->set_data_type(index_column_params[i].data_type());
        column_meta->set_dimension(index_column_params[i].dimension());
        break;
      }
    }
    if (j == index_column_params.size()) {
      LOG_ERROR("Index column not found in collection config. column[%s]",
                column_meta->column_name().c_str());
      return ErrorCode_RuntimeError;
    }
  }

  return 0;
}

int MysqlHandler::validate_mysql() {
  MysqlValidator validator(connector_mgr_);
  int ret = validator.init();
  if (ret != 0) {
    MLOG_ERROR("Init mysql validator failed.");
    return ret;
  }

  if (!validator.validate_version()) {
    MLOG_ERROR("Validate mysql version failed.");
    return ErrorCode_UnsupportedMysqlVersion;
  }

  if (!validator.validate_binlog_format()) {
    MLOG_ERROR("Validate binlog format failed.");
    return ErrorCode_UnsupportedBinlogFormat;
  }

  if (!validator.validate_database_exist()) {
    MLOG_ERROR("Validate database failed.");
    return ErrorCode_InvalidCollectionConfig;
  }

  return 0;
}

int MysqlHandler::get_table_snapshot(std::string *binlog_file,
                                     uint64_t *position) {
  if (!inited_) {
    return ErrorCode_NoInitialized;
  }

  InfoFetcherPtr fetcher = mysql_reader_->get_info_fetcher();
  if (!fetcher) {
    MLOG_ERROR("Get info fetcher failed.");
    return ErrorCode_RuntimeError;
  }

  return fetcher->get_table_snapshot(table_name_, binlog_file, position);
}

}  // end namespace repository
}  // namespace be
}  // end namespace proxima
