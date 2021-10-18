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
 *   \brief    Main function
 */

#include <signal.h>
#include <string.h>
#include <iostream>
#include <ailego/debug/bug_report.h>
#include <ailego/utility/process_helper.h>
#include <gflags/gflags.h>
#include "repository/repository_common/config.h"
#include "repository/repository_common/version.h"
#include "mysql_repository.h"

DEFINE_string(config, "", "Read configuration from this file");
DEFINE_string(pidfile, "", "Write the pid into this file");
DEFINE_bool(daemon, false, "Run this app in daemon mode");

static bool ValidateConfig(const char *flagname, const std::string &config) {
  return !config.empty();
}

DEFINE_validator(config, ValidateConfig);

static inline std::string GetVersion() {
  return std::string("Version: 0.0.1");
}

static inline std::string PrintUsage() {
  std::string usage =
      "Usage: \n"
      "    mysql_repository [options] \n\n"
      "Options: \n"
      "    --config          Read configuration from this file.\n"
      "    --daemon          Run this app in daemon mode.\n"
      "    --pidfile         Write the pid into this file.\n";
  return usage;
}

void ShutdownHandler(int sig) {
  LOG_INFO("Receive stop signal: %d", sig);
  auto &module = proxima::be::repository::MysqlRepository::Instance();
  module.stop();
  module.cleanup();
}

static inline void SetupSignals() {
  ailego::ProcessHelper::IgnoreSignal(SIGHUP);
  ailego::ProcessHelper::IgnoreSignal(SIGPIPE);
  ailego::ProcessHelper::RegisterSignal(SIGINT, ShutdownHandler);
  ailego::ProcessHelper::RegisterSignal(SIGTERM, ShutdownHandler);
  ailego::ProcessHelper::RegisterSignal(SIGUSR1, ShutdownHandler);
  ailego::ProcessHelper::RegisterSignal(SIGUSR2, ShutdownHandler);
}

int main(int argc, char *argv[]) {
  // Parse arguments
  for (int i = 1; i < argc; ++i) {
    const char *arg = argv[i];

    if (!strcmp(arg, "-help") || !strcmp(arg, "--help") || !strcmp(arg, "-h")) {
      std::cout << PrintUsage() << std::endl;
      exit(0);
    } else if (!strcmp(arg, "-version") || !strcmp(arg, "--version") ||
               !strcmp(arg, "-v")) {
      std::cout << proxima::be::repository::Version::Details() << std::endl;
      exit(0);
    }
  }

  gflags::SetVersionString(GetVersion());
  gflags::ParseCommandLineNonHelpFlags(&argc, &argv, false);

  // Load config
  proxima::be::repository::Config &config =
      proxima::be::repository::Config::Instance();
  int ret = config.load_repository_config(FLAGS_config);
  if (ret != 0) {
    std::cerr << "Mysql repository load config failed." << std::endl;
    exit(1);
  }

  if (!config.validate_repository_config()) {
    std::cerr << "Mysql repository valid config error." << std::endl;
    exit(1);
  }

  // Initialize bug report
  ailego::BugReport::Bootstrap(argc, argv, config.get_log_dir().c_str());

  // Start module
  auto &module = proxima::be::repository::MysqlRepository::Instance();

  ret = module.init(FLAGS_daemon, FLAGS_pidfile);
  if (ret != 0) {
    std::cerr << "Mysql repository init failed." << std::endl;
    exit(1);
  }

  ret = module.start();
  if (ret != 0) {
    std::cerr << "Mysql repository start failed." << std::endl;
    module.stop();
    module.cleanup();
    exit(1);
  } else {
    std::cout << "Mysql repository start successfully." << std::endl;
  }

  // Handle signals
  SetupSignals();

  // Wait for signals
  pause();

  // Stop and cleanup module
  module.stop();
  module.cleanup();
  return 0;
}
