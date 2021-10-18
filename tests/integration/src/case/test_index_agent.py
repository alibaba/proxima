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
import random

from pyproximabe import *
from global_conf import GlobalConf
from collection_creator import CollectionCreator
import client_helper

OperationType = WriteRequest.OperationType

class TestIndexAgentBase(unittest.TestCase):
  def setUp(self):
    self.global_conf = GlobalConf()
    self.creator = CollectionCreator()
    self.client = client_helper.get_client(self.global_conf)

    self.collection_name = "collection1"
    self.collection_name2 = "collection2"
    self.repository_name = "test_repo"

    self.clean_env()

    self.index_columns = ["column1"]
    self.index_dimensions = [16]
    self.schema = self.create_schema(self.collection_name, self.index_columns,
                                     self.index_dimensions,
                                     with_repo=self.with_repo)
    status = self.client.create_collection(self.schema)
    self.assertTrue(status.ok())
    self.connection = self.creator.get_connection()

  def tearDown(self):
    self.clean_env()

  def reconnect(self):
    self.client = client_helper.get_client(self.global_conf)
  def clean_env(self):
    status, collections = self.client.list_collections()
    self.assertTrue(status.ok())
    for collection in collections:
      status = self.client.drop_collection(collection.collection_config.collection_name)
      self.assertTrue(status.ok())

  def create_schema(self, collection_name, column_name, dims,
                    forward_columns=["col_a", "col_b"],
                    repository_name="test_repo",
                    with_repo=True):
    return self.creator.create_schema(collection_name,
                                      repository_table="test_collection",
                                      repository_name=repository_name,
                                      forward_columns=forward_columns,
                                      index_columns=column_name,
                                      index_dimensions=dims,
                                      db_name="test_db",
                                      with_repo=with_repo)

  def create_schema1(self, collection_name, index_columns=None,
                    dimensions=None, forward_columns=None,
                     with_repo=True):
    return self.creator.create_schema(collection_name,
                                      repository_table="test_collection",
                                      repository_name="test_repo",
                                      forward_columns=forward_columns,
                                      index_columns=index_columns,
                                      index_dimensions=dimensions,
                                      db_name="test_db",
                                      with_repo=with_repo)

  def create_all_index_data_types_schema(self, collection_name, dim,
                                         with_repo=True):
    index_data_types = [DataType.VECTOR_FP32,
                        DataType.VECTOR_FP16,
                        DataType.VECTOR_INT8,
                        DataType.VECTOR_INT4,
                        DataType.VECTOR_BINARY32,
                        DataType.VECTOR_BINARY64]
    index_columns = []
    index_dimensions = []
    for i in range(0, len(index_data_types)):
      index_columns.append('column_' + str(i))
      index_dimensions.append(dim)
    return self.creator.create_schema(collection_name,
                                      repository_table="test_collection",
                                      repository_name="test_repo",
                                      forward_columns=["col_a", "col_b"],
                                      index_columns=index_columns,
                                      index_data_types=index_data_types,
                                      index_dimensions=index_dimensions,
                                      db_name="test_db",
                                      with_repo=with_repo)

  def create_all_forward_data_types_schema(self, collection_name, dim,
                                           with_repo=True):
    index_data_types = [DataType.VECTOR_FP32]
    index_columns = ['column_0']
    index_dimensions = [dim]
    forward_columns = []
    forward_cnt = 9
    for i in range(0, forward_cnt):
      forward_columns.append('forward_' + str(i))

    return self.creator.create_schema(collection_name,
                                      repository_table="test_collection",
                                      repository_name="test_repo",
                                      forward_columns=forward_columns,
                                      index_columns=index_columns,
                                      index_data_types=index_data_types,
                                      index_dimensions=index_dimensions,
                                      db_name="test_db",
                                      with_repo=with_repo)

  def create_single_request(self, magic_number, operation_type,
                            forwards = [1.234, 'abc'], lsn=10, is_bytes=False, is_vector=False):
    index_tuple_metas = [['column1', DataType.VECTOR_FP32, self.index_dimensions[0]]]
    index_tuple_types = ['string']
    forward_tuple_names = ['col_a', 'col_b']
    forward_tuple_types = [DataType.FLOAT,
                           DataType.STRING]
    if not is_bytes:
      feature = "[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2]"
    else:
      vector = [1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2]
      values = []
      for value in vector:
        values.append(struct.pack('f', value))
      feature = b''.join(values)
    if is_vector:
        feature = [1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2]
    rows = [[1, operation_type, lsn, feature,
             forwards[0], forwards[1]]
      ]
    return self.creator.create_dataset_request(self.collection_name,
                                               magic_number,
                                               index_tuple_metas = index_tuple_metas,
                                               index_tuple_types = index_tuple_types,
                                               forward_tuple_names = forward_tuple_names,
                                               forward_tuple_types = forward_tuple_types,
                                               rows = rows,
                                               with_repo=self.with_repo)

  def create_batch_request(self, magic_number, count, operation_type,
                           forwards=[0.234, 'abc'],
                           lsn = 10,
                           index_value_base = 0):
    index_tuple_metas = [['column1', DataType.VECTOR_FP32, self.index_dimensions[0]]]
    index_tuple_types = ['string']
    rows = []
    forward_tuple_names=['col_a', 'col_b']
    forward_tuple_types=[DataType.FLOAT,
                         DataType.STRING]
    for i in range(1, count + 1):
      rows.append([i, operation_type, lsn + i,
                   '[' + str(i + index_value_base) + ",1,1,1,1,1,1,1,2,2,2,2,2,2,2,2]",
                   forwards[0] + i, forwards[1]])

    return self.creator.create_dataset_request(self.collection_name,
                                               magic_number,
                                               index_tuple_metas = index_tuple_metas,
                                               index_tuple_types = index_tuple_types,
                                               forward_tuple_names = forward_tuple_names,
                                               forward_tuple_types = forward_tuple_types,
                                               rows = rows,
                                               with_repo=self.with_repo)

  def create_one_forward_request(self, collection,
                                 magic_number, count, operation_type,
                                 forwards=[0.234],
                                 lsn = 10,
                                 index_value_base = 0):
    index_tuple_metas = [['column1', DataType.VECTOR_FP32, self.index_dimensions[0]]]
    index_tuple_types = ['string']
    rows = []
    forward_tuple_names=['col_a']
    forward_tuple_types=[DataType.FLOAT]
    for i in range(1, count + 1):
      rows.append([i, operation_type, lsn + i,
                   '[' + str(i + index_value_base) + ",1,1,1,1,1,1,1,2,2,2,2,2,2,2,2]",
                   forwards[0] + i])

    return self.creator.create_dataset_request(collection,
                                               magic_number,
                                               index_tuple_metas = index_tuple_metas,
                                               index_tuple_types = index_tuple_types,
                                               forward_tuple_names = forward_tuple_names,
                                               forward_tuple_types = forward_tuple_types,
                                               rows = rows,
                                               with_repo=self.with_repo)

  def create_multi_index_request(self, collection, dim,
                                 magic_number, count, operation_type,
                                 lsn = 10,
                                 index_value_base = 0):
    index_tuple_metas = [['index2', DataType.VECTOR_FP32, dim],
                         ['index1', DataType.VECTOR_FP32, dim]]
    index_tuple_names = ['index2', 'index1']
    index_tuple_types = ['string', 'string']
    rows = []
    forward_tuple_names=['col_a', 'index1']
    forward_tuple_types=[DataType.FLOAT, DataType.STRING]
    for i in range(1, count + 1):
      rows.append([i, operation_type, lsn + i,
                   '[' + str(i + index_value_base) + ",1,1,1,1,1,1,1,2,2,2,2,2,2,2,2]",
                   '[' + str(i + index_value_base) + ",1,1,1,1,1,1,1,2,2,2,2,2,2,2,2]",
                   0.234 + i,
                  '[' + str(i + index_value_base) + ",1,1,1,1,1,1,1,2,2,2,2,2,2,2,2]"])

    return self.creator.create_dataset_request(collection,
                                               magic_number,
                                               index_tuple_metas = index_tuple_metas,
                                               index_tuple_types = index_tuple_types,
                                               forward_tuple_names = forward_tuple_names,
                                               forward_tuple_types = forward_tuple_types,
                                               rows = rows,
                                               with_repo=self.with_repo)

  def create_all_operations_request(self,
                                    magic_number,
                                    count,
                                    operation_types,
                                    forwards=[0.234, 'abc'],
                                    lsn = 10,
                                    index_value_base = 0):
    index_tuple_metas = [['column1', DataType.VECTOR_FP32, self.index_dimensions[0]]]
    index_tuple_types = ['string']
    forward_tuple_names = ['col_a', 'col_b']
    forward_tuple_types = [DataType.FLOAT, DataType.STRING]

    rows = []
    lsn = 1
    for i in range(1, count + 1):
      idx = 1
      for operation_type in operation_types:
        rows.append([i, operation_type, lsn,
                     '[' + str(i + index_value_base) + ",1,1,1,1,1,1,1,2,2,2,2,2,2,2,2]",
                     forwards[0] + idx, forwards[1]])
        idx += 1
        lsn += 1

    return self.creator.create_dataset_request(self.collection_name,
                                               magic_number,
                                               index_tuple_metas = index_tuple_metas,
                                               index_tuple_types = index_tuple_types,
                                               forward_tuple_names = forward_tuple_names,
                                               forward_tuple_types = forward_tuple_types,
                                               rows = rows,
                                               with_repo=self.with_repo)

  def create_all_index_data_types_insert_request(self, magic_number, dim, count):
    index_tuple_metas = []
    index_tuple_types = []
    index_data_types = [DataType.VECTOR_FP32,
                        DataType.VECTOR_FP16,
                        DataType.VECTOR_INT8,
                        DataType.VECTOR_INT4,
                        DataType.VECTOR_BINARY32,
                        DataType.VECTOR_BINARY64]
    total_types = 6
    for i in range(0, total_types):
      index_tuple_metas.append(['column_' + str(i), index_data_types[i], dim])
    forward_tuple_names = ['col_a', 'col_b']
    forward_tuple_types = [DataType.FLOAT, DataType.STRING]
    rows = []
    for i in range(1, count + 1):
      row = [i, OperationType.INSERT, 9 + i]
      for j in range(0, total_types):
        vec = str(i)
        if j == 5:
          vec = str(i)
        elif j == 4:
          vec = str(i) + ',' + str(i)
        else:
          for k in range(1, dim):
            vec += ',' + str(i)
        row.append('[' + vec + ']')
      row.append(0.234 + i)
      row.append('abc')
      rows.append(row)

    return self.creator.create_dataset_request(self.collection_name2,
                                               magic_number,
                                               index_tuple_metas = index_tuple_metas,
                                               index_tuple_types = None,
                                               forward_tuple_names = forward_tuple_names,
                                               forward_tuple_types = forward_tuple_types,
                                               rows = rows,
                                               with_repo=self.with_repo)

  def create_all_forward_data_types_insert_request(self, magic_number, dim, count):
    index_tuple_metas = [['column_0', DataType.VECTOR_FP32, dim]]
    index_tuple_types = ['string']
    total_types = 9
    forward_tuple_names = []
    forward_tuple_types = [DataType.BINARY,
                           DataType.STRING,
                           DataType.BOOL,
                           DataType.INT32,
                           DataType.INT64,
                           DataType.UINT32,
                           DataType.UINT64,
                           DataType.FLOAT,
                           DataType.DOUBLE]
    for i in range(0, total_types):
      forward_tuple_names.append('forward_' + str(i))
    rows = []
    for i in range(1, count + 1):
      row = [i, OperationType.INSERT, 9 + i]
      vec = str(i)
      for k in range(1, dim):
        vec += ',' + str(i)
      row.append('[' + vec + ']')
      row.append(str(i).encode('UTF-8'))
      row.append(str(i) * i)
      if i % 2 == 1:
        row.append(True)
      else:
        row.append(False)
      row.append(i)
      row.append(i * 10)
      row.append(i * 100)
      row.append(i * 1000)
      row.append(i * 1.0)
      row.append(i * 10.0)
      rows.append(row)

    return self.creator.create_dataset_request(self.collection_name2,
                                               magic_number,
                                               index_tuple_metas = index_tuple_metas,
                                               index_tuple_types = index_tuple_types,
                                               forward_tuple_names = forward_tuple_names,
                                               forward_tuple_types = forward_tuple_types,
                                               rows = rows,
                                               with_repo=self.with_repo)

  def create_request(self, collection_name,
                     magic_number, count, operation_type=None,
                     operation_types=None,
                     forward_tuple_names=['col_a', 'col_b'],
                     forward_tuple_types=None,
                     forwards=[0.234, 'abc'],
                     index_tuple_metas = None,
                     index_tuple_types = ['string'],
                     lsn = 10, index_value_base = 0,
                     vector_exception=False,
                     key_repeated=False,
                     empty_request=False):
    rows = []
    if not index_tuple_metas:
      index_tuple_metas = [['column1', DataType.VECTOR_FP32, self.index_dimensions[0]]]
    if not forward_tuple_types:
      forward_tuple_types=[DataType.FLOAT,  DataType.STRING]
    index_num = len(index_tuple_metas)
    index_types = []
    if index_tuple_types:
      index_types = index_tuple_types
    if not operation_types:
      operation_types = []
      for i in range(1, count + 1):
        operation_types.append(operation_type)

    for i in range(1, count + 1):
      row = []
      if not key_repeated:
        row = [i + index_value_base, operation_types[i - 1], lsn + i]
      else:
        row = [1, operation_types[i - 1], lsn + i]
      for j in range(0, index_num):
        vector = str(i + index_value_base) + ",1,1,1,1,1,1,1,2,2,2,2,2,2,2,2"
        if vector_exception:
          vector += "1,1,1"
        row.append('[' + vector + ']')
      row.append(forwards[0] + i)
      row.append(forwards[1])
      if not empty_request:
        rows.append(row)

    return self.creator.create_dataset_request(collection_name,
                                               magic_number,
                                               index_tuple_metas = index_tuple_metas,
                                               index_tuple_types = index_types,
                                               forward_tuple_names = forward_tuple_names,
                                               forward_tuple_types = forward_tuple_types,
                                               rows = rows)

  def simple_query(self, topk=10):
    features = [[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,3]]
    return self.client.query(self.collection_name,
                             'column1',
                             features,
                             data_type=DataType.VECTOR_FP32,
                             dimension=16,
                             batch_count=1,
                             topk=topk)

  def query(self, index_column, topk, dim, feature_type):
    features = []
    if feature_type == DataType.VECTOR_INT4:
      dim /= 2
    elif feature_type == DataType.VECTOR_BINARY32:
      dim /= 32
    elif feature_type == DataType.VECTOR_BINARY64:
      dim /= 64
    for i in range(0, int(dim)):
      features.append(1)
    # for f in features:
    #     if feature_type == common_pb2.FeatureType.FT_FP32:
    #       fea_bytes += struct.pack('f', f)
    #     elif feature_type == common_pb2.FeatureType.FT_FP16:
    #       fea_bytes += struct.pack('h', 0)
    #     elif feature_type == common_pb2.FeatureType.FT_INT8:
    #       fea_bytes += struct.pack('b', f)
    #     elif feature_type == common_pb2.FeatureType.FT_INT4:
    #       fea_bytes += struct.pack('b', 17)
    #     elif feature_type == common_pb2.FeatureType.FT_BINARY32:
    #       fea_bytes += struct.pack('I', f)
    #     elif feature_type == common_pb2.FeatureType.FT_BINARY64:
    #       fea_bytes += struct.pack('L', f)

    return self.client.query(self.collection_name2,
                             index_column,
                             features,
                             data_type=feature_type,
                             dimension=dim,
                             batch_count=1,
                             topk=topk)

  def get_magic_number(self, collection_name):
    status, collection = self.client.describe_collection(collection_name)
    self.assertTrue(status.ok())
    return collection.magic_number

