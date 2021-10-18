#include "grpc_client.h"
#include "http_client.h"

namespace proxima {
namespace be {

ProximaSearchClientPtr ProximaSearchClient::Create(const std::string &type) {
  if (type.empty() || type == "GrpcClient") {
    return ProximaSearchClientPtr(new GrpcProximaSearchClient());
  } else if (type == "HttpClient") {
    return ProximaSearchClientPtr(new HttpProximaSearchClient());
  } else {
    return ProximaSearchClientPtr();
  }
}

ProximaSearchClientPtr ProximaSearchClient::ProximaSearchClient::Create() {
  return Create("");
}

WriteRequestPtr WriteRequest::Create() {
  return std::make_shared<PbWriteRequest>();
}

QueryRequestPtr QueryRequest::Create() {
  return std::make_shared<PbQueryRequest>();
}

QueryResponsePtr QueryResponse::Create() {
  return std::make_shared<PbQueryResponse>();
}

GetDocumentRequestPtr GetDocumentRequest::Create() {
  return std::make_shared<PbGetDocumentRequest>();
}

GetDocumentResponsePtr GetDocumentResponse::Create() {
  return std::make_shared<PbGetDocumentResponse>();
}


}  // namespace be
}  // end namespace proxima
