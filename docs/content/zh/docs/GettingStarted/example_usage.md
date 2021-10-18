---
title: "使用样例"
linkTitle: "使用样例"
draft: false
weight: 7
---


在上文[安装指南]({{< ref "installation.md" >}})中，我们详细介绍了 ProximaBE 的安装和启动方法， 接下来我们将使用一个样例来介绍 ProximaBE 的服务使用方法。 在样例程序中，我们将使用两种示例代码演示创建一个名为"Plants"的集合，并尝试插入一批数据，然后查询比对的过程。



## 1. Http 示例

示例中我们选用 curl 工具来演示 http 协议的访问 ProximaBE 过程，用户也可以使用其它的 http 发送工具来替代。

### 1.1. 创建Collection

下列示例脚本将创建一个名为 Plants 的集合，有一个名为 "ImageVector" 的向量索引列， 以及 "Price" 和 "Description" 两个正排列。

其中索引列的具体承载数据类型为 8 维 float 类型的向量，索引类型为向量图索引。

```s
$ curl -X POST http://127.0.0.1:16001/v1/collection/Plants \
       -d '{"collection_name":"Plants",
          	"forward_column_names":["Price", "Description"],
          	"index_column_params":[{
            	"column_name":"ImageVector",
            	"index_type":"IT_PROXIMA_GRAPH_INDEX",
            	"data_type":"DT_VECTOR_FP32",
            	"dimension": 8}]
           }'
```

Exec output:

```
{"code":0,"reason":"Success"}
```



### 1.2 查看Collection

创建成功的集合，我们可以直接使用访问查看。

```s
$ curl -X GET http://127.0.0.1:16001/v1/collection/Plants
```

Exec output:

```
{"status":{"code":0,"reason":"Success"},"collection":{"uuid":"8e5b79bbdc198cc2dccdde0710f86f45","config":
{"collection_name":"Plants","index_column_params":
[{"data_type":"DT_VECTOR_FP32","dimension":8,"index_type":"IT_PROXIMA_GRAPH_INDEX","column_name":"ImageVector","
extra_params":[]}],"forward_column_names":
["Price","Description"],"max_docs_per_segment":"18446744073709551615"},"status":"CS_SERVING","magic_number":"0"}
}
```



### 1.3. 写入文档

下面示例中向 Plants 集合插入了 2 条数据(受限于展示，用户可以自由扩展 rows 字段)，注意rows中的字段值顺序应与 row_meta 中的描述保持一致。

```s
$ curl -X POST http://127.0.0.1:16001/v1/collection/Plants/index \
       -d '{"collection_name":"Plants",
            "row_meta": {
              "forward_column_names":["Price", "Description"],
              "index_column_metas": [{
                "column_name":"ImageVector",
                "data_type":"DT_VECTOR_FP32",
                "dimension":8}]
            },
            "rows":[
              {
                "primary_key":0,
                "operation_type":"OP_INSERT",
                "forward_column_values":{
                  "values":[{"float_value":0.1}, {"string_value":"ginkgo tree with number 0"}]},
                "index_column_values":{
                  "values":[{"string_value":"[0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8]"}]}
              },
              {
                "primary_key":1,
                "operation_type":"OP_INSERT",
                "forward_column_values":{
                  "values":[{"float_value":1.1}, {"string_value":"ginkgo tree with number 1"}]},
                "index_column_values":{
                  "values":[{"string_value":"[1.1, 1.2, 1.3, 1.4, 1.5, 1.6, 1.7, 1.8]"}]}
              }
            ]
          }'
```

Exec output:

```
{"code":0,"reason":"Success"}
```



### 1.4. 查询文档

下面示例代码中展示了我们Knn 查找top1图像的过程，结果应该准确命中第一条文档，用户可以自由设置topk。

```s
$ curl -X POST http://127.0.0.1:16001/v1/collection/Plants/query \
       -d '{"collection_name":"Plants",
            "query_type":"QT_KNN",
            "knn_param": {
              "column_name":"ImageVector",
              "topk":1,
              "matrix": "[0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8]",
              "batch_count":1,
              "dimension":8,
              "data_type":"DT_VECTOR_FP32"}
           }'
```

