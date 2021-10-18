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

class ConfReplacer:
  def __init__(self, conf_path):
    self.conf_path = conf_path
    self.global_conf = os.path.join(conf_path, 'global.conf')
    self.cf = configparser.ConfigParser()

  def init(self):
    arr = self.cf.read(self.global_conf)
    if len(arr) == 0:
      print (self.global_conf)
      return False
    self.items = self.cf.items("common")
    return True

  def replace(self):
    files = os.listdir(self.conf_path)
    for f in files:
      if f.endswith('.tpl'):
        src_name = os.path.join(self.conf_path, f)
        dst_name = src_name[0:-4]
        shutil.copyfile(src_name, dst_name)
        for item in self.items:
          cmd = "sed -i 's#${%s}#%s#g' %s" % (item[0], item[1], dst_name)
          ret = os.system(cmd)
#          print (cmd)
          if ret != 0:
            print (cmd)
            return False
    return True

if __name__ == '__main__':
  if len(sys.argv) != 2:
    print ('usage: ./conf_replacer.py conf_directory')
    sys.exit(-1)
  replacer = ConfReplacer(sys.argv[1])
  ret = replacer.init()
  if not ret:
    sys.exit(-1)
  if not replacer.replace():
    sys.exit(-1)
  sys.exit(0)
