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


#include "meta/meta_cache.h"
#include <ailego/io/file.h>
#include <gtest/gtest.h>

using namespace proxima::be;
using namespace proxima::be::meta;

TEST(MetaCacheTest, TestFunction) {
  MetaCache cache;

  EXPECT_TRUE(cache.append_column(nullptr) != 0);
  EXPECT_TRUE(cache.append_collection(nullptr) != 0);

  CollectionMeta meta;
  meta.set_name("name");
  meta.set_uid("uid");
  meta.mutable_forward_columns()->assign({"forward1", "forward2"});
  meta.set_max_docs_per_segment(10);
  meta.set_revision(10);
  meta.set_status(CollectionStatus::SERVING);
  meta.set_current(false);


  CollectionImplPtr collection = std::make_shared<CollectionImpl>(meta);
  cache.append_collection(collection);


  EXPECT_TRUE(cache.exist_collection(collection->name()));

  CollectionMetaPtrList collections;
  cache.get_collections(&collections);
  // Only list enabled collection
  ASSERT_EQ(collections.size(), 0);

  cache.get_collections(MetaCache::PassAllFilter, &collections);
  // List all collections
  ASSERT_EQ(collections.size(), 1);

  collections.clear();
  // List collection with specific name
  cache.get_collections(collection->name(), &collections);
  EXPECT_EQ(collections.size(), 1);

  collections.clear();
  cache.get_collections_by_repo("xxx", &collections);
  EXPECT_EQ(collections.size(), 0);

  // Only list enabled collection
  CollectionImplPtr current = cache.get_collection(collection->name());
  EXPECT_TRUE(!current);

  meta.set_current();
  meta.set_status(CollectionStatus::SERVING);
  current = std::make_shared<CollectionImpl>(meta);
  // Append one enabled collection
  cache.append_collection(current);

  current.reset();
  current = cache.get_collection(collection->name());
  EXPECT_TRUE(current);

  collections.clear();
  cache.get_collections(&collections);
  // Only list enabled collection
  EXPECT_EQ(collections.size(), 1);

  collections.clear();
  cache.get_collections(MetaCache::PassAllFilter, &collections);
  // List all collections
  EXPECT_EQ(collections.size(), 2);

  cache.delete_collection(collection->name());
  collections.clear();
  cache.get_collections(MetaCache::PassAllFilter, &collections);
  // List all collections
  EXPECT_EQ(collections.size(), 0);


  // Test column
  cache.append_collection(current);

  collections.clear();
  cache.get_collections(MetaCache::PassAllFilter, &collections);
  // List all collections
  EXPECT_EQ(collections.size(), 1);
  EXPECT_TRUE((*collections.begin())->is_current());

  ColumnMeta column_meta;
  column_meta.set_name("name");
  column_meta.set_index_type(IndexTypes::UNDEFINED);
  column_meta.set_data_type(DataTypes::UNDEFINED);

  ColumnImplPtr column = std::make_shared<ColumnImpl>(column_meta);
  EXPECT_TRUE(cache.append_column(column) != 0);

  column->set_collection_uuid(current->uuid());
  EXPECT_EQ(cache.append_column(column), 0);

  current.reset();
  current = cache.get_collection(collection->name());
  auto &columns = current->columns();
  EXPECT_EQ(columns.size(), 1);

  DatabaseRepositoryMeta repo;
  repo.set_name("repo");
  repo.set_user("user");
  auto repo_ptr = std::make_shared<DatabaseRepositoryImpl>(
      current->uid(), current->uuid(), repo);
  cache.append_repository(repo_ptr);
  collections.clear();
  cache.get_collections_by_repo(repo.name(), &collections);
  EXPECT_EQ(collections.size(), 1);
}
