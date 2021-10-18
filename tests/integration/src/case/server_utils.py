# Copyright 2021 Alibaba, Inc. and its affiliates. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#     http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import sys, time, os, json, logging, subprocess
import http.client

from google.protobuf.json_format import MessageToJson

from global_conf import GlobalConf
from log import *

class ServerUtils:
  def __init__(self):
    gf = GlobalConf()
    self.http_port = gf.http_port()
    self.grpc_port = gf.grpc_port()
    self.mysql_port = gf.mysql_port()

  # SIGKILL -9 | SIGUSR1 -10 | SIGUSR2 -12
  def stop_proxima_be(self, signal='SIGUSR2'):
    logging.info("Begin stop proxima be")
    cmd = "lsof -i :%s | grep LISTEN | grep proxima | awk '{print $2}' | xargs kill -s %s" % (str(self.http_port), signal)
    logging.info("stop cmd: %s", cmd)
    ret = os.system(cmd)
    if ret != 0:
      logging.error("execute cmd %s failed.", cmd)
      return False
    
    time.sleep(5)

    times = 30
    cmd = "lsof -i :%s" % (str(self.http_port))
    while times > 0:
      try:
        output = subprocess.check_output(cmd, shell=True)
        logging.info("output: %s", output)
      except:
        break
      time.sleep(5)
      times -= 5

    logging.info("End stop proxima be")

    return True
                                                                            
  def start_proxima_be(self):
    logging.info("Begin start proxima be")
    src_path = os.getenv('SRC_PATH')
    it_dir = src_path + '/tests/integration/script'
    build_dir = os.getenv('BUILD_DIR_NAME')
    cmd = 'sh %s/start_proxima_be.sh %s %s' % (it_dir, build_dir, src_path)
    logging.info(cmd)
    ret = os.system(cmd)

    times = 30
    cmd = "lsof -i :%s" % (str(self.http_port))
    while times > 0:
      try:
        output = subprocess.check_output(cmd, shell=True)
        logging.info("output: %s", output)
        break
      except:
        time.sleep(1)
        times -= 1

    logging.info("End start proxima be")

    return ret

  # SIGKILL -9 | SIGUSR1 -10 | SIGUSR2 -12
  def stop_mysql_repo(self, signal='SIGUSR2'):
    cmd = "ps auxwww | grep 'bin/mysql_repository' | grep -v grep | awk '{print $2}' | xargs kill -s %s" % (signal)
    logging.info("stop cmd: %s", cmd)
    ret = os.system(cmd)
    if ret != 0:
      logging.error("execute cmd %s failed.", cmd)
      return False
    time.sleep(1)
    return True
                                                                            
  def start_mysql_repo(self):
    src_path = os.getenv('SRC_PATH')
    it_dir = src_path + '/tests/integration/script'
    build_dir = os.getenv('BUILD_DIR_NAME')
    cmd = 'sh %s/start_repo.sh %s %s' % (it_dir, build_dir, src_path)
    logging.info(cmd)
    ret = os.system(cmd)
    time.sleep(1)
    return ret
  
  def stop_mysql(self):
    cmd = 'mysqladmin -u root -proot shutdown'
#    cmd = "ps auxwww | grep mysqld | grep -v grep | awk '{print $2}' | xarg kill -9"
    logging.info("stop cmd: %s", cmd)
    ret = os.system(cmd)
    if ret != 0:
      logging.error("execute cmd %s failed.", cmd)
      return False
    time.sleep(1)
    return True

  def start_mysql(self):
    src_path = os.getenv('SRC_PATH')
    it_dir = src_path + '/tests/integration/script'
    build_dir = os.getenv('BUILD_DIR_NAME')
    cmd = 'sh %s/start_mysql.sh %s %s' % (it_dir, build_dir, src_path)
    logging.info(cmd)
    ret = os.system(cmd)
    return ret    
    
