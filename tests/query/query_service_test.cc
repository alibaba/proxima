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
 *
 *   \author   guonix
 *   \date     Dec 2020
 *   \brief
 */


#include <memory>
#include <gtest/gtest.h>
#include <meta/meta_impl.h>
#include "index/mock_index_service.h"  // for MockIndexService
#include "index/mock_segment.h"        // for MockSegment
#include "meta/mock_meta_service.h"    // for MockMetaService
#include "query/query_service_builder.h"

using QueryRequest = proxima::be::proto::QueryRequest;
using QueryResponse = proxima::be::proto::QueryResponse;
using GetDocumentRequest = proxima::be::proto::GetDocumentRequest;
using GetDocumentResponse = proxima::be::proto::GetDocumentResponse;

using KnnParam = proxima::be::proto::QueryRequest::KnnQueryParam;


class QueryServiceTest : public Test {
 protected:
  // Sets up the test fixture.
  void SetUp() override {
    meta_service_ = std::make_shared<MockMetaService>();
    index_service_ = std::make_shared<MockIndexService>();

    init_request();
    init_response();
  }

  // Tears down the test fixture.
  void TearDown() override {
    meta_service_.reset();
    index_service_.reset();
  }

 private:
  void init_request() {
    request_.reset(new (std::nothrow) QueryRequest());
    request_->set_query_type(proxima::be::proto::QueryRequest_QueryType_QT_KNN);
    request_->set_collection_name(collection_);
    // not supported yet
    request_->set_debug_mode(false);
    // Allocate KNN Param
    param_ = request_->mutable_knn_param();

    param_->set_column_name("column_name");
    param_->set_topk(1);
    param_->set_dimension(10);
    param_->set_data_type(proxima::be::proto::DataType::DT_VECTOR_FP16);
    param_->set_features("features");
    param_->set_batch_count(1);
    param_->set_radius(0.1f);

    auto kv = param_->add_extra_params();
    kv->set_key("string_key1");
    kv->set_value("value1");
    kv = param_->add_extra_params();
    kv->set_key("int_key1");
    kv->set_value("10");

    equal_request_.reset(new (std::nothrow) GetDocumentRequest());
    equal_request_->set_collection_name(collection_);
    // not supported yet
    equal_request_->set_debug_mode(false);
    equal_request_->set_primary_key(1);
  }

  void init_response() {
    response_.reset(new (std::nothrow) QueryResponse());
  }

 protected:
  MockMetaServicePtr meta_service_{nullptr};
  MockIndexServicePtr index_service_{nullptr};
  std::unique_ptr<QueryRequest> request_{nullptr};
  std::unique_ptr<QueryResponse> response_{nullptr};
  KnnParam *param_{nullptr};
  std::unique_ptr<GetDocumentRequest> equal_request_{nullptr};
  // std::unique_ptr<GetDocumentResponse> equal_response_{nullptr};

  std::string collection_{"unittest"};
};

using QueryService = proxima::be::query::QueryService;
using QueryServiceBuilder = proxima::be::query::QueryServiceBuilder;

TEST_F(QueryServiceTest, TestInitialize) {
  EXPECT_FALSE(QueryServiceBuilder::Create(nullptr, meta_service_, 1));
  EXPECT_FALSE(QueryServiceBuilder::Create(nullptr, nullptr, 1));
  EXPECT_FALSE(QueryServiceBuilder::Create(index_service_, nullptr, 1));

  auto svc = QueryServiceBuilder::Create(index_service_, meta_service_, 1);
  EXPECT_TRUE(svc);
  EXPECT_TRUE(svc->initialized());
  EXPECT_TRUE(svc->cleanup() == 0);
}

