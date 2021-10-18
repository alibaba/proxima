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

import unittest
import logging
import time
import random

from pyproximabe import *
from global_conf import GlobalConf
from collection_creator import CollectionCreator
import client_helper


class TestAdminAgentBase(unittest.TestCase):
    def setup_client(self):
        self.client = client_helper.get_client(self.global_conf)

    def setUp(self):
        self.global_conf = GlobalConf()
        self.creator = CollectionCreator()
        self.setup_client()

        self.collection_name_list = ["collection1", "collection2", "collection3"]

        self.clean_env()

        self.schema_list = [
            self.create_collection_config(self.collection_name_list[i], [f"column{str(i)}"], [16 * (i + 1)])
            for i in range(3)]
        self.forward_columns_list = [[], ["col_a"], ["col_a", "col_b"]]

        self.index_columns_list = [["column0"], ["column1", "column2"]]
        self.index_dimensions_list = [[16], [16, 16]]
        self.schema_index_column_list = [
            self.create_collection_config(self.collection_name_list[i], self.index_columns_list[i],
                                          self.index_dimensions_list[i]) for i in range(len(self.index_columns_list))]
        self.connection = self.creator.get_connection()

    def tearDown(self):
        self.clean_env()

    def clean_env(self):
        for collection in self.collection_name_list:
            status = self.client.drop_collection(collection)

    def update_schema(self, schema, json_obj):
        index_columns = schema.index_columns
        for i in range(0, len(index_columns)):
            index_columns[i].uid = json_obj["index_columns"][i]["uid"]
        schema.uid = json_obj["uid"]

    def create_collection_config(self, collection_name, column_name, dim,
                                 forward_columns=["col_a", "col_b"],
                                 repository_name="test_repo"):
        return self.creator.create_schema(collection_name,
                                          repository_table="test_collection",
                                          repository_name=repository_name,
                                          forward_columns=forward_columns,
                                          revision=1,
                                          index_columns=column_name,
                                          index_dimensions=dim,
                                          db_name="test_db")

    def create_collection_succ_and_return(self, schema):
        status = self.client.create_collection(schema)
        self.assertTrue(status.ok)
        status, collection_info = self.client.describe_collection(schema.collection_name)
        self.assertTrue(status.ok)
        return collection_info


