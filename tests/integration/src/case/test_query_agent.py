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
import unittest
import time
import random
import json

from pyproximabe import *
from global_conf import GlobalConf
from collection_creator import CollectionCreator
import client_helper

OperationType = WriteRequest.OperationType

class TestQueryAgentBase(unittest.TestCase):
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
    self.schema = self.create_schema(self.collection_name,
                                     index_columns=self.index_columns,
                                     index_dimensions=self.index_dimensions)
    status = self.client.create_collection(self.schema)
    self.assertTrue(status.ok())
    self.connection = self.creator.get_connection()

    self.dimension = 16
    self.topk = 10

  def tearDown(self):
    self.clean_env()

  def clean_env(self):
    status, collections = self.client.list_collections()
    self.assertTrue(status.ok())
    for collection in collections:
      status = self.client.drop_collection(collection.collection_config.collection_name)
      self.assertTrue(status.ok())

  def create_schema(self, collection_name,
                    forward_columns=["col_a", "col_b"],
                    index_columns=[],
                    index_dimensions=[],
                    index_data_types=None,
                    max_docs_per_segment=100,
                    index_measures=None):
    return self.creator.create_schema(collection_name,
                                      repository_table="test_collection",
                                      repository_name="test_repo",
                                      forward_columns=forward_columns,
                                      index_columns=index_columns,
                                      index_dimensions=index_dimensions,
                                      index_data_types=index_data_types,
                                      index_measures=index_measures,
                                      db_name="test_db",
                                      with_repo=True)

  def create_all_forward_data_types_schema(self, collection_name, dim):
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
                                      db_name="test_db")

  def create_single_request(self, magic_number, operation_type,
                            forwards = [1.234, 'abc'], lsn=10):
    index_tuple_names = ['column1']
    index_tuple_types = ['string']
    forward_tuple_names = ['col_a', 'col_b']
    forward_tuple_types = [DataType.FLOAT,
                           DataType.STRING]
    rows = [[1, operation_type, lsn, "[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2]", forwards[0], forwards[1]]
      ]
    return self.creator.create_dataset_request(self.collection_name,
                                               magic_number,
                                               index_tuple_names = index_tuple_names,
                                               index_tuple_types = index_tuple_types,
                                               forward_tuple_names = forward_tuple_names,
                                               forward_tuple_types = forward_tuple_types,
                                               rows = rows)

  def create_batch_request(self, magic_number, count, operation_type,
                           forwards=[0.234, 'abc'],
                           lsn = 10,
                           index_value_base = 0):
    index_tuple_names = ['column1']
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
                                               index_tuple_names = index_tuple_names,
                                               index_tuple_types = index_tuple_types,
                                               forward_tuple_names = forward_tuple_names,
                                               forward_tuple_types = forward_tuple_types,
                                               rows = rows)

  def create_batch_request(self, collection_name, magic_number,
                           count, operation_type,
                           forwards=None,
                           lsn = 10,
                           index_column_count = 1,
                           index_value_base = 0,
                           key_base=0,
                           index_tuple_names=None,
                           index_data_types=None,
                           index_dimensions=None,
                           forward_tuple_names=['col_a', 'col_b'],
                           same_features=False,
                           generate_multi=False,
                           piece_count=100,
                           forward_tuple_types=[DataType.FLOAT,
                                                DataType.STRING]):
    if not index_tuple_names:
      index_metas = [[self.index_columns[0], DataType.VECTOR_FP32, self.index_dimensions[0]]]
    else:
      index_metas = []
      for i in range(0, len(index_tuple_names)):
        index_name = index_tuple_names[i]
        if not index_data_types:
          index_metas.append([index_name, DataType.VECTOR_FP32, self.index_dimensions[0]])
        else:
          index_metas.append([index_name, index_data_types[i], index_dimensions[i]])
    index_tuple_types = []
    for name in index_metas:
      index_tuple_types.append("string")
    rows = []

    if forwards is not None and len(forwards) == 0:
      forward_tuple_names = []
      forward_tuple_types = []
    for i in range(1, count + 1):
      row = [key_base + i, operation_type, lsn + i]
      for j in range(0, index_column_count):
        if index_data_types:
          if index_data_types[j] == DataType.VECTOR_BINARY32:
            row.append('[' + str(i + key_base) + ']')
        else:
          if same_features:
            row.append('[' + str(i + index_value_base) + ",1,1,1,1,1,1,1,1,1,1,1,1,1,1,1]")
          else:
            row.append('[' + str(i + index_value_base) + ",1,1,1,1,1,1,1,2,2,2,2,2,2,2,2]")
      if forwards is None:
        tmp_forwards=[0.234, 'abc']
        row.append(tmp_forwards[0] + i + key_base)
        row.append(tmp_forwards[1])
      else:
        for f in forwards:
          row.append(f)
      rows.append(row)

    if not generate_multi:
      return self.creator.create_dataset_request(collection_name,
                                                 magic_number,
                                                 index_tuple_metas = index_metas,
                                                 index_tuple_types = index_tuple_types,
                                                 forward_tuple_names = forward_tuple_names,
                                                 forward_tuple_types = forward_tuple_types,
                                                 rows = rows)
    else:
      req_list = []
      random.shuffle(rows)
      for i in range(0, len(rows), piece_count):
        if i + piece_count > len(rows):
          piece_count = len(rows) - i
        req = self.creator.create_dataset_request(collection_name,
                                                 magic_number,
                                                 index_tuple_metas = index_metas,
                                                 index_tuple_types = index_tuple_types,
                                                 forward_tuple_names = forward_tuple_names,
                                                 forward_tuple_types = forward_tuple_types,
                                                 rows = rows[i:i+piece_count])
        req_list.append(req)
      return req_list
      

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
                                               rows = rows)

  def get_magic_number(self, collection_name):
    status, collection = self.client.describe_collection(collection_name)
    self.assertTrue(status.ok())
    return collection.magic_number

  def simple_query(self, topk=10):
    features = [[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,3]]
    return self.client.query(self.collection_name,
                             'column1',
                             features,
                             data_type=DataType.VECTOR_FP32,
                             dimension=16,
                             batch_count=1,
                             topk=self.topk)

  def query(self, index_column, topk, dim,
            feature_type = DataType.VECTOR_FP32,
            batch_count = 1,
            radius=None):
    orig_dim = dim
    features = []
    if feature_type == DataType.VECTOR_INT4:
      dim /= 2
    elif feature_type == DataType.VECTOR_BINARY32:
      dim /= 32
    elif feature_type == DataType.VECTOR_BINARY64:
      dim /= 64
    dim = int(dim)
    for q in range(1, batch_count + 1):
      vector = []
      for i in range(0, int(dim)):
        vector.append(q)
      features.append(vector)
      # fea_bytes = []
      # for f in features:
      #   if feature_type == FeatureType.FT_FP32:
      #     fea_bytes += struct.pack('f', f)
      #   elif feature_type == FeatureType.FT_FP16:
      #     fea_bytes += struct.pack('h', 0)
      #   elif feature_type == FeatureType.FT_INT8:
      #     fea_bytes += struct.pack('b', f)
      #   elif feature_type == FeatureType.FT_INT4:
      #     fea_bytes += struct.pack('b', 17)
      #   elif feature_type == FeatureType.FT_BINARY32:
      #     fea_bytes += struct.pack('I', f)
      #   elif feature_type == FeatureType.FT_BINARY64:
      #     fea_bytes += struct.pack('L', f)

    return self.client.query(self.collection_name2,
                             index_column,
                             features,
                             data_type=feature_type,
                             dimension=orig_dim,
                             batch_count=batch_count,
                             topk=topk,
                             radius=radius)

  def matrix_query(self, index_column, topk, dim,
                   feature_type = DataType.VECTOR_FP32,
                   batch_count = 1,
                   radius=None):
    orig_dim = dim
    features = []
    if feature_type == DataType.VECTOR_INT4:
      dim /= 2
    elif feature_type == DataType.VECTOR_BINARY32:
      dim /= 32
    elif feature_type == DataType.VECTOR_BINARY64:
      dim /= 64
    dim = int(dim)
    for q in range(1, batch_count + 1):
      vector = []
      for i in range(0, int(dim)):
        vector.append(q)
      features.append(vector)

    return self.client.query(self.collection_name2,
                             index_column,
                             json.dumps(features),
                             data_type=feature_type,
                             dimension=orig_dim,
                             batch_count=batch_count,
                             topk=topk,
                             radius=radius)



