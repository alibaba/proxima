---
title: "C++ SDK"
linkTitle: "C++ SDK"
weight: 102
draft: false
---


ProximaBE的C++ SDK主要是针对用户上游C/C++模块的集成，其具备高吞吐和低负载的特点，功能上主要是包含集合管理、文档读写等能力，可以节省用户的编码成本。 

这篇文章中，我们将介绍C++ SDK核心接口的用法以及代码示例，以协助用户更快接入。 




## 1. 创建客户端

第一步，需要创建客户端，默认客户端采用grpc协议，且为同步调用。

```c++
#include <iostream>
#include "proxima_searcher_client.h"

using proxima::be;

// 默认类型客户端
auto client = ProximaSearchClient::Create();
```

第二步，创建完客户端之后，我们需要显示连接ProximaBE服务:

```c++
// 设置连接参数，注意这里需要填写grpc协议监听端口
ChannelOptions options("127.0.0.1:16000");

// 连接服务端
Status status = client->connect(options);
if (status.code != 0) {
  std::cerr << "Connect server failed." << std::endl;
  return status.code;
}
```

我们在连接过程中，除了简单的测试rpc连通性之外，还会与server校验版本信息，确保兼容性。 

客户端所有接口，都会返回Status，用户需要检查status.code是否正常，其结构如下:

```c++
struct Status {
  /// 错误码，0代表成功，其它非0代表调用错误
  int code;
  
  /// 错误原因, 默认为"Success",否则是错误描述
  std::string reason;
}
```

