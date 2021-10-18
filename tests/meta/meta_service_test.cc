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

#include <gtest/gtest.h>
#include "common/uuid_helper.h"
#include "meta/meta_service_builder.h"
#include "temp_file_inl.h"

using namespace proxima::be;
using namespace proxima::be::meta;
using namespace testing;

class MetaServiceTest : public Test {
 public:
  // Sets up the test fixture.
  static void SetUpTestSuite() {
    database_ = TempFile();
    std::string uri = "sqlite://" + database_;
    meta_ = MetaServiceBuilder::Create(uri);
    EXPECT_TRUE(meta_);
    EXPECT_EQ(meta_->init(), 0);
    EXPECT_EQ(meta_->start(), 0);
  }

  // Tears down the test fixture.
  static void TearDownTestSuite() {
    EXPECT_EQ(meta_->stop(), 0);
    EXPECT_EQ(meta_->cleanup(), 0);
    ailego::File::Delete(database_);
  }

 protected:
  static std::string database_;
  static MetaServicePtr meta_;
};

std::string MetaServiceTest::database_;
MetaServicePtr MetaServiceTest::meta_;

TEST_F(MetaServiceTest, TestMetaServiceFunction) {
  auto meta_service = meta_;

  {  // TEST Empty MetaService
    // Drop collection
    EXPECT_EQ(meta_service->drop_collection("xxx"), 0);
    // Test none exists collection
    auto collection = meta_service->get_collection("xxx", 0);
    EXPECT_FALSE(collection);
    // Test empty collections
    CollectionMetaPtrList collections;
    EXPECT_EQ(meta_service->get_collections(&collections), 0);
    EXPECT_TRUE(collections.empty());
    // Test empty collections
    EXPECT_TRUE(meta_service->get_collections("xxx", &collections) != 0);
    EXPECT_TRUE(collections.empty());
    // Test empty collections
    EXPECT_FALSE(meta_service->get_current_collection("xx"));
    // Test empty collections
    EXPECT_EQ(meta_service->get_latest_collections(&collections), 0);
    EXPECT_TRUE(collections.empty());
    // Test empty collections
    EXPECT_EQ(meta_service->get_collections_by_repo("xxx", &collections), 0);
    EXPECT_TRUE(collections.empty());
    // Test update status
    EXPECT_TRUE(
        meta_service->update_status("xxx", CollectionStatus::INITIALIZED) != 0);
    // Test update collection
    CollectionMetaPtr meta;
    CollectionBase param;
    EXPECT_TRUE(meta_service->update_collection(param, &meta) != 0);
    // Test has collection
    EXPECT_FALSE(meta_service->exist_collection("does not exist collection"));
  }

  {
    CollectionBase param;
    {
      param.set_name("name");
      param.mutable_forward_columns()->assign({"forward1", "forward2"});
      param.set_max_docs_per_segment(10);

      auto column1 = std::make_shared<ColumnMeta>();
      column1->set_name("column_name");
      column1->set_index_type(IndexTypes::PROXIMA_GRAPH_INDEX);
      column1->set_data_type(DataTypes::VECTOR_FP32);
      column1->mutable_parameters()->insert("abc", "abc");
      param.append(column1);

      // For Issue 32264482, create collection with multi-columns
      auto column2 = std::make_shared<ColumnMeta>();
      column2->set_name("column_name_a");
      column2->set_index_type(IndexTypes::PROXIMA_GRAPH_INDEX);
      column2->set_data_type(DataTypes::VECTOR_FP32);
      column2->mutable_parameters()->insert("abc", "abc");
      param.append(column2);

      auto db = std::make_shared<DatabaseRepositoryMeta>();
      db->set_name("db");
      db->set_connection("mysql://host:1234/test_db");
      db->set_user("user");
      db->set_password("password");
      db->set_table_name("table_name");
      param.set_repository(db);
    }

    CollectionMetaPtr meta;

    EXPECT_EQ(meta_service->create_collection(param, &meta), 0);
    // Test has collection
    EXPECT_TRUE(meta_service->exist_collection(meta->name()));
    // Issue (32201692): meta should be updated, after create collection.
    EXPECT_EQ(meta->revision(), 0);
    EXPECT_TRUE(valid_uuid(meta->uid()));
    EXPECT_TRUE(meta->is_current());
    auto collection = meta_service->get_collection(meta->name(), 0);
    ASSERT_TRUE(collection);
    EXPECT_TRUE(collection->readable());
    EXPECT_TRUE(collection->writable());

    collection = meta_service->get_current_collection(meta->name());
    EXPECT_TRUE(collection);

    CollectionMetaPtrList collections;
    EXPECT_EQ(meta_service->get_collections(&collections), 0);
    EXPECT_FALSE(collections.empty());
    EXPECT_EQ(collections.size(), 1);

    CollectionMeta update_param(*collection);
    update_param.set_uid("updated_uid");
    EXPECT_EQ(meta_service->update_collection(update_param, &meta), 0);
    EXPECT_TRUE(meta->status() == CollectionStatus::INITIALIZED);
    collections.clear();
    EXPECT_EQ(meta_service->get_collections(&collections), 0);
    EXPECT_FALSE(collections.empty());
    EXPECT_EQ(collections.size(), 2);

    int enabled = 0, disabled = 0;
    for (auto &c : collections) {
      if (c->is_current()) {
        enabled++;
      } else {
        disabled++;
      }
    }
    EXPECT_EQ(enabled, 1);
    EXPECT_EQ(disabled, 1);

    EXPECT_EQ(meta_service->drop_collection(meta->name()), 0);
    collections.clear();
    EXPECT_EQ(meta_service->get_collections(&collections), 0);
    EXPECT_TRUE(collections.empty());
    EXPECT_FALSE(meta_service->exist_collection(meta->name()));

    // ------------------- Bug Fix ------------------------
    // For issue: #32086561
    // Test create - drop - create - get_collections_by_repo failed
    EXPECT_EQ(meta_service->create_collection(param, &meta), 0);
    EXPECT_EQ(
        meta_service->enable_collection(meta->name(), meta->revision(), true),
        0);
    collections.clear();
    collection = meta_service->get_current_collection(meta->name());
    EXPECT_TRUE(collection);
    EXPECT_TRUE(collection->repository());

    EXPECT_EQ(meta_service->get_collections_by_repo(meta->repository()->name(),
                                                    &collections),
              0);
    EXPECT_TRUE(!collections.empty());
    EXPECT_EQ(meta_service->drop_collection(meta->name()), 0);

    // For Issue: #32422770
    EXPECT_EQ(meta_service->create_collection(param, &meta), 0);
    {  // Test name changed
      CollectionMeta updated_meta(*meta);
      updated_meta.set_name("abc");
      EXPECT_TRUE(meta_service->update_collection(updated_meta, &meta) != 0);
    }
    {  // Test column changed
      CollectionMeta updated_meta(*meta);
      auto c = *updated_meta.mutable_index_columns()->begin();
      c->set_data_type(DataTypes::VECTOR_INT8);
      EXPECT_TRUE(meta_service->update_collection(updated_meta, &meta) != 0);
    }
    {  // Test name of column changed, uid should be updated
      CollectionMeta updated_meta(*meta);
      auto c = *updated_meta.mutable_index_columns()->begin();
      c->set_name("updated_name");
      EXPECT_TRUE(meta_service->update_collection(updated_meta, &meta) == 0);
      auto origin_column = *meta->mutable_index_columns()->begin();
      EXPECT_TRUE(c->uid() != origin_column->uid());
    }
    collections.clear();
  }
}

