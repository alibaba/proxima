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

import time
import random
import numpy as np

from pyproximabe import *

HOST = '127.0.0.1'

GRPC_PORT = 16000

HTTP_PORT = 16001

TIMEOUT = 10

handler = random.choice(['grpc', 'http'])
print('use', handler)

if handler == 'grpc':
    client = Client(HOST, GRPC_PORT, handler='grpc', timeout=TIMEOUT)
else:
    client = Client(HOST, HTTP_PORT, handler='http', timeout=TIMEOUT)

collection_name = 'iris'

# drop collection if exist
_ = client.drop_collection(collection_name)

# create collection
index_column = IndexColumnParam(name='length',
                                dimension=4,
                                index_type=IndexType.PROXIMA_GRAPH_INDEX)
collection_config = CollectionConfig(collection_name=collection_name,
                                     index_column_params=[index_column],
                                     max_docs_per_segment=10000,
                                     forward_column_names=['iris_type'])
status = client.create_collection(collection_config)
print('---------create collection------------')
print(status)
if not status.ok():
    # error handling
    pass
print('\n')

# write
rows = []
iris_datas = [
    ('[4.8,3.0,1.4,0.3]', 'Iris-setosa'),
    ('[5.1,3.8,1.6,0.2]', 'Iris-setosa'),
    ('[4.6,3.2,1.4,0.2]', 'Iris-setosa'),
    ('[5.3,3.7,1.5,0.2]', 'Iris-setosa'),
    ('[5.0,3.3,1.4,0.2]', 'Iris-setosa'),
    ('[7.0,3.2,4.7,1.4]', 'Iris-versicolor'),
    ('[6.4,3.2,4.5,1.5]', 'Iris-versicolor'),
    ('[6.9,3.1,4.9,1.5]', 'Iris-versicolor'),
    ('[5.5,2.3,4.0,1.3]', 'Iris-versicolor'),
    ('[6.5,2.8,4.6,1.5]', 'Iris-versicolor'),
    ('[6.7,3.0,5.2,2.3]', 'Iris-virginica'),
    ('[6.3,2.5,5.0,1.9]', 'Iris-virginica'),
    ('[6.5,3.0,5.2,2.0]', 'Iris-virginica'),
    ('[6.2,3.4,5.4,2.3]', 'Iris-virginica'),
    ('[5.9,3.0,5.1,1.8]', 'Iris-virginica'),
]
row_meta = WriteRequest.RowMeta(index_column_metas=[WriteRequest.IndexColumnMeta(name='length',
                                                                                 data_type='VECTOR_FP32',
                                                                                 dimension=4)],
                                forward_column_names=['iris_type'],
                                forward_column_types=[DataType.STRING])

for i, data in enumerate(iris_datas):
    rows.append(
        WriteRequest.Row(primary_key=i,
                         operation_type=WriteRequest.OperationType.INSERT,
                         index_column_values=[data[0]],
                         forward_column_values=[data[1]]))
write_request = WriteRequest(collection_name=collection_name,
                             rows=rows,
                             row_meta=row_meta)
status = client.write(write_request)
print("-----------write-------------")
print(status)
print('\n')
time.sleep(1)

status = client.delete_document_by_keys(collection_name, 2)
print("-----------delete-------------")
print(status)
print('\n')

# get document by key
status, res = client.get_document_by_key(collection_name, primary_key=3)
print('---------get document by key------------')
print(status)
print(res)
print('\n')

# query
status, knn_res = client.query(collection_name,
                               column_name='length',
                               features=[[5.1, 3.5, 1.4, 0.2],
                                         [5.5, 2.3, 4.0, 1.3]],
                               data_type='VECTOR_FP32',
                               topk=2)
print('---------query knn------------')
print(status)
print(knn_res)
for i, result in enumerate(knn_res.results):
    print(f'Query: {i}')
    for doc in result:
        forward_values = ','.join(
            f'{k}={v}' for k, v in doc.forward_column_values.items())
        print(
            f'    primary_key={doc.primary_key}, score={doc.score}, forward_column_values=[{forward_values}]'
        )
print('\n')

# query by numpy array
array = np.array([[5.1, 3.5, 1.4, 0.2], [5.5, 2.3, 4.0, 1.3]],
                 dtype=np.float32)
_, _ = client.query(collection_name,
                    column_name='length',
                    features=array.tobytes(),
                    batch_count=2,
                    dimension=4,
                    data_type='VECTOR_FP32',  # optional for bytes feature
                    topk=2)

# stats collection
status, collection_stats = client.stats_collection(collection_name)
print('----------------get collection stats----------------')
print(status)
print(collection_stats)
print('\n')

# describe
status, collection_info = client.describe_collection(collection_name)
print('----------------describe collection----------------')
print(status)
print(collection_info)
print('\n')

# list all collections
status, collections_data = client.list_collections()
print('----------------list collections----------------')
print(status)
print(collections_data)
print('\n')

# drop
status = client.drop_collection(collection_name)
print('------------------drop-----------------')
print(status)

client.close()
