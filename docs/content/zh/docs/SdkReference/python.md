---
title: "Python SDK"
linkTitle: "Python SDK"
weight: 101
draft: false
---


目前只支持python 3.6及以上的版本。

## 1. 创建客户端
可以创建同步客户端或者异步客户端。

* 同步客户端
```python
from pyproximabe import *
client = Client('127.0.0.1', 16000)
```
Proxima BE服务支持 GRPC 和 HTTP 两种协议，监听在不同的端口上。同步客户端同时支持这两种协议，默认使用GRPC协议。如果有需求，可以用下面的方式来指定协议。
```python
client = Client('127.0.0.1', 16001, 'http')
```

* 异步客户端
```python
from pyproximabe import *
client = AsyncClient(HOST, GRPC_PORT)
```
异步客户端基于asyncio实现，除了需要await结果外，和同步客户端的使用方式完全一致。 异步客户端只支持GRPC协议。

同步或异步客户端支持的参数如下
```python
def __init__(self, host, port=16000, handler='grpc', timeout=10):
```

| 参数   | 说明                              |
| :----- | :-------------------------------- |
| host | 服务器地址，str类型|
| port | 服务器端口，int类型|
| handler | 协议类型，str类型，一般不需要指定。<br />同步客户端支持grpc/http，异步客户端只支持grpc。|
| timeout | 超时时间，单位秒，float类型，默认为10。指定为None则不超时。|

使用完毕后，可以调用`close()`关闭客户端，其参数为空，返回值为None。
```
client.close()
```


