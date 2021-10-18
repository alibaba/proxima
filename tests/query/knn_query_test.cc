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

#include "query/knn_query.h"
#include <gtest/gtest.h>
#include <meta/meta_impl.h>
#include "index/mock_index_service.h"  // for MockIndexService
#include "index/mock_segment.h"        // for MockSegment
#include "meta/mock_meta_service.h"    // for MockMetaService
#include "mock_executor.h"             // for MockExecutor
#include "mock_query_context.h"        // for Mock*Context

using QueryRequest = proxima::be::proto::QueryRequest;
using QueryResponse = proxima::be::proto::QueryResponse;
using KnnParam = proxima::be::proto::QueryRequest::KnnQueryParam;

class KNNQueryTest : public Test {
 protected:
  // Sets up the test fixture.
  void SetUp() override {
    init_request();
    init_response();
  }

  // Tears down the test fixture.
  void TearDown() override {
    cleanup_request();
    cleanup_response();
  }

 private:
  void init_request() {
    request_ = new (std::nothrow) QueryRequest();
    request_->set_query_type(proxima::be::proto::QueryRequest_QueryType_QT_KNN);
    request_->set_collection_name(collection_);
    // not supported yet
    request_->set_debug_mode(false);
    // Allocate KNN Param
    param_ = request_->mutable_knn_param();

    param_->set_column_name("column_name");
    param_->set_topk(3);
    param_->set_dimension(10);
    param_->set_data_type(proto::DataType::DT_VECTOR_FP16);
    param_->set_features("features");
    param_->set_batch_count(1);
    param_->set_radius(0.1f);

    auto kv = param_->add_extra_params();
    kv->set_key("string_key1");
    kv->set_value("value1");
    kv = param_->add_extra_params();
    kv->set_key("int_key1");
    kv->set_value("10");
  }

  void init_response() {
    response_ = new (std::nothrow) QueryResponse();
  }

  template <typename Pointer>
  void delete_pointer_if(Pointer *ptr) {
    delete ptr;
    ptr = nullptr;
  }

  void cleanup_request() {
    delete_pointer_if(request_);
  }

  void cleanup_response() {
    delete_pointer_if(response_);
  }

 protected:
  QueryRequest *request_{nullptr};
  KnnParam *param_{nullptr};
  QueryResponse *response_{nullptr};
  std::string collection_{"unittest"};
};

TEST_F(KNNQueryTest, TestBaseFunctional) {
  auto meta_service = std::make_shared<MockMetaService>();

  auto meta = std::make_shared<MetaWrapper>(meta_service);
  // Test invalid params
  auto knn = std::make_shared<KNNQuery>(0, nullptr, nullptr, nullptr, nullptr,
                                        nullptr, nullptr);

  EXPECT_EQ(knn->mode(), IOMode::READONLY);
  EXPECT_EQ(knn->type(), QueryType::KNN);
  EXPECT_EQ(knn->id(), 0);
}

TEST_F(KNNQueryTest, TestValidate) {
  auto executor = std::make_shared<MockExecutor>();
  auto meta_service = std::make_shared<MockMetaService>();
  auto index_service = std::make_shared<MockIndexService>();

  auto meta = std::make_shared<MetaWrapper>(meta_service);
  // Test invalid params
  auto knn = std::make_shared<KNNQuery>(
      0, nullptr, index_service, meta, executor,
      std::make_shared<proxima::be::Profiler>(false), response_);
  EXPECT_TRUE(knn->validate() != 0);

  knn.reset(new (std::nothrow) KNNQuery(
      0, request_, index_service, meta, nullptr,
      std::make_shared<proxima::be::Profiler>(false), response_));
  EXPECT_TRUE(knn->validate() != 0);

  knn.reset(new (std::nothrow) KNNQuery(
      0, request_, index_service, meta, executor,
      std::make_shared<proxima::be::Profiler>(false), nullptr));
  EXPECT_TRUE(knn->validate() != 0);


  CollectionMeta collection_meta;
  collection_meta.mutable_forward_columns()->push_back("forward1");
  collection_meta.mutable_forward_columns()->push_back("forward2");
  auto column1 = std::make_shared<ColumnMeta>("column_name");
  collection_meta.append(column1);

  CollectionImplPtr collection =
      std::make_shared<CollectionImpl>(collection_meta);

  // Return 0, with invalid collection meta
  EXPECT_CALL(*meta_service, get_current_collection(_))
      .WillOnce(Invoke(
          [](const std::string &) -> CollectionMetaPtr { return nullptr; }))
      .RetiresOnSaturation();  // success
  // Set all right arguments
  knn.reset(new (std::nothrow) KNNQuery(
      0, request_, index_service, meta, executor,
      std::make_shared<proxima::be::Profiler>(false), response_));
  // Can't valid column from meta wrapper
  EXPECT_TRUE(knn->validate() != 0);


  // Return collection
  EXPECT_CALL(*meta_service, get_current_collection("unittest"))
      .WillOnce(Invoke([&collection](const std::string &) -> CollectionMetaPtr {
        return collection->meta();
      }))
      .RetiresOnSaturation();  // success

  EXPECT_EQ(knn->validate(), 0);

  EXPECT_EQ(knn->column(), "column_name");
  EXPECT_EQ(knn->batch_count(), 1);
}

