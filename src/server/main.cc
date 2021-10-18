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
 *   \brief    Entry function of proxima search engine execution
 */

#include <signal.h>
#include <string.h>
#include <iostream>
#include <ailego/debug/bug_report.h>
#include <ailego/utility/process_helper.h>
#include <ailego/utility/string_helper.h>
#include <aitheta2/index_plugin.h>
#include <gflags/gflags.h>
#include "common/config.h"
#include "common/logger.h"
#include "common/version.h"
#include "server/proxima_search_engine.h"

DEFINE_string(config, "", "Read configuration from this file");
DEFINE_string(plugin, "", "Load proxima plugins from so files");
DEFINE_string(pidfile, "", "Write the pid into this file");
DEFINE_bool(daemon, false, "Run this app in daemon mode");

static bool ValidateConfig(const char *flagname, const std::string &config) {
  return !config.empty();
}

DEFINE_validator(config, ValidateConfig);

static inline std::string GetUsage() {
  std::string usage =
      "Usage: \n"
      "    proxima_be [options]\n\n"
      "Options: \n"
      "    --config <file_path>     Read configuration from this file.\n"
      "    --plugin <so_path>       Load proxima plugins, split with ','.\n"
      "    --daemon                 Run this app in daemon mode.\n"
      "    --pidfile <file_path>    Write the pid into this file.\n"
      "    --version, -v            Display version information.\n"
      "    --help, -h               Display available options.\n";
  return usage;
}

static bool LoadPlugins(const std::string &plugin_paths) {
  aitheta2::IndexPluginBroker broker;
  std::vector<std::string> so_list;
  ailego::StringHelper::Split<std::string>(plugin_paths, ',', &so_list);

  std::string error;
  for (auto &so_path : so_list) {
    if (!broker.emplace(so_path, &error)) {
      std::cerr << "Failed to load plugin: " << so_path << "(" << error << ")"
                << std::endl;
      return false;
    } else {
      std::cout << "Loaded plugin: " << so_path << std::endl;
    }
  }
  return true;
}

void ShutdownHandler(int sig) {
  LOG_INFO("Receive stop signal: %d", sig);
  auto &engine = proxima::be::server::ProximaSearchEngine::Instance();
  engine.stop();
}

static inline void SetupSignals() {
  ailego::ProcessHelper::IgnoreSignal(SIGHUP);
  ailego::ProcessHelper::IgnoreSignal(SIGPIPE);
  ailego::ProcessHelper::IgnoreSignal(SIGCHLD);

  ailego::ProcessHelper::RegisterSignal(SIGINT, ShutdownHandler);
  ailego::ProcessHelper::RegisterSignal(SIGTERM, ShutdownHandler);

  // These two signals are reserved for other usage
  ailego::ProcessHelper::RegisterSignal(SIGUSR1, ShutdownHandler);
  ailego::ProcessHelper::RegisterSignal(SIGUSR2, ShutdownHandler);
}

int main(int argc, char *argv[]) {
  // Parse arguments
  for (int i = 1; i < argc; ++i) {
    const char *arg = argv[i];

    if (!strcmp(arg, "-help") || !strcmp(arg, "--help") || !strcmp(arg, "-h")) {
      std::cout << GetUsage() << std::endl;
      exit(0);
    } else if (!strcmp(arg, "-version") || !strcmp(arg, "--version") ||
               !strcmp(arg, "-v")) {
      std::cout << proxima::be::Version::Details() << std::endl;
      exit(0);
    }
  }
  gflags::ParseCommandLineNonHelpFlags(&argc, &argv, false);

  // Load config
  proxima::be::Config &config = proxima::be::Config::Instance();
  int ret = config.load_config(FLAGS_config);
  if (ret != 0) {
    std::cerr << "ProximaSE load configuration failed." << std::endl;
    exit(1);
  }
  if (!config.validate_config()) {
    std::cerr << "ProximaSE validate configuration failed." << std::endl;
    exit(1);
  }

  // Load plugins
  if (!FLAGS_plugin.empty()) {
    if (!LoadPlugins(FLAGS_plugin)) {
      std::cerr << "ProximaSE load plugins failed." << std::endl;
      exit(1);
    }
  }

  // Initialize bug report
  ailego::BugReport::Bootstrap(argc, argv, config.get_log_dir().c_str());

  // Start engine
  auto &engine = proxima::be::server::ProximaSearchEngine::Instance();
  ret = engine.init(FLAGS_daemon, FLAGS_pidfile);
  if (ret != 0) {
    std::cerr << "ProximaSE init failed." << std::endl;
    exit(1);
  }
  engine.set_version(proxima::be::Version::String());

  ret = engine.start();
  if (ret != 0) {
    std::cerr << "ProximaSE start failed." << std::endl;
    engine.stop();
    engine.cleanup();
    exit(1);
  } else {
    std::cout << "ProximaSE start successfully." << std::endl;
  }

  // Handle signals
  SetupSignals();

  // Wait for signals
  pause();

  // Stop and cleanup engine
  engine.stop();
  engine.cleanup();
  return 0;
}
