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
 *   \brief    Client tool which can connect & operate remote
 *             collections of proxima be
 */

#include <iostream>
#include <map>
#include <string>
#include <ailego/utility/time_helper.h>
#include <brpc/channel.h>
#include <gflags/gflags.h>
#include <google/protobuf/util/json_util.h>
#include "common/logger.h"
#include "common/version.h"
#include "proto/proxima_be.pb.h"

DEFINE_string(command, "", "Command type: create | drop");
DEFINE_string(host, "", "The host of proxima be");
DEFINE_string(collection, "", "Collection name");
DEFINE_string(schema, "", "Collection schema");

static bool ValidateNotEmpty(const char *flagname, const std::string &value) {
  return !value.empty();
}

DEFINE_validator(command, ValidateNotEmpty);
DEFINE_validator(host, ValidateNotEmpty);
DEFINE_validator(collection, ValidateNotEmpty);

static inline void PrintUsage() {
  std::cout << "Usage:" << std::endl;
  std::cout << " admin_client <args>" << std::endl << std::endl;
  std::cout << "Args: " << std::endl;
  std::cout << " --command      Command type: create | drop" << std::endl;
  std::cout << " --host         The host of proxima be" << std::endl;
  std::cout << " --collection   Specify collection name" << std::endl;
  std::cout << " --schema       Specify collection schema format" << std::endl;
  std::cout << " --help, -h     Display help info" << std::endl;
  std::cout << " --version, -v  Dipslay version info" << std::endl;
}

static brpc::Channel g_client_channel;

static bool InitClientChannel() {
  brpc::ChannelOptions options;
  options.protocol = "http";
  options.timeout_ms = 60000;
  options.max_retry = 2;

  if (g_client_channel.Init(FLAGS_host.c_str(), "", &options) != 0) {
    LOG_ERROR("Init client channel failed.");
    return false;
  }
  return true;
}

static void CreateCollection() {
  if (FLAGS_schema.empty()) {
    LOG_ERROR("Input schema can't be empty");
    exit(1);
  }

  char url[2048] = {0};
  snprintf(url, sizeof(url), "%s/v1/collection/%s", FLAGS_host.c_str(),
           FLAGS_collection.c_str());
  ailego::ElapsedTime timer;
  brpc::Controller cntl;
  cntl.http_request().uri() = url;
  cntl.http_request().set_method(brpc::HTTP_METHOD_POST);
  cntl.request_attachment().append(FLAGS_schema);
  g_client_channel.CallMethod(nullptr, &cntl, nullptr, nullptr, nullptr);

  if (!cntl.Failed()) {
    google::protobuf::util::JsonParseOptions options;
    options.ignore_unknown_fields = true;
    proxima::be::proto::Status response;
    std::string resp_body = cntl.response_attachment().to_string();
    auto status = google::protobuf::util::JsonStringToMessage(
        resp_body, &response, options);
    if (status.ok() && response.code() == 0) {
      LOG_INFO("Create collection success. collection[%s] rt[%zums]",
               FLAGS_collection.c_str(), (size_t)timer.milli_seconds());
    } else {
      LOG_ERROR("Create collection error. code[%d] reason[%s]", response.code(),
                response.reason().c_str());
    }
  } else {
    LOG_ERROR("Create collection error. error_msg[%s]",
              cntl.ErrorText().c_str());
  }
}

static void DropCollection() {
  char url[2048] = {0};
  snprintf(url, sizeof(url), "%s/v1/collection/%s", FLAGS_host.c_str(),
           FLAGS_collection.c_str());
  ailego::ElapsedTime timer;
  brpc::Controller cntl;
  cntl.http_request().uri() = url;
  cntl.http_request().set_method(brpc::HTTP_METHOD_DELETE);
  g_client_channel.CallMethod(nullptr, &cntl, nullptr, nullptr, nullptr);

  if (!cntl.Failed()) {
    google::protobuf::util::JsonParseOptions options;
    options.ignore_unknown_fields = true;
    proxima::be::proto::Status response;
    std::string resp_body = cntl.response_attachment().to_string();
    auto status = google::protobuf::util::JsonStringToMessage(
        resp_body, &response, options);
    if (status.ok() && response.code() == 0) {
      LOG_INFO("Drop collection success. collection[%s] rt[%zums]",
               FLAGS_collection.c_str(), (size_t)timer.milli_seconds());
    } else {
      LOG_ERROR("Drop collection error. code[%d] reason[%s]", response.code(),
                response.reason().c_str());
    }
  } else {
    LOG_ERROR("Drop collection error. error_msg[%s]", cntl.ErrorText().c_str());
  }
}

int main(int argc, char **argv) {
  // Parse arguments
  for (int i = 1; i < argc; ++i) {
    const char *arg = argv[i];
    if (!strcmp(arg, "-help") || !strcmp(arg, "--help") || !strcmp(arg, "-h")) {
      PrintUsage();
      exit(0);
    } else if (!strcmp(arg, "-version") || !strcmp(arg, "--version") ||
               !strcmp(arg, "-v")) {
      std::cout << proxima::be::Version::Details() << std::endl;
      exit(0);
    }
  }
  gflags::ParseCommandLineNonHelpFlags(&argc, &argv, false);

  // Init client channel
  if (!InitClientChannel()) {
    LOG_ERROR("Init client channel failed. host[%s]", FLAGS_host.c_str());
    exit(1);
  }

  // Register commands
  std::map<std::string, std::function<void(void)>> collection_ops = {
      {"create", CreateCollection}, {"drop", DropCollection}};
  if (collection_ops.find(FLAGS_command) != collection_ops.end()) {
    collection_ops[FLAGS_command]();
  } else {
    LOG_ERROR("Unsupported command type: %s", FLAGS_command.c_str());
    exit(1);
  }

  return 0;
}
