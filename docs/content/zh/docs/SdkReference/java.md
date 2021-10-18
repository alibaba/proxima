---
title: "Java SDK"
linkTitle: "Java SDK"
weight: 103
draft: false
---


本文主要介绍用户如何使用 Java SDK，来进行集合管理、文档的增删改查等功能。

## 1. 前提条件

使用 ProximaBE 的 Java SDK，必须满足以下两点：

```
-  JDK 1.8 或者以上版本
-  使用 Apache Maven
```

## 2. 安装 Java SDK

可以通过Apache Maven二方库依赖的方式，将 SDK 下载到自己的项目中。

```java
<dependency>
<groupId>com.alibaba.proxima</groupId>
<artifactId>proxima-be-java-sdk</artifactId>
<version>0.1.2-SNAPSHOT</version>
</dependency>
```


## 3. 客户端创建
ProximaSearchClient 是ProximaBE 对外的所有接口的代理层，用户通过传入连接参数，创建该客户端对象，就可以对 ProximaBE 中的集合进行相关的操作。

### 3.1. 初始化连接参数

**构建类**：ConnectParam.Builder

*首先通过调用 ConnectParam 的 newBuilder 接口，创建出 ConnectParam 的 Builder 对象，然后依次通过Builder对象设置 ProximaBE 的 host 地址，以及 grpc 端口等信息，最终调用 Builder的 build 接口，即可构建出 ConnectParam 对象。*

**Builder 参数说明**：

| 参数   | 说明                              |
| :----- | :-------------------------------- |
| host | ProximaBE对应的服务地址，String类型默认为 localhost |
| port | ProximaBE对应的GRPC服务端口，int类型，默认16000 |
| IdleTimeout | GRPC链接空闲的超时时间，long类型，默认12小时 |

### 3.2. 创建客户端对象

**接口**：ProximaGrpcSearchClient(ConnectParam connectParam)

*通过传入的 connectParam 对象创建 ProximaBE 客户端，目前支持 Grpc 协议，创建成功后返回 ProximaSearchClient 对象，目前 Client 中的接口均为同步接口。*

**参数说明**：

- ConnectParam：表示连接 ProximaBE 所需要的相关连接参数信息

### 3.3. 示例

创建 ProximaSearchClient 对象

```java
import com.alibaba.proxima.se.client.*;

// init connect params
ConnectParam connectParam = ConnectParam.newBuilder()
                .withHost("127.0.0.1")
                .withPort(16000)
                .build();

// create client
ProximaSearchClient client = new ProximaGrpcSearchClient(connectParam);

```

## 4. 集合管理

集合管理主要包括集合的创建、集合的查询和集合的删除等操作。

### 4.1. 创建集合

#### 4.1.1 创建 CollectionConfig

**构造类**：CollectionConfig.Builder

*首先通过调用 CollectionConfig 的 newBuilder 接口，创建出 CollectionConfig 的 Builder 对象，然后通过 Builder 对象设置集合名称、索引列信息、正排列信息等，最终调用 Builder 的 build 接口，即可构建出 CollectionConfig 对象。*

**Builder 参数说明：**

| 参数   | 说明                              |
| :----- | :-------------------------------- |
|  collectionName | 集合的名称，String类型 |
| indexColumnParams | 索引列参数列表，List<IndexColumnParam>类型，IndexColumnParam类型参数见下表 |
| forwardColumnNames | 正排列的名称列表，List<String>类型 |
| maxDocsPerSegment | 分片最大文档数，long类型，默认值为0，表示不限制单片文档数 |
| databaseRepository | 数据库源相关配置，如果是数据库旁路集合，需要配置该选项，目前只支持 mysql 数据库 |

**IndexColumnParam 参数:**

| 参数   | 说明                              |
| :----- | :-------------------------------- |
| columnName | 索引列名称，String 类型|
| indexType | 索引类型，IndexType 类型，目前支持 PROXIMA_GRAPH_INDEX 类型索引|
| dataType | 索引列数据类型，DataType 类型|
| dimension | 索引的维度，int 类型|
| extraParams | 其它的参数，Map类型|

#### 4.1.2 调用创建接口

**接口：** Status **createCollection**(CollectionConfig collectionConfig)

*根据参数 CollectionConfig 对象创建集合，返回值为 Status 对象，表示集合创建成功与否，如果传入的 CollectionConfig 对象非法，会抛出 ProximaBEException 异常。*

**参数说明：**