代码中涉及到其它结构以及字段，可以参见[ChannelOptions](#channeloptions)



## 2. 集合管理 

我们提供了Collection的创建、删除、查询、统计等接口，供用户方便的管理集合。



### 2.1. 创建Collection

创建一个名为Plants的集合，其中正排列为价格"price"以及描述"description"，索引列为向量列，存储4维float类型

```c++
/// 描述Collection的具体格式
CollectionConfig config;
config.collection_name = "Plants";
config.forward_columns = {"Price", "Description"};
config.index_columns = {IndexColumnParam("ImageVector", DataType::VECTOR_FP32, 8)}

/// 创建Collection
Status status = client->create_collection(config);
if (status.code != 0) {
  std::cerr << "Create collection failed." << std::endl;
  return status.code;
}
```

集合配置详细信息请参考: [CollectionConfig](#collectionconfig)



### 2.2. 查询Collection

创建完集合之后，我们可以使用查询接口查看其状态。

```c++
/// 获取CollectionInfo
CollectionInfo collection_info;
Status status = client->describe_collection("Plants", &collection_info);
if (status.code != 0) {
  std::cerr << "Get collection info failed." << std::endl;
  return status.code;
}

/// 打印CollectionInfo
std::cout << collection_info.collection_name << std::endl;
std::cout << collection_info.collection_status << std::endl;
std::cout << collection_info.uuid << std::endl;
......
```

除此之外，我们还可以将所有Collection信息一次性列出来

```c++
/// 获取所有的Collection信息
std::vector<CollectionInfo> collection_infos;
Status status = client->list_collections(&collection_infos);
if (status.code != 0) {
	std::cerr << "List collection infos failed." << std::endl;
  return status.code;
}
```

返回的集合信息详细字段请参考: [CollectionInfo](#collectioninfo)



### 2.3. 统计Collection

Collection经过一段时间的读写之后，我们可以通过统计接口观察其装载的文档数据。

```c++
/// 获取CollectionStats
CollectionStats collection_stats;
Status status = client->stats_collection("Plants", &collection_stats);
if (status.code != 0) {
  std::cerr << "Get collection statistics failed." << std::endl;
  return status.code;
}

/// 打印CollectionStats
std::cout << collection_stats.collection_name << std::endl;
std::cout << collection_stats.total_doc_count << std::endl;
std::cout << collection_stats.total_segment_cout << std::endl;
.....
```

集合统计详细字段信息请参考: [CollectionStats](#collectionstats)



### 2.4. 删除Collection

Collection完成其历史使命后，我们可以彻底删除掉某个collection。

```c++
/// 删除集合
Status status = client->drop_collection("Plants");
if (status.code != 0) {
  std::cerr << "Drop collection failed." << std::endl;
  return status.code;
}
```



## 3. 文档管理

对于某一个集合而言，我们提供增加、更新、删除文档的接口，同时支持批量模式。

### 3.1. 插入文档

我们在之前已经创建完成的名为"Plants"的集合中，插入100条数据

```c++
/// 创建一个写入请求
auto write_request = WriteRequest::Create();

/// 设置集合以及文档数据格式
write_request->set_collection_name("Plants");
write_request->add_forward_columns({"Price", "Description"});
write_request->add_index_column("ImageVector", DataType::VECTOR_FP32, 8);

/// 批量填充100条文档数据
for (int i = 0; i < 100; i++) {
	auto row = write_request->add_row();
  row->set_primary_key(i);
  /// 设置为插入操作
  row->set_operation_type(OperationType::INSERT);
  row->add_forward_value((float)i + 0.1f);
  row->add_forward_value("iris with number " + std::to_string(i));
  row->add_index_value({i, i, i, i, i, i, i, i});
}

/// 写入到服务端
Status status = client->write(*write_request);
if (status.code != 0) {
  std::cerr << "Write records failed." << std::endl;
  return status.code;
}
```

在上述参考代码中，有几点值得注意:

* 设置文档数据格式。 这里最早其实在创建Collection时候，我们填写过一次，这里需要再次设置的主要原因有两个。 第一个是由于我们未来会支持实时量化的能力，可能用户配置的类型与输入类型不一致，使用我们动态转换的能力。第二个则是我们可以基于这个信息做二次校验，防止插入数据格式错误。

* 批量模式。 理论上在一个WriteRequest中我们可以组合任意多的文档，但由于会增大单个网络请求包大小，一般还是建议用户根据实际情况进行限制。

  

### 3.2. 更新文档

更新过程基本与插入过程一致，仅改变文档的操作类型即可。

```c++
.....
/// 批量更新100条文档数据
for (int i = 0; i < 100; i++) {
	auto row = write_request->add_row();
  row->set_primary_key(i);
  /// 设置为更新操作
  row->set_operation_type(OperationType::UPDATE);
  row->add_forward_value((float)i + 0.2f);
  row->add_forward_value("iris with number " + std::to_string(i));
  row->add_index_value({i, i, i, i, i, i, i, i});
}
......
```


> **NOTE**: 值得注意的是，目前我们实际实现中采取的是标记删除的模式，更新=删除+新增，而删除占比过高会极大影响查询性能，这一点用户务必注意。


写入请求的详细信息可以参考: [WriteRequest](#writerequest)



### 3.3. 删除文档

删除过程依然可以复用上述的代码结构，如果批量请求中没有插入、更新请求，那么代码可以极大简化。

```c++
/// 创建一个写入请求
auto write_request = WriteRequest::Create();

/// 设置集合以及文档数据格式
write_request->set_collection_name("Plants");

/// 批量删除100条文档数据
for (int i = 0; i < 100; i++) {
	auto row = write_request->add_row();
  /// 设置为删除操作，仅需要填写primary key即可
  row->set_primary_key(i);
  row->set_operation_type(OperationType::DELETE);
}

/// 发送到服务端
Status status = client->write(*write_request);
if (status.code != 0) {
  std::cerr << "Write records failed." << std::endl;
  return status.code;
}
```


> **NOTE**: 值得注意的是，由于我们内部采用图索引结构，删除目前采取标记删除方式，删除比例过高会带来连通性问题，影响召回和性能。我们一般不建议用户删除超过50%的文档，否则查询性能将会弱化很多。


## 4. 文档查询

最后我们希望能够在创建的"Plants"集合中查询这100条数据，相似度召回10条结果。

```c++

auto query_request = QueryRequest::Create();
auto query_response = QueryResponse::Create();

/// 填充请求参数
query_request->set_collection_name("Plants");
auto knn_param = query_request->add_knn_query_param();
knn_param->set_column_name("ImageVector");
knn_param->set_topk(10);
knn_param->set_features({0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8});

/// Knn查询
Status status = client->query(*query_request, query_response.get());
if (status.code != 0) {
  std::cerr << "Query records failed." << std::endl;
  return status.code;
}

/// 打印结果
for (size_t i = 0; i < query_response->result_count(); i++) {
  auto result = query_response->result(i);
  for (size_t j = 0; j < result->document_count(); j++){
    auto doc = result->document(j);
    /// 获取文档的primary key和相似度距离
    std::cout << doc->primary_key() << std::endl;
    std::cout << doc->score() << std::endl;
    
    /// 按顺序取正排字段
    float price;
    std::string desc;
    doc->get_forward_value("Price", &price);
    doc->get_forward_value("Description", &desc);
    std::cout << price << std::endl;
    std::cout << desc << std::endl;
  }
}
```

我们query同样也可以支持批量模式

```c++
const *vector_bytes;
size_t vector_size;
......
auto knn_param = query_request->add_knn_query_param();
knn_param->set_column_name("ImageVector");
knn_param->set_topk(10);
knn_param->set_features(vector_bytes, vector_size, 100);
knn_param->set_data_type(DataType::VECTOR_FP32);
knn_param->set_dimension(8);
```

上述代码中描述了设置一段连续的bytes作为查询向量，包含100个连续的查询向量，其中每个为8维float类型。

> **NOTE**: 还有一点值得注意的是，最终的正排数据的获取，必须按照CollectionConfig定义的顺序逐一获取，顺序不对或者类型不对，均会导致获取到空结果


上述参考代码中查询请求和查询结果的详细信息请参考: [QueryRequest](#queryrequest)



## 5. 错误码

| 错误码 | 错误原因                 | 备注 |
| ------ | ------------------------ | ---- |
| 0~9999 | 服务端错误               |      |
| 10000  | 初始化客户端失败         |      |
| 10001  | RPC连接出错              |      |
| 10002  | 客户端与服务端版本不匹配 |      |
| 10003  | 客户端还未连接           |      |
| 10004  | 请求校验失败             |      |



## 6. 接口&类型



### ProximaSearchClient

```c++
/**
 * ProximaSearchClient wrappers the operations used to call proxima search
 * engine's service. Server may be running on another machines. It shields
 * implementation of communication protocol and rpc details, and provide
 * extremely high bench performance.
 *
 * Usage exp:
 *     auto client = ProximaSearchClient::Create();
 *     if (client != nullptr) {
 *       client->connect(ChannelOptions("127.0.0.1:16000"));
 *       ...
 *       client->create_collection();
 *       client->close();
 *     }
 *
 * Please read the examples/client_example.cc for more details.
 *
 * Note: the functions of this class are sync call.
 */
class ProximaSearchClient {
 public:
  /// @brief Create a client instance and return its shared ptr.
  ///
  /// @param type Client type, only support "GrpcClient" now.
  /// @return Shared ptr pointed to client impl.
  ///
  /// @note If input type is wrong, it may return nullptr
  static ProximaSearchClientPtr Create(const std::string &type);

  //! Create a shared ptr of client with default type
  static ProximaSearchClientPtr Create();

  /// @brief Try to connect remote server and establish connection.
  ///
  /// @param options Socket connection relative configs.
  /// @return Status.code 0 means success, other means fail
  ///
  /// @note This function will try to send a list collections command
  ///       to test if the server alive.
  virtual Status connect(const ChannelOptions &options) = 0;

  //! Close connection to remote server and cleanup self
  virtual Status close() = 0;

  /// @brief Create a collection with specific config.
  ///
  /// @param config Collection config
  /// @return       Status.code 0 means success, other means fail
  virtual Status create_collection(const CollectionConfig &config) = 0;

  /// @brief Drop a collection with specific name.
  ///
  /// @param collection_name Collection name
  /// @return Status.code 0 means success, other means fail
  virtual Status drop_collection(const std::string &collection_name) = 0;

  /// @brief Show the detailed information of collection.
  ///
  /// @param[in]  collection_name Collection name
  /// @param[out] collection_info Collection information
  /// @return Status.code 0 means success, other means fail
  virtual Status describe_collection(const std::string &collection_name,
                                     CollectionInfo *collection_info) = 0;

  /// @brief Get collection statics.
  ///
  /// @param[in]  collection_name Collection name
  /// @param[out] stats           Collection statistics struct
  /// @return Status.code 0 means success, other means fail.
  virtual Status stats_collection(const std::string &collection_name,
                                  CollectionStats *stats) = 0;

  /// @brief List all collections.
  ///
  /// @param[out] collections Collection infomations
  /// @return Status.code 0 means success, other means fail
  virtual Status list_collections(std::vector<CollectionInfo> *collections) = 0;

  /// @brief Insert/Update/Delete records.
  ///
  /// @param request Write request
  /// @return Status.code means success, other means fail
  virtual Status write(const WriteRequest &request) = 0;

  /// @brief Knn query similar results
  ///
  /// @param[in]  request  Query request
  /// @param[out] respnose Query response
  /// @return Status.code means success, other means fail
  virtual Status query(const QueryRequest &request,
                       QueryResponse *response) = 0;

  /// @brief Get document by primary key
  ///
  /// @param[in]  request  Get document request
  /// @param[out] response Get document response
  /// @return Status.code means success, other means fail
  virtual Status get_document_by_key(const GetDocumentRequest &request,
                                     GetDocumentResponse *response) = 0;
};
```



### ChannelOptions

```c++
/**
 * ChannelOptions represents the  connection config.
 */
struct ChannelOptions {
  /// Host name of proximabe server
  /// For exapmle: "127.0.0.1:16000"
  /// Required field
  std::string host{};

  /// Max rpc duration out over server
  /// Optional field, default 1000
  uint32_t timeout_ms{1000U};

  /// Max retry times when rpc failed
  /// Optional filed, default 3
  uint32_t max_retry{3U};

  /// Connection pool count
  /// Optional filed, default 1
  uint32_t connection_count{1};

  ChannelOptions(const std::string &val) : host(val) {}
};
```



### CollectionConfig

```c++
/**
 * CollectionConfig describes the config options of collection.
 * It includes description of index columns and forward columns.
 * Index columns means that this column data is for knn searching.
 * Forward columns means that this column data is just for display,
 * which is not anticipating in search process.
 */
struct CollectionConfig {
  /// Collection name, it should be unique
  /// Required field
  std::string collection_name{};

  /// Collection will split into serveral segments
  /// This param means max doc limits in one segment
  /// Optional field, default 0, means no limit
  uint32_t max_docs_per_segment{0U};

  /// Forward column names
  /// Optional field
  std::vector<std::string> forward_columns{};

  /// Index column infos
  /// Required filed
  std::vector<IndexColumnParam> index_columns{};

  /// Database repository config
  /// Optional field, default empty
  DatabaseRepository database_repository{};
};


/**
 * IndexColumnParam represents the index config of index column.
 */
struct IndexColumnParam {
  /// Column name
  /// Required field
  std::string column_name{};

  /// Column index type
  /// Optional field, default IndexType::PROXIMA_GRAPH_INDEX
  IndexType index_type{IndexType::PROXIMA_GRAPH_INDEX};

  /// Stored data type
  /// Optional filed, default DataType::VECTOR_FP32
  DataType data_type{DataType::VECTOR_FP32};

  /// Stored data dimension
  /// Optional filed, default 0
  uint32_t dimension{0U};

  /// Extra params for column index
  /// Optional field
  /// For example:
  ///   {"ef_construction": "400", "ef_search": "300"}
  std::vector<KVPair> extra_params{};

  IndexColumnParam() = default;

  IndexColumnParam(const std::string &val1, DataType val2, uint32_t val3)
      : column_name(val1), data_type(val2), dimension(val3) {}
};
```



### CollectionInfo

```c++
/**
 * CollectionInfo describes the detailed information of collection,
 * which is ProximaSE server returned.
 */
struct CollectionInfo {
  enum class CollectionStatus : uint32_t {
    INITIALIZED = 0,
    SERVING = 1,
    DRPPED = 2
  };

  //! Collection name
  std::string collection_name{};

  //! Collection status
  CollectionStatus collection_status{CollectionStatus::INITIALIZED};

  //! Unique uuid to a collection
  std::string collection_uuid{};

  //! Latest record's log sequence number
  uint64_t latest_lsn{0U};

  //! Latest record's lsn context
  std::string latest_lsn_context{};

  //! Server magic number, generally is server started timestamp
  uint64_t magic_number{0U};

  //! Collection's config max doc number per segment
  uint32_t max_docs_per_segment{0U};

  //! Collection's forward column names
  std::vector<std::string> forward_columns{};

  //! Collection's index column params
  std::vector<IndexColumnParam> index_columns{};

  //! Collection's database repository information
  DatabaseRepository database_repository{};
};
```



### CollectionStats

```c++
/**
 * CollectionStats describes the detailed stastistics of collection
 */
struct CollectionStats {
  /**
   * Segment state
   */
  enum class SegmentState : uint32_t {
    CREATED = 0,
    WRITING = 1,
    DUMPING = 2,
    COMPACTING = 3,
    PERSIST = 4
  };

  /*
   * SegmentStats describes the detailed stastistics of segment
   */
  struct SegmentStats {
    //! Segment unique id
    uint64_t segment_id{0U};

    //! Segment state
    SegmentState segment_state{SegmentState::CREATED};

    //! Document count in this segment
    uint64_t doc_count{0U};

    //! Index file count of this segment
    uint64_t index_file_count{0U};

    //! Totaol index file size
    uint64_t index_file_size{0U};

    //! Min document id
    uint64_t min_doc_id{0U};

    //! Max document id
    uint64_t max_doc_id{0U};

    //! Min primary key value of the segment
    uint64_t min_primary_key{0U};

    //! Min primary key value of the segment
    uint64_t max_primary_key{0U};

    //! Earliest record timestamp
    uint64_t min_timestamp{0U};

    //! Last record timestamp
    uint64_t max_timestamp{0U};

    //! Minimal log sequence number
    uint64_t min_lsn{0U};

    //! Maximum log sequence number
    uint64_t max_lsn{0U};
  };

  //! Collection name
  std::string collection_name{};

  //! Total document count of this collection
  uint64_t total_doc_count{0U};

  //! Total segment count of this collectoin
  uint64_t total_segment_count{0U};

  //! Total index file count
  uint64_t total_index_file_count{0U};

  //! Total index file size
  uint64_t total_index_file_size{0U};

  //! Detailed segment stastistics
  std::vector<SegmentStats> segment_stats{};
};
```



### WriteRequest

```c++
/**
 * WriteRequest shows how to wrapper write request data fields.
 *
 * Usage exp:
 *   WriteRequestPtr request = WriteRequest::Create();
 *   request->set_collection_name("test_collection");
 *   request->set_row_meta({"test_column"}, {});
 *   auto row = request->add_row();
 *   row->set_primary_key = 123;
 *   row->set_operation_type(OperationType::OP_INSERT);
 *   row->add_index_value({0.1, 0.2, 0.3});
 *   ...
 *   client->write(*request);
 */
class WriteRequest {
 public:
  /**
   * A row describes the format of one record
   */
  class Row {
   public:
    //! Set primary key, required
    virtual void set_primary_key(uint64_t val) = 0;

    //! Set operation type, optional, default DataType::INSERT
    virtual void set_operation_type(OperationType op_type) = 0;

    //! Set lsn, optional, default 0
    virtual void set_lsn(uint64_t lsn) = 0;

    //! Set lsn context, optional, default ""
    virtual void set_lsn_context(const std::string &lsn_context) = 0;

    /// @brief Add forward value with string type
    ///
    /// @note  Add forward value sort must match configured
    ///        forward columns in CollectionConfig
    virtual void add_forward_value(const std::string &val) = 0;

    //! Add forward value with bool type
    virtual void add_forward_value(bool val) = 0;

    //! Add forward value with int32 type
    virtual void add_forward_value(int32_t val) = 0;

    //! Add forward value with int64 type
    virtual void add_forward_value(int64_t val) = 0;

    //! Add forward value with uint32 type
    virtual void add_forward_value(uint32_t val) = 0;

    //! Add forward value with uint64 type
    virtual void add_forward_value(uint64_t val) = 0;

    //! Add forward value with float type
    virtual void add_forward_value(float val) = 0;

    //! Add forward value with double type
    virtual void add_forward_value(double val) = 0;

    /// @brief Add index value, vector bytes type
    ///
    /// @note Add index value sort must match configured
    ///       index columns in CollectionConfig
    virtual void add_index_value(const void *val, size_t val_len) = 0;

    //! Add index value, vector array type
    virtual void add_index_value(const std::vector<float> &val) = 0;

    /// Add index value by json format
    /// Two json format:
    ///   "[0.1, 0.2, 0.3, 0.4]"
    ///   "[[0.1, 0.2, 0.3, 0.4], [0.5, 0.6, 0.7, 0.8]]"
    virtual void add_index_value_by_json(const std::string &json_val) = 0;
  };
  using RowPtr = std::shared_ptr<Row>;

 public:
  //! Constructor
  static WriteRequestPtr Create();

  //! Set collection name, required, must be unique
  virtual void set_collection_name(const std::string &val) = 0;

  /// @brief Add forward column in row meta
  /// @note Forward column names' sort must match configured
  ///       forward columns in CollectionConfig
  virtual void add_forward_column(const std::string &column_name) = 0;

  /// @brief Add forward columns in row meta
  /// @note Forward column names' sort must match configured
  ///       forward columns in CollectionConfig
  virtual void add_forward_columns(
      const std::vector<std::string> &column_names) = 0;

  /// @brief Add index column in row meta
  ///
  /// @param column_name Column name
  /// @param data_type   Send data type
  /// @param dimension   Send data dimension
  ///
  /// @note Index column names' sort must match configured
  ///       index columns in CollectionConfig
  virtual void add_index_column(const std::string &column_name,
                                DataType data_type, uint32_t dimension) = 0;

  //! Add row data, required, can't send empty request
  virtual WriteRequest::RowPtr add_row() = 0;

  //! Set request id for tracelog, optional
  virtual void set_request_id(const std::string &request_id) = 0;

  //! Set magic number for validation, optional
  virtual void set_magic_number(uint64_t magic_number) = 0;
};
```



### QueryRequest

```c++
/**
 * QueryRequest shows how to wrapper query data fields.
 *
 * Usage exp:
 *   QueryRequestPtr request = QueryRequest::Create();
 *   request->set_collection_name("test_colletion");
 *   auto knn_param = request->add_knn_query_param();
 *   knn_param->set_column_name("test_column");
 *   knn_param->set_features({0.1, 0.2, 0.3, 0.4});
 *   knn_param->set_batch_count(1);
 *   knn_param->set_dimension(4);
 *   knn_param->set_data_type(DT_VECTOR_FP32);
 *   ...
 *
 */
class QueryRequest {
 public:
  /**
   * KnnQueryParam describes the options of knn query
   */
  class KnnQueryParam {
   public:
    //! Set column name, required
    virtual void set_column_name(const std::string &val) = 0;

    //! Set topk, required
    virtual void set_topk(uint32_t val) = 0;

    /// Set query vector with bytes format by single
    /// Required set
    virtual void set_features(const void *val, size_t val_len) = 0;

    //! Set features with vector array format by single
    virtual void set_features(const std::vector<float> &val) = 0;

    //! Set query vector with bytes format by batch
    virtual void set_features(const void *val, size_t val_len,
                              uint32_t batch) = 0;

    /// Set features by json format
    /// Two json format:
    ///   "[0.1, 0.2, 0.3, 0.4]"
    ///   "[[0.1, 0.2, 0.3, 0.4], [0.5, 0.6, 0.7, 0.8]]"
    virtual void set_features_by_json(const std::string &json_val) = 0;

    //! Set features by json format and by batch
    virtual void set_features_by_json(const std::string &json_val,
                                      uint32_t batch) = 0;

    //! Set vector data dimension, required
    virtual void set_dimension(uint32_t val) = 0;

    //! Set vector data type, required
    virtual void set_data_type(DataType val) = 0;

    //! Set search radius, optional, default 0.0f not open
    virtual void set_radius(float val) = 0;

    //! Set if use linear search, optional, default false
    virtual void set_linear(bool val) = 0;

    //! Add extra params, like ef_search ..etc, optional
    virtual void add_extra_param(const std::string &key,
                                 const std::string &val) = 0;
  };
  using KnnQueryParamPtr = std::shared_ptr<KnnQueryParam>;

 public:
  //! Constructor
  static QueryRequestPtr Create();

  //! Set collection name, required
  virtual void set_collection_name(const std::string &val) = 0;

  //! Set knn query param, required
  virtual QueryRequest::KnnQueryParamPtr add_knn_query_param() = 0;

  //! Set debug mode, optional, default false
  virtual void set_debug_mode(bool val) = 0;
};
```



### QueryResponse

```c++
/**
 * QueryResponse shows the format of query response.
 */
class QueryResponse {
 public:
  /**
   * Result represents a knn query's result
   */
  class Result {
   public:
    //! Return document count
    virtual size_t document_count() const = 0;

    //! Return document pointer of specific pos
    virtual DocumentPtr document(int index) const = 0;
  };

 public:
  //! Constructor
  static QueryResponsePtr Create();

  //! Return debug info
  virtual const std::string &debug_info() const = 0;

  //! Return query latency, microseconds
  virtual uint64_t latency_us() const = 0;

  //! Return batch result count
  virtual size_t result_count() const = 0;

  //! Return result pointer of specific batch pos
  virtual QueryResponse::ResultPtr result(int index) const = 0;
};


/**
 * Document shows the format of knn query response
 */
class Document {
 public:
  //! Return document primary key
  virtual uint64_t primary_key() const = 0;

  //! Return calculated knn distance score
  virtual float score() const = 0;

  //! Return forward values count
  virtual size_t forward_count() const = 0;

  //! Get forward names
  virtual void get_forward_names(
      std::vector<std::string> *forward_names) const = 0;

  //! Get forward value with string type
  virtual void get_forward_value(const std::string &key,
                                 std::string *val) const = 0;

  //! Get forward value with bool type
  virtual void get_forward_value(const std::string &key, bool *val) const = 0;

  //! Get forward value with int32 type
  virtual void get_forward_value(const std::string &key,
                                 int32_t *val) const = 0;

  //! Get forward value with int64 type
  virtual void get_forward_value(const std::string &key,
                                 int64_t *val) const = 0;

  //! Get forward value with uint32 type
  virtual void get_forward_value(const std::string &key,
                                 uint32_t *val) const = 0;

  //! Get forward value with uint64 type
  virtual void get_forward_value(const std::string &key,
                                 uint64_t *val) const = 0;

  //! Get forward value with float type
  virtual void get_forward_value(const std::string &key, float *val) const = 0;

  //! Get forward value with double type
  virtual void get_forward_value(const std::string &key, double *val) const = 0;
};
```

