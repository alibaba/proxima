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

import sys, time, os, json, configparser
import shutil

class GlobalConf:
  def __init__(self, conf_path=None):
    if conf_path:
      self.conf_path = conf_path
    else:
      path = './conf/global.conf'
      if not os.path.exists(path):
        path = '../conf/global.conf'
        if not os.path.exists(path):
          path = '../../conf/global.conf'
          if not os.path.exists(path):
            raise Exception("global.conf no exists")
      self.path = path
    self.cf = configparser.ConfigParser()
    arr = self.cf.read(self.path)
    if len(arr) == 0:
      print ("Invalid " + self.path)
      raise Exception("Invalid global.conf")
    self.items = self.cf.items("common")
    
  def mysql_port(self):
    return self.cf["common"]["mysql_port"]

  def http_port(self):
    return self.cf["common"]["http_port"]

  def grpc_port(self):
    return self.cf["common"]["grpc_port"]

if __name__ == '__main__':
  gf = GlobalConf()
  print (gf.mysql_port())
  print (gf.http_port())
  print (gf.grpc_port())

