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


#include "meta/meta_agent.h"
#include <gtest/gtest.h>
#include "meta/meta_impl.h"
#include "meta/meta_service_builder.h"
#include "mock_meta_service.h"
#include "temp_file_inl.h"

using namespace proxima::be;
using namespace proxima::be::meta;

class MetaAgentTest : public testing::Test {
 protected:
  // Sets up the test fixture.
  void SetUp() override {
    meta_service_ = std::make_shared<MockMetaService>();
  }

  // Tears down the test fixture.
  void TearDown() override {
    meta_service_.reset();
  }

 protected:
  MockMetaServicePtr meta_service_;
};

TEST_F(MetaAgentTest, TestAgentCreate) {
  MockMetaServicePtr meta_service = nullptr;
  {
    auto agent = MetaAgent::Create(meta_service);
    EXPECT_FALSE(agent);
  }

  {
    meta_service = std::make_shared<MockMetaService>();
    auto agent = MetaAgent::Create(meta_service);
    EXPECT_TRUE(agent);
    EXPECT_TRUE(agent->get_service());
  }
}

TEST_F(MetaAgentTest, TestInitializeAndCleanup) {
  EXPECT_CALL(*meta_service_, init_impl())
      .Times(2)
      .WillOnce(Return(1))
      .WillOnce(Return(0))
      .RetiresOnSaturation();
  EXPECT_CALL(*meta_service_, cleanup_impl())
      .WillOnce(Return(0))
      .RetiresOnSaturation();

  EXPECT_CALL(*meta_service_, start_impl())
      .WillOnce(Return(0))
      .RetiresOnSaturation();
  EXPECT_CALL(*meta_service_, stop_impl())
      .WillOnce(Return(0))
      .RetiresOnSaturation();

  auto agent = MetaAgent::Create(meta_service_);
  EXPECT_EQ(agent->init(), 1);

  EXPECT_EQ(agent->init(), 0);
  EXPECT_EQ(agent->start(), 0);
  EXPECT_EQ(agent->stop(), 0);
  EXPECT_EQ(agent->cleanup(), 0);
}

