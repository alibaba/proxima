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
 *   \brief    Interface of Bilin Config
 */

#pragma once

#include <ailego/pattern/singleton.h>
#include "proto/config.pb.h"
#include "macro_define.h"

namespace proxima {
namespace be {

/*! Config
 */
class Config : public ailego::Singleton<Config> {
 public:
  //! Load config from file
  int load_config(const std::string &file_name);

  //! Cleanup config options
  int cleanup();

  //! Validate config
  bool validate_config() const;

  /** ============Common Config for ProximaSE============= **/
  //! Get rpc protocol
  std::string get_protocol() const;

  //! Get grpc listen port
  uint32_t get_grpc_listen_port() const;

  //! Get http listen port
  uint32_t get_http_listen_port() const;

  //! Get log directory
  std::string get_log_dir() const;

  //! Get log file name
  std::string get_log_file() const;

  //! Get log print level [0~5]
  uint32_t get_log_level() const;

  //! Get logger type
  std::string get_logger_type() const;

  /** ============Index Config============= **/
  //! Get index build thread count
  uint32_t get_index_build_thread_count(void) const;

  //! Get index dump thread count
  uint32_t get_index_dump_thread_count(void) const;

  //! Get index agent max build qps
  uint32_t get_index_max_build_qps(void) const;

  //! Get directory of index data
  std::string get_index_directory(void) const;

  //! Get flush internal seconds
  uint32_t get_index_flush_internal(void) const;

  //! Get optimize internal seconds
  uint32_t get_index_optimize_internal(void) const;

  /** ============Meta Config============= **/
  std::string get_meta_uri(void) const;

  /** ============Query Config============= **/
  //! Get query thread count
  uint32_t get_query_thread_count(void) const;

  //! Get metrics config
  const proto::MetricsConfig &metrics_config() const {
    return config_.common_config().metrics_config();
  }

 private:
  //! Members
  std::string config_file_{};
  proto::ProximaSEConfig config_{};
};


}  // namespace be
}  // end namespace proxima
