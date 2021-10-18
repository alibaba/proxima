/**
 *   Copyright 2021 Alibaba, Inc. and its affiliates. All Rights Reserved.
 * 
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *   
 *       http://www.apache.org/licenses/LICENSE-2.0
 *   
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.

 *   \author   Haichao.chc
 *   \date     Oct 2020
 *   \brief    Abstract class to describe proxima client interface and action.
 *             You can get more details usage from examples
 */

#pragma once

#include <memory>
#include <string>
#include <vector>

namespace proxima {
namespace be {

struct Status;
struct ChannelOptions;
struct CollectionConfig;
struct CollectionInfo;
struct CollectionStats;
class WriteRequest;
class QueryRequest;
class Document;
class QueryResponse;
class GetDocumentRequest;
class GetDocumentResponse;
class ProximaSearchClient;

using ProximaSearchClientPtr = std::shared_ptr<ProximaSearchClient>;

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
  //! Destructor
  virtual ~ProximaSearchClient() = default;

  /// @brief Create a client instance and return its shared ptr.
  ///
  /// @param type Client type, support "GrpcClient" and "HttpClient" now.
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

/**
 * IndexColumn's index type, only supports PROXIMA_GRAPH_INDEX for vector.
 */
enum class IndexType : uint32_t { UNDEFINED = 0, PROXIMA_GRAPH_INDEX = 1 };

/**
 * Supported input data type.
 */
enum class DataType : uint32_t {
  UNDEFINED = 0,
  BINARY = 1,
  STRING = 2,
  BOOL = 3,
  INT32 = 4,
  INT64 = 5,
  UINT32 = 6,
  UINT64 = 7,
  FLOAT = 8,
  DOUBLE = 9,

  VECTOR_BINARY32 = 20,
  VECTOR_BINARY64 = 21,
  VECTOR_FP16 = 22,
  VECTOR_FP32 = 23,
  VECTOR_FP64 = 24,
  VECTOR_INT4 = 25,
  VECTOR_INT8 = 26,
  VECTOR_INT16 = 27
};

/**
 * Operation type of records.
 */
enum class OperationType : uint32_t { INSERT = 0, UPDATE = 1, DELETE = 2 };

/**
 * Status struct wrappers remote server's response.
 */
struct Status {
  /// Response error code
  ///  0 means success
  /// ~0 means error
  int code{0U};

  /// Response error message, default is "Success"
  std::string reason{"Success"};
};

/**
 * ChannelOptions represents the  connection config.
 */
struct ChannelOptions {
  /// Host name of proxima be server
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

/**
 * Common key-value pair struct
 */
struct KVPair {
  std::string key{};
  std::string value{};
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

/**
 * DatabaseRepository represents database config which stores
 * source data, it's like some kind of ETL config.
 */
struct DatabaseRepository {
  /// Repository name, make sure it's unique
  /// Required field
  std::string repository_name{};

  /// Database connection uri, like JDBC string format
  /// Required field
  std::string connection_uri{};

  /// Table name in database
  /// Required field
  std::string table_name{};

  /// User name which connect to database
  /// Optional field, default empty
  std::string user{};

  /// Password relative to user name
  /// Optional password, default empty
  std::string password{};
};

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

using WriteRequestPtr = std::shared_ptr<WriteRequest>;
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
    //! Destructor
    virtual ~Row() = default;

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

  //! Destructor
  virtual ~WriteRequest() = default;

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


using QueryRequestPtr = std::shared_ptr<QueryRequest>;
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
    // Destructor
    virtual ~KnnQueryParam() = default;

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

  //! Destructor
  virtual ~QueryRequest() = default;

  //! Set collection name, required
  virtual void set_collection_name(const std::string &val) = 0;

  //! Set knn query param, required
  virtual QueryRequest::KnnQueryParamPtr add_knn_query_param() = 0;

  //! Set debug mode, optional, default false
  virtual void set_debug_mode(bool val) = 0;
};

using DocumentPtr = std::shared_ptr<Document>;
/**
 * Document shows the format of knn query response
 */
class Document {
 public:
  //! Destructor
  virtual ~Document() = default;

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


using QueryResponsePtr = std::shared_ptr<QueryResponse>;
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
    //! Destructor
    virtual ~Result() = default;

    //! Return document count
    virtual size_t document_count() const = 0;

    //! Return document pointer of specific pos
    virtual DocumentPtr document(int index) const = 0;
  };
  using ResultPtr = std::shared_ptr<Result>;

 public:
  //! Constructor
  static QueryResponsePtr Create();

  //! Destructor
  virtual ~QueryResponse() = default;

  //! Return debug info
  virtual const std::string &debug_info() const = 0;

  //! Return query latency, microseconds
  virtual uint64_t latency_us() const = 0;

  //! Return batch result count
  virtual size_t result_count() const = 0;

  //! Return result pointer of specific batch pos
  virtual QueryResponse::ResultPtr result(int index) const = 0;
};


using GetDocumentRequestPtr = std::shared_ptr<GetDocumentRequest>;
/*
 * GetDocumentRequest shows the format of get document request.
 *
 * Usage exp:
 *   GetDocumentRequestPtr request = GetDocumentRequest::Create();
 *   request->set_collection_name("test_collection");
 *   request->set_primary_key(123);
 *   ...
 */
class GetDocumentRequest {
 public:
  //! Constructor
  static GetDocumentRequestPtr Create();

  //! Destructor
  virtual ~GetDocumentRequest() = default;

  //! Set collection name, required
  virtual void set_collection_name(const std::string &val) = 0;

  //! Set primary key, required
  virtual void set_primary_key(uint64_t val) = 0;

  //! Set debug mode, optional, default false
  virtual void set_debug_mode(bool val) = 0;
};


using GetDocumentResponsePtr = std::shared_ptr<GetDocumentResponse>;
/*
 * GetDocumentResponse shows the format of get document response
 */
class GetDocumentResponse {
 public:
  //! Constructor
  static GetDocumentResponsePtr Create();

  //! Destructor
  virtual ~GetDocumentResponse() = default;

  //! Return debug info
  virtual const std::string &debug_info() const = 0;

  //! Return document that found
  virtual DocumentPtr document() const = 0;
};


}  // end namespace be
}  // end namespace proxima