TEST_F(MetaAgentTest, TestCollectionOperationWithMock) {
  {  // Test Create
    EXPECT_CALL(*meta_service_, create_collection(_, _))
        .WillOnce(Return(0))
        .WillOnce(Return(1))
        .RetiresOnSaturation();
    auto agent = MetaAgent::Create(meta_service_);
    CollectionBase create_param;
    EXPECT_EQ(agent->create_collection(create_param, nullptr), 0);
    EXPECT_EQ(agent->create_collection(create_param, nullptr), 1);
  }

  {  // Test Update
    EXPECT_CALL(*meta_service_, update_collection(_, _))
        .WillOnce(Return(1))
        .WillOnce(Return(0))
        .RetiresOnSaturation();

    auto agent = MetaAgent::Create(meta_service_);
    CollectionBase update_param;
    EXPECT_EQ(agent->update_collection(update_param, nullptr), 1);
    EXPECT_EQ(agent->update_collection(update_param, nullptr), 0);
  }

  {  // Test update_status
    EXPECT_CALL(*meta_service_, update_status(_, _))
        .WillOnce(Return(1))
        .WillOnce(Return(0))
        .RetiresOnSaturation();

    auto agent = MetaAgent::Create(meta_service_);
    std::string name("name");
    EXPECT_EQ(agent->update_status(name, CollectionStatus::SERVING), 1);
    EXPECT_EQ(agent->update_status(name, CollectionStatus::SERVING), 0);
  }

  {  // Test enable_collection
    EXPECT_CALL(*meta_service_, enable_collection(_, _, _))
        .WillOnce(Return(1))
        .WillOnce(Return(0))
        .RetiresOnSaturation();

    auto agent = MetaAgent::Create(meta_service_);
    std::string name("name");
    EXPECT_EQ(agent->enable_collection(name, 0), 1);
    EXPECT_EQ(agent->enable_collection(name, 0), 0);
  }

  {  // Test suspend_collection_read
    EXPECT_CALL(*meta_service_, suspend_collection_read(_))
        .WillOnce(Return(1))
        .WillOnce(Return(0))
        .RetiresOnSaturation();

    auto agent = MetaAgent::Create(meta_service_);
    std::string name("name");
    EXPECT_EQ(agent->suspend_collection_read(name), 1);
    EXPECT_EQ(agent->suspend_collection_read(name), 0);
  }

  {  // Test suspend_collection_read
    EXPECT_CALL(*meta_service_, resume_collection_read(_))
        .WillOnce(Return(1))
        .WillOnce(Return(0))
        .RetiresOnSaturation();

    auto agent = MetaAgent::Create(meta_service_);
    std::string name("name");
    EXPECT_EQ(agent->resume_collection_read(name), 1);
    EXPECT_EQ(agent->resume_collection_read(name), 0);
  }

  {  // Test suspend_collection_read
    EXPECT_CALL(*meta_service_, suspend_collection_write(_))
        .WillOnce(Return(1))
        .WillOnce(Return(0))
        .RetiresOnSaturation();

    auto agent = MetaAgent::Create(meta_service_);
    std::string name("name");
    EXPECT_EQ(agent->suspend_collection_write(name), 1);
    EXPECT_EQ(agent->suspend_collection_write(name), 0);
  }

  {  // Test suspend_collection_read
    EXPECT_CALL(*meta_service_, resume_collection_write(_))
        .WillOnce(Return(1))
        .WillOnce(Return(0))
        .RetiresOnSaturation();

    auto agent = MetaAgent::Create(meta_service_);
    std::string name("name");
    EXPECT_EQ(agent->resume_collection_write(name), 1);
    EXPECT_EQ(agent->resume_collection_write(name), 0);
  }

  {  // Test delete_collection
    EXPECT_CALL(*meta_service_, drop_collection(_))
        .WillOnce(Return(1))
        .WillOnce(Return(0))
        .RetiresOnSaturation();

    auto agent = MetaAgent::Create(meta_service_);
    std::string name;
    EXPECT_TRUE(agent->delete_collection(name) != 0);
    name.assign("name");
    EXPECT_EQ(agent->delete_collection(name), 1);
    EXPECT_EQ(agent->delete_collection(name), 0);
  }

  {  // Test delete_collection
    EXPECT_CALL(*meta_service_, get_latest_collections(_))
        .WillOnce(Return(1))
        .WillOnce(Return(0))
        .RetiresOnSaturation();

    auto agent = MetaAgent::Create(meta_service_);
    EXPECT_TRUE(agent->list_collections(nullptr) != 0);
    CollectionMetaPtrList collections;
    EXPECT_EQ(agent->list_collections(&collections), 1);
    EXPECT_EQ(agent->list_collections(&collections), 0);
  }

  {  // Test delete_collection
    EXPECT_CALL(*meta_service_, get_collections(_, _))
        .WillOnce(Return(1))
        .WillOnce(Return(0))
        .RetiresOnSaturation();

    auto agent = MetaAgent::Create(meta_service_);
    std::string name;
    CollectionMetaPtrList collections;
    EXPECT_TRUE(agent->get_collection_history(name, &collections) != 0);
    name.assign("name");
    EXPECT_EQ(agent->get_collection_history(name, &collections), 1);
    EXPECT_EQ(agent->get_collection_history(name, &collections), 0);
  }

  {  // Test delete_collection
    EXPECT_CALL(*meta_service_, get_current_collection(_))
        .WillOnce(Return(nullptr))
        .RetiresOnSaturation();

    auto agent = MetaAgent::Create(meta_service_);
    std::string name;
    EXPECT_FALSE(agent->get_collection(name));
    name.assign("name");
    EXPECT_FALSE(agent->get_collection(name));
  }

  {  // Test delete_collection
    EXPECT_CALL(*meta_service_, exist_collection(_))
        .WillOnce(Return(false))
        .WillOnce(Return(true))
        .RetiresOnSaturation();

    auto agent = MetaAgent::Create(meta_service_);
    std::string name;
    EXPECT_FALSE(agent->exist_collection(name));
    name.assign("name");
    EXPECT_FALSE(agent->exist_collection(name));
    EXPECT_TRUE(agent->exist_collection(name));
  }

  //// Valid URI
  // pb_meta.set_connection_uri("mysql://host:8080/connection_uri");
  //{  // no index column
  //  EXPECT_CALL(*meta_service_, create_collection(_, _))
  //      .WillOnce(Return(0))
  //      .RetiresOnSaturation();
  //
  //  auto agent = MetaAgent::Create(meta_service_);
  //  int code = agent->create_collection(pb_meta, nullptr);
  //  EXPECT_TRUE(code != 0);
  //}

  //{  // Set Column
  //  auto pb_column = std::make_shared<meta::CreateColumnParam>();
  //  pb_column->set_name("column");
  //  pb_column->mutable_parameters()->set("key", "10");
  //  pb_meta.append(pb_column);
  //}
}

