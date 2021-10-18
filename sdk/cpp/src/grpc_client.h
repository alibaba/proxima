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
 *   \brief    Implementation with grpc protobuf protocol
 */

#pragma once

#include <map>
#include <queue>

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
#include <brpc/channel.h>
#include <brpc/selective_channel.h>
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

#include "proxima_search_client.h"

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
#include "proto/proxima_be.pb.h"
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

namespace proxima {
namespace be {

class PbWriteRequest;
class PbQueryRequest;
class PbQueryResponse;
class PbGetDocumentRequest;
class PbGetDocumentResponse;

/*
 * ProximaSearchClient implementation with grpc protobuf protocol
 */
class GrpcProximaSearchClient : public ProximaSearchClient {
 public:
  //! Constructor
  GrpcProximaSearchClient() = default;

  //! Destructor
  ~GrpcProximaSearchClient() override = default;

  //! Connect remote server
  Status connect(const ChannelOptions &options) override;

  //! Close connection to remote server
  Status close() override;

  //! Create a collection with config
  Status create_collection(const CollectionConfig &config) override;

  //! Drop a collection with specific collection name
  Status drop_collection(const std::string &collection_name) override;

  //! Get collection information with specific collection name
  Status describe_collection(const std::string &collection_name,
                             CollectionInfo *collection_info) override;

  //! Get collection stastics with specific collection name
  Status stats_collection(const std::string &collection_name,
                          CollectionStats *stats) override;

  //! Get all the collections' information
  Status list_collections(std::vector<CollectionInfo> *collections) override;

  //! Write records, including insert/update/delete operations
  Status write(const WriteRequest &request) override;

  //! Query records, including knn query saerch
  Status query(const QueryRequest &request, QueryResponse *response) override;

  //! Get specific record with primary key
  Status get_document_by_key(const GetDocumentRequest &request,
                             GetDocumentResponse *response) override;

 protected:
  virtual void rpc_create_collection(brpc::Controller *cntl,
                                     const proto::CollectionConfig *request,
                                     proto::Status *response);

  virtual void rpc_drop_collection(brpc::Controller *cntl,
                                   const proto::CollectionName *request,
                                   proto::Status *response);

  virtual void rpc_describe_collection(
      brpc::Controller *cntl, const proto::CollectionName *request,
      proto::DescribeCollectionResponse *response);

  virtual void rpc_stats_collection(brpc::Controller *cntl,
                                    const proto::CollectionName *request,
                                    proto::StatsCollectionResponse *response);

  virtual void rpc_list_collections(brpc::Controller *cntl,
                                    const proto::ListCondition *request,
                                    proto::ListCollectionsResponse *response);

  virtual void rpc_write(brpc::Controller *cntl,
                         const proto::WriteRequest *request,
                         proto::Status *response);

  virtual void rpc_query(brpc::Controller *cntl,
                         const proto::QueryRequest *request,
                         proto::QueryResponse *response);

  virtual void rpc_get_document_by_key(brpc::Controller *cntl,
                                       const proto::GetDocumentRequest *request,
                                       proto::GetDocumentResponse *response);

 protected:
  static constexpr uint32_t ErrorCode_InitChannel = 10000;
  static constexpr uint32_t ErrorCode_RpcError = 10001;
  static constexpr uint32_t ErrorCode_MismatchedVersion = 10002;
  static constexpr uint32_t ErrorCode_NotConnected = 10003;
  static constexpr uint32_t ErrorCode_ValidateError = 10004;

 private:
  //! Check version
  bool check_server_version(Status *status);

  //! Convert struct to protobuf object
  void convert(const CollectionConfig &config,
               proto::CollectionConfig *pb_request);

  //! Convert struct to protobuf object
  void convert(const proto::CollectionInfo &pb_response,
               CollectionInfo *collection_info);

  //! Convert struct to protobuf object
  void convert(const proto::CollectionStats &pb_response,
               CollectionStats *collection_stats);

  //! Validate legality of collection config
  Status validate(const CollectionConfig &config);

  //! Validate legality of write request
  Status validate(const PbWriteRequest &request);

  //! Validate legality of query request
  Status validate(const PbQueryRequest &request);

  //! Validate legality of get document request
  Status validate(const PbGetDocumentRequest &request);

 protected:
  bool connected_{false};

 private:
  brpc::SelectiveChannel client_channel_{};
};


/*
 * WriteRequest implementation with protobuf protocol
 */
class PbWriteRequest : public WriteRequest {
 public:
  /*
   * A row describes the format of one record
   */
  class PbRow : public WriteRequest::Row {
   public:
    //! Constructor
    PbRow(proto::WriteRequest::Row *p_row) : row_(p_row) {}

