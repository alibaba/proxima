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
 *   \author   hongqing.hu
 *   \date     Dec 2020
 *   \brief
 */


#include <gtest/gtest.h>
#define private public
#define protected public
#include "agent/index_agent.h"
#include "index/file_helper.h"
#include "index/mock_index_service.h"  // for MockIndexService
#include "meta/mock_meta_service.h"    // for MockMetaService
#include "proto/common.pb.h"

#undef protected
#undef private

using namespace proxima::be;
using namespace proxima::be::agent;

class IndexAgentTest : public Test {
 protected:
  // Sets up the test fixture.
  void SetUp() override {
    meta_service_ = std::make_shared<MockMetaService>();
    char cmd_buf[100];
    snprintf(cmd_buf, 100, "rm -rf ./agent_friends/");
    system(cmd_buf);

    FillSchema(&proxy_schema_, proxy_request_, true);
    FillSchema(&direct_schema_, direct_request_, false);

    collection_path_ = "./" + collection_name_;
  }

  // Tears down the test fixture.
  void TearDown() override {
    meta_service_.reset();
    index::FileHelper::RemoveDirectory(collection_path_);
  }

  void FillSchema(meta::CollectionMetaPtr *schema, WriteRequest &request,
                  bool with_repo = true) {
    *schema = std::make_shared<meta::CollectionMeta>();
    auto *forward_columns = (*schema)->mutable_forward_columns();
    forward_columns->emplace_back("age");
    meta::ColumnMetaPtr column_meta = std::make_shared<meta::ColumnMeta>();
    column_meta->set_name("face");
    column_meta->set_index_type(IndexTypes::PROXIMA_GRAPH_INDEX);
    column_meta->set_data_type(DataTypes::VECTOR_FP32);
    column_meta->set_dimension(16);
    column_meta->mutable_parameters()->set("metric_type", "SquaredEuclidean");
    (*schema)->append(column_meta);
    (*schema)->set_name(collection_name_);
    if (with_repo) {
      meta::RepositoryBasePtr repo =
          std::make_shared<meta::DatabaseRepositoryMeta>();
      repo->set_name(collection_name_);
      (*schema)->set_repository(repo);
    }
    request.set_collection_name(collection_name_);
    index::CollectionDatasetPtr dataset =
        std::make_shared<index::CollectionDataset>(0);
    auto *row_data = dataset->add_row_data();
    row_data->primary_key = 123456;
    row_data->operation_type = OperationTypes::INSERT;

    if (with_repo) {
      row_data->lsn_check = true;
      row_data->lsn = 1;
      row_data->lsn_context = "binlog:123";
      request.set_request_type(WriteRequest::RequestType::PROXY);
    } else {
      row_data->lsn_check = false;
      request.set_request_type(WriteRequest::RequestType::DIRECT);
    }

    row_data->column_datas.resize(1);
    row_data->column_datas[0].column_name = "face";
    row_data->column_datas[0].data_type = DataTypes::VECTOR_FP32;
    row_data->column_datas[0].dimension = 16;
    std::vector<float> vectors = {1, 2,  3,  4,  5,  6,  7,  8,
                                  9, 10, 11, 12, 13, 14, 15, 16};
    row_data->column_datas[0].data.resize(vectors.size() * sizeof(float));
    memcpy((void *)&(row_data->column_datas[0].data[0]), (void *)vectors.data(),
           vectors.size() * sizeof(float));
    proto::GenericValueList forward_list;
    forward_list.add_values()->set_int32_value(32);
    forward_list.SerializeToString(&(row_data->forward_data));

    request.add_collection_dataset(dataset);
  }

 protected:
  std::string collection_name_{"agent_friends"};
  std::string collection_path_{};
  meta::CollectionMetaPtr proxy_schema_{};
  meta::CollectionMetaPtr direct_schema_{};
  WriteRequest proxy_request_{};
  WriteRequest direct_request_{};
  MockMetaServicePtr meta_service_{nullptr};
  MockIndexServicePtr index_service_{nullptr};
};