- collectionConfig：其中包含了集合的基础配置信息，主要有集合名称、索引列信息、正排列名称列表等。

**返回值：**

- Status：表示创建成功与否，其中包含错误码和错误描述

#### 4.1.3 示例

**示例 1**：创建一个带有两个正排列，以及单个向量索引列的集合

```java
public boolean createCollection(ProximaSearchClient client) {
        String collectionName = "collection1";
        int dimension = 8;

        // create collection builder
        CollectionConfig config = CollectionConfig.newBuilder()
                .withCollectionName(collectionName)                            // set collection name
                .withForwardColumnNames(Arrays.asList("forward1", "forward2")) // set forward column names
                .addIndexColumnParam("index1", DataType.VECTOR_FP32, dimension)// add one index column param
                .build();                                                      // build the CollectionConfig

        // call create collection interface
        try {
            Status status = client.createCollection(config);
            if (status.ok()) {
                System.out.println("============== Create collection success. ================");
                return true;
            } else {
                System.out.println("Create collection failed." + status.toString());
                return false;
            }
        } catch (ProximaBEException e) {
            e.printStackTrace();
            return false;
        }
    }
```

**示例 2**：创建一个带有两个正排列、单个向量索引列、以及作为Mysql 数据旁路的集合

```java
public boolean createCollectionWithRepo(ProximaSearchClient client) {
    String collectionName = "collection2";
    int dimension = 8;

    // create database repository
    DatabaseRepository repo = DatabaseRepository.newBuilder()
            .withConnectionUri("mysql://host.com:8080/mysql_database")  // set connection uri
            .withTableName("mysql_table1")                              // set table name
            .withRepositoryName("mysql_repo")                           // set repository name
            .withUser("root")                                           // set user name
            .withPassword("root")                                       // set password
            .build();

    // create collection builder
    CollectionConfig config = CollectionConfig.newBuilder()
            .withCollectionName(collectionName)                            // set collection name
            .withForwardColumnNames(Arrays.asList("forward1", "forward2")) // set forward column name list
            .addIndexColumnParam("index1", DataType.VECTOR_FP32, dimension)// add one index column param
            .withDatabaseRepository(repo)                                  // set database repoistory
            .build();

    // call create collection interface
    try {
        Status status = client.createCollection(config);
        if (status.ok()) {
            System.out.println("============== Create collection success. ================");
            return true;
        } else {
            System.out.println("Create collection failed." + status.toString());
            return false;
        }
    } catch (ProximaBEException e) {
        e.printStackTrace();
        return false;
    }
}
```

### 4.2. 查询集合配置信息
**接口：** DescribeCollectionResponse **describeCollection**(String collectionName)

*根据传入的集合名称，获取当前集合对应的相关配置信息。*

**参数说明**：

- collectionName：集合名称

**返回值：**

- DescribeCollectionResponse：其中包含 Status 以及 CollectionInfo 两部分，Status 描述当前请求成功与否，CollectionInfo 描述集合的配置信息

**示例：**

查询 collection 的配置信息

```java
public boolean describeCollection(ProximaSearchClient client) {
    String collectionName = "collection1";
    // describe collection
    DescribeCollectionResponse descResponse = client.describeCollection(collectionName);
    if (descResponse.ok()) {
        System.out.println("================ Describe collection success. ===================");
        CollectionInfo info = descResponse.getCollectionInfo();
        CollectionConfig collectionConfig = info.getCollectionConfig();
        System.out.println("CollectionName: " + collectionConfig.getCollectionName());
        System.out.println("CollectionStatus: " + info.getCollectionStatus());
        System.out.println("Uuid: " + info.getUuid());
        System.out.println("Forward columns: " + collectionConfig.getForwardColumnNames());
        List<IndexColumnParam> indexColumnParamList = collectionConfig.getIndexColumnParams();
        for (int i = 0; i < indexColumnParamList.size(); ++i) {
            IndexColumnParam indexParam = indexColumnParamList.get(i);
            System.out.println("IndexColumn: " + indexParam.getColumnName());
            System.out.println("IndexType: " + indexParam.getDataType());
            System.out.println("DataType: " + indexParam.getDataType());
            System.out.println("Dimension: " + indexParam.getDimension());
        }
        return true;
    } else {
        System.out.println("Describe collection failed " + descResponse.toString());
        return false;
    }
}
```

### 4.3. 查询集合统计信息

**接口：** StatsCollectionResponse **statsCollection**(String collectionName)

