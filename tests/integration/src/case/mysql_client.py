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

import sys, time, os, json, subprocess, logging
from global_conf import GlobalConf

class MysqlClient:
  def __init__(self, user='root', password='root',
               ip='127.0.0.1', port=None, db = 'test_db', bin_path='/usr/bin/env LANG=en_US.UTF-8 mysql'):
    self.base_dir = os.path.abspath(os.path.dirname(__file__))
    self.user = user
    self.password = password
    self.db = db
    self.ip = ip
    if not port:
      gf = GlobalConf()
      port = gf.mysql_port()
    self.port = int(port)
    if not bin_path:
      self.bin_path = self.base_dir + '/../../../../../deps/thirdparty/mysql/mysql-5.7.31/usr/local/mysql/bin/mysql'
    else:
      self.bin_path = bin_path
    self.command = '%s -u%s -p%s -h %s -P%d < ' % (self.bin_path, self.user, self.password, self.ip, self.port)
    self.sql_file = '.tmp.sql'

  def get_connection_uri(self):
    return "mysql://%s:%s@%s:%d/%s" % (self.user, self.password, self.ip, self.port, self.db)

  def execute(self, query):
    f = open(self.sql_file, 'w')
    f.write(query)
    f.close()
    cmd = self.command + self.sql_file
    logging.info("Cmd: %s" % (cmd))
    ret = subprocess.getstatusoutput(cmd)
    logging.info("%s", ret[1])
    return ret[0]

  def execute_batch_sql(self, sql_file, retry_times=3):
    i = 0
    code = 0
    while i < retry_times:
      cmd = self.command + sql_file
      logging.info("Cmd: %s" %(cmd))
      ret = subprocess.getstatusoutput(cmd)
      logging.info("%s", ret[1])
      code = ret[0]
      if code != 0:
        time.sleep(5)
      else:
        break
      i += 1
    return code

  def purge_binlog(self, file_name):
    arr = file_name.split('.')
    no = int(arr[1]) + 1
    no_str = str(no)
    pack_len = 6 - len(no_str)
    no_str = '0' * pack_len + no_str
    next_file = arr[0] + '.' + no_str
    cmd = "purge master logs to '" + next_file + "'"
    return self.execute(cmd)

if __name__ == '__main__':
  client = MysqlClient()
#  print (client.execute("select * from tt.t1"))

  print (client.execute_batch_sql('tmp.sql'))
