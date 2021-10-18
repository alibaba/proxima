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
 *   \brief    Grpc server module of proxima search engine
 */

#pragma once

#include <thread>
#include <brpc/server.h>
#include "common/macro_define.h"
#include "proxima_request_handler.h"

namespace proxima {
namespace be {
namespace server {

class GrpcServer;
using GrpcServerUPtr = std::unique_ptr<GrpcServer>;

/*
 * GrpcServer provides grpc service with protobuf protocol
 */
class GrpcServer {
 public:
  PROXIMA_DISALLOW_COPY_AND_ASSIGN(GrpcServer);

  //! Constructor
  GrpcServer() = default;

  //! Destructor
  ~GrpcServer();

  //! Constructor
  static GrpcServerUPtr Create();

 public:
  //! Start grpc server
  int bind_and_start(const agent::IndexAgentPtr &index_agent,
                     const query::QueryAgentPtr &query_agent,
                     const admin::AdminAgentPtr &admin_agent,
                     const std::string &version);

  //! Stop grpc server
  int stop();

  //! Check if server is running
  bool is_running();

 private:
  int start_server();

  int stop_server();

 private:
  brpc::Server server_{};
  std::unique_ptr<std::thread> thread_{};
};


}  // end namespace server
}  // namespace be
}  // end namespace proxima