class TestQueryAgent(TestQueryAgentBase):
  def test_query_with_single_index_column(self):
    # 1 send data
    magic_number = self.get_magic_number(self.collection_name)
    count = 20
    req = self.create_batch_request(self.collection_name, magic_number,
                                    count, OperationType.INSERT)
    response = self.client.write(req)
    self.assertEqual(response.code, 0)
    time.sleep(1)

    # 2 query
    status, response = self.simple_query()
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    self.assertEqual(len(documents), self.topk)
    for i in range(0, self.topk):
      self.assertEqual(documents[i].primary_key, i + 1)
      self.assertEqual(documents[i].score, 1.0 + i * i)
      self.assertEqual(len(documents[i].forward_column_values), 2)
      self.assertAlmostEqual(documents[i].forward_column_values['col_a'], 1.234 + i, delta=0.000001)
      self.assertEqual(documents[i].forward_column_values['col_b'], 'abc')

  def test_query_with_multi_index_columns(self):
    # 1 create collection with multi index columns
    index_columns = ['ic0', 'ic1', 'ic2']
    schema = self.create_schema(self.collection_name2,
                                index_columns=index_columns,
                                index_dimensions=[self.dimension, self.dimension, self.dimension])
    status = self.client.create_collection(schema)
    self.assertEqual(status.code, 0)

    # 2 send data
    magic_number = self.get_magic_number(self.collection_name2)
    count = 20
    index_columns.reverse()
    req = self.create_batch_request(self.collection_name2, magic_number,
                                    count, OperationType.INSERT,
                                    index_column_count = 3,
                                    index_tuple_names=index_columns)
    response = self.client.write(req)
    self.assertEqual(response.code, 0)
    time.sleep(1)

    # 3 query
    for index in index_columns:
      status, response = self.query(index, self.topk, self.dimension)
      self.assertTrue(status.ok())
      logging.info("query result: %s", response)
      results = response.results
      self.assertEqual(len(results), 1)
      documents = results[0]
      self.assertEqual(len(documents), self.topk)
      score = 8.0
      for i in range(0, self.topk):
        self.assertEqual(documents[i].primary_key, i + 1)
        if i == 0:
          self.assertEqual(documents[i].score, 8.0)
        else:
          self.assertEqual(documents[i].score, score)
        self.assertEqual(len(documents[i].forward_column_values), 2)
        self.assertAlmostEqual(documents[i].forward_column_values['col_a'], 1.234 + i, delta=0.000001)
        self.assertEqual(documents[1].forward_column_values['col_b'], 'abc')
        score += (2 * i + 1)

  def test_query_with_no_forward_column(self):
    # 1 create collection with no forward columns
    index_columns = ['ic0']
    schema = self.create_schema(self.collection_name2,
                                index_columns=index_columns,
                                index_dimensions=[self.dimension],
                                forward_columns=[])
    logging.info("schema: %s", schema)
    status = self.client.create_collection(schema)
    self.assertEqual(status.code, 0)

    # 2 send data
    magic_number = self.get_magic_number(self.collection_name)
    count = 20
    req = self.create_batch_request(self.collection_name2, magic_number,
                                    count, OperationType.INSERT,
                                    forwards=[],
                                    index_tuple_names=index_columns)
    logging.info("req: %s", req)
    response = self.client.write(req)
    self.assertEqual(response.code, 0)
    time.sleep(1)

    # 3 query
    status, response = self.query(index_columns[0], self.topk, self.dimension)
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    self.assertEqual(len(documents), self.topk)
    score = 8.0
    for i in range(0, self.topk):
        self.assertEqual(documents[i].primary_key, i + 1)
        if i == 0:
          self.assertEqual(documents[i].score, 8.0)
        else:
          self.assertEqual(documents[i].score, score)
        self.assertEqual(len(documents[i].forward_column_values), 0)

        score += (2 * i + 1)

  def test_query_with_all_forward_data_types(self):
    dim = 64
    new_schema = self.create_all_forward_data_types_schema(self.collection_name2, dim)
    logging.info("new schema: %s", new_schema)
    status = self.client.create_collection(new_schema)
    self.assertEqual(status.code, 0)

    magic_number = self.get_magic_number(self.collection_name)

    batch_count = 6
    req = self.create_all_forward_data_types_insert_request(magic_number,
                                                            dim, batch_count)
    logging.info("request: %s", req)
    response = self.client.write(req)
    self.assertEqual(response.code, 0)

    time.sleep(1)

    topk = 5
    status, response = self.query("column_0", topk, dim, DataType.VECTOR_FP32)
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
    self.assertAlmostEqual(documents[0].forward_column_values['forward_7'], 1.0, delta=0.000001)
    self.assertAlmostEqual(documents[0].forward_column_values['forward_8'], 10.0, delta=0.000001)
    self.assertEqual(documents[1].primary_key, 2)
    self.assertEqual(documents[1].score, 64.0)
    self.assertEqual(documents[2].primary_key, 3)
    self.assertEqual(documents[2].score, 256.0)
    self.assertEqual(documents[3].primary_key, 4)
    self.assertEqual(documents[3].score, 576.0)
    self.assertEqual(documents[4].primary_key, 5)
    self.assertEqual(documents[4].score, 1024.0)
    self.assertEqual(len(documents[4].forward_column_values), 9)
    self.assertEqual(documents[4].forward_column_values['forward_0'], '5'.encode('UTF-8'))
    self.assertEqual(documents[4].forward_column_values['forward_1'], '55555')
    self.assertEqual(documents[4].forward_column_values['forward_2'], True)
    self.assertEqual(documents[4].forward_column_values['forward_3'], 5)
    self.assertEqual(documents[4].forward_column_values['forward_4'], 50)
    self.assertEqual(documents[4].forward_column_values['forward_5'], 500)
    self.assertEqual(documents[4].forward_column_values['forward_6'], 5000)
    self.assertAlmostEqual(documents[4].forward_column_values['forward_7'], 5.0, delta=0.000001)
    self.assertAlmostEqual(documents[4].forward_column_values['forward_8'], 50.0, delta=0.000001)

  def test_query_with_squared_euclidean(self):
    # 1 create collection
    index_columns = ['ic0']
    schema = self.create_schema(self.collection_name2,
                                index_columns=index_columns,
                                index_dimensions=[self.dimension],
                                index_measures=['SquaredEuclidean'])
    logging.info("schema: %s", schema)
    status = self.client.create_collection(schema)
    self.assertEqual(status.code, 0)

    # 2 send data
    magic_number = self.get_magic_number(self.collection_name)
    count = 20
    req = self.create_batch_request(self.collection_name2, magic_number,
                                    count, OperationType.INSERT,
                                    same_features=True,
                                    index_tuple_names=index_columns)
    response = self.client.write(req)
    self.assertEqual(response.code, 0)
    time.sleep(1)

    # 3 query
    status, response = self.query(index_columns[0], self.topk, self.dimension)
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    self.assertEqual(len(documents), self.topk)
    for i in range(0, self.topk):
      self.assertEqual(documents[i].primary_key, i + 1)
      self.assertEqual(documents[i].score, i * i)
      self.assertEqual(len(documents[i].forward_column_values), 2)
      self.assertAlmostEqual(documents[i].forward_column_values['col_a'], 1.234 + i, delta=0.000001)
      self.assertEqual(documents[i].forward_column_values['col_b'], 'abc')

  def test_query_with_euclidean(self):
    # 1 create collection
    index_columns = ['ic0']
    schema = self.create_schema(self.collection_name2,
                                index_columns=index_columns,
                                index_dimensions=[self.dimension],
                                index_measures=['Euclidean'])
    logging.info("schema: %s", schema)
    status = self.client.create_collection(schema)
    self.assertEqual(status.code, 0)

    # 2 send data
    magic_number = self.get_magic_number(self.collection_name)
    count = 20
    req = self.create_batch_request(self.collection_name2, magic_number,
                                    count, OperationType.INSERT,
                                    same_features=True,
                                    index_tuple_names=index_columns)
    response = self.client.write(req)
    self.assertEqual(response.code, 0)
    time.sleep(1)

    # 3 query
    status, response = self.query(index_columns[0], self.topk, self.dimension)
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    self.assertEqual(len(documents), self.topk)
    for i in range(0, self.topk):
      self.assertEqual(documents[i].primary_key, i + 1)
      self.assertEqual(documents[i].score, i)
      self.assertEqual(len(documents[i].forward_column_values), 2)
      self.assertAlmostEqual(documents[i].forward_column_values['col_a'], 1.234 + i, delta=0.000001)
      self.assertEqual(documents[i].forward_column_values['col_b'], 'abc')

  def test_query_with_inner_product(self):
    # 1 create collection
    index_columns = ['ic0']
    schema = self.create_schema(self.collection_name2,
                                index_columns=index_columns,
                                index_dimensions=[self.dimension],
                                index_measures=['InnerProduct'])
    logging.info("schema: %s", schema)
    status = self.client.create_collection(schema)
    self.assertEqual(status.code, 0)

    # 2 send data
    magic_number = self.get_magic_number(self.collection_name2)
    count = 20
    req = self.create_batch_request(self.collection_name2, magic_number,
                                    count, OperationType.INSERT,
                                    same_features=True,
                                    index_tuple_names=index_columns)
    logging.info("req: %s", req)
    response = self.client.write(req)
    self.assertEqual(response.code, 0)
    time.sleep(1)

    # 3 query
    status, response = self.query(index_columns[0], self.topk, self.dimension)
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    self.assertEqual(len(documents), self.topk)
    for i in range(0, self.topk):
      self.assertEqual(documents[i].primary_key, count - i)
      self.assertEqual(documents[i].score, -1.0 * (15 + count - i))
      self.assertEqual(len(documents[i].forward_column_values), 2)
      self.assertAlmostEqual(documents[i].forward_column_values['col_a'], 0.234 + count - i, delta=0.000001)
      self.assertEqual(documents[i].forward_column_values['col_b'], 'abc')

  def test_query_with_hamming(self):
    # 1 create collection
    dimension = 32
    index_columns = ['ic0']
    schema = self.create_schema(self.collection_name2,
                                index_columns=index_columns,
                                index_dimensions=[dimension],
                                index_data_types=[DataType.VECTOR_BINARY32],
                                index_measures=['Hamming'])
    logging.info("schema: %s", schema)
    status = self.client.create_collection(schema)
    self.assertEqual(status.code, 0)

    # 2 send data
    magic_number = self.get_magic_number(self.collection_name2)
    count = 20
    req = self.create_batch_request(self.collection_name2, magic_number,
                                    count, OperationType.INSERT,
                                    index_data_types=[DataType.VECTOR_BINARY32],
                                    index_tuple_names=index_columns,
                                    index_dimensions=[dimension])
    response = self.client.write(req)
    self.assertEqual(response.code, 0)
    time.sleep(1)

    # 3 query n
    status, response = self.query(index_columns[0], self.topk, dimension,
                                  feature_type=DataType.VECTOR_BINARY32)
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    scores = [0.0, 1.0, 1.0, 1.0, 1.0, 2.0, 2.0, 2.0, 2.0, 2.0]
    self.assertEqual(len(documents), self.topk)
    for i in range(0, self.topk):
      self.assertEqual(documents[i].score, scores[i])
      self.assertEqual(documents[0].forward_column_values['col_b'], 'abc')

  # def test_query_with_metric_type_different(self):
  #   # 1 create collection
  #   index_columns = ['ic0']
  #   schema = self.create_schema(self.collection_name2,
  #                               index_columns=index_columns,
  #                               index_dimensions=[self.dimension],
  #                               index_measures=['SquaredEuclidean'])
  #   logging.info("schema: %s", schema)
  #   status, data = self.client.create_collection(schema)
  #   logging.info("data: %s", data)
  #   self.assertEqual(status, 200)
  #   self.assertEqual(data["code"], 0)
  #   self.collection_count += 1

  #   # 2 send data
  #   response = self.client.get_collection(self.collection_name2)
  #   magic_number = response.entity.magic_number
  #   count = 20
  #   req = self.create_batch_request(self.collection_name2, magic_number,
  #                                   count, OperationType.INSERT,
  #                                   same_features=True,
  #                                   index_tuple_names=index_columns)
  #   response = self.client.write(req)
  #   self.assertEqual(response.code, 0)
  #   time.sleep(1)

  #   # 3 query
  #   query = self.create_query(index_columns[0], self.topk, self.dimension,
  #                             metric_type=MetricType.MT_EUCLIDEAN)
  #   response = self.client.query(query)
  #   logging.info("response: %s", response)
  #   self.assertEqual(response.code, 0)
  #   documents = response.entity[0]
  #   self.assertEqual(len(documents), self.topk)
  #   for i in range(0, self.topk):
  #     self.assertEqual(documents[i].primary_key, i + 1)
  #     self.assertEqual(documents[i].score, i * i)
  #     self.assertEqual(documents[i].forward[0].key, 'col_a')
  #     self.assertAlmostEqual(documents[i].forward[0].value.float_value,
  #                            1.234 + i, delta=0.000001)
  #     self.assertEqual(documents[i].forward[1].key, 'col_b')
  #     self.assertEqual(documents[i].forward[1].value.bytes_value, 'abc'.encode('UTF-8'))

  def test_query_with_batch(self):
    # 1 create collection with multi index columns
    index_columns = ['ic0']
    schema = self.create_schema(self.collection_name2,
                                index_columns=index_columns,
                                index_dimensions=[self.dimension])
    logging.info("schema: %s", schema)
    status = self.client.create_collection(schema)
    self.assertEqual(status.code, 0)

    # 2 send data
    magic_number = self.get_magic_number(self.collection_name)
    count = 20
    index_columns.reverse()
    req = self.create_batch_request(self.collection_name2, magic_number,
                                    count, OperationType.INSERT,
                                    index_column_count = 1,
                                    index_tuple_names=index_columns)
    response = self.client.write(req)
    self.assertEqual(response.code, 0)
    time.sleep(1)

    # 3 query
    for index in index_columns:
      status, response = self.query(index, self.topk,
                                    self.dimension, batch_count=2)
      self.assertTrue(status.ok())
      logging.info("query result: %s", response)
      results = response.results
      self.assertEqual(len(results), 2)
      documents = results[0]
      self.assertEqual(len(documents), self.topk)
      score = 8.0
      for i in range(0, self.topk):
        self.assertEqual(documents[i].primary_key, i + 1)
        if i == 0:
          self.assertEqual(documents[i].score, 8.0)
        else:
          self.assertEqual(documents[i].score, score)
        self.assertEqual(len(documents[i].forward_column_values), 2)
        self.assertAlmostEqual(documents[i].forward_column_values['col_a'], 1.234 + i, delta=0.000001)
        self.assertEqual(documents[i].forward_column_values['col_b'], 'abc')
        score += (2 * i + 1)

      documents = results[1]
      self.assertEqual(len(documents), self.topk)
      score = 8.0
      for i in range(0, self.topk):
        if i == 0:
          self.assertEqual(documents[i].primary_key, 2)
        elif i > 2:
          self.assertEqual(documents[i].primary_key, i + 1)

        if i == 0:
          self.assertEqual(documents[i].score, 7.0)
        elif i <= 2:
          self.assertEqual(documents[i].score, 8.0)
        else:
          self.assertEqual(documents[i].score, score)
        self.assertEqual(len(documents[i].forward_column_values), 2)
        self.assertEqual(documents[i].forward_column_values['col_b'], 'abc')
        if i >= 2:
          score += (2 * i - 1)

  def test_query_with_radius(self):
    # 1 create collection
    index_columns = ['ic0']
    schema = self.create_schema(self.collection_name2,
                                index_columns=index_columns,
                                index_dimensions=[self.dimension],
                                index_measures=['SquaredEuclidean'])
    logging.info("schema: %s", schema)
    status = self.client.create_collection(schema)
    self.assertEqual(status.code, 0)

    # 2 send data
    magic_number = self.get_magic_number(self.collection_name)
    count = 20
    req = self.create_batch_request(self.collection_name2, magic_number,
                                    count, OperationType.INSERT,
                                    same_features=True,
                                    index_tuple_names=index_columns)
    response = self.client.write(req)
    self.assertEqual(response.code, 0)
    time.sleep(1)

    # 3 query
    status, response = self.query(index_columns[0], self.topk, self.dimension,
                                  radius = 0.9)
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    self.assertEqual(len(documents), 1)
    self.assertEqual(documents[0].primary_key, 1)
    self.assertEqual(documents[0].score, 0)
    self.assertAlmostEqual(documents[0].forward_column_values['col_a'], 1.234, delta=0.000001)
    self.assertEqual(documents[0].forward_column_values['col_b'], 'abc')

  def test_query_with_topk(self):
    # 1 create collection
    index_columns = ['ic0']
    schema = self.create_schema(self.collection_name2,
                                index_columns=index_columns,
                                index_dimensions=[self.dimension],
                                index_measures=['SquaredEuclidean'])
    logging.info("schema: %s", schema)
    status = self.client.create_collection(schema)
    self.assertEqual(status.code, 0)

    # 2 send data
    magic_number = self.get_magic_number(self.collection_name)
    count = 20
    req = self.create_batch_request(self.collection_name2, magic_number,
                                    count, OperationType.INSERT,
                                    same_features=True,
                                    index_tuple_names=index_columns)
    response = self.client.write(req)
    self.assertEqual(response.code, 0)
    time.sleep(1)

    # 3 query
    # topk > count
    topks = [100, count, 2, 0]
    for topk in topks:
      status, response = self.query(index_columns[0], topk, self.dimension)
      self.assertTrue(status.ok())
      logging.info("query result: %s", response)
      results = response.results
      self.assertEqual(len(results), 1)
      documents = results[0]
      result_count = topk
      if result_count > count:
        result_count = count
      self.assertEqual(len(documents), result_count)
      for i in range(0, result_count):
        self.assertEqual(documents[i].primary_key, i + 1)
        self.assertEqual(documents[i].score, i * i)
        self.assertEqual(len(documents[i].forward_column_values), 2)
        self.assertAlmostEqual(documents[i].forward_column_values['col_a'], 1.234 + i, delta=0.000001)
        self.assertEqual(documents[i].forward_column_values['col_b'], 'abc')

  def test_query_with_multi_dump_segment(self):
    # 1 create collection
    index_columns = ['ic0']
    schema = self.create_schema(self.collection_name2,
                                index_columns=index_columns,
                                index_dimensions=[self.dimension],
                                index_measures=['SquaredEuclidean'])
    logging.info("schema: %s", schema)
    status = self.client.create_collection(schema)
    self.assertEqual(status.code, 0)

    # 2 send data
    magic_number = self.get_magic_number(self.collection_name)
    count = 333
    req_list = self.create_batch_request(self.collection_name2, magic_number,
                                         count, OperationType.INSERT,
                                         same_features=True,
                                         key_base = 0,
                                         index_value_base=0,
                                         generate_multi=True,
                                         piece_count=100,
                                         index_tuple_names=index_columns)
    for req in req_list:
      response = self.client.write(req)
      self.assertEqual(response.code, 0)
      time.sleep(5)

    # 3 query
    topk = count
    status, response = self.query(index_columns[0],
                                  topk, self.dimension)
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    self.assertEqual(len(documents), count)
    for i in range(0, count):
      self.assertEqual(documents[i].primary_key, i + 1)
      self.assertEqual(documents[i].score, i * i)
      self.assertEqual(len(documents[i].forward_column_values), 2)
      self.assertAlmostEqual(documents[i].forward_column_values['col_a'], 1.234 + i, delta=0.00001)
      self.assertEqual(documents[i].forward_column_values['col_b'], 'abc')

  def test_query_with_empty(self):
    # 1 query
    status, response = self.simple_query()
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    self.assertEqual(len(documents), 0)

  def test_query_with_pk_exist(self):
    # 1 send data
    magic_number = self.get_magic_number(self.collection_name)
    count = 220
    req = self.create_batch_request(self.collection_name, magic_number,
                                    count, OperationType.INSERT)
    response = self.client.write(req)
    self.assertEqual(response.code, 0)
    time.sleep(2)

    # 2 query
    for pk in range(1, count + 1):
      status, document = self.client.get_document_by_key(self.collection_name, pk)
      self.assertEqual(status.code, 0)
      logging.info("query result: %s", document)

      self.assertEqual(document.primary_key, pk)
      self.assertEqual(document.score, 0.0)
      self.assertEqual(len(document.forward_column_values), 2)
      self.assertAlmostEqual(document.forward_column_values['col_a'], 0.234 + pk, delta=0.00001)
      self.assertEqual(document.forward_column_values['col_b'], 'abc')

  def test_query_with_pk_no_exist(self):
    # 1 send data
    magic_number = self.get_magic_number(self.collection_name)
    count = 30
    req = self.create_batch_request(self.collection_name, magic_number,
                                    count, OperationType.INSERT)
    response = self.client.write(req)
    self.assertEqual(response.code, 0)
    time.sleep(2)

    # 2 query
    pk = 100
    status, document = self.client.get_document_by_key(self.collection_name, pk)
    logging.info("query status: %s", status)
    self.assertTrue(status.ok())
    self.assertIsNone(document)

  def test_matrix_query_with_multi_index_columns(self):
    # 1 create collection with multi index columns
    index_columns = ['ic0', 'ic1', 'ic2']
    schema = self.create_schema(self.collection_name2,
                                index_columns=index_columns,
                                index_dimensions=[self.dimension, self.dimension, self.dimension])
    status = self.client.create_collection(schema)
    self.assertEqual(status.code, 0)

    # 2 send data
    magic_number = self.get_magic_number(self.collection_name2)
    count = 20
    index_columns.reverse()
    req = self.create_batch_request(self.collection_name2, magic_number,
                                    count, OperationType.INSERT,
                                    index_column_count = 3,
                                    index_tuple_names=index_columns)
    response = self.client.write(req)
    self.assertEqual(response.code, 0)
    time.sleep(1)

    # 3 query
    for index in index_columns:
      status, response = self.matrix_query(index, self.topk, self.dimension)
      self.assertTrue(status.ok())
      logging.info("query result: %s", response)
      results = response.results
      self.assertEqual(len(results), 1)
      documents = results[0]
      self.assertEqual(len(documents), self.topk)
      score = 8.0
      for i in range(0, self.topk):
        self.assertEqual(documents[i].primary_key, i + 1)
        if i == 0:
          self.assertEqual(documents[i].score, 8.0)
        else:
          self.assertEqual(documents[i].score, score)
        self.assertEqual(len(documents[i].forward_column_values), 2)
        self.assertAlmostEqual(documents[i].forward_column_values['col_a'], 1.234 + i, delta=0.000001)
        self.assertEqual(documents[1].forward_column_values['col_b'], 'abc')
        score += (2 * i + 1)


if __name__ == '__main__':
  unittest.main()