TEST_F(KNNQueryTest, TestPrepare) {
  auto executor = std::make_shared<MockExecutor>();
  auto meta_service = std::make_shared<MockMetaService>();
  auto index_service = std::make_shared<MockIndexService>();

  EXPECT_CALL(*index_service, list_segments(collection_, _))
      .WillOnce(Return(1))
      .WillOnce(Return(0))  // Success but no available segments
      .RetiresOnSaturation();

  auto meta = std::make_shared<MetaWrapper>(meta_service);
  auto knn = std::make_shared<KNNQuery>(
      0, request_, index_service, meta, executor,
      std::make_shared<proxima::be::Profiler>(false), response_);
  EXPECT_TRUE(knn->prepare() != 0);
  EXPECT_TRUE(knn->prepare() != 0);
}

TEST_F(KNNQueryTest, TestEvaluate) {
  auto executor = std::make_shared<MockExecutor>();
  auto meta_service = std::make_shared<MockMetaService>();
  auto index_service = std::make_shared<MockIndexService>();
  auto segment = std::make_shared<MockSegment>();

  EXPECT_CALL(*index_service, list_segments(_, _))
      .WillRepeatedly(
          Invoke([&segment](const std::string &,
                            index::SegmentPtrList *segments) -> int {
            EXPECT_TRUE(segments != nullptr);
            segments->push_back(segment);
            return 0;
          }));

  CollectionMeta collection_meta;
  collection_meta.mutable_forward_columns()->push_back("forward1");
  collection_meta.mutable_forward_columns()->push_back("forward2");
  auto column1 = std::make_shared<ColumnMeta>("column_name");
  column1->set_data_type(DataTypes::VECTOR_FP16);
  collection_meta.append(column1);

  CollectionImplPtr collection =
      std::make_shared<CollectionImpl>(collection_meta);

  EXPECT_CALL(*meta_service, get_current_collection(_))
      .WillOnce(Invoke(
          [](const std::string &) -> CollectionMetaPtr { return nullptr; }))
      .WillOnce(Invoke([&collection](const std::string &) -> CollectionMetaPtr {
        return collection->meta();
      }))
      .RetiresOnSaturation();  // success

  auto meta = std::make_shared<MetaWrapper>(meta_service);
  auto knn = std::make_shared<KNNQuery>(
      0, request_, index_service, meta, executor,
      std::make_shared<proxima::be::Profiler>(false), response_);
  EXPECT_EQ(knn->prepare(), PROXIMA_BE_ERROR_CODE(MismatchedDataType));
  EXPECT_EQ(knn->prepare(), 0);

  {  // evaluate failed with fake execute
    EXPECT_CALL(*executor, execute_tasks(_))
        .WillOnce(Return(0))
        .RetiresOnSaturation();

    EXPECT_TRUE(knn->evaluate() != 0);
  }

  {  // Evaluate success, but no enough values
    // Execute task
    EXPECT_CALL(*executor, execute_tasks(_))
        .WillOnce(Invoke([](const TaskPtrList &tasks) {
          for (auto &task : tasks) {
            task->status(Task::Status::SCHEDULED);
            task->run();
          }
          return 0;
        }))  // Fake Execute
        .RetiresOnSaturation();

    // Set results
    EXPECT_CALL(*segment, knn_search(_, _, _, _, _))
        .WillRepeatedly(Invoke([](const std::string &, const std::string &,
                                  const QueryParams &, uint32_t batch,
                                  std::vector<QueryResultList> *results) {
          EXPECT_EQ(batch, 1);
          results->push_back({});
          return 0;
        }))
        .RetiresOnSaturation();

    // No enough results
    EXPECT_EQ(knn->evaluate(), 0);
  }

  response_->Clear();

  {  // Test serialize
    EXPECT_CALL(*executor, execute_tasks(_))
        .WillRepeatedly(Invoke([](const TaskPtrList &tasks) {
          for (auto &task : tasks) {
            task->status(Task::Status::SCHEDULED);
            task->run();
          }
          return 0;
        }))  // Fake Execute
        .RetiresOnSaturation();

    CollectionImplPtr collection_impl = nullptr;
    {  // Init collection
      CollectionMeta temp_meta;
      temp_meta.mutable_forward_columns()->push_back("forward1");
      temp_meta.mutable_forward_columns()->push_back("forward2");
      collection_impl.reset(new CollectionImpl(temp_meta));
    }

    testing::Mock::AllowLeak(static_cast<void *>(meta_service.get()));
    EXPECT_CALL(*meta_service, get_collection(_, _))
        .WillRepeatedly(Invoke([&collection_impl](const std::string &collection,
                                                  uint64_t revision) {
          EXPECT_EQ(revision, 1u);
          return collection_impl->meta();
        }))
        .RetiresOnSaturation();

    // Set results
    EXPECT_CALL(*segment, knn_search(_, _, _, _, _))
        .WillRepeatedly(Invoke([](const std::string &, const std::string &,
                                  const QueryParams &, uint32_t batch,
                                  std::vector<QueryResultList> *results) {
          results->clear();

          EXPECT_EQ(batch, 1);
          {
            QueryResult result95;
            result95.primary_key = 1U;
            result95.lsn = 1U;
            result95.revision = 1;
            result95.score = 0.95f;
            {
              proxima::be::proto::GenericValueList values;
              auto value = values.add_values();
              value->set_int32_value(10);
              value = values.add_values();
              value->set_string_value("strvalue");
              // Forward
              result95.forward_data.assign(values.SerializeAsString());
            }

            QueryResult result96;
            result96.primary_key = 2U;
            result96.lsn = 1U;
            result96.revision = 1;
            result96.score = 0.96f;
            {
              proxima::be::proto::GenericValueList values;
              auto value = values.add_values();
              value->set_int32_value(10);
              value = values.add_values();
              value->set_string_value("strvalue");
              // Forward
              result96.forward_data.assign(values.SerializeAsString());
            }
            QueryResult result93;
            result93.primary_key = 3U;
            result93.lsn = 1U;
            result93.revision = 1;
            result93.score = 0.93f;
            {
              proxima::be::proto::GenericValueList values;
              auto value = values.add_values();
              value->set_int32_value(10);
              value = values.add_values();
              value->set_string_value("strvalue");
              // Forward
              result93.forward_data.assign(values.SerializeAsString());
            }
            results->push_back({result93, result95, result96});
          }
          return 0;
        }))
        .RetiresOnSaturation();

    // No enough results
    EXPECT_EQ(knn->evaluate(), 0);
    EXPECT_EQ(response_->results_size(), 1);
    EXPECT_EQ(response_->results(0).documents_size(), 3);

    EXPECT_EQ(response_->results(0).documents(0).primary_key(), 3U);
    EXPECT_EQ(response_->results(0).documents(0).forward_column_values_size(),
              2);
    auto &kv = response_->results(0).documents(0).forward_column_values(0);
    EXPECT_EQ(kv.key(), "forward1");
    EXPECT_EQ(kv.value().int32_value(), 10);
    auto &kv1 = response_->results(0).documents(0).forward_column_values(1);
    EXPECT_EQ(kv1.key(), "forward2");
    EXPECT_EQ(kv1.value().string_value(), "strvalue");
    EXPECT_EQ(response_->results(0).documents(1).primary_key(), 3U);
    EXPECT_EQ(response_->results(0).documents(2).primary_key(), 1U);
  }

  response_->Clear();
}

TEST_F(KNNQueryTest, TestFinalize) {
  auto executor = std::make_shared<MockExecutor>();
  auto meta_service = std::make_shared<MockMetaService>();
  auto index_service = std::make_shared<MockIndexService>();
  auto meta = std::make_shared<MetaWrapper>(meta_service);

  auto knn = std::make_shared<KNNQuery>(
      0, request_, index_service, meta, executor,
      std::make_shared<proxima::be::Profiler>(false), response_);

  EXPECT_EQ(knn->finalize(), 0);
}
