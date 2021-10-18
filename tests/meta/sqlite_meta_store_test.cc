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
 *   \date     Nov 2020
 *   \brief
 */


#include "meta/sqlite/sqlite_meta_store.h"
#include <gtest/gtest.h>
#include "meta/meta_impl.h"
#include "temp_file_inl.h"

using namespace proxima::be;
using namespace proxima::be::meta;
using namespace proxima::be::meta::sqlite;

TEST(SqliteMetaStoreTest, TestInitialize) {
  SQLiteMetaStore store;
  ScopeFile database(TempFile());
  std::string database_uri("sqlite://");
  database_uri.append(database);

  ailego::Uri uri;

  // Invalid uri
  ailego::Uri::Parse("sqlite:./test.sqlite", &uri);
  // Test Init and Cleanup
  EXPECT_EQ(store.initialize(&uri), PROXIMA_BE_ERROR_CODE(RuntimeError));

  ailego::Uri::Parse(database_uri, &uri);
  // Test Init and Cleanup
  EXPECT_EQ(store.initialize(&uri), 0);
  EXPECT_EQ(store.cleanup(), 0);

  // Double Init
  EXPECT_EQ(store.initialize(&uri), 0);
  EXPECT_EQ(store.initialize(&uri), 0);
}

TEST(SqliteMetaStoreTest, TestCollectionFunction) {
  SQLiteMetaStore store;
  ScopeFile database(TempFile());
  std::string database_uri("sqlite://");
  ailego::Uri uri(database_uri.append(database));

  // Init store
  EXPECT_EQ(store.initialize(&uri), 0);

  CollectionMeta meta;
  meta.set_name("name");
  meta.set_uid("uid");
  meta.mutable_forward_columns()->assign({"forward1", "forward2"});
  meta.set_max_docs_per_segment(10);
  meta.set_revision(10);
  meta.set_status(CollectionStatus::INITIALIZED);
  meta.set_current(false);

  CollectionImplPtr collection = std::make_shared<CollectionImpl>(meta);
  EXPECT_EQ(store.create_collection(*collection), 0);

  CollectionImplPtr collection_record = std::make_shared<CollectionImpl>();
  int row_count = 0;
  EXPECT_EQ(store.list_collections(
                [&collection_record, &row_count]() -> CollectionObject * {
                  row_count++;
                  return collection_record.get();
                }),
            0);

  EXPECT_EQ(row_count, 1);
  EXPECT_EQ(meta.max_docs_per_segment(),
            collection_record->max_docs_per_segment());
  EXPECT_EQ(meta.revision(), collection_record->revision());


  {
    // Test update collection
    collection_record->set_status(20);
    collection_record->set_uid("updated_uid");
    EXPECT_EQ(store.update_collection(*collection_record), 0);
    row_count = 0;
    CollectionImplPtr collection_updated = std::make_shared<CollectionImpl>();
    EXPECT_EQ(store.list_collections(
                  [&collection_updated, &row_count]() -> CollectionObject * {
                    row_count++;
                    return collection_updated.get();
                  }),
              0);
    EXPECT_EQ(collection_record->uid(), collection_updated->uid());
    EXPECT_EQ(collection_record->status(), collection_updated->status());
    EXPECT_EQ(collection_record->uuid(), collection_updated->uuid());
    EXPECT_EQ(collection_updated->status(), 20);
  }

  {  // Test Delete Collection
    CollectionImplPtr collection_insert =
        std::make_shared<CollectionImpl>(meta);
    EXPECT_EQ(store.create_collection(*collection_insert), 0);

    CollectionImplPtr fetch_collection = std::make_shared<CollectionImpl>();
    row_count = 0;
    EXPECT_EQ(store.list_collections(
                  [&fetch_collection, &row_count]() -> CollectionObject * {
                    row_count++;
                    return fetch_collection.get();
                  }),
              0);

    EXPECT_EQ(row_count, 2);

    // Delete by uuid
    EXPECT_EQ(store.delete_collection_by_uuid(collection_insert->uuid()), 0);

    row_count = 0;
    EXPECT_EQ(store.list_collections(
                  [&fetch_collection, &row_count]() -> CollectionObject * {
                    row_count++;
                    return fetch_collection.get();
                  }),
              0);
    EXPECT_EQ(row_count, 1);

    // Delete by name
    EXPECT_EQ(store.delete_collection(collection_insert->name()), 0);

    row_count = 0;
    EXPECT_EQ(store.list_collections(
                  [&fetch_collection, &row_count]() -> CollectionObject * {
                    row_count++;
                    return fetch_collection.get();
                  }),
              0);
    EXPECT_EQ(row_count, 0);
  }
}