    //! Destructor
    ~PbRow() override = default;

    //! Set primary key, must set
    void set_primary_key(uint64_t val) override {
      row_->set_primary_key(val);
    }

    //! Set operation type, default OP_INSERT
    void set_operation_type(OperationType op_type) override {
      row_->set_operation_type((proto::OperationType)op_type);
    }

    //! Set lsn, optional set, generally used by database repo
    void set_lsn(uint64_t lsn) override {
      row_->mutable_lsn_context()->set_lsn(lsn);
    }

    //! Set lsn context, optional set, generally used by database repo
    void set_lsn_context(const std::string &lsn_context) override {
      row_->mutable_lsn_context()->set_context(lsn_context);
    }

    //! Add forward value, must match forward column names
    void add_forward_value(const std::string &val) override {
      row_->mutable_forward_column_values()->add_values()->set_string_value(
          val);
    }

    //! Add forward value with bool type
    void add_forward_value(bool val) override {
      row_->mutable_forward_column_values()->add_values()->set_bool_value(val);
    }

    //! Add forward value with int32 type
    void add_forward_value(int32_t val) override {
      row_->mutable_forward_column_values()->add_values()->set_int32_value(val);
    }

    //! Add forward value with int64 type
    void add_forward_value(int64_t val) override {
      row_->mutable_forward_column_values()->add_values()->set_int64_value(val);
    }

    //! Add forward value with uint32 type
    void add_forward_value(uint32_t val) override {
      row_->mutable_forward_column_values()->add_values()->set_uint32_value(
          val);
    }

    //! Add forward value with uint64 type
    void add_forward_value(uint64_t val) override {
      row_->mutable_forward_column_values()->add_values()->set_uint64_value(
          val);
    }

    //! Add forward value with float type
    void add_forward_value(float val) override {
      row_->mutable_forward_column_values()->add_values()->set_float_value(val);
    }

    //! Add forward value with double type
    void add_forward_value(double val) override {
      row_->mutable_forward_column_values()->add_values()->set_double_value(
          val);
    }

    //! Add index value, vector bytes type
    void add_index_value(const void *val, size_t val_len) override {
      row_->mutable_index_column_values()->add_values()->set_bytes_value(
          std::string((const char *)val, val_len));
    }

    //! Add index value, vector array type
    void add_index_value(const std::vector<float> &val) override {
      row_->mutable_index_column_values()->add_values()->set_bytes_value(
          std::string((const char *)val.data(), val.size() * sizeof(float)));
    }

    //! Add index value by json format
    void add_index_value_by_json(const std::string &json_val) override {
      row_->mutable_index_column_values()->add_values()->set_string_value(
          json_val);
    }

   private:
    proto::WriteRequest::Row *row_{nullptr};
  };

 public:
  //! Constructor
  PbWriteRequest() = default;

  //! Destructor
  ~PbWriteRequest() override = default;

  //! Set collection name, must set
  void set_collection_name(const std::string &val) override {
    request_.set_collection_name(val);
  }

  //! Add forward column
  void add_forward_column(const std::string &column_name) override {
    request_.mutable_row_meta()->add_forward_column_names(column_name);
  }

  //! Add forward columns
  void add_forward_columns(
      const std::vector<std::string> &column_names) override {
    for (auto &it : column_names) {
      request_.mutable_row_meta()->add_forward_column_names(it);
    }
  }

  //! Add index column
  void add_index_column(const std::string &column_name, DataType data_type,
                        uint32_t dimension) override {
    auto *index_column = request_.mutable_row_meta()->add_index_column_metas();
    index_column->set_column_name(column_name);
    index_column->set_data_type((proto::DataType)data_type);
    index_column->set_dimension(dimension);
  }

  //! Add row data, must add, can't send empty request
  WriteRequest::RowPtr add_row() override {
    return std::make_shared<PbWriteRequest::PbRow>(request_.add_rows());
  }

  //! Set request id for tracelog, optional set
  void set_request_id(const std::string &request_id) override {
    request_.set_request_id(request_id);
  }

  //! Set magic number for validation, optional set
  void set_magic_number(uint64_t magic_number) override {
    request_.set_magic_number(magic_number);
  }

  //! Return raw protobuf data pointer, read only
  const proto::WriteRequest *data() const {
    return &request_;
  }

 private:
  proto::WriteRequest request_;
};

/*
 * QueryRequest implementation with protobuf protocol
 */
class PbQueryRequest : public QueryRequest {
 public:
  /*
   * KnnQueryParam implementation with protobuf protocol
   */
  class PbKnnQueryParam : public QueryRequest::KnnQueryParam {
   public:
    //! Constructor
    PbKnnQueryParam(proto::QueryRequest::KnnQueryParam *val)
        : knn_param_(val) {}