Exec output:

```
{"status":{"code":0,"reason":"Success"},"results":[{"documents":
[{"score":0,"primary_key":"0","forward_column_values":[{"key":"Price","value":{"float_value":0.1}},
{"key":"Description","value":{"string_value":"ginkgo tree with number
0"}}]}]}],"debug_info":"","latency_us":"636"}
```



### 1.5 Collection统计

经过一段时间累积，我们希望能够获取某个集合的详细信息，则可以通过下列示例访问:

```s
$ curl -X GET http://127.0.0.1:16001/v1/collection/Plants/stats
```

Exec output:

```
{"status":{"code":0,"reason":"Success"},"collection_stats":{"segment_stats":
[{"state":"SS_WRITING","max_lsn":"0","min_lsn":"0","doc_count":"2","max_doc_id":"1","min_doc_id":"0","segment_id
":0,"segment_path":"","max_timestamp":"0","min_timestamp":"0","index_file_size":"2142208","max_primary_key":"1",
"min_primary_key":"0","index_file_count":"2"}],"collection_name":"Plants","collection_path":"/home/x x x/c
ode/proxima-
se/build/test_dir/Plants","total_doc_count":"2","total_segment_count":"1","total_index_file_size":"22241280","to
tal_index_file_count":"6"}}
```



### 1.6 删除Collection

```s
$ curl -X DELETE http://127.0.0.1:16001/v1/collection/Plants
```

Exec output:

```
{"code":0,"reason":"Success"}
```



## 2. Python 示例

### 2.1. Python SDK 安装

在安装python sdk之前请确保系统中已安装python3.6以及pip程序。

```s
$ pip install -i https://pypi.antfin-inc.com/simple/ -U pyproximabe==0.1.2
```


### 2.2. 连接Client

在具体操作之前，我们需要创建client连接ProximaBE，当前支持http和grpc协议。Client默认为同步请求，同时我们也支持AsyncClient做异步请求。参考代码如下:

```python
from pyproximabe import *

# Init client
client = Client('127.0.0.1', 16000)
```



### 2.3. 创建Collection

在下列代码中，我们创建了一个名为Plants的集合。其中一个索引列名为"ImageVector"，顾名思义主要是存放图片向量，以及"Price"和"Description"两个正排列。

创建Collection的参数主要是需要描述索引列的结构，比如索引类型、数据类型、维度等信息。 正排列则仅描述列名即可。

```python
# Init index column
index_column = IndexColumnParam(name='ImageVector',
                                dimension=8,
                                data_type=DataType.VECTOR_FP32,
                                index_type=IndexType.PROXIMA_GRAPH_INDEX)
# Init collection config
collection_config = CollectionConfig('Plants',
                                     index_column_params=[index_column],
                                     max_docs_per_segment=0,
                                     forward_column_names=['Price','Description'])
# Create collection
status = client.create_collection(collection_config)

# Check Return
print(status)
```

Exec output:

```
success
```



### 2.3 查看Collection

```python
# Get collection info
status = client.describe_collection('Plants')

print(status)
```

Exec output:

```
(<pyproximabe.core.types.ProximaBeStatus object at 0x7f63b0a41ac8>, CollectionInfo{'collection_config':
CollectionConfig{'collection_name': 'Plants', 'index_column_params': [IndexColumnParam{'name': 'ImageVector',
'dimension': 8, 'index_type': <IndexType.PROXIMA_GRAPH_INDEX: 1>, 'data_type': <DataType.VECTOR_FP32: 23>,
'extra_params': {}}], 'max_docs_per_segment': 18446744073709551615, 'forward_column_names': ['Price',
'Description'], 'repository_config': None}, 'status': <Status.SERVING: 1>, 'uuid':
'dbd57611e812164b7e7b697ea0af684c', 'latest_lsn_context': LsnContext{'lsn': 0, 'context': ''}, 'magic_number':
0})
```



