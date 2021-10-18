---
title: "Golang SDK"
linkTitle: "Golang SDK"
weight: 104
draft: false
---


用户可以使用Golang SDK进行集合管理、文档的增删该查等功能
## 1. BE客户端创建
ProximaSearchClient 是BE的客户端接口对象，用户通过该实例对象与BE进行交互，完成的管控功能
### 1.1. 创建客户端
**<u>NewProximaSearchClient(ConnectionProtocol, &#42;Address) (ProximaSearchClient, error)</u>**

*创建BE客户端，创建成功后返回ProximaSearchClient对象, 否则返回error。*

**参数说明：**
- **<font color=#2f3f4f>ConnectionProtocol: </font>** *<font color=#2f3f4f>与BE交互的协议类型，目前支持GRPC、HTTP两种协议，其中GRPC在性能以及资源占用上有明显的优势。</font>*
- **<font color=#2f3f4f>&#42;Address：</font>** *<font color=#2f3f4f>该参数指定BE的网络连接信息，主要包含BE的域名，以及端口号,详细定义如下：</font>*


```go
// Address reference to entrypoint of ProximaBE
type Address struct {
	// Host IP address or host name
	Host string `json:"host:omitempty"`
	// Port
	Port uint32 `json:"port"`
}

```

**返回值：**
- **<font color=#2f3f4f>ProximaSearchClient: </font>** *<font color=#2f3f4f>BE客户端对象</font>*
- **<font color=#2f3f4f>error: </font>** *<font color=#2f3f4f>执行错误时返回非nil</font>*

**<font color=#3f4f5f>特殊说明：</font>**
* <font color=#3f4f5f size=2>SDK中已封装了与BE通信的协议类型，如想进一步了解协议内容，请参考[Restful API]({{< ref "../ApiReference/http.md" >}})章节</font>


***示例：*** <u>创建BE客户端</u>
```go
package main

import (
	"log"
	"proxima-be-sdk-go/proxima/se/sdk"
)

func main() {
	// Create client of BE with GRPC protocol and connect to localhost:16000
	client, err := sdk.NewProximaSearchClient(sdk.GrpcProtocol, &sdk.Address{
		Host: "localhost",
		Port: 16000,
	})
	if err != nil {
		log.Fatal("Can't create ProximaClient instance.", err)
	}
}
```


## 2. 集合管理

### 2.1. 集合创建

**<u>CreateCollection(config &#42;CollectionConfig) error</u>**

*根据参数创建集合，创建成功后返回nil, 否则返回error。*

**参数说明：**
- **<font color=#2f3f4f>config: </font>** *<font color=#2f3f4f><u>CollectionConfig</u>中指定了集合的配置信息，包含不仅限于集合名、分片配置、正排列名、索引列信息等，当BE作为Mysql的从属库时，需指定Mysql Repo的相关配置，详情如示例。</font>*

**返回值：**
- **<font color=#2f3f4f>error: </font>** *<font color=#2f3f4f>正常执行返回nil, 否则返回有效error对象</font>*

***示例 1.1：*** <u>创建一个带有两个正排列，以及两个向量索引列的集合</u>
```go
fun createCollctionDemo(client *ProximaSearchClient) {
	// Create collection with no attached repository
	config := &sdk.CollectionConfig{
		CollectionName:    "example",
		MaxDocsPerSegment: 0, // 0 means unlimited, which is equal max value of system
		ForwardColumns:    []string{"forward", "forward1"},
		Columns: []sdk.ColumnIndex{{
			Name:        "column",
			IndexType:   0,                                     // 0 means default index, which is sdk.ProximaGraphIndex
			DataType:    0,                                     // 0 means default, which is sdk.VectorFP32
			Dimension:   8,                                     // Required field, no default value, 0 is not legal argument
			ExtraParams: map[string]string{"ef_search": "200"}, // Advanced params
		}, {
			Name:        "column1",
			IndexType:   sdk.ProximaGraphIndex,
			DataType:    sdk.VectorFP16, // Index type is fp16, query could be fp32 for lack of language types
			Dimension:   128,
			ExtraParams: map[string]string{},
		},
		},
		Repository: nil, // No repository attached
	}
	if err = client.CreateCollection(config); err != nil {
		log.Fatal("Can't create collection.", err)
	}
	log.Print("Create collection succeed.")
}
```


***示例 1.2：*** <u>创建一个带有两个正排列、单个向量索引列、以及Mysql Repo的集合</u>
```go
fun createCollctionWithRepoDemo(client *ProximaSearchClient) {
	// Create collection with attached repository
	config := &sdk.CollectionConfig{
		CollectionName:    "example_with_repo",
		MaxDocsPerSegment: 0, // 0 means unlimited, which is equal max value of system
		ForwardColumns:    []string{"forward", "forward1"},
		Columns: []sdk.ColumnIndex{{
			Name:        "column1",
			IndexType:   0,                                     // 0 means default index, which is sdk.ProximaGraphIndex
			DataType:    0,                                     // 0 means default, which is sdk.VectorFP32
			Dimension:   512,                                   // Required field, no default value, 0 is not legal argument
			ExtraParams: map[string]string{"ef_search": "200"}, // Advanced params
		},
		},
		Repository: &sdk.DatabaseRepository{
			Repository: sdk.Repository{
				Name: "mysql_repo",
				Type: sdk.Database,
			},
			Connection: "mysql://host.com:8080/mysql_database", // JDBC connection uri
			TableName:  "table_name",                           // Table name
			User:       "root",                                 // User name
			Password:   "root",                                 // Password
		},
	}

	if err = client.CreateCollection(config); err != nil {
		log.Fatal("Can't create collection.", err)
	}
	log.Print("Create collection with attached mysql repository succeed.")
}
```

### 2.2. 获取集合配置信息
**<u>DescribeCollection(name string) (&#42;CollectionInfo, error)</u>**

*获取参数name指定的集合配置信息*

**参数说明：**
- **<font color=#2f3f4f>name: </font>** *<font color=#2f3f4f>string指定集合名称</font>*

**返回值：**
- **<font color=#2f3f4f>CollectionInfo: </font>** *<font color=#2f3f4f>指向CollectionInfo的指针，成功返回时该参数非nil, 失败时通过error参数判定失败原因</font>*
- **<font color=#2f3f4f>error: </font>** *<font color=#2f3f4f>失败返回非nil</font>*


***示例 2.1：*** <u>获取集合配置信息</u>
```go
fun describeCollectionDemo(client *ProximaSearchClient) {
	// Retrieve collection named by 'example_with_repo'
	info, err = client.DescribeCollection("example_with_repo")
	if err != nil {
		log.Fatal("Lost collection named by 'example_with_repo', which created before.", err)
	}
	log.Printf("Collection(With Repository): %+v\n", info)

	// Delete collection
	if err = client.DropCollection("example_with_repo"); err != nil {
		log.Fatal("Failed to drop collection,", err)
	}
	log.Print("Drop collection succeed.")
}
```

### 2.3. 删除指定集合
**<u>DropCollection(name string) error</u>**

*删除指定集合，集合名由参数name指定*

**参数说明：**
- **<font color=#2f3f4f>name: </font>** *<font color=#2f3f4f>string指定集合名称</font>*

**返回值：**
- **<font color=#2f3f4f>error: </font>** *<font color=#2f3f4f>失败返回非nil</font>*


***示例 3.1：*** <u>删除集合</u>
```go
fun dropCollectionDemo(client *ProximaSearchClient) {
	client.DropCollection("example_with_repo")
	if err = client.CreateCollection(config); err != nil {
		log.Fatal("Can't create collection.", err)
	}
	log.Print("Create collection with attached mysql repository succeed.")
}
```

### 2.4. 获取集合统计信息
**<u>StatCollection(string) (&#42;CollectionStat, error)</u>**

*获取参数指定的集合统计信息，执行成功返回指向CollectionStat的有效指针，失败返回error*

**参数说明：**
- **<font color=#2f3f4f>string: </font>** *<font color=#2f3f4f>指定集合名称</font>*

**返回值：**
- **<font color=#2f3f4f>&#42;CollectionStat: </font>** *<font color=#2f3f4f>Collection的统计信息</font>*
- **<font color=#2f3f4f>error: </font>** *<font color=#2f3f4f>失败返回非nil</font>*


***示例 4.1：*** <u>获取集合统计信息</u>
```go
fun statCollectionDemo(client *ProximaSearchClient) {
	stat, err := client.StatCollection("example")
	if err != nil {
		log.Fatal("Stat collection failed.", err)
	}
	log.Printf("Collection Stat: %+v", stat)
}
```

### 2.5. 获取集合列表
**<u>ListCollections(filters ...ListCollectionFilter) ([]&#42;CollectionInfo, error)</u>**

*获取符合条件的集合列表，执行成功返回CollectionInfo的列表，失败返回error*

**参数说明：**
- **<font color=#2f3f4f>ListCollectionFilter: </font>** *<font color=#2f3f4f>过滤器对象，过滤器可采用生成器自动生成，可选的生成器如下：</font>*
	**<font color=#2f3f4f>- ByRepo(repo string)： </font>** *<font color=#2f3f4f>包含指定repo名称的Collection</font>*

**返回值：**
- **<font color=#2f3f4f>[]&#42;CollectionInfo: </font>** *<font color=#2f3f4f>CollectionInfo列表</font>*
- **<font color=#2f3f4f>error: </font>** *<font color=#2f3f4f>失败返回非nil</font>*


***示例 5.1：*** <u>获取集合列表</u>
```go
fun listCollectionsDemo(client *ProximaSearchClient) {
	// List all collections
	collections, err := client.ListCollections()
	if err != nil {
		log.Fatal("Can't retrieve collections from Proxima Server.", err)
	}
	log.Printf("Collections (%d): \n", len(collections))
	for _, collection := range collections {
		log.Printf("%+v\n", collection)
	}

	// List all collections by Repo
	collections, err = client.ListCollections(sdk.ByRepo("repo"))
	if err != nil {
		log.Fatal("Can't retrieve collections from Proxima Server.", err)
	}
	log.Printf("Collections (%d): \n", len(collections))
	for _, collection := range collections {
		log.Printf("%+v\n", collection)
	}
}
```

## 3. 文档管理
文档管理整合为一个通用接口Write, 通过接口参数描述所需的操作，接口定义如下：

**<u>Write(&#42;WriteRequest) error</u>**

*向BE中批量写入文档操作请求，执行成功后返回nil, 否则返回值error中描述了错误信息*

**参数说明：**
- **<font color=#2f3f4f>WriteRequest: </font>** *<font color=#2f3f4f>写入请求内容</font>*

```go
// Row record
type Row struct {
	// Primary key of row record
	PrimaryKey uint64
	// Operation type
	OperationType
	// Index column value list
	IndexColumnValues []interface{}
	// Forward column value list
	ForwardColumnValues []interface{}
	// Log Sequence Number context
	*LsnContext
}

// RowMeta meta for row records
type RowMeta struct {
	// Index column name list
	IndexColumnNames []string
	// Index column name list
	ForwardColumnNames []string
}

// WriteRequest object, the parameter of ProximaBEClient.Write method
type WriteRequest struct {
	// Name of collection
	CollectionName string
	// Meta header
	Meta RowMeta
	// Row record list
	Rows []Row
	// Request ID, Optional
	RequestID string
	// Magic number, Optional
	MagicNumber uint64
}
```
Write API中根据传入参数的 Row.OperationType 判定当前数据是插入、删除、更新中的某种操作类型，可选的OperationType列表如下：

- **Insert: ** 新增文档
- **Update：** 更新文档
- **Delete：** 删除文档

**返回值：**
- **<font color=#2f3f4f>error: </font>** *<font color=#2f3f4f>失败返回非nil</font>*

### 3.1. 文档插入
指定Row.OperationType为OPInsert可完成文档的插入操作

***示例 1.1：*** <u>文档插入</u>
```go
fun insertDocumentDemo(client *ProximaSearchClient) {
	rows := &sdk.WriteRequest{
		CollectionName: "example",
		Meta: sdk.RowMeta{
			IndexColumnNames:   []string{"column"},
			ForwardColumnNames: []string{"forward", "forward1"},
		},
		Rows: []sdk.Row{
			{
				PrimaryKey:          0,
				OperationType:       sdk.Insert,
				ForwardColumnValues: []interface{}{1, float32(1.1)},
				IndexColumnValues: []interface{}{ // Data type should same with index created before
					[]float32{1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0},
				},
				LsnContext: nil, // With empty context
			},
		},
		RequestID:   "", // Optional field
		MagicNumber: 0,  // Optional field
	}
	// Write rows to collection
	err = client.Write(rows)
	if err != nil {
		log.Fatal("Insert data to collection failed.", err)
	}
}
```

### 3.2. 文档更新
指定Row.OperationType为OPUpdate可完成文档的更新操作

***示例 2.1：***<u>更新文档</u>
```go
fun updateDocumentDemo(client *ProximaSearchClient) {
	rows := &sdk.WriteRequest{
		CollectionName: "example",
		Meta: sdk.RowMeta{
			IndexColumnNames:   []string{"column"},
			ForwardColumnNames: []string{"forward", "forward1"},
		},
		Rows: []sdk.Row{
			{
				PrimaryKey:          0,
				OperationType:       sdk.Update,
				ForwardColumnValues: []interface{}{2, float32(2.2)},
				IndexColumnValues: []interface{}{ // Data type should same with index created before
					[]float32{21.0, 22.0, 23.0, 24.0, 25.0, 26.0, 27.0, 28.0},
				},
				LsnContext: &sdk.LsnContext{ // With valid context
					LSN:     0,
					Context: "write context hear",
				},
			}},
		RequestID:   "", // Optional field
		MagicNumber: 0,  // Optional field
	}
	// Write rows to collection
	err = client.Write(rows)
	if err != nil {
		log.Fatal("Insert data to collection failed.", err)
	}
}
```

### 3.3. 文档删除
指定Row.OperationType为OPDelete可完成文档的删除操作

***示例 3.1：***<u>删除文档</u>
```go
fun deleteDocumentDemo(client *ProximaSearchClient) {
	rows := &sdk.WriteRequest{
		CollectionName: "example",
		Meta: sdk.RowMeta{
			IndexColumnNames:   []string{"column"},
			ForwardColumnNames: []string{"forward", "forward1"},
		},
		Rows: []sdk.Row{
			{
				PrimaryKey:          0,
				OperationType:       sdk.Delete,
			},
		},
		RequestID:   "", // Optional field
		MagicNumber: 0,  // Optional field
	}
	// Write rows to collection
	err = client.Write(rows)
	if err != nil {
		log.Fatal("Insert data to collection failed.", err)
	}
}
```

### 3.4. 批量文档操作
请求中包含多行数据，并指定OperationType完成批量文档操作

***示例 4.1：***<u>批量文档操作</u>
```go
fun batchDocumentOperationDemo(client *ProximaSearchClient) {
	rows := &sdk.WriteRequest{
		CollectionName: "example",
		Meta: sdk.RowMeta{
			IndexColumnNames:   []string{"column"},
			ForwardColumnNames: []string{"forward", "forward1"},
		},
		Rows: []sdk.Row{
			{
				PrimaryKey:          0,
				OperationType:       sdk.Insert,
				ForwardColumnValues: []interface{}{1, float32(1.1)},
				IndexColumnValues: []interface{}{ // Data type should same with index created before
					[]float32{1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0},
				},
				LsnContext: nil, // With empty context
			}, {
				PrimaryKey:          2,
				OperationType:       sdk.Insert,
				ForwardColumnValues: []interface{}{2, float32(2.2)},
				IndexColumnValues: []interface{}{ // Data type should same with index created before
					[]float32{21.0, 22.0, 23.0, 24.0, 25.0, 26.0, 27.0, 28.0},
				},
				LsnContext: &sdk.LsnContext{ // With valid context
					LSN:     0,
					Context: "write context hear",
				},
			}},
		RequestID:   "", // Optional field
		MagicNumber: 0,  // Optional field
	}
	// Write rows to collection
	err = client.Write(rows)
	if err != nil {
		log.Fatal("Insert data to collection failed.", err)
	}
}
```

## 4. 文档查询

### 4.1. 向量查询
**<u>Query(collection string, column string, features interface{}, opts ...QueryOption) (&#42;QueryResponse, error)</u>**

*单次、批量向BE发起向量查询请求，执行成功返回QueryResponse的有效指针，失败返回error*

**参数说明：**
- **<font color=#2f3f4f>collection: </font>** *<font color=#2f3f4f>string指定集合名称</font>*
- **<font color=#2f3f4f>column: </font>** *<font color=#2f3f4f>string指定列名</font>*
- **<font color=#2f3f4f>features: </font>** *<font color=#2f3f4f>指定查询向量，该参数可以是切片、数组、或者二维矩阵（每行一个向量），向量中的原始数据类型支持int8/uint32/uint64/float32中的一种（由于语言限制，如果索引创建时数据类型配置为DTVectorFP16，请使用float32类型与BE交互，服务器端会自动进行类型转换）</font>*
- **<font color=#2f3f4f>opts: </font>** *<font color=#2f3f4f>QueryOption可选参数列表，可选的参数如下：</font>*
	- **<font color=#2f3f4f>WithTopK(int): </font>** *<font color=#2f3f4f>最近的邻居个数</font>*
	- **<font color=#2f3f4f>WithRadius(float32): </font>** *<font color=#2f3f4f>搜索半径，可选区间为0~1的浮点数，越大召回效果上升、性能下降，越小召回效果下降、性能上升。当请求中同时打开暴力匹配选项，则此参数无效。</font>*
	- **<font color=#2f3f4f>WithLinearSearch(): </font>** *<font color=#2f3f4f>暴力匹配选项，返回的为绝对topk结果（性能最低）</font>*
	- **<font color=#2f3f4f>WithDebugMode(): </font>** *<font color=#2f3f4f>调试模式，将详细搜索的各阶段统计数据返回（不建议生产环境中打开）</font>*
	- **<font color=#2f3f4f>WithParam(string, interface{}): </font>** *<font color=#2f3f4f>高级检索参数，可选配置项待发布</font>*

**返回值：**
- **<font color=#2f3f4f>&#42;QueryResponse: </font>** *<font color=#2f3f4f>最近邻的documents结果</font>*
- **<font color=#2f3f4f>error: </font>** *<font color=#2f3f4f>失败返回非nil</font>*


***示例 1.1：*** <u>单次向量查询</u>
```go
fun queryDemo(client *ProximaSearchClient) {
	// Query one vector
	resp, err := client.Query("example", "column", []float32{1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0}, sdk.WithTopK(10))
	if err != nil {
		log.Fatal("Query failed.", err)
	}
	log.Printf("Response: %+v\n", resp)
}
```

***示例 1.2：*** <u>批量向量查询</u>
```go
fun batchQueryDemo(client *ProximaSearchClient) {
	// Query with matrix
	resp, err = client.Query("example", "column",
		[][]float32{{1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0},
			{1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0},
			{1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0},
			{1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0},
		},
		sdk.WithTopK(10),
		sdk.WithDebugMode(),    // Enable debug mode, do not recommend on product environments
		sdk.WithRadius(1.5),    // Search radius, no effect if sdk.WithLinearSearch enabled
		sdk.WithLinearSearch(), // Enable linear search
		sdk.WithParam("customize_param", 10),
		sdk.WithParam("customize_param2", 1.0),
		sdk.WithParam("customize_param3", "str"))
	if err != nil {
		log.Fatal("Query failed.", err)
	}
	log.Printf("Response: %+v\n", resp)
}
```

### 4.2. 主键查询
**<u>GetDocumentByKey(collection string, primaryKey uint64) (&#42;Document, error)</u>**

*通过文档主键获取文档，执行成功返回Document的有效指针，失败返回error*

**参数说明：**
- **<font color=#2f3f4f>collection: </font>** *<font color=#2f3f4f>集合名称</font>*
- **<font color=#2f3f4f>primaryKey: </font>** *<font color=#2f3f4f>文档主键</font>*

**返回值：**
- **<font color=#2f3f4f>&#42;Document: </font>** *<font color=#2f3f4f>文档对象</font>*
- **<font color=#2f3f4f>error: </font>** *<font color=#2f3f4f>失败返回非nil</font>*


***示例 2.1：*** <u>根据文档主键获取文档内容</u>
```go
fun queryDocByPKDemo(client *ProximaSearchClient) {
	// Retrieve document by primary key
	doc, err := client.GetDocumentByKey("example", 0)
	if err != nil {
		log.Fatal("Failed to retrieve document from server.", err)
	}
	log.Printf("Document: %+v\n", doc)

}
```