import time
import random
import numpy as np
import asyncio

from pyproximase import *

HOST = '127.0.0.1'

GRPC_PORT = 16000


async def main():
    client = AsyncClient(HOST, GRPC_PORT)

    collection_name = 'iris'

    # drop collection if exist
    _ = await client.drop_collection(collection_name)

    # create collection
    index_column = IndexColumnParam(name='length',
                                    dimension=4,
                                    index_type=IndexType.PROXIMA_GRAPH_INDEX)
    collection_config = CollectionConfig(collection_name=collection_name,
                                         index_column_params=[index_column],
                                         max_docs_per_segment=10000,
                                         forward_column_names=['iris_type'])
    status = await client.create_collection(collection_config)
    print('---------create collection------------')
    print(status)
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
    # create 3 requests
    requests = [
        client.write(
            WriteRequest(collection_name=collection_name,
                         rows=rows[0:5],
                         row_meta=row_meta)),
        client.write(
            WriteRequest(collection_name=collection_name,
                         rows=rows[5:10],
                         row_meta=row_meta)),
        client.write(
            WriteRequest(collection_name=collection_name,
                         rows=rows[10:],
                         row_meta=row_meta))
    ]
    print("-----------write-------------")
    responses = await asyncio.gather(*requests)
    for r in responses:
        print(r)
    print('\n')
    time.sleep(1)

    status = await client.delete_document_by_keys(collection_name, 2)
    print("-----------delete-------------")
    print(status)
    print('\n')

    # get document by key
    get_req = client.get_document_by_key(collection_name, primary_key=3)
    query_req = client.query(collection_name,
                             column_name='length',
                             features=[[5.1, 3.5, 1.4, 0.2],
                                       [5.5, 2.3, 4.0, 1.3]],
                             data_type='VECTOR_FP32',
                             topk=2)

    responses = await asyncio.gather(get_req, query_req)
    print('---------get document by key------------')
    status, get_rsp = responses[0]
    print(status)
    print(get_rsp)
    print('\n')

    # query
    print('---------query knn------------')
    status, knn_rsp = responses[1]
    print(status)
    print(knn_rsp)
    for i, result in enumerate(knn_rsp.results):
        print(f'Query: {i}')
        for doc in result:
            forward_values = ','.join(
                f'{k}={v}' for k, v in doc.forward_column_values.items())
            print(
                f'    primary_key={doc.primary_key}, score={doc.score}, forward_column_values=[{forward_values}]'
            )
    print('\n')

    responses = await asyncio.gather(
        client.stats_collection(collection_name),
        client.describe_collection(collection_name),
        client.list_collections()
    )
    # stats collection
    status, collection_stats = responses[0]
    print('----------------get collection stats----------------')
    print(status)
    print(collection_stats)
    print('\n')

    # describe
    status, collection_info = responses[1]
    print('----------------describe collection----------------')
    print(status)
    print(collection_info)
    print('\n')

    # list all collections
    status, collections_data = responses[2]
    print('----------------list collections----------------')
    print(status)
    print(collections_data)
    print('\n')

    # drop
    status = await client.drop_collection(collection_name)
    print('------------------drop-----------------')
    print(status)


loop = asyncio.get_event_loop()
result = loop.run_until_complete(main())
