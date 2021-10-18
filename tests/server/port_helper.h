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
 *   \date     Feb 2021
 *   \brief    PortHelper definition
 */

#pragma once

#include <fstream>
#include <iostream>
#include <string>

namespace proxima {
namespace be {

class PortHelper {
 public:
  static void GetPort(int *port, int *pid) {
    *pid = getpid();
    static std::string cmd =
        "export PORT=$(python -c 'import socket; s=socket.socket(); "
        "s.bind((\"\", 0)); print(s.getsockname()[1]); s.close()')"
        "&& echo ${PORT} > %s.txt";
    char cmd_buf[1024];
    snprintf(cmd_buf, 1024, cmd.c_str(), std::to_string(*pid).c_str());
    system(cmd_buf);
    char filename_buf[1024];
    snprintf(filename_buf, 1024, "%s.txt", std::to_string(*pid).c_str());
    std::string filename = std::string(filename_buf);
    std::ifstream infile(filename);
    std::string port_str;
    std::getline(infile, port_str);
    *port = std::stoi(port_str);
  }

  static void RemovePortFile(int pid) {
    static std::string cmd = "rm %s.txt";
    char cmd_buf[1024];
    snprintf(cmd_buf, 1024, cmd.c_str(), std::to_string(pid).c_str());
    std::cout << "Execute: " << std::string(cmd_buf) << std::endl;
    system(cmd_buf);
  }
};

}  // namespace be
}  // end namespace proxima
