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
 *   \brief    Implementation with grpc protocol protocol
 */

#include "grpc_client.h"
#include <iostream>
#include <ailego/utility/string_helper.h>
#include "version.h"

namespace proxima {
namespace be {

Status GrpcProximaSearchClient::connect(const ChannelOptions &options) {
  Status status;

  brpc::ChannelOptions brpc_options;
  brpc_options.protocol = "h2:grpc";
  brpc_options.timeout_ms = options.timeout_ms;
  brpc_options.max_retry = options.max_retry;

  int ret = client_channel_.Init("rr", &brpc_options);
  if (ret != 0) {
    status.code = ErrorCode_InitChannel;
    status.reason = "Init client channel failed";
    return status;
  }

  for (uint32_t i = 0; i < options.connection_count; i++) {
    auto *sub_channel = new brpc::Channel;
    brpc_options.connection_group =
        std::string("group").append(std::to_string(i));
    ret = sub_channel->Init(options.host.c_str(), &brpc_options);
    if (ret != 0) {
      status.code = ErrorCode_InitChannel;
      status.reason = "Init sub client channel failed.";
      return status;
    }

    ret = client_channel_.AddChannel(sub_channel, nullptr);
    if (ret != 0) {
      status.code = ErrorCode_InitChannel;
      status.reason = "Add sub channel failed.";
      return status;
    }
  }

  // check versions of client and server if matched first
  if (!check_server_version(&status)) {
    return status;
  }

  connected_ = true;
  return status;
}

Status GrpcProximaSearchClient::close() {
  // DO NOTHING NOW
  connected_ = false;
  return Status();
}

#define CHECK_CONNECTED()                 \
  if (!connected_) {                      \
    status.code = ErrorCode_NotConnected; \
    status.reason = "Not connected yet";  \
    return status;                        \
  }

#define RETURN_STATUS(cntl, response)  \
  if (cntl.Failed()) {                 \
    status.code = ErrorCode_RpcError;  \
    status.reason = cntl.ErrorText();  \
  } else {                             \
    status.code = response.code();     \
    status.reason = response.reason(); \
  }                                    \
  return status;


Status GrpcProximaSearchClient::create_collection(
    const CollectionConfig &config) {
  Status status;

  // Check connected
  CHECK_CONNECTED();

  // Validate request
  status = this->validate(config);
  if (status.code != 0) {
    return status;
  }

  // Prepare
  brpc::Controller cntl;
  proto::CollectionConfig request;
  this->convert(config, &request);
  proto::Status response;

  this->rpc_create_collection(&cntl, &request, &response);

  RETURN_STATUS(cntl, response);
}

Status GrpcProximaSearchClient::drop_collection(
    const std::string &collection_name) {
  Status status;

  // Check connected
  CHECK_CONNECTED();

  // Validate request
  if (collection_name.empty()) {
    status.code = ErrorCode_ValidateError;
    status.reason = "Collection name can't be empty";
    return status;
  }

  // Prepare
  brpc::Controller cntl;
  proto::Status response;
  proto::CollectionName request;
  request.set_collection_name(collection_name);

  this->rpc_drop_collection(&cntl, &request, &response);

  // Send rpc request
  RETURN_STATUS(cntl, response);
}

Status GrpcProximaSearchClient::describe_collection(
    const std::string &collection_name, CollectionInfo *collection_info) {
  Status status;

  // Check connected
  CHECK_CONNECTED();

  // Validate request
  if (collection_name.empty()) {
    status.code = ErrorCode_ValidateError;
    status.reason = "Collection name can't be empty";
    return status;
  }

  // Prepare
  brpc::Controller cntl;
  proto::DescribeCollectionResponse response;
  proto::CollectionName request;
  request.set_collection_name(collection_name);

  // Send rpc request
  this->rpc_describe_collection(&cntl, &request, &response);

  // Transform response
  if (!cntl.Failed() && response.status().code() == 0) {
    this->convert(response.collection(), collection_info);
  }

  RETURN_STATUS(cntl, response.status());
}

Status GrpcProximaSearchClient::stats_collection(
    const std::string &collection_name, CollectionStats *stats) {
  Status status;

  // Check connected
  CHECK_CONNECTED();

  // Validate request
  if (collection_name.empty()) {
    status.code = ErrorCode_ValidateError;
    status.reason = "Collection name can't be empty";
    return status;
  }

  // Prepare
  brpc::Controller cntl;
  proto::StatsCollectionResponse response;
  proto::CollectionName request;
  request.set_collection_name(collection_name);

  this->rpc_stats_collection(&cntl, &request, &response);

  // Transform response
  if (!cntl.Failed() && response.status().code() == 0) {
    this->convert(response.collection_stats(), stats);
  }

  RETURN_STATUS(cntl, response.status());
}

Status GrpcProximaSearchClient::list_collections(
    std::vector<CollectionInfo> *collections) {
  Status status;

  // Check connected
  CHECK_CONNECTED();

  // Prepare
  brpc::Controller cntl;
  proto::ListCollectionsResponse response;
  proto::ListCondition request;

  // Send rpc request
  this->rpc_list_collections(&cntl, &request, &response);

  // Transform response
  if (!cntl.Failed() && response.status().code() == 0) {
    for (int i = 0; i < response.collections_size(); i++) {
      CollectionInfo ci;
      this->convert(response.collections(i), &ci);
      collections->emplace_back(ci);
    }
  }

  RETURN_STATUS(cntl, response.status());
}

Status GrpcProximaSearchClient::write(const WriteRequest &write_request) {
  Status status;

  // Check connected
  CHECK_CONNECTED();

  // Validate
  auto &pb_req = (const PbWriteRequest &)write_request;
  status = this->validate(pb_req);
  if (status.code != 0) {
    return status;
  }

  // Prepare
  brpc::Controller cntl;
  proto::Status response;

  this->rpc_write(&cntl, pb_req.data(), &response);

  RETURN_STATUS(cntl, response);
}

Status GrpcProximaSearchClient::query(const QueryRequest &query_request,
                                      QueryResponse *query_response) {
  Status status;

  // Check connected
  CHECK_CONNECTED();

  // Validate
  auto &pb_req = (const PbQueryRequest &)query_request;
  auto *pb_resp = (PbQueryResponse *)query_response;

  status = this->validate(pb_req);
  if (status.code != 0) {
    return status;
  }

  // Prepare
  brpc::Controller cntl;

  this->rpc_query(&cntl, pb_req.data(), pb_resp->data());

  RETURN_STATUS(cntl, pb_resp->data()->status());
}

Status GrpcProximaSearchClient::get_document_by_key(
    const GetDocumentRequest &get_request, GetDocumentResponse *get_response) {
  Status status;

  // Check connected
  CHECK_CONNECTED();

  // Validate
  auto &pb_req = (const PbGetDocumentRequest &)get_request;
  auto *pb_resp = (PbGetDocumentResponse *)get_response;

  status = this->validate(pb_req);
  if (status.code != 0) {
    return status;
  }

  // Prepare
  brpc::Controller cntl;

  this->rpc_get_document_by_key(&cntl, pb_req.data(), pb_resp->data());

  RETURN_STATUS(cntl, pb_resp->data()->status());
}

bool GrpcProximaSearchClient::check_server_version(Status *status) {
  proto::ProximaService_Stub stub(&client_channel_);
  brpc::Controller cntl;
  proto::GetVersionRequest request;
  proto::GetVersionResponse response;

  stub.get_version(&cntl, &request, &response, nullptr);
  if (cntl.Failed()) {
    status->code = ErrorCode_RpcError;
    status->reason = cntl.ErrorText();
    return false;
  }

  if (response.status().code() != 0) {
    status->code = response.status().code();
    status->reason = response.status().reason();
  }

  std::string server_version = response.version();
  std::string client_version = Version::String();
  if (server_version == client_version) {
    return true;
  }

  // Temporarily we just use first two seq number of  version string to compare
  // For exp: version[0.1.2] match version[0.1.3] with "0.1"
  std::vector<std::string> server_sub_seqs;
  ailego::StringHelper::Split(server_version, '.', &server_sub_seqs);
  std::vector<std::string> client_sub_seqs;
  ailego::StringHelper::Split(client_version, '.', &client_sub_seqs);

  int compare_count = 2;
  for (int i = 0; i < compare_count; i++) {
    if (client_sub_seqs[i] != server_sub_seqs[i]) {
      status->code = ErrorCode_MismatchedVersion;
      status->reason = std::string()
                           .append("client version:")
                           .append(Version::String())
                           .append(" not match server version:")
                           .append(server_version);
      return false;
    }
  }

  return true;
}

void GrpcProximaSearchClient::convert(const CollectionConfig &config,
                                      proto::CollectionConfig *pb_request) {
  pb_request->set_collection_name(config.collection_name);
  pb_request->set_max_docs_per_segment(config.max_docs_per_segment);

  for (auto &it : config.forward_columns) {
    pb_request->add_forward_column_names(it);
  }

  for (auto &it : config.index_columns) {
    auto *param = pb_request->add_index_column_params();
    param->set_column_name(it.column_name);
    param->set_index_type((proto::IndexType)it.index_type);
    param->set_data_type((proto::DataType)it.data_type);
    param->set_dimension(it.dimension);

    for (auto &kv : it.extra_params) {
      auto *extra_param = param->add_extra_params();
      extra_param->set_key(kv.key);
      extra_param->set_value(kv.value);
    }
  }

  auto &input_repo = config.database_repository;
  if (!input_repo.repository_name.empty()) {
    auto *repo_config = pb_request->mutable_repository_config();
    repo_config->set_repository_type(
        proto::CollectionConfig::RepositoryConfig::RT_DATABASE);
    repo_config->set_repository_name(input_repo.repository_name);
    repo_config->mutable_database()->set_connection_uri(
        input_repo.connection_uri);
    repo_config->mutable_database()->set_table_name(input_repo.table_name);
    repo_config->mutable_database()->set_user(input_repo.user);
    repo_config->mutable_database()->set_password(input_repo.password);
  }
}

void GrpcProximaSearchClient::convert(const proto::CollectionInfo &pb_response,
                                      CollectionInfo *collection_info) {
  collection_info->collection_name = pb_response.config().collection_name();
  collection_info->collection_status =
      (CollectionInfo::CollectionStatus)pb_response.status();
  collection_info->collection_uuid = pb_response.uuid();
  collection_info->latest_lsn = pb_response.latest_lsn_context().lsn();
  collection_info->latest_lsn_context =
      pb_response.latest_lsn_context().context();
  collection_info->magic_number = pb_response.magic_number();
  collection_info->max_docs_per_segment =
      pb_response.config().max_docs_per_segment();

  // copy forward columns
  for (int i = 0; i < pb_response.config().forward_column_names_size(); i++) {
    collection_info->forward_columns.emplace_back(
        pb_response.config().forward_column_names(i));
  }

  // copy index columns
  for (int i = 0; i < pb_response.config().index_column_params_size(); i++) {
    auto &rp = pb_response.config().index_column_params(i);
    IndexColumnParam index_param;
    index_param.column_name = rp.column_name();
    index_param.index_type = (IndexType)rp.index_type();
    index_param.data_type = (DataType)rp.data_type();
    index_param.dimension = rp.dimension();
    for (int j = 0; j < rp.extra_params_size(); j++) {
      KVPair pair;
      pair.key = rp.extra_params(j).key();
      pair.value = rp.extra_params(j).value();
      index_param.extra_params.emplace_back(pair);
    }
    collection_info->index_columns.emplace_back(index_param);
  }

  // copy database repository
  if (pb_response.config().has_repository_config() &&
      pb_response.config().repository_config().repository_type() ==
          proto::CollectionConfig::RepositoryConfig::RT_DATABASE) {
    auto &rc = pb_response.config().repository_config();
    collection_info->database_repository.repository_name = rc.repository_name();
    collection_info->database_repository.connection_uri =
        rc.database().connection_uri();
    collection_info->database_repository.table_name =
        rc.database().table_name();
    collection_info->database_repository.user = rc.database().user();
    collection_info->database_repository.password = rc.database().password();
  }
}

void GrpcProximaSearchClient::convert(const proto::CollectionStats &pb_response,
                                      CollectionStats *collection_stats) {
  collection_stats->collection_name = pb_response.collection_name();
  collection_stats->total_doc_count = pb_response.total_doc_count();
  collection_stats->total_segment_count = pb_response.total_segment_count();
  collection_stats->total_index_file_count =
      pb_response.total_index_file_count();
  collection_stats->total_index_file_size = pb_response.total_index_file_size();

  for (int i = 0; i < pb_response.segment_stats_size(); i++) {
    auto &ss = pb_response.segment_stats(i);
    CollectionStats::SegmentStats segment_stats;
    segment_stats.segment_id = ss.segment_id();
    segment_stats.segment_state = (CollectionStats::SegmentState)ss.state();
    segment_stats.doc_count = ss.doc_count();
    segment_stats.index_file_count = ss.index_file_count();
    segment_stats.index_file_size = ss.index_file_size();
    segment_stats.min_doc_id = ss.min_doc_id();
    segment_stats.max_doc_id = ss.max_doc_id();
    segment_stats.min_primary_key = ss.min_primary_key();
    segment_stats.max_primary_key = ss.max_primary_key();
    segment_stats.min_timestamp = ss.min_timestamp();
    segment_stats.max_timestamp = ss.max_timestamp();
    segment_stats.min_lsn = ss.min_lsn();
    segment_stats.max_lsn = ss.max_lsn();
    collection_stats->segment_stats.emplace_back(segment_stats);
  }
}

Status GrpcProximaSearchClient::validate(const CollectionConfig &config) {
  Status status;
  if (config.collection_name.empty()) {
    status.code = ErrorCode_ValidateError;
    status.reason = "Collection name can't be empty";
    return status;
  }

  if (config.index_columns.size() == 0) {
    status.code = ErrorCode_ValidateError;
    status.reason = "Index columns can't be empty";
    return status;
  }

  for (auto &index_column : config.index_columns) {
    if (index_column.column_name.empty()) {
      status.code = ErrorCode_ValidateError;
      status.reason = "Column name can't be empty";
      return status;
    }

    if (index_column.dimension == 0U) {
      status.code = ErrorCode_ValidateError;
      status.reason = "Dimension can't be 0";
      return status;
    }

    if (index_column.data_type == DataType::UNDEFINED) {
      status.code = ErrorCode_ValidateError;
      status.reason = "Data type can't be undefined";
      return status;
    }
  }
  return status;
}

Status GrpcProximaSearchClient::validate(const PbWriteRequest &request) {
  Status status;
  auto *wreq = request.data();
  if (!wreq) {
    status.code = ErrorCode_ValidateError;
    status.reason = "Invalid write request";
    return status;
  }

  if (wreq->collection_name().empty()) {
    status.code = ErrorCode_ValidateError;
    status.reason = "Collection name can't be empty";
    return status;
  }

  if (wreq->rows_size() == 0) {
    status.code = ErrorCode_ValidateError;
    status.reason = "Rows can't be empty";
    return status;
  }

  for (int i = 0; i < wreq->rows_size(); i++) {
    auto &row = wreq->rows(i);
    if (row.operation_type() == proto::OperationType::OP_INSERT ||
        row.operation_type() == proto::OperationType::OP_UPDATE) {
      if (row.index_column_values().values_size() !=
          wreq->row_meta().index_column_metas_size()) {
        status.code = ErrorCode_ValidateError;
        status.reason = "Index columns not match values";
        return status;
      }

      if (row.forward_column_values().values_size() !=
          wreq->row_meta().forward_column_names_size()) {
        status.code = ErrorCode_ValidateError;
        status.reason = "Forward columns not match values";
        return status;
      }
    }
  }

  return status;
}

Status GrpcProximaSearchClient::validate(const PbQueryRequest &request) {
  Status status;
  auto *qreq = request.data();
  if (!qreq) {
    status.code = ErrorCode_ValidateError;
    status.reason = "Invalid query request.";
    return status;
  }

  if (qreq->collection_name().empty()) {
    status.code = ErrorCode_ValidateError;
    status.reason = "Collection name can't be empty";
    return status;
  }

  if (qreq->knn_param().column_name().empty()) {
    status.code = ErrorCode_ValidateError;
    status.reason = "Knn param column name can't be empty";
    return status;
  }

  if (qreq->knn_param().topk() == 0U) {
    status.code = ErrorCode_ValidateError;
    status.reason = "Knn param topk can't be 0";
    return status;
  }

  if (qreq->knn_param().features().empty() &&
      qreq->knn_param().matrix().empty()) {
    status.code = ErrorCode_ValidateError;
    status.reason =
        "Knn param features and matrix can't be empty at the same time";
    return status;
  }

  if (qreq->knn_param().batch_count() == 0U) {
    status.code = ErrorCode_ValidateError;
    status.reason = "Knn param batch count can't be 0";
    return status;
  }

  if (qreq->knn_param().dimension() == 0U) {
    status.code = ErrorCode_ValidateError;
    status.reason = "Knn param dimension can't be 0";
    return status;
  }

  if (qreq->knn_param().data_type() == proto::DataType::DT_UNDEFINED) {
    status.code = ErrorCode_ValidateError;
    status.reason = "Knn param data type can't be undefined";
    return status;
  }

  return status;
}

Status GrpcProximaSearchClient::validate(const PbGetDocumentRequest &request) {
  Status status;
  auto *gdreq = request.data();
  if (!gdreq) {
    status.code = ErrorCode_ValidateError;
    status.reason = "Invalid get document request";
    return status;
  }

  if (gdreq->collection_name().empty()) {
    status.code = ErrorCode_ValidateError;
    status.reason = "Collection name can't be empty";
    return status;
  }

  return status;
}

void GrpcProximaSearchClient::rpc_create_collection(
    brpc::Controller *cntl, const proto::CollectionConfig *request,
    proto::Status *response) {
  proto::ProximaService_Stub stub(&client_channel_);
  stub.create_collection(cntl, request, response, nullptr);
}

void GrpcProximaSearchClient::rpc_drop_collection(
    brpc::Controller *cntl, const proto::CollectionName *request,
    proto::Status *response) {
  proto::ProximaService_Stub stub(&client_channel_);
  stub.drop_collection(cntl, request, response, nullptr);
}

void GrpcProximaSearchClient::rpc_describe_collection(
    brpc::Controller *cntl, const proto::CollectionName *request,
    proto::DescribeCollectionResponse *response) {
  proto::ProximaService_Stub stub(&client_channel_);
  stub.describe_collection(cntl, request, response, nullptr);
}

void GrpcProximaSearchClient::rpc_stats_collection(
    brpc::Controller *cntl, const proto::CollectionName *request,
    proto::StatsCollectionResponse *response) {
  proto::ProximaService_Stub stub(&client_channel_);
  stub.stats_collection(cntl, request, response, nullptr);
}

void GrpcProximaSearchClient::rpc_list_collections(
    brpc::Controller *cntl, const proto::ListCondition *request,
    proto::ListCollectionsResponse *response) {
  proto::ProximaService_Stub stub(&client_channel_);
  stub.list_collections(cntl, request, response, nullptr);
}

void GrpcProximaSearchClient::rpc_write(brpc::Controller *cntl,
                                        const proto::WriteRequest *request,
                                        proto::Status *response) {
  proto::ProximaService_Stub stub(&client_channel_);
  stub.write(cntl, request, response, nullptr);
}

void GrpcProximaSearchClient::rpc_query(brpc::Controller *cntl,
                                        const proto::QueryRequest *request,
                                        proto::QueryResponse *response) {
  proto::ProximaService_Stub stub(&client_channel_);
  stub.query(cntl, request, response, nullptr);
}

void GrpcProximaSearchClient::rpc_get_document_by_key(
    brpc::Controller *cntl, const proto::GetDocumentRequest *request,
    proto::GetDocumentResponse *response) {
  proto::ProximaService_Stub stub(&client_channel_);
  stub.get_document_by_key(cntl, request, response, nullptr);
}


#undef CHECK_CONNECTED
#undef RETURN_STATUS

}  // namespace be
}  // end namespace proxima
