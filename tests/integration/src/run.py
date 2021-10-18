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

import os
import sys
import time
import unittest
import logging

def get_allsuite(path):
  return unittest.defaultTestLoader.discover(path, pattern="test_*.py")

def get_onesuite(path, name):
  suite = unittest.TestSuite()
  arr = name.split('.')
  if len(arr) == 1:
    pattern = name + ".py"
    suite = unittest.defaultTestLoader.discover(path, pattern=pattern)
  elif len(arr) == 2:
    suite = unittest.defaultTestLoader.discover(path, pattern=name)
  elif len(arr) == 3:
    case = unittest.defaultTestLoader.loadTestsFromName(name)
    tmp_suite = unittest.TestSuite()
    tmp_suite.addTest(case)
    suite.addTest(tmp_suite)
  return suite

def get_size(log_file):
  return os.path.getsize(log_file)

def get_content(log_file, offset):
  f = open(log_file, 'r')
  f.seek(offset)
  return f.read()

def print_case_status(case, status):
  print (case, '...', status)
  logging.info("%s ... %s", str(case), status)

if __name__ == '__main__':
  print("=============================================")
  path = sys.argv[1]
  sys.path.append(path)
  sys.path.append(path + '/case')
  if(len(sys.argv) == 2):
    suite = get_allsuite(path + "/case/")
  elif len(sys.argv) == 3:
    suite = get_onesuite(path + "/case/", sys.argv[2])
  else:
    print("Wrong argument for run.py")
    exit()
  log_file = 'it.log'
  failures_vec = []
  errors_vec = []
  totals = []
  start = time.time()
  for su in suite:
    for case in su:
      try:
          tests = vars(case)['_tests']
      except Exception as ex:
          print(vars(case))
          print(ex)
          raise
      for test in tests:
        totals.append(test)
        print_case_status(test, '')
        offset = get_size(log_file)
        result = test.run()
        if len(result.errors) != 0 or len(result.failures) != 0:
          log_content = get_content(log_file, offset)
          print (log_content.strip())
          if len(result.errors) != 0:
            for error in result.errors:
              print (error[1])
            print_case_status(test, 'ERROR')
            errors_vec.append(result.errors)
          else:
            for failure in result.failures:
              print (failure[0], failure[1])
            print_case_status(test, 'FAILURE')
            failures_vec.append(result.failures)
        else:
          print_case_status(test, 'OK')
  
  cost = time.time() - start

  print ("\n")
  for errors in errors_vec:
    for error in errors:
      print ("=" * 70)
      print ("ERROR: %s" % (error[0]))
      print ("-" * 70)
      print (error[1])
      print (">\n")

  for failures in failures_vec:
    for failure in failures:
      print ("=" * 70)
      print ("FAILURE: %s" % (failure[0]))
      print ("-" * 70)
      print ("%s>\n" % (failure[1]))

  print ("\nRan %s tests in %.3fs\n" % (len(totals), cost))
  if len(errors_vec) != 0 or len(failures_vec) != 0:
    print ("FAILED (errors=%d, failures=%d)" % (len(errors_vec), len(failures_vec)))
    sys.exit(1)