TEST(SqliteMetaStoreTest, TestColumnFunction) {
  SQLiteMetaStore store;
  ScopeFile database(TempFile());
  std::string database_uri("sqlite://");
  ailego::Uri uri(database_uri.append(database));

  // Init store
  EXPECT_EQ(store.initialize(&uri), 0);

  ColumnMeta meta;
  meta.set_name("name");
  meta.set_data_type(DataTypes::VECTOR_BINARY64);
  meta.set_index_type(IndexTypes::UNDEFINED);
  meta.mutable_parameters()->insert("abc", "abc");

  ColumnImplPtr column = std::make_shared<ColumnImpl>("uid", "uuid", meta);
  EXPECT_EQ(store.create_column(*column), 0);
  EXPECT_EQ(store.delete_columns_by_uid("abc"), 0);
  EXPECT_EQ(store.delete_columns_by_uuid("abc"), 0);

  {
    ColumnImplPtr column_record = std::make_shared<ColumnImpl>();
    int row_count = 0;
    EXPECT_EQ(
        store.list_columns([&column_record, &row_count]() -> ColumnObject * {
          row_count++;
          return column_record.get();
        }),
        0);
    EXPECT_EQ(row_count, 1);
    EXPECT_EQ(column_record->collection_uid(), column->collection_uid());
    EXPECT_EQ(column_record->collection_uuid(), column->collection_uuid());
    EXPECT_EQ(store.delete_columns_by_uuid(column->collection_uuid()), 0);

    row_count = 0;
    EXPECT_EQ(
        store.list_columns([&column_record, &row_count]() -> ColumnObject * {
          row_count++;
          return column_record.get();
        }),
        0);
    EXPECT_EQ(row_count, 0);

    EXPECT_EQ(store.create_column(*column), 0);
    EXPECT_EQ(store.delete_columns_by_uid(column->collection_uid()), 0);

    row_count = 0;
    EXPECT_EQ(
        store.list_columns([&column_record, &row_count]() -> ColumnObject * {
          row_count++;
          return column_record.get();
        }),
        0);
    EXPECT_EQ(row_count, 0);
  }
}

