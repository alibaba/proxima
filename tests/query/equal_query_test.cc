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


#include "query/equal_query.h"
#include <gtest/gtest.h>
#include <meta/meta_impl.h>
#include "index/mock_index_service.h"  // for MockIndexService
#include "index/mock_segment.h"        // for MockSegment
#include "meta/mock_meta_service.h"    // for MockMetaService
#include "mock_executor.h"             // for MockExecutor


using GetDocumentRequest = proxima::be::proto::GetDocumentRequest;
using GetDocumentResponse = proxima::be::proto::GetDocumentResponse;

class EqualQueryTest : public Test {
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
    request_ = new (std::nothrow) GetDocumentRequest();
    request_->set_collection_name(collection_);
    request_->set_debug_mode(false);
    request_->set_primary_key(1);
  }

  void init_response() {
    response_ = new (std::nothrow) GetDocumentResponse();
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
  GetDocumentRequest *request_{nullptr};
  GetDocumentResponse *response_{nullptr};
  std::string collection_{"unittest"};
};

TEST_F(EqualQueryTest, TestBaseFunctional) {
  // Test invalid params
  auto query = std::make_shared<EqualQuery>(0, nullptr, nullptr, nullptr,
                                            nullptr, nullptr, nullptr);

  EXPECT_EQ(query->mode(), IOMode::READONLY);
  EXPECT_EQ(query->type(), QueryType::EQUAL);
  EXPECT_EQ(query->id(), 0);
}

TEST_F(EqualQueryTest, TestValidate) {
  auto executor = std::make_shared<MockExecutor>();
  auto meta_service = std::make_shared<MockMetaService>();
  auto index_service = std::make_shared<MockIndexService>();
  auto meta = std::make_shared<MetaWrapper>(meta_service);
  // Test invalid params
  auto query = std::make_shared<EqualQuery>(
      0, nullptr, index_service, meta, executor,
      std::make_shared<proxima::be::Profiler>(false), response_);
  EXPECT_TRUE(query->validate() != 0);

  query.reset(new (std::nothrow) EqualQuery(
      0, request_, index_service, meta, nullptr,
      std::make_shared<proxima::be::Profiler>(false), response_));
  EXPECT_TRUE(query->validate() != 0);

  // Return 0, with invalid collection meta
  EXPECT_CALL(*meta_service, get_current_collection(_))
      .WillOnce(Invoke(
          [](const std::string &) -> CollectionMetaPtr { return nullptr; }))
      .RetiresOnSaturation();  // success

  query.reset(new (std::nothrow) EqualQuery(
      0, request_, index_service, meta, executor,
      std::make_shared<proxima::be::Profiler>(false), nullptr));
  EXPECT_TRUE(query->validate() != 0);

  CollectionMeta collection_meta;
  collection_meta.mutable_forward_columns()->push_back("forward1");
  collection_meta.mutable_forward_columns()->push_back("forward2");
  auto column1 = std::make_shared<ColumnMeta>();
  collection_meta.append(column1);

  CollectionImplPtr collection =
      std::make_shared<CollectionImpl>(collection_meta);

  // Return 0, with invalid collection meta
  EXPECT_CALL(*meta_service, get_current_collection(_))
      .WillOnce(Invoke(
          [](const std::string &) -> CollectionMetaPtr { return nullptr; }))
      .RetiresOnSaturation();  // success
  // Set all right arguments
  query.reset(new (std::nothrow) EqualQuery(
      0, request_, index_service, meta, executor,
      std::make_shared<proxima::be::Profiler>(false), response_));
  // Can't valid column from meta wrapper
  EXPECT_TRUE(query->validate() != 0);

  // Return collection
  EXPECT_CALL(*meta_service, get_current_collection("unittest"))
      .WillOnce(Invoke([&collection](const std::string &) -> CollectionMetaPtr {
        return collection->meta();
      }))
      .RetiresOnSaturation();  // success

  EXPECT_EQ(query->validate(), 0);
}

TEST_F(EqualQueryTest, TestPrepare) {
  auto executor = std::make_shared<MockExecutor>();
  auto meta_service = std::make_shared<MockMetaService>();
  auto index_service = std::make_shared<MockIndexService>();
  auto meta = std::make_shared<MetaWrapper>(meta_service);

  EXPECT_CALL(*index_service, list_segments(collection_, _))
      .WillOnce(Return(1))
      .WillOnce(Return(0))  // Success but no available segments
      .RetiresOnSaturation();

  auto query = std::make_shared<EqualQuery>(
      0, request_, index_service, meta, executor,
      std::make_shared<proxima::be::Profiler>(false), response_);
  EXPECT_TRUE(query->prepare() != 0);
  EXPECT_TRUE(query->prepare() != 0);
}

