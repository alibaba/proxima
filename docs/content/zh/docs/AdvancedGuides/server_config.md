---
title: "服务配置"
linkTitle: "服务配置"
weight: 11
draft: false
---



ProximaBE 设计时遵循最简配置原则，最大化降低用户启动成本，用户大部分场景直接使用默认配置文件即可。当然我们也开放了诸如日志、线程、网络端口等配置项，供用户根据自己场景配置。



## 1. 文件目录

ProximaBE 服务部署完之后，典型的文件结构如下:

```
proxima-be/
├── conf
│   └── proxima_be.conf
├── data
│   ├── plants
│   │   ├── data.del
│   │   ├── data.fwd.0
│   │   ├── data.id
│   │   ├── data.lsn
│   │   ├── data.manifest
│   │   └── data.pxa.image_vector.0
│   └── proxima_be.sqlite
└── log
    ├── proxima_be.log.ERROR -> proxima_be.log.ERROR.20210331-144121.10
    ├── proxima_be.log.INFO -> proxima_be.log.INFO.20210331-105941.10
    ├── proxima_be.log.WARNING -> proxima_be.log.WARNING.20210331-144121.10
    ├── proxima_be.log.ERROR.20210331-144121.10
    ├── proxima_be.log.INFO.20210331-105941.10
    ├── proxima_be.log.WARNING.20210331-144121.10
    └── start.log
```

* conf目录，主要是服务静态配置，服务启动时会一次性加载生效
* data目录，主要是保存元数据以及集合数据
* log目录，保存日志数据，默认按2G大小切割



## 2. 配置项说明

配置项主要是指 proxima_be.conf 的结构，我们按主体功能将配置项分成4个部分:

* CommonConfig， 通用配置，包括网络端口、日志等配置项
* QueryConfig，查询配置，包括查询线程数配置项
* IndexConfig，写入配置，包括写入线程管理、限速等配置项
* MetaConfig， 元数据管理配置



proxima_be.conf 一份典型的默认配置如下，下面我们将详细介绍每个配置项的功能

```
common_config {
	grpc_listen_port: 16000
	http_listen_port: 16001
	logger_type: "AppendLogger"
	log_directory: "./log/"
	log_file: "proxima_be.log"
	log_level: 1
}

query_config {
	query_thread_count: 8
}

index_config {
	max_build_qps: 0
	index_directory: "./"
	flush_internal: 300
}

meta_config {
	meta_uri: "sqlite:///proxima_be_meta.sqlite"
}
```



### 2.1 CommonConfig

| 参数名           | 类型   | 默认值           | 必需 | 说明                                                         |
| ---------------- | ------ | ---------------- | ---- | ------------------------------------------------------------ |
| grpc_listen_port | uint32 | 16000            | 否   | grpc协议监听端口                                             |
| http_listen_port | uint32 | 16001            | 否   | http协议监听端口                                             |
| logger_type      | string | "AppendLogger"   | 否   | 目前支持两种Logger<br>AppendLogger--自动增加切割日志<br>SysLogger--打印到系统日志 |
| log_directory    | string | "./log/"         | 否   | 日志目录                                                     |
| log_file         | string | "proxima_be.log" | 否   | 日志名称                                                     |
| log_level        | uint32 | 2                | 否   | 最低打印日志级别 <br>1--DEBUG<br>2--INFO<br>3--WARN<br>4--ERROR<br>5--FATAL |



### 2.2 QueryConfig

| 参数名             | 类型   | 默认值 | 必需 | 说明                                                         |
| ------------------ | ------ | ------ | ---- | ------------------------------------------------------------ |
| query_thread_count | uint32 | 8      | 否   | 查询的线程数据量，这里一般建议配置为<br> 机器核数 ，写入和查询复用同一个线程池 |



### 2.3 IndexConfig

| 参数名          | 类型   | 默认值 | 必需 | 说明                                |
| --------------- | ------ | ------ | ---- | ----------------------------------- |
| max_build_qps   | uint32 | 0      | 否   | 写入限速，默认为0，代表不开限速功能 |
| index_directory | string | "./"   | 否   | 索引目录，默认为当前目录            |
| flush_internal  | uint32 | 300    | 否   | 内存数据定期同步间隔，单位为秒      |



### 2.4 MetaConfig

| 参数名   | 类型   | 默认值      | 必需 | 说明                                 |
| -------- | ------ | ----------- | ---- | ------------------------------------ |
| meta_uri | string | "sqlite://" | 否   | 元数据存储位置，默认为二进制当前目录 |