TEST(SqliteMetaStoreTest, TestRepositoryFunction) {
  SQLiteMetaStore store;
  ScopeFile database(TempFile());
  std::string database_uri("sqlite://");
  ailego::Uri uri(database_uri.append(database));

  // Init store
  EXPECT_EQ(store.initialize(&uri), 0);

  DatabaseRepositoryMeta meta;
  meta.set_name("repo_name");
  meta.set_user("user");
  meta.set_password("password");
  meta.set_connection("invalid_uri");
  meta.set_table_name("table_name");

  auto repo = std::make_shared<DatabaseRepositoryImpl>(meta);

  EXPECT_FALSE(repo->collection_uid().empty());
  EXPECT_FALSE(repo->collection_uuid().empty());

  int row_count = 0;
  {  // Test Create and List and Drop by uid
    EXPECT_EQ(store.create_repository(*repo), 0);

    auto listed_repo = std::make_shared<DatabaseRepositoryImpl>();
    EXPECT_EQ(store.list_repositories(
                  [&listed_repo, &row_count]() -> DatabaseRepositoryObject * {
                    row_count++;
                    return listed_repo.get();
                  }),
              0);
    // Test List
    EXPECT_EQ(row_count, 1);

    EXPECT_EQ(repo->name(), listed_repo->name());
    EXPECT_EQ(repo->collection_uid(), listed_repo->collection_uid());
    EXPECT_EQ(repo->collection_uuid(), listed_repo->collection_uuid());
    EXPECT_EQ(repo->table(), listed_repo->table());
    EXPECT_EQ(repo->connection(), listed_repo->connection());
    EXPECT_EQ(repo->user(), listed_repo->user());
    EXPECT_EQ(repo->password(), listed_repo->password());

    EXPECT_EQ(store.delete_repositories_by_uid(repo->collection_uid()), 0);
    row_count = 0;
    listed_repo = std::make_shared<DatabaseRepositoryImpl>();
    EXPECT_EQ(store.list_repositories(
                  [&listed_repo, &row_count]() -> DatabaseRepositoryObject * {
                    row_count++;
                    return listed_repo.get();
                  }),
              0);
    // Test List
    EXPECT_EQ(row_count, 0);
  }


  {  // Test Create and List and Drop by uuid
    EXPECT_EQ(store.create_repository(*repo), 0);

    auto listed_repo = std::make_shared<DatabaseRepositoryImpl>();
    EXPECT_EQ(store.list_repositories(
                  [&listed_repo, &row_count]() -> DatabaseRepositoryObject * {
                    row_count++;
                    return listed_repo.get();
                  }),
              0);
    // Test List
    EXPECT_EQ(row_count, 1);

    EXPECT_EQ(store.delete_repositories_by_uuid(repo->collection_uuid()), 0);
    row_count = 0;
    EXPECT_EQ(store.list_repositories(
                  [&listed_repo, &row_count]() -> DatabaseRepositoryObject * {
                    row_count++;
                    return listed_repo.get();
                  }),
              0);
    // Test List
    EXPECT_EQ(row_count, 0);
  }

  {  // Test Create and List and Drop by uuid
    EXPECT_EQ(store.create_repository(*repo), 0);

    auto listed_repo = std::make_shared<DatabaseRepositoryImpl>();
    EXPECT_EQ(store.list_repositories(
                  [&listed_repo, &row_count]() -> DatabaseRepositoryObject * {
                    row_count++;
                    return listed_repo.get();
                  }),
              0);
    // Test List
    EXPECT_EQ(row_count, 1);

    EXPECT_EQ(store.delete_repositories_by_uuid(repo->collection_uuid()), 0);
    row_count = 0;
    EXPECT_EQ(store.list_repositories(
                  [&listed_repo, &row_count]() -> DatabaseRepositoryObject * {
                    row_count++;
                    return listed_repo.get();
                  }),
              0);
    // Test List
    EXPECT_EQ(row_count, 0);
  }

  {  // Create multi repositories
    EXPECT_EQ(store.create_repository(*repo), 0);
    for (int i = 0; i < 9; i++) {
      auto create_repo = std::make_shared<DatabaseRepositoryImpl>(meta);
      create_repo->set_collection_uuid(repo->collection_uuid());
      EXPECT_EQ(store.create_repository(*create_repo), 0);
    }

    row_count = 0;
    auto listed_repo = std::make_shared<DatabaseRepositoryImpl>();
    EXPECT_EQ(store.list_repositories(
                  [&listed_repo, &row_count]() -> DatabaseRepositoryObject * {
                    row_count++;
                    return listed_repo.get();
                  }),
              0);
    // Test List
    EXPECT_EQ(row_count, 10);

    EXPECT_EQ(store.delete_repositories_by_uid(repo->collection_uid()), 0);
    row_count = 0;
    EXPECT_EQ(store.list_repositories(
                  [&listed_repo, &row_count]() -> DatabaseRepositoryObject * {
                    row_count++;
                    return listed_repo.get();
                  }),
              0);
    // Test List
    EXPECT_EQ(row_count, 9);

    EXPECT_EQ(store.delete_repositories_by_uuid(repo->collection_uuid()), 0);
    row_count = 0;
    EXPECT_EQ(store.list_repositories(
                  [&listed_repo, &row_count]() -> DatabaseRepositoryObject * {
                    row_count++;
                    return listed_repo.get();
                  }),
              0);
    // Test List
    EXPECT_EQ(row_count, 0);
  }

  {  // Test Create and List and Drop by uid and uuid
    EXPECT_EQ(store.create_repository(*repo), 0);

    auto listed_repo = std::make_shared<DatabaseRepositoryImpl>();
    EXPECT_EQ(store.list_repositories(
                  [&listed_repo, &row_count]() -> DatabaseRepositoryObject * {
                    row_count++;
                    return listed_repo.get();
                  }),
              0);
    // Test List
    EXPECT_EQ(row_count, 1);

    // Invalid arguments for delete_repositories_by_*
    EXPECT_EQ(store.delete_repositories_by_uuid("uuid"), 0);
    EXPECT_EQ(store.delete_repositories_by_uid("uid"), 0);

    row_count = 0;
    EXPECT_EQ(store.list_repositories(
                  [&listed_repo, &row_count]() -> DatabaseRepositoryObject * {
                    row_count++;
                    return listed_repo.get();
                  }),
              0);
    // Test List
    EXPECT_EQ(row_count, 1);
  }
}