TEST_F(EqualQueryTest, TestEvaluate) {
  auto executor = std::make_shared<MockExecutor>();
  auto meta_service = std::make_shared<MockMetaService>();
  auto index_service = std::make_shared<MockIndexService>();
  auto meta = std::make_shared<MetaWrapper>(meta_service);
  auto segment = std::make_shared<MockSegment>();

  EXPECT_CALL(*index_service, list_segments(_, _))
      .WillOnce(Invoke([&segment](const std::string &,
                                  index::SegmentPtrList *segments) -> int {
        segments->push_back(segment);
        return 0;
      }))
      .RetiresOnSaturation();

  CollectionImplPtr collection_impl = nullptr;
  {  // Init collection
    CollectionMeta cmeta;
    cmeta.mutable_forward_columns()->push_back("forward1");
    cmeta.mutable_forward_columns()->push_back("forward2");
    cmeta.set_revision(10);
    collection_impl.reset(new CollectionImpl(cmeta));
  }

  EXPECT_CALL(*meta_service, get_collection(_, _))
      .WillRepeatedly(
          Invoke([this, &collection_impl](const std::string &collection,
                                          uint64_t revision) {
            EXPECT_EQ(collection_, collection);
            EXPECT_EQ(revision, 10);
            return collection_impl->meta();
          }))
      .RetiresOnSaturation();

  auto query = std::make_shared<EqualQuery>(
      0, request_, index_service, meta, executor,
      std::make_shared<proxima::be::Profiler>(false), response_);

  EXPECT_EQ(query->prepare(), 0);

  {  // evaluate failed with fake execute
    EXPECT_CALL(*executor, execute_tasks(_))
        .WillOnce(Return(0))
        .RetiresOnSaturation();

    // Success, but with no results
    EXPECT_EQ(query->evaluate(), 0);
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
    EXPECT_CALL(*segment, kv_search(_, _))
        .WillOnce(Invoke([](uint64_t primary_key, QueryResult *result) {
          EXPECT_EQ(primary_key, 1);
          result->primary_key = 1;
          result->revision = 10;
          return 0;
        }))
        .RetiresOnSaturation();

    // Expect mismatch error
    EXPECT_EQ(query->evaluate(), PROXIMA_BE_ERROR_CODE(MismatchedForward));
  }

  response_->Clear();

  {  // Test serialize
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
    EXPECT_CALL(*segment, kv_search(_, _))
        .WillOnce(Invoke([](uint64_t primary_key, QueryResult *result) {
          EXPECT_EQ(primary_key, 1);
          result->primary_key = 1U;
          result->lsn = 1U;
          result->revision = 10;
          result->score = 0.95f;
          proxima::be::proto::GenericValueList values;
          auto value = values.add_values();
          value->set_int32_value(10);
          value = values.add_values();
          value->set_string_value("strvalue");
          // Forward
          result->forward_data.assign(values.SerializeAsString());
          return 0;
        }))
        .RetiresOnSaturation();

    // Expect 0
    EXPECT_EQ(query->evaluate(), 0);


    EXPECT_EQ(response_->document().primary_key(), 1U);
    EXPECT_EQ(response_->document().forward_column_values_size(), 2);
    auto &kv = response_->document().forward_column_values(0);
    EXPECT_EQ(kv.key(), "forward1");
    EXPECT_EQ(kv.value().int32_value(), 10);
    auto &kv1 = response_->document().forward_column_values(1);
    EXPECT_EQ(kv1.key(), "forward2");
    EXPECT_EQ(kv1.value().string_value(), "strvalue");
  }

  response_->Clear();
}

TEST_F(EqualQueryTest, TestFinalize) {
  auto executor = std::make_shared<MockExecutor>();
  auto meta_service = std::make_shared<MockMetaService>();
  auto index_service = std::make_shared<MockIndexService>();
  auto meta = std::make_shared<MetaWrapper>(meta_service);
  auto query = std::make_shared<EqualQuery>(
      0, request_, index_service, meta, executor,
      std::make_shared<proxima::be::Profiler>(false), response_);

  EXPECT_EQ(query->finalize(), 0);
}