TEST_F(IndexAgentTest, TestGeneral) {
  IndexAgentPtr agent = IndexAgent::Create(meta_service_);
  int ret = agent->init();
  ASSERT_EQ(ret, 0);

  EXPECT_CALL(*meta_service_, get_latest_collections(_))
      .WillOnce(testing::Return(0));
  ret = agent->start();
  ASSERT_EQ(ret, 0);

  meta::CollectionMetaPtr schema = proxy_schema_;
  // create collection
  EXPECT_CALL(*meta_service_, get_current_collection(_))
      .Times(1)
      .WillOnce(
          Invoke([&schema](const std::string &) -> meta::CollectionMetaPtr {
            return schema;
          }))
      .RetiresOnSaturation();
  ret = agent->create_collection(collection_name_);
  ASSERT_EQ(ret, 0);

  // process
  proxy_request_.set_magic_number(agent->agent_timestamp_);
  // create collection
  EXPECT_CALL(*meta_service_, get_current_collection(_))
      .Times(2)
      .WillOnce(
          Invoke([&schema](const std::string &) -> meta::CollectionMetaPtr {
            return schema;
          }))
      .WillOnce(
          Invoke([&schema](const std::string &) -> meta::CollectionMetaPtr {
            return schema;
          }))
      .RetiresOnSaturation();
  ret = agent->write(proxy_request_);
  ASSERT_EQ(ret, 0);

  sleep(1);

  // get collection stats
  index::CollectionStats stats;
  ret = agent->get_collection_stats(collection_name_, &stats);
  ASSERT_EQ(ret, 0);

  // is collection suspend
  EXPECT_CALL(*meta_service_, get_current_collection(_))
      .WillOnce(
          Invoke([&schema](const std::string &) -> meta::CollectionMetaPtr {
            return schema;
          }))
      .RetiresOnSaturation();
  ASSERT_EQ(agent->is_collection_suspend(collection_name_), false);

  // update collection
  meta::CollectionMetaPtr new_schema;
  WriteRequest tmp_request;
  FillSchema(&new_schema, tmp_request, true);
  new_schema->set_revision(2);
  EXPECT_CALL(*meta_service_, get_collection(_, _))
      .WillOnce(Invoke([&new_schema](const std::string &, uint32_t revision)
                           -> meta::CollectionMetaPtr { return new_schema; }))
      .RetiresOnSaturation();

  ret = agent->update_collection(collection_name_, 2);
  ASSERT_EQ(ret, 0);

  sleep(1);

  // drop collection
  ret = agent->drop_collection(collection_name_);
  ASSERT_EQ(ret, 0);

  ret = agent->stop();
  ASSERT_EQ(ret, 0);
  ret = agent->cleanup();
  ASSERT_EQ(ret, 0);
}

TEST_F(IndexAgentTest, TestCreateCollectionWithMetaServiceFailed) {
  IndexAgentPtr agent = IndexAgent::Create(meta_service_);
  int ret = agent->init();
  ASSERT_EQ(ret, 0);

  EXPECT_CALL(*meta_service_, get_latest_collections(_))
      .WillOnce(testing::Return(0));
  ret = agent->start();
  ASSERT_EQ(ret, 0);

  // create collection
  EXPECT_CALL(*meta_service_, get_current_collection(_))
      .WillOnce(Invoke([](const std::string &) -> meta::CollectionMetaPtr {
        return nullptr;
      }))
      .RetiresOnSaturation();
  ret = agent->create_collection(collection_name_);
  ASSERT_EQ(ret, ErrorCode_InexistentCollection);

  ret = agent->stop();
  ASSERT_EQ(ret, 0);
  ret = agent->cleanup();
  ASSERT_EQ(ret, 0);
}

TEST_F(IndexAgentTest, TestCreateCollectionWithIndexServiceFailed) {
  IndexAgentPtr agent = IndexAgent::Create(meta_service_);
  int ret = agent->init();
  ASSERT_EQ(ret, 0);

  EXPECT_CALL(*meta_service_, get_latest_collections(_))
      .WillOnce(testing::Return(0));
  ret = agent->start();
  ASSERT_EQ(ret, 0);

  index::FileHelper::CreateDirectory(collection_path_);

  agent->get_service()->index_directory_ = "./";
  meta::CollectionMetaPtr schema = std::make_shared<meta::CollectionMeta>();
  // create collection
  EXPECT_CALL(*meta_service_, get_current_collection(_))
      .WillOnce(
          Invoke([&schema](const std::string &) -> meta::CollectionMetaPtr {
            return schema;
          }))
      .RetiresOnSaturation();
  ret = agent->create_collection(collection_name_);
  ASSERT_EQ(ret, ErrorCode_DuplicateCollection);

  ret = agent->stop();
  ASSERT_EQ(ret, 0);
  ret = agent->cleanup();
  ASSERT_EQ(ret, 0);
}