TEST_F(QueryServiceTest, TestSearch) {
  {  // Invalid params
    auto svc = QueryServiceBuilder::Create(index_service_, meta_service_, 1);
    EXPECT_TRUE(svc);
    EXPECT_TRUE(svc->initialized());
    EXPECT_TRUE(svc->search(nullptr, nullptr, nullptr) != 0);
    QueryRequest request;
    request.set_query_type(
        proxima::be::proto::
            QueryRequest_QueryType_QueryRequest_QueryType_INT_MIN_SENTINEL_DO_NOT_USE_);
    EXPECT_TRUE(svc->search(&request, nullptr, nullptr) != 0);
    QueryResponse response;
    EXPECT_TRUE(svc->search(nullptr, &response, nullptr) != 0);
    svc->cleanup();
  }

  {  // Valid KNN Search
    // Mock Meta
    CollectionMeta collection_meta;
    collection_meta.mutable_forward_columns()->push_back("forward1");
    collection_meta.mutable_forward_columns()->push_back("forward2");
    auto column1 = std::make_shared<ColumnMeta>("column_name");
    column1->set_data_type(DataTypes::VECTOR_FP16);
    collection_meta.append(column1);

    CollectionImplPtr collection =
        std::make_shared<CollectionImpl>(collection_meta);
    // Return collection
    EXPECT_CALL(*meta_service_, get_current_collection(_))
        .WillRepeatedly(
            Invoke([&collection](const std::string &) -> CollectionMetaPtr {
              return collection->meta();
            }))
        .RetiresOnSaturation();  // success

    EXPECT_CALL(*meta_service_, get_collection(_, _))
        .WillOnce(Invoke([&collection](const std::string &, uint64_t revision) {
          EXPECT_EQ(revision, 1u);
          return collection->meta();
        }))
        .RetiresOnSaturation();

    auto segment = std::make_shared<MockSegment>();
    testing::Mock::AllowLeak(static_cast<void *>(segment.get()));
    // Set results
    EXPECT_CALL(*segment, knn_search(_, _, _, _, _))
        .WillOnce(Invoke([](const std::string &, const std::string &query,
                            const QueryParams &, uint32_t batch,
                            std::vector<QueryResultList> *results) {
          results->clear();
          EXPECT_EQ(batch, 1);
          EXPECT_EQ(query, "features");
          QueryResult result;
          result.primary_key = 1U;
          result.lsn = 1U;
          result.revision = 1;
          result.score = 0.95f;
          proxima::be::proto::GenericValueList values;
          auto value = values.add_values();
          value->set_int32_value(10);
          value = values.add_values();
          value->set_string_value("str_value");
          // Forward
          result.forward_data.assign(values.SerializeAsString());
          results->push_back({result});
          return 0;
        }))
        .RetiresOnSaturation();

    EXPECT_CALL(*index_service_, list_segments(_, _))
        .WillOnce(Invoke([&segment](const std::string &,
                                    index::SegmentPtrList *segments) -> int {
          segments->push_back(segment);
          return 0;
        }))
        .RetiresOnSaturation();

    auto svc = QueryServiceBuilder::Create(index_service_, meta_service_, 1);
    EXPECT_TRUE(svc);
    auto profiler = std::make_shared<proxima::be::Profiler>(false);
    EXPECT_EQ(svc->search(request_.get(), response_.get(), profiler), 0);
    EXPECT_EQ(response_->results_size(), 1);
    EXPECT_EQ(response_->results(0).documents_size(), 1);

    EXPECT_EQ(response_->results(0).documents(0).primary_key(), 1U);
    EXPECT_EQ(response_->results(0).documents(0).forward_column_values_size(),
              2);

    auto &kv = response_->results(0).documents(0).forward_column_values(0);
    EXPECT_EQ(kv.key(), "forward1");
    EXPECT_EQ(kv.value().int32_value(), 10);
    auto &kv1 = response_->results(0).documents(0).forward_column_values(1);
    EXPECT_EQ(kv1.key(), "forward2");
    EXPECT_EQ(kv1.value().string_value(), "str_value");

    response_->Clear();
  }

  //{ // Valid Equal Search
  //  // failed, no mocks with meta service
  //  EXPECT_TRUE(svc.search(equal_request_, response_) != 0);
  //  response_->Clear();
  //}
}
