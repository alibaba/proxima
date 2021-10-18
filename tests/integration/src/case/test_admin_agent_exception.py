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
import time
import unittest

from pyproximabe import Client
from collection_creator import CollectionCreator
from server_utils import ServerUtils
from test_admin_agent import TestAdminAgentBase


class TestAdminAgentException(TestAdminAgentBase):
    def setUp(self):
        super().setUp()
        self.server_utils = ServerUtils()

    def test_create_duplicate_collection(self):
        # create collection
        collection = self.create_collection_succ_and_return(self.schema_list[0])
        logging.info("original collection result: %s", collection)

        # create same collection
        status = self.client.create_collection(self.schema_list[0])
        self.assertFalse(status.ok())
        self.assertEqual(status.code, -4000)
        self.assertEqual(status.reason, 'Duplicate Collection')

    def test_create_illegal_collection(self):
        # empty index columns
        self.schema_list[0].index_column_params.pop()
        status = self.client.create_collection(self.schema_list[0])
        self.assertFalse(status.ok())
        self.assertEqual(status.code, -2002)
        self.assertEqual(status.reason, 'Empty Columns')
        # todo:add detail reason

    def test_drop_repeated_collection(self):
        # create collection
        collection = self.create_collection_succ_and_return(self.schema_list[0])
        logging.info("original collection result: %s", collection)

        # drop collection
        status = self.client.drop_collection(self.collection_name_list[0])
        self.assertTrue(status.ok())
        logging.info("Drop collection result: %s", status)

        # drop collection repeated
        status = self.client.drop_collection(self.collection_name_list[0])
        self.assertEqual(status.code, -4002)
        self.assertEqual(status.reason, 'Collection Not Exist')

    def test_drop_nonexist_collection(self):
        for idx in range(len(self.collection_name_list)):
            status = self.client.drop_collection(self.collection_name_list[idx])
            self.assertEqual(status.code, -4002)
            self.assertEqual(status.reason, 'Collection Not Exist')

    # def test_update_nonexist_collection(self):
    #     # create collection
    #     status = self.client.create_collection(self.schema_list[0])
    #     self.assertEqual(code, 200)
    #     self.assertEqual(status.code, 0)
    #     collection = obj["collection"]
    #     logging.info("original collection result: %s", collection)
    #
    #     # update collection with fake name
    #     self.schema_list[0].name = "collection_100"
    #     status = self.client.update_collection(self.schema_list[0])
    #     self.assertEqual(code, 200)
    #     self.assertEqual(status.code, -1001)
    #     self.assertEqual(status.reason, 'Collection not exist')
    #
    # def test_illegal_update_collection(self):
    #     # create collection
    #     status = self.client.create_collection(self.schema_list[0])
    #     self.assertEqual(code, 200)
    #     self.assertEqual(status.code, 0)
    #     collection = obj["collection"]
    #     logging.info("original collection result: %s", collection)
    #
    #     self.update_schema(schema=self.schema_list[0], json_obj=obj["collection"])
    #     # todo: lack detail reason
    #     # update connection
    #     self.schema_list[0].connection = "mysql://root:root@127.0.0.1:4306/fake_db"
    #     status = self.client.update_collection(self.schema_list[0])
    #     # update repository_name
    #     self.schema_list[0].repository_table = "fake_name"
    #     status = self.client.update_collection(self.schema_list[0])
    #     # update repository_table
    #     self.schema_list[0].repository_table = "fake_table"
    #     status = self.client.update_collection(self.schema_list[0])
    #     # update is_current
    #     self.schema_list[0].is_current = False
    #     status = self.client.update_collection(self.schema_list[0])
    #
    # def test_remove_all_index_columns(self):
    #     # create collection
    #     status = self.client.create_collection(self.schema_index_column_list[1])
    #     self.assertEqual(code, 200)
    #     self.assertEqual(status.code, 0)
    #     collection = obj["collection"]
    #     logging.info("original collection result: %s", collection)
    #
    #     self.update_schema(schema=self.schema_index_column_list[1], json_obj=obj["collection"])
    #
    #     self.schema_index_column_list[1].index_columns.pop()
    #     self.schema_index_column_list[1].index_columns.pop()
    #     # todo: lack detail reason
    #     status = self.client.update_collection(self.schema_index_column_list[1])

    def test_restart_server_with_list_collections(self):
        collection = self.create_collection_succ_and_return(self.schema_list[0])
        logging.info("Create collection result: %s", collection)
        self.assert_collection(collection, 0)

        time.sleep(5)

        # restart proxima be
        self.server_utils.stop_proxima_be('SIGUSR1')
        self.server_utils.start_proxima_be()
        time.sleep(2)

        # resetup
        self.setup_client()
        # describe collection
        status, collection = self.client.describe_collection(self.collection_name_list[0])
        logging.info('describe collection, status=[%s], collection=[%s]', status, collection)
        self.assertTrue(status.ok())
        self.assert_collection(collection, 0)

    def test_restart_server_with_create_collections(self):
        # create collection
        collection = self.create_collection_succ_and_return(self.schema_list[0])
        logging.info("Create collection result: %s", collection)
        self.assert_collection(collection, 0)

        time.sleep(5)

        # restart proxima be
        self.server_utils.stop_proxima_be('SIGUSR1')
        self.server_utils.start_proxima_be()
        time.sleep(2)

        # resetup
        self.setUp()

        collection = self.create_collection_succ_and_return(self.schema_list[1])
        logging.info("Create collection result: %s", collection)
        self.assert_collection(collection, 1)

    # def test_restart_server_with_update_collections(self):
    #     # create collection
    #     status = self.client.create_collection(self.schema_list[0])
    #     self.assertEqual(code, 200)
    #     self.assertEqual(status.code, 0)
    #     collection = obj["collection"]
    #     logging.info("original collection result: %s", collection)
    #
    #     self.assertEqual(collection["name"], self.collection_name_list[0])
    #     self.assertEqual(collection["connection"], self.connection)
    #     self.assertEqual(collection["repository_table"], "test_collection")
    #     self.assertEqual(collection["repository_name"], "test_repo")
    #     self.assertEqual(len(collection["forward_columns"]), 2)
    #     self.assertEqual(collection["forward_columns"][0], "col_a")
    #     self.assertEqual(collection["forward_columns"][1], "col_b")
    #     self.assertEqual(collection["status"], "SERVING")
    #     self.assertEqual(collection["is_current"], True)
    #     self.assertEqual(collection["index_columns"][0]["name"], "column0")
    #     self.assertEqual(collection["index_columns"][0]["alias"], "column0")
    #     self.assertEqual(collection["index_columns"][0]["index_type"], "PROXIMA_GRAPH_INDEX")
    #     self.assertEqual(collection["index_columns"][0]["data_type"], "FT_FP32")
    #     self.assertEqual(len(collection["index_columns"][0]["parameters"]), 1)
    #     self.assertEqual(collection["index_columns"][0]["parameters"][0]["key"], "dimension")
    #     self.assertEqual(collection["index_columns"][0]["parameters"][0]["value"], "16")
    #
    #     self.client.close()
    #     time.sleep(5)
    #
    #     # restart proxima be
    #     self.server_utils.stop_proxima_be('SIGUSR1')
    #     self.server_utils.start_proxima_be()
    #     time.sleep(2)
    #
    #     # update collection
    #     self.client = AdminAgentClient()
    #
    #     # update_collection
    #     self.update_schema(schema=self.schema_list[0], json_obj=obj["collection"])
    #     self.schema_list[0].forward_columns.append("col_c")
    #     status = self.client.update_collection(self.schema_list[0])
    #     collection = obj["collection"]
    #     logging.info("Updated collection result: %s", collection)
    #
    #     self.assertEqual(collection["name"], self.collection_name_list[0])
    #     self.assertEqual(collection["connection"], self.connection)
    #     self.assertEqual(collection["repository_table"], "test_collection")
    #     self.assertEqual(collection["repository_name"], "test_repo")
    #     self.assertEqual(len(collection["forward_columns"]), 3)
    #     self.assertEqual(collection["forward_columns"][0], "col_a")
    #     self.assertEqual(collection["forward_columns"][1], "col_b")
    #     self.assertEqual(collection["forward_columns"][2], "col_c")
    #     self.assertEqual(collection["status"], "SERVING")
    #     self.assertEqual(collection["is_current"], True)
    #     self.assertEqual(collection["index_columns"][0]["name"], "column0")
    #     self.assertEqual(collection["index_columns"][0]["alias"], "column0")
    #     self.assertEqual(collection["index_columns"][0]["index_type"], "PROXIMA_GRAPH_INDEX")
    #     self.assertEqual(collection["index_columns"][0]["data_type"], "FT_FP32")
    #     self.assertEqual(len(collection["index_columns"][0]["parameters"]), 1)
    #     self.assertEqual(collection["index_columns"][0]["parameters"][0]["key"], "dimension")
    #     self.assertEqual(collection["index_columns"][0]["parameters"][0]["value"], "16")

    def test_restart_server_with_drop_collections(self):
        # create collection
        collection = self.create_collection_succ_and_return(self.schema_list[0])
        logging.info("Create collection result: %s", collection)
        self.assert_collection(collection, 0)

        time.sleep(5)

        # restart proxima be
        self.server_utils.stop_proxima_be('SIGUSR1')
        self.server_utils.start_proxima_be()
        time.sleep(2)

        # resetup
        self.setup_client()

        status = self.client.drop_collection(self.collection_name_list[0])
        logging.info("Drop collection result: %s", status)
        self.assertTrue(status.ok())

    def assert_collection(self, collection, idx):
        config = collection.collection_config
        self.assertEqual(config.collection_name, self.collection_name_list[idx])
        self.assertTrue(config.repository_config is not None)
        repo_config = config.repository_config
        self.assertEqual(repo_config.connection_uri, self.connection)
        self.assertEqual(repo_config.table_name, "test_collection")
        self.assertEqual(repo_config.repository_name, "test_repo")
        self.assertEqual(repo_config.user, self.creator.user)
        self.assertEqual(repo_config.user, self.creator.password)
        forward_column_names = config.forward_column_names
        self.assertEqual(len(forward_column_names), 2)
        self.assertEqual(forward_column_names[0], "col_a")
        self.assertEqual(forward_column_names[1], "col_b")
        self.assertEqual(collection.status.name, "SERVING")
        index_columns = config.index_column_params
        self.assertEqual(len(index_columns), 1)
        index_column = index_columns[0]
        self.assertEqual(index_column.name, f"column{idx}")
        self.assertEqual(index_column.index_type.name, "PROXIMA_GRAPH_INDEX")
        self.assertEqual(index_column.data_type.name, "VECTOR_FP32")
        self.assertEqual(index_column.dimension, 16 * (idx + 1))
        self.assertFalse(index_column.extra_params)


if __name__ == '__main__':
    unittest.main()