TEST_F(IndexAgentTest, TestUpdateCollectionFailed) {
  IndexAgentPtr agent = IndexAgent::Create(meta_service_);
  int ret = agent->init();
  ASSERT_EQ(ret, 0);

  EXPECT_CALL(*meta_service_, get_latest_collections(_))
      .WillOnce(testing::Return(0));
  ret = agent->start();
  ASSERT_EQ(ret, 0);

  // get counter failed
  ret = agent->update_collection(collection_name_, 100);
  ASSERT_EQ(ret, ErrorCode_RuntimeError);

  // create collection
  meta::CollectionMetaPtr schema = std::make_shared<meta::CollectionMeta>();
  // create collection
  EXPECT_CALL(*meta_service_, get_current_collection(_))
      .WillOnce(
          Invoke([&schema](const std::string &) -> meta::CollectionMetaPtr {
            return schema;
          }))
      .RetiresOnSaturation();
  ret = agent->create_collection(collection_name_);
  ASSERT_EQ(ret, 0);

  // get collection meta failed
  EXPECT_CALL(*meta_service_, get_collection(_, _))
      .WillOnce(
          Invoke([](const std::string &, uint32_t) -> meta::CollectionMetaPtr {
            return nullptr;
          }))
      .RetiresOnSaturation();
  ret = agent->update_collection(collection_name_, 100);
  ASSERT_EQ(ret, ErrorCode_InexistentCollection);


  // index service update collection failed
  meta::CollectionMetaPtr new_schema = std::make_shared<meta::CollectionMeta>();
  EXPECT_CALL(*meta_service_, get_collection(_, _))
      .WillOnce(Invoke([&new_schema](const std::string &, uint32_t revision)
                           -> meta::CollectionMetaPtr { return new_schema; }))
      .RetiresOnSaturation();
  ret = agent->update_collection(collection_name_, 100);
  ASSERT_EQ(ret, ErrorCode_MismatchedSchema);

  ret = agent->stop();
  ASSERT_EQ(ret, 0);
  ret = agent->cleanup();
  ASSERT_EQ(ret, 0);
}

TEST_F(IndexAgentTest, TestDropCollectionFailed) {
  IndexAgentPtr agent = IndexAgent::Create(meta_service_);
  int ret = agent->init();
  ASSERT_EQ(ret, 0);

  EXPECT_CALL(*meta_service_, get_latest_collections(_))
      .WillOnce(testing::Return(0));
  ret = agent->start();
  ASSERT_EQ(ret, 0);

  // get counter failed
  ret = agent->drop_collection(collection_name_);
  ASSERT_EQ(ret, ErrorCode_InexistentCollection);

  ret = agent->stop();
  ASSERT_EQ(ret, 0);
  ret = agent->cleanup();
  ASSERT_EQ(ret, 0);
}

TEST_F(IndexAgentTest, TestGetCollectionStatsFailed) {
  IndexAgentPtr agent = IndexAgent::Create(meta_service_);
  int ret = agent->init();
  ASSERT_EQ(ret, 0);

  EXPECT_CALL(*meta_service_, get_latest_collections(_))
      .WillOnce(testing::Return(0));
  ret = agent->start();
  ASSERT_EQ(ret, 0);

  // get collection stats
  index::CollectionStats stats;
  ret = agent->get_collection_stats(collection_name_, &stats);
  ASSERT_EQ(ret, ErrorCode_InexistentCollection);

  ret = agent->stop();
  ASSERT_EQ(ret, 0);
  ret = agent->cleanup();
  ASSERT_EQ(ret, 0);
}

