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

 *   \author   Dianzhang.Chen
 *   \date     Mar 2021
 *   \brief    Implementation of Config
 */

#include "config.h"
#include <thread>
#include <ailego/io/file.h>
#include <google/protobuf/text_format.h>
#include "error_code.h"
#include "logger.h"

namespace proxima {
namespace be {
namespace repository {

int Config::load_repository_config(const std::string &file_name) {
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

  bool ret = google::protobuf::TextFormat::ParseFromString(file_content,
                                                           &repository_config_);
  if (!ret) {
    LOG_ERROR("Parse file %s content %s failed.", file_name.c_str(),
              file_content.c_str());

    return ErrorCode_LoadConfig;
  }
  config_file_ = file_name;
  // repository_config_file_ = file_name;     // todo <cdz> : maybe use seprate
  // file

  return 0;
}

int Config::cleanup() {
  return 0;
}

bool Config::validate_repository_config() const {
  /** ========== valid Common Config========= **/
  // todo<cdz>
  return true;
}

/** ============Common Config for Repository============= **/
std::string Config::get_log_dir() const {
  std::string log_dir = "./log/";
  if (repository_config_.has_common_config() &&
      !repository_config_.common_config().log_directory().empty()) {
    log_dir = repository_config_.common_config().log_directory();
  }
  return log_dir;
}

std::string Config::get_log_file() const {
  std::string log_file = "mysql_repository.log";
  if (repository_config_.has_common_config() &&
      !repository_config_.common_config().log_file().empty()) {
    log_file = repository_config_.common_config().log_file();
  }
  return log_file;
}

uint32_t Config::get_log_level() const {
  uint32_t actual_log_level = 1;
  if (repository_config_.has_common_config()) {
    uint32_t input_log_level = repository_config_.common_config().log_level();
    if (input_log_level < 1 || input_log_level > 5) {
      actual_log_level = 1;
    } else {
      actual_log_level = input_log_level - 1;
    }
  }
  return actual_log_level;
}

std::string Config::get_logger_name() const {
  std::string logger_name = "RepositoryAppendLogger";
  if (!repository_config_.common_config().logger_name().empty()) {
    logger_name = repository_config_.common_config().logger_name();
  }
  return logger_name;
}

std::string Config::get_index_agent_uri(void) const {
  std::string index_agent_addr("0.0.0.0:16000");
  if (repository_config_.has_repository_config() &&
      !repository_config_.repository_config().index_agent_addr().empty()) {
    return repository_config_.repository_config().index_agent_addr();
  }
  return index_agent_addr;
}

const std::string &Config::get_repository_name(void) const {
  static std::string name = "mysql_repository";
  if (repository_config_.has_repository_config() &&
      !repository_config_.repository_config().repository_name().empty()) {
    return repository_config_.repository_config().repository_name();
  }
  return name;
}

const std::string &Config::get_load_balance(void) const {
  static std::string load_balance("");
  if (repository_config_.has_repository_config() &&
      !repository_config_.repository_config().load_balance().empty()) {
    return repository_config_.repository_config().load_balance();
  }
  return load_balance;
}

uint32_t Config::get_batch_size(void) const {
  uint32_t batch_size = 64;
  if (repository_config_.has_repository_config() &&
      repository_config_.repository_config().batch_size() != 0) {
    return repository_config_.repository_config().batch_size();
  }
  return batch_size;
}

uint32_t Config::get_batch_interval(void) const {
  uint32_t batch_interval = 5000;
  if (repository_config_.has_repository_config() &&
      repository_config_.repository_config().batch_interval() != 0) {
    return repository_config_.repository_config().batch_interval();
  }
  return batch_interval;
}

int Config::get_max_retry(void) const {
  int max_retry = 3;
  if (repository_config_.has_repository_config() &&
      repository_config_.repository_config().max_retry() != 0) {
    return repository_config_.repository_config().max_retry();
  }
  return max_retry;
}

int Config::get_timeout_ms(void) const {
  int timeout_ms = 500;
  if (repository_config_.has_repository_config() &&
      repository_config_.repository_config().timeout_ms() != 0) {
    return repository_config_.repository_config().timeout_ms();
  }
  return timeout_ms;
}

}  // end namespace repository
}  // namespace be
}  // end namespace proxima
