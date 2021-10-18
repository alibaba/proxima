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
 *   \brief    Interface of MysqlRepository
 */

#pragma once

#include <string>
#include <ailego/io/pid_file.h>
#include <ailego/pattern/singleton.h>
#include "repository/collection_manager.h"

namespace proxima {
namespace be {
namespace repository {

class MysqlRepository : public ailego::Singleton<MysqlRepository> {
 public:
  //! Initialize
  int init(bool daemonized, const std::string &pid_file);

  //! Cleanup memory
  int cleanup();

  //! Start server
  int start();

  //! Stop server
  int stop();

 private:
  //! Initilize logger
  int init_logger();

  //! Start as daemon
  void daemonize();

 private:
  bool daemonized_{false};
  ailego::PidFile pid_file_{};

  CollectionManagerPtr collection_manager_{};

  bool is_running_{false};
  std::mutex mutex_{};
};

}  // end namespace repository
}  // namespace be
}  // end namespace proxima
