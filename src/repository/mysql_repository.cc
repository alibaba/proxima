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
 *   \date     Dec 2020
 *   \brief    Implementation of MysqlRepository
 */

#include "mysql_repository.h"
#include <ailego/utility/process_helper.h>
#include "repository/collection_creator.h"
#include "repository/repository_common/config.h"
#include "repository/repository_common/error_code.h"
#include "repository_common/logger.h"

namespace proxima {
namespace be {
namespace repository {

int MysqlRepository::init(bool daemonized, const std::string &pid_file) {
  if (!pid_file.empty() && !pid_file_.open(pid_file)) {
    LOG_ERROR("ProximaSE open the pid file failed, pid_file=[%s].",
              pid_file.c_str());
    return ErrorCode_OpenFile;
  }

  LOG_INFO("Start to init repository");
  daemonized_ = daemonized;

  // init logger
  int ret = init_logger();
  if (ret != 0) {
    LOG_ERROR("Mysql repository init logger error");
    return ret;
  }

  // init meta agent
  CollectionCreatorPtr collection_creator =
      std::make_shared<CollectionCreatorImpl>();
  collection_manager_ = std::make_shared<CollectionManager>(collection_creator);
  ret = collection_manager_->init();
  if (ret != 0) {
    LOG_ERROR("Init collection manager failed.");
    return ret;
  }

  return 0;
}

int MysqlRepository::cleanup() {
  // add lock protection
  // cleanup maybe be called in multithread
  std::lock_guard<std::mutex> lock(mutex_);

  if (collection_manager_ != nullptr) {
    collection_manager_->cleanup();
    collection_manager_.reset();
  }

  LogUtil::Shutdown();
  Config::Instance().cleanup();

  daemonized_ = false;

  return 0;
}

int MysqlRepository::start() {
  if (daemonized_) {
    daemonize();
  }

  // start collection manager
  // todo: will not return, unless finished(because of while loop), so can i use
  // another thread to start?  to review
  collection_manager_->start();

  is_running_ = true;
  LOG_INFO("Mysql repository start successfully.");
  return 0;
}

int MysqlRepository::stop() {
  // add lock protection
  // stop maybe be called in multithread
  std::lock_guard<std::mutex> lock(mutex_);
  if (!is_running_) {
    return 0;
  }

  if (collection_manager_ != nullptr) {
    collection_manager_->stop();
  }

  pid_file_.close();
  is_running_ = false;

  return 0;
}

int MysqlRepository::init_logger() {
  // get logger config
  std::string log_dir = repository::Config::Instance().get_log_dir();
  std::string log_file = repository::Config::Instance().get_log_file();
  uint32_t log_level = repository::Config::Instance().get_log_level();
  auto logger_name = repository::Config::Instance().get_logger_name();

  // int logger
  return LogUtil::Init(log_dir, log_file, log_level, logger_name);
}

void MysqlRepository::daemonize() {
  std::string log_dir = Config::Instance().get_log_dir();
  std::string stdout_path = log_dir + "./stdout.log";
  std::string stderr_path = log_dir + "./stderr.log";
  ailego::ProcessHelper::Daemon(stdout_path.c_str(), stderr_path.c_str());
}

}  // namespace repository
}  // namespace be
}  // end namespace proxima
