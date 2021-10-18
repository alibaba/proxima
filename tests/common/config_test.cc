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
 *
 *   \author   haichao.chc
 *   \date     Dec 2020
 *   \brief
 */

#define private public
#define protected public
#include "common/config.h"
#undef private
#undef protected

#include <thread>
#include <ailego/io/file.h>
#include <gtest/gtest.h>


using namespace proxima::be;

TEST(ConfigTest, TestGeneral) {
  auto &config = Config::Instance();
  std::string pwd_path;
  ailego::FileHelper::GetWorkingDirectory(&pwd_path);

  ASSERT_EQ(config.get_protocol(), "grpc|http");
  ASSERT_EQ(config.get_grpc_listen_port(), 16000);
  ASSERT_EQ(config.get_http_listen_port(), 16001);
  ASSERT_EQ(config.get_log_dir(), pwd_path + "/log/");
  ASSERT_EQ(config.get_log_file(), "proxima_be.log");
  ASSERT_EQ(config.get_log_level(), 2);
  ASSERT_EQ(config.get_logger_type(), "AppendLogger");
  ASSERT_EQ(config.get_index_build_thread_count(), 10);
  ASSERT_EQ(config.get_index_dump_thread_count(), 3);
  ASSERT_EQ(config.get_index_max_build_qps(), 0);
  ASSERT_EQ(config.get_index_directory(), pwd_path);
  ASSERT_EQ(config.get_index_flush_internal(), 300);
  ASSERT_EQ(config.get_meta_uri(), std::string("sqlite://")
                                       .append(pwd_path)
                                       .append("/proxima_be_meta.sqlite"));
  ASSERT_EQ(config.get_query_thread_count(),
            std::thread::hardware_concurrency());

  // check wrong config
  auto *common_config = config.config_.mutable_common_config();
  common_config->set_protocol("h2sofa");
  ASSERT_EQ(config.validate_config(), false);
  common_config->set_protocol("http");
  ASSERT_EQ(config.validate_config(), true);
  common_config->set_protocol("grpc");
  ASSERT_EQ(config.validate_config(), true);

  common_config->set_grpc_listen_port(127433);
  ASSERT_EQ(config.validate_config(), false);
  common_config->set_grpc_listen_port(12345);
  ASSERT_EQ(config.validate_config(), true);
  common_config->set_grpc_listen_port(0);
  ASSERT_EQ(config.validate_config(), true);

  common_config->set_http_listen_port(123456);
  ASSERT_EQ(config.validate_config(), false);
  common_config->set_http_listen_port(12345);
  ASSERT_EQ(config.validate_config(), true);
  common_config->set_http_listen_port(0);
  ASSERT_EQ(config.validate_config(), true);

  common_config->set_logger_type("XXLogger");
  ASSERT_EQ(config.validate_config(), false);
  common_config->set_logger_type("SysLogger");
  ASSERT_EQ(config.validate_config(), true);
  common_config->set_logger_type("ConsoleLogger");
  ASSERT_EQ(config.validate_config(), true);

  auto *index_config = config.config_.mutable_index_config();
  index_config->set_build_thread_count(1000);
  ASSERT_EQ(config.validate_config(), false);
  index_config->set_build_thread_count(0);
  ASSERT_EQ(config.validate_config(), true);

  index_config->set_dump_thread_count(1000);
  ASSERT_EQ(config.validate_config(), false);
  index_config->set_dump_thread_count(0);
  ASSERT_EQ(config.validate_config(), true);

  auto *query_config = config.config_.mutable_query_config();
  query_config->set_query_thread_count(1000);
  ASSERT_EQ(config.validate_config(), false);
  query_config->set_query_thread_count(0);
  ASSERT_EQ(config.validate_config(), true);
}
