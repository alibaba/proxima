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
import os
import random

from pyproximabe import *
from global_conf import GlobalConf
from collection_creator import CollectionCreator
from mysql_client import MysqlClient
from server_utils import ServerUtils
import client_helper

class TestMysqlRepository(unittest.TestCase):
  def setUp(self):
    self.global_conf = GlobalConf()
    self.creator = CollectionCreator()
    self.client = client_helper.get_client(self.global_conf)
    self.mysql_client = MysqlClient()
    self.creator = CollectionCreator()
    self.server_utils = ServerUtils()
    self.repository_name = "test_repo"

    self.clean_env()

    ret = self.mysql_client.execute_batch_sql("data/test_clean_db.sql")
    self.assertEqual(ret, 0)

  def tearDown(self):
    self.clean_env()
    time.sleep(1)

  def clean_env(self):
    status, collections = self.client.list_collections()
    self.assertTrue(status.ok())
    for collection in collections:
      status = self.client.drop_collection(collection.collection_config.collection_name)
      self.assertTrue(status.ok())

  def get_content(self, file_name):
    src_path = os.getenv('SRC_PATH')
    f = open(src_path + '/tests/integration/log/' + file_name, 'r')
    return f.read()

  def create_schema(self, collection_name,
                    repository_table="test_collection",
                    forward_columns=["col_a", "col_b"],
                    index_columns=[],
                    index_dimensions=[],
                    index_data_types=None,
                    index_measures=None):
    return self.creator.create_schema(collection_name,
                                      repository_table=repository_table,
                                      repository_name="test_repo",
                                      forward_columns=forward_columns,
                                      index_columns=index_columns,
                                      index_dimensions=index_dimensions,
                                      index_data_types=index_data_types,
                                      index_measures=index_measures,
                                      db_name="test_db",
                                      with_repo=True)

  def query(self, collection_name, topk=10, column_name='column1'):
    features = [1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,1]
    return self.client.query(collection_name,
                             column_name,
                             features,
                             data_type=DataType.VECTOR_FP32,
                             dimension=16,
                             batch_count=1,
                             topk=topk,
                             is_linear=True)

  def create_query(self, collection_name, topk=10, column_name='column1'):
    query = query_service_pb2.QueryRequest()
    query.query_type = query_service_pb2.QueryType.QT_KNN
    query.collection_name = collection_name
    query.knn_params.column_name = column_name
    query.knn_params.topk = topk
    query.knn_params.dimension = 16
    query.knn_params.feature_type = common_pb2.FeatureType.FT_FP32
    features = [1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,1]
    fea_bytes = []
    for f in features:
        fea_bytes += struct.pack('f', f)
    query.knn_params.features = bytes(fea_bytes)
    query.knn_params.batch_count = 1
    query.knn_params.is_linear = True

    return query

  def get_latest_lsn(self, collection_name):
    status, collection = self.client.describe_collection(collection_name)
    self.assertTrue(status.ok())
    return collection.latest_lsn_context

  def test_scan_full_table(self):
    collection_name = 'test_scan_full_table'
    schema = self.create_schema(collection_name,
                                repository_table=collection_name,
                                index_columns=["column1"],
                                index_dimensions=[16])
    status = self.client.create_collection(schema)
    self.assertEqual(status.code, 0)

    ret = self.mysql_client.execute_batch_sql('data/test_scan_table.sql')
    self.assertEqual(ret, 0)
    time.sleep(2)

    status, response = self.query(collection_name)
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    self.assertEqual(len(documents), 2)
    self.assertEqual(documents[0].primary_key, 1)
    self.assertEqual(documents[0].score, 0.0)
    self.assertAlmostEqual(documents[0].forward_column_values['col_a'],
                           11.111, delta=0.000001)
    self.assertEqual(documents[0].forward_column_values['col_b'], 100)
    self.assertEqual(documents[1].primary_key, 2)
    self.assertEqual(documents[1].score, 1.0)
    self.assertAlmostEqual(documents[1].forward_column_values['col_a'],
                           12.111, delta=0.000001)
    self.assertEqual(documents[1].forward_column_values['col_b'], 200)

  def test_scan_full_table_with_empty_table(self):
    collection_name = 'full_table_with_empty_table'
    schema = self.create_schema(collection_name,
                                repository_table=collection_name,
                                index_columns=["column1"],
                                index_dimensions=[16])
    status = self.client.create_collection(schema)
    self.assertEqual(status.code, 0)

    ret = self.mysql_client.execute_batch_sql('data/test_scan_full_table_with_empty_table.sql')
    self.assertEqual(ret, 0)
    time.sleep(2)

    status, response = self.query(collection_name)
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    self.assertEqual(len(documents), 0)

  def test_increment_mode(self):
    # prepare full table data
    ret = self.mysql_client.execute_batch_sql('data/test_increment_mode_full.sql')
    self.assertEqual(ret, 0)

    # create collection
    collection_name = 'test_increment_mode'
    schema = self.create_schema(collection_name,
                                repository_table=collection_name,
                                index_columns=["column1"],
                                index_dimensions=[16])
    status = self.client.create_collection(schema)
    self.assertEqual(status.code, 0)

    time.sleep(5)

    # query result
    status, response = self.query(collection_name)
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    self.assertEqual(len(documents), 2)
    self.assertEqual(documents[0].primary_key, 1)
    self.assertEqual(documents[0].score, 0.0)
    self.assertAlmostEqual(documents[0].forward_column_values['col_a'],
                           11.111, delta=0.000001)
    self.assertEqual(documents[0].forward_column_values['col_b'], 100)
    self.assertEqual(documents[1].primary_key, 2)
    self.assertEqual(documents[1].score, 1.0)
    self.assertAlmostEqual(documents[1].forward_column_values['col_a'],
                           12.111, delta=0.000001)
    self.assertEqual(documents[1].forward_column_values['col_b'], 200)

    # prepare increment table data
    ret = self.mysql_client.execute_batch_sql('data/test_increment_mode_inc.sql')
    self.assertEqual(ret, 0)
    time.sleep(6)

    # query result
    status, response = self.query(collection_name)
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    self.assertEqual(len(documents), 7)
    for i in range(0, 7):
      self.assertEqual(documents[i].primary_key, i + 1)
      self.assertEqual(documents[i].score, i * i)
      self.assertAlmostEqual(documents[i].forward_column_values['col_a'],
                             11.111 + i, delta=0.000001)
      self.assertEqual(documents[i].forward_column_values['col_b'], 100 * (i + 1))


  def test_forward_with_numeric(self):
    collection_name = 'test_forward_with_numeric'
    # prepare full table data
    ret = self.mysql_client.execute_batch_sql('data/test_forward_numeric_full.sql')
    self.assertEqual(ret, 0)

    # create collection
    forward_columns=['f1', 'f2', 'f3', 'f4', 'f5', 'f6', 'f7']
    schema = self.create_schema(collection_name,
                                repository_table=collection_name,
                                index_columns=["column1"],
                                forward_columns=forward_columns,
                                index_dimensions=[16])
    status = self.client.create_collection(schema)
    self.assertEqual(status.code, 0)

    time.sleep(5)

    # query result
    status, response = self.query(collection_name)
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    total = 4
    self.assertEqual(len(documents), total)
    for i in range(0, total):
      self.assertEqual(documents[i].primary_key, i + 1)
      self.assertEqual(documents[i].score, i * i)
      self.assertEqual(documents[i].forward_column_values['f1'], 121 + i)
      self.assertEqual(documents[i].forward_column_values['f2'], 20001 + i)
      self.assertEqual(documents[i].forward_column_values['f3'], 65601 + i)
      self.assertEqual(documents[i].forward_column_values['f4'], 2000000001 + i)
      self.assertEqual(documents[i].forward_column_values['f5'], 8000000001 + i)
      self.assertAlmostEqual(documents[i].forward_column_values['f6'], 1.1234 + i, delta=0.0001)
      self.assertAlmostEqual(documents[i].forward_column_values['f7'], 1.11223344 + i, delta=0.00001)

    # prepare increment table data
    ret = self.mysql_client.execute_batch_sql('data/test_forward_numeric_inc.sql')
    self.assertEqual(ret, 0)
    time.sleep(6)

    # query result
    status, response = self.query(collection_name)
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    total = 6
    self.assertEqual(len(documents), total)
    for i in range(1, total + 1):
      self.assertEqual(documents[i - 1].primary_key, i + 1)
      self.assertEqual(documents[i - 1].score, i * i)
      self.assertEqual(documents[i - 1].forward_column_values['f1'], 121 + i)
      self.assertEqual(documents[i - 1].forward_column_values['f2'], 20002 + i)
      self.assertEqual(documents[i - 1].forward_column_values['f3'], 65601 + i)
      self.assertEqual(documents[i - 1].forward_column_values['f4'], 2000000001 + i)
      self.assertEqual(documents[i - 1].forward_column_values['f5'], 8000000001 + i)
      self.assertAlmostEqual(documents[i - 1].forward_column_values['f6'], 1.1234 + i, delta=0.0001)
      self.assertAlmostEqual(documents[i - 1].forward_column_values['f7'], 1.11223344 + i, delta=0.00001)

  def test_forward_with_date_and_time(self):
    collection_name = 'test_forward_with_date_and_time'

    # prepare full table data
    ret = self.mysql_client.execute_batch_sql('data/test_forward_date_and_time_full.sql')
    self.assertEqual(ret, 0)

    # create collection
    forward_columns=['f1', 'f2', 'f3', 'f4', 'f6']
    schema = self.create_schema(collection_name,
                                repository_table=collection_name,
                                index_columns=["column1"],
                                forward_columns=forward_columns,
                                index_dimensions=[16])
    status = self.client.create_collection(schema)
    self.assertEqual(status.code, 0)

    time.sleep(5)

    # query result
    status, response = self.query(collection_name)
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    total = 4
    self.assertEqual(len(documents), total)
    for i in range(0, total):
      self.assertEqual(documents[i].primary_key, i + 1)
      self.assertEqual(documents[i].score, i * i)
      self.assertEqual(documents[i].forward_column_values['f1'], '2021-01-1' + str(i + 2))
      self.assertEqual(documents[i].forward_column_values['f2'], '13:00:0' + str(i + 1))
      self.assertEqual(documents[i].forward_column_values['f3'], '2021-01-1%d 13:00:0%d' % (i + 2, i + 1))
      self.assertEqual(documents[i].forward_column_values['f4'], '2021-01-1%d 13:00:00' % (i + 2))
      self.assertEqual(documents[i].forward_column_values['f6'], '2021')

    # prepare increment table data
    ret = self.mysql_client.execute_batch_sql('data/test_forward_date_and_time_inc.sql')
    self.assertEqual(ret, 0)
    time.sleep(6)

    # query result
    status, response = self.query(collection_name)
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    total = 7
    self.assertEqual(len(documents), total)
    for i in range(1, total + 1):
      self.assertEqual(documents[i-1].primary_key, i + 1)
      self.assertEqual(documents[i-1].score, i * i)
      self.assertEqual(documents[i-1].forward_column_values['f1'], '2021-01-1' + str(i + 2))
      self.assertEqual(documents[i-1].forward_column_values['f2'], '13:00:0' + str(i + 1))
      self.assertEqual(documents[i-1].forward_column_values['f3'], '2021-01-1%d 13:00:0%d' % (i + 2, i + 1))
      self.assertEqual(documents[i-1].forward_column_values['f4'], '2021-01-1%d 13:00:00' % (i + 2))
      self.assertEqual(documents[i-1].forward_column_values['f6'], '2021')

  def test_forward_with_date_and_time_included_fracation(self):
    collection_name = 'test_forward_with_date_and_time_frac'

    # prepare full table data
    ret = self.mysql_client.execute_batch_sql('data/test_forward_date_and_time_frac_full.sql')
    self.assertEqual(ret, 0)

    # create collection
    forward_columns=['f1', 'f2', 'f3', 'f4', 'f6']
    schema = self.create_schema(collection_name,
                                repository_table=collection_name,
                                index_columns=["column1"],
                                forward_columns=forward_columns,
                                index_dimensions=[16])
    status = self.client.create_collection(schema)
    self.assertEqual(status.code, 0)

    time.sleep(5)

    # query result
    status, response = self.query(collection_name)
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    total = 4
    self.assertEqual(len(documents), total)
    for i in range(0, total):
      self.assertEqual(documents[i].primary_key, i + 1)
      self.assertEqual(documents[i].score, i * i)
      self.assertEqual(documents[i].forward_column_values['f1'], '2021-01-1' + str(i + 2))
      self.assertEqual(documents[i].forward_column_values['f2'], '13:00:0%d.9' %(i + 1))
      self.assertEqual(documents[i].forward_column_values['f3'], '2021-01-1%d 13:00:0%d.123' % (i + 2, i + 1))
      self.assertEqual(documents[i].forward_column_values['f4'], '2021-01-1%d 13:00:00.123456' % (i + 2))
      self.assertEqual(documents[i].forward_column_values['f6'], '2021')

    # prepare increment table data
    ret = self.mysql_client.execute_batch_sql('data/test_forward_date_and_time_frac_inc.sql')
    self.assertEqual(ret, 0)
    time.sleep(6)

    # query result
    status, response = self.query(collection_name)
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    total = 7
    self.assertEqual(len(documents), total)
    for i in range(1, total + 1):
      self.assertEqual(documents[i-1].primary_key, i + 1)
      self.assertEqual(documents[i-1].score, i * i)
      self.assertEqual(documents[i-1].forward_column_values['f1'], '2021-01-1' + str(i + 2))
      self.assertEqual(documents[i-1].forward_column_values['f2'], '13:00:0%d.9' %(i + 1))
      self.assertEqual(documents[i-1].forward_column_values['f3'], '2021-01-1%d 13:00:0%d.123' % (i + 2, i + 1))
      self.assertEqual(documents[i-1].forward_column_values['f4'], '2021-01-1%d 13:00:00.123456' % (i + 2))
      self.assertEqual(documents[i-1].forward_column_values['f6'], '2021')

  def test_forward_with_char_and_varchar(self):
    collection_name = 'test_forward_with_char_and_varchar'

    # prepare full table data
    ret = self.mysql_client.execute_batch_sql('data/test_forward_with_char_and_varchar_full.sql')
    self.assertEqual(ret, 0)

    # create collection
    forward_columns=['f1', 'f2', 'f3', 'f4']
    schema = self.create_schema(collection_name,
                                repository_table=collection_name,
                                index_columns=["column1"],
                                forward_columns=forward_columns,
                                index_dimensions=[16])
    status = self.client.create_collection(schema)
    self.assertEqual(status.code, 0)

    time.sleep(5)

    # query result
    status, response = self.query(collection_name)
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    total = 4
    self.assertEqual(len(documents), total)
    for i in range(0, total):
      self.assertEqual(documents[i].primary_key, i + 1)
      self.assertEqual(documents[i].score, i * i)
      self.assertEqual(documents[i].forward_column_values['f1'], (str(i + 1) * 16))
      self.assertEqual(documents[i].forward_column_values['f2'], (str(i + 1) * 4))
      self.assertEqual(documents[i].forward_column_values['f3'], (str(i + 1) * 3))
      self.assertEqual(documents[i].forward_column_values['f4'], (str(i + 1) * 257))

    # prepare increment table data
    ret = self.mysql_client.execute_batch_sql('data/test_forward_with_char_and_varchar_inc.sql')
    self.assertEqual(ret, 0)
    time.sleep(6)

    # query result
    status, response = self.query(collection_name)
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    total = 6
    self.assertEqual(len(documents), total)
    for i in range(0, total):
      self.assertEqual(documents[i].primary_key, i + 1)
      self.assertEqual(documents[i].score, i * i)
      self.assertEqual(documents[i].forward_column_values['f1'], (str(i + 1) * 16))
      self.assertEqual(documents[i].forward_column_values['f2'], (str(i + 1) * 4))
      self.assertEqual(documents[i].forward_column_values['f3'], (str(i + 1) * 3))
      self.assertEqual(documents[i].forward_column_values['f4'], (str(i + 1) * 257))

  def test_forward_with_text(self):
    collection_name = 'test_forward_with_text'

    # prepare full table data
    ret = self.mysql_client.execute_batch_sql('data/test_forward_with_text_full.sql')
    self.assertEqual(ret, 0)

    # create collection
    forward_columns=['f1', 'f2', 'f3', 'f4']
    schema = self.create_schema(collection_name,
                                repository_table=collection_name,
                                index_columns=["column1"],
                                forward_columns=forward_columns,
                                index_dimensions=[16])
    status = self.client.create_collection(schema)
    self.assertEqual(status.code, 0)

    time.sleep(5)

    # query result
    status, response = self.query(collection_name)
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    total = 4
    self.assertEqual(len(documents), total)
    for i in range(0, total):
      self.assertEqual(documents[i].primary_key, i + 1)
      self.assertEqual(documents[i].score, i * i)
      self.assertEqual(documents[i].forward_column_values['f1'], (str(i + 1) * 16))
      self.assertEqual(documents[i].forward_column_values['f2'], (str(i + 1) * 4))
      self.assertEqual(documents[i].forward_column_values['f3'], (str(i + 1) * 3))
      self.assertEqual(documents[i].forward_column_values['f4'], (str(i + 1) * 257))

    # prepare increment table data
    ret = self.mysql_client.execute_batch_sql('data/test_forward_with_text_inc.sql')
    self.assertEqual(ret, 0)
    time.sleep(6)

    # query result
    status, response = self.query(collection_name)
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    total = 6
    self.assertEqual(len(documents), total)
    for i in range(0, total):
      self.assertEqual(documents[i].primary_key, i + 1)
      self.assertEqual(documents[i].score, i * i)
      self.assertEqual(documents[i].forward_column_values['f1'], (str(i + 1) * 16))
      self.assertEqual(documents[i].forward_column_values['f2'], (str(i + 1) * 4))
      self.assertEqual(documents[i].forward_column_values['f3'], (str(i + 1) * 3))
      self.assertEqual(documents[i].forward_column_values['f4'], (str(i + 1) * 257))

  def test_forward_with_text_gbk(self):
    collection_name = 'test_forward_with_text_gbk'

    # prepare full table data
    ret = self.mysql_client.execute_batch_sql('data/test_forward_with_text_gbk_full.sql')
    self.assertEqual(ret, 0)

    # create collection
    forward_columns=['f1', 'f2', 'f3', 'f4']
    schema = self.create_schema(collection_name,
                                repository_table=collection_name,
                                index_columns=["column1"],
                                forward_columns=forward_columns,
                                index_dimensions=[16])
    status = self.client.create_collection(schema)
    self.assertEqual(status.code, 0)

    time.sleep(5)

    # query result
    status, response = self.query(collection_name)
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    total = 4
    self.assertEqual(len(documents), total)
    for i in range(0, total):
      self.assertEqual(documents[i].primary_key, i + 1)
      self.assertEqual(documents[i].score, i * i)
      self.assertEqual(documents[i].forward_column_values['f1'], ('我是向量检索引擎'))
      self.assertEqual(documents[i].forward_column_values['f2'], (str(i + 1) * 4 + '你'))
      self.assertEqual(documents[i].forward_column_values['f3'], (str(i + 1) * 3 + '你'))
      self.assertEqual(documents[i].forward_column_values['f4'], (str(i + 1) * 257 + '你'))

    # prepare increment table data
    ret = self.mysql_client.execute_batch_sql('data/test_forward_with_text_gbk_inc.sql')
    self.assertEqual(ret, 0)
    time.sleep(6)

    # query result
    status, response = self.query(collection_name)
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    total = 6
    self.assertEqual(len(documents), total)
    for i in range(0, total):
      self.assertEqual(documents[i].primary_key, i + 1)
      self.assertEqual(documents[i].score, i * i)
      self.assertEqual(documents[i].forward_column_values['f1'], ('我是向量检索引擎'))
      self.assertEqual(documents[i].forward_column_values['f2'], (str(i + 1) * 4 + '你'))
      self.assertEqual(documents[i].forward_column_values['f3'], (str(i + 1) * 3 + '你'))
      self.assertEqual(documents[i].forward_column_values['f4'], (str(i + 1) * 257 + '你'))

  def test_forward_with_blob(self):
    collection_name = 'test_forward_with_blob'

    # prepare full table data
    ret = self.mysql_client.execute_batch_sql('data/test_forward_with_blob_full.sql')
    self.assertEqual(ret, 0)

    # create collection
    forward_columns=['f1', 'f2', 'f3', 'f4']
    schema = self.create_schema(collection_name,
                                repository_table=collection_name,
                                index_columns=["column1"],
                                forward_columns=forward_columns,
                                index_dimensions=[16])
    status = self.client.create_collection(schema)
    self.assertEqual(status.code, 0)

    time.sleep(5)

    # query result
    status, response = self.query(collection_name)
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    total = 4
    self.assertEqual(len(documents), total)
    for i in range(0, total):
      self.assertEqual(documents[i].primary_key, i + 1)
      self.assertEqual(documents[i].score, i * i)
      self.assertEqual(documents[i].forward_column_values['f1'], ('\xf1\xf2\xf3\xf4').encode('ISO-8859-1'))
      self.assertEqual(documents[i].forward_column_values['f2'], (str(i + 1) * 4).encode('utf-8'))
      self.assertEqual(documents[i].forward_column_values['f3'], (str(i + 1) * 3).encode('utf-8'))
      self.assertEqual(documents[i].forward_column_values['f4'], (str(i + 1) * 257).encode('utf-8'))

    # prepare increment table data
    ret = self.mysql_client.execute_batch_sql('data/test_forward_with_blob_inc.sql')
    self.assertEqual(ret, 0)
    time.sleep(6)

    # query result
    status, response = self.query(collection_name)
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    total = 6
    self.assertEqual(len(documents), total)
    for i in range(0, total):
      self.assertEqual(documents[i].primary_key, i + 1)
      self.assertEqual(documents[i].score, i * i)
      self.assertEqual(documents[i].forward_column_values['f1'], ('\xf1\xf2\xf3\xf4').encode('ISO-8859-1'))
      self.assertEqual(documents[i].forward_column_values['f2'], (str(i + 1) * 4).encode('utf-8'))
      self.assertEqual(documents[i].forward_column_values['f3'], (str(i + 1) * 3).encode('utf-8'))
      self.assertEqual(documents[i].forward_column_values['f4'], (str(i + 1) * 257).encode('utf-8'))

  def test_forward_with_blob_gbk(self):
    collection_name = 'test_forward_with_blob_gbk'

    # prepare full table data
    ret = self.mysql_client.execute_batch_sql('data/test_forward_with_blob_gbk_full.sql')
    self.assertEqual(ret, 0)

    # create collection
    forward_columns=['f1', 'f2', 'f3', 'f4']
    schema = self.create_schema(collection_name,
                                repository_table=collection_name,
                                index_columns=["column1"],
                                forward_columns=forward_columns,
                                index_dimensions=[16])
    status = self.client.create_collection(schema)
    self.assertEqual(status.code, 0)

    time.sleep(5)

    # query result
    status, response = self.query(collection_name)
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    total = 4
    self.assertEqual(len(documents), total)
    for i in range(0, total):
      self.assertEqual(documents[i].primary_key, i + 1)
      self.assertEqual(documents[i].score, i * i)
      self.assertEqual(documents[i].forward_column_values['f1'], ('我').encode('gbk'))
      self.assertEqual(documents[i].forward_column_values['f2'], ('你' + str(i + 1) * 4).encode('gbk'))
      self.assertEqual(documents[i].forward_column_values['f3'], (str(i + 1) * 3).encode('gbk'))
      self.assertEqual(documents[i].forward_column_values['f4'], (str(i + 1) * 257).encode('gbk'))

    # prepare increment table data
    ret = self.mysql_client.execute_batch_sql('data/test_forward_with_blob_gbk_inc.sql')
    self.assertEqual(ret, 0)
    time.sleep(6)

    # query result
    status, response = self.query(collection_name)
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    total = 6
    self.assertEqual(len(documents), total)
    for i in range(0, total):
      self.assertEqual(documents[i].primary_key, i + 1)
      self.assertEqual(documents[i].score, i * i)
      self.assertEqual(documents[i].forward_column_values['f1'], ('我').encode('gbk'))
      self.assertEqual(documents[i].forward_column_values['f2'], ('你' + str(i + 1) * 4).encode('gbk'))
      self.assertEqual(documents[i].forward_column_values['f3'], (str(i + 1) * 3).encode('gbk'))
      self.assertEqual(documents[i].forward_column_values['f4'], (str(i + 1) * 257).encode('gbk'))

  def test_forward_with_bit(self):
    collection_name = 'test_forward_with_bit'

    # prepare full table data
    ret = self.mysql_client.execute_batch_sql('data/test_forward_with_bit_full.sql')
    self.assertEqual(ret, 0)

    # create collection
    forward_columns=['f1', 'f2', 'f3', 'f4']
    schema = self.create_schema(collection_name,
                                repository_table=collection_name,
                                index_columns=["column1"],
                                forward_columns=forward_columns,
                                index_dimensions=[16])
    status = self.client.create_collection(schema)
    self.assertEqual(status.code, 0)

    time.sleep(5)

    # query result
    status, response = self.query(collection_name)
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    total = 3
    self.assertEqual(len(documents), total)
    for i in range(0, total):
      self.assertEqual(documents[i].primary_key, i + 1)
      self.assertEqual(documents[i].score, i * i)
      self.assertEqual(documents[i].forward_column_values['f1'], i)
      self.assertEqual(documents[i].forward_column_values['f2'], i + 128)
      self.assertEqual(documents[i].forward_column_values['f3'], i + 65535)
      self.assertEqual(documents[i].forward_column_values['f4'], i + 5000000000)

    # prepare increment table data
    ret = self.mysql_client.execute_batch_sql('data/test_forward_with_bit_inc.sql')
    self.assertEqual(ret, 0)
    time.sleep(6)

    # query result
    status, response = self.query(collection_name)
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    total = 6
    self.assertEqual(len(documents), total)
    for i in range(0, total):
      self.assertEqual(documents[i].primary_key, i + 1)
      self.assertEqual(documents[i].score, i * i)
      self.assertEqual(documents[i].forward_column_values['f1'], i)
      self.assertEqual(documents[i].forward_column_values['f2'], i + 128)
      self.assertEqual(documents[i].forward_column_values['f3'], i + 65535)
      self.assertEqual(documents[i].forward_column_values['f4'], i + 5000000000)

  def test_forward_with_binary_and_varbinary(self):
    collection_name = 'test_forward_with_binary_and_varbinary'

    # prepare full table data
    ret = self.mysql_client.execute_batch_sql('data/test_forward_with_binary_and_varbinary_full.sql')
    self.assertEqual(ret, 0)

    # create collection
    forward_columns=['f1', 'f2', 'f3', 'f4']
    schema = self.create_schema(collection_name,
                                repository_table=collection_name,
                                index_columns=["column1"],
                                forward_columns=forward_columns,
                                index_dimensions=[16])
    status = self.client.create_collection(schema)
    self.assertEqual(status.code, 0)

    time.sleep(5)

    # query result
    status, response = self.query(collection_name)
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    total = 4
    self.assertEqual(len(documents), total)
    for i in range(0, total):
      self.assertEqual(documents[i].primary_key, i + 1)
      self.assertEqual(documents[i].score, i * i)
      self.assertEqual(documents[i].forward_column_values['f1'], ('\xf1' + '\x00' * 15).encode('ISO-8859-1'))
      self.assertEqual(documents[i].forward_column_values['f2'], (str(i + 1) * 4 + '\x00' * 60).encode('utf-8'))
      self.assertEqual(documents[i].forward_column_values['f3'], ('\xf1' * 3).encode('ISO-8859-1'))
      self.assertEqual(documents[i].forward_column_values['f4'], (str(i + 1) * 257).encode('utf-8'))

    # prepare increment table data
    ret = self.mysql_client.execute_batch_sql('data/test_forward_with_binary_and_varbinary_inc.sql')
    self.assertEqual(ret, 0)
    time.sleep(6)

    # query result
    status, response = self.query(collection_name)
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    total = 6
    self.assertEqual(len(documents), total)
    for i in range(0, total):
      self.assertEqual(documents[i].primary_key, i + 1)
      self.assertEqual(documents[i].score, i * i)
      self.assertEqual(documents[i].forward_column_values['f1'], ('\xf1' + '\x00' * 15).encode('ISO-8859-1'))
      self.assertEqual(documents[i].forward_column_values['f2'], (str(i + 1) * 4 + '\x00' * 60).encode('utf-8'))
      self.assertEqual(documents[i].forward_column_values['f3'], ('\xf1' * 3).encode('ISO-8859-1'))
      self.assertEqual(documents[i].forward_column_values['f4'], (str(i + 1) * 257).encode('utf-8'))

  def test_forward_with_set_and_enum(self):
    collection_name = 'test_forward_with_set_and_enum'

    # prepare full table data
    ret = self.mysql_client.execute_batch_sql('data/test_forward_with_set_and_enum_full.sql')
    self.assertEqual(ret, 0)

    # create collection
    forward_columns=['f1', 'f2', 'f3', 'f4']
    schema = self.create_schema(collection_name,
                                repository_table=collection_name,
                                index_columns=["column1"],
                                forward_columns=forward_columns,
                                index_dimensions=[16])
    status = self.client.create_collection(schema)
    self.assertEqual(status.code, 0)

    time.sleep(5)

    # query result
    status, response = self.query(collection_name)
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    total = 4
    self.assertEqual(len(documents), total)
    for i in range(0, total):
      self.assertEqual(documents[i].primary_key, i + 1)
      self.assertEqual(documents[i].score, i * i)
      self.assertEqual(documents[i].forward_column_values['f1'], (1 << i) + 128)
      self.assertEqual(documents[i].forward_column_values['f2'], (1 << i))
      self.assertEqual(documents[i].forward_column_values['f3'], (i + 1))
      self.assertEqual(documents[i].forward_column_values['f4'], (i + 1))

    # prepare increment table data
    ret = self.mysql_client.execute_batch_sql('data/test_forward_with_set_and_enum_inc.sql')
    self.assertEqual(ret, 0)
    time.sleep(6)

    # query result
    status, response = self.query(collection_name)
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    total = 6
    self.assertEqual(len(documents), total)
    for i in range(0, total):
      self.assertEqual(documents[i].primary_key, i + 1)
      self.assertEqual(documents[i].score, i * i)
      self.assertEqual(documents[i].forward_column_values['f1'], (1 << i) + 128)
      self.assertEqual(documents[i].forward_column_values['f2'], (1 << i))
      self.assertEqual(documents[i].forward_column_values['f3'], (i + 1))
      self.assertEqual(documents[i].forward_column_values['f4'], (i + 1))

  def test_forward_with_json(self):
    collection_name = 'test_forward_with_json'

    # prepare full table data
    ret = self.mysql_client.execute_batch_sql('data/test_forward_with_json_full.sql')
    self.assertEqual(ret, 0)

    # create collection
    forward_columns=['f1', 'f2', 'f3', 'f4']
    schema = self.create_schema(collection_name,
                                repository_table=collection_name,
                                index_columns=["column1"],
                                forward_columns=forward_columns,
                                index_dimensions=[16])
    status = self.client.create_collection(schema)
    self.assertEqual(status.code, 0)

    time.sleep(5)

    # query result
    status, response = self.query(collection_name)
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    total = 4
    self.assertEqual(len(documents), total)
    for i in range(0, total):
      self.assertEqual(documents[i].primary_key, i + 1)
      self.assertEqual(documents[i].score, i * i)
      self.assertEqual(documents[i].forward_column_values['f1'], (str(i + 1) * 16))
