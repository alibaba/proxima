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
 *   \brief    Implementation of pb service interface, it will be registered
 *             to http/grpc server for rpc service
 */

#pragma once

#include <brpc/server.h>
#include "admin/admin_agent.h"
#include "agent/index_agent.h"
#include "proto/proxima_be.pb.h"
#include "query/query_agent.h"

namespace proxima {
namespace be {
namespace server {

class ProximaRequestHandler : public proto::ProximaService,
                              public proto::HttpProximaService {
 public:
  ProximaRequestHandler(const agent::IndexAgentPtr &index_agent,
                        const query::QueryAgentPtr &query_agent,
                        const admin::AdminAgentPtr &admin_agent);

  ~ProximaRequestHandler() = default;

 public:
  void create_collection(::google::protobuf::RpcController *controller,
                         const proto::CollectionConfig *request,
                         proto::Status *response,
                         ::google::protobuf::Closure *done) override;


  void drop_collection(::google::protobuf::RpcController *controller,
                       const proto::CollectionName *request,
                       proto::Status *response,
                       ::google::protobuf::Closure *done) override;

  void describe_collection(::google::protobuf::RpcController *controller,
                           const proto::CollectionName *request,
                           proto::DescribeCollectionResponse *response,
                           ::google::protobuf::Closure *done) override;

  void list_collections(::google::protobuf::RpcController *controller,
                        const proto::ListCondition *request,
                        proto::ListCollectionsResponse *response,
                        ::google::protobuf::Closure *done) override;

  void stats_collection(::google::protobuf::RpcController *controller,
                        const proto::CollectionName *request,
                        proto::StatsCollectionResponse *response,
                        ::google::protobuf::Closure *done) override;

  void write(::google::protobuf::RpcController *controller,
             const proto::WriteRequest *request, proto::Status *response,
             ::google::protobuf::Closure *done) override;

  void query(::google::protobuf::RpcController *controller,
             const proto::QueryRequest *request, proto::QueryResponse *response,
             ::google::protobuf::Closure *done) override;

  void get_document_by_key(::google::protobuf::RpcController *controller,
                           const proto::GetDocumentRequest *request,
                           proto::GetDocumentResponse *response,
                           ::google::protobuf::Closure *done) override;

  void get_version(::google::protobuf::RpcController *controller,
                   const proto::GetVersionRequest *request,
                   proto::GetVersionResponse *response,
                   ::google::protobuf::Closure *done) override;


 public:
  // Restful apis from HttpProximaService
  void collection(::google::protobuf::RpcController *controller,
                  const proto::HttpRequest *request,
                  proto::HttpResponse *response,
                  ::google::protobuf::Closure *done) override;

  void stats_collection(::google::protobuf::RpcController *controller,
                        const proto::HttpRequest *request,
                        proto::HttpResponse *response,
                        ::google::protobuf::Closure *done) override;

  void write(::google::protobuf::RpcController *controller,
             const proto::HttpRequest *request, proto::HttpResponse *response,
             ::google::protobuf::Closure *done) override;

  void query(::google::protobuf::RpcController *controller,
             const proto::HttpRequest *request, proto::HttpResponse *response,
             ::google::protobuf::Closure *done) override;

  void get_document_by_key(::google::protobuf::RpcController *controller,
                           const proto::HttpRequest *request,
                           proto::HttpResponse *response,
                           ::google::protobuf::Closure *done) override;

  void list_collections(::google::protobuf::RpcController *controller,
                        const proto::HttpRequest *request,
                        proto::HttpResponse *response,
                        ::google::protobuf::Closure *done) override;

  void get_version(::google::protobuf::RpcController *controller,
                   const proto::HttpRequest *request,
                   proto::HttpResponse *response,
                   ::google::protobuf::Closure *done) override;

 public:
  void set_version(const std::string &val) {
    version_ = val;
  }

 private:
  int write_impl(const proto::WriteRequest &request, proto::Status *response);

  void create_collection(brpc::Controller *controller);

  void drop_collection(brpc::Controller *controller);

  void describe_collection(brpc::Controller *controller);

  int parse_collection(brpc::Controller *controller, std::string *collection);

 private:
  std::string version_{};
  agent::IndexAgentPtr index_agent_{};
  query::QueryAgentPtr query_agent_{};
  admin::AdminAgentPtr admin_agent_{};
};

}  // end namespace server
}  // namespace be
}  // end namespace proxima
