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

import logging
import struct
import grpc
import unittest
import time
import json

from pyproximabe import *
from global_conf import GlobalConf
from collection_creator import CollectionCreator
from test_index_agent import TestIndexAgentBase
from server_utils import ServerUtils
OperationType = WriteRequest.OperationType

class TestIndexAgentException(TestIndexAgentBase):
  def setUp(self):
    self.with_repo = True
    super().setUp()
    self.server_utils = ServerUtils()

  def test_invalid_collection(self):
    magic_number = self.get_magic_number(self.collection_name)
    batch_count = 2
    collection_name = "invalid_collection";
    req = self.create_request(collection_name, magic_number,
                              batch_count, OperationType.INSERT)
    logging.info("request: %s", req)
    response = self.client.write(req)
    logging.info("process result: %s", response)
    self.assertEqual(response.code, -4002)
    self.assertEqual(response.reason, "Collection Not Exist")

  def test_invalid_forward_column(self):
    magic_number = self.get_magic_number(self.collection_name)
    batch_count = 2
    forwards_columns = ['invalid_forward', 'col_b']
    req = self.create_request(self.collection_name, magic_number,
                              batch_count, OperationType.INSERT,
                              forward_tuple_names=forwards_columns)
    logging.info("request: %s", req)
    response = self.client.write(req)
    logging.info("process result: %s", response)
    self.assertEqual(response.code, -2012)
    self.assertEqual(response.reason, "Invalid Write Request")

  def test_invalid_index_column(self):
    magic_number = self.get_magic_number(self.collection_name)
    batch_count = 2
    index_metas = [['invalid_column', DataType.VECTOR_FP32, self.index_dimensions[0]]]
    req = self.create_request(self.collection_name, magic_number,
                              batch_count, OperationType.INSERT,
                              index_tuple_metas=index_metas)
    logging.info("request: %s", req)
    response = self.client.write(req)
    logging.info("process result: %s", response)
    self.assertEqual(response.code, -2012)
    self.assertEqual(response.reason, "Invalid Write Request")

  def test_index_columns_num_mismatched(self):
    magic_number = self.get_magic_number(self.collection_name)
    batch_count = 2
    index_metas = [['invalid_column', DataType.VECTOR_FP32, self.index_dimensions[0]],
                   ['column1', DataType.VECTOR_FP32, self.index_dimensions[0]]
                   ]
    req = self.create_request(self.collection_name, magic_number,
                              batch_count, OperationType.INSERT,
                              index_tuple_metas=index_metas,
                              index_tuple_types=None)
    logging.info("request: %s", req)
    response = self.client.write(req)
    logging.info("process result: %s", response)
    self.assertEqual(response.code, -2012)
    self.assertEqual(response.reason, "Invalid Write Request")

  def test_invalid_lsn_sequence(self):
    magic_number = self.get_magic_number(self.collection_name)
    batch_count = 2
    forwards=[0.234, 'abc']
    req = self.create_request(self.collection_name, magic_number,
                              batch_count, OperationType.INSERT,
                              forwards=forwards)
    logging.info("request: %s", req)
    response = self.client.write(req)
    logging.info("process result: %s", response)
    self.assertEqual(response.code, 0)

    forwards=[1.234, 'def']
    req = self.create_request(self.collection_name, magic_number,
                              batch_count, OperationType.UPDATE,
                              forwards=forwards)
    logging.info("request: %s", req)
    response = self.client.write(req)
    logging.info("process result: %s", response)
    self.assertEqual(response.code, 0)

    time.sleep(1)

    status, response = self.simple_query()
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    self.assertEqual(len(documents), 2)
    self.assertEqual(documents[0].primary_key, 1)
    self.assertEqual(documents[0].score, 1.0)
    self.assertEqual(len(documents[0].forward_column_values), 2)
    self.assertAlmostEqual(documents[0].forward_column_values['col_a'], 1.234, delta=0.000001)
    self.assertEqual(documents[0].forward_column_values['col_b'], 'abc')

    self.assertEqual(documents[1].primary_key, 2)
    self.assertEqual(documents[1].score, 2.0)
    self.assertEqual(len(documents[1].forward_column_values), 2)
    self.assertAlmostEqual(documents[1].forward_column_values['col_a'], 2.234, delta=0.000001)
    self.assertEqual(documents[1].forward_column_values['col_b'], 'abc')

  def test_invalid_index_vector_data(self):
    magic_number = self.get_magic_number(self.collection_name)
    batch_count = 2
    req = self.create_request(self.collection_name, magic_number,
                              batch_count, OperationType.INSERT,
                              vector_exception=True)
    logging.info("request: %s", req)
    response = self.client.write(req)
    logging.info("process result: %s", response)
    self.assertEqual(response.code, -2023)
    self.assertEqual(response.reason, 'Mismatched Dimension')

  # def test_request_failed_with_collection_suspend(self):
  #   status, data = self.admin_client.suspend_collection(self.collection_name)
  #   logging.info("suspend collection result: %s", data)
  #   self.assertEqual(status, 200)
  #   self.assertEqual(data['code'], 0)

  #   response = self.client.get_collection(self.collection_name)
  #   logging.info("get collection result: %s", response)
  #   self.assertEqual(response.entity.config.status, 2) # 2 means cs_suspended

  #   magic_number = response.entity.magic_number
  #   batch_count = 2
  #   req = self.create_request(self.collection_name, magic_number,
  #                             batch_count, OperationType.INSERT)
  #   logging.info("request: %s", req)
  #   response = self.client.write(req)
  #   logging.info("process result: %s", response)
  #   self.assertEqual(response.code, -4010)
  #   self.assertEqual(response.reason, "Collection suspended")

  #   # resume collection
  #   status, data = self.admin_client.resume_collection(self.collection_name)
  #   logging.info("resume collection result: %s", data)
  #   self.assertEqual(status, 200)
  #   self.assertEqual(data['code'], 0)

  #   response = self.client.get_collection(self.collection_name)
  #   logging.info("get collection result: %s", response)
  #   self.assertEqual(response.entity.config.status, 1) # 1 means cs_serving

  #   response = self.client.write(req)
  #   logging.info("process result: %s", response)
  #   self.assertEqual(response.code, 0)
  #   self.assertEqual(response.reason, "Success")

  #   time.sleep(1)

  def test_update_with_key_not_exist(self):
    magic_number = self.get_magic_number(self.collection_name)
    batch_count = 2
    forwards=[0.234, 'abc']
    req = self.create_request(self.collection_name, magic_number,
                              batch_count, OperationType.UPDATE,
                              forwards=forwards)
    logging.info("request: %s", req)
    response = self.client.write(req)
    logging.info("process result: %s", response)
    self.assertEqual(response.code, 0)

    time.sleep(1)

    status, response = self.simple_query()
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    self.assertEqual(len(documents), 0)

  def test_delete_with_key_not_exist(self):
    magic_number = self.get_magic_number(self.collection_name)
    batch_count = 2
    forwards=[0.234, 'abc']
    operation_types = [OperationType.INSERT,
                       OperationType.DELETE]
    req = self.create_request(self.collection_name, magic_number,
                              batch_count,
                              operation_types=operation_types,
                              forwards=forwards)
    logging.info("request: %s", req)
    response = self.client.write(req)
    logging.info("process result: %s", response)
    self.assertEqual(response.code, 0)

    time.sleep(1)

    status, response = self.simple_query()
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    self.assertEqual(len(documents), 1)

  def test_insert_pk_repeated(self):
    magic_number = self.get_magic_number(self.collection_name)
    batch_count = 5
    forwards=[0.234, 'abc']
    req = self.create_request(self.collection_name, magic_number,
                              batch_count,
                              OperationType.INSERT,
                              forwards=forwards,
                              key_repeated = True)
    logging.info("request: %s", req)
    response = self.client.write(req)
    logging.info("process result: %s", response)
    self.assertEqual(response.code, 0)

    time.sleep(1)

    status, response = self.simple_query()
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    self.assertEqual(len(documents), 1)

  def test_empty_request(self):
    magic_number = self.get_magic_number(self.collection_name)
    batch_count = 5
    forwards=[0.234, 'abc']
    try:
      req = self.create_request(self.collection_name, magic_number,
                              batch_count,
                              OperationType.INSERT,
                              forwards=forwards,
                              empty_request=True)
      self.assertTrue(False)
    except:
      pass

    status, response = self.simple_query()
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    self.assertEqual(len(documents), 0)

  def test_magic_number_mismatched(self):
    magic_number = 0
    batch_count = 5
    forwards=[0.234, 'abc']
    req = self.create_request(self.collection_name, magic_number,
                              batch_count,
                              OperationType.INSERT,
                              forwards=forwards)
    logging.info("request: %s", req)
    response = self.client.write(req)
    logging.info("process result: %s", response)
    self.assertEqual(response.code, -2021)
    self.assertEqual(response.reason, "Mismatched Magic Number")

  # def test_revision_mismatched(self):
  #   response = self.client.get_collection(self.collection_name)
  #   logging.info("get collection result: %s", response)
  #   magic_number = response.entity.magic_number
  #   batch_count = 5
  #   forwards=[0.234, 'abc']
  #   req = self.create_request(self.collection_name, magic_number,
  #                             batch_count,
  #                             OperationType.INSERT,
  #                             schema_revision=0,
  #                             forwards=forwards)
  #   logging.info("request: %s", req)
  #   response = self.client.write(req)
  #   logging.info("process result: %s", response)
  #   time.sleep(1)

  #   # update collection
  #   self.creator.update_schema(self.schema, self.collection_meta)
  #   status, data = self.admin_client.update_collection(self.schema)
  #   logging.info("update schema: %s", self.schema)
  #   logging.info("update result: %s", data)
  #   self.assertEqual(status, 200)
  #   self.assertEqual(data["code"], 0)

  #   req = self.create_request(self.collection_name, magic_number,
  #                             batch_count,
  #                             OperationType.UPDATE,
  #                             schema_revision=0,
  #                             forwards=forwards)
  #   logging.info("request: %s", req)
  #   response = self.client.write(req)
  #   logging.info("process result: %s", response)
  #   self.assertEqual(response.code, -4000)
  #   self.assertEqual(response.reason, "Collection schema revision mismatched")

  #   # get revision
  #   response = self.client.get_collection(self.collection_name)
  #   logging.info("get collection result: %s", response)
  #   revision = response.entity.config.schema_revision

  #   req = self.create_request(self.collection_name, magic_number,
  #                             batch_count,
  #                             OperationType.UPDATE,
  #                             schema_revision=revision,
  #                             forwards=forwards)
  #   logging.info("request: %s", req)
  #   response = self.client.write(req)
  #   logging.info("process result: %s", response)
  #   self.assertEqual(response.code, 0)

  def test_restart_server_with_not_empty_data_flow(self):
    # 1 send request
    magic_number = self.get_magic_number(self.collection_name)
    batch_count = 64
    forwards=[0.234, 'abc']
    req = self.create_request(self.collection_name, magic_number,
                              batch_count, OperationType.INSERT,
                              forwards=forwards)
    logging.info("request: %s", req)
    response = self.client.write(req)
    logging.info("process result: %s", response)
    self.assertEqual(response.code, 0)

    time.sleep(2)

    # 2 query result
    status, response = self.simple_query()
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    self.assertEqual(len(documents), 10)
    for i in range(0, len(documents)):
      self.assertEqual(documents[i].primary_key, i + 1)
      self.assertEqual(documents[i].score, 1.0 + i * i)
      self.assertEqual(len(documents[i].forward_column_values), 2)
    self.assertAlmostEqual(documents[i].forward_column_values['col_a'], 1.234 + i, delta=0.000001)
    self.assertEqual(documents[i].forward_column_values['col_b'], 'abc')

    # 3 restart proxima be
    self.server_utils.stop_proxima_be('SIGUSR1')
    self.server_utils.start_proxima_be()

    time.sleep(2)

    # 4 requery the result
    status, response = self.simple_query()
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    self.assertEqual(len(documents), 10)
    for i in range(0, len(documents)):
      self.assertEqual(documents[i].primary_key, i + 1)
      self.assertEqual(documents[i].score, 1.0 + i * i)
      self.assertEqual(len(documents[i].forward_column_values), 2)
    self.assertAlmostEqual(documents[i].forward_column_values['col_a'], 1.234 + i, delta=0.000001)
    self.assertEqual(documents[i].forward_column_values['col_b'], 'abc')

    # 5 send new data request
    magic_number = self.get_magic_number(self.collection_name)

    batch_count = 1
    req = self.create_request(self.collection_name, magic_number,
                              batch_count, OperationType.INSERT,
                              forwards=forwards,
                              index_value_base=65)
    logging.info("request: %s", req)
    response = self.client.write(req)
    logging.info("process result: %s", response)
    self.assertEqual(response.code, 0)

    time.sleep(1)

    # 5 query the result
    topk = 65
    status, response = self.simple_query(topk=topk)
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    self.assertEqual(len(documents), topk)

  def test_restart_server_with_empty_data_flow(self):
    # 1 create new collection
    schema = self.create_schema(self.collection_name2,
                                self.index_columns, self.index_dimensions)
    status= self.client.create_collection(schema)
    self.assertTrue(status.ok())

    # 2 query result
    status, response = self.simple_query()
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    self.assertEqual(len(documents), 0)

    time.sleep(5)

    # 3 restart proxima be
    self.server_utils.stop_proxima_be('SIGUSR1')
    self.server_utils.start_proxima_be()

    time.sleep(2)

    # 4 requery the result
    status, response = self.simple_query()
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    self.assertEqual(len(documents), 0)

    # 5 send dataset request
    magic_number = self.get_magic_number(self.collection_name)
    batch_count = 20
    forwards=[0.234, 'abc']
    req = self.create_request(self.collection_name, magic_number,
                              batch_count, OperationType.INSERT,
                              forwards=forwards)
    logging.info("request: %s", req)
    response = self.client.write(req)
    logging.info("process result: %s", response)
    self.assertEqual(response.code, 0)

    time.sleep(1)

    # 5 query the result
    topk = 30
    status, response = self.simple_query(topk=topk)
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    self.assertEqual(len(documents), batch_count)


if __name__ == '__main__':
  unittest.main()