#      self.assertEqual(documents[i].forward[1].key, 'f2')
#      self.assertEqual(documents[i].forward_column_values['f2'], (str(i + 1) * 4).encode('utf-8'))
      self.assertEqual(documents[i].forward_column_values['f3'], (str(i + 1) * 3))
      self.assertEqual(documents[i].forward_column_values['f4'], (str(i + 1) * 257))

    # prepare increment table data
    ret = self.mysql_client.execute_batch_sql('data/test_forward_with_json_inc.sql')
    self.assertEqual(ret, 0)
    time.sleep(6)

    # query result
    status, response = self.query(collection_name)
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    total = 6
    self.assertEqual(len(documents), total)
    for i in range(0, total):
      self.assertEqual(documents[i].primary_key, i + 1)
      self.assertEqual(documents[i].score, i * i)
      self.assertEqual(documents[i].forward_column_values['f1'], (str(i + 1) * 16))
#      self.assertEqual(documents[i].forward_column_values['f2'], (str(i + 1) * 4).encode('utf-8'))
      self.assertEqual(documents[i].forward_column_values['f3'], (str(i + 1) * 3))
      self.assertEqual(documents[i].forward_column_values['f4'], (str(i + 1) * 257))

  def test_forward_with_geometry(self):
    collection_name = 'test_forward_with_geometry'

    # prepare full table data
    ret = self.mysql_client.execute_batch_sql('data/test_forward_with_geometry_full.sql')
    self.assertEqual(ret, 0)

    # create collection
    forward_columns=['f1', 'f2', 'f3', 'f4']
    schema = self.create_schema(collection_name,
                                repository_table=collection_name,
                                index_columns=["column1"],
                                forward_columns=forward_columns,
                                index_dimensions=[16])
    status = self.client.create_collection(schema)
    self.assertEqual(status.code, 0)

    time.sleep(5)

    # query result
    status, response = self.query(collection_name)
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    total = 4
    self.assertEqual(len(documents), total)
    for i in range(0, total):
      self.assertEqual(documents[i].primary_key, i + 1)
      self.assertEqual(documents[i].score, i * i)
      self.assertEqual(documents[i].forward_column_values['f1'], (str(i + 1) * 16))
