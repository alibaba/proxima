# Proxima Bilin Engine

## 背景介绍

随着 AI 技术的广泛应用，以及数据规模的不断增长，对非结构化数据处理的需求也日益增多。向量检索也逐渐成了 AI 技术链路中不可或缺的一环，同时也是对传统搜索技术的补充。

Proxima 是阿里巴巴达摩院系统 AI 实验室自研的向量检索内核。目前，其核心能力广泛应用于阿里巴巴和蚂蚁集团内众多业务，如淘宝搜索和推荐、蚂蚁人脸支付、优酷视频搜索、阿里妈妈广告检索等。同时，Proxima
还深度集成在各式各类的大数据和数据库产品中，如阿里云 Hologres、搜索引擎 Elastic Search 和 ZSearch、离线引擎 MaxCompute (ODPS) 等，为其提供向量检索的能力。

Proxima BE，全称 Proxima Bilin Engine，是 Proxima 团队开发的服务化引擎，实现了对大数据的高性能相似性搜索。支持 RESTful HTTP 接口访问，同时也支持多种语言的 SDK 以 GRPC
协议访问。

## 核心能力

<p align="center">
<img src="docs/static/resources/images/main.png" width="70%" height="70%">
</p>
<br>

Proxima BE 的主要核心能力有以下几点:

* **支持单机超大规模索引**：基于底层向量索引的工程和检索算法优化，使得有限成本下，实现了高效率的检索方法，并支持磁盘索引，单片索引可达几十亿的规模。

* **支持多数据源全量和增量同步**：通过 Mysql Repository 等组件，可将 mysql 等数据源中的数据，实时同步至索引服务，提供查询能力，简化数据处理流程。

* **支持向量索引实时增删改查**：基于全新 CRUD 图索引，支持在线大规模向量索引的从 0 到 1 的流式写入，并实现了索引即时增删改查，避免索引需定期重建。

* **支持正排数据查询**：支持在查询时，可展示文档的所有结构化字段。同时后期将基于此功能，进一步扩展出与文本与向量联合检索等功能。

## 如何构建

环境要求：

* Linux or MacOS
* gcc >= 4.9
* cmake >= 3.14

```shell
git clone https://github.com/alibaba/proximabilin.git

git submodule update --init

mkdir build && cd build

# Intel haswell 架构下 Debug 方式编译
#cmake -DCMAKE_BUILD_TYPE=Debug -DENABLE_NEHALEM=ON ..

# Intel haswell 架构下 Release 方式编译
cmake -DCMAKE_BUILD_TYPE=Release -DENABLE_HASWELL=ON ..

make all
```


## 获取 Docker 镜像

| 平台 | 地址 |
| -------- | ------ |
| Linux X86_64 | ghcr.io/alibaba/proxima-be |

## 快速开始

* [快速入门]()
* [使用示例]()

## 使用手册

* [进阶指南]()
* [API 手册]()
* [SDK 手册]()
* [常见问题]()

## 案列展示

## License

[Apache License 2.0](http://www.apache.org/licenses/LICENSE-2.0)

## 如何交流

* 邮箱
* 钉钉群

## 声明

Proxima BE 依赖了如下项目:

* [brpc](https://github.com/apache/incubator-brpc)
* [protobuf](https://github.com/protocolbuffers/protobuf.git)
* [sqlite](https://github.com/sqlite/sqlite)
* [sparsehash](https://github.com/sparsehash/sparsehash)
* [mysql](https://github.com/mysql)