*根据传入的集合名称，获取当前集合对应的相关统计信息。*

**参数说明**：

- collectionName：集合名称

**返回值：**

- StatsCollectionResponse：其中包含 Status 以及 CollectionStats 两部分，Status 描述当前请求成功与否，CollectionStats 描述集合的相关统计信息，包含的信息如下：

| 字段名称   | 说明                              |
| :----- | :-------------------------------- |
| collectionPath | collection 的索引路径 |
| totalDocCount | 总的文档数量 |
| totalSegmentCount | 总的segment数量|
| totalIndexFileCount | 总的索引文件数量|
| totalIndexFileSize | 总的索引文件大小|
| segmentStatsList | 每个 segment 相应的统计信息，包括 segment 的总文档数量、索<br>引文件数量和大小，以及最大/小 doc id等|

**示例 ：**

查询指定集合的统计信息

```java
public boolean statsCollection(ProximaSearchClient client) {
    String collectionName = "collection1";
    // call stats collection interface
    StatsCollectionResponse statsResponse = client.statsCollection(collectionName);
    if (statsResponse.ok()) {
        System.out.println("==================== Stats collection success. ======================");
        CollectionStats stats = statsResponse.getCollectionStats();
        System.out.println("CollectionName: " + stats.getCollectionName());
        System.out.println("TotalDocCount: " + stats.getTotalDocCount());
        System.out.println("TotalSegmentCount: " + stats.getTotalSegmentCount());
        for (int i = 0; i < stats.getSegmentStatsCount(); ++i) {
            System.out.println("Segment: " + i);
            System.out.println("SegmentDocCount: " + stats.getSegmentStats(i).getDocDount());
            System.out.println("SegmentIndexFileCount: " + stats.getSegmentStats(i).getIndexFileCount());
            System.out.println("SegmentMaxDocId: " + stats.getSegmentStats(i).getMaxDocId());
        }
        return true;
    } else {
        System.out.println("Stats collection failed " + statsResponse.getStatus().toString());
        return false;
    }
}
```

### 4.4. 删除集合

**接口：** Status **dropCollection**(String collectionName)

*删除指定集合，集合名由参数 collectionName 指定*

**参数说明**：

- collectionName：删除的集合名称

**返回值**：

- Status：表示删除成功与否，其中包含错误码和错误描述

**示例** ：

删除指定集合

```java
public boolean dropCollection(ProximaSearchClient client) {
    String collectionName = "collection1";
    // call drop collection interface
    Status status = client.dropCollection(collectionName);
    if (status.ok()) {
        System.out.println("==================== Dorp collection success. ========================");
        return true;
    } else {
        System.out.println("Drop collection failed " + status.toString());
        return false;
    }
}
```

### 4.5. 获取集合列表

**接口：** ListCollectionsResponse **listCollections**(ListCondition listCondition)

*根据传入的 ListCondition 条件获取满足条件的集合列表*

**参数说明**：

- listCondition：集合需要满足的条件，目前只包含了一个 Repository 名称字段，仅用在Mysql 同步数据时，如果该字段为空或者 listCondition 为空时，返回所有的集合

**返回值**：

- ListCollectionsResponse：包含 Status 和 CollectionInfo List 两部分，前者描述请求是否成功，后者描述所有集合的配置信息。

***示例 ：***

获取 repository name 为 mysql_repo 的所有集合

```java
public boolean listCollections(ProximaSearchClient client) {
    // build list condition object
    ListCondition listCondition = ListCondition.newBuilder()
            .withRepositoryName("mysql_repo")  // set repository name
            .build();

    // call list collections interface
    ListCollectionsResponse listResponse = client.listCollections(listCondition);
    if (listResponse.ok()) {
        System.out.println("List collections success.");
        for (int i = 0; i < listResponse.getCollectionCount(); ++i) {
            System.out.println("Collection " + i);
            CollectionInfo info = listResponse.getCollection(i);
            CollectionConfig collectionConfig = info.getCollectionConfig();
            System.out.println("CollectionName: " + collectionConfig.getCollectionName());
            System.out.println("CollectionStatus: " + info.getCollectionStatus());
            System.out.println("Uuid: " + info.getUuid());
            System.out.println("Forward columns: " + collectionConfig.getForwardColumnNames());
            List<IndexColumnParam> indexColumnParamList = collectionConfig.getIndexColumnParams();
            for (int j = 0; j < indexColumnParamList.size(); ++j) {
                IndexColumnParam indexParam = indexColumnParamList.get(j);
                System.out.println("IndexColumn: " + indexParam.getColumnName());
                System.out.println("IndexType: " + indexParam.getDataType());
                System.out.println("DataType: " + indexParam.getDataType());
                System.out.println("Dimension: " + indexParam.getDimension());
            }
        }
        return true;
    } else {
        System.out.println("List collections failed " + listResponse.toString());
        return false;
    }
}
```