    //! Destructor
    ~PbKnnQueryParam() override = default;

    //! Set column name, must set
    void set_column_name(const std::string &val) override {
      knn_param_->set_column_name(val);
    }

    //! Set topk, must set
    void set_topk(uint32_t val) override {
      knn_param_->set_topk(val);
    }

    //! Set features with vector array format by single
    void set_features(const void *val, size_t val_len) override {
      knn_param_->set_batch_count(1);
      knn_param_->set_features((const char *)val, val_len);
    }

    //! Set features with vector array format by single
    void set_features(const std::vector<float> &val) override {
      knn_param_->set_features(
          std::string((const char *)val.data(), val.size() * sizeof(float)));
      knn_param_->set_batch_count(1);
      knn_param_->set_data_type((proto::DataType)DataType::VECTOR_FP32);
      knn_param_->set_dimension(val.size());
    }

    //! Set query vector with bytes format by batch
    void set_features(const void *val, size_t val_len,
                      uint32_t batch) override {
      knn_param_->set_batch_count(batch);
      knn_param_->set_features((const char *)val, val_len);
    }

    //! Set features by json format
    void set_features_by_json(const std::string &json_val) override {
      knn_param_->set_batch_count(1);
      knn_param_->set_matrix(json_val);
    }


    //! Set features by json format and by batch
    void set_features_by_json(const std::string &json_val,
                              uint32_t batch) override {
      knn_param_->set_batch_count(batch);
      knn_param_->set_matrix(json_val);
    }

    //! Set search radius, default 0.0f, not open
    void set_radius(float val) override {
      knn_param_->set_radius(val);
    }

    //! Set if use linear search, default false
    void set_linear(bool val) override {
      knn_param_->set_is_linear(val);
    }

    //! Set vector data dimension, must set
    void set_dimension(uint32_t val) override {
      knn_param_->set_dimension(val);
    }

    //! Set vector data type, must set
    void set_data_type(DataType val) override {
      knn_param_->set_data_type((proto::DataType)val);
    }

    //! Add extra params, like ef_search ..etc
    void add_extra_param(const std::string &key,
                         const std::string &val) override {
      auto *extra_param = knn_param_->add_extra_params();
      extra_param->set_key(key);
      extra_param->set_value(val);
    }

   private:
    proto::QueryRequest::KnnQueryParam *knn_param_{nullptr};
  };

 public:
  //! Constructor
  PbQueryRequest() = default;

  //! Destructor
  ~PbQueryRequest() override = default;

  //! Set collection name, must set
  void set_collection_name(const std::string &val) override {
    request_.set_collection_name(val);
  }

  //! Set debug mode, optional set
  void set_debug_mode(bool val) override {
    request_.set_debug_mode(val);
  }

  //! Set knn query param
  QueryRequest::KnnQueryParamPtr add_knn_query_param() override {
    return std::make_shared<PbKnnQueryParam>(request_.mutable_knn_param());
  }

  //! Return protobuf data pointer, readonly
  const proto::QueryRequest *data() const {
    return &request_;
  }

 private:
  proto::QueryRequest request_{};
};

/*
 * Document implementation with protobuf protocol
 */
class PbDocument : public Document {
 public:
  //! Constructor
  PbDocument(const proto::Document *doc_val) {
    doc_ = doc_val;
    /// Actually it shoule be in another function to load
    /// It's safe to iterate input value, so just keep it in construtor
    for (int i = 0; i < doc_->forward_column_values_size(); i++) {
      auto &fwd_val = doc_->forward_column_values(i);
      auto &key = fwd_val.key();
      auto &val = fwd_val.value();
      forward_map_.emplace(key, &val);
    }
  }

  //! Destructor
  ~PbDocument() override = default;

  //! Return primary key
  uint64_t primary_key() const override {
    return doc_->primary_key();
  }

  //! Return knn distance score
  float score() const override {
    return doc_->score();
  }

  //! Return forward count
  size_t forward_count() const override {
    return forward_map_.size();
  }

  //! Return forward names
  void get_forward_names(
      std::vector<std::string> *forward_names) const override {
    for (const auto &it : forward_map_) {
      forward_names->emplace_back(it.first);
    }
  }

  //! Return forward value with string type
  void get_forward_value(const std::string &key,
                         std::string *val) const override {
    if (forward_map_.find(key) != forward_map_.end()) {
      *val = forward_map_.at(key)->string_value();
    }
  }

  //! Return forward value with bool type
  void get_forward_value(const std::string &key, bool *val) const override {
    if (forward_map_.find(key) != forward_map_.end()) {
      *val = forward_map_.at(key)->bool_value();
    }
  }

