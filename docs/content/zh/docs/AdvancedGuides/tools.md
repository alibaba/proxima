
---
title: "常用工具"
linkTitle: "常用工具"
weight: 16
draft: false
---


在 Proxima BE 发布的镜像中，附带了几个常用工具，主要是方便客户管理 collection 和文档。



## 1. 如何获取

相关工具位于镜像 ghcr.io/proxima/proxima-be 的目录 /var/lib/proxima-be/bin/

## 2. admin_client

### 2.1 使用方法

```s
$ admin_client  -h
Usage:
 admin_client <args>

Args:
 --command      Command type: create | drop
 --host         The host of proxima be, http port
 --collection   Specify collection name
 --schema       Specify collection schema format
 --help, -h     Display help info
 --version, -v  Dipslay version info
```



### 2.2 创建Collection

```s
$ admin_client --command create --host 127.0.0.1:16001 --collection test_collection \
    --schema '{"collection_name":"test_collection", "index_column_params":[{"column_name":"test_column", 
    "index_type": "IT_PROXIMA_GRAPH_INDEX", "data_type":"DT_VECTOR_FP32", "dimension":8}]}'
```



### 2.3 删除Collection

```s
$ admin_client --command drop --host 127.0.0.1:16001 --collection test_collection
```



## 3. bench_client

### 3.1 使用方法

```s
$ bench_client -h
Usage:
 bench_client <args>

Args:
 --command        Command type: search|insert|delete|update
 --host           The host of proxima be
 --collection     Specify collection name
 --column         Specify column name
 --file           Read input data from file
 --protocol       Protocol http or grpc
 --concurrency    Send concurrency (default 10)
 --topk           Topk results (default 10)
 --perf           Output perf result (default false)
 --help, -h       Display help info
 --version, -v    Display version info
```



### 3.2 插入数据

```s
$ bench_client --command insert --host 127.0.0.1:16000 --collection test_collection --column test_column --file 
data.txt
```



数据格式支持明文和二进制两种，key与向量之间用";"分隔，多维向量采用空间分割，样例数据如下:

```
0;-0.009256 -0.079674 -0.070349 0.007072 -0.064061 -0.010632 0.083429 -0.074821
1;-0.061519 -0.001263 -0.016528 0.031539 0.041385 -0.017736 -0.005704 0.129443
2;-0.039616 -0.063191 0.057591 -0.090278 -0.007452 -0.035939 -0.021892 -0.037860
3;0.042097 0.050037 0.055060 0.150511 -0.052841 -0.005502 -0.018618 0.054607
```



### 3.3 查询数据

query数据格式同上述的插入数据格式相同。

```s
$ bench_client --command search --host 127.0.0.1:16000 --collection test_collection --column test_column --file 
query.txt
```



### 3.4 删除数据

data数据格式同上述的插入数据格式相同。

```s
$ bench_client --command delete --host 127.0.0.1:16000 --collection test_collection --column test_column --file 
data.txt
```

