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

#pragma once

#include "grpc_client.h"

namespace proxima {
namespace be {

/*
 * HttpProximaSearchClient communicate to BE server with http protocol
 */
class HttpProximaSearchClient : public GrpcProximaSearchClient {
 public:
  //! Constructor
  HttpProximaSearchClient() = default;

  //! Destructor
  ~HttpProximaSearchClient() override = default;

  //! Connect remote server
  Status connect(const ChannelOptions &options) override;

  //! Close connection to remote server
  Status close() override;

  /// Other functions inherit from GrpcProximaSearchClient
 protected:
  void rpc_create_collection(brpc::Controller *cntl,
                             const proto::CollectionConfig *request,
                             proto::Status *response) override;

  void rpc_drop_collection(brpc::Controller *cntl,
                           const proto::CollectionName *request,
                           proto::Status *response) override;

  void rpc_describe_collection(
      brpc::Controller *cntl, const proto::CollectionName *request,
      proto::DescribeCollectionResponse *response) override;

  void rpc_stats_collection(brpc::Controller *cntl,
                            const proto::CollectionName *request,
                            proto::StatsCollectionResponse *response) override;

  void rpc_list_collections(brpc::Controller *cntl,
                            const proto::ListCondition *request,
                            proto::ListCollectionsResponse *response) override;

  void rpc_write(brpc::Controller *cntl, const proto::WriteRequest *request,
                 proto::Status *response) override;

  void rpc_query(brpc::Controller *cntl, const proto::QueryRequest *request,
                 proto::QueryResponse *response) override;

  void rpc_get_document_by_key(brpc::Controller *cntl,
                               const proto::GetDocumentRequest *request,
                               proto::GetDocumentResponse *response) override;

 private:
  Status check_server_version();

 private:
  brpc::Channel client_channel_{};
  std::string http_host_{};
};

}  // namespace be
}  // end namespace proxima
