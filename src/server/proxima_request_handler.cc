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
 *   \brief    Implementation of proxima request handler
 */

#include "proxima_request_handler.h"
#include <ailego/encoding/json.h>
#include <ailego/utility/string_helper.h>
#include <brpc/server.h>

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
#include <google/protobuf/util/json_util.h>
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

#include "agent/write_request.h"
#include "common/error_code.h"
#include "common/protobuf_helper.h"
#include "metrics/metrics.h"
#include "write_request_builder.h"


namespace proxima {
namespace be {
namespace server {

using metrics::ProtocolType;

namespace {

static void inline SetStatus(int ret, proto::Status *status) {
  status->set_code(ret);
  status->set_reason(ErrorCode::What(ret));
}

//! Parse collection meta from json string
static inline int ParseRequestFromJson(const std::string &json_str,
                                       google::protobuf::Message *meta) {
  ProtobufHelper::JsonParseOptions options;
  // ignore params which can't be automatic parse from json
  options.ignore_unknown_fields = true;

  if (!ProtobufHelper::JsonToMessage(json_str, options, meta)) {
    LOG_ERROR("ParseRequestFromJson failed. json[%s]", json_str.c_str());
    return ErrorCode_InvalidArgument;
  }
  return 0;
}

//! Serialize response to controller
static inline void SerializeResponse(const google::protobuf::Message &response,
                                     brpc::Controller *brpc_controller) {
  brpc_controller->http_response().set_content_type("application/json");
  std::string json_resp;
  if (!ProtobufHelper::MessageToJson(response, &json_resp)) {
    LOG_ERROR("Can't serialize PB response to json. message[%s]",
              response.ShortDebugString().c_str());
  } else {
    brpc_controller->response_attachment().append(json_resp);
  }
}

static inline void UnknownMethod(brpc::Controller *controller,
                                 int allowed_method,
                                 google::protobuf::Message *rsp,
                                 proto::Status *status) {
  SetStatus(PROXIMA_BE_ERROR_CODE(InvalidQuery), status);
  controller->http_response().set_status_code(
      brpc::HTTP_STATUS_METHOD_NOT_ALLOWED);
  status->mutable_reason()->append(": invalid http method");
  const char *allowed = nullptr;
  switch (allowed_method) {
    case brpc::HTTP_METHOD_POST:
      allowed = "POST";
      break;
    case brpc::HTTP_METHOD_GET:
      allowed = "GET";
      break;
    case brpc::HTTP_METHOD_PUT:
      allowed = "PUT";
      break;
    case brpc::HTTP_METHOD_DELETE:
      allowed = "DELETE";
      break;
      // default ignore
  }
  if (allowed) {
    controller->http_response().SetHeader("Allowed", allowed);
  }
  SerializeResponse(*rsp, controller);
}

}  // namespace

#define RETURN_IF_NOT_HTTP_METHOD(CONTROLLER, METHOD, RSP, STATUS) \
  if (CONTROLLER->http_request().method() != METHOD) {             \
    UnknownMethod(CONTROLLER, METHOD, RSP, STATUS);                \
    return;                                                        \
  }

ProximaRequestHandler::ProximaRequestHandler(
    const agent::IndexAgentPtr &p_index_agent,
    const query::QueryAgentPtr &p_query_agent,
    const admin::AdminAgentPtr &p_admin_agent)
    : index_agent_(p_index_agent),
      query_agent_(p_query_agent),
      admin_agent_(p_admin_agent) {}


void ProximaRequestHandler::create_collection(
    ::google::protobuf::RpcController * /*controller*/,
    const proto::CollectionConfig *request, proto::Status *response,
    ::google::protobuf::Closure *done) {
  brpc::ClosureGuard done_guard(done);
  int ret = admin_agent_->create_collection(*request);
  SetStatus(ret, response);
}

void ProximaRequestHandler::drop_collection(
    ::google::protobuf::RpcController * /*controller*/,
    const proto::CollectionName *request, proto::Status *response,
    ::google::protobuf::Closure *done) {
  brpc::ClosureGuard done_guard(done);
  int ret = admin_agent_->drop_collection(request->collection_name());
  SetStatus(ret, response);
}

void ProximaRequestHandler::describe_collection(
    ::google::protobuf::RpcController * /*controller*/,
    const proto::CollectionName *request,
    proto::DescribeCollectionResponse *response,
    ::google::protobuf::Closure *done) {
  brpc::ClosureGuard done_guard(done);
  int ret =
      admin_agent_->describe_collection(request->collection_name(), response);
  SetStatus(ret, response->mutable_status());
}

void ProximaRequestHandler::list_collections(
    ::google::protobuf::RpcController * /*controller*/,
    const proto::ListCondition *request,
    proto::ListCollectionsResponse *response,
    ::google::protobuf::Closure *done) {
  brpc::ClosureGuard done_guard(done);
  int ret = admin_agent_->list_collections(*request, response);
  SetStatus(ret, response->mutable_status());
}

void ProximaRequestHandler::stats_collection(
    ::google::protobuf::RpcController * /*controller*/,
    const proto::CollectionName *request,
    proto::StatsCollectionResponse *response,
    ::google::protobuf::Closure *done) {
  brpc::ClosureGuard done_guard(done);
  int ret =
      admin_agent_->stats_collection(request->collection_name(), response);
  SetStatus(ret, response->mutable_status());
}

void ProximaRequestHandler::write(
    ::google::protobuf::RpcController * /*controller*/,
    const proto::WriteRequest *request, proto::Status *response,
    ::google::protobuf::Closure *done) {
  int code = 0;
  metrics::WriteMetrics metrics{ProtocolType::kGrpc, &code};
  metrics.update_with_write_request(*request);
  LOG_DEBUG("%s", request->ShortDebugString().c_str());
  brpc::ClosureGuard done_guard(done);
  code = this->write_impl(*request, response);
}

void ProximaRequestHandler::query(::google::protobuf::RpcController *,
                                  const proto::QueryRequest *request,
                                  proto::QueryResponse *response,
                                  ::google::protobuf::Closure *done) {
  brpc::ClosureGuard done_guard(done);
  ailego::ElapsedTime latency;
  int code = 0;
  metrics::QueryMetrics metrics{metrics::ProtocolType::kGrpc, &code};
  metrics.update_with_query_request(*request);

  code = query_agent_->search(request, response);
  if (code != 0) {
    LOG_ERROR("Can't handle query. code[%d] what[%s]", code,
              ErrorCode::What(code));
  }

  response->set_latency_us(latency.micro_seconds());
  SetStatus(code, response->mutable_status());
}

void ProximaRequestHandler::get_document_by_key(
    ::google::protobuf::RpcController * /*controller*/,
    const proto::GetDocumentRequest *request,
    proto::GetDocumentResponse *response, ::google::protobuf::Closure *done) {
  brpc::ClosureGuard done_guard(done);
  int code = 0;
  metrics::GetDocumentMetrics metrics{ProtocolType::kGrpc, &code};
  code = query_agent_->search_by_key(request, response);
  if (code != 0) {
    LOG_ERROR("Can't handle query. code[%d] what[%s]", code,
              ErrorCode::What(code));
  }
  SetStatus(code, response->mutable_status());
}

void ProximaRequestHandler::get_version(
    ::google::protobuf::RpcController * /* controller */,
    const proto::GetVersionRequest * /* request */,
    proto::GetVersionResponse *response, ::google::protobuf::Closure *done) {
  brpc::ClosureGuard done_guard(done);
  response->set_version(version_);
  SetStatus(0, response->mutable_status());
}

void ProximaRequestHandler::collection(
    ::google::protobuf::RpcController *controller,
    const proto::HttpRequest * /*request*/, proto::HttpResponse * /*response*/,
    ::google::protobuf::Closure *done) {
  brpc::ClosureGuard done_guard(done);
  auto *brpc_controller = dynamic_cast<brpc::Controller *>(controller);
  auto method = brpc_controller->http_request().method();
  if (method == brpc::HttpMethod::HTTP_METHOD_POST) {
    create_collection(brpc_controller);
  } else if (method == brpc::HttpMethod::HTTP_METHOD_GET) {
    describe_collection(brpc_controller);
  } else if (method == brpc::HttpMethod::HTTP_METHOD_DELETE) {
    drop_collection(brpc_controller);
  } else {
    proto::Status status;
    SetStatus(PROXIMA_BE_ERROR_CODE(InvalidQuery), &status);
    status.set_reason(": invalid http method");
    brpc_controller->http_response().set_status_code(
        brpc::HTTP_STATUS_METHOD_NOT_ALLOWED);
    SerializeResponse(status, brpc_controller);
  }
}

void ProximaRequestHandler::stats_collection(
    ::google::protobuf::RpcController *controller,
    const proto::HttpRequest * /*request*/, proto::HttpResponse * /*response*/,
    ::google::protobuf::Closure *done) {
  brpc::ClosureGuard done_guard(done);
  auto *brpc_controller = dynamic_cast<brpc::Controller *>(controller);

  proto::StatsCollectionResponse pb_response;
  RETURN_IF_NOT_HTTP_METHOD(brpc_controller, brpc::HTTP_METHOD_GET,
                            &pb_response, pb_response.mutable_status())
  std::string collection_name;
  int code = parse_collection(brpc_controller, &collection_name);
  if (code == 0) {
    code = admin_agent_->stats_collection(collection_name, &pb_response);
  }
  SetStatus(code, pb_response.mutable_status());
  SerializeResponse(pb_response, brpc_controller);
}

void ProximaRequestHandler::write(::google::protobuf::RpcController *controller,
                                  const proto::HttpRequest * /*request*/,
                                  proto::HttpResponse * /*response*/,
                                  ::google::protobuf::Closure *done) {
  int code = 0;
  metrics::WriteMetrics metrics{ProtocolType::kHttp, &code};
  brpc::ClosureGuard done_guard(done);

  auto *brpc_controller = dynamic_cast<brpc::Controller *>(controller);
  proto::Status status;
  RETURN_IF_NOT_HTTP_METHOD(brpc_controller, brpc::HTTP_METHOD_POST, &status,
                            &status)

  std::string collection_name;
  code = parse_collection(brpc_controller, &collection_name);
  if (code == 0) {
    const std::string http_body =
        brpc_controller->request_attachment().to_string();
    proto::WriteRequest pb_request;
    code = ParseRequestFromJson(http_body, &pb_request);
    if (code == 0) {
      pb_request.set_collection_name(collection_name);
      metrics.update_with_write_request(pb_request);
      code = this->write_impl(pb_request, &status);
    } else {
      SetStatus(code, &status);
    }
  }

  SerializeResponse(status, brpc_controller);
}

void ProximaRequestHandler::query(::google::protobuf::RpcController *controller,
                                  const proto::HttpRequest * /*request*/,
                                  proto::HttpResponse * /*response*/,
                                  ::google::protobuf::Closure *done) {
  brpc::ClosureGuard done_guard(done);
  ailego::ElapsedTime latency;
  int code = 0;
  metrics::QueryMetrics metrics{metrics::ProtocolType::kHttp, &code};
  auto *brpc_controller = dynamic_cast<brpc::Controller *>(controller);

  // Check http method
  proto::QueryResponse pb_response;
  RETURN_IF_NOT_HTTP_METHOD(brpc_controller, brpc::HTTP_METHOD_POST,
                            &pb_response, pb_response.mutable_status())

  std::string collection_name;
  code = parse_collection(brpc_controller, &collection_name);
  if (code == 0) {
    const std::string body = brpc_controller->request_attachment().to_string();
    proto::QueryRequest pb_request;
    code = ParseRequestFromJson(body, &pb_request);
    if (code == 0) {
      metrics.update_with_query_request(pb_request);
      pb_request.set_collection_name(collection_name);
      code = query_agent_->search(&pb_request, &pb_response);
      if (code != 0) {
        LOG_ERROR("Can't handle query. code[%d] what[%s]", code,
                  ErrorCode::What(code));
      }
    }
  }

  pb_response.set_latency_us(latency.micro_seconds());
  SetStatus(code, pb_response.mutable_status());
  SerializeResponse(pb_response, brpc_controller);
}

void ProximaRequestHandler::get_document_by_key(
    ::google::protobuf::RpcController *controller,
    const proto::HttpRequest * /*request*/, proto::HttpResponse * /*response*/,
    ::google::protobuf::Closure *done) {
  brpc::ClosureGuard done_guard(done);
  auto *brpc_controller = dynamic_cast<brpc::Controller *>(controller);

  int code = 0;
  metrics::GetDocumentMetrics metrics{ProtocolType::kHttp, &code};

  // Check http method
  proto::GetDocumentResponse pb_response;
  RETURN_IF_NOT_HTTP_METHOD(brpc_controller, brpc::HTTP_METHOD_GET,
                            &pb_response, pb_response.mutable_status())

  std::string collection_name;
  code = parse_collection(brpc_controller, &collection_name);
  if (code == 0) {
    proto::GetDocumentRequest pb_request;
    pb_request.set_collection_name(collection_name);
    auto *key = brpc_controller->http_request().uri().GetQuery("key");
    if (key) {
      pb_request.set_primary_key(std::strtoull(key->c_str(), nullptr, 10));
      code = query_agent_->search_by_key(&pb_request, &pb_response);
      if (code != 0) {
        LOG_ERROR("Can't handle query. code[%d] what[%s]", code,
                  ErrorCode::What(code));
      }
    } else {
      code = PROXIMA_BE_ERROR_CODE(InvalidArgument);
    }
  }

  SetStatus(code, pb_response.mutable_status());
  SerializeResponse(pb_response, brpc_controller);
}

void ProximaRequestHandler::list_collections(
    ::google::protobuf::RpcController *controller,
    const proto::HttpRequest * /*request*/, proto::HttpResponse * /*response*/,
    ::google::protobuf::Closure *done) {
  brpc::ClosureGuard done_guard(done);
  auto *brpc_controller = static_cast<brpc::Controller *>(controller);

  proto::ListCollectionsResponse pb_response;
  RETURN_IF_NOT_HTTP_METHOD(brpc_controller, brpc::HTTP_METHOD_GET,
                            &pb_response, pb_response.mutable_status())
  proto::ListCondition pb_request;
  auto *repo = brpc_controller->http_request().uri().GetQuery("repository");
  if (repo) {
    pb_request.set_repository_name(*repo);
  }
  int code = admin_agent_->list_collections(pb_request, &pb_response);
  SetStatus(code, pb_response.mutable_status());
  SerializeResponse(pb_response, brpc_controller);
}

void ProximaRequestHandler::get_version(
    ::google::protobuf::RpcController *controller,
    const proto::HttpRequest * /* request */,
    proto::HttpResponse * /* response */, ::google::protobuf::Closure *done) {
  brpc::ClosureGuard done_guard(done);
  auto *brpc_controller = dynamic_cast<brpc::Controller *>(controller);

  proto::GetVersionResponse pb_response;
  pb_response.set_version(version_);
  SetStatus(0, pb_response.mutable_status());
  SerializeResponse(pb_response, brpc_controller);
}

int ProximaRequestHandler::write_impl(const proto::WriteRequest &request,
                                      proto::Status *response) {
  auto &collection_name = request.collection_name();
  auto meta = index_agent_->get_collection_meta(collection_name);
  auto column_order = index_agent_->get_column_order(collection_name);
  if (!meta || !column_order) {
    SetStatus(ErrorCode_InexistentCollection, response);
    LOG_ERROR("Invalid collection. collection[%s]", collection_name.c_str());
    return ErrorCode_InexistentCollection;
  }

  agent::WriteRequest write_request;
  int code =
      WriteRequestBuilder::build(*meta, *column_order, request, &write_request);
  if (code != 0) {
    SetStatus(code, response);
    LOG_ERROR("Write request builder build failed. code[%d] collection[%s]",
              code, collection_name.c_str());
    return code;
  }

  code = index_agent_->write(write_request);
  if (code != 0) {
    LOG_ERROR("Index agent write request failed. code[%d] collection[%s]", code,
              collection_name.c_str());
  }

  SetStatus(code, response);
  return code;
}

void ProximaRequestHandler::create_collection(brpc::Controller *controller) {
  proto::Status pb_response;
  RETURN_IF_NOT_HTTP_METHOD(controller, brpc::HTTP_METHOD_POST, &pb_response,
                            &pb_response)
  const std::string &http_body = controller->request_attachment().to_string();
  proto::CollectionConfig pb_request;

  int code = ParseRequestFromJson(http_body, &pb_request);
  if (code == 0) {
    if (pb_request.collection_name().empty()) {
      pb_request.set_collection_name(
          controller->http_request().unresolved_path());
    }
    code = admin_agent_->create_collection(pb_request);
  }

  SetStatus(code, &pb_response);
  SerializeResponse(pb_response, controller);
}

void ProximaRequestHandler::describe_collection(brpc::Controller *controller) {
  proto::DescribeCollectionResponse pb_response;
  RETURN_IF_NOT_HTTP_METHOD(controller, brpc::HTTP_METHOD_GET, &pb_response,
                            pb_response.mutable_status())
  const std::string &collection_name =
      controller->http_request().unresolved_path();
  int code = admin_agent_->describe_collection(collection_name, &pb_response);
  SetStatus(code, pb_response.mutable_status());
  SerializeResponse(pb_response, controller);
}

void ProximaRequestHandler::drop_collection(brpc::Controller *controller) {
  proto::Status pb_response;
  RETURN_IF_NOT_HTTP_METHOD(controller, brpc::HTTP_METHOD_DELETE, &pb_response,
                            &pb_response)
  const std::string &collection_name =
      controller->http_request().unresolved_path();
  int code = admin_agent_->drop_collection(collection_name);

  SetStatus(code, &pb_response);
  SerializeResponse(pb_response, controller);
}

int ProximaRequestHandler::parse_collection(brpc::Controller *controller,
                                            std::string *collection_name) {
  auto &path = controller->http_request().uri().path();
  std::vector<std::string> elements;
  ailego::StringHelper::Split(path, "/", &elements);
  if (elements.size() < 4) {
    return PROXIMA_BE_ERROR_CODE(InvalidArgument);
  }
  collection_name->assign(elements[3]);
  return 0;
}

}  // end namespace server
}  // namespace be
}  // end namespace proxima