  //! Return forward value with int32 type
  void get_forward_value(const std::string &key, int32_t *val) const override {
    if (forward_map_.find(key) != forward_map_.end()) {
      *val = forward_map_.at(key)->int32_value();
    }
  }

  //! Return forward value with int64 type
  void get_forward_value(const std::string &key, int64_t *val) const override {
    if (forward_map_.find(key) != forward_map_.end()) {
      *val = forward_map_.at(key)->int64_value();
    }
  }

  //! Return forward value with uint32 type
  void get_forward_value(const std::string &key, uint32_t *val) const override {
    if (forward_map_.find(key) != forward_map_.end()) {
      *val = forward_map_.at(key)->uint32_value();
    }
  }

  //! Return forward value with uint64 type
  void get_forward_value(const std::string &key, uint64_t *val) const override {
    if (forward_map_.find(key) != forward_map_.end()) {
      *val = forward_map_.at(key)->uint64_value();
    }
  }

  //! Return forward value with float type
  void get_forward_value(const std::string &key, float *val) const override {
    if (forward_map_.find(key) != forward_map_.end()) {
      *val = forward_map_.at(key)->float_value();
    }
  }

  //! Return forward value with double type
  void get_forward_value(const std::string &key, double *val) const override {
    if (forward_map_.find(key) != forward_map_.end()) {
      *val = forward_map_.at(key)->double_value();
    }
  }

 private:
  const proto::Document *doc_{nullptr};
  std::map<std::string, const proto::GenericValue *> forward_map_{};
};

/*
 * QueryResponse implementation of protobuf protocol
 */
class PbQueryResponse : public QueryResponse {
 public:
  /*
   * Result implementation of protobuf protocol
   */
  class PbResult : public QueryResponse::Result {
   public:
    //! Constructor
    PbResult(const proto::QueryResponse::Result *val) : result_(val) {}

    //! Destructor
    ~PbResult() override = default;

    //! Return document count
    size_t document_count() const override {
      return result_->documents_size();
    }

    //! Return document pointer of specific pos
    DocumentPtr document(int index) const override {
      if (index < result_->documents_size()) {
        return std::make_shared<PbDocument>(&result_->documents(index));
      } else {
        return DocumentPtr();
      }
    }

   private:
    const proto::QueryResponse::Result *result_{nullptr};
  };

 public:
  //! Constructor
  PbQueryResponse() = default;

  //! Destructor
  ~PbQueryResponse() override = default;

  //! Return debug info
  const std::string &debug_info() const override {
    return response_.debug_info();
  }

  //! Return query latency, microseconds
  uint64_t latency_us() const override {
    return response_.latency_us();
  }

  //! Return batch result count
  size_t result_count() const override {
    return response_.results_size();
  }

  //! Return result of specific batch pos
  QueryResponse::ResultPtr result(int index) const override {
    return std::make_shared<PbQueryResponse::PbResult>(
        &response_.results(index));
  }

  //! Return protobuf data pointer, readonly
  proto::QueryResponse *data() {
    return &response_;
  }

 private:
  proto::QueryResponse response_;
};

/*
 * GetDocumentRequest implementation of protobuf protocol
 */
class PbGetDocumentRequest : public GetDocumentRequest {
 public:
  //! Constructor
  PbGetDocumentRequest() = default;

  //! Destructor
  ~PbGetDocumentRequest() = default;

  //! Set collection name, must set
  void set_collection_name(const std::string &val) override {
    request_.set_collection_name(val);
  }

  //! Set primary key, must set
  void set_primary_key(uint64_t val) override {
    request_.set_primary_key(val);
  }

  //! Set debug mode, default false
  void set_debug_mode(bool val) override {
    request_.set_debug_mode(val);
  }

  //! Return protobuf data pointer, readonly
  const proto::GetDocumentRequest *data() const {
    return &request_;
  }

 private:
  proto::GetDocumentRequest request_;
};


/*
 * GetDocumentResponse implementation of protobuf protocol
 */
class PbGetDocumentResponse : public GetDocumentResponse {
 public:
  //! Constructor
  PbGetDocumentResponse() = default;

  //! Destructor
  ~PbGetDocumentResponse() override = default;

  //! Return debug info
  const std::string &debug_info() const override {
    return response_.debug_info();
  }

  //! If not exist the key, return nullptr, or return document shared ptr
  DocumentPtr document() const override {
    if (response_.has_document()) {
      return std::make_shared<PbDocument>(&response_.document());
    } else {
      return DocumentPtr();
    }
  }

  //! Return protobuf data pointer
  proto::GetDocumentResponse *data() {
    return &response_;
  }

 private:
  proto::GetDocumentResponse response_;
};


}  // namespace be
}  // end namespace proxima