TEST_F(MetaAgentTest, IntegrationTest) {
  ScopeFile database(TempFile());
  std::string db_uri("sqlite://");
  std::cout << "Create temporary file: " << database.file_ << std::endl;

  auto meta_service = MetaServiceBuilder::Create(db_uri.append(database));
  auto agent = MetaAgent::Create(meta_service);
  {  // Check Agent
    EXPECT_TRUE(agent);
    EXPECT_TRUE(agent->get_service());

    EXPECT_EQ(agent->init(), 0);
    EXPECT_EQ(agent->start(), 0);
  }

  meta::CollectionBase create_param;
  {
    create_param.set_name("collection");
    create_param.mutable_forward_columns()->push_back("forward1");
    create_param.mutable_forward_columns()->push_back("forward2");
    create_param.set_max_docs_per_segment(10);

    auto repo = std::make_shared<DatabaseRepositoryMeta>();
    repo->set_name("repo_name");
    // Invalid URI
    repo->set_connection("invalid_connection");
    repo->set_user("user");
    repo->set_password("password");
    repo->set_table_name("table_name");

    create_param.set_repository(repo);
  }
  {  // Test Empty Columns
    meta::CollectionMetaPtr meta;
    EXPECT_EQ(agent->create_collection(create_param, &meta),
              PROXIMA_BE_ERROR_CODE(EmptyColumns));
  }
  {  // Set Column 1
    auto column = std::make_shared<meta::ColumnMeta>();
    column->set_name("column1");
    column->mutable_parameters()->set("key", "10");
    create_param.append(column);
  }
  {  // Test undefined data type
    meta::CollectionMetaPtr meta;
    EXPECT_EQ(agent->create_collection(create_param, &meta),
              PROXIMA_BE_ERROR_CODE(InvalidDataType));
    create_param.mutable_index_columns()->clear();

    auto column = std::make_shared<meta::ColumnMeta>();
    column->set_name("column1");
    column->set_data_type(DataTypes::VECTOR_INT8);
    column->mutable_parameters()->set("key", "10");
    create_param.append(column);
  }
  {  // Test Invalid URI
    meta::CollectionMetaPtr meta;
    EXPECT_EQ(agent->create_collection(create_param, &meta),
              PROXIMA_BE_ERROR_CODE(InvalidURI));
    RepositoryHelper::Child<DatabaseRepositoryMeta>(create_param.repository())
        ->set_connection("mysql://host:8080/connection_uri");
  }

  meta::CollectionMetaPtr meta;
  EXPECT_EQ(agent->create_collection(create_param, &meta), 0);
  std::cout << "Create collection" << std::endl;
  std::cout << "  revision: " << meta->revision() << std::endl;
  std::cout << "  uid: " << meta->uid() << std::endl;
  std::cout << "  is_current: " << meta->is_current() << std::endl;
  std::cout << "  readable: " << meta->readable() << std::endl;
  std::cout << "  writable: " << meta->writable() << std::endl;
  std::cout << " Column: " << std::endl;
  auto column = meta->index_columns().begin();
  std::cout << "  uid: " << (*column)->uid() << std::endl;

  {
    CollectionBase update_param(create_param);
    update_param.repository()->set_name("updated");
    meta::CollectionMetaPtr meta1;
    EXPECT_EQ(agent->update_collection(update_param, &meta1), 0);
    EXPECT_TRUE(meta1);
    EXPECT_EQ(meta1->revision(), 1);
    EXPECT_EQ(meta1->repository()->name(), "updated");
    EXPECT_EQ(meta1->uid(), meta->uid());
    auto left = *meta1->index_columns().begin();
    auto right = *meta->index_columns().begin();
    std::cout << "  readable: " << meta1->readable() << std::endl;
    std::cout << "  writable: " << meta1->writable() << std::endl;
    EXPECT_EQ(left->uid(), right->uid());
    EXPECT_EQ(CollectionStatus::INITIALIZED, meta1->status());
  }

  {  // Test List Collection
    std::string collection("collections2");
    CollectionMetaPtrList collections;
    int code = agent->get_collection_history(collection, &collections);
    EXPECT_TRUE(code != 0);
    EXPECT_EQ(collections.size(), 0);

    collection.assign("collection");
    code = agent->get_collection_history(collection, &collections);
    EXPECT_EQ(code, 0);
    EXPECT_EQ(collections.size(), 2);

    collections.clear();
    code = agent->list_collections(&collections);
    EXPECT_EQ(code, 0);
    EXPECT_EQ(collections.size(), 1);
  }

  {  // Test update status
    std::string collection("collection");
    EXPECT_EQ(agent->update_status(collection, CollectionStatus::SERVING), 0);
    auto current = agent->get_collection(collection);
    EXPECT_EQ(current->status(), CollectionStatus::SERVING);
  }

  {  // Test enable_collection
    std::string collection("collection");
    auto previous = agent->get_collection(collection);
    std::cout << previous->revision() << std::endl;
    // Test not exists revision
    EXPECT_TRUE(agent->enable_collection(collection, 3) != 0);
    // Updated revision
    EXPECT_EQ(agent->enable_collection(collection, 1), 0);
    auto current = agent->get_collection(collection);
    std::cout << current->revision() << std::endl;
    EXPECT_NE(previous->revision(), current->revision());
    EXPECT_TRUE(current->writable());
    EXPECT_TRUE(current->readable());
    EXPECT_EQ(agent->enable_collection(collection, 0), 0);
    EXPECT_FALSE(current->writable());
    EXPECT_FALSE(current->readable());
  }

  {  // Test suspend_collection_read and resume_collection_read
    std::string collection("collection");
    EXPECT_EQ(agent->suspend_collection_read(collection), 0);
    auto current = agent->get_collection(collection);
    EXPECT_FALSE(current->readable());
    EXPECT_EQ(agent->resume_collection_read(collection), 0);
    current = agent->get_collection(collection);
    EXPECT_TRUE(current->readable());
    // Test does not exist collection
    collection.assign("abc");
    EXPECT_NE(agent->suspend_collection_read(collection), 0);
    EXPECT_NE(agent->resume_collection_read(collection), 0);
  }

  {  // Test suspend_collection_write and resume_collection_write
    std::string collection("collection");
    EXPECT_EQ(agent->suspend_collection_write(collection), 0);
    auto current = agent->get_collection(collection);
    EXPECT_FALSE(current->writable());
    EXPECT_EQ(agent->resume_collection_write(collection), 0);
    current = agent->get_collection(collection);
    EXPECT_TRUE(current->writable());
    // Test does not exist collection
    collection.assign("abc");
    EXPECT_NE(agent->suspend_collection_write(collection), 0);
    EXPECT_NE(agent->resume_collection_write(collection), 0);
  }

  {  // Test exist_collection
    std::string collection("collection");
    EXPECT_TRUE(agent->exist_collection(collection));
    collection.assign("abc");
    EXPECT_FALSE(agent->exist_collection(collection));
  }

  {  // Test Drop Collection
    std::string collection;
    int code = agent->delete_collection(collection);
    EXPECT_TRUE(code != 0);
    collection.assign("collection");
    code = agent->delete_collection(collection);
    EXPECT_EQ(code, 0);

    CollectionMetaPtrList collections;
    code = agent->list_collections(&collections);
    EXPECT_EQ(code, 0);
    EXPECT_EQ(collections.size(), 0);


    collections.clear();
    code = agent->get_collection_history(collection, &collections);
    EXPECT_TRUE(code != 0);
    EXPECT_EQ(collections.size(), 0);
  }

  {  // Cleanup agent
    EXPECT_EQ(agent->stop(), 0);
    EXPECT_EQ(agent->cleanup(), 0);
  }
}
