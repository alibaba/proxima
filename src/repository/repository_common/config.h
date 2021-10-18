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
 *   \brief    Interface of Mysql Repository Config
 */

#pragma once

#include <ailego/pattern/singleton.h>
#include "proto/repository_config.pb.h"

namespace proxima {
namespace be {
namespace repository {

/*! Config
 */
class Config : public ailego::Singleton<Config> {
 public:
  //! Load config from file
  int load_repository_config(const std::string &file_name);

  //! Cleanup config options
  int cleanup();

  //! Validate config
  bool validate_config() const;

  //! Validate repository config
  bool validate_repository_config() const;

  /** ============Common Config for Repository============= **/
  //! Get log directory
  std::string get_log_dir() const;

  //! Get log file name
  std::string get_log_file() const;

  //! Get log print level [0~5]
  uint32_t get_log_level() const;

  //! Get logger name
  std::string get_logger_name() const;

  /** ============Repository Config=========== **/
  //! Get index agent server address
  std::string get_index_agent_uri(void) const;

  //! Get repository name
  const std::string &get_repository_name(void) const;

  //! Get brpc load balance
  const std::string &get_load_balance(void) const;

  //! Get index agent server address
  uint32_t get_batch_size(void) const;

  //! Get batch interval
  uint32_t get_batch_interval(void) const;

  //! Get brpc max_retry
  int get_max_retry(void) const;

  //! Get brpc max_retry
  int get_timeout_ms(void) const;

 private:
  //! Members
  std::string config_file_{};
  proto::RepositoryConfig repository_config_{};
};

}  // end namespace repository
}  // namespace be
}  // end namespace proxima
