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

import sys, time, os, json, random
import http.client

from pyproximabe import *
from google.protobuf.json_format import MessageToJson

from global_conf import GlobalConf
from log import *


class CollectionCreator:
    def __init__(self):
        self.gf = GlobalConf()
        self.user = 'root'
        self.password = 'root'

    def generate_collection_name(self, prefix):
        return prefix + str(random.randint(100000, 1000000))

    def get_connection(self, db_name=None):
        connection = "mysql://127.0.0.1:" + str(self.gf.mysql_port()) + "/"
        if db_name:
            return connection + db_name
        else:
            return connection + "test_db"

    def create_schema(self,
                      collection_name,
                      repository_table=None,
                      repository_name=None,
                      forward_columns=None,
                      revision=None,
                      index_columns=None,
                      index_types=None,
                      index_data_types=None,
                      index_dimensions=None,
                      index_measures=None,
                      max_docs_per_segment=100,
                      db_name=None,
                      with_repo=True):
        db_repo = None
        if with_repo:
            connection = self.get_connection(db_name)
            if not repository_table:
                repository_table = "test_collection"
            if not repository_name:
                repository_name = "test_repo"
            db_repo = DatabaseRepository(repository_name, connection, repository_table, 'root', 'root')

        indexes = []
        if index_columns:
            for i in range(0, len(index_columns)):
                name = index_columns[i]
                index_type = index_types[i] if index_types is not None else IndexType.PROXIMA_GRAPH_INDEX
                data_type = index_data_types[i] if index_data_types is not None else DataType.VECTOR_FP32
                extra_params = {}
                if index_measures:
                    extra_params['metric_type'] = str(index_measures[i])
                index = IndexColumnParam(name,
                                         dimension=index_dimensions[i],
                                         index_type=index_type,
                                         data_type=data_type,
                                         extra_params=extra_params)
                indexes.append(index)
        config = CollectionConfig(collection_name=collection_name,
                                  index_column_params=indexes,
                                  max_docs_per_segment=max_docs_per_segment,
                                  forward_column_names=forward_columns,
                                  repository_config=db_repo)
        return config

    def update_schema(self, schema, json_obj):
        index_columns = schema.index_columns
        for i in range(0, len(index_columns)):
            index_columns[i].uid = json_obj["index_columns"][i]["uid"]
        schema.uid = json_obj["uid"]

    def create_dataset_request(self,
                               collection_name,
                               magic_number,
                               index_tuple_metas=None,
                               index_tuple_types=None,
                               forward_tuple_names=None,
                               forward_tuple_types=None,
                               rows=None,
                               with_repo=True):
        index_column_metas = []
        for element in index_tuple_metas:
            index_column_metas.append(WriteRequest.IndexColumnMeta(
                    element[0], element[1], element[2]))
        row_meta = WriteRequest.RowMeta(
            index_column_metas=index_column_metas,
            forward_column_names=forward_tuple_names,
            forward_column_types=forward_tuple_types)
        rows_data = []
        for row in rows:
            index_values=[]
            if not index_tuple_metas:
                index_tuple_metas = [['column1', DataType.VECTOR_FP32, 16]]
            for i in range(0, len(index_tuple_metas)):
                index_values.append(row[3 + i])
            start_position = 3 + len(index_tuple_metas)
            forward_values=[]
            for i in range(0, len(forward_tuple_names)):
                forward_values.append(row[start_position + i])
            if with_repo:
                lsn_context = LsnContext(row[2], "")
            else:
                lsn_context = None
#            if row[1] != OperationType.DELETE:
            rows_data.append(
                WriteRequest.Row(primary_key=row[0],
                                 operation_type=row[1],
                                 index_column_values=index_values,
                                 forward_column_values=forward_values,
                                 lsn_context=lsn_context))
            # else:
            #     rows_data.append(
            #         WriteRequest.Row(primary_key=row[0],
            #                          operation_type=row[1]))
        write_request = WriteRequest(collection_name,
                                     rows_data,
                                     row_meta=row_meta,
                                     magic_number=magic_number)
        return write_request

    def __convert_forward_column(self, field_value, field_type):
        value = common_pb2.GenericValue()
        if field_type == common_pb2.GenericValueMeta.FieldType.FT_BYTES:
            value.bytes_value = field_value.encode('UTF-8')
        elif field_type == common_pb2.GenericValueMeta.FieldType.FT_STRING:
            value.string_value = field_value
        elif field_type == common_pb2.GenericValueMeta.FieldType.FT_BOOL:
            value.bool_value = field_value
        elif field_type == common_pb2.GenericValueMeta.FieldType.FT_INT32:
            value.int32_value = field_value
        elif field_type == common_pb2.GenericValueMeta.FieldType.FT_INT64:
            value.int64_value = field_value
        elif field_type == common_pb2.GenericValueMeta.FieldType.FT_UINT32:
            value.uint32_value = field_value
        elif field_type == common_pb2.GenericValueMeta.FieldType.FT_UINT64:
            value.uint64_value = field_value
        elif field_type == common_pb2.GenericValueMeta.FieldType.FT_FLOAT:
            value.float_value = field_value
        elif field_type == common_pb2.GenericValueMeta.FieldType.FT_DOUBLE:
            value.double_value = field_value

        return value
