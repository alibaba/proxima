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
 *   \brief    Implementation of proxima search engine
 */

#include "proxima_search_engine.h"
#include <chrono>
#include <thread>
#include <ailego/utility/process_helper.h>
#include <ailego/utility/string_helper.h>
#include "common/config.h"
#include "common/error_code.h"
#include "common/logger.h"
#include "metrics/metrics_collector.h"

namespace proxima {
namespace be {
namespace server {

int ProximaSearchEngine::init(bool daemonized, const std::string &pid_file) {
  if (!pid_file.empty() && !pid_file_.open(pid_file)) {
    LOG_ERROR("ProximaSE open the pid file failed, pid_file=[%s].",
              pid_file.c_str());
    return ErrorCode_OpenFile;
  }
  daemonized_ = daemonized;

  // init logger
  int ret = init_logger();
  if (ret != 0) {
    LOG_ERROR("ProximaSE init logger error.");
    return ret;
  }

  // get config
  Config &config = Config::Instance();

  // init metrics
  ret =
      metrics::MetricsCollector::CreateAndInitMetrics(config.metrics_config());
  if (ret != 0) {
    LOG_ERROR("ProximaSE init metrics error");
    return ret;
  }

  // init meta agent
  meta_agent_ = meta::MetaAgent::Create(config.get_meta_uri());
  if (!meta_agent_) {
    LOG_ERROR("Create meta agent failed.");
    return ErrorCode_RuntimeError;
  }

  ret = meta_agent_->init();
  if (ret != 0) {
    LOG_ERROR("Init meta agent failed.");
    return ret;
  }

  // init index agent
  index_agent_ = agent::IndexAgent::Create(meta_agent_->get_service());
  if (!index_agent_) {
    LOG_ERROR("Create index agent failed.");
    return ErrorCode_RuntimeError;
  }

  ret = index_agent_->init();
  if (ret != 0) {
    LOG_ERROR("Init index agent failed.");
    return ret;
  }

  // init query agent
  uint32_t concurrency = config.get_query_thread_count();
  query_agent_ = query::QueryAgent::Create(
      index_agent_->get_service(), meta_agent_->get_service(), concurrency);
  if (!query_agent_) {
    LOG_ERROR("Create query agent failed.");
    return ErrorCode_RuntimeError;
  }

  ret = query_agent_->init();
  if (ret != 0) {
    LOG_ERROR("Init query agent failed.");
    return ret;
  }

  // init admin agent
  admin_agent_ =
      admin::AdminAgent::Create(meta_agent_, index_agent_, query_agent_);
  if (!admin_agent_) {
    LOG_ERROR("Create admin agent failed.");
    return ErrorCode_RuntimeError;
  }

  ret = admin_agent_->init();
  if (ret != 0) {
    LOG_ERROR("Init admin agent failed.");
    return ret;
  }

  grpc_server_ = GrpcServer::Create();
  if (!grpc_server_) {
    LOG_ERROR("GrpcServer create failed.");
    return ErrorCode_RuntimeError;
  }

  http_server_ = HttpServer::Create();
  if (!http_server_) {
    LOG_ERROR("HttpServer create failed.");
    return ErrorCode_RuntimeError;
  }

  return 0;
}

int ProximaSearchEngine::cleanup() {
  if (admin_agent_ != nullptr) {
    admin_agent_->cleanup();
  }

  if (query_agent_ != nullptr) {
    query_agent_->cleanup();
  }

  if (index_agent_ != nullptr) {
    index_agent_->cleanup();
  }

  if (meta_agent_ != nullptr) {
    meta_agent_->cleanup();
  }

  LOG_INFO("ProximaSE cleanup complete.");
  LogUtil::Shutdown();
  Config::Instance().cleanup();

  daemonized_ = false;
  return 0;
}

int ProximaSearchEngine::start() {
  if (daemonized_) {
    daemonize();
  }

  // start meta agent
  int ret = meta_agent_->start();
  if (ret != 0) {
    LOG_ERROR("Start meta agent failed.");
    return ret;
  }

  // start index agent
  ret = index_agent_->start();
  if (ret != 0) {
    LOG_ERROR("Start index agent failed.");
    return ret;
  }

  // start query agent
  ret = query_agent_->start();
  if (ret != 0) {
    LOG_ERROR("Start query agent failed.");
    return ret;
  }

  // start admin agent
  ret = admin_agent_->start();
  if (ret != 0) {
    LOG_ERROR("Start admin agent failed.");
    return ret;
  }

  // start grpc server
  if (support_brpc_protocol()) {
    ret = grpc_server_->bind_and_start(index_agent_, query_agent_, admin_agent_,
                                       version_);
    if (ret != 0) {
      LOG_ERROR("GrpcServer bind and start failed.");
      return ret;
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));
    if (!grpc_server_->is_running()) {
      return ErrorCode_StartServer;
    }
  }

  // start http server
  if (support_http_protocol()) {
    ret = http_server_->bind_and_start(index_agent_, query_agent_, admin_agent_,
                                       version_);
    if (ret != 0) {
      LOG_ERROR("HttpServer bind and start failed.");
      return ret;
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));

    if (!http_server_->is_running()) {
      return ErrorCode_StartServer;
    }
  }

  LOG_INFO("ProximaSE start successfully.");
  return 0;
}

int ProximaSearchEngine::stop() {
  if (is_stopping_.exchange(true)) {
    return 0;
  }

  if (grpc_server_ != nullptr && grpc_server_->is_running()) {
    grpc_server_->stop();
  }

  if (http_server_ != nullptr && http_server_->is_running()) {
    http_server_->stop();
  }

  if (admin_agent_ != nullptr) {
    admin_agent_->stop();
  }

  if (query_agent_ != nullptr) {
    query_agent_->stop();
  }

  if (index_agent_ != nullptr) {
    index_agent_->stop();
  }

  if (meta_agent_ != nullptr) {
    meta_agent_->stop();
  }

  pid_file_.close();

  LOG_INFO("ProximaSE stopped.");
  return 0;
}

int ProximaSearchEngine::init_logger() {
  // get logger config
  std::string log_dir = Config::Instance().get_log_dir();
  std::string log_file = Config::Instance().get_log_file();
  uint32_t log_level = Config::Instance().get_log_level();
  std::string logger_type = Config::Instance().get_logger_type();

  // int logger
  return LogUtil::Init(log_dir, log_file, log_level, logger_type);
}

void ProximaSearchEngine::daemonize() {
  std::string log_dir = Config::Instance().get_log_dir();
  std::string stdout_path = log_dir + "./stdout.log";
  std::string stderr_path = log_dir + "./stderr.log";
  ailego::ProcessHelper::Daemon(stdout_path.c_str(), stderr_path.c_str());
}

bool ProximaSearchEngine::support_brpc_protocol() {
  std::string protocol = Config::Instance().get_protocol();
  std::vector<std::string> prots;
  ailego::StringHelper::Split<std::string>(protocol, '|', &prots);
  auto it = std::find_if(prots.begin(), prots.end(),
                         [](const std::string &str) { return str == "grpc"; });
  return (it != prots.end());
}

bool ProximaSearchEngine::support_http_protocol() {
  std::string protocol = Config::Instance().get_protocol();
  std::vector<std::string> prots;
  ailego::StringHelper::Split<std::string>(protocol, '|', &prots);
  auto it = std::find_if(prots.begin(), prots.end(),
                         [](const std::string &str) { return str == "http"; });
  return (it != prots.end());
}

}  // end namespace server
}  // namespace be
}  // end namespace proxima