void create_collection_meta(const std::string &name, CollectionBase *meta) {
  meta->set_name(name);
  meta->mutable_forward_columns()->assign({"forward1", "forward2"});
  meta->set_max_docs_per_segment(10);

  auto column1 = std::make_shared<ColumnMeta>();
  column1->set_name("column_name");
  column1->set_index_type(IndexTypes::PROXIMA_GRAPH_INDEX);
  column1->set_data_type(DataTypes::VECTOR_FP32);
  column1->mutable_parameters()->insert("abc", "abc");
  meta->append(column1);

  auto db = std::make_shared<DatabaseRepositoryMeta>();
  db->set_name("db");
  db->set_connection("mysql://host:1234/test_db");
  db->set_user("user");
  db->set_password("password");
  db->set_table_name("table_name");
  meta->set_repository(db);
}


TEST_F(MetaServiceTest, TestMultiThreads) {
  auto meta_service = meta_;
  ailego::ThreadPool thread_pool(10, false);
  // test multithread insert records
  for (size_t i = 0; i < 5; i++) {
    thread_pool.execute(
        [&meta_service](int i) {
          for (int j = 0; j < 10; j++) {
            CollectionBase meta;
            std::string name =
                "name_" + std::to_string(j) + "_thread_" + std::to_string(i);
            create_collection_meta(name, &meta);
            int code = meta_service->create_collection(meta, nullptr);
            EXPECT_EQ(code, 0);
          }
        },
        i);
  }

  thread_pool.wait_finish();
}