TEST_F(IndexAgentTest, TestIsCollectionSuspendFailed) {
  IndexAgentPtr agent = IndexAgent::Create(meta_service_);
  int ret = agent->init();
  ASSERT_EQ(ret, 0);

  EXPECT_CALL(*meta_service_, get_latest_collections(_))
      .WillOnce(testing::Return(0));
  ret = agent->start();
  ASSERT_EQ(ret, 0);

  // is collection suspend
  meta::CollectionMetaPtr schema;
  EXPECT_CALL(*meta_service_, get_current_collection(_))
      .WillOnce(
          Invoke([&schema](const std::string &) -> meta::CollectionMetaPtr {
            return schema;
          }))
      .RetiresOnSaturation();
  bool result = agent->is_collection_suspend(collection_name_);
  ASSERT_EQ(result, false);

  ret = agent->stop();
  ASSERT_EQ(ret, 0);
  ret = agent->cleanup();
  ASSERT_EQ(ret, 0);
}

TEST_F(IndexAgentTest, TestInitFailed) {
  IndexAgentPtr agent = IndexAgent::Create(meta_service_);
  agent->meta_service_ = nullptr;

  // meta service nullptr
  int ret = agent->init();
  ASSERT_EQ(ret, ErrorCode_RuntimeError);
}

TEST_F(IndexAgentTest, TestStartFailed) {
  IndexAgentPtr agent = IndexAgent::Create(meta_service_);
  int ret = agent->init();
  ASSERT_EQ(ret, 0);

  // load index service failed
  EXPECT_CALL(*meta_service_, get_latest_collections(_))
      .WillOnce(testing::Return(1));
  ret = agent->start();
  ASSERT_EQ(ret, 1);
}

TEST_F(IndexAgentTest, TestLoadIndexServiceFailed) {
  IndexAgentPtr agent = IndexAgent::Create(meta_service_);
  int ret = agent->init();
  ASSERT_EQ(ret, 0);

  ret = agent->index_service_->start();
  ASSERT_EQ(ret, 0);

  // get latest collections failed
  EXPECT_CALL(*meta_service_, get_latest_collections(_))
      .WillOnce(testing::Return(1));
  ret = agent->load_index_service();
  ASSERT_EQ(ret, 1);

  meta::CollectionMetaPtr schema = proxy_schema_;
  EXPECT_CALL(*meta_service_, get_latest_collections(_))
      .WillOnce(Invoke([&schema](meta::CollectionMetaPtrList *schemas) -> int {
        schemas->emplace_back(schema);
        return 0;
      }))
      .RetiresOnSaturation();

  ret = agent->load_index_service();
  ASSERT_EQ(ret, ErrorCode_InvalidIndexDataFormat);
}

TEST_F(IndexAgentTest, TestWriteSuccessWithProxy) {
  IndexAgentPtr agent = IndexAgent::Create(meta_service_);
  int ret = agent->init();
  ASSERT_EQ(ret, 0);

  EXPECT_CALL(*meta_service_, get_latest_collections(_))
      .WillOnce(testing::Return(0));
  ret = agent->start();
  ASSERT_EQ(ret, 0);

  meta::CollectionMetaPtr schema = proxy_schema_;
  // create collection
  EXPECT_CALL(*meta_service_, get_current_collection(_))
      .Times(1)
      .WillOnce(
          Invoke([&schema](const std::string &) -> meta::CollectionMetaPtr {
            return schema;
          }))
      .RetiresOnSaturation();
  ret = agent->create_collection(collection_name_);
  ASSERT_EQ(ret, 0);

  // process
  proxy_request_.set_magic_number(agent->agent_timestamp_);
  // create collection
  EXPECT_CALL(*meta_service_, get_current_collection(_))
      .Times(2)
      .WillOnce(
          Invoke([&schema](const std::string &) -> meta::CollectionMetaPtr {
            return schema;
          }))
      .WillOnce(
          Invoke([&schema](const std::string &) -> meta::CollectionMetaPtr {
            return schema;
          }))
      .RetiresOnSaturation();
  ret = agent->write(proxy_request_);
  ASSERT_EQ(ret, 0);

  sleep(1);

  // drop collection
  ret = agent->drop_collection(collection_name_);
  ASSERT_EQ(ret, 0);

  ret = agent->stop();
  ASSERT_EQ(ret, 0);
  ret = agent->cleanup();
  ASSERT_EQ(ret, 0);
}

