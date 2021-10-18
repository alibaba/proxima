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

#include "meta/meta_store_factory.h"
#include <ailego/io/file.h>
#include <gtest/gtest.h>

using namespace proxima::be;
using namespace proxima::be::meta;

namespace proxima {
namespace be {
namespace meta {

class TestStore : public MetaStore {
  //! initialize metastore
  int initialize(const ailego::Uri *uri) override {
    return 0;
  }

  //! cleanup
  int cleanup() override {
    return 0;
  }

  //! Create the collection
  int create_collection(const CollectionObject &collection) override {
    return 0;
  }

  //! Update collection declaration
  int update_collection(const CollectionObject &collection) override {
    return 0;
  }

  //! Delete collection
  int delete_collection(const std::string &name) override {
    return 0;
  }

  //! Delete collection
  int delete_collection_by_uuid(const std::string &name) override {
    return 0;
  }

  //! Retrieve all collections
  int list_collections(CollectionAllocator allocator) const override {
    return 0;
  }

  /** CRUD of ColumnMeta **/
  //! Create column
  int create_column(const ColumnObject &column) override {
    return 0;
  }

  //! Delete columns
  int delete_columns_by_uid(const std::string &uid) override {
    return 0;
  }

  //! Delete columns
  int delete_columns_by_uuid(const std::string &uuid) override {
    return 0;
  }

  //! Retrieve all collections
  int list_columns(ColumnAllocator allocator) const override {
    return 0;
  }

  //! Create repository
  int create_repository(const DatabaseRepositoryObject &repository) override {
    return 0;
  }

  //! Delete repositories
  int delete_repositories_by_uid(const std::string &uid) override {
    return 0;
  }

  //! Delete repositories
  int delete_repositories_by_uuid(const std::string &uuid) override {
    return 0;
  }

  //! Retrieve all repositories
  int list_repositories(DatabaseRepositoryAllocator allocator) const override {
    return 0;
  }


  //! Flush all changes to storage
  int flush() const override {
    return 0;
  }
};
META_FACTORY_REGISTER_INSTANCE_ALIAS(test, TestStore);
}  // namespace meta
}  // namespace be
}  // namespace proxima

TEST(MetaStoreFactoryTest, TestCreate) {
  ailego::Uri uri;
  auto ptr = MetaStoreFactory::Instance().create("test", &uri);
  ASSERT_TRUE(ptr);

  ptr = MetaStoreFactory::Instance().create("test1", &uri);
  ASSERT_FALSE(ptr);

  ptr = MetaStoreFactory::Instance().create("Test", &uri);
  ASSERT_FALSE(ptr);
}
