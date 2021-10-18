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

 *   \author   Haichao.chc
 *   \date     Oct 2020
 *   \brief    Main class to describe proxima search engine
 *             interface and action. It's designed as sigleton.
 */

#pragma once

#include <string>
#include <ailego/io/pid_file.h>
#include <ailego/pattern/singleton.h>
#include "admin/admin_agent.h"
#include "agent/index_agent.h"
#include "common/macro_define.h"
#include "meta/meta_agent.h"
#include "query/query_agent.h"
#include "grpc_server.h"
#include "http_server.h"

namespace proxima {
namespace be {
namespace server {

class ProximaSearchEngine : public ailego::Singleton<ProximaSearchEngine> {
 public:
  //! Initialize
  int init(bool daemonized, const std::string &pid_file);

  //! Cleanup memory
  int cleanup();

  //! Start server
  int start();

  //! Stop server
  int stop();

 public:
  //! Set version
  void set_version(const char *val) {
    version_ = val;
  }

 private:
  //! Initilize logger
  int init_logger();

  //! Start as daemon
  void daemonize();

  //! Return if support brpc protocol
  bool support_brpc_protocol();

  //! Return if support http protocol
  bool support_http_protocol();

 private:
  bool daemonized_{false};
  ailego::PidFile pid_file_{};
  std::string version_{};

  agent::IndexAgentPtr index_agent_{};
  query::QueryAgentPtr query_agent_{};
  meta::MetaAgentPtr meta_agent_{};
  admin::AdminAgentPtr admin_agent_{};

  GrpcServerUPtr grpc_server_{};
  HttpServerUPtr http_server_{};

  std::atomic<bool> is_stopping_{false};
};

}  // end namespace server
}  // namespace be
}  // end namespace proxima