TEST_F(IndexAgentTest, TestWriteSuccessWithDirect) {
  IndexAgentPtr agent = IndexAgent::Create(meta_service_);
  int ret = agent->init();
  ASSERT_EQ(ret, 0);

  EXPECT_CALL(*meta_service_, get_latest_collections(_))
      .WillOnce(testing::Return(0));
  ret = agent->start();
  ASSERT_EQ(ret, 0);

  meta::CollectionMetaPtr schema = direct_schema_;
  // create collection
  EXPECT_CALL(*meta_service_, get_current_collection(_))
      .Times(1)
      .WillOnce(
          Invoke([&schema](const std::string &) -> meta::CollectionMetaPtr {
            return schema;
          }))
      .RetiresOnSaturation();
  ret = agent->create_collection(collection_name_);
  ASSERT_EQ(ret, 0);

  // process
  EXPECT_CALL(*meta_service_, get_current_collection(_))
      .Times(2)
      .WillOnce(
          Invoke([&schema](const std::string &) -> meta::CollectionMetaPtr {
            return schema;
          }))
      .WillOnce(
          Invoke([&schema](const std::string &) -> meta::CollectionMetaPtr {
            return schema;
          }))
      .RetiresOnSaturation();
  ret = agent->write(direct_request_);
  ASSERT_EQ(ret, 0);

  sleep(1);

  // drop collection
  ret = agent->drop_collection(collection_name_);
  ASSERT_EQ(ret, 0);

  ret = agent->stop();
  ASSERT_EQ(ret, 0);
  ret = agent->cleanup();
  ASSERT_EQ(ret, 0);
}

TEST_F(IndexAgentTest, TestWriteSuccessWithDirectRepeatedWrite) {
  IndexAgentPtr agent = IndexAgent::Create(meta_service_);
  int ret = agent->init();
  ASSERT_EQ(ret, 0);

  EXPECT_CALL(*meta_service_, get_latest_collections(_))
      .WillOnce(testing::Return(0));
  ret = agent->start();
  ASSERT_EQ(ret, 0);

  meta::CollectionMetaPtr schema = direct_schema_;
  // create collection
  EXPECT_CALL(*meta_service_, get_current_collection(_))
      .Times(1)
      .WillOnce(
          Invoke([&schema](const std::string &) -> meta::CollectionMetaPtr {
            return schema;
          }))
      .RetiresOnSaturation();
  ret = agent->create_collection(collection_name_);
  ASSERT_EQ(ret, 0);

  // process
  EXPECT_CALL(*meta_service_, get_current_collection(_))
      .Times(2)
      .WillOnce(
          Invoke([&schema](const std::string &) -> meta::CollectionMetaPtr {
            return schema;
          }))
      .WillOnce(
          Invoke([&schema](const std::string &) -> meta::CollectionMetaPtr {
            return schema;
          }))
      .RetiresOnSaturation();
  ret = agent->write(direct_request_);
  ASSERT_EQ(ret, 0);

  sleep(1);

  // process
  EXPECT_CALL(*meta_service_, get_current_collection(_))
      .Times(2)
      .WillOnce(
          Invoke([&schema](const std::string &) -> meta::CollectionMetaPtr {
            return schema;
          }))
      .WillOnce(
          Invoke([&schema](const std::string &) -> meta::CollectionMetaPtr {
            return schema;
          }))
      .RetiresOnSaturation();
  ret = agent->write(direct_request_);
  ASSERT_EQ(ret, ErrorCode_DuplicateKey);

  sleep(1);

  // drop collection
  ret = agent->drop_collection(collection_name_);
  ASSERT_EQ(ret, 0);

  ret = agent->stop();
  ASSERT_EQ(ret, 0);
  ret = agent->cleanup();
  ASSERT_EQ(ret, 0);
}

TEST_F(IndexAgentTest, TestWriteWithEmptyRequest) {
  IndexAgentPtr agent = IndexAgent::Create(meta_service_);
  int ret = agent->init();
  ASSERT_EQ(ret, 0);

  EXPECT_CALL(*meta_service_, get_latest_collections(_))
      .WillOnce(testing::Return(0));
  ret = agent->start();
  ASSERT_EQ(ret, 0);

  WriteRequest request;
  ret = agent->write(request);
  ASSERT_EQ(ret, 0);
}

TEST_F(IndexAgentTest, TestWriteFailedWithCollectionSuspend) {
  IndexAgentPtr agent = IndexAgent::Create(meta_service_);
  int ret = agent->init();
  ASSERT_EQ(ret, 0);

  EXPECT_CALL(*meta_service_, get_latest_collections(_))
      .WillOnce(testing::Return(0));
  ret = agent->start();
  ASSERT_EQ(ret, 0);

  meta::CollectionMetaPtr schema = proxy_schema_;
  // create collection
  EXPECT_CALL(*meta_service_, get_current_collection(_))
      .Times(1)
      .WillOnce(
          Invoke([&schema](const std::string &) -> meta::CollectionMetaPtr {
            return schema;
          }))
      .RetiresOnSaturation();
  ret = agent->create_collection(collection_name_);
  ASSERT_EQ(ret, 0);

  // process
  proxy_request_.set_magic_number(agent->agent_timestamp_);
  schema->set_writable(false);
  // create collection
  EXPECT_CALL(*meta_service_, get_current_collection(_))
      .Times(1)
      .WillOnce(
          Invoke([&schema](const std::string &) -> meta::CollectionMetaPtr {
            return schema;
          }))
      .RetiresOnSaturation();
  ret = agent->write(proxy_request_);
  ASSERT_EQ(ret, ErrorCode_SuspendedCollection);

  // drop collection
  ret = agent->drop_collection(collection_name_);
  ASSERT_EQ(ret, 0);

  ret = agent->stop();
  ASSERT_EQ(ret, 0);
  ret = agent->cleanup();
  ASSERT_EQ(ret, 0);
}

TEST_F(IndexAgentTest, TestWriteFailedWithMagicNumber) {
  IndexAgentPtr agent = IndexAgent::Create(meta_service_);
  int ret = agent->init();
  ASSERT_EQ(ret, 0);

  EXPECT_CALL(*meta_service_, get_latest_collections(_))
      .WillOnce(testing::Return(0));
  ret = agent->start();
  ASSERT_EQ(ret, 0);

  meta::CollectionMetaPtr schema = proxy_schema_;
  // create collection
  EXPECT_CALL(*meta_service_, get_current_collection(_))
      .Times(1)
      .WillOnce(
          Invoke([&schema](const std::string &) -> meta::CollectionMetaPtr {
            return schema;
          }))
      .RetiresOnSaturation();
  ret = agent->create_collection(collection_name_);
  ASSERT_EQ(ret, 0);

  // process
  EXPECT_CALL(*meta_service_, get_current_collection(_))
      .Times(1)
      .WillOnce(
          Invoke([&schema](const std::string &) -> meta::CollectionMetaPtr {
            return schema;
          }))
      .RetiresOnSaturation();
  ret = agent->write(proxy_request_);
  ASSERT_EQ(ret, ErrorCode_MismatchedMagicNumber);

  sleep(1);

  // drop collection
  ret = agent->drop_collection(collection_name_);
  ASSERT_EQ(ret, 0);

  ret = agent->stop();
  ASSERT_EQ(ret, 0);
  ret = agent->cleanup();
  ASSERT_EQ(ret, 0);
}

TEST_F(IndexAgentTest, TestWriteDatasetFailed) {
  IndexAgentPtr agent = IndexAgent::Create(meta_service_);
  int ret = agent->init();
  ASSERT_EQ(ret, 0);

  // load index service failed
  EXPECT_CALL(*meta_service_, get_latest_collections(_))
      .WillOnce(testing::Return(0));
  ret = agent->start();
  ASSERT_EQ(ret, 0);

  index::CollectionDatasetPtr record = std::make_shared<CollectionDataset>(0);
  CollectionCounterPtr counter = std::make_shared<CollectionCounter>();
  agent->write_dataset("invalid", record, counter.get());

  ret = agent->stop();
  ASSERT_EQ(ret, 0);
  ret = agent->cleanup();
  ASSERT_EQ(ret, 0);
}