## 5. 文档管理

文档管理主要包括文档的插入、更新和删除操作，ProximaBE 中将文档的这三种操作，整合为一个通用的 Write 接口，通过一个 WriteRequest 结构来描述相关的操作，WriteRequest 中既可以描述单条记录的操作，也可以描述批量记录的操作。

### 5.1. 创建 WriteRequest

**构造类：WriteRequest.Builder**

*首先通过 WriteRequest 的 newBuilder() 接口创建出 WriteRequest 的 Builder 对象，然后通过 Builder 对象设置相关参数，包括集合名称、正排列和索引列，以及 Rows等信息，最终调用 Builder 的 build 接口创建出 WriteRequest 对象。*

**参数说明**：
| 参数   | 说明                              |
| :----- | :-------------------------------- |
| collectionName |集合名称，String类型|
| forwardColumnList | 正排列的名称列表，List类型，插入和更新时必须要设置，删除时不需要设置 |
| indexColumnMetaList | 索引列的 Meta 列表，插入和更新时必须要设置，删除时不需要设置|
| rows | 请求中包含的文档操作列表，List类型，每一个 row 表示一个文档操作，具体内容见 Rows 参数说明 |
| requestId | 请求的 id，String类型，默认为空 |
| magicNumber | 请求中的魔数，long类型，仅在通过 Mysql 作为数据源时需要设置 |

**Row 参数说明**

| 参数   | 说明                              |
| :----- | :-------------------------------- |
| primaryKey | row的主键，long类型 |
| operationType | row的操作类型，候选值为 INSERT、UPDATE、DELETE三种|
|  indexValues | 对应索引列的值列表，List类型，operationType为DELETE操作时，不需要指定，支持三种格式：<br> <ul><li>向量数组，如[1.0, 2.0, 3.0]</li><li>json数组str，如"[1.0,2.0,3.0]"</li><li>bytes，必须为小端字节序。</li></ul> |
| forwardValues | 对应正排列的值列表，List类型，operationType为DELETE时，不需要指定 |
| lsnContext | mysql 的 lsn 信息，仅当用 mysql 做数据同步时需要|


### 5.2. 执行写入

**接口：** Status **write**(WriteRequest request)

*向 ProximaBE 中写入文档操作请求，执行完成后返回 Status，表示执行成功与否，如果传入的 WriteRequest 请求非法，会抛出 ProximaBEException 异常。*

**参数说明：**

- request：包含某个集合的插入、更新和删除等文档操作

***返回值：***

- Status：表示写入请求的成功与否

### 5.3. 示例

**示例1**：向集合 collection1 中分别插入和更新一条记录

```java
public boolean write1(ProximaSearchClient client) {
    String collectionName = "collection1";
    float[] vectors1 = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    float[] vectors2 = {1.5f, 2.5f, 3.5f, 4.5f, 5.5f, 6.5f, 7.5f, 8.5f};
    int dimension = 8;
    long primaryKey = 123456;
    int forward1Value = 123;
    float forward2Value = 100.123f;

    // build insert row
    WriteRequest.Row insertRow = WriteRequest.Row.newBuilder()
            .withPrimaryKey(primaryKey)                           // set primary key
            .withOperationType(WriteRequest.OperationType.INSERT) // set operation type
            .addIndexValue(vectors1)                        // add one index value
            .addForwardValue(forward1Value)                 // add one forard value
            .addForwardValue(forward2Value)                 // add one forward value
            .build();

    // build update row
    WriteRequest.Row updateRow = WriteRequest.Row.newBuilder()
            .withPrimaryKey(primaryKey)                           // set primary key
            .withOperationType(WriteRequest.OperationType.UPDATE) // set operation type
            .addIndexValue(vectors2)                        // add index value
            .addForwardValue(forward1Value)                 // add forward value
            .addForwardValue(forward2Value)                 // add forward value
            .build();

    // build delete row
    long deleteKey = 10000;
    WriteRequest.Row deleteRow = WriteRequest.Row.newBuilder()
            .withPrimaryKey(deleteKey)                            // set primary key
            .withOperationType(WriteRequest.OperationType.DELETE) // set operation type
            .build();

    // build write request
    WriteRequest writeRequest = WriteRequest.newBuilder()
                .withCollectionName(collectionName)                            // set collection name
                .withForwardColumnList(Arrays.asList("forward1", "forward2"))  // set forward column list
                .addIndexColumnMeta("index1", DataType.VECTOR_FP32, dimension) // add one index column meta, otherwise call withIndexColumnMetaList
                .addRow(insertRow)                                             // add insert row
                .addRow(updateRow)                                             // add update row
                .addRow(deleteRow)                                             // add delete row
                .build();

    try {
        // call write interface
        Status status = client.write(writeRequest);
        if (status.ok()) {
            System.out.println("============== Write success. ===============");
            return true;
        } else {
            System.out.println("Write failed " + status.toString());
            return false;
        }
    } catch (ProximaBEException e) {
        e.printStackTrace();
        return false;
    }*/
}
```
**示例2**：向集合 collection1 中删除一条记录

