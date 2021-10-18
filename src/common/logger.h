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
 *   \brief    Encapsulation of aitheta2 index logger
 */

#pragma once

#include <ailego/io/file.h>
#include <aitheta2/index_factory.h>
#include <gflags/gflags.h>
#include "common/error_code.h"

namespace proxima {
namespace be {

class LogUtil {
 public:
  static int Init(const std::string &log_dir, const std::string &log_file,
                  int log_level, const std::string &logger_type) {
    if (log_dir.empty() || log_file.empty()) {
      return ErrorCode_InvalidArgument;
    }

    if (!ailego::File::IsExist(log_dir)) {
      ailego::File::MakePath(log_dir);
    }

    auto logger = aitheta2::IndexFactory::CreateLogger(logger_type);
    if (!logger) {
      LOG_FATAL("Invalid logger_type[%s]", logger_type.c_str());
      return ErrorCode_InvalidArgument;
    }

    aitheta2::IndexParams params;
    params.set("proxima.file.logger.log_dir", log_dir);
    params.set("proxima.file.logger.log_file", log_file);
    params.set("proxima.file.logger.path", log_dir + "/" + log_file);
    std::string program_name = ailego::File::BaseName(gflags::GetArgv0());
    params.set("proxima.program.program_name", program_name);

    int ret = logger->init(params);
    if (ret != 0) {
      return ret;
    }

    aitheta2::IndexLoggerBroker::SetLevel(log_level);
    aitheta2::IndexLoggerBroker::Register(logger);
    return 0;
  }

  static void Shutdown() {
    aitheta2::IndexLoggerBroker::Unregister();
  }
};

}  // namespace be
}  // end namespace proxima
