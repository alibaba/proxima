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
 *   \brief    Implementation with http protocol
 */

#include "http_client.h"
#include <iostream>
#include <ailego/utility/string_helper.h>
#include "common/protobuf_helper.h"
#include "version.h"

namespace proxima {
namespace be {

Status HttpProximaSearchClient::connect(const ChannelOptions &options) {
  Status status;

  brpc::ChannelOptions brpc_options;
  brpc_options.protocol = "http";
  brpc_options.timeout_ms = options.timeout_ms;
  brpc_options.max_retry = options.max_retry;

  http_host_ = std::string("http://").append(options.host);
  int ret = client_channel_.Init(http_host_.c_str(), "", &brpc_options);
  if (ret != 0) {
    status.code = ErrorCode_InitChannel;
    status.reason = "Init client channel failed";
    return status;
  }

  // check versions of client and server if matched first
  status = check_server_version();
  if (status.code != 0) {
    return status;
  }

  connected_ = true;
  return status;
}

Status HttpProximaSearchClient::close() {
  // DO NOTHING NOW
  connected_ = false;
  return Status();
}

void HttpProximaSearchClient::rpc_create_collection(
    brpc::Controller *cntl, const proto::CollectionConfig *request,
    proto::Status *response) {
  std::string url;
  url.append(http_host_)
      .append("/v1/collection/")
      .append(request->collection_name());

  std::string json_body;
  ProtobufHelper::MessageToJson(*request, &json_body);

  cntl->http_request().uri() = url;
  cntl->http_request().set_method(brpc::HTTP_METHOD_POST);
  cntl->request_attachment().append(json_body);
  client_channel_.CallMethod(nullptr, cntl, nullptr, nullptr, nullptr);

  if (!cntl->Failed()) {
    ProtobufHelper::JsonToMessage(cntl->response_attachment().to_string(),
                                  response);
  }
}

void HttpProximaSearchClient::rpc_drop_collection(
    brpc::Controller *cntl, const proto::CollectionName *request,
    proto::Status *response) {
  std::string url;
  url.append(http_host_)
      .append("/v1/collection/")
      .append(request->collection_name());
  cntl->http_request().uri() = url;
  cntl->http_request().set_method(brpc::HTTP_METHOD_DELETE);
  client_channel_.CallMethod(nullptr, cntl, nullptr, nullptr, nullptr);

  if (!cntl->Failed()) {
    ProtobufHelper::JsonToMessage(cntl->response_attachment().to_string(),
                                  response);
  }
}

void HttpProximaSearchClient::rpc_describe_collection(
    brpc::Controller *cntl, const proto::CollectionName *request,
    proto::DescribeCollectionResponse *response) {
  std::string url;
  url.append(http_host_)
      .append("/v1/collection/")
      .append(request->collection_name());
  cntl->http_request().uri() = url;
  cntl->http_request().set_method(brpc::HTTP_METHOD_GET);
  client_channel_.CallMethod(nullptr, cntl, nullptr, nullptr, nullptr);

  if (!cntl->Failed()) {
    ProtobufHelper::JsonToMessage(cntl->response_attachment().to_string(),
                                  response);
  }
}


void HttpProximaSearchClient::rpc_stats_collection(
    brpc::Controller *cntl, const proto::CollectionName *request,
    proto::StatsCollectionResponse *response) {
  std::string url;
  url.append(http_host_)
      .append("/v1/collection/")
      .append(request->collection_name())
      .append("/stats");
  cntl->http_request().uri() = url;
  cntl->http_request().set_method(brpc::HTTP_METHOD_GET);
  client_channel_.CallMethod(nullptr, cntl, nullptr, nullptr, nullptr);

  if (!cntl->Failed()) {
    ProtobufHelper::JsonToMessage(cntl->response_attachment().to_string(),
                                  response);
  }
}

void HttpProximaSearchClient::rpc_list_collections(
    brpc::Controller *cntl, const proto::ListCondition * /* request */,
    proto::ListCollectionsResponse *response) {
  std::string url;
  url.append(http_host_).append("/v1/collections");
  cntl->http_request().uri() = url;
  cntl->http_request().set_method(brpc::HTTP_METHOD_GET);
  client_channel_.CallMethod(nullptr, cntl, nullptr, nullptr, nullptr);

  if (!cntl->Failed()) {
    ProtobufHelper::JsonToMessage(cntl->response_attachment().to_string(),
                                  response);
  }
}

void HttpProximaSearchClient::rpc_write(brpc::Controller *cntl,
                                        const proto::WriteRequest *request,
                                        proto::Status *response) {
  std::string url;
  url.append(http_host_)
      .append("/v1/collection/")
      .append(request->collection_name())
      .append("/index");

  std::string json_body;
  ProtobufHelper::MessageToJson(*request, &json_body);

  cntl->http_request().uri() = url;
  cntl->http_request().set_method(brpc::HTTP_METHOD_POST);
  cntl->request_attachment().append(json_body);
  client_channel_.CallMethod(nullptr, cntl, nullptr, nullptr, nullptr);

  if (!cntl->Failed()) {
    ProtobufHelper::JsonToMessage(cntl->response_attachment().to_string(),
                                  response);
  }
}

void HttpProximaSearchClient::rpc_query(brpc::Controller *cntl,
                                        const proto::QueryRequest *request,
                                        proto::QueryResponse *response) {
  std::string url;
  url.append(http_host_)
      .append("/v1/collection/")
      .append(request->collection_name())
      .append("/query");

  std::string json_body;
  ProtobufHelper::MessageToJson(*request, &json_body);

  cntl->http_request().uri() = url;
  cntl->http_request().set_method(brpc::HTTP_METHOD_POST);
  cntl->request_attachment().append(json_body);
  client_channel_.CallMethod(nullptr, cntl, nullptr, nullptr, nullptr);

  if (!cntl->Failed()) {
    ProtobufHelper::JsonToMessage(cntl->response_attachment().to_string(),
                                  response);
  }
}

void HttpProximaSearchClient::rpc_get_document_by_key(
    brpc::Controller *cntl, const proto::GetDocumentRequest *request,
    proto::GetDocumentResponse *response) {
  std::string url;
  url.append(http_host_)
      .append("/v1/collection/")
      .append(request->collection_name())
      .append("/doc?key=")
      .append(std::to_string(request->primary_key()));

  cntl->http_request().uri() = url;
  cntl->http_request().set_method(brpc::HTTP_METHOD_GET);
  client_channel_.CallMethod(nullptr, cntl, nullptr, nullptr, nullptr);

  if (!cntl->Failed()) {
    ProtobufHelper::JsonToMessage(cntl->response_attachment().to_string(),
                                  response);
  }
}

Status HttpProximaSearchClient::check_server_version() {
  Status status;
  std::string url;
  url.append(http_host_).append("/service_version");

  brpc::Controller cntl;
  cntl.http_request().uri() = url;
  cntl.http_request().set_method(brpc::HTTP_METHOD_GET);
  client_channel_.CallMethod(nullptr, &cntl, nullptr, nullptr, nullptr);
  if (cntl.Failed()) {
    status.code = ErrorCode_RpcError;
    status.reason = cntl.ErrorText();
    return status;
  }

  proto::GetVersionResponse resp;
  ProtobufHelper::JsonToMessage(cntl.response_attachment().to_string(), &resp);
  std::string server_version = resp.version();
  std::string client_version = Version::String();
  if (server_version == client_version) {
    return status;
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
      status.code = ErrorCode_MismatchedVersion;
      status.reason = std::string()
                          .append("client version:")
                          .append(Version::String())
                          .append(" not match server version:")
                          .append(server_version);
      return status;
    }
  }

  return status;
}

}  // namespace be
}  // namespace proxima