#      self.assertEqual(documents[i].forward_column_values['f2'], (str(i + 1) * 4).encode('utf-8'))
      self.assertEqual(documents[i].forward_column_values['f3'], (str(i + 1) * 3))
      self.assertEqual(documents[i].forward_column_values['f4'], (str(i + 1) * 257))

    # prepare increment table data
    ret = self.mysql_client.execute_batch_sql('data/test_forward_with_geometry_inc.sql')
    self.assertEqual(ret, 0)
    time.sleep(6)

    # query result
    status, response = self.query(collection_name)
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    total = 6
    self.assertEqual(len(documents), total)
    for i in range(0, total):
      self.assertEqual(documents[i].primary_key, i + 1)
      self.assertEqual(documents[i].score, i * i)
      self.assertEqual(documents[i].forward_column_values['f1'], (str(i + 1) * 16))
#      self.assertEqual(documents[i].forward_column_values['f2'], (str(i + 1) * 4).encode('utf-8'))
      self.assertEqual(documents[i].forward_column_values['f3'], (str(i + 1) * 3))
      self.assertEqual(documents[i].forward_column_values['f4'], (str(i + 1) * 257))

  def test_forward_with_decimal(self):
    collection_name = 'test_forward_with_decimal'

    # prepare full table data
    ret = self.mysql_client.execute_batch_sql('data/test_forward_with_decimal_full.sql')
    self.assertEqual(ret, 0)

    # create collection
    forward_columns=['f1', 'f2', 'f3', 'f4']
    schema = self.create_schema(collection_name,
                                repository_table=collection_name,
                                index_columns=["column1"],
                                forward_columns=forward_columns,
                                index_dimensions=[16])
    status = self.client.create_collection(schema)
    self.assertEqual(status.code, 0)

    time.sleep(5)

    # query result
    status, response = self.query(collection_name)
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    total = 4
    self.assertEqual(len(documents), total)
    for i in range(0, total):
      self.assertEqual(documents[i].primary_key, i + 1)
      self.assertEqual(documents[i].score, i * i)
      self.assertEqual(documents[i].forward_column_values['f1'], (str(i + 1) * 16))
      self.assertEqual(documents[i].forward_column_values['f2'], '12345.%d123456789' % (i))
      self.assertEqual(documents[i].forward_column_values['f3'], (str(i + 1) * 3))
      self.assertEqual(documents[i].forward_column_values['f4'], (str(i + 1) * 257))

    # prepare increment table data
    ret = self.mysql_client.execute_batch_sql('data/test_forward_with_decimal_inc.sql')
    self.assertEqual(ret, 0)
    time.sleep(6)

    # query result
    status, response = self.query(collection_name)
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    total = 6
    self.assertEqual(len(documents), total)
    for i in range(0, total):
      self.assertEqual(documents[i].primary_key, i + 1)
      self.assertEqual(documents[i].score, i * i)
      self.assertEqual(documents[i].forward_column_values['f1'], (str(i + 1) * 16))
      self.assertEqual(documents[i].forward_column_values['f2'], '12345.%d123456789' % (i))
      self.assertEqual(documents[i].forward_column_values['f3'], (str(i + 1) * 3))
      self.assertEqual(documents[i].forward_column_values['f4'], (str(i + 1) * 257))

  def test_forward_with_types_null(self):
    collection_name = 'test_forward_with_types_null'

    # prepare full table data
    ret = self.mysql_client.execute_batch_sql('data/test_forward_with_types_null_full.sql')
    self.assertEqual(ret, 0)

    # create collection
    forward_columns=['f1', 'f2', 'f3', 'f4', 'f5', 'f6', 'f7', 'f8', 'f9', 'f10',
                     'f11', 'f12', 'f13', 'f14', 'f15', 'f16', 'f17', 'f18', 'f19']
    schema = self.create_schema(collection_name,
                                repository_table=collection_name,
                                index_columns=["f20"],
                                forward_columns=forward_columns,
                                index_dimensions=[16])
    status = self.client.create_collection(schema)
    self.assertEqual(status.code, 0)

    time.sleep(5)

    # query result
    status, response = self.query(collection_name, column_name='f20')
    logging.info("query status: %s", status)
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    total = 4
    self.assertEqual(len(documents), total)
    for i in range(0, total):
      self.assertEqual(documents[i].primary_key, i + 1)
      self.assertEqual(documents[i].score, i * i)
      self.assertEqual(documents[i].forward_column_values['f1'], (str(i + 1) * 16))

    # prepare increment table data
    ret = self.mysql_client.execute_batch_sql('data/test_forward_with_types_null_inc.sql')
    self.assertEqual(ret, 0)
    time.sleep(6)

    # query result
    status, response = self.query(collection_name, column_name='f20')
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    total = 7
    self.assertEqual(len(documents), total)
    for i in range(0, total):
      self.assertEqual(documents[i].primary_key, i + 2)
      self.assertEqual(documents[i].score, (i + 1) * (i + 1))
      self.assertEqual(documents[i].forward_column_values['f1'], (str(i + 2) * 16))

  def test_forward_with_empty_value(self):
    collection_name = 'test_forward_with_empty_value'

    # prepare full table data
    ret = self.mysql_client.execute_batch_sql('data/test_forward_with_empty_value_full.sql')
    self.assertEqual(ret, 0)

    # create collection
    forward_columns=['f1', 'f2', 'f3', 'f4', 'f5', 'f6', 'f7', 'f8', 'f9', 'f10',
                     'f11', 'f12', 'f13', 'f14', 'f15', 'f16', 'f17', 'f18', 'f19']
    schema = self.create_schema(collection_name,
                                repository_table=collection_name,
                                index_columns=["f20"],
                                forward_columns=forward_columns,
                                index_dimensions=[16])
    status = self.client.create_collection(schema)
    self.assertEqual(status.code, 0)

    time.sleep(5)

    # query result
    status, response = self.query(collection_name, column_name='f20')
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    total = 4
    self.assertEqual(len(documents), total)
    for i in range(0, total):
      self.assertEqual(documents[i].primary_key, i + 1)
      self.assertEqual(documents[i].score, i * i)
      self.assertEqual(documents[i].forward_column_values['f1'],
                       str(i + 1) * 16)
      self.assertEqual(documents[i].forward_column_values['f2'],
                       "")
      self.assertEqual(documents[i].forward_column_values['f3'],
                       "")
      self.assertEqual(documents[i].forward_column_values['f10'],
                       "")
      self.assertEqual(documents[i].forward_column_values['f11'],
                       "")
      self.assertEqual(documents[i].forward_column_values['f12'].decode("utf-8"),
                       "")
      self.assertEqual(documents[i].forward_column_values['f13'], 0)
      self.assertEqual(documents[i].forward_column_values['f14'].decode("utf-8"),
                       "\x00\x00\x00\x00\x00\x00\x00\x00")
      self.assertEqual(documents[i].forward_column_values['f15'].decode("utf-8"),
                       "")
      self.assertEqual(documents[i].forward_column_values['f16'], 0)

    # prepare increment table data
    ret = self.mysql_client.execute_batch_sql('data/test_forward_with_empty_value_inc.sql')
    self.assertEqual(ret, 0)
    time.sleep(6)

    # query result
    status, response = self.query(collection_name, column_name='f20')
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    total = 7
    self.assertEqual(len(documents), total)
    for i in range(0, total):
      self.assertEqual(documents[i].primary_key, i + 2)
      self.assertEqual(documents[i].score, (i + 1) * (i + 1))
      self.assertEqual(documents[i].forward_column_values['f1'],
                       str(i + 2) * 16)
      self.assertEqual(documents[i].forward_column_values['f2'],
                       "")
      self.assertEqual(documents[i].forward_column_values['f3'],
                       "")
      self.assertEqual(documents[i].forward_column_values['f10'],
                       "")
      self.assertEqual(documents[i].forward_column_values['f11'],
                       "")
      self.assertEqual(documents[i].forward_column_values['f12'].decode("utf-8"),
                       "")
      self.assertEqual(documents[i].forward_column_values['f13'], 0)
      self.assertEqual(documents[i].forward_column_values['f14'].decode("utf-8"),
                       "\x00\x00\x00\x00\x00\x00\x00\x00")
      self.assertEqual(documents[i].forward_column_values['f15'].decode("utf-8"),
                       "")
      self.assertEqual(documents[i].forward_column_values['f16'], 0)

  def test_repository_restart(self):
    # prepare full table data
    ret = self.mysql_client.execute_batch_sql('data/test_repository_restart_full.sql')
    self.assertEqual(ret, 0)

    # create collection
    collection_name = 'test_repository_restart'
    schema = self.create_schema(collection_name,
                                repository_table=collection_name,
                                index_columns=["column1"],
                                index_dimensions=[16])
    status = self.client.create_collection(schema)
    self.assertEqual(status.code, 0)

    time.sleep(5)

    # query result
    status, response = self.query(collection_name)
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    count = 2
    self.assertEqual(len(documents), count)
    for i in range(0, count):
      self.assertEqual(documents[i].primary_key, i + 1)
      self.assertEqual(documents[i].score, i * i)
      self.assertAlmostEqual(documents[i].forward_column_values['col_a'],
                             11.111 + i, delta=0.000001)
      self.assertEqual(documents[i].forward_column_values['col_b'], 100 * (i + 1))

    # get latest lsn
    latest_lsn = self.get_latest_lsn(collection_name)
    self.assertEqual(latest_lsn.lsn, 2)

    # stop mysql repo
    self.server_utils.stop_mysql_repo()

    # prepare table data
    ret = self.mysql_client.execute_batch_sql('data/test_repository_restart_full_1.sql')
    self.assertEqual(ret, 0)

    # start mysql repo
    self.server_utils.start_mysql_repo()

    time.sleep(5)

    # query result
    status, response = self.query(collection_name)
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    count = 4
    self.assertEqual(len(documents), count)
    for i in range(0, count):
      self.assertEqual(documents[i].primary_key, i + 1)
      self.assertEqual(documents[i].score, i * i)
      self.assertAlmostEqual(documents[i].forward_column_values['col_a'],
                             11.111 + i, delta=0.000001)
      self.assertEqual(documents[i].forward_column_values['col_b'], 100 * (i + 1))

    # get latest lsn
    latest_lsn = self.get_latest_lsn(collection_name)
    self.assertEqual(latest_lsn.lsn, 4)

    # prepare increment table data
    ret = self.mysql_client.execute_batch_sql('data/test_repository_restart_inc.sql')
    self.assertEqual(ret, 0)
    time.sleep(6)

    # query result
    status, response = self.query(collection_name)
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    count = 6
    for i in range(0, count):
      self.assertEqual(documents[i].primary_key, i + 1)
      self.assertEqual(documents[i].score, i * i)
      self.assertAlmostEqual(documents[i].forward_column_values['col_a'],
                             11.111 + i, delta=0.000001)
      self.assertEqual(documents[i].forward_column_values['col_b'], 100 * (i + 1))

    # get latest lsn
    latest_lsn = self.get_latest_lsn(collection_name)
    self.assertEqual(latest_lsn.lsn, 4)

    # restart mysql repository
    self.server_utils.stop_mysql_repo()
    time.sleep(2)
    self.server_utils.start_mysql_repo()

    # prepare increment table data
    ret = self.mysql_client.execute_batch_sql('data/test_repository_restart_inc_1.sql')
    self.assertEqual(ret, 0)
    time.sleep(6)

    # query result
    status, response = self.query(collection_name)
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    count = 7
    self.assertEqual(len(documents), count)
    for i in range(0, count):
      self.assertEqual(documents[i].primary_key, i + 1)
      self.assertEqual(documents[i].score, i * i)
      self.assertAlmostEqual(documents[i].forward_column_values['col_a'],
                             11.111 + i, delta=0.000001)
      self.assertEqual(documents[i].forward_column_values['col_b'], 100 * (i + 1))

    # get latest lsn
    latest_lsn = self.get_latest_lsn(collection_name)
    self.assertEqual(latest_lsn.lsn, 4)

  def test_proxima_be_restart_with_collection_empty(self):
    # prepare full table data
    ret = self.mysql_client.execute_batch_sql('data/test_proxima_be_restart_with_collection_empty_meta.sql')
    self.assertEqual(ret, 0)

    # create collection
    collection_name = 'test_proxima_be_restart_with_collection_empty'
    schema = self.create_schema(collection_name,
                                repository_table=collection_name,
                                index_columns=["column1"],
                                index_dimensions=[16])
    status = self.client.create_collection(schema)
    self.assertEqual(status.code, 0)

    # stop the proxima be server
    self.server_utils.stop_proxima_be()

    # start the proxima be server
    self.server_utils.start_proxima_be()

    time.sleep(5)

    # prepare full table data
    ret = self.mysql_client.execute_batch_sql('data/test_proxima_be_restart_with_collection_empty_full.sql')
    self.assertEqual(ret, 0)

    time.sleep(5)

    # prepare inc table data
    ret = self.mysql_client.execute_batch_sql('data/test_proxima_be_restart_with_collection_empty_inc.sql')
    self.assertEqual(ret, 0)

    time.sleep(3)

    # query result
    status, response = self.query(collection_name)
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    count = 2
    self.assertEqual(len(documents), count)
    for i in range(0, count):
      self.assertEqual(documents[i].primary_key, i + 1)
      self.assertEqual(documents[i].score, i * i)
      self.assertAlmostEqual(documents[i].forward_column_values['col_a'],
                             11.111 + i, delta=0.000001)
      self.assertEqual(documents[i].forward_column_values['col_b'], 100 * (i + 1))

    # get latest lsn
    latest_lsn = self.get_latest_lsn(collection_name)
    self.assertEqual(latest_lsn.lsn, 2)
    context = latest_lsn.context
    arr = context.split(';')
    self.assertEqual(len(arr), 4)
    self.assertEqual(arr[3].strip(), '1')

  def test_proxima_be_restart_with_full_stage(self):
    # prepare full table data
    ret = self.mysql_client.execute_batch_sql('data/test_proxima_be_restart_with_full_stage_full.sql')
    self.assertEqual(ret, 0)

    # create collection
    collection_name = 'test_proxima_be_restart_with_full_stage'
    schema = self.create_schema(collection_name,
                                repository_table=collection_name,
                                index_columns=["column1"],
                                index_dimensions=[16])

    status = self.client.create_collection(schema)
    self.assertEqual(status.code, 0)

    time.sleep(1)

    # stop the proxima be server
    self.server_utils.stop_proxima_be()

    # logging.info("<<<<<<<<<<< before mysql_repo >>>>>>>>>>>")
    # logging.info("%s", self.get_content('mysql_repo.log.INFO'))
    # logging.info("<<<<<<<<<<< before proxima_be >>>>>>>>>>>")
    # logging.info("%s", self.get_content('proxima_be.log.INFO'))
    # start the proxima be server
    self.server_utils.start_proxima_be()

    time.sleep(5)

    # logging.info("<<<<<<<<<<< after mysql_repo >>>>>>>>>>>")
    # logging.info("%s", self.get_content('mysql_repo.log.INFO'))
    # logging.info("<<<<<<<<<<< after proxima_be >>>>>>>>>>>")
    # logging.info("%s", self.get_content('proxima_be.log.INFO'))

    # query result
    prev_count = 0
    while True:
      status, response = self.query(collection_name, topk=10000)
      self.assertTrue(status.ok())
      logging.info("query result: %s", response)
      results = response.results
      self.assertEqual(len(results), 1)
      documents = results[0]
      count = 10000
      logging.info("actual: %d, prev: %d", len(documents), prev_count)
      if len(documents) < count and len(documents) > prev_count:
        prev_count = len(documents)
        time.sleep(10)
        continue
      self.assertEqual(len(documents), count)
      for i in range(0, count):
        self.assertEqual(documents[i].primary_key, i + 1)
        self.assertEqual(len(documents[i].forward_column_values), 2)
      break

  def test_proxima_be_restart_with_inc_stage(self):
    # prepare full table data
    ret = self.mysql_client.execute_batch_sql('data/test_proxima_be_restart_with_inc_stage_full.sql')
    self.assertEqual(ret, 0)

    # create collection
    collection_name = 'test_proxima_be_restart_with_inc_stage'
    schema = self.create_schema(collection_name,
                                repository_table=collection_name,
                                index_columns=["column1"],
                                index_dimensions=[16])

    status = self.client.create_collection(schema)
    self.assertEqual(status.code, 0)

    time.sleep(5)

    # prepare inc table data
    ret = self.mysql_client.execute_batch_sql('data/test_proxima_be_restart_with_inc_stage_inc_1.sql')
    self.assertEqual(ret, 0)

    time.sleep(5)

    # query result
    prev_count = 0
    while True:
      status, response = self.query(collection_name, topk=10000)
      self.assertTrue(status.ok())
      logging.info("query result: %s", response)
      results = response.results
      self.assertEqual(len(results), 1)
      documents = results[0]
      count = 400
      logging.info("actual: %d, prev: %d", len(documents), prev_count)
      if len(documents) < count and len(documents) > prev_count:
        prev_count = len(documents)
        time.sleep(5)
        continue
      self.assertEqual(len(documents), count)
      for i in range(0, count):
        self.assertEqual(documents[i].primary_key, i + 1)
        self.assertEqual(len(documents[i].forward_column_values), 2)
      break

    # stop the proxima be server
    self.server_utils.stop_proxima_be()

    # start the proxima be server
    self.server_utils.start_proxima_be()

    time.sleep(5)

    # prepare inc table data
    ret = self.mysql_client.execute_batch_sql('data/test_proxima_be_restart_with_inc_stage_inc_2.sql')
    self.assertEqual(ret, 0)

    time.sleep(5)

    # query result
    prev_count = 0
    while True:
      status, response = self.query(collection_name, topk=10000)
      self.assertTrue(status.ok())
      logging.info("query result: %s", response)
      results = response.results
      self.assertEqual(len(results), 1)
      documents = results[0]
      count = 2610
      if len(documents) < count and len(documents) > prev_count:
        prev_count = len(documents)
        time.sleep(10)
        continue
      self.assertEqual(len(documents), count)
      for i in range(0, count):
        self.assertEqual(documents[i].primary_key, i + 1)
        self.assertEqual(len(documents[i].forward_column_values), 2)
      break

  # def test_update_collection(self):
  #   # prepare full table data
  #   ret = self.mysql_client.execute_batch_sql('data/test_update_collection_full.sql')
  #   self.assertEqual(ret, 0)

  #   # create collection
  #   collection_name = 'test_update_collection'
  #   schema = self.create_schema(collection_name,
  #                               repository_table=collection_name,
  #                               index_columns=["column1"],
  #                               index_dimensions=[16])
  #   status = self.client.create_collection(schema)
  #   self.assertEqual(status.code, 0)
  #   collection_meta = data["collection"]

  #   time.sleep(5)

  #   # query result
  #   status, response = self.query(collection_name)
  #   self.assertTrue(status.ok())
  #   logging.info("query result: %s", response)
  #   results = response.results
  #   self.assertEqual(len(results), 1)
  #   documents = results[0]
  #   count = 2
  #   self.assertEqual(len(documents), count)
  #   for i in range(0, count):
  #     self.assertEqual(documents[i].primary_key, i + 1)
  #     self.assertEqual(documents[i].score, i * i)
  #     self.assertAlmostEqual(documents[i].forward_column_values['col_a'],
  #                            11.111 + i, delta=0.000001)
  #     self.assertEqual(documents[i].forward_column_values['col_b'], 100 * (i + 1))

  #   # get latest lsn
  #   latest_lsn = self.get_latest_lsn(collection_name)
  #   self.assertEqual(latest_lsn.lsn, 2)

  #   # suspend collection
  #   status = self.client.suspend_collection(collection_name)
  #   logging.info("suspend collection result: %s", data)
  #   self.assertEqual(status.code, 0)

  #   # prepare table data
  #   ret = self.mysql_client.execute_batch_sql('data/test_update_collection_inc.sql')
  #   self.assertEqual(ret, 0)
  #   time.sleep(3)

  #   # update schema
  #   self.creator.update_schema(schema, collection_meta)
  #   status = self.client.update_collection(schema)
  #   logging.info("update schema: %s", schema)
  #   logging.info("update result: %s", data)
  #   self.assertEqual(status.code, 0)

  #   # resume collection
  #   status = self.client.resume_collection(collection_name)
  #   logging.info("resume collection result: %s", data)
  #   self.assertEqual(status.code, 0)

  #   time.sleep(5)

  #   # query result
  #   status, response = self.query(collection_name)
  #   self.assertTrue(status.ok())
  #   logging.info("query result: %s", response)
  #   results = response.results
  #   self.assertEqual(len(results), 1)
  #   documents = results[0]
  #   count = 4
  #   self.assertEqual(len(documents), count)
  #   for i in range(0, count):
  #     self.assertEqual(documents[i].primary_key, i + 1)
  #     self.assertEqual(documents[i].score, i * i)
  #     self.assertAlmostEqual(documents[i].forward_column_values['col_a'],
  #                            11.111 + i, delta=0.000001)
  #     self.assertEqual(documents[i].forward_column_values['col_b'], 100 * (i + 1))

  #   # get latest lsn
  #   latest_lsn = self.get_latest_lsn(collection_name)
  #   self.assertEqual(latest_lsn.lsn, 4)

  def test_collection_create_and_remove(self):
    # create collection
    collection_name = 'test_collection_create_and_remove'
    schema = self.create_schema(collection_name,
                                repository_table=collection_name,
                                index_columns=["column1"],
                                index_dimensions=[16])
    status = self.client.create_collection(schema)
    self.assertEqual(status.code, 0)

    # send data
    ret = self.mysql_client.execute_batch_sql('data/test_collection_create_and_remove_full.sql')
    self.assertEqual(ret, 0)
    time.sleep(5)

    # query result
    status, response = self.query(collection_name)
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    count = 2
    self.assertEqual(len(documents), count)
    for i in range(0, count):
      self.assertEqual(documents[i].primary_key, i + 1)
      self.assertEqual(documents[i].score, i * i)
      self.assertAlmostEqual(documents[i].forward_column_values['col_a'],
                             11.111 + i, delta=0.000001)
      self.assertEqual(documents[i].forward_column_values['col_b'], 100 * (i + 1))

    # remove the collection
    status = self.client.drop_collection(collection_name)
    self.assertEqual(status.code, 0)

    time.sleep(2)

    # recreate the collection
    status = self.client.create_collection(schema)
    self.assertEqual(status.code, 0)

    time.sleep(5)

    # query result
    status, response = self.query(collection_name)
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    count = 2
    self.assertEqual(len(documents), count)
    for i in range(0, count):
      self.assertEqual(documents[i].primary_key, i + 1)
      self.assertEqual(documents[i].score, i * i)
      self.assertAlmostEqual(documents[i].forward_column_values['col_a'],
                             11.111 + i, delta=0.000001)
      self.assertEqual(documents[i].forward_column_values['col_b'], 100 * (i + 1))

  def test_mysql_restart_with_full_mode(self):
    # send data
    ret = self.mysql_client.execute_batch_sql('data/test_mysql_restart_with_full_mode_full.sql')
    self.assertEqual(ret, 0)

    # create collection
    collection_name = 'test_mysql_restart_with_full_mode'
    schema = self.create_schema(collection_name,
                                repository_table=collection_name,
                                index_columns=["column1"],
                                index_dimensions=[16])
    status = self.client.create_collection(schema)
    self.assertEqual(status.code, 0)

    # # suspend collection
    # status = self.client.suspend_collection(collection_name)
    # logging.info("suspend collection result: %s", data)
    # self.assertEqual(status.code, 0)

    # time.sleep(5)

    # query result
    status, response = self.query(collection_name)
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]

    # stop mysql
    self.server_utils.stop_mysql()
    time.sleep(2)

    # start mysql
    self.server_utils.start_mysql()
    time.sleep(2)

    # # resume collection
    # status = self.client.resume_collection(collection_name)
    # logging.info("resume collection result: %s", data)
    # self.assertEqual(status.code, 0)

    time.sleep(10)

    # query result
    status, response = self.query(collection_name, topk=300)
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    count = 300
    self.assertEqual(len(documents), count)
    for i in range(0, count):
      self.assertEqual(documents[i].primary_key, i + 1)
      self.assertEqual(documents[i].score, i * i)
      self.assertAlmostEqual(documents[i].forward_column_values['col_a'],
                             11.111 + i, delta=0.001)
      self.assertEqual(documents[i].forward_column_values['col_b'], 100 * (i + 1))

  def test_mysql_restart_with_inc_mode(self):
    # send full data
    ret = self.mysql_client.execute_batch_sql('data/test_mysql_restart_with_inc_mode_full.sql')
    self.assertEqual(ret, 0)

    # create collection
    collection_name = 'test_mysql_restart_with_inc_mode'
    schema = self.create_schema(collection_name,
                                repository_table=collection_name,
                                index_columns=["column1"],
                                index_dimensions=[16])
    status = self.client.create_collection(schema)
    self.assertEqual(status.code, 0)

    time.sleep(5)

    # query result
    status, response = self.query(collection_name, topk=300)
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    count = 100
    self.assertEqual(len(documents), count)
    for i in range(0, count):
      self.assertEqual(documents[i].primary_key, i + 1)
      self.assertEqual(documents[i].score, i * i)
      self.assertAlmostEqual(documents[i].forward_column_values['col_a'],
                             11.111 + i, delta=0.001)
      self.assertEqual(documents[i].forward_column_values['col_b'], 100 * (i + 1))

    # send inc1 data
    ret = self.mysql_client.execute_batch_sql('data/test_mysql_restart_with_inc_mode_inc_1.sql')
    self.assertEqual(ret, 0)

    time.sleep(5)

    # query result
    status, response = self.query(collection_name, topk=300)
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    count = 200
    self.assertEqual(len(documents), count)
    for i in range(0, count):
      self.assertEqual(documents[i].primary_key, i + 1)
      self.assertEqual(documents[i].score, i * i)
      self.assertAlmostEqual(documents[i].forward_column_values['col_a'],
                             11.111 + i, delta=0.001)
      self.assertEqual(documents[i].forward_column_values['col_b'], 100 * (i + 1))

    # stop mysql
    self.server_utils.stop_mysql()
    time.sleep(2)

    # start mysql
    self.server_utils.start_mysql()
    time.sleep(2)

    # send inc2 data
    ret = self.mysql_client.execute_batch_sql('data/test_mysql_restart_with_inc_mode_inc_2.sql')
    self.assertEqual(ret, 0)

    time.sleep(5)

    # query result
    status, response = self.query(collection_name, topk=300)
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    count = 300
    self.assertEqual(len(documents), count)
    for i in range(0, count):
      self.assertEqual(documents[i].primary_key, i + 1)
      self.assertEqual(documents[i].score, i * i)
      self.assertAlmostEqual(documents[i].forward_column_values['col_a'],
                             11.111 + i, delta=0.001)
      self.assertEqual(documents[i].forward_column_values['col_b'], 100 * (i + 1))

  def test_mysql_restart_with_create_collection(self):
    # send full data
    ret = self.mysql_client.execute_batch_sql('data/test_mysql_restart_with_create_collection_full.sql')
    self.assertEqual(ret, 0)

    # stop mysql
    self.server_utils.stop_mysql()
    time.sleep(2)

    # create collection
    collection_name = 'test_mysql_restart_with_create_collection'
    schema = self.create_schema(collection_name,
                                repository_table=collection_name,
                                index_columns=["column1"],
                                index_dimensions=[16])
    status = self.client.create_collection(schema)
    self.assertEqual(status.code, 0)

    time.sleep(5)

    # query result
    status, response = self.query(collection_name, topk=300)
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    count = 0
    self.assertEqual(len(documents), count)

    # start mysql
    self.server_utils.start_mysql()

    time.sleep(5)

    # query result
    status, response = self.query(collection_name, topk=300)
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    count = 100
    self.assertEqual(len(documents), count)
    for i in range(0, count):
      self.assertEqual(documents[i].primary_key, i + 1)
      self.assertEqual(documents[i].score, i * i)
      self.assertAlmostEqual(documents[i].forward_column_values['col_a'],
                             11.111 + i, delta=0.001)
      self.assertEqual(documents[i].forward_column_values['col_b'], 100 * (i + 1))

  def test_multi_collections(self):
    # create collection
    collections = []
    collection_count = 5
    repository_table = 'test_multi_collections'
    for i in range(0, collection_count):
      collection = 'test_multi_collections_' + str(i)
      collections.append(collection)
      schema = self.create_schema(collection,
                                repository_table=repository_table,
                                index_columns=["column1"],
                                index_dimensions=[16])
      status = self.client.create_collection(schema)
      self.assertEqual(status.code, 0)

    # send full data
    ret = self.mysql_client.execute_batch_sql('data/test_multi_collections_full.sql')
    self.assertEqual(ret, 0)

    time.sleep(10)

    # query result
    topk = 300
    for i in range(0, collection_count):
      collection = 'test_multi_collections_' + str(i)
      status, response = self.query(collection, topk=topk)
      self.assertTrue(status.ok())
      logging.info("query result: %s", response)
      results = response.results
      self.assertEqual(len(results), 1)
      documents = results[0]
      count = 100
      self.assertEqual(len(documents), count)
      for i in range(0, count):
        self.assertEqual(documents[i].primary_key, i + 1)
        self.assertEqual(documents[i].score, i * i)
        self.assertAlmostEqual(documents[i].forward_column_values['col_a'],
                               11.111 + i, delta=0.000001)
        self.assertEqual(documents[i].forward_column_values['col_b'], 100 * (i + 1))

    # send inc data
    ret = self.mysql_client.execute_batch_sql('data/test_multi_collections_inc.sql')
    self.assertEqual(ret, 0)

    time.sleep(10)

    # query result
    topk = 300
    for i in range(0, collection_count):
      collection = 'test_multi_collections_' + str(i)
      status, response = self.query(collection, topk=topk)
      self.assertTrue(status.ok())
      logging.info("query result: %s", response)
      results = response.results
      self.assertEqual(len(results), 1)
      documents = results[0]
      count = 300
      self.assertEqual(len(documents), count)
      for i in range(0, count):
        self.assertEqual(documents[i].primary_key, i + 1)
        self.assertEqual(documents[i].score, i * i)
        self.assertAlmostEqual(documents[i].forward_column_values['col_a'],
                               11.111 + i, delta=0.001)
        self.assertEqual(documents[i].forward_column_values['col_b'], 100 * (i + 1))

  # def test_suspend_and_resume_collection(self):
  #   # create collection
  #   collection_name = 'test_mysql_restart_with_full_mode'
  #   schema = self.create_schema(collection_name,
  #                               repository_table=collection_name,
  #                               index_columns=["column1"],
  #                               index_dimensions=[16])
  #   status = self.client.create_collection(schema)
  #   self.assertEqual(status.code, 0)

  #   # suspend collection
  #   status = self.client.suspend_collection(collection_name)
  #   logging.info("suspend collection result: %s", data)
  #   self.assertEqual(status.code, 0)

  #   # send data
  #   ret = self.mysql_client.execute_batch_sql('data/test_suspend_and_resume_collection_full.sql')
  #   self.assertEqual(ret, 0)

  #   time.sleep(2)

  #   # query result
  #   status, response = self.query(collection_name)
  #   self.assertTrue(status.ok())
  #   logging.info("query result: %s", response)
  #   results = response.results
  #   self.assertEqual(len(results), 1)
  #   documents = results[0]
  #   self.assertEqual(len(documents), 0)

  #   # resume collection
  #   status = self.client.resume_collection(collection_name)
  #   logging.info("resume collection result: %s", data)
  #   self.assertEqual(status.code, 0)

  #   time.sleep(10)

  #   # query result
  #   query = self.create_query(collection_name, 300)
  #   status, response = self.query(collection_name)
  #   self.assertTrue(status.ok())
  #   logging.info("query result: %s", response)
  #   results = response.results
  #   self.assertEqual(len(results), 1)
  #   documents = results[0]
  #   count = 300
  #   self.assertEqual(len(documents), count)
  #   for i in range(0, count):
  #     self.assertEqual(documents[i].primary_key, i + 1)
  #     self.assertEqual(documents[i].score, i * i)
  #     self.assertAlmostEqual(documents[i].forward_column_values['col_a'],
  #                            11.111 + i, delta=0.001)
  #     self.assertEqual(documents[i].forward_column_values['col_b'], 100 * (i + 1))

  def test_invalid_lsn_info(self):
    # send data
    ret = self.mysql_client.execute_batch_sql('data/test_invalid_lsn_info_full.sql')
    self.assertEqual(ret, 0)

    time.sleep(5)

    # create collection
    collection_name = 'test_invalid_lsn_info'
    schema = self.create_schema(collection_name,
                                repository_table=collection_name,
                                index_columns=["column1"],
                                index_dimensions=[16])
    status = self.client.create_collection(schema)
    self.assertEqual(status.code, 0)

    time.sleep(5)

    # query result
    status, response = self.query(collection_name, topk=300)
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    count = 100
    self.assertEqual(len(documents), count)
    for i in range(0, count):
      self.assertEqual(documents[i].primary_key, i + 1)
      self.assertEqual(documents[i].score, i * i)
      self.assertAlmostEqual(documents[i].forward_column_values['col_a'],
                             11.111 + i, delta=0.001)
      self.assertEqual(documents[i].forward_column_values['col_b'], 100 * (i + 1))

    # get latest lsn
    latest_lsn = self.get_latest_lsn(collection_name)
    self.assertEqual(latest_lsn.lsn, 100)
    file_name = latest_lsn.context.split(';')[0]

    # stop mysql repository
    self.server_utils.stop_mysql_repo()

    # mysql flush logs
    ret = self.mysql_client.execute("flush logs;flush logs; flush logs;")
    self.assertEqual(ret, 0)

    # purge binlog
    ret = self.mysql_client.purge_binlog(file_name)
    self.assertEqual(ret, 0)

    # start mysql repository
    self.server_utils.start_mysql_repo()

    # send data
    ret = self.mysql_client.execute_batch_sql('data/test_invalid_lsn_info_inc.sql')
    self.assertEqual(ret, 0)

    time.sleep(5)

    # query result
    status, response = self.query(collection_name, topk=300)
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    count = 300
    self.assertEqual(len(documents), count)
    for i in range(0, count):
      self.assertEqual(documents[i].primary_key, i + 1)
      self.assertEqual(documents[i].score, i * i)
      self.assertAlmostEqual(documents[i].forward_column_values['col_a'],
                             11.111 + i, delta=0.001)
      self.assertEqual(documents[i].forward_column_values['col_b'], 100 * (i + 1))

  def test_forward_with_charset_utf8(self):
    collection_name = 'test_forward_with_charset_utf8'

    # prepare full table data
    ret = self.mysql_client.execute_batch_sql('data/test_forward_with_charset_utf8_full.sql')
    self.assertEqual(ret, 0)

    # create collection
    forward_columns=['f1', 'f2', 'f3', 'f4']
    schema = self.create_schema(collection_name,
                                repository_table=collection_name,
                                index_columns=["column1"],
                                forward_columns=forward_columns,
                                index_dimensions=[16])
    status = self.client.create_collection(schema)
    self.assertEqual(status.code, 0)

    time.sleep(5)

    # query result
    status, response = self.query(collection_name)
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    total = 4
    self.assertEqual(len(documents), total)
    for i in range(0, total):
      self.assertEqual(documents[i].primary_key, i + 1)
      self.assertEqual(documents[i].score, i * i)
      self.assertEqual(documents[i].forward_column_values['f1'], ('第一个字段'))
      self.assertEqual(documents[i].forward_column_values['f2'], ('定长字段'))
      self.assertEqual(documents[i].forward_column_values['f3'], ('我是谁'))
      self.assertEqual(documents[i].forward_column_values['f4'], ('第三个字段'))

    # prepare increment table data
    ret = self.mysql_client.execute_batch_sql('data/test_forward_with_charset_utf8_inc.sql')
    self.assertEqual(ret, 0)
    time.sleep(6)

    # query result
    status, response = self.query(collection_name)
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    total = 6
    self.assertEqual(len(documents), total)
    for i in range(0, total):
      self.assertEqual(documents[i].primary_key, i + 1)
      self.assertEqual(documents[i].score, i * i)
      self.assertEqual(documents[i].forward_column_values['f1'], ('第一个字段'))
      self.assertEqual(documents[i].forward_column_values['f2'], ('定长字段'))
      self.assertEqual(documents[i].forward_column_values['f3'], ('我是谁'))
      self.assertEqual(documents[i].forward_column_values['f4'], ('第三个字段'))

  def test_forward_with_charset_gbk(self):
    collection_name = 'test_forward_with_charset_gbk'

    # prepare full table data
    ret = self.mysql_client.execute_batch_sql('data/test_forward_with_charset_gbk_full.sql')
    self.assertEqual(ret, 0)

    # create collection
    forward_columns=['f1', 'f2', 'f3', 'f4']
    schema = self.create_schema(collection_name,
                                repository_table=collection_name,
                                index_columns=["column1"],
                                forward_columns=forward_columns,
                                index_dimensions=[16])
    status = self.client.create_collection(schema)
    self.assertEqual(status.code, 0)

    time.sleep(5)

    # query result
    status, response = self.query(collection_name)
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    total = 4
    self.assertEqual(len(documents), total)
    for i in range(0, total):
      self.assertEqual(documents[i].primary_key, i + 1)
      self.assertEqual(documents[i].score, i * i)
      self.assertEqual(documents[i].forward_column_values['f1'], ('第一个字段'))
      self.assertEqual(documents[i].forward_column_values['f2'], ('定长字段'))
      self.assertEqual(documents[i].forward_column_values['f3'], ('我是谁'))
      self.assertEqual(documents[i].forward_column_values['f4'], ('第三个字段'))

    # prepare increment table data
    ret = self.mysql_client.execute_batch_sql('data/test_forward_with_charset_gbk_inc.sql')
    self.assertEqual(ret, 0)
    time.sleep(6)

    # query result
    status, response = self.query(collection_name)
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    total = 6
    self.assertEqual(len(documents), total)
    for i in range(0, total):
      self.assertEqual(documents[i].primary_key, i + 1)
      self.assertEqual(documents[i].score, i * i)
      self.assertEqual(documents[i].forward_column_values['f1'], ('第一个字段'))
      self.assertEqual(documents[i].forward_column_values['f2'], ('定长字段'))
      self.assertEqual(documents[i].forward_column_values['f3'], ('我是谁'))
      self.assertEqual(documents[i].forward_column_values['f4'], ('第三个字段'))

  def test_one_field_both_index_and_forward(self):
    collection_name = 'test_one_field_both_index_and_forward'

    # prepare full table data
    ret = self.mysql_client.execute_batch_sql('data/test_one_field_both_index_and_forward_full.sql')
    self.assertEqual(ret, 0)

    # create collection
    schema = self.create_schema(collection_name,
                                repository_table=collection_name,
                                forward_columns=["f1", "column1"],
                                index_columns=["column1", "column2"],
                                index_dimensions=[16, 16])
    status = self.client.create_collection(schema)
    self.assertEqual(status.code, 0)

    time.sleep(5)

    # query result
    status, response = self.query(collection_name)
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    total = 4
    self.assertEqual(len(documents), total)
    for i in range(0, total):
      self.assertEqual(documents[i].primary_key, i + 1)
      self.assertEqual(documents[i].score, i * i)
      self.assertEqual(documents[i].forward_column_values['f1'], (str(i + 1) * 16))
      self.assertEqual(documents[i].forward_column_values['column1'], '[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,' + str(i + 1) + ']')

    # prepare increment table data
    ret = self.mysql_client.execute_batch_sql('data/test_one_field_both_index_and_forward_inc.sql')
    self.assertEqual(ret, 0)
    time.sleep(6)

    # query result
    status, response = self.query(collection_name)
    self.assertTrue(status.ok())
    logging.info("query result: %s", response)
    results = response.results
    self.assertEqual(len(results), 1)
    documents = results[0]
    total = 6
    self.assertEqual(len(documents), total)
    for i in range(0, total):
      self.assertEqual(documents[i].primary_key, i + 1)
      self.assertEqual(documents[i].score, i * i)
      self.assertEqual(documents[i].forward_column_values['f1'], (str(i + 1) * 16))
      self.assertEqual(documents[i].forward_column_values['column1'], '[1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,' + str(i + 1) + ']')

if __name__ == '__main__':
  unittest.main()