class TestIndexAgentDatabase(TestIndexAgentBase):
  def setUp(self):
    self.with_repo = True
    super().setUp()

  def test_single_insert(self):
    magic_number = self.get_magic_number(self.collection_name)
    req = self.create_single_request(magic_number, OperationType.INSERT)
    logging.info("request: %s", req)
    response = self.client.write(req)
    logging.info("process result: %s", response)
    self.assertTrue(response.ok())

    time.sleep(1)

    status, response = self.simple_query()
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    self.assertEqual(len(documents), 1)
    self.assertEqual(documents[0].primary_key, 1)
    self.assertEqual(documents[0].score, 1.0)
    self.assertEqual(len(documents[0].forward_column_values), 2)
    self.assertAlmostEqual(documents[0].forward_column_values['col_a'], 1.234, delta=0.000001)
    self.assertEqual(documents[0].forward_column_values['col_b'], 'abc')

  def test_single_insert_with_bytes(self):
    magic_number = self.get_magic_number(self.collection_name)
    req = self.create_single_request(magic_number,
                                     OperationType.INSERT, is_bytes=True)
    logging.info("request: %s", req)
    response = self.client.write(req)
    logging.info("process result: %s", response)
    self.assertTrue(response.ok())

    time.sleep(1)

    status, response = self.simple_query()
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    self.assertEqual(len(documents), 1)
    self.assertEqual(documents[0].primary_key, 1)
    self.assertEqual(documents[0].score, 1.0)
    self.assertEqual(len(documents[0].forward_column_values), 2)
    self.assertAlmostEqual(documents[0].forward_column_values['col_a'], 1.234, delta=0.000001)
    self.assertEqual(documents[0].forward_column_values['col_b'], 'abc')

  def test_single_insert_with_vector(self):
      magic_number = self.get_magic_number(self.collection_name)
      req = self.create_single_request(magic_number,
                                       OperationType.INSERT, is_vector=True)
      logging.info("request: %s", req)
      response = self.client.write(req)
      logging.info("process result: %s", response)
      self.assertTrue(response.ok())

      time.sleep(1)

      status, response = self.simple_query()
      self.assertTrue(status.ok())
      logging.info("query result: %s", response)
      results = response.results
      self.assertEqual(len(results), 1)
      documents = results[0]
      self.assertEqual(len(documents), 1)
      self.assertEqual(documents[0].primary_key, 1)
      self.assertEqual(documents[0].score, 1.0)
      self.assertEqual(len(documents[0].forward_column_values), 2)
      self.assertAlmostEqual(documents[0].forward_column_values['col_a'], 1.234, delta=0.000001)
      self.assertEqual(documents[0].forward_column_values['col_b'], 'abc')

  def test_batch_insert(self):
    magic_number = self.get_magic_number(self.collection_name)
    batch_count = 2
    req = self.create_batch_request(magic_number, batch_count, OperationType.INSERT)
    logging.info("request: %s", req)
    response = self.client.write(req)
    self.assertTrue(response.ok())

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

  def test_insert_with_all_index_data_types(self):
    dim = 64
    new_schema = self.create_all_index_data_types_schema(self.collection_name2, dim)
    logging.info("new schema: %s", new_schema)
    status = self.client.create_collection(new_schema)
    logging.info("create collection result: %s", status)
    self.assertTrue(status.ok())

    magic_number = self.get_magic_number(self.collection_name2)
    batch_count = 6
    req = self.create_all_index_data_types_insert_request(magic_number,
                                                          dim, batch_count)
    logging.info("request: %s", req)
    response = self.client.write(req)
    logging.info("process result: %s", response)
    self.assertTrue(response.ok())

    time.sleep(1)

    topk = 5
    # data type fp32
    status,response = self.query("column_0", topk, dim, DataType.VECTOR_FP32)
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    self.assertEqual(len(documents), topk)
    self.assertEqual(documents[0].primary_key, 1)
    self.assertEqual(documents[0].score, 0.0)
    self.assertEqual(len(documents[0].forward_column_values), 2)
    self.assertEqual(documents[1].primary_key, 2)
    self.assertEqual(documents[1].score, 64.0)
    self.assertEqual(documents[2].primary_key, 3)
    self.assertEqual(documents[2].score, 256.0)
    self.assertEqual(documents[3].primary_key, 4)
    self.assertEqual(documents[3].score, 576.0)
    self.assertEqual(documents[4].primary_key, 5)
    self.assertEqual(documents[4].score, 1024.0)

    # # data type fp16 client not support
    # status,response = self.query("column_1", topk, dim, DataType.VECTOR_FP16)
    # self.assertTrue(status.ok())
    # logging.info("query result: %s", response)
    # results = response.results
    # self.assertEqual(len(results), 1)
    # documents = results[0]
    # self.assertEqual(len(documents), topk)
    # self.assertEqual(documents[0].primary_key, 1)
    # self.assertEqual(documents[0].score, 64)
    # self.assertEqual(len(documents[0].forward_column_values), 2)
    # self.assertAlmostEqual(documents[0].forward_column_values['col_a'], 1.234, delta=0.000001)
    # self.assertEqual(documents[1].forward_column_values['col_b'], 'abc')
    # self.assertEqual(documents[1].primary_key, 2)
    # self.assertEqual(documents[1].score, 256.0)
    # self.assertEqual(documents[2].primary_key, 3)
    # self.assertEqual(documents[2].score, 576.0)
    # self.assertEqual(documents[3].primary_key, 4)
    # self.assertEqual(documents[3].score, 1024)
    # self.assertEqual(documents[4].primary_key, 5)
    # self.assertEqual(documents[4].score, 1600.0)

    # # data type int8 client not support
    # status,response = self.query("column_2", topk, dim, DataType.VECTOR_INT8)
    # self.assertTrue(status.ok())
    # logging.info("query result: %s", response)
    # results = response.results
    # self.assertEqual(len(results), 1)
    # documents = results[0]
    # self.assertEqual(len(documents), topk)
    # self.assertEqual(documents[0].primary_key, 1)
    # self.assertEqual(documents[0].score, 0)
    # self.assertEqual(len(documents[0].forward_column_values), 2)
    # self.assertEqual(documents[1].primary_key, 2)
    # self.assertEqual(documents[1].score, 64.0)
    # self.assertEqual(documents[2].primary_key, 3)
    # self.assertEqual(documents[2].score, 256.0)
    # self.assertEqual(documents[3].primary_key, 4)
    # self.assertEqual(documents[3].score, 576.0)
    # self.assertEqual(documents[4].primary_key, 5)
    # self.assertEqual(documents[4].score, 1024.0)

    # # data type int4
    # query = self.create_query("column_3", topk, dim, DataType.INT4)
    # logging.info("query %s", query)
    # response = self.client.query(query)
    # logging.info("query result: %s", response)
    # self.assertEqual(response.code, 0)
    # documents = response.entity[0]
    # self.assertEqual(len(documents), topk)
    # self.assertEqual(documents[0].primary_key, 1)
    # self.assertEqual(documents[0].score, 0)
    # self.assertEqual(len(documents[0].forward), 2)
    # self.assertEqual(documents[0].forward[0].key, 'col_a')
    # self.assertAlmostEqual(documents[0].forward[0].value.float_value, 1.234)
    # self.assertEqual(documents[0].forward[1].key, 'col_b')
    # self.assertEqual(documents[0].forward[1].value.bytes_value, 'abc'.encode('UTF-8'))

    # self.assertEqual(documents[1].primary_key, 2)
    # self.assertEqual(documents[1].score, 64.0)
    # self.assertEqual(documents[2].primary_key, 3)
    # self.assertEqual(documents[2].score, 256.0)
    # self.assertEqual(documents[3].primary_key, 4)
    # self.assertEqual(documents[3].score, 576.0)
    # self.assertEqual(documents[4].primary_key, 5)
    # self.assertEqual(documents[4].score, 1024.0)

    # # data type binary32
    # query = self.create_query("column_4", topk, dim, DataType.BINARY32)
    # logging.info("query %s", query)
    # response = self.client.query(query)
    # logging.info("query result: %s", response)
    # self.assertEqual(response.code, 0)
    # documents = response.entity[0]
    # self.assertEqual(len(documents), topk)
    # self.assertEqual(documents[0].primary_key, 1)
    # self.assertEqual(documents[0].score, 0)
    # self.assertEqual(len(documents[0].forward), 2)
    # self.assertEqual(documents[0].forward[0].key, 'col_a')
    # self.assertAlmostEqual(documents[0].forward[0].value.float_value, 1.234)
    # self.assertEqual(documents[0].forward[1].key, 'col_b')
    # self.assertEqual(documents[0].forward[1].value.bytes_value, 'abc'.encode('UTF-8'))

    # self.assertEqual(documents[1].score, 2.0)
    # self.assertEqual(documents[2].score, 2.0)
    # self.assertEqual(documents[3].score, 4.0)
    # self.assertEqual(documents[4].score, 4.0)

    # # data type binary64
    # query = self.create_query("column_5", topk, dim, DataType.BINARY64)
    # logging.info("query %s", query)
    # response = self.client.query(query)
    # logging.info("query result: %s", response)
    # self.assertEqual(response.code, 0)
    # documents = response.entity[0]
    # self.assertEqual(len(documents), topk)
    # self.assertEqual(documents[0].primary_key, 1)
    # self.assertEqual(documents[0].score, 0)
    # self.assertEqual(len(documents[0].forward), 2)
    # self.assertEqual(documents[0].forward[0].key, 'col_a')
    # self.assertAlmostEqual(documents[0].forward[0].value.float_value, 1.234)
    # self.assertEqual(documents[0].forward[1].key, 'col_b')
    # self.assertEqual(documents[0].forward[1].value.bytes_value, 'abc'.encode('UTF-8'))

    # self.assertEqual(documents[1].score, 1.0)
    # self.assertEqual(documents[2].score, 1.0)
    # self.assertEqual(documents[3].score, 2.0)
    # self.assertEqual(documents[4].score, 2.0)

  def test_insert_with_all_forward_data_types(self):
    dim = 64
    new_schema = self.create_all_forward_data_types_schema(self.collection_name2, dim)
    logging.info("new schema: %s", new_schema)
    status = self.client.create_collection(new_schema)
    self.assertTrue(status.ok())

    magic_number = self.get_magic_number(self.collection_name2)
    batch_count = 6
    req = self.create_all_forward_data_types_insert_request(magic_number,
                                                            dim, batch_count)
    logging.info("request: %s", req)
    response = self.client.write(req)
    self.assertTrue(status.ok())

    time.sleep(1)

    topk = 5
    status,response = self.query("column_0", topk, dim, DataType.VECTOR_FP32)
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    self.assertEqual(len(documents), topk)
    self.assertEqual(documents[0].primary_key, 1)
    self.assertEqual(documents[0].score, 0.0)
    self.assertEqual(len(documents[0].forward_column_values), 9)
    self.assertEqual(documents[0].forward_column_values['forward_0'], '1'.encode('UTF-8'))
    self.assertEqual(documents[0].forward_column_values['forward_1'], '1')
    self.assertEqual(documents[0].forward_column_values['forward_2'], True)
    self.assertEqual(documents[0].forward_column_values['forward_3'], 1)
    self.assertEqual(documents[0].forward_column_values['forward_4'], 10)
    self.assertEqual(documents[0].forward_column_values['forward_5'], 100)
    self.assertEqual(documents[0].forward_column_values['forward_6'], 1000)
    self.assertEqual(documents[0].forward_column_values['forward_7'], 1.0)
    self.assertEqual(documents[0].forward_column_values['forward_8'], 10.0)

    self.assertEqual(documents[1].primary_key, 2)
    self.assertEqual(documents[1].score, 64.0)
    self.assertEqual(documents[2].primary_key, 3)
    self.assertEqual(documents[2].score, 256.0)
    self.assertEqual(documents[3].primary_key, 4)
    self.assertEqual(documents[3].score, 576.0)
    self.assertEqual(documents[4].primary_key, 5)
    self.assertEqual(documents[4].score, 1024.0)

  def test_single_update(self):
    magic_number = self.get_magic_number(self.collection_name)
    forwards = [1.234, 'abc']
    req = self.create_single_request(magic_number, OperationType.INSERT, forwards = forwards, lsn = 10)
    response = self.client.write(req)
    self.assertTrue(response.ok())

    forwards = [2.234, 'def']
    req = self.create_single_request(magic_number, OperationType.UPDATE, forwards = forwards, lsn = 11)
    logging.info("req: %s", req)
    response = self.client.write(req)
    self.assertTrue(response.ok())

    time.sleep(1)

    status, response = self.simple_query()
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    self.assertEqual(len(documents), 1)
    self.assertEqual(documents[0].primary_key, 1)
    self.assertEqual(documents[0].score, 1.0)
    self.assertEqual(len(documents[0].forward_column_values), 2)
    self.assertAlmostEqual(documents[0].forward_column_values['col_a'], 2.234, delta=0.000001)
    self.assertEqual(documents[0].forward_column_values['col_b'], 'def')

  def test_single_update_with_bytes(self):
    magic_number = self.get_magic_number(self.collection_name)
    forwards = [1.234, 'abc']
    req = self.create_single_request(magic_number, OperationType.INSERT,
                                     forwards = forwards,
                                     lsn = 10, is_bytes = True)
    response = self.client.write(req)
    self.assertTrue(response.ok())

    forwards = [2.234, 'def']
    req = self.create_single_request(magic_number,
                                     OperationType.UPDATE,
                                     forwards = forwards, lsn = 11, is_bytes = True)
    logging.info("req: %s", req)
    response = self.client.write(req)
    self.assertTrue(response.ok())

    time.sleep(1)

    status, response = self.simple_query()
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    self.assertEqual(len(documents), 1)
    self.assertEqual(documents[0].primary_key, 1)
    self.assertEqual(documents[0].score, 1.0)
    self.assertEqual(len(documents[0].forward_column_values), 2)
    self.assertAlmostEqual(documents[0].forward_column_values['col_a'], 2.234, delta=0.000001)
    self.assertEqual(documents[0].forward_column_values['col_b'], 'def')

  def test_batch_update(self):
    magic_number = self.get_magic_number(self.collection_name)
    batch_count = 6
    forwards = [0.234, 'abc']
    req = self.create_batch_request(magic_number, batch_count,
                                    OperationType.INSERT,
                                    forwards = forwards,
                                    lsn = 10)
    logging.info("request: %s", req)
    response = self.client.write(req)
    self.assertTrue(response.ok())

    forwards = [1.234, 'def']
    req = self.create_batch_request(magic_number, batch_count,
                                    OperationType.UPDATE,
                                    forwards = forwards,
                                    lsn = 20)
    logging.info("request: %s", req)
    response = self.client.write(req)
    self.assertTrue(response.ok())

    time.sleep(1)

    status, response = self.simple_query()
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    self.assertEqual(len(documents), 6)
    self.assertEqual(documents[0].primary_key, 1)
    self.assertEqual(documents[0].score, 1.0)
    self.assertEqual(len(documents[0].forward_column_values), 2)
    self.assertAlmostEqual(documents[0].forward_column_values['col_a'], 2.234, delta=0.000001)
    self.assertEqual(documents[0].forward_column_values['col_b'], 'def')
    self.assertEqual(documents[1].primary_key, 2)
    self.assertEqual(documents[1].score, 2.0)
    self.assertEqual(documents[2].primary_key, 3)
    self.assertEqual(documents[2].score, 5.0)
    self.assertEqual(documents[3].primary_key, 4)
    self.assertEqual(documents[3].score, 10.0)
    self.assertEqual(documents[4].primary_key, 5)
    self.assertEqual(documents[4].score, 17.0)
    self.assertEqual(documents[5].primary_key, 6)
    self.assertEqual(documents[5].score, 26.0)
    self.assertEqual(len(documents[5].forward_column_values), 2)
    self.assertAlmostEqual(documents[5].forward_column_values['col_a'], 7.234, delta=0.000001)
    self.assertEqual(documents[5].forward_column_values['col_b'], 'def')

  def test_update_only_index_column(self):
    magic_number = self.get_magic_number(self.collection_name)
    batch_count = 2
    forwards = [0.234, 'abc']
    req = self.create_batch_request(magic_number, batch_count,
                                    OperationType.INSERT,
                                    forwards = forwards,
                                    lsn = 10,
                                    index_value_base = 0)
    logging.info("request: %s", req)
    response = self.client.write(req)
    self.assertTrue(response.ok())

    req = self.create_batch_request(magic_number, batch_count,
                                    OperationType.UPDATE,
                                    forwards = forwards,
                                    lsn = 20,
                                    index_value_base = 10)
    logging.info("request: %s", req)
    response = self.client.write(req)
    self.assertTrue(response.ok())

    time.sleep(1)

    status, response = self.simple_query()
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    self.assertEqual(len(documents), 2)
    self.assertEqual(documents[0].primary_key, 1)
    self.assertEqual(documents[0].score, 101.0)
    self.assertEqual(len(documents[0].forward_column_values), 2)
    self.assertAlmostEqual(documents[0].forward_column_values['col_a'], 1.234, delta=0.000001)
    self.assertEqual(documents[0].forward_column_values['col_b'], 'abc')
    self.assertEqual(documents[1].primary_key, 2)
    self.assertEqual(documents[1].score, 122.0)
    self.assertEqual(len(documents[1].forward_column_values), 2)
    self.assertAlmostEqual(documents[1].forward_column_values['col_a'], 2.234, delta=0.000001)
    self.assertEqual(documents[1].forward_column_values['col_b'], 'abc')

  def test_update_both_index_and_forward_column(self):
    magic_number = self.get_magic_number(self.collection_name)
    batch_count = 2
    forwards = [0.234, 'abc']
    req = self.create_batch_request(magic_number, batch_count,
                                    OperationType.INSERT,
                                    forwards = forwards,
                                    lsn = 10,
                                    index_value_base = 0)
    logging.info("request: %s", req)
    response = self.client.write(req)
    self.assertTrue(response.ok())

    forwards = [1.234, 'def']
    req = self.create_batch_request(magic_number, batch_count,
                                    OperationType.UPDATE,
                                    forwards = forwards,
                                    lsn = 20,
                                    index_value_base = 10)
    logging.info("request: %s", req)
    response = self.client.write(req)
    self.assertTrue(response.ok())

    time.sleep(1)

    status, response = self.simple_query()
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    self.assertEqual(len(documents), 2)
    self.assertEqual(documents[0].primary_key, 1)
    self.assertEqual(documents[0].score, 101.0)
    self.assertEqual(len(documents[0].forward_column_values), 2)
    self.assertAlmostEqual(documents[0].forward_column_values['col_a'], 2.234, delta=0.000001)
    self.assertEqual(documents[0].forward_column_values['col_b'], 'def')
    self.assertEqual(documents[1].primary_key, 2)
    self.assertEqual(documents[1].score, 122.0)
    self.assertEqual(len(documents[1].forward_column_values), 2)
    self.assertAlmostEqual(documents[1].forward_column_values['col_a'], 3.234, delta=0.000001)
    self.assertEqual(documents[1].forward_column_values['col_b'], 'def')

  def test_single_delete(self):
    magic_number = self.get_magic_number(self.collection_name)
    req = self.create_single_request(magic_number, OperationType.INSERT)
    logging.info("request: %s", req)
    response = self.client.write(req)
    self.assertTrue(response.ok())

    time.sleep(1)
    status, response = self.simple_query()
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    self.assertEqual(len(documents), 1)
    self.assertEqual(documents[0].primary_key, 1)
    self.assertEqual(documents[0].score, 1.0)
    self.assertEqual(len(documents[0].forward_column_values), 2)
    self.assertAlmostEqual(documents[0].forward_column_values['col_a'], 1.234, delta=0.000001)
    self.assertEqual(documents[0].forward_column_values['col_b'], 'abc')

    req = self.create_single_request(magic_number, OperationType.DELETE)
    logging.info("request: %s", req)
    response = self.client.write(req)
    self.assertTrue(response.ok())

    time.sleep(1)

    status, response = self.simple_query()
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    self.assertEqual(len(documents), 0)

  def test_batch_delete(self):
    magic_number = self.get_magic_number(self.collection_name)
    batch_count = 8
    req = self.create_batch_request(magic_number, batch_count, OperationType.INSERT)
    logging.info("request: %s", req)
    response = self.client.write(req)
    self.assertTrue(response.ok())

    time.sleep(1)

    status, response = self.simple_query()
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    self.assertEqual(len(documents), batch_count)
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

    req = self.create_batch_request(magic_number, batch_count, OperationType.DELETE)
    logging.info("request: %s", req)
    response = self.client.write(req)
    self.assertTrue(response.ok())

    time.sleep(1)

    status, response = self.simple_query()
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    self.assertEqual(len(documents), 0)

  def test_all_operations(self):
    magic_number = self.get_magic_number(self.collection_name)
    batch_count = 5
    operation_types = [OperationType.INSERT,
                       OperationType.UPDATE,
                       OperationType.DELETE,
                       OperationType.INSERT,
                       OperationType.UPDATE,
                       OperationType.DELETE,
                       OperationType.INSERT,
                       OperationType.UPDATE,
                       OperationType.UPDATE]
    req = self.create_all_operations_request(magic_number, batch_count,
                                             operation_types=operation_types)
    logging.info("request: %s", req)
    response = self.client.write(req)
    self.assertTrue(response.ok())

    time.sleep(1)

    status, response = self.simple_query()
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    self.assertEqual(len(documents), batch_count)
    for i in range (1, batch_count + 1):
      self.assertEqual(documents[i - 1].primary_key, i)
      self.assertEqual(documents[i - 1].score, (i - 1) * (i - 1) + 1.0)
      self.assertEqual(len(documents[i - 1].forward_column_values), 2)
      self.assertAlmostEqual(documents[i - 1].forward_column_values['col_a'], 9.234, delta=0.000001)
      self.assertEqual(documents[i - 1].forward_column_values['col_b'], 'abc')

  def test_one_forward_column(self):
    dim = 16
    forwards = ["col_a"]
    new_schema = self.create_schema(self.collection_name2, ["column1"], [dim],
                                    forward_columns=forwards)
    logging.info("new schema: %s", new_schema)
    status = self.client.create_collection(new_schema)
    self.assertTrue(status.ok())

    magic_number = self.get_magic_number(self.collection_name2)
    batch_count = 6
    req = self.create_one_forward_request(self.collection_name2, magic_number,
                                          batch_count,
                                          OperationType.INSERT)
    logging.info("request: %s", req)
    response = self.client.write(req)
    self.assertTrue(response.ok())

    time.sleep(1)

    topk = 5
    status,response = self.query("column1", topk, dim, DataType.VECTOR_FP32)
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    self.assertEqual(len(documents), topk)
    score = 8.0
    for i in range(0, topk):
      self.assertEqual(documents[i].primary_key, i + 1)
      if i != 0:
        score += (i - 1) * 2 + 1
      self.assertEqual(documents[i].score, score)
      self.assertEqual(len(documents[i].forward_column_values), 1)
      self.assertAlmostEqual(documents[i].forward_column_values['col_a'], 1.234 + i, delta=0.000001)

  def test_one_field_both_index_and_forward(self):
    dim = 16
    forwards = ["col_a", "index1"]
    new_schema = self.create_schema1(self.collection_name2,
                                     index_columns=["index1", "index2"],
                                     dimensions=[dim, dim],
                                     forward_columns=forwards)
    logging.info("new schema: %s", new_schema)
    status = self.client.create_collection(new_schema)
    self.assertTrue(status.ok())

    magic_number = self.get_magic_number(self.collection_name2)
    batch_count = 6
    req = self.create_multi_index_request(self.collection_name2, dim,
                                          magic_number,
                                          batch_count,
                                          OperationType.INSERT)
    logging.info("request: %s", req)
    response = self.client.write(req)
    self.assertTrue(response.ok())

    time.sleep(1)

    topk = 5
    status,response = self.query("index1", topk, dim, DataType.VECTOR_FP32)
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    self.assertEqual(len(documents), topk)
    score = 8.0
    for i in range(0, topk):
      self.assertEqual(documents[i].primary_key, i + 1)
      if i != 0:
        score += (i - 1) * 2 + 1
      self.assertEqual(documents[i].score, score)
      self.assertEqual(len(documents[i].forward_column_values), 2)
      self.assertAlmostEqual(documents[i].forward_column_values['col_a'], 1.234 + i, delta=0.000001)
      str_value = '[' + str(i + 1) + ',1,1,1,1,1,1,1,2,2,2,2,2,2,2,2]'
      self.assertEqual(documents[i].forward_column_values['index1'], str_value)

  def test_forward_column_num_mismatched(self):
    magic_number = self.get_magic_number(self.collection_name)
    batch_count = 2
    forwards_columns = ['col_a']
    forward_tuple_types = [DataType.FLOAT]

    req = self.create_request(self.collection_name, magic_number,
                              batch_count, OperationType.INSERT,
                              forward_tuple_names=forwards_columns,
                              forward_tuple_types=forward_tuple_types)
    logging.info("request: %s", req)
    response = self.client.write(req)
    logging.info("process result: %s", response)
    self.assertEqual(response.code, 0)

    time.sleep(1)

    status,response = self.simple_query()
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
    self.assertEqual(documents[0].forward_column_values['col_b'], None)
    self.assertEqual(documents[1].primary_key, 2)
    self.assertEqual(documents[1].score, 2.0)
    self.assertEqual(len(documents[1].forward_column_values), 2)
    self.assertAlmostEqual(documents[1].forward_column_values['col_a'], 2.234, delta=0.000001)
    self.assertEqual(documents[1].forward_column_values['col_b'], None)