## 2. 集合管理
### 2.1. 集合创建
#### 创建直写集合
```python
collection_name = 'iris'
index_column = IndexColumnParam('length', 4, IndexType.PROXIMA_GRAPH_INDEX)
collection_config = CollectionConfig(collection_name, [index_column], ['iris_type'])
status = client.create_collection(collection_config)
if not status.ok():
    # error handling
    logging.error('create collection failed, status=%s', status)
```
首先，创建一个或多个索引列
```python
index_column = IndexColumnParam('length', 4, IndexType.PROXIMA_GRAPH_INDEX)
```
参数如下
```python
def __init__(self,
             name,
             dimension,
             index_type=IndexType.PROXIMA_GRAPH_INDEX,
             data_type=DataType.VECTOR_FP32,
             extra_params=None):
```
| 参数   | 说明                              |
| :----- | :-------------------------------- |
| name | 索引列名称，str类型。|
| dimension | 向量维度，int类型。|
| index_type | 索引类型，[IndexType](#indextype) 类型，默认为PROXIMA_GRAPH_INDEX。|
| data_type | 数据类型，[DataType](#datatype) 类型，默认为VECTOR_FP32。|
| extra_params | 扩展参数，dict类型，默认为空。|

然后，创建`CollectionConfig`。
```python
collection_config = CollectionConfig(collection_name, [index_column], ['iris_type'])
```
(python_collection_config)=
参数如下
```python
def __init__(self,
                collection_name,
                index_column_params,
                forward_column_names=None,
                repository_config=None,
                max_docs_per_segment=0):

```
| 参数   | 说明                              |
| :----- | :-------------------------------- |
| collection_name | 集合名称，str类型。 |
| index_column_params | 索引列列表，List[IndexColumnParam]类型。|
| forward_column_names | 正排列列表，List[str]类型。默认为空。|
| repository_config | 仓库配置，DatabaseRepository类型。默认为None。|
| max_docs_per_segment | 每个segment的最大文档数，long类型。默认为0，文档数不限。 |

最后，调用client接口。
```python
status = client.create_collection(collection_config)
if not status.ok():
    # error handling
    logging.error('create collection failed, status=%s', status)
```
`create_collection()`接收`CollectionConfig`参数，返回[ProximaBeStatus](#错误处理)。
```python
def create_collection(self, collection_config):
```

#### 创建数据库旁路集合
```python
MYSQL_PORT = 3306
MYSQL_HOST = HOST

mysql_table_name = 'iris_table'
mysql_database_name = 'test_db'
mysql_user='root'
mysql_password='root'

index_column = IndexColumnParam('length', 4, IndexType.PROXIMA_GRAPH_INDEX)
database_repository = DatabaseRepository('test_repository',
                                         f'mysql://{MYSQL_HOST}:{MYSQL_PORT}/{mysql_database_name}',
                                         mysql_table_name, mysql_user, mysql_password)
collection_config = CollectionConfig(collection_name, [index_column], ['iris_type'], database_repository)
status = client.create_collection(collection_config)
```
除了需要创建`DatabaseRepository`外，其他参数和直写集合完全相同。目前只支持mysql。
```python
database_repository = DatabaseRepository('test_repository',
                                         f'mysql://{MYSQL_HOST}:{MYSQL_PORT}/{mysql_database_name}',
                                         mysql_table_name, mysql_user, mysql_password)
```
参数列表如下
```python
def __init__(self, repository_name, connection_uri, table_name, user, password):
```
| 参数   | 说明                              |
| :----- | :-------------------------------- |
| repository_name | 仓库名称，str类型。 |
| connection_uri | 数据库连接串，str类型，如"mysql://localhost/database"，不包含用户名和密码。|
| table_name | 表名，str类型。|
| user | 用户名，str类型。 |
| password | 数据库密码，str类型。 |

### 2.2. 描述集合
```python
status, collection_info = client.describe_collection(collection_name)
if not status.ok():
    pass  # error handling
print(collection_info)
```
`describe_collection()`接收str类型的`collection_name`
```python
def describe_collection(self, collection_name):
```
返回值有两个
* [ProximaBeStatus](#错误处理)
* [CollectionInfo](#collectioninfo)

### 2.3. 获取集合统计信息
```python
status, collection_stats = client.stats_collection(collection_name)
if not status.ok():
    pass  # error handling
print(collection_stats)
```
`stats_collection()`接收str类型的`collection_name`
```python
def stats_collection(self, collection_name):
```
返回值有两个
* [ProximaBeStatus](#错误处理)
* [CollectionStats](#collectionstats)
### 2.4. 删除集合
```python
status = client.drop_collection(collection_name)
```
`drop_collection()`接收str类型的`collection_name`，返回[ProximaBeStatus](#错误处理)。

```python
def drop_collection(self, collection_name):
```
### 2.5. 获取集合列表
```python
status, collections_data = client.list_collections()
```
`list_collections()`参数列表如下
```python
def list_collections(self, repository_name=None):
```
| 参数   | 说明                              |
| :----- | :-------------------------------- |
| repository_name | 仓库名称，str类型，默认为空。参数为空时，返回所有的集合列表，否则返回指定仓库下的集合列表。|
返回值有两个
* [ProximaBeStatus](#错误处理)
* List[[CollectionInfo](#collectioninfo)]

## 3. 文档管理
### 3.1. 通用接口
```python
import struct
row_meta = WriteRequest.RowMeta([WriteRequest.IndexColumnMeta('length', 'VECTOR_FP32', 4)],
                                ['iris_type'],
                                [DataType.STRING])
rows = [
    WriteRequest.Row(100001,
                     WriteRequest.OperationType.INSERT,
                     [[5.9,3.0,5.1,1.8]],
                     ['Iris-virginica']),
    WriteRequest.Row(100002,
                     WriteRequest.OperationType.INSERT,
                     ['[5.9,3.0,5.1,1.8]'],
                     ['Iris-virginica']),
    WriteRequest.Row(10002,
                     WriteRequest.OperationType.UPDATE,
                     [struct.pack('<4f', 5.9, 3.0, 5.1, 1.8)],
                     ['Iris-virginica']),
    WriteRequest.Row(10003, WriteRequest.OperationType.DELETE),
]
write_request = WriteRequest(collection_name, rows, row_meta)
status = client.write(write_request)

```
首先，创建`WriteRequest.RowMeta`，描述插入行的数据类型等信息。
```python
row_meta = WriteRequest.RowMeta([WriteRequest.IndexColumnMeta('length', 'VECTOR_FP32', 4)],
                                ['iris_type'],
                                [DataType.STRING])
```
`WriteRequest.IndexColumnMeta`的参数如下
```python
def __init__(self, name, data_type, dimension):
```
| 参数   | 说明                              |
| :----- | :-------------------------------- |
| name | 索引列名称，str类型。|
| data_type | 数据类型，[DataType](#datatype) 类型。|
| dimension | 向量维度，int类型。|
`WriteRequest.RowMeta`的参数如下
```python
def __init__(self,
             index_column_metas,
             forward_column_names=None,
             forward_column_types=None):
```
| 参数   | 说明                              |
| :----- | :-------------------------------- |
| index_column_metas | 索引列列表，List[WriteRequest.IndexColumnMeta]类型。 |
| forward_column_names | 正排列名称列表，List[str]。默认为空。 |
| forward_column_types | 正排列类型列表，List[DataType]。默认为空。 |

然后创建`WriteRequest.Row`。
```python
    WriteRequest.Row(100001,
                     WriteRequest.OperationType.INSERT,
                     [[5.9,3.0,5.1,1.8]],
                     ['Iris-virginica']),
```
`WriteRequest.Row`的参数如下
```python
 def __init__(self,
              primary_key,
              operation_type,
              index_column_values=None,
              forward_column_values=None,
              lsn_context=None):
```
| 参数   | 说明                              |
| :----- | :-------------------------------- |
| primary_key | 主键，long类型。 |
| operation_type | 操作类型， [WriteRequest.OperationType](#writerequestoperationtype) 类型。 |
| index_column_values | 索引列的值列表，list类型，operation_type为删除时不需要指定。支持三种类型<br /> <ul><li>向量list，如[1.0, 2.0, 3.0]</li><li>json数组str，如"[1.0,2.0,3.0]"</li><li>bytes，必须为小端字节序。</li></ul>|
| forward_column_values | 正排列的值列表，list类型。|
| lsn_context | 日志序列号上下文，[LsnContext](#lsncontext)类型，默认为空。一般不需要设置。|
然后创建`WriteRequest`
```python
write_request = WriteRequest(collection_name, rows, row_meta)
```
WriteRequest参数如下
```python
def __init__(self,
             collection_name,
             rows,
             row_meta=None,
             request_id=None,
             magic_number=None):
```
| 参数   | 说明                              |
| :----- | :-------------------------------- |
| collection_name | 集合名称，str类型。 |
| rows | 文档列表，List[WriteRequest.Row]类型。|
| row_meta | 文档元数据，WriteRequest.RowMeta类型。如果所有的operation_type都为DELETE，不需要指定。 |
| request_id | 请求id，str类型，默认为空。一般不需要设置。|
| magic_number | 服务端魔数，long类型，默认为0。一般不需要设置。 |

最后调用client接口。
```python
status = client.write(write_request)
```
`write()`接收`WriteRequest`参数，返回[ProximaBeStatus](#错误处理)。
```python
def write(self, write_request):
```

### 3.2. 删除接口
```python
status = client.delete_document_by_keys(collection_name, [10001, 10002])
```
在只需要删除时，python sdk提供了简化接口，只需要指定集合名称和主键列表。
参数如下
```python
def delete_document_by_keys(self, collection_name, primary_keys):
```
| 参数   | 说明                              |
| :----- | :-------------------------------- |
| collection_name | 集合名称，str类型。|
| primary_keys | 主键列表，List[long]类型。|
`delete_document_by_keys()`返回值为[ProximaBeStatus](#错误处理)。

## 4. 文档查询
### 4.1. 向量查询
```python
status, knn_res = client.query(collection_name,
                               'length',
                               [[5.1, 3.5, 1.4, 0.2],
                                         [5.5, 2.3, 4.0, 1.3]],
                               'VECTOR_FP32',
                               topk=2)
for i, result in enumerate(knn_res.results):
    print(f'Query: {i}')
    for doc in result:
        forward_values = ','.join(
            f'{k}={v}' for k, v in doc.forward_column_values.items())
        print(
            f'    primary_key={doc.primary_key}, score={doc.score}, forward_column_values=[{forward_values}]'
        )
```
`query()`的参数如下
```python
def query(self,
          collection_name,
          column_name,
          features,
          data_type=None,
          dimension=None,
          batch_count=None,
          topk=100,
          is_linear=False,
          radius=None,
          extra_params=None):
```
| 参数   | 说明                              |
| :----- | :-------------------------------- |
| collection_name | 集合名称，str类型。|
| column_name | 索引列名称，str类型。|
| features | 特征，支持三种类型<br /> <ul><li>小端字节序的bytes</li><li>json数组字符串，支持打平或者嵌套json数组。如2个batch的2维向量，既可以写"[1.0,2.0,3.0,4.0]"也可以写"[[1.0,2.0],[3.0,4.0]]"</li><li>向量列表，如[[5.1, 3.5, 1.4, 0.2], [5.5, 2.3, 4.0, 1.3]]</li></ul>|
| data_type | 数据类型，[DataType](#datatype) 类型。默认为空。<ul><li>bytes类型的特征允许为空，使用集合创建时的数据类型。</li><li>向量列表类型的特征时必须设置。</li></ul>|
| dimension | 向量维度，int类型。默认为空。<ul><li>bytes类型的特征必须设置。</li><li>向量列表类型的特征时支持自动推导。</li></ul>|
| batch_count | batch大小，int类型。默认为空。<ul><li>bytes类型的特征必须设置。</li><li>向量列表类型的特征支持自动推导。</li></ul> |
| topk | 单条查询向量返回的结果数，int类型。默认为100。|
| is_linear | 是否做线性查找，bool类型。默认为False，基于索引做查找。|
| radius | 搜索半径，只返回以搜索向量为球心的球体内的向量，float类型。默认为0.0，搜索半径不限。|
| extra_params | 扩展参数，dict类型。 |
返回值有两个
* [ProximaBeStatus](#错误处理)
* [QueryResponse](#queryresponse)

### 4.2. 主键查询
```python
status, res = client.get_document_by_key(collection_name, primary_key=100001)
```
`get_document_by_key()`的参数如下
```python
def get_document_by_key(self, collection_name, primary_key):
```
| 参数   | 说明                              |
| :----- | :-------------------------------- |
| collection_name | 集合名称，str类型。|
| primary_key | 主键，long类型。 |
返回值有两个
* [ProximaBeStatus](#错误处理)。主键不存在时，status.ok() == True.
* [Document](#document)。主键不存在时，返回None。



## 5. 错误处理

#### 错误处理

python sdk接口一般会返回`ProximaBeStatus`类型，其包含两个属性

| 属性   | 说明                              |
| :----- | :-------------------------------- |
| code | 错误码，int类型。|
| reason | 错误详情，str类型。 |

可以通过`ok()`接口来判断是否成功。
```python
def ok(self):
    return self.code == 0
```

```{admonition} 异常安全
:class: warning
请求服务端成功时才会返回ProximaBeStatus，如果客户端参数检查失败或者网络有问题，会抛出ProximaSeException异常。
```
```python
class ProximaSeException(Exception):
    pass
```



## 6. 类型定义

### IndexType
```python
class IndexType(IntEnum):
    UNDEFINED = 0
    PROXIMA_GRAPH_INDEX = 1
```

### DataType
```python
class DataType(IntEnum):
    UNDEFINED = 0
    BINARY = 1
    STRING = 2
    BOOL = 3
    INT32 = 4
    INT64 = 5
    UINT32 = 6
    UINT64 = 7
    FLOAT = 8
    DOUBLE = 9
    VECTOR_BINARY32 = 20
    VECTOR_BINARY64 = 21
    VECTOR_FP16 = 22
    VECTOR_FP32 = 23
    VECTOR_FP64 = 24
    VECTOR_INT4 = 25
    VECTOR_INT8 = 26
    VECTOR_INT16 = 27
```

### WriteRequest.OperationType
```python
class OperationType(IntEnum):
    INSERT = 0
    UPDATE = 1
    DELETE = 2
```

### Document
`Document` 包含以下属性
| 属性   | 说明                              |
| :----- | :-------------------------------- |
| primary_key | 主键，long类型。|
| score | 分值，即距离查询向量的距离，float类型。|
| forward_column_values | 正排名称到值的映射，dict类型。|


### QueryResponse
`QueryResponse`包含以下属性
| 属性   | 说明                              |
| :----- | :-------------------------------- |
| results | 结果列表，List[List[[Document](#document)]]类型。第一重list表示多个查询向量的结果，第二重list表示每个向量的Document列表|
| debug_info | 服务端调试信息，str类型。 |
| latency_us | 服务端统计的耗时，单位为微秒。 |


### CollectionInfo
`CollectionInfo`包含以下属性

| 属性   | 说明                              |
| :----- | :-------------------------------- |
| collection_config | 集合配置， CollectionConfig类型。|
| status | 集合状态，CollectionInfo.Status 类型。 |
| uuid | 集合唯一标识，str类型。 |
| latest_lsn_context | 最新日志序列号上下文， [LsnContext](#lsncontext) 类型。 |
| magic_number | 服务端魔数，long类型。|

{ref}`CollectionConfig定义见这里<python_collection_config>`

`CollectionInfo.Status`定义
```python
class Status(IntEnum):
    """Collection Status"""
    INITIALIZED = 0
    SERVING = 1
    DROPPED = 2
```

### CollectionStats
`CollectionStats`包含以下属性
| 属性   | 说明                              |
| :----- | :-------------------------------- |
| collection_name | 集合名称，str类型。|
| collection_path | 集合路径，str类型。|
| total_doc_count | 文档总数，long类型。|
| total_segment_count | 分段总个数，long类型。|
| total_index_file_count | 索引文件总个数，long类型。|
| total_index_file_size | 索引文件总大小，long类型。|
| segment_stats | 分段统计信息，List[CollectionStats.SegmentStats]类型。|

`CollectionStats.SegmentStats`包含以下属性
| 属性   | 说明                              |
| :----- | :-------------------------------- |
|  segment_id | 段id，int类型。|
|  state | 段状态，CollectionStats.SegmentState类型。|
|  doc_count | 文档数，long类型。|
|  index_file_count | 索引文件个数，long类型。|
|  index_file_size | 索引文件大小，long类型。|
|  min_doc_id | 当前分段的最小文档id，long类型。|
|  max_doc_id | 当前分段的最大文档id，long类型。|
|  min_primary_key | 当前分段的最小主键，long类型。|
|  max_primary_key | 当前分段的最大主键，long类型。|
|  min_timestamp | 当前分段的最小时间戳，long类型。|
|  max_timestamp | 当前分段的最大时间戳，long类型。|
|  min_lsn | 当前分段的最小日志序列号，long类型。|
|  max_lsn | 当前分段的最大日志序列号，long类型。|
|  segment_path | 段文件路径，str类型。|

`CollectionStats.SegmentState`定义如下
```python
class SegmentState(IntEnum):
    CREATED = 0
    WRITING = 1
    DUMPING = 2
    COMPACTING = 3
    PERSIST = 4
```

### LsnContext
`LsnContext` 包含以下属性
| 属性   | 说明                              |
| :----- | :-------------------------------- |
| lsn | 序列号，long类型。|
| context | 上下文，str类型。 |



## 7. 其他示例

* [直写集合示例](/data/python-examples/example.py)
* [mysql旁路集合示例](/data/python-examples/mysql_example.py)
* [异步直写集合示例](/data/python-examples/example_async.py)

## 8. Python API Reference
<!-- TODO<cdz>: 添加相应的 API 链接 -->