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
 *   \brief    Implementation of grpc server
 */

#include "grpc_server.h"
#include <chrono>
#include "common/config.h"
#include "common/error_code.h"
#include "common/logger.h"

extern std::string GetVersion();

namespace proxima {
namespace be {
namespace server {


GrpcServerUPtr GrpcServer::Create() {
  return std::unique_ptr<GrpcServer>(new GrpcServer());
}

GrpcServer::~GrpcServer() {
  if (thread_ != nullptr) {
    this->stop();
  }
}

int GrpcServer::bind_and_start(const agent::IndexAgentPtr &index_agent,
                               const query::QueryAgentPtr &query_agent,
                               const admin::AdminAgentPtr &admin_agent,
                               const std::string &version) {
  auto *request_handler =
      new ProximaRequestHandler(index_agent, query_agent, admin_agent);
  if (!request_handler) {
    LOG_ERROR("Create proxima request handler failed.");
    return ErrorCode_RuntimeError;
  }

  // Set server version
  request_handler->set_version(version);
  server_.set_version(version);

  // Register grpc service
  int ret = server_.AddService((proto::ProximaService *)request_handler,
                               brpc::SERVER_OWNS_SERVICE);
  if (ret != 0) {
    LOG_ERROR("Grpc server add service failed.");
    return ret;
  }

  // async start grpc server in single thread
  thread_ = std::unique_ptr<std::thread>(
      new std::thread(&GrpcServer::start_server, this));

  // sleep 1s
  std::this_thread::sleep_for(std::chrono::seconds(1));

  return 0;
}

int GrpcServer::stop() {
  this->stop_server();
  if (thread_ != nullptr) {
    thread_->join();
    thread_ = nullptr;
  }
  return 0;
}

bool GrpcServer::is_running() {
  return server_.IsRunning();
}

int GrpcServer::start_server() {
  brpc::ServerOptions options;

  // Do not set auto concurrency limiter, it's unstable
  // max_concurrency | idle_timeout_sec options not open now

  // Configured by query thread count,
  // the pthread pool is shared in global.
  // We config query thread count + 1, just for preserving
  // one thread for scheduler.
  options.num_threads = Config::Instance().get_query_thread_count() + 1;
  uint32_t listen_port = Config::Instance().get_grpc_listen_port();
  int ret = server_.Start(listen_port, &options);
  if (ret != 0) {
    LOG_ERROR("Grpc server start failed.");
    return ret;
  }

  LOG_INFO("Grpc server start success. port[%u]", listen_port);

  while (server_.IsRunning()) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  LOG_INFO("Grpc server thread exit.");
  return 0;
}

int GrpcServer::stop_server() {
  server_.Stop(0);
  server_.Join();
  return 0;
}


}  // end namespace server
}  // namespace be
}  // end namespace proxima