### 2.4. 写入文档

名为"plants"的Collection创建完成之后，我们向其中写入100条文档。

发送数据前，我们需要描述发送的数据格式RowMeta，主要供后端校验使用。至于写入模式分两种，可以支持批量，也可以支持单条发送，而写入请求类型支持OperationType.INSERT，OperationType.DELETE，OperationType.UPDATE 三种。

```python
# Set record data format
index_column_meta = WriteRequest.IndexColumnMeta(name='ImageVector',
                                                 data_type=DataType.VECTOR_FP32,
                                                 dimension=8)
row_meta = WriteRequest.RowMeta(index_column_metas=[index_column_meta],
                                forward_column_names=['Price','Description'],
                                forward_column_types=[DataType.FLOAT, DataType.STRING])
# Send 100 records
rows = []
for i in range(0, 100):
  vector = [i+0.1, i+0.2, i+0.3, i+0.4, i+0.5, i+0.6, i+0.7, i+0.8]
  price = i + 0.1
  description = "ginkgo tree with number " + str(i)
  row = WriteRequest.Row(primary_key=i,
                         operation_type=WriteRequest.OperationType.INSERT,
                         index_column_values=[vector],
                         forward_column_values=[price, description])
  rows.append(row)

write_request = WriteRequest(collection_name='Plants',
                             rows=rows,
                             row_meta = row_meta)
status = client.write(write_request)
print(status)
```

Exec Output:

```
success
```



### 2.5. 查询文档

插入完100条文档到Plants之后，我们挑选5条查询看看，是否能够准确返回对应的图片。

```python
# Query
query_vector = [0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8]
status, knn_res = client.query(collection_name='Plants',
                               column_name='ImageVector',
                               features=query_vector,
                               data_type=DataType.VECTOR_FP32,
                               topk = 5)
print(status)
print(knn_res)
```

Exec ouput:

```
success

QueryResponse{'results': [[Document{'primary_key': 0, 'score': 0.0, 'forward_column_values': {'Price':
0.10000000149011612, 'Description': 'ginkgo tree with number 0'}}, Document{'primary_key': 1, 'score': 8.0,
'forward_column_values': {'Price': 1.100000023841858, 'Description': 'ginkgo tree with number 1'}},
Document{'primary_key': 2, 'score': 32.0, 'forward_column_values': {'Price': 2.0999999046325684, 'Description':
'ginkgo tree with number 2'}}, Document{'primary_key': 3, 'score': 72.0, 'forward_column_values': {'Price':
3.0999999046325684, 'Description': 'ginkgo tree with number 3'}}, Document{'primary_key': 4, 'score': 128.0,
'forward_column_values': {'Price': 4.099999904632568, 'Description': 'ginkgo tree with number 4'}}]],
'debug_info': '', 'latency_us': 269}
```



### 2.6. Collection统计

我们也可以通过统计接口查看Collection的内部数据和状态。

```python
# Get collection statistics
status, collection_stats = client.stats_collection('Plants')

print(status)
print(collection_stats)
```

Exec ouput:

```
success
CollectionStats{'collection_name': 'Plants', 'collection_path': '/home/xxx/code/proxima-
be/build/test_dir/Plants', 'total_doc_count': 100, 'total_segment_count': 1, 'total_index_file_count': 6,
'total_index_file_size': 22241280, 'segment_stats': [SegmentStats{'segment_id': 0, 'state':
<SegmentState.WRITING: 1>, 'doc_count': 100, 'index_file_count': 2, 'index_file_size': 2142208, 'min_doc_id': 0,
'max_doc_id': 99, 'min_primary_key': 0, 'max_primary_key': 99, 'min_timestamp': 0, 'max_timestamp': 0, 'min_lsn': 0, 'max_lsn': 0, 'segment_path': ''}]}
```



### 2.7. 删除Collection

实验完关于"plants"的一切之后，我们希望能够删除Collection。

```python
# Drop collection
status = client.drop_collection('Plants')

print(status)
```

Exec output:

```
success
```