```java
public boolean write2(ProximaSearchClient client) {
    String collectionName = "collection1";
    // build delete row
    long deleteKey = 123456;
    WriteRequest.Row deleteRow = WriteRequest.Row.newBuilder()
            .withPrimaryKey(deleteKey)                            // set primary key
            .withOperationType(WriteRequest.OperationType.DELETE) // set operation type
            .build();

    // build write request
    WriteRequest writeRequest = WriteRequest.newBuilder()
                .withCollectionName(collectionName)                            // set collection name
                .addRow(deleteRow)                                             // add delete row
                .build();

    try {
        // call write interface
        Status status = client.write(writeRequest);
        if (status.ok()) {
            System.out.println("============== Write success. ===============");
            return true;
        } else {
            System.out.println("Write failed " + status.toString());
            return false;
        }
    } catch (ProximaBEException e) {
        e.printStackTrace();
        return false;
    }*/
}
```



## 6. 文档查询

文档查询目前主要包括两种类型，一种是根据向量来进行 knn 的查询，另一种是直接根据文档的主键来查询相应的文档。

### 6.1. 向量查询

#### 6.1.1 创建 QueryRequest

**构造类：QueryRequest.Builder**

*通过 QueryRequest 的 newBuilder 接口创建出 QueryRequest 的 Builder 对象，然后通过 Builder 对象设置相关查询参数，主要包括集合名称和 KnnQueryParam等参数，最终调用 Builder 的 build 接口构建出 QueryRequest 对象。*

**参数说明：**
| 参数   | 说明                              |
| :----- | :-------------------------------- |
| collectionName | 要查询的集合名称，String类型|
| knnQueryParam | knn 查询的参数，KnnQueryParam类型，具体内容见下表|
| debugMode | 是否开启调试模式，boolean类型，默认为false|

**KnnQueryParam 参数**

| 参数   | 说明                              |
| :----- | :-------------------------------- |
| columnName | 要查询的索引列名，String类型|
| topk | 查询的结果数量，int类型，默认为100|
| features | 查询的特征，可以是单个或者多个向量，支持三种类型：<br /> <ul><li>小端字节序的bytes</li><li>json数组字符串，支持嵌套json数组。如2个batch的2维向量，可以写为"[[1.0,2.0],[3.0,4.0]]"</li><li>向量列表，如[[5.1, 3.5, 1.4, 0.2], [5.5, 2.3, 4.0, 1.3]]</li></ul> |
| dataType | 查询向量的数据类型，向量列表类型的特征支持自动推导，其它类型的必须设置 |
| dimension | 查询向量的维度，向量列表类型的特征支持自动推导，其它类型的必须设置|
| batchCount | 查询的batch数量，向量列表类型的特征支持自动推导，其它类型的必须设置|
| radius | 过滤结果的分数阈值，仅当分数小于该阈值的结果会返回，float类型，默认值为0，表示不过滤 |
| isLinear | 是否使用暴力检索，boolean类型，默认false |

#### 6.1.2 执行查询

**接口：** QueryResponse **query**(QueryRequest request)

*通过调用上述接口，向 ProximaBE 发起查询请求，返回 QueryResponse，当传入的 QueryRequest 非法时，会抛出 ProximaBEException 异常。*

**参数说明：**

- request：表示查询请求，包含查询的集合以及查询的向量等信息

**返回值：**

- QueryResponse：表示查询的返回结果，包含查询的状态信息，以及查询的文档结果。