class TestIndexAgentDirect(TestIndexAgentBase):
  def setUp(self):
    self.with_repo = False
    super().setUp()

  def test_single_insert(self):
    req = self.create_single_request(None, OperationType.INSERT)
    logging.info("request: %s", req)
    response = self.client.write(req)
    logging.info("process result: %s", response)
    self.assertTrue(response.ok())

    time.sleep(1)

    status, response = self.simple_query()
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    self.assertEqual(len(documents), 1)
    self.assertEqual(documents[0].primary_key, 1)
    self.assertEqual(documents[0].score, 1.0)
    self.assertEqual(len(documents[0].forward_column_values), 2)
    self.assertAlmostEqual(documents[0].forward_column_values['col_a'], 1.234, delta=0.000001)
    self.assertEqual(documents[0].forward_column_values['col_b'], 'abc')

  def test_single_insert_with_bytes(self):
    req = self.create_single_request(None, OperationType.INSERT)
    logging.info("request: %s", req)
    response = self.client.write(req)
    logging.info("process result: %s", response)
    self.assertTrue(response.ok())

    time.sleep(1)

    status, response = self.simple_query()
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    self.assertEqual(len(documents), 1)
    self.assertEqual(documents[0].primary_key, 1)
    self.assertEqual(documents[0].score, 1.0)
    self.assertEqual(len(documents[0].forward_column_values), 2)
    self.assertAlmostEqual(documents[0].forward_column_values['col_a'], 1.234, delta=0.000001)
    self.assertEqual(documents[0].forward_column_values['col_b'], 'abc')

  def test_batch_insert(self):
    batch_count = 2
    req = self.create_batch_request(None, batch_count, OperationType.INSERT)
    logging.info("request: %s", req)
    response = self.client.write(req)
    self.assertTrue(response.ok())

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

  def test_insert_with_all_forward_data_types(self):
    dim = 64
    new_schema = self.create_all_forward_data_types_schema(self.collection_name2,
                                                           dim,
                                                           with_repo=self.with_repo)
    logging.info("new schema: %s", new_schema)
    status = self.client.create_collection(new_schema)
    self.assertTrue(status.ok())

    batch_count = 6
    req = self.create_all_forward_data_types_insert_request(None,
                                                            dim, batch_count)
    logging.info("request: %s", req)
    response = self.client.write(req)
    self.assertTrue(status.ok())

    time.sleep(1)

    topk = 5
    status,response = self.query("column_0", topk, dim, DataType.VECTOR_FP32)
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    self.assertEqual(len(documents), topk)
    self.assertEqual(documents[0].primary_key, 1)
    self.assertEqual(documents[0].score, 0.0)
    self.assertEqual(len(documents[0].forward_column_values), 9)
    self.assertEqual(documents[0].forward_column_values['forward_0'], '1'.encode('UTF-8'))
    self.assertEqual(documents[0].forward_column_values['forward_1'], '1')
    self.assertEqual(documents[0].forward_column_values['forward_2'], True)
    self.assertEqual(documents[0].forward_column_values['forward_3'], 1)
    self.assertEqual(documents[0].forward_column_values['forward_4'], 10)
    self.assertEqual(documents[0].forward_column_values['forward_5'], 100)
    self.assertEqual(documents[0].forward_column_values['forward_6'], 1000)
    self.assertEqual(documents[0].forward_column_values['forward_7'], 1.0)
    self.assertEqual(documents[0].forward_column_values['forward_8'], 10.0)

    self.assertEqual(documents[1].primary_key, 2)
    self.assertEqual(documents[1].score, 64.0)
    self.assertEqual(documents[2].primary_key, 3)
    self.assertEqual(documents[2].score, 256.0)
    self.assertEqual(documents[3].primary_key, 4)
    self.assertEqual(documents[3].score, 576.0)
    self.assertEqual(documents[4].primary_key, 5)
    self.assertEqual(documents[4].score, 1024.0)

  def test_single_update(self):
    forwards = [1.234, 'abc']
    req = self.create_single_request(None, OperationType.INSERT, forwards = forwards, lsn = 10)
    response = self.client.write(req)
    self.assertTrue(response.ok())

    forwards = [2.234, 'def']
    req = self.create_single_request(None, OperationType.UPDATE, forwards = forwards, lsn = 11)
    logging.info("req: %s", req)
    response = self.client.write(req)
    self.assertTrue(response.ok())

    time.sleep(1)

    status, response = self.simple_query()
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    self.assertEqual(len(documents), 1)
    self.assertEqual(documents[0].primary_key, 1)
    self.assertEqual(documents[0].score, 1.0)
    self.assertEqual(len(documents[0].forward_column_values), 2)
    self.assertAlmostEqual(documents[0].forward_column_values['col_a'], 2.234, delta=0.000001)
    self.assertEqual(documents[0].forward_column_values['col_b'], 'def')

  def test_single_update_with_bytes(self):
    forwards = [1.234, 'abc']
    req = self.create_single_request(None, OperationType.INSERT,
                                     forwards = forwards,
                                     lsn = 10, is_bytes = True)
    response = self.client.write(req)
    self.assertTrue(response.ok())

    forwards = [2.234, 'def']
    req = self.create_single_request(None, OperationType.UPDATE, forwards = forwards, lsn = 11)
    logging.info("req: %s", req)
    response = self.client.write(req)
    self.assertTrue(response.ok())

    time.sleep(1)

    status, response = self.simple_query()
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    self.assertEqual(len(documents), 1)
    self.assertEqual(documents[0].primary_key, 1)
    self.assertEqual(documents[0].score, 1.0)
    self.assertEqual(len(documents[0].forward_column_values), 2)
    self.assertAlmostEqual(documents[0].forward_column_values['col_a'], 2.234, delta=0.000001)
    self.assertEqual(documents[0].forward_column_values['col_b'], 'def')

  def test_batch_update(self):
    batch_count = 6
    forwards = [0.234, 'abc']
    req = self.create_batch_request(None, batch_count,
                                    OperationType.INSERT,
                                    forwards = forwards,
                                    lsn = 10)
    logging.info("request: %s", req)
    response = self.client.write(req)
    self.assertTrue(response.ok())

    forwards = [1.234, 'def']
    req = self.create_batch_request(None, batch_count,
                                    OperationType.UPDATE,
                                    forwards = forwards,
                                    lsn = 20)
    logging.info("request: %s", req)
    response = self.client.write(req)
    self.assertTrue(response.ok())

    time.sleep(1)

    status, response = self.simple_query()
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    self.assertEqual(len(documents), 6)
    self.assertEqual(documents[0].primary_key, 1)
    self.assertEqual(documents[0].score, 1.0)
    self.assertEqual(len(documents[0].forward_column_values), 2)
    self.assertAlmostEqual(documents[0].forward_column_values['col_a'], 2.234, delta=0.000001)
    self.assertEqual(documents[0].forward_column_values['col_b'], 'def')
    self.assertEqual(documents[1].primary_key, 2)
    self.assertEqual(documents[1].score, 2.0)
    self.assertEqual(documents[2].primary_key, 3)
    self.assertEqual(documents[2].score, 5.0)
    self.assertEqual(documents[3].primary_key, 4)
    self.assertEqual(documents[3].score, 10.0)
    self.assertEqual(documents[4].primary_key, 5)
    self.assertEqual(documents[4].score, 17.0)
    self.assertEqual(documents[5].primary_key, 6)
    self.assertEqual(documents[5].score, 26.0)
    self.assertEqual(len(documents[5].forward_column_values), 2)
    self.assertAlmostEqual(documents[5].forward_column_values['col_a'], 7.234, delta=0.000001)
    self.assertEqual(documents[5].forward_column_values['col_b'], 'def')

  def test_update_only_index_column(self):
    batch_count = 2
    forwards = [0.234, 'abc']
    req = self.create_batch_request(None, batch_count,
                                    OperationType.INSERT,
                                    forwards = forwards,
                                    lsn = 10,
                                    index_value_base = 0)
    logging.info("request: %s", req)
    response = self.client.write(req)
    self.assertTrue(response.ok())

    req = self.create_batch_request(None, batch_count,
                                    OperationType.UPDATE,
                                    forwards = forwards,
                                    lsn = 20,
                                    index_value_base = 10)
    logging.info("request: %s", req)
    response = self.client.write(req)
    self.assertTrue(response.ok())

    time.sleep(1)

    status, response = self.simple_query()
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    self.assertEqual(len(documents), 2)
    self.assertEqual(documents[0].primary_key, 1)
    self.assertEqual(documents[0].score, 101.0)
    self.assertEqual(len(documents[0].forward_column_values), 2)
    self.assertAlmostEqual(documents[0].forward_column_values['col_a'], 1.234, delta=0.000001)
    self.assertEqual(documents[0].forward_column_values['col_b'], 'abc')
    self.assertEqual(documents[1].primary_key, 2)
    self.assertEqual(documents[1].score, 122.0)
    self.assertEqual(len(documents[1].forward_column_values), 2)
    self.assertAlmostEqual(documents[1].forward_column_values['col_a'], 2.234, delta=0.000001)
    self.assertEqual(documents[1].forward_column_values['col_b'], 'abc')

  def test_update_both_index_and_forward_column(self):
    batch_count = 2
    forwards = [0.234, 'abc']
    req = self.create_batch_request(None, batch_count,
                                    OperationType.INSERT,
                                    forwards = forwards,
                                    lsn = 10,
                                    index_value_base = 0)
    logging.info("request: %s", req)
    response = self.client.write(req)
    self.assertTrue(response.ok())

    forwards = [1.234, 'def']
    req = self.create_batch_request(None, batch_count,
                                    OperationType.UPDATE,
                                    forwards = forwards,
                                    lsn = 20,
                                    index_value_base = 10)
    logging.info("request: %s", req)
    response = self.client.write(req)
    self.assertTrue(response.ok())

    time.sleep(1)

    status, response = self.simple_query()
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    self.assertEqual(len(documents), 2)
    self.assertEqual(documents[0].primary_key, 1)
    self.assertEqual(documents[0].score, 101.0)
    self.assertEqual(len(documents[0].forward_column_values), 2)
    self.assertAlmostEqual(documents[0].forward_column_values['col_a'], 2.234, delta=0.000001)
    self.assertEqual(documents[0].forward_column_values['col_b'], 'def')
    self.assertEqual(documents[1].primary_key, 2)
    self.assertEqual(documents[1].score, 122.0)
    self.assertEqual(len(documents[1].forward_column_values), 2)
    self.assertAlmostEqual(documents[1].forward_column_values['col_a'], 3.234, delta=0.000001)
    self.assertEqual(documents[1].forward_column_values['col_b'], 'def')

  def test_single_delete(self):
    req = self.create_single_request(None, OperationType.INSERT)
    logging.info("request: %s", req)
    response = self.client.write(req)
    self.assertTrue(response.ok())

    time.sleep(1)
    status, response = self.simple_query()
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    self.assertEqual(len(documents), 1)
    self.assertEqual(documents[0].primary_key, 1)
    self.assertEqual(documents[0].score, 1.0)
    self.assertEqual(len(documents[0].forward_column_values), 2)
    self.assertAlmostEqual(documents[0].forward_column_values['col_a'], 1.234, delta=0.000001)
    self.assertEqual(documents[0].forward_column_values['col_b'], 'abc')

    req = self.create_single_request(None, OperationType.DELETE)
    logging.info("request: %s", req)
    response = self.client.write(req)
    self.assertTrue(response.ok())

    time.sleep(1)

    status, response = self.simple_query()
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    self.assertEqual(len(documents), 0)

  def test_batch_delete(self):
    batch_count = 8
    req = self.create_batch_request(None, batch_count, OperationType.INSERT)
    logging.info("request: %s", req)
    response = self.client.write(req)
    self.assertTrue(response.ok())

    time.sleep(1)

    status, response = self.simple_query()
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    self.assertEqual(len(documents), batch_count)
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

    req = self.create_batch_request(None, batch_count, OperationType.DELETE)
    logging.info("request: %s", req)
    response = self.client.write(req)
    self.assertTrue(response.ok())

    time.sleep(1)

    status, response = self.simple_query()
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    self.assertEqual(len(documents), 0)

  def test_all_operations(self):
    batch_count = 5
    operation_types = [OperationType.INSERT,
                       OperationType.UPDATE,
                       OperationType.DELETE,
                       OperationType.INSERT,
                       OperationType.UPDATE,
                       OperationType.DELETE,
                       OperationType.INSERT,
                       OperationType.UPDATE,
                       OperationType.UPDATE]
    req = self.create_all_operations_request(None, batch_count,
                                             operation_types=operation_types)
    logging.info("request: %s", req)
    response = self.client.write(req)
    self.assertTrue(response.ok())

    time.sleep(1)

    status, response = self.simple_query()
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    self.assertEqual(len(documents), batch_count)
    for i in range (1, batch_count + 1):
      self.assertEqual(documents[i - 1].primary_key, i)
      self.assertEqual(documents[i - 1].score, (i - 1) * (i - 1) + 1.0)
      self.assertEqual(len(documents[i - 1].forward_column_values), 2)
      self.assertAlmostEqual(documents[i - 1].forward_column_values['col_a'], 9.234, delta=0.000001)
      self.assertEqual(documents[i - 1].forward_column_values['col_b'], 'abc')

  def test_one_forward_column(self):
    dim = 16
    forwards = ["col_a"]
    new_schema = self.create_schema(self.collection_name2, ["column1"], [dim],
                                    forward_columns=forwards,
                                    with_repo = self.with_repo)
    logging.info("new schema: %s", new_schema)
    status = self.client.create_collection(new_schema)
    self.assertTrue(status.ok())

    batch_count = 6
    req = self.create_one_forward_request(self.collection_name2, None,
                                          batch_count,
                                          OperationType.INSERT)
    logging.info("request: %s", req)
    response = self.client.write(req)
    self.assertTrue(response.ok())

    time.sleep(1)

    topk = 5
    status,response = self.query("column1", topk, dim, DataType.VECTOR_FP32)
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    self.assertEqual(len(documents), topk)
    score = 8.0
    for i in range(0, topk):
      self.assertEqual(documents[i].primary_key, i + 1)
      if i != 0:
        score += (i - 1) * 2 + 1
      self.assertEqual(documents[i].score, score)
      self.assertEqual(len(documents[i].forward_column_values), 1)
      self.assertAlmostEqual(documents[i].forward_column_values['col_a'], 1.234 + i, delta=0.000001)

  def test_one_field_both_index_and_forward(self):
    dim = 16
    forwards = ["col_a", "index1"]
    new_schema = self.create_schema1(self.collection_name2,
                                     index_columns=["index1", "index2"],
                                     dimensions=[dim, dim],
                                     forward_columns=forwards,
                                     with_repo = self.with_repo)
    logging.info("new schema: %s", new_schema)
    status = self.client.create_collection(new_schema)
    self.assertTrue(status.ok())

    magic_number = self.get_magic_number(self.collection_name2)
    batch_count = 6
    req = self.create_multi_index_request(self.collection_name2, dim, None,
                                          batch_count,
                                          OperationType.INSERT)
    logging.info("request: %s", req)
    response = self.client.write(req)
    self.assertTrue(response.ok())

    time.sleep(1)

    topk = 5
    status,response = self.query("index1", topk, dim, DataType.VECTOR_FP32)
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    self.assertEqual(len(documents), topk)
    score = 8.0
    for i in range(0, topk):
      self.assertEqual(documents[i].primary_key, i + 1)
      if i != 0:
        score += (i - 1) * 2 + 1
      self.assertEqual(documents[i].score, score)
      self.assertEqual(len(documents[i].forward_column_values), 2)
      self.assertAlmostEqual(documents[i].forward_column_values['col_a'], 1.234 + i, delta=0.000001)
      str_value = '[' + str(i + 1) + ',1,1,1,1,1,1,1,2,2,2,2,2,2,2,2]'
      self.assertEqual(documents[i].forward_column_values['index1'], str_value)

  def test_forward_column_num_mismatched(self):
    magic_number = self.get_magic_number(self.collection_name)
    batch_count = 2
    forwards_columns = ['col_a']
    forward_tuple_types = [DataType.FLOAT]

    req = self.create_request(self.collection_name, magic_number,
                              batch_count, OperationType.INSERT,
                              forward_tuple_names=forwards_columns,
                              forward_tuple_types=forward_tuple_types)
    logging.info("request: %s", req)
    response = self.client.write(req)
    logging.info("process result: %s", response)
    self.assertEqual(response.code, 0)

    time.sleep(1)

    status,response = self.simple_query()
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
    self.assertEqual(documents[0].forward_column_values['col_b'], None)
    self.assertEqual(documents[1].primary_key, 2)
    self.assertEqual(documents[1].score, 2.0)
    self.assertEqual(len(documents[1].forward_column_values), 2)
    self.assertAlmostEqual(documents[1].forward_column_values['col_a'], 2.234, delta=0.000001)
    self.assertEqual(documents[1].forward_column_values['col_b'], None)

if __name__ == '__main__':
  unittest.main()
