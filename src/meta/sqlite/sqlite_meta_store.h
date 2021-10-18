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
 *   \date     Oct 2020
 *   \brief
 */

#pragma once

#include <atomic>
#include <unordered_map>
#include "meta/meta_store.h"
#include "sqlite_statement.h"

namespace proxima {
namespace be {
namespace meta {
namespace sqlite {

/*!
 * SQLiteMateStore
 */
class SQLiteMetaStore : public MetaStore {
 public:
  //! Constructor
  SQLiteMetaStore();

  //! Destructor
  ~SQLiteMetaStore() override;

 public:
  //! initialize metastore
  int initialize(const ailego::Uri *uri) override;

  //! cleanup
  int cleanup() override;

  //! Create the collection
  int create_collection(const CollectionObject &collection) override;

  //! Update collection declaration
  int update_collection(const CollectionObject &collection) override;

  //! Delete collection
  int delete_collection(const std::string &name) override;

  //! Delete collection by uuid
  int delete_collection_by_uuid(const std::string &uuid) override;

  //! Retrieve all collections
  int list_collections(CollectionAllocator allocator) const override;

  //! Create column
  int create_column(const ColumnObject &column) override;

  //! Delete columns
  int delete_columns_by_uid(const std::string &uid) override;

  //! Delete columns
  int delete_columns_by_uuid(const std::string &uuid) override;

  //! Retrieve all collections
  int list_columns(ColumnAllocator allocator) const override;

  //! Create repository
  int create_repository(const DatabaseRepositoryObject &repository) override;

  //! Delete repositories
  int delete_repositories_by_uid(const std::string &uid) override;

  //! Delete repositories
  int delete_repositories_by_uuid(const std::string &uuid) override;

  //! Retrieve all repositories
  int list_repositories(DatabaseRepositoryAllocator allocator) const override;

  //! Flush all changes to storage
  int flush() const override;

 private:
  //! Cleanup
  int do_cleanup();

  //! Sync tables to database
  int sync(sqlite3 *handle);

  //! Initialize all the statements
  int init_statements(const std::string &database);

  //! Get statement
  StatementPtr &statement(uint32_t index) const;

  //! Put statement into map
  int put(uint32_t hash, const StatementPtr &stmt);

 private:
  //! Initialized flag
  std::atomic_bool initialized_{false};

  //! DB storage path
  std::string database_{};

  //! Statements
  mutable std::unordered_map<uint32_t, StatementPtr> statements_{};
};


}  // namespace sqlite
}  // namespace meta
}  // namespace be
}  // namespace proxima
