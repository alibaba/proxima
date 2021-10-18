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
 *   \brief    Implementation of Config
 */

#include "config.h"
#include <thread>
#include <ailego/io/file.h>
#include <ailego/utility/string_helper.h>
#include <google/protobuf/text_format.h>
#include "common/logger.h"
#include "error_code.h"

namespace proxima {
namespace be {

int Config::load_config(const std::string &file_name) {
  ailego::File file;
  if (!file.open(file_name.c_str(), true, false)) {
    LOG_ERROR("Open file %s failed, maybe file not exist.", file_name.c_str());
    return ErrorCode_LoadConfig;
  }

  std::string file_content;
  file_content.resize(file.size());
  size_t read_size = file.read(&(file_content[0]), file.size());
  if (read_size != file.size()) {
    LOG_ERROR("File %s read error, expected size: %zu, actual size: %zu.",
              file_name.c_str(), file.size(), read_size);
    return ErrorCode_LoadConfig;
  }

  bool ret =
      google::protobuf::TextFormat::ParseFromString(file_content, &config_);
  if (!ret) {
    LOG_ERROR("Parse file %s content %s failed.", file_name.c_str(),
              file_content.c_str());

    return ErrorCode_LoadConfig;
  }

  config_file_ = file_name;

  LOG_INFO(
      "Load config complete. protocol[%s] grpc_listen_port[%u] "
      "http_listen_port[%u] log_directory[%s] log_file[%s] log_level[%u] "
      "build_thread_count[%u] dump_thread_count[%u] "
      "max_build_qps[%u] index_directory[%s] "
      "flush_internal[%u] optimize_internal[%u] meta_uri[%s] "
      "query_thread_count[%u]",
      this->get_protocol().c_str(), this->get_grpc_listen_port(),
      this->get_http_listen_port(), this->get_log_dir().c_str(),
      this->get_log_file().c_str(), this->get_log_level() + 1,
      this->get_index_build_thread_count(), this->get_index_dump_thread_count(),
      this->get_index_max_build_qps(), this->get_index_directory().c_str(),
      this->get_index_flush_internal(), this->get_index_optimize_internal(),
      this->get_meta_uri().c_str(), this->get_query_thread_count());

  return 0;
}

int Config::cleanup() {
  config_.Clear();
  return 0;
}

bool Config::validate_config() const {
  /** ========== valid Common Config========= **/
  std::string protocol = this->get_protocol();
  std::vector<std::string> prots;
  ailego::StringHelper::Split<std::string>(protocol, '|', &prots);
  auto it =
      std::find_if(prots.begin(), prots.end(), [](const std::string &str) {
        return (str == "grpc") || (str == "http");
      });
  if (it == prots.end()) {
    LOG_ERROR(
        "Config error, protocol must contains grpc or http at least. "
        "protocol[%s]",
        protocol.c_str());
    return false;
  }

  if (this->get_grpc_listen_port() > 65535) {
    LOG_ERROR("Config error, grpc_listen_port must be [0, 65535]. port[%u]",
              this->get_grpc_listen_port());
    return false;
  }

  if (this->get_http_listen_port() > 65535) {
    LOG_ERROR("Config error, http_listen_port must be [0, 65535]. port[%u]",
              this->get_http_listen_port());
    return false;
  }

  if (this->get_logger_type() != "ConsoleLogger" &&
      this->get_logger_type() != "AppendLogger" &&
      this->get_logger_type() != "SysLogger") {
    LOG_ERROR("Config error, unknown logger type. logger[%s]",
              this->get_logger_type().c_str());
    return false;
  }

  /** ========== valid Index Config========= **/
  if (this->get_index_build_thread_count() > 500) {
    LOG_ERROR(
        "Config error, build_thread_count must be [1, 500]. thread_count[%u]",
        this->get_index_build_thread_count());
    return false;
  }

  if (this->get_index_dump_thread_count() > 500) {
    LOG_ERROR(
        "Config error, dump_thread_count must be [2, 500]. thread_count[%u]",
        this->get_index_dump_thread_count());
    return false;
  }

  /** ========== valid Query Config========= **/
  if (this->get_query_thread_count() > 500) {
    LOG_ERROR(
        "Config error, query_thread_count must be [1, 500]. thread_count[%u]",
        this->get_query_thread_count());
    return false;
  }

  /** ========== valid Meta Config========= **/
  return true;
}

std::string Config::get_protocol() const {
  std::string protocol = "grpc|http";
  if (config_.has_common_config() &&
      !config_.common_config().protocol().empty()) {
    protocol = config_.common_config().protocol();
  }
  return protocol;
}

uint32_t Config::get_grpc_listen_port() const {
  uint32_t listen_port = 16000;
  if (config_.has_common_config() &&
      config_.common_config().grpc_listen_port() != 0U) {
    listen_port = config_.common_config().grpc_listen_port();
  }
  return listen_port;
}

uint32_t Config::get_http_listen_port() const {
  uint32_t listen_port = 16001;
  if (config_.has_common_config() &&
      config_.common_config().http_listen_port() != 0U) {
    listen_port = config_.common_config().http_listen_port();
  }
  return listen_port;
}

std::string Config::get_log_dir() const {
  std::string log_dir;
  ailego::FileHelper::GetWorkingDirectory(&log_dir);
  log_dir.append("/log/");
  if (config_.has_common_config() &&
      !config_.common_config().log_directory().empty()) {
    log_dir = config_.common_config().log_directory();
  }
  return log_dir;
}

std::string Config::get_log_file() const {
  std::string log_file = "proxima_be.log";
  if (config_.has_common_config() &&
      !config_.common_config().log_file().empty()) {
    log_file = config_.common_config().log_file();
  }
  return log_file;
}

uint32_t Config::get_log_level() const {
  uint32_t actual_log_level = 2;
  if (config_.has_common_config()) {
    uint32_t input_log_level = config_.common_config().log_level();
    if (input_log_level < 1 || input_log_level > 5) {
      actual_log_level = 1;
    } else {
      actual_log_level = input_log_level - 1;
    }
  }
  return actual_log_level;
}

std::string Config::get_logger_type() const {
  std::string logger_type = "AppendLogger";
  if (!config_.common_config().logger_type().empty()) {
    logger_type = config_.common_config().logger_type();
  }
  return logger_type;
}

uint32_t Config::get_index_build_thread_count(void) const {
  uint32_t thread_count = 10U;
  if (config_.has_index_config() &&
      config_.index_config().build_thread_count() != 0) {
    thread_count = config_.index_config().build_thread_count();
  }
  return thread_count;
}

uint32_t Config::get_index_dump_thread_count(void) const {
  uint32_t thread_count = 3U;
  if (config_.has_index_config() &&
      config_.index_config().dump_thread_count() != 0) {
    thread_count = config_.index_config().dump_thread_count();
  }
  return thread_count;
}

uint32_t Config::get_index_max_build_qps(void) const {
  uint32_t max_build_qps = 0;
  if (config_.has_index_config()) {
    max_build_qps = config_.index_config().max_build_qps();
  }
  return max_build_qps;
}

std::string Config::get_index_directory(void) const {
  std::string index_directory;
  // Set default path to current binary path
  ailego::FileHelper::GetWorkingDirectory(&index_directory);
  if (config_.has_index_config() &&
      !config_.index_config().index_directory().empty()) {
    index_directory = config_.index_config().index_directory();
  }
  return index_directory;
}

uint32_t Config::get_index_flush_internal(void) const {
  uint32_t flush_internal = 300U;
  if (config_.has_index_config() &&
      config_.index_config().flush_internal() != 0) {
    flush_internal = config_.index_config().flush_internal();
  }
  return flush_internal;
}

uint32_t Config::get_index_optimize_internal(void) const {
  uint32_t optimize_internal = 0U;
  if (config_.has_index_config() &&
      config_.index_config().optimize_internal() != 0) {
    optimize_internal = config_.index_config().optimize_internal();
  }
  return optimize_internal;
}

std::string Config::get_meta_uri(void) const {
  if (config_.has_meta_config() && !config_.meta_config().meta_uri().empty()) {
    return config_.meta_config().meta_uri();
  }
  std::string meta_uri = "sqlite://";
  std::string work_directory;
  ailego::FileHelper::GetWorkingDirectory(&work_directory);
  meta_uri.append(work_directory);
  meta_uri.append("/proxima_be_meta.sqlite");
  return meta_uri;
}

uint32_t Config::get_query_thread_count(void) const {
  /// Default set thread count to the number of cpu cores
  uint32_t thread_count = std::thread::hardware_concurrency();
  if (config_.has_query_config() &&
      config_.query_config().query_thread_count() != 0) {
    thread_count = config_.query_config().query_thread_count();
  }
  return thread_count;
}

}  // namespace be
}  // end namespace proxima
