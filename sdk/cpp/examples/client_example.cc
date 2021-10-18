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
 *   \brief    Example usage of proxima client
 */

#include <iostream>

#include "proxima_search_client.h"

using namespace proxima::be;

int main() {
  // Create a client instance
  ProximaSearchClientPtr client = ProximaSearchClient::Create();

  // Try to connect server
  {
    ChannelOptions options("127.0.0.1:16000");
    Status status = client->connect(options);
    if (status.code != 0) {
      std::cerr << "Connect server failed. code[" << status.code << "] reason["
                << status.reason << "]" << std::endl;
      return status.code;
    }
    std::cout << "===>Connect server success." << std::endl;
  }

  // Create collection
  {
    // Describe a collection config which
    // set collection name --> "test_collection"
    // set one index column --> "test_column"
    // set two forward columsn --> "fwd_column1" and "fwd_column2"
    CollectionConfig config;
    config.collection_name = "test_collection";
    config.forward_columns = {"fwd_column1", "fwd_column2"};
    config.index_columns = {
        IndexColumnParam("test_column", DataType::VECTOR_FP32, 8)};

    Status status = client->create_collection(config);
    if (status.code != 0) {
      std::cerr << "Create collection failed. code[" << status.code
                << "] reason[" << status.reason << "]" << std::endl;
      return status.code;
    }
    std::cout << "===>Create collection success." << std::endl;
  }

  // Get collection information
  {
    CollectionInfo collection_info;
    Status status =
        client->describe_collection("test_collection", &collection_info);
    if (status.code != 0) {
      std::cerr << "Describe collection failed. code[" << status.code
                << "] reason[" << status.reason << "]" << std::endl;
      return status.code;
    }

    std::cout << "===>Describe collection success." << std::endl;
    std::cout << "collection_name: " << collection_info.collection_name
              << std::endl;
    std::cout << "collection_status: " << (int)collection_info.collection_status
              << std::endl;
    std::cout << "collection_uuid: " << collection_info.collection_uuid
              << std::endl;
    for (size_t i = 0; i < collection_info.forward_columns.size(); i++) {
      std::cout << "forward_column: " << collection_info.forward_columns[i]
                << std::endl;
    }
    for (size_t i = 0; i < collection_info.index_columns.size(); i++) {
      IndexColumnParam &index_param = collection_info.index_columns[i];
      std::cout << "index_column: " << index_param.column_name << std::endl;
      std::cout << "index_type: " << (int)index_param.index_type << std::endl;
      std::cout << "data_type: " << (int)index_param.data_type << std::endl;
      std::cout << "dimension: " << index_param.dimension << std::endl;
    }
  }

  // Insert records
  {
    WriteRequestPtr write_request = WriteRequest::Create();
    write_request->set_collection_name("test_collection");
    // Set row meta first, just decribe the format of sending records
    // Make sure the sort of forward columns and index columns  match
    // upper CollectionConfig's sort.
    write_request->add_forward_columns({"fwd_column1", "fwd_column2"});
    write_request->add_index_column("test_column", DataType::VECTOR_FP32, 8);

    for (int i = 0; i < 10; i++) {
      WriteRequest::RowPtr row = write_request->add_row();
      row->set_primary_key(i);
      row->set_operation_type(OperationType::INSERT);
      row->add_index_value({i + 0.1f, i + 0.2f, i + 0.3f, i + 0.4f, i + 0.5f,
                            i + 0.6f, i + 0.7f, i + 0.8f});  // "test_column"
      row->add_forward_value("hello" + std::to_string(i));   // "fwd_column1"
      row->add_forward_value(1);                             // "fwd_column2"
    }

    Status status = client->write(*write_request);

    if (status.code != 0) {
      std::cerr << "Write records failed. code[" << status.code << "] reason["
                << status.reason << "]" << std::endl;
      return status.code;
    }

    std::cout << "===>Write records success." << std::endl;
  }

  // Get document by key
  {
    GetDocumentRequestPtr get_document_request = GetDocumentRequest::Create();
    GetDocumentResponsePtr get_document_response =
        GetDocumentResponse::Create();
    get_document_request->set_collection_name("test_collection");
    get_document_request->set_primary_key(0);
    Status status = client->get_document_by_key(*get_document_request,
                                                get_document_response.get());
    if (status.code != 0) {
      std::cerr << "Get document by key failed. code[" << status.code
                << "] reason[" << status.reason << "]" << std::endl;
      return status.code;
    }
    std::cout << "===>Get document by key success." << std::endl;
    auto doc = get_document_response->document();
    std::cout << "doc_key: " << doc->primary_key() << std::endl;
  }

  // Query records
  {
    QueryRequestPtr query_request = QueryRequest::Create();
    QueryResponsePtr query_response = QueryResponse::Create();

    query_request->set_collection_name("test_collection");
    QueryRequest::KnnQueryParamPtr knn_param =
        query_request->add_knn_query_param();
    knn_param->set_column_name("test_column");
    knn_param->set_topk(10);
    knn_param->set_features({0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8});

    Status status = client->query(*query_request, query_response.get());
    if (status.code != 0) {
      std::cerr << "Query records failed. code[" << status.code << "] reason["
                << status.reason << "]" << std::endl;
      return status.code;
    }
    std::cout << "===>Query records success." << std::endl;

    for (size_t i = 0; i < query_response->result_count(); i++) {
      auto result = query_response->result(i);
      for (size_t j = 0; j < result->document_count(); j++) {
        auto doc = result->document(j);
        std::cout << "doc_key: " << doc->primary_key() << std::endl;
        std::cout << "doc_score: " << doc->score() << std::endl;

        std::string fwd_val1;
        int fwd_val2;
        doc->get_forward_value("fwd_column1", &fwd_val1);
        doc->get_forward_value("fwd_column2", &fwd_val2);

        std::cout << "forward count: " << doc->forward_count() << std::endl;
        std::cout << "fwd_column1: " << fwd_val1 << std::endl;
        std::cout << "fwd_column2: " << fwd_val2 << std::endl;
      }
    }

    // Print end
  }

  // Drop collection
  {
    Status status = client->drop_collection("test_collection");
    if (status.code != 0) {
      std::cerr << "Drop collection failed. code[" << status.code << "] reason["
                << status.reason << "]" << std::endl;
      return status.code;
    }

    std::cout << "===>Drop collection success" << std::endl;
  }
}
