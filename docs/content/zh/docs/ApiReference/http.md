---
title: "REST APIs"
linkTitle: "REST APIs"
weight: 51
draft: false
---

ProximaBE  同时提供了 RESTful API，以及 GRPC API 两种协议，以满足在不同应用场景下的集成需求（相较于 GRPC协议，RESTful API 有明显的适用范围、语言原生支持的优势），对于性能敏感的应用场景，建议使用GRPC协议，将在后续进行讲解。

目前支持的 APIs 列表，可统分为两类api,一类是集合管理接口，另外一类是文档管理接口。

## 1 版本

**请求：**

```s
$ HTTP 1.1 GET /service_version
```
- - -

**返回值：**

- status: *[ProximaBE  执行状态](#41-执行状态)* 服务器执行状态
- version：*String,* 发布版本的字符串标识

- - -

**示例：**

```shell
# Example: Get the version of ProximaBE 
##################################################
# Request:
curl -X GET \
  http://11.122.49.225:16100/service_version \
  -H 'cache-control: no-cache'

##################################################
# Response:
{
    "status": {
        "code": 0,
        "reason": "Success"
    },
    "version": "0.1.0-50-g16af91e"
}
```

## 2. 集合管理
用户可通过 API 进行集合的增、删、改（退后发布）、查等功能。

### 2.1 集合增、删、查
目前发布的 ProximaBE  0.1.0 版本中暂不支持集合的更新操作。

#### 2.1.1 创建集合

**请求：**

```s
$ HTTP 1.1 POST /v1/collection/{name}
```
- - -
**路径参数：**

- name: 需要创建的集合名称（需全局唯一）
- - -

**请求主体：**

- collection&#95;name：*String 可选参数,* 默认与路径参数同名
- max&#95;docs&#95;per&#95;segment: *Uint64 可选参数，默认值为系统最大值~(uint64)0,* 设置集合数据的分片大小阈值。
- forward&#95;column&#95;names：*List[String,...] 可选参数,* 正排名列表,定义用户为文档关联的属性列表。
- index&#95;column&#95;params：*List[<u>IndexColumnParam</u>,...],* 索引列表
	- column&#95;name: *String,* 索引列名称
	- index&#95;type: *String,* 索引类型名，可选参数如下表：
		- IT&#95;PROXIMA&#95;GRAPH&#95;INDEX： 图式索引
    - data&#95;type: *[DataTypes](#42-索引列数据类型),* 索引列数据类型，只能使用字符串标识
    - dimension： *Uint32,* 数据维数，仅针对向量列有效
    - extra&#95;params: *List[KeyValuePair,...],* 高阶参数列表
		- key: *String, * 参数名称
		- value: "String, " 参数值
- - -

**返回值：**

请参考：[ProximaBE  执行状态](#41-执行状态)

- - -

**示例：**

```s
# Example: Create Collection with two index colums
##################################################
# Request:
curl -X POST \
  http://11.122.49.225:16100/v1/collection/example \
  -H 'cache-control: no-cache' \
  -H 'content-type: application/json' \
  -d '{
    "collection_name":"example",
    "forward_column_names":[
        "forward",
        "forward1"
    ],
    "index_column_params":[
        {
            "column_name":"column",
            "index_type":"IT_PROXIMA_GRAPH_INDEX",
            "data_type":"DT_VECTOR_FP32",
            "dimension":8,
            "extra_params":[
                {
                    "key":"ef_search",
                    "value":"200"
                }
            ]
        },
        {
            "column_name":"column1",
            "index_type":"IT_PROXIMA_GRAPH_INDEX",
            "data_type":"DT_VECTOR_FP16",
            "dimension":128
        }
    ]
}'

##################################################
# Response:
{
    "code": 0,
    "reason": ""
}
```

- - -

#### 2.1.2 查询集合

**请求：**

```s
$ HTTP 1.1 GET /v1/collection/{name}
```
- - -
**路径参数：**

- name: 需要查询的集合名称
- - -

**返回值：**
(CollectionInfo)=

- status: *[Status](#41-执行状态),* 服务执行状态状态
- collection： *CollectionInfo,* 集合详细信息
	- uuid: *String,* 集合全局唯一ID
	- status: *String,* 集合状态码，可选值如下：
		- CS&#95;INITIALIZED: 集合已初始化完成
		- CS&#95;SERVING: 集合正常提供服务
		- CS&#95;DROPPED： 集合已删除
	- magic&#95;number: *Uint64,* 集合魔法数
	- config： *[CollectionConfig](#211-创建集合), * 请参考创建集合的请求主体

- - -

**示例：**

```s
# Example: Describe Collection
##################################################
# Request:
curl -X GET \
  http://11.122.49.225:16100/v1/collection/example \
  -H 'cache-control: no-cache'

##################################################
# Response:
{
    "status": {
        "code": 0,
        "reason": ""
    },
    "collection": {
        "uuid": "d918becac22e471db41d55050809b8af",
        "config": {
            "collection_name": "example",
            "index_column_params": [
                {
                    "data_type": "DT_VECTOR_FP16",
                    "dimension": 128,
                    "index_type": "IT_PROXIMA_GRAPH_INDEX",
                    "column_name": "column1",
                    "extra_params": []
                },
                {
                    "data_type": "DT_VECTOR_FP32",
                    "dimension": 8,
                    "index_type": "IT_PROXIMA_GRAPH_INDEX",
                    "column_name": "column",
                    "extra_params": [
                        {
                            "key": "ef_search",
                            "value": "200"
                        }
                    ]
                }
            ],
            "forward_column_names": [
                "forward",
                "forward1"
            ],
            "max_docs_per_segment": "18446744073709551615"
        },
        "status": "CS_SERVING",
        "magic_number": "0"
    }
}
```

- - -

#### 2.1.3 删除集合

**请求：**

```s
$ HTTP 1.1 DELETE /v1/collection/{name}
```
- - -
**路径参数：**

- name: 需要删除的集合名称
- - -

**返回值：**

请参考：[ProximaBE  执行状态](#41-执行状态)

- - -

**示例：**

```s
# Example: Delete Collection
##################################################
# Request:
curl -X DELETE \
  http://11.122.49.225:16100/v1/collection/example \
  -H 'cache-control: no-cache'

##################################################
# Response:
{
    "code": 0,
    "reason": ""
}
```

### 2.2 集合统计

获取集合的统计信息，其中包含不仅限于文件数，大小，文档个数等信息。

**请求：**

```s
$ HTTP 1.1 GET /v1/collection/{name}/stats
```
- - -
**路径参数：**

- name: 需要获取统计信息的集合名称
- - -

**返回值：**

- status: *[ProximaBE  执行状态](#41-执行状态)*
- collection&#95;stats: *CollectionStats,* 集合统计信息，完整定义如下：
	- collection&#95;name: *String,* 集合名称
	- collection&#95;path: *String,* 集合的存储路径
    - total&#95;doc&#95;count: *String,* 集合的总文档个数
    - total&#95;segment&#95;count: *String,* 集合分片个数
    - total&#95;index&#95;file&#95;size: *String,* 集合总的存储大小
    - total&#95;index&#95;file&#95;count: *String,* 集合内总的文件个数
	- segment&#95;stats: *SegmentStats,* 分片统计信息**列表**，多片模式下会有多个结果
		- state: *String,* 分片状态码，可能的值罗列如下：
			- SS&#95;WRITING： 分片正在写入
		- max&#95;lsn: *String,* 分片内最大的记录号
        - min&#95;lsn: *String,* 分片内最小的记录号
        - doc&#95;count: *String,* 分片内文档个数
        - max&#95;doc&#95;id: *String,* 分片内最大文档ID
        - min&#95;doc&#95;id: *String,* 分片内最小文档ID
        - segment&#95;id: *Uint32,* 分片编号
        - segment&#95;path: *String,* 分片的存储路径
        - max&#95;timestamp: *String,* 分片内文档的最大时间戳
        - min&#95;timestamp: *String,* 分片内文档的最小时间戳
        - index&#95;file&#95;size: *String,* 分片的存储大小
        - max&#95;primary&#95;key: *String,* 分片内文档最大的主键值
        - min&#95;primary&#95;key: *String,* 分片内文档最小的主键值
        - index&#95;file&#95;count: *Uint32,* 分片内文件个数


- - -

**示例：**

```s
# Example: Stats Collection
##################################################
# Request:
curl -X GET \
  http://11.122.49.225:16100/v1/collection/example/stats \
  -H 'cache-control: no-cache'

##################################################
# Response:
{
    "status": {
        "code": 0,
        "reason": ""
    },
    "collection_stats": {
        "segment_stats": [
            {
                "state": "SS_WRITING",
                "max_lsn": "0",
                "min_lsn": "4294967295",
                "doc_count": "0",
                "max_doc_id": "0",
                "min_doc_id": "0",
                "segment_id": 0,
                "segment_path": "",
                "max_timestamp": "0",
                "min_timestamp": "4294967295",
                "index_file_size": "3223552",
                "max_primary_key": "0",
                "min_primary_key": "4294967295",
                "index_file_count": "3"
            }
        ],
        "collection_name": "example",
        "collection_path": "/home/xiaoxin.gxx/workspace/test/r/indices/example",
        "total_doc_count": "0",
        "total_segment_count": "1",
        "total_index_file_size": "7913472",
        "total_index_file_count": "7"
    }
}
```

### 2.3 获取集合列表

获取满足条件的集合列表，当请求参数没有时默认为获取所有集合。

**请求：**

```s
$ HTTP 1.1 GET /v1/collections?repo={repo}
```
- - -
**请求参数：**

- repo: *可选，* 获取已关联到指定Repo的集合列表

- - -

**返回值：**

- status: *[ProximaBE  执行状态](#41-执行状态)*
- collections: *{ref}`CollectionInfo <CollectionInfo>`,* 满足条件的集合列表
- - -

**示例：**

```s
# Example: List Collections
##################################################
# Request:
curl -X GET \
  http://11.122.49.225:16100/v1/collections?repo= \
  -H 'cache-control: no-cache'

##################################################
# Response:
{
    "status": {
        "code": 0,
        "reason": ""
    },
    "collections": [
        {
            "uuid": "d918becac22e471db41d55050809b8af",
            "config": {
                "collection_name": "example",
                "index_column_params": [
                    {
                        "data_type": "DT_VECTOR_FP16",
                        "dimension": 128,
                        "index_type": "IT_PROXIMA_GRAPH_INDEX",
                        "column_name": "column1",
                        "extra_params": []
                    },
                    {
                        "data_type": "DT_VECTOR_FP32",
                        "dimension": 8,
                        "index_type": "IT_PROXIMA_GRAPH_INDEX",
                        "column_name": "column",
                        "extra_params": [
                            {
                                "key": "ef_search",
                                "value": "200"
                            }
                        ]
                    }
                ],
                "forward_column_names": [
                    "forward",
                    "forward1"
                ],
                "max_docs_per_segment": "18446744073709551615"
            },
            "status": "CS_SERVING",
            "magic_number": "0"
        },
        {
            "uuid": "620976bd0d6728bb5ad566912f457066",
            "config": {
                "collection_name": "example3",
                "index_column_params": [
                    {
                        "data_type": "DT_VECTOR_FP16",
                        "dimension": 128,
                        "index_type": "IT_PROXIMA_GRAPH_INDEX",
                        "column_name": "column1",
                        "extra_params": []
                    },
                    {
                        "data_type": "DT_VECTOR_FP32",
                        "dimension": 8,
                        "index_type": "IT_PROXIMA_GRAPH_INDEX",
                        "column_name": "column",
                        "extra_params": [
                            {
                                "key": "ef_search",
                                "value": "200"
                            }
                        ]
                    }
                ],
                "forward_column_names": [
                    "forward",
                    "forward1"
                ],
                "max_docs_per_segment": "18446744073709551615"
            },
            "status": "CS_SERVING",
            "magic_number": "0"
        }
    ]
}
```

## 3 文档管理

### 3.1 文档插入、删除、更新
通过指定Rows[*].Row.operation&#95;type可进行不同的文档操作

**请求：**

```s
$ HTTP 1.1 POST /v1/collection/{name}/index
```
- - -
**请求主体：**

- request&#95;id: *String, 可选参数* 请求ID(用于与外部系统关联)
- collection&#95;name：*String 可选参数,* 默认与路径参数同名
- row_meta: *RowMeta,* 集合描述信息，其中包含正排列名称列表，以及索引列列表
	- forward&#95;column&#95;names: *List[String],* 正排列名称列表
	- index&#95;olumn#95;metas: *List[IndexColumnMeta],* 索引列配置列表
		- column&#95;name: *String,* 索引列名称
		- data&#95;type: *[DataTypes](#42-索引列数据类型),* 索引列数据类型
		- dimension: *Uint32,* 索引列数据维度（仅针对向量列有效）
- rows: *List[[4.3 文档数据](#43-文档数据)...],* 文档数据列表
- magic&#95;number: *String, 可选参数* 字符串标识的64位无符号整数，魔数（用于与外部系统关联）

- - -

**返回值：**

请参考：[ProximaBE  执行状态](#41-执行状态)

- - -
**示例：**

```shell
# Example: Insert document into collection
##################################################
# Request:
curl -X POST \
  http://11.122.49.225:16100/v1/collection/example/index \
  -H 'cache-control: no-cache' \
  -H 'content-type: application/json' \
  -d '{
    "collection_name":"example",
    "row_meta":{
        "forward_column_names":[
            "forward",
            "forward1"
        ],
        "index_column_metas":[
            {
                "column_name":"column",
                "data_type":"DT_VECTOR_FP32",
                "dimension":8
            }
        ]
    },
    "rows":[
        {
            "primary_key":"5",
            "index_column_values":{
                "values":[
                    {
                        "string_value":"[1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0]"
                    }
                ]
            },
            "forward_column_values":{
                "values":[
                    {
                        "int64_value":"1"
                    },
                    {
                        "float_value":1.1
                    }
                ]
            }
        },
        {
            "primary_key":"3",
            "index_column_values":{
                "values":[
                    {
                        "bytes_value":"QagAAEGwAABBuAAAQcAAAEHIAABB0AAAQdgAAEHgAAA="
                    }
                ]
            },
            "forward_column_values":{
                "values":[
                    {
                        "int64_value":"2"
                    },
                    {
                        "float_value":2.2
                    }
                ]
            },
            "lsn_context":{
                "context":"write context hear"
            }
        }
    ]
}'

##################################################
# Response:
{
    "code": 0,
    "reason": "Success"
}
```


### 3.2 文档查询
执行近邻搜索

**请求：**

```shell
HTTP 1.1 POST /v1/collection/{name}/query
```
- - -
**请求主体：**

- collection&#95;name：*String 可选参数,* 默认与路径参数同名
- debug&#95;mode: *Bool, 可选参数,* 打开调试模式，该操作会加大请求的latency，请不要在生产环境中使用。
- knn&#95;param: *[4.4 KnnQueryParam](#45-近邻检索参数),* 近邻搜索请求参数

- - -

**返回值：**

- status: *[ProximaBE  执行状态](#41-执行状态),* 查询请求状态
- results: *List[[4.5 文档列表](#46-文档列表),...]* 文档列表
- debug&#95;info: *String,* Json格式的字符串表达
- latency&#95;us: *String,* 请求延时

- - -
**示例：**

```shell
# Example: Execute KNN Query
##################################################
# Request:
curl -X POST \
  http://11.122.49.225:16100/v1/collection/example/query \
  -H 'cache-control: no-cache' \
  -H 'content-type: application/json' \
  -d '{
    "collection_name":"example",
    "debug_mode":true,
    "knn_param":{
        "column_name":"column",
        "topk":10,
        "matrix":"[[1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0], [1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0], [1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0], [1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0]]",
        "batch_count":4,
        "dimension":8,
        "data_type":"DT_VECTOR_FP32",
        "radius":1.5,
        "is_linear":true,
        "extra_params":[
            {
                "key":"customize_param",
                "value":"10"
            },
            {
                "key":"customize_param2",
                "value":"1"
            },
            {
                "key":"customize_param3",
                "value":"str"
            }
        ]
    }
}'

##################################################
# Response:
{
    "status": {
        "code": 0,
        "reason": "Success"
    },
    "results": [
        {
            "documents": []
        },
        {
            "documents": [
                {
                    "score": 0,
                    "primary_key": "0",
                    "forward_column_values": [
                        {
                            "key": "forward",
                            "value": {
                                "int64_value": "1"
                            }
                        },
                        {
                            "key": "forward1",
                            "value": {
                                "float_value": 1.1
                            }
                        },
                        {
                            "key": "forward2",
                            "value": {
                                "bool_value": true
                            }
                        }
                    ]
                },
                {
                    "score": 0,
                    "primary_key": "2",
                    "forward_column_values": [
                        {
                            "key": "forward",
                            "value": {
                                "int64_value": "2"
                            }
                        },
                        {
                            "key": "forward1",
                            "value": {
                                "float_value": 2.2
                            }
                        },
                        {
                            "key": "forward2",
                            "value": {
                                "bool_value": false
                            }
                        }
                    ]
                }
            ]
        },
        {
            "documents": [
                {
                    "score": 0,
                    "primary_key": "0",
                    "forward_column_values": [
                        {
                            "key": "forward",
                            "value": {
                                "int64_value": "1"
                            }
                        },
                        {
                            "key": "forward1",
                            "value": {
                                "float_value": 1.1
                            }
                        },
                        {
                            "key": "forward2",
                            "value": {
                                "bool_value": true
                            }
                        }
                    ]
                },
                {
                    "score": 0,
                    "primary_key": "2",
                    "forward_column_values": [
                        {
                            "key": "forward",
                            "value": {
                                "int64_value": "2"
                            }
                        },
                        {
                            "key": "forward1",
                            "value": {
                                "float_value": 2.2
                            }
                        },
                        {
                            "key": "forward2",
                            "value": {
                                "bool_value": false
                            }
                        }
                    ]
                }
            ]
        },
        {
            "documents": [
                {
                    "score": 0,
                    "primary_key": "0",
                    "forward_column_values": [
                        {
                            "key": "forward",
                            "value": {
                                "int64_value": "1"
                            }
                        },
                        {
                            "key": "forward1",
                            "value": {
                                "float_value": 1.1
                            }
                        },
                        {
                            "key": "forward2",
                            "value": {
                                "bool_value": true
                            }
                        }
                    ]
                },
                {
                    "score": 0,
                    "primary_key": "2",
                    "forward_column_values": [
                        {
                            "key": "forward",
                            "value": {
                                "int64_value": "2"
                            }
                        },
                        {
                            "key": "forward1",
                            "value": {
                                "float_value": 2.2
                            }
                        },
                        {
                            "key": "forward2",
                            "value": {
                                "bool_value": false
                            }
                        }
                    ]
                }
            ]
        }
    ],
    "debug_info": "{\"query\":{\"latency\":205,\"prepare\":51,\"evaluate\":{\"execute\":102,\"latency\":139,\"merge_and_sort\":34},\"validate\":10},\"latency\":277,\"query_id\":11}",
    "latency_us": "585"
}
```

### 3.3 根据主键查询文档

**请求：**

```shell
HTTP 1.1 GET /v1/collection/{name}/doc?key={key}
```

- - -
**请求参数：**

- key: *Uint64，* 指定文档主键

- - -

**返回值：**

- status: *[ProximaBE  执行状态](#41-执行状态),* 查询请求状态
- results: *List[[4.6 文档](#47-文档),...]* 文档
- debug&#95;info: *String,* Json格式的字符串表达

- - -
**示例：**

```shell
# Example: Query document by key
##################################################
# Request:
curl -X GET \
  'http://11.122.49.225:16100/v1/collection/example/doc?key=2' \
  -H 'cache-control: no-cache'

##################################################
# Response:
{
    "status": {
        "code": 0,
        "reason": "Success"
    },
    "document": {
        "score": 0,
        "primary_key": "2",
        "forward_column_values": [
            {
                "key": "forward",
                "value": {
                    "int64_value": "2"
                }
            },
            {
                "key": "forward1",
                "value": {
                    "float_value": 2.2
                }
            },
            {
                "key": "forward2",
                "value": {
                    "bool_value": false
                }
            }
        ]
    },
    "debug_info": ""
}
```

## 4 通用数据类型

### 4.1 执行状态

**Status:**

- code：*Int32,* 服务器指定状态码，0表示正常执行，非零代表执行错误,并设置reason字段为具体错误信息
- reason: *String 可选参数，* 附加错误信息

### 4.2 索引列数据类型

**DataTypes:** *String，* 标识索引列类型，可选类型如下：

- DT&#95;VECTOR&#95;BINARY32： 多维向量类型，每一维数据为32位二进制数据
- DT&#95;VECTOR&#95;BINARY64： 多维向量类型，每一维数据为64位二进制数据
- DT&#95;VECTOR&#95;FP16： 多维向量类型，每一维数据为16位浮点数
- DT&#95;VECTOR&#95;FP32: 多维向量类型，每一维数据为32位浮点数
- DT&#95;VECTOR&#95;INT8: 多维向量类型，每一维数据为8位有符号整数

### 4.3 文档数据

**Row:**

- primary&#95;key: *Uint64,非必须字段* 文档主键
- operation&#95;type: *String,* 字符串标识的操作类型，可选列表如下：
	- OP&#95;INSERT: 将该文档做插入操作
	- OP&#95;UPDATE: primary&#95;key为主键更新该文档
	- OP&#95;DELETE: 删除primary&#95;key为主键的文档
- index&#95;column&#95;values: 向量列数据
	- values:
		- bytes&#95;value: *String,* base64位编码后的向量数据
		- string&#95;value: *String,* json字符串格式的向量数据，与bytes&#95;value二选一即可
- forward&#95;column&#95;values: 正排列数据
	- values: 正排数据列表，通过不同的主键标识数据类型
		- bytes&#95;value: *String,* base64位编码后的二进制数据
		- string&#95;value: *String,* 字符串数据
		- bool&#95;value: *Bool,* 布尔型数据，可选[true, false]
		- int32&#95;value: *Int32,* 32位有符号整数
		- int64&#95;value: *String,* 字符串标识的64位有符号整数
		- uint32&#95;value: *Uint32,* 32位无符号整数
		- uint64&#95;value: *Uint64,* 字符串标识的64位无符号整数
		- float&#95;value: *Float,* 32位浮点数
		- double&#95;value: *Double,* 32位浮点数
- lsn&#95;context: *[4&#95;4 LsnContext](#44-记录上下文),非必须字段,* 文档上下文（用于标识唯一的来源，一般用于数据同步场景）

**示例1：<u> base64编码的向量数据</u>**
```json
{
    "index_column_values":{
        "values":[
            {
                "bytes_value":"P4AAAEAAAABAQAAAQIAAAECgAABAwAAAQOAAAEEAAAA="
            }
        ]
    },
    "forward_column_values":{
        "values":[
            {
                "int64_value":"1"
            },
            {
                "float_value":1.1
            }
        ]
    }
}
```

**示例2：<u> json字符串格式向量数据</u>**
```json
{
    "index_column_values":{
        "values":[
            {
                "string_value":"[1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0]"
            }
        ]
    },
    "forward_column_values":{
        "values":[
            {
                "int64_value":"1"
            },
            {
                "float_value":1.1
            }
        ]
    }
}
```

### 4.4 记录上下文

**LsnContext:**

### 4.5 近邻检索参数

**KnnQueryParam:**

- column&#95;name: *String,* 索引列名称
- topk: *Uint32,* topn最近的邻居个数
- 请求向量列表（以下参数二选一即可）：
	- matrix: *List[List[String]...],* Json格式的向量请求列表，可以发多个
	- features: *String,* 字符串标识的请求向量（需Base64编码），如果是批量请求，需连续编码无间隔字符
- batch&#95;count: *Uint32,* 批量请求的个数
- dimension: *Uint32,* 向量维度
- data&#95;type: *[4.2 DataType](#42-索引列数据类型),* 请参考DataType定义
- radius: *Float,* 搜索半径，不同距离函数下搜索半径取值不同
- is&#95;linear: *Bool,* 线性搜索标志
- extra&#95;params: *Map[String, String],* 附加搜索参数（暂不开放）


### 4.6 文档列表

**Documents:**

- documents: *List[[4.6 文档](#47-文档),...]*

### 4.7 文档

**Document:**

- score: *Float,* 评分
- primary&#95;key: *Uint64,* 字符串格式的64位整数
- forward&#95;column&#95;values: *List[[正排字段](#48-正排字段), ...]*

### 4.8 正排字段

**Property:**

- key: *String,* 属性名称
- value: *[4.3 文档数据Value](#43-文档数据),* 可变属性值