#### 6.1.3. 示例

根据某个向量查询最相似的文档

```java
public boolean query(ProximaSearchClient client) {
    String collectionName = "collection1";
    float[] features = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    //  when batch querying, features can be following:
    //  float[][] features = {
    //          {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f},
    //          {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f}
    //  };

    // build query request
    QueryRequest queryRequest = QueryRequest.newBuilder()
            .withCollectionName(collectionName)              // set collection name
            .withKnnQueryParam(                              // set knn query param
                    QueryRequest.KnnQueryParam.newBuilder()  // new KnnQueryParam builder
                            .withColumnName("index1")        // set index column name
                            .withTopk(10)                    // set topk
                            .withFeatures(features)          // set query features
                            .build())                        // build knnQueryParam object
            .build();                                        // build query request

    QueryResponse queryResponse = null;
    try {
        // call query interface
        queryResponse = client.query(queryRequest);
    } catch (ProximaBEException e) {
        e.printStackTrace();
        return false;
    }
    if (queryResponse.ok()) {
        System.out.println("================ Query success =====================");
        for (int i = 0; i < queryResponse.getQueryResultCount(); ++i) {
            QueryResult queryResult = queryResponse.getQueryResult(i);
            for (int d = 0; d < queryResult.getDocumentCount(); ++d) {
                Document document = queryResult.getDocument(d);
                System.out.println("Doc key: " + document.getPrimaryKey());
                System.out.println("Doc score: " + document.getScore());

                Set<String> forwardKeys = document.getForwardKeySet();
                for (String forwardKey : forwardKeys) {
                    System.out.println("Doc forward: " + forwardKey + " -> " +
                                                          document.getForwardValue(forwardKey).getStringValue());
                }
            }
        }
        return true;
    } else {
        System.out.println("Query failed: " + queryResponse.getStatus().toString());
        return false;
    }
}
```

### 6.2. 主键查询

#### 6.2.1 创建 GetDocumentRequest

**构造类：GetDocumentRequest.Builder**

*通过 GetDocumentRequest 的 newBuilder 接口，创建出 GetDocumentRequest 的 Builder 对象，然后通过 Builder 对象设置相关查询参数，主要包括集合名称和 主键等参数，最终调用 Builder 的 build 接口构建出 GetDocumentRequest 对象。*

**Builder 参数说明：**

| 参数   | 说明                              |
| :----- | :-------------------------------- |
| collectionName | 要查询的集合名称，String类型 |
| primaryKey | 要查询的文档的主键，long类型 |
| debugMode | 是否开启调试模式， boolean类型，默认为false |

#### 6.2.2 执行查询

**接口：** GetDocumentResponse **getDocumentByKey**(GetDocumentRequest request)

*通过调用上述接口，向 ProximaBE 发起查询请求，返回 GetDocumentResponse 对象，当传入的 GetDocumentRequest 非法时，会抛出 ProximaBEException 异常。*

**参数说明**：

- request：表示查询指定文档请求

**返回值**：

- GetDocumentResponse：表示查询的返回结果，包含查询的状态信息，以及查询文档的结果。

#### 6.2.3 示例

```java
public boolean getDocument(ProximaSearchClient client) {
    String collectionName = "collection1";
    // build get document request
    GetDocumentRequest getRequest = GetDocumentRequest.newBuilder()
            .withCollectionName(collectionName)    // set collection name
            .withPrimaryKey(123456)                // set primary key
            .withDebugMode(true)                   // set debug mode, defalut false
            .build();                              // build GetDocumentRequest object
    GetDocumentResponse getResponse = null;
    try {
        // call get document by key interface
        getResponse = client.getDocumentByKey(getRequest);
    } catch (ProximaBEException e) {
        e.printStackTrace();
        return false;
    }
    if (getResponse.ok()) {
        System.out.println("======================== Get Document success ======================== ");
        Document document = getResponse.getDocument();
        System.out.println("PrimaryKey: " + document.getPrimaryKey());
        System.out.println("Score: " + document.getScore());
        Set<String> forwardKeys = document.getForwardKeySet();
        for (String forwardKey : forwardKeys) {
            System.out.println("Doc forward: " + forwardKey + " -> " +
                   document.getForwardValue(forwardKey).getStringValue());
        }
        System.out.println("DebugInfo: " + getResponse.getDebugInfo());
        return true;
    } else {
        System.out.println("Get document failed " + getResponse.getStatus().toString());
        return false;
    }
}
```