class TestAdminAgent(TestAdminAgentBase):
    def test_create_collection(self):
        collection = self.create_collection_succ_and_return(self.schema_list[0])
        logging.info("Create collection result: %s", collection)

        config = collection.collection_config
        self.assertEqual(config.collection_name, self.collection_name_list[0])
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
        self.assertEqual(index_column.name, "column0")
        self.assertEqual(index_column.index_type.name, "PROXIMA_GRAPH_INDEX")
        self.assertEqual(index_column.data_type.name, "VECTOR_FP32")
        self.assertEqual(index_column.dimension, 16)
        self.assertFalse(index_column.extra_params)

    def test_create_multi_collection(self):
        for idx in range(3):
            collection = self.create_collection_succ_and_return(self.schema_list[idx])
            logging.info("Create collection result: %s", collection)

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

    def test_create_collection_forward_columns(self):
        for idx in range(len(self.schema_list)):
            self.schema_list[idx].forward_column_names[:] = self.forward_columns_list[idx]
            collection = self.create_collection_succ_and_return(self.schema_list[idx])
            logging.info("Create collection result: %s", collection)

            config = collection.collection_config
            self.assertEqual(self.forward_columns_list[idx], config.forward_column_names)

    def test_create_collection_index_columns(self):
        for idx in range(len(self.schema_index_column_list)):
            collection = self.create_collection_succ_and_return(self.schema_index_column_list[idx])
            logging.info("Create collection result: %s", collection)

            index_columns = collection.collection_config.index_column_params
            for column in index_columns:
                self.assertTrue(column.name in self.index_columns_list[idx])
                self.assertEqual(column.index_type.name, "PROXIMA_GRAPH_INDEX")
                self.assertEqual(column.data_type.name, "VECTOR_FP32")

    def test_drop_collection(self):
        # create collection
        collection = self.create_collection_succ_and_return(self.schema_list[0])

        # drop collection
        status = self.client.drop_collection(self.collection_name_list[0])
        self.assertTrue(status.ok())
        logging.info("Drop collection result: %s", status)

    # def test_update_collection(self):
    #     # create collection
    #     code, obj = self.client.create_collection(self.schema_list[0])
    #     self.assertEqual(code, 200)
    #     self.assertEqual(status.code, 0)
    #     collection = obj["collection"]
    #     logging.info("original collection result: %s", collection)
    #
    #     # update_collection
    #     self.update_schema(schema=self.schema_list[0], json_obj=obj["collection"])
    #     self.schema_list[0].forward_columns.append("col_c")
    #     self.schema_list[0].revision = 2
    #     code, obj = self.client.update_collection(self.schema_list[0])
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
    #
    # def test_collection_history(self):
    #     # collection not exist
    #     code, obj = self.client.get_collection_history(self.collection_name_list[0])
    #     self.assertEqual(code, 200)
    #     self.assertEqual(obj['code'], -1001)
    #
    #     # create collection
    #     code, obj = self.client.create_collection(self.schema_list[0])
    #     self.assertEqual(code, 200)
    #     self.assertEqual(status.code, 0)
    #     collection = obj["collection"]
    #     logging.info("original collection result: %s", collection)
    #
    #     code, obj = self.client.get_collection_history(self.collection_name_list[0])
    #     self.assertEqual(code, 200)
    #     self.assertEqual(obj['code'], 0)
    #     collections = obj['collections']
    #     self.assertEqual(len(collections), 1)
    #     collection = collections[0]
    #     self.assertEqual(collection["name"], self.collection_name_list[0])
    #     self.assertEqual(len(collection["forward_columns"]), 2)
    #     self.assertEqual(collection["forward_columns"][0], "col_a")
    #     self.assertEqual(collection["forward_columns"][1], "col_b")
    #
    #     # update_collection
    #     self.update_schema(schema=self.schema_list[0], json_obj=collection)
    #     self.schema_list[0].forward_columns.append("col_c")
    #     self.schema_list[0].revision = 2
    #     code, obj = self.client.update_collection(self.schema_list[0])
    #     collection = obj["collection"]
    #     logging.info("Updated collection result: %s", collection)
    #
    #     code, obj = self.client.get_collection_history(self.collection_name_list[0])
    #     self.assertEqual(code, 200)
    #     self.assertEqual(obj['code'], 0)
    #     collections = obj['collections']
    #     self.assertEqual(len(collections), 2)
    #     v1 = collections[0]
    #     self.assertEqual(v1["name"], self.collection_name_list[0])
    #
    #     self.assertEqual(len(v1["forward_columns"]), 3)
    #     self.assertEqual(v1["forward_columns"][0], "col_a")
    #     self.assertEqual(v1["forward_columns"][1], "col_b")
    #     self.assertEqual(v1["forward_columns"][2], "col_c")
    #
    #     v0 = collections[1]
    #     self.assertEqual(v0["name"], self.collection_name_list[0])
    #     self.assertEqual(v0['revision'], 0)
    #     self.assertEqual(len(v0["forward_columns"]), 2)
    #     self.assertEqual(v0["forward_columns"][0], "col_a")
    #     self.assertEqual(v0["forward_columns"][1], "col_b")
    #
    # def test_remove_index_columns(self):
    #     # create collection
    #     code, obj = self.client.create_collection(self.schema_index_column_list[1])
    #     self.assertEqual(code, 200)
    #     self.assertEqual(status.code, 0)
    #     collection = obj["collection"]
    #     logging.info("original collection result: %s", collection)
    #
    #     # update collection
    #     self.update_schema(schema=self.schema_index_column_list[1], json_obj=obj["collection"])
    #     self.schema_index_column_list[1].index_columns.pop()
    #     code, obj = self.client.update_collection(self.schema_index_column_list[1])
    #     collection = obj["collection"]
    #     logging.info("Updated collection result: %s", collection)
    #
    #     self.assertEqual(collection["name"], "collection2")
    #     self.assertEqual(collection["connection"], self.connection)
    #     self.assertEqual(collection["repository_table"], "test_collection")
    #     self.assertEqual(collection["repository_name"], "test_repo")
    #     self.assertEqual(len(collection["forward_columns"]), 2)
    #     self.assertEqual(collection["forward_columns"][0], "col_a")
    #     self.assertEqual(collection["forward_columns"][1], "col_b")
    #     self.assertEqual(collection["status"], "SERVING")
    #     self.assertEqual(collection["is_current"], True)
    #     self.assertEqual(collection["index_columns"][0]["name"], "column1")
    #     self.assertEqual(collection["index_columns"][0]["alias"], "column1")
    #     self.assertEqual(collection["index_columns"][0]["index_type"], "PROXIMA_GRAPH_INDEX")
    #     self.assertEqual(collection["index_columns"][0]["data_type"], "FT_FP32")
    #     self.assertEqual(len(collection["index_columns"][0]["parameters"]), 1)
    #     self.assertEqual(collection["index_columns"][0]["parameters"][0]["key"], "dimension")
    #     self.assertEqual(collection["index_columns"][0]["parameters"][0]["value"], "16")
    #
    # def test_remove_all_forward_columns(self):
    #     # create collection
    #     code, obj = self.client.create_collection(self.schema_list[0])
    #     self.assertEqual(code, 200)
    #     self.assertEqual(status.code, 0)
    #     collection = obj["collection"]
    #     logging.info("original collection result: %s", collection)
    #
    #     # remove forward columns
    #     self.update_schema(schema=self.schema_list[0], json_obj=obj["collection"])
    #     self.schema_list[0].forward_columns.remove("col_a")
    #     self.schema_list[0].forward_columns.remove("col_b")
    #     code, obj = self.client.update_collection(self.schema_list[0])
    #     collection = obj["collection"]
    #     logging.info("Updated collection result: %s", collection)
    #
    #     self.assertEqual(collection["name"], self.collection_name_list[0])
    #     self.assertEqual(collection["connection"], self.connection)
    #     self.assertEqual(collection["repository_table"], "test_collection")
    #     self.assertEqual(collection["repository_name"], "test_repo")
    #     self.assertEqual(len(collection["forward_columns"]), 0)
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
    def test_list_collections(self):
        for idx in range(3):
            collection = self.create_collection_succ_and_return(self.schema_list[idx])
            logging.info("Create collection result: %s", collection)

        # list collections
        status, collections = self.client.list_collections()
        logging.info("List collection result: %s", collections)
        self.assertTrue(status.ok())
        for idx in range(3):
            collection = None
            for c in collections:
                if c.collection_config.collection_name == self.collection_name_list[idx]:
                    collection = c
                    break
            self.assertIsNotNone(collection)
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

    def test_list_collections_with_repo(self):
        # recreate the schema
        repo_list = ["test_repo", "test_repo1", "test_repo2"]
        schema_list = [
            self.create_collection_config(self.collection_name_list[i],
                                          [f"column{str(i)}"],
                                          [16 * (i + 1)],
                                          repository_name=repo_list[i])
            for i in range(3)]

        # create multi collection
        for idx in range(3):
           status = self.client.create_collection(schema_list[idx])
           self.assertTrue(status.ok())
        status, collections = self.client.list_collections(repository_name="test_repo")
        self.assertTrue(status.ok())
        self.assertEqual(len(collections), 1)
        idx = 0
        collection = collections[idx]
        logging.info("Create collection result: %s", collection)

        # check collection config
        config = collection.collection_config
        self.assertEqual(config.collection_name, self.collection_name_list[0])
        self.assertTrue(config.repository_config is not None)
        repo_config = config.repository_config
        self.assertEqual(repo_config.connection_uri, self.connection)
        self.assertEqual(repo_config.table_name, "test_collection")
        self.assertEqual(repo_config.repository_name, "test_repo")
        forward_column_names = config.forward_column_names
        self.assertEqual(len(forward_column_names), 2)
        self.assertEqual(forward_column_names[0], "col_a")
        self.assertEqual(forward_column_names[1], "col_b")
        index_columns = config.index_column_params
        self.assertEqual(len(index_columns), 1)
        index_column = index_columns[0]
        self.assertEqual(index_column.name, f"column{str(idx)}")
        self.assertEqual(index_column.index_type.name, "PROXIMA_GRAPH_INDEX")
        self.assertEqual(index_column.data_type.name, "VECTOR_FP32")
        self.assertEqual(index_column.dimension, 16 * (idx + 1))
        self.assertFalse(index_column.extra_params)
        # check status
        self.assertEqual(collection.status.name, "SERVING")

    # def test_reload_collections(self):
    #     # create collection
    #     code, obj = self.client.create_collection(self.schema_list[0])
    #     self.assertEqual(code, 200)
    #     self.assertEqual(status.code, 0)
    #     collection = obj["collection"]
    #     logging.info("original collection result: %s", collection)
    #
    #     # reload meta
    #     self.client.reload_meta()
    #
    #     # list_collection
    #     code, obj = self.client.describe_collection(self.collection_name_list[0])
    #     self.assertEqual(code, 200)
    #     self.assertEqual(status.code, 0)
    #     self.assertEqual(status.reason, '')
    #     collection = obj["collection"]
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
    #     logging.info("List collection result: %s", collection)
    #
    # def test_suspend_and_resume_collections(self):
    #     # create collection
    #     code, obj = self.client.create_collection(self.schema_list[0])
    #     self.assertEqual(code, 200)
    #     self.assertEqual(status.code, 0)
    #     collection = obj["collection"]
    #     self.assertEqual(collection["status"], 'SERVING')
    #     logging.info("original collection result: %s", collection)
    #
    #     # suspend collection
    #     self.client.suspend_collection(collection["name"])
    #     code, obj = self.client.describe_collection(collection["name"])
    #     collection = obj["collection"]
    #     self.assertEqual(collection["writable"], False)
    #     logging.info("suspend  collection result: %s", collection)
    #
    #     self.assertEqual(collection["connection"], self.connection)
    #     self.assertEqual(collection["repository_table"], "test_collection")
    #     self.assertEqual(collection["repository_name"], "test_repo")
    #     self.assertEqual(len(collection["forward_columns"]), 2)
    #     self.assertEqual(collection["forward_columns"][0], "col_a")
    #     self.assertEqual(collection["forward_columns"][1], "col_b")
    #     self.assertEqual(collection["is_current"], True)
    #     self.assertEqual(collection["index_columns"][0]["name"], "column0")
    #     self.assertEqual(collection["index_columns"][0]["alias"], "column0")
    #     self.assertEqual(collection["index_columns"][0]["index_type"], "PROXIMA_GRAPH_INDEX")
    #     self.assertEqual(collection["index_columns"][0]["data_type"], "FT_FP32")
    #     self.assertEqual(len(collection["index_columns"][0]["parameters"]), 1)
    #     self.assertEqual(collection["index_columns"][0]["parameters"][0]["key"], "dimension")
    #     self.assertEqual(collection["index_columns"][0]["parameters"][0]["value"], "16")
    #
    #     # resume collection
    #     self.client.resume_collection(collection["name"])
    #     code, obj = self.client.describe_collection(collection["name"])
    #     collection = obj["collection"]
    #     self.assertEqual(collection["status"], 'SERVING')
    #     logging.info("resume   collection result: %s", collection)
    #
    #     self.assertEqual(collection["connection"], self.connection)
    #     self.assertEqual(collection["repository_table"], "test_collection")
    #     self.assertEqual(collection["repository_name"], "test_repo")
    #     self.assertEqual(len(collection["forward_columns"]), 2)
    #     self.assertEqual(collection["forward_columns"][0], "col_a")
    #     self.assertEqual(collection["forward_columns"][1], "col_b")
    #     self.assertEqual(collection["is_current"], True)
    #     self.assertEqual(collection["index_columns"][0]["name"], "column0")
    #     self.assertEqual(collection["index_columns"][0]["alias"], "column0")
    #     self.assertEqual(collection["index_columns"][0]["index_type"], "PROXIMA_GRAPH_INDEX")
    #     self.assertEqual(collection["index_columns"][0]["data_type"], "FT_FP32")
    #     self.assertEqual(len(collection["index_columns"][0]["parameters"]), 1)
    #     self.assertEqual(collection["index_columns"][0]["parameters"][0]["key"], "dimension")
    #     self.assertEqual(collection["index_columns"][0]["parameters"][0]["value"], "16")
    #
    # def test_stats_all_collection(self):
    #     for idx in range(3):
    #         code, obj = self.client.create_collection(self.schema_list[idx])
    #         self.assertEqual(code, 200)
    #         self.assertEqual(status.code, 0)
    #         collection = obj["collection"]
    #         logging.info("Create collection result: %s", collection)
    #     # get all collection stats
    #     code, data = self.client.get_collection_stats('')
    #     stats = data['collection_stats']
    #     self.assertEqual(code, 200)
    #     self.assertEqual(data["code"], 0)
    #     self.assertTrue(len(stats) == 3)
    #     collection2 = [c for c in stats if c['collection_name'] == 'collection2'][0]
    #     self.assertGreater(int(collection2['total_segment_count']), 0)
    #     self.assertGreater(len(collection2['segment_stats']), 0)

    def test_stats_collection(self):
        collection = self.create_collection_succ_and_return(self.schema_list[0])
        logging.info("Create collection result: %s", collection)

        status, stats = self.client.stats_collection(self.collection_name_list[0])
        logging.info("Collection stats: %s", stats)
        self.assertTrue(status.ok())
        self.assertGreater(stats.total_segment_count, 0)
        self.assertGreater(len(stats.segment_stats), 0)

    def test_stats_invalid_collection(self):
        status, data = self.client.stats_collection("collection_not_exist")
        self.assertFalse(status.ok())
        self.assertIsNone(data)
        self.assertEqual(status.code, -4002)
        self.assertEqual(status.reason, 'Collection Not Exist')

    # def test_query_service(self):
    #     code, status = self.client.get_query_service_status()
    #     self.assertEqual(code, 200)
    #     self.assertEqual('running', status)
    #
    #     self.client.stop_query_service()
    #     time.sleep(0.1)
    #     code, status = self.client.get_query_service_status()
    #     self.assertEqual(code, 200)
    #     self.assertEqual('stopped', status)
    #
    #     self.client.start_query_service()
    #     time.sleep(0.1)
    #     code, status = self.client.get_query_service_status()
    #     self.assertEqual(code, 200)
    #     self.assertEqual('running', status)


if __name__ == '__main__':
    unittest.main()
