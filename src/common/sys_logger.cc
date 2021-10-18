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

 *   \author   Jiliang.ljl
 *   \date     Mar 2021
 *   \brief    Implementation of syslog
 */

#include <syslog.h>
#include <ailego/io/file.h>
#include <aitheta2/index_factory.h>

namespace proxima {
namespace be {

class SysLogger : public aitheta2::IndexLogger {
 public:
  SysLogger() = default;

  ~SysLogger() {
    this->cleanup();
  }

 public:
  int init(const aitheta2::IndexParams &params) override {
    program_name_ = params.get_as_string("proxima.program.program_name");
    // openlog require first argument to be accessible forever
    openlog(program_name_.c_str(), LOG_PID, LOG_DAEMON);
    return 0;
  }

  int cleanup() override {
    closelog();
    program_name_.clear();
    return 0;
  }

  void log(int level, const char *file, int line, const char *format,
           va_list args) override {
    static int log_level_mapping[] = {LOG_DEBUG, LOG_INFO, LOG_WARNING, LOG_ERR,
                                      LOG_CRIT};
    char buf[2048];
    vsnprintf(buf, sizeof(buf), format, args);
    syslog(log_level_mapping[level], "%s:%d %s", file, line, buf);
  }

 private:
  std::string program_name_;
};

INDEX_FACTORY_REGISTER_LOGGER(SysLogger);

}  // namespace be
}  // end namespace proxima
