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
 *   \brief    Implementation of http server
 */

#include "http_server.h"
#include <chrono>
#include "common/config.h"
#include "common/error_code.h"
#include "common/logger.h"

extern std::string GetVersion();

namespace proxima {
namespace be {
namespace server {


HttpServerUPtr HttpServer::Create() {
  return std::unique_ptr<HttpServer>(new HttpServer());
}

HttpServer::~HttpServer() {
  if (thread_ != nullptr) {
    this->stop();
  }
}

int HttpServer::bind_and_start(const agent::IndexAgentPtr &index_agent,
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

  // Register http service
  int ret = server_.AddService(
      (proto::HttpProximaService *)request_handler, brpc::SERVER_OWNS_SERVICE,
      // HTTP:
      // GET /v1/collection/{collection}
      // POST /v1/collection/{collection}
      // PUT /v1/collection/{collection}, Not implement yet
      // DEL /v1/collection/{collection}
      "/v1/collection/* => collection,"
      // HTTP: GET /v1/collection/{collection}/stats
      "/v1/collection/*/stats => stats_collection,"
      // HTTP: POST /v1/collection/{collection}/index
      "/v1/collection/*/index => write,"
      // HTTP: GET /v1/collection/{collection}/doc?key={primary_key}
      "/v1/collection/*/doc => get_document_by_key,"
      // HTTP: POST /v1/collection/{collection}/query
      "/v1/collection/*/query => query,"
      // HTTP: GET /v1/collections?repository={repository}
      "/v1/collections => list_collections,"
      // HTTP: GET /version
      "/service_version => get_version");

  if (ret != 0) {
    LOG_ERROR("Http server add service failed.");
    return ret;
  }

  // set server version
  // server_.set_version(GetVersion());

  // async start http server in single thread
  thread_ = std::unique_ptr<std::thread>(
      new std::thread(&HttpServer::start_server, this));

  // sleep 1s
  std::this_thread::sleep_for(std::chrono::seconds(1));

  return 0;
}

int HttpServer::stop() {
  this->stop_server();
  if (thread_ != nullptr) {
    thread_->join();
    thread_ = nullptr;
  }
  return 0;
}

bool HttpServer::is_running() {
  return server_.IsRunning();
}

int HttpServer::start_server() {
  brpc::ServerOptions options;

  // Do not set auto concurrency limiter, it's unstable
  // max_concurrency | idle_timeout_sec options not open now

  // Configured by query thread count,
  // the pthread pool is shared in global.
  // We config query thread count + 1, just for preserving
  // one thread for scheduler.
  options.num_threads = Config::Instance().get_query_thread_count() + 1;
  uint32_t listen_port = Config::Instance().get_http_listen_port();
  int ret = server_.Start(listen_port, &options);
  if (ret != 0) {
    LOG_ERROR("Http server start failed.");
    return ret;
  }

  LOG_INFO("Http server start success. port[%u]", listen_port);

  while (server_.IsRunning()) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  LOG_INFO("Http server thread exit.");
  return 0;
}

int HttpServer::stop_server() {
  server_.Stop(0);
  server_.Join();
  return 0;
}


}  // end namespace server
}  // namespace be
}  // end namespace proxima
