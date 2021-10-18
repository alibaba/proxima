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

from pyproximabe import *
from global_conf import GlobalConf
from collection_creator import CollectionCreator
from test_query_agent import TestQueryAgentBase
from server_utils import ServerUtils
OperationType = WriteRequest.OperationType

class TestQueryAgentException(TestQueryAgentBase):
  def setUp(self):
    super().setUp()
    self.server_utils = ServerUtils()

  # def create_schema(self, collection_name,
  #                   forward_columns=["col_a", "col_b"],
  #                   index_columns=[],
  #                   index_dimensions=[],
  #                   index_alias_columns=None,
  #                   index_data_types=None,
  #                   index_measures=None):
  #   if not index_alias_columns:
  #     index_alias_columns = index_columns
  #   return self.creator.create_schema(collection_name,
  #                                     repository_table="test_collection",
  #                                     repository_name="test_repo",
  #                                     forward_columns=forward_columns,
  #                                     revision=1,
  #                                     index_columns=index_columns,
  #                                     index_alias_columns=index_alias_columns,
  #                                     index_dimensions=index_dimensions,
  #                                     index_data_types=index_data_types,
  #                                     index_measures=index_measures,
  #                                     db_name="test_db")

  # def create_all_forward_data_types_schema(self, collection_name, dim):
  #   index_data_types = [common_pb2.FeatureType.FT_FP32]
  #   index_columns = ['column_0']
  #   index_dimensions = [dim]
  #   forward_columns = []
  #   forward_cnt = 9
  #   for i in range(0, forward_cnt):
  #     forward_columns.append('forward_' + str(i))

  #   return self.creator.create_schema(collection_name,
  #                                     repository_table="test_collection",
  #                                     repository_name="test_repo",
  #                                     forward_columns=forward_columns,
  #                                     revision=1,
  #                                     index_columns=index_columns,
  #                                     index_data_types=index_data_types,
  #                                     index_alias_columns=index_columns,
  #                                     index_dimensions=index_dimensions,
  #                                     db_name="test_db")

  # def create_single_request(self, agent_timestamp, operation_type,
  #                           forwards = [1.234, 'abc'], lsn=10):
  #   index_tuple_names = [self.index_column]
  #   index_tuple_types = [common_pb2.GenericValueMeta.FieldType.FT_BYTES]
  #   forward_tuple_names = ['col_a', 'col_b']
  #   forward_tuple_types = [common_pb2.GenericValueMeta.FieldType.FT_FLOAT,
  #                          common_pb2.GenericValueMeta.FieldType.FT_BYTES]
  #   rows = [[1, operation_type, lsn, "[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2]", forwards[0], forwards[1]]
  #     ]
  #   return self.creator.create_dataset_request(self.collection_name,
  #                                              agent_timestamp,
  #                                              schema_revision=0,
  #                                              index_tuple_names = index_tuple_names,
  #                                              index_tuple_types = index_tuple_types,
  #                                              forward_tuple_names = forward_tuple_names,
  #                                              forward_tuple_types = forward_tuple_types,
  #                                              rows = rows)

  # def create_batch_request(self, collection_name, agent_timestamp,
  #                          count, operation_type,
  #                          forwards=None,
  #                          lsn = 10,
  #                          index_column_count = 1,
  #                          index_value_base = 0,
  #                          index_tuple_names=None,
  #                          index_data_types=None,
  #                          forward_tuple_names=['col_a', 'col_b'],
  #                          same_features=False,
  #                          forward_tuple_types=[common_pb2.GenericValueMeta.FieldType.FT_FLOAT,
  #                                               common_pb2.GenericValueMeta.FieldType.FT_BYTES]):
  #   if not index_tuple_names:
  #     index_tuple_names = [self.index_column]
  #   index_tuple_types = []
  #   for name in index_tuple_names:
  #     index_tuple_types.append(common_pb2.GenericValueMeta.FieldType.FT_BYTES)
  #   rows = []

  #   if forwards is not None and len(forwards) == 0:
  #     forward_tuple_names = []
  #     forward_tuple_types = []
  #   for i in range(1, count + 1):
  #     row = [i, operation_type, lsn + i]
  #     for j in range(0, index_column_count):
  #       if index_data_types:
  #         if index_data_types[j] == common_pb2.FeatureType.FT_BINARY32:
  #           row.append('[' + str(i) + ']')
  #       else:
  #         if same_features:
  #           row.append('[' + str(i + index_value_base) + ",1,1,1,1,1,1,1,1,1,1,1,1,1,1,1]")
  #         else:
  #           row.append('[' + str(i + index_value_base) + ",1,1,1,1,1,1,1,2,2,2,2,2,2,2,2]")
  #     if forwards is None:
  #       tmp_forwards=[0.234, 'abc']
  #       row.append(tmp_forwards[0] + i)
  #       row.append(tmp_forwards[1])
  #     else:
  #       for f in forwards:
  #         row.append(f)
  #     rows.append(row)

  #   return self.creator.create_dataset_request(collection_name,
  #                                              agent_timestamp,
  #                                              schema_revision=0,
  #                                              index_tuple_names = index_tuple_names,
  #                                              index_tuple_types = index_tuple_types,
  #                                              forward_tuple_names = forward_tuple_names,
  #                                              forward_tuple_types = forward_tuple_types,
  #                                              rows = rows)

  # def create_all_forward_data_types_insert_request(self, agent_timestamp, dim, count):
  #   index_tuple_names = ['column_0']
  #   index_tuple_types = [common_pb2.GenericValueMeta.FieldType.FT_BYTES]
  #   total_types = 9
  #   forward_tuple_names = []
  #   forward_tuple_types = [common_pb2.GenericValueMeta.FieldType.FT_BYTES,
  #                          common_pb2.GenericValueMeta.FieldType.FT_STRING,
  #                          common_pb2.GenericValueMeta.FieldType.FT_BOOL,
  #                          common_pb2.GenericValueMeta.FieldType.FT_INT32,
  #                          common_pb2.GenericValueMeta.FieldType.FT_INT64,
  #                          common_pb2.GenericValueMeta.FieldType.FT_UINT32,
  #                          common_pb2.GenericValueMeta.FieldType.FT_UINT64,
  #                          common_pb2.GenericValueMeta.FieldType.FT_FLOAT,
  #                          common_pb2.GenericValueMeta.FieldType.FT_DOUBLE]
  #   for i in range(0, total_types):
  #     forward_tuple_names.append('forward_' + str(i))
  #   rows = []
  #   for i in range(1, count + 1):
  #     row = [i, common_pb2.OperationType.INSERT, 9 + i]
  #     vec = str(i)
  #     for k in range(1, dim):
  #       vec += ',' + str(i)
  #     row.append('[' + vec + ']')
  #     row.append(str(i))
  #     row.append(str(i) * i)
  #     if i % 2 == 1:
  #       row.append(True)
  #     else:
  #       row.append(False)
  #     row.append(i)
  #     row.append(i * 10)
  #     row.append(i * 100)
  #     row.append(i * 1000)
  #     row.append(i * 1.0)
  #     row.append(i * 10.0)
  #     rows.append(row)

  #   return self.creator.create_dataset_request(self.collection_name2,
  #                                              agent_timestamp,
  #                                              schema_revision=0,
  #                                              index_tuple_names = index_tuple_names,
  #                                              index_tuple_types = index_tuple_types,
  #                                              forward_tuple_names = forward_tuple_names,
  #                                              forward_tuple_types = forward_tuple_types,
  #                                              rows = rows)
  def simple_query(self, collection_name=None,
                   index_column='column1',
                   feature_type=DataType.VECTOR_FP32,
                   dimension=16,
                   topk=None,
                   vector_empty=False,
                   invalid_vector_size=False,
                   batch_count=1):
    if not collection_name:
      collection_name = self.collection_name

    if topk is None:
      topk = self.topk
    features = [1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,3]
    fea_bytes = []
    for f in features:
        fea_bytes += struct.pack('f', f)
    if invalid_vector_size:
      fea_bytes = fea_bytes[0:-3]
    if feature_type == DataType.VECTOR_FP32 and not vector_empty:
      features = bytes(fea_bytes)
    elif not vector_empty:
      features = bytes(features)
    else:
      features = bytes([])
    return self.client.query(collection_name,
                             index_column,
                             features,
                             data_type=feature_type,
                             dimension=dimension,
                             batch_count=batch_count,
                             topk=topk)

  def matrix_query(self, collection_name=None,
                   index_column='column1',
                   feature_type=DataType.VECTOR_FP32,
                   matrix="[]",
                   dimension=16,
                   batch_count=1):
    if not collection_name:
      collection_name = self.collection_name

    topk = self.topk
    return self.client.query(collection_name,
                             index_column,
                             matrix,
                             data_type=feature_type,
                             dimension=dimension,
                             batch_count=batch_count,
                             topk=topk)


  # def create_query(self, index_column, topk, dim,
  #                  feature_type = common_pb2.FeatureType.FT_FP32,
  #                  metric_type = common_pb2.MetricType.MT_SQUARED_EUCLIDEAN,
  #                  batch_count = 1,
  #                  radius=None):
  #   query = query_bervice_pb2.QueryRequest()
  #   query.query_type = query_bervice_pb2.QueryType.QT_KNN
  #   query.collection_name = self.collection_name2
  #   query.knn_params.column_name = index_column
  #   query.knn_params.topk = topk
  #   query.knn_params.dimension = dim
  #   query.knn_params.feature_type = feature_type # common_pb2.FeatureType.FT_FP32

  #   features = []
  #   if feature_type == common_pb2.FeatureType.FT_INT4:
  #     dim /= 2
  #   elif feature_type == common_pb2.FeatureType.FT_BINARY32:
  #     dim /= 32
  #   elif feature_type == common_pb2.FeatureType.FT_BINARY64:
  #     dim /= 64
  #   for q in range(1, batch_count + 1):
  #     for i in range(0, int(dim)):
  #       features.append(q)
  #     fea_bytes = []
  #     for f in features:
  #       if feature_type == common_pb2.FeatureType.FT_FP32:
  #         fea_bytes += struct.pack('f', f)
  #       elif feature_type == common_pb2.FeatureType.FT_FP16:
  #         fea_bytes += struct.pack('h', 0)
  #       elif feature_type == common_pb2.FeatureType.FT_INT8:
  #         fea_bytes += struct.pack('b', f)
  #       elif feature_type == common_pb2.FeatureType.FT_INT4:
  #         fea_bytes += struct.pack('b', 17)
  #       elif feature_type == common_pb2.FeatureType.FT_BINARY32:
  #         fea_bytes += struct.pack('I', f)
  #       elif feature_type == common_pb2.FeatureType.FT_BINARY64:
  #         fea_bytes += struct.pack('L', f)
  #     query.knn_params.features = bytes(fea_bytes)
  #   query.knn_params.batch_count = batch_count
  #   if radius:
  #     query.knn_params.radius = radius

  #   return query

  def test_query_with_invalid_collection(self):
    status, response = self.simple_query(collection_name='invalid_collection')
    logging.info("response: %s", status)
    self.assertEqual(status.code, -4002)
    self.assertEqual(status.reason, 'Collection Not Exist')

  def test_query_with_invalid_index_column(self):
    status, response = self.simple_query(index_column='invalid_index')
    logging.info("response: %s", status)
    self.assertEqual(status.code, -4003)
    self.assertEqual(status.reason, 'Column Not Exist')

  def test_query_with_invalid_data_type(self):
    feature_type=DataType.VECTOR_BINARY32
    status, response = self.simple_query(feature_type=feature_type)
    logging.info("response: %s", status)
    self.assertEqual(status.code, -2024)
    self.assertEqual(status.reason, 'Mismatched Data Type')

  def test_query_with_invalid_dimension(self):
    status, response = self.simple_query(dimension=32)
    logging.info("response: %s", status)
    self.assertEqual(status.code, -2010)
    self.assertEqual(status.reason, 'Invalid Query')

  def test_query_with_topk_zero(self):
    # 1 send data
    magic_number = self.get_magic_number(self.collection_name)
    count = 20
    req = self.create_batch_request(self.collection_name, magic_number,
                                    count, OperationType.INSERT)
    response = self.client.write(req)
    self.assertEqual(response.code, 0)
    time.sleep(1)

    # 2 query
    status, response = self.simple_query(topk=0)
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    self.assertEqual(len(documents), 0)

  def test_query_with_empty_vector(self):
    try:
      status, response = self.simple_query(vector_empty=True)
      self.assertTrue(False)
    except core.types.ProximaSeException as e:
      self.assertEqual(str(e), "Empty features:b''")

  def test_query_with_invalid_vector_size(self):
    status, response = self.simple_query(invalid_vector_size=True)
    logging.info("response: %s", status)
    self.assertEqual(status.code, -2010)
    self.assertEqual(status.reason, 'Invalid Query')

  def test_query_with_invalid_batch_count(self):
    try:
      status, response = self.simple_query(batch_count=0)
      self.assertTrue(False)
    except core.types.ProximaSeException as e:
      self.assertEqual(str(e), "Empty dimension[16] or batch_count[0]")

  def test_query_with_server_restart(self):
    # 1 send data
    magic_number = self.get_magic_number(self.collection_name)
    count = 220
    req = self.create_batch_request(self.collection_name, magic_number,
                                    count, OperationType.INSERT)
    response = self.client.write(req)
    self.assertEqual(response.code, 0)
    time.sleep(3)

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

    # 3 restart proxima be
    self.server_utils.stop_proxima_be('SIGUSR1')
    self.server_utils.start_proxima_be()

    # 4 query
    status, response = self.simple_query(topk=count)
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    self.assertEqual(len(documents), count)
    for i in range(0, count):
      self.assertEqual(documents[i].primary_key, i + 1)
      self.assertEqual(documents[i].score, 1.0 + i * i)
      self.assertEqual(len(documents[i].forward_column_values), 2)
      self.assertAlmostEqual(documents[i].forward_column_values['col_a'], 1.234 + i, delta=0.00001)
      self.assertEqual(documents[i].forward_column_values['col_b'], 'abc')

  def test_matrix_query_with_invalid_vector_size(self):
    status, response = self.maxtrix_query(invalid_vector_size=True)
    logging.info("response: %s", status)
    self.assertEqual(status.code, -2010)
    self.assertEqual(status.reason, 'Invalid Query')

  def test_matrix_query_with_invalid_vector_size(self):
    matrix = "[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2]"
    status, response = self.matrix_query(matrix=matrix)
    logging.info("response: %s", status)
    self.assertEqual(status.code, -2010)
    self.assertEqual(status.reason, 'Invalid Query')

  def test_matrix_query_with_invalid_batch_count(self):
    matrix = "[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2]"
    status, response = self.matrix_query(matrix=matrix, batch_count=2)
    logging.info("response: %s", status)
    self.assertEqual(status.code, -2010)
    self.assertEqual(status.reason, 'Invalid Query')

  def test_matrix_query_with_invalid_matrix(self):
    matrix = "[[1,1,1,1,1,1,1,1],[2,2,2,2,2,2,2,2],[3]]"
    status, response = self.matrix_query(matrix=matrix, dimension=8, batch_count=3)
    logging.info("response: %s", status)
    self.assertEqual(status.code, -2010)
    self.assertEqual(status.reason, 'Invalid Query')

  def test_matrix_query_with_invalid_json_array(self):
    matrix = "["
    status, response = self.matrix_query(matrix=matrix)
    logging.info("response: %s", status)
    self.assertEqual(status.code, -2010)
    self.assertEqual(status.reason, 'Invalid Query')
    matrix = "[[1,1,1,1,1,1,1,1],[2,2,2,2,2,2,2,2]"
    status, response = self.matrix_query(matrix=matrix, dimension=8, batch_count=2)
    logging.info("response: %s", status)
    self.assertEqual(status.code, -2010)
    self.assertEqual(status.reason, 'Invalid Query')

  def test_matrix_query_with_json_object(self):
    matrix = "{}"
    status, response = self.matrix_query(matrix=matrix, dimension=8, batch_count=3)
    logging.info("response: %s", status)
    self.assertEqual(status.code, -2010)
    self.assertEqual(status.reason, 'Invalid Query')

if __name__ == '__main__':
  unittest.main()
