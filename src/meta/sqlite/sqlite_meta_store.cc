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

#include "sqlite_meta_store.h"
#include <ailego/hash/fnv1.h>
#include <ailego/internal/platform.h>
#include "common/error_code.h"
#include "common/logger.h"
#include "meta/meta_store_factory.h"

namespace proxima {
namespace be {
namespace meta {
namespace sqlite {

#define SQLITE_SQL_CODE(__VALUE__) SQLITE_SQL_HASH_##__VALUE__
#define SQLITE_SQL_STR(__VALUE__) SQLITE_SQL##__VALUE__

#define DEFINE_SQLITE_SQL(__VALUE__, __SQL__)            \
  constexpr const char *SQLITE_SQL##__VALUE__ = __SQL__; \
  constexpr uint32_t SQLITE_SQL_HASH_##__VALUE__ = ailego::Fnv1::Hash32(__SQL__)

// Global Sql Statements
// Create Collection SQL
DEFINE_SQLITE_SQL(kCreateCollection,
                  "INSERT INTO "
                  "collections(name, uid, uuid, forward_columns, "
                  "max_docs_per_segment, revision, status, "
                  "current, io_mode) "
                  "VALUES(?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8, ?9);");

// Update Collection SQL
DEFINE_SQLITE_SQL(
    kUpdateCollection,
    "UPDATE collections set name=?1, uid=?2, "
    "forward_columns=?3, max_docs_per_segment=?4, revision=?5, status=?6, "
    "current=?7, io_mode=?8 WHERE uuid=?9;");

// Delete Collection SQL
DEFINE_SQLITE_SQL(kDeleteCollection, "DELETE FROM collections WHERE name=?1;");

// Delete Collection by uuid SQL
DEFINE_SQLITE_SQL(kDeleteCollectionByUUID,
                  "DELETE FROM collections WHERE uuid=?1;");

// List All Collection SQL
DEFINE_SQLITE_SQL(kListAllCollections, "SELECT * from collections;");

// Create Column SQL
DEFINE_SQLITE_SQL(kCreateColumn,
                  "INSERT INTO "
                  "columns(collection_uid, collection_uuid, name, uid, "
                  "dimension, index_type, "
                  "data_type, parameters) "
                  "VALUES(?1, ?2, ?3, ?4, ?5, ?6, ?7, ?8);");

// Delete Column SQL
DEFINE_SQLITE_SQL(kDeleteColumn,
                  "DELETE FROM columns WHERE collection_uid=?1;");

// Delete Column by uuid SQL
DEFINE_SQLITE_SQL(kDeleteColumnByUUID,
                  "DELETE FROM columns WHERE collection_uuid=?1;");

// List Column SQL
DEFINE_SQLITE_SQL(kListColumn, "SELECT * from columns;");

// Create Repository SQL
DEFINE_SQLITE_SQL(kCreateRepository,
                  "INSERT INTO "
                  "database_repositories (name, collection_uid, "
                  "collection_uuid, table_name, connection, user, password) "
                  "VALUES(?1, ?2, ?3, ?4, ?5, ?6, ?7);");

// Delete Repositories SQL
DEFINE_SQLITE_SQL(kDeleteRepositoriesByUID,
                  "DELETE FROM database_repositories WHERE collection_uid=?1;");

// Delete Repositories SQL
DEFINE_SQLITE_SQL(
    kDeleteRepositoriesByUUID,
    "DELETE FROM database_repositories WHERE collection_uuid=?1;");

// List All Repositories SQL
DEFINE_SQLITE_SQL(kListAllRepositories, "SELECT * from database_repositories;");


//! Constructor
SQLiteMetaStore::SQLiteMetaStore() = default;

//! Destructor
SQLiteMetaStore::~SQLiteMetaStore() {
  do_cleanup();
}

//! initialize metastore
int SQLiteMetaStore::initialize(const ailego::Uri *uri) {
  ailego_assert_with(uri != nullptr, "Invalid uri param passed");
  if (initialized_) {
    return 0;
  }

  if (!uri || !uri->is_valid()) {
    return PROXIMA_BE_ERROR_CODE(RuntimeError);
  }

  // check sqlite3 macro MULTITHREAD or SERIALIZED
  if (sqlite3_threadsafe() == 0) {
    LOG_ERROR(
        "Sqlite should be compiled with macro SQLITE_THREADSAFE = 1 or 2");
    return PROXIMA_BE_ERROR_CODE(RuntimeError);
  }

  sqlite3 *handle = nullptr;
  database_ = uri->path();
  int code =
      sqlite3_open_v2(database_.c_str(), &handle,
                      SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);
  if (code != SQLITE_OK) {
    LOG_ERROR("Failed to open sqlite db. msg[%s]", sqlite3_errstr(code));
    return PROXIMA_BE_ERROR_CODE(RuntimeError);
  }

  //! Synchronize Schema
  code = sync(handle);
  if (code == 0) {
    // init statements
    code = init_statements(database_);
  }

  if (code == 0) {
    initialized_ = true;
  }

  sqlite3_close(handle);
  return code;
}

int SQLiteMetaStore::cleanup() {
  return do_cleanup();
}

#define SQLITE_METASTORE_INITIALIZE_CHECK() \
  if (!initialized_) return PROXIMA_BE_ERROR_CODE(RuntimeError)

//! Create the collection
int SQLiteMetaStore::create_collection(const CollectionObject &collection) {
  SQLITE_METASTORE_INITIALIZE_CHECK();

  StatementPtr &stmt = statement(SQLITE_SQL_CODE(kCreateCollection));

  int code = stmt->exec(
      [&collection](sqlite3_stmt *s) -> int {
        sqlite3_bind_text(s, 1, collection.name().c_str(),
                          collection.name().length(), nullptr);
        sqlite3_bind_text(s, 2, collection.uid().c_str(),
                          collection.uid().length(), nullptr);
        sqlite3_bind_text(s, 3, collection.uuid().c_str(),
                          collection.uuid().length(), nullptr);
        sqlite3_bind_text(s, 4, collection.forward_columns().c_str(),
                          collection.forward_columns().length(), nullptr);
        sqlite3_bind_int64(
            s, 5,
            static_cast<sqlite3_int64>(collection.max_docs_per_segment()));
        sqlite3_bind_int(s, 6, collection.revision());
        sqlite3_bind_int(s, 7, collection.status());
        sqlite3_bind_int(s, 8, collection.current());
        sqlite3_bind_int(s, 9, collection.io_mode());
        return 0;
      },
      nullptr);

  if (code != 0) {
    LOG_ERROR("Failed to create collection. code[%d]", code);
  }
  return code;
}

//! Update collection declaration
int SQLiteMetaStore::update_collection(const CollectionObject &collection) {
  SQLITE_METASTORE_INITIALIZE_CHECK();

  StatementPtr &stmt = statement(SQLITE_SQL_CODE(kUpdateCollection));
  int code = stmt->exec(
      [&collection](sqlite3_stmt *s) -> int {
        sqlite3_bind_text(s, 1, collection.name().c_str(),
                          collection.name().length(), nullptr);
        sqlite3_bind_text(s, 2, collection.uid().c_str(),
                          collection.uid().length(), nullptr);
        sqlite3_bind_text(s, 3, collection.forward_columns().c_str(),
                          collection.forward_columns().length(), nullptr);
        sqlite3_bind_int64(
            s, 4,
            static_cast<sqlite3_int64>(collection.max_docs_per_segment()));
        sqlite3_bind_int(s, 5, collection.revision());
        sqlite3_bind_int(s, 6, collection.status());
        sqlite3_bind_int(s, 7, collection.current());
        sqlite3_bind_int(s, 8, collection.io_mode());
        sqlite3_bind_text(s, 9, collection.uuid().c_str(),
                          collection.uuid().length(), nullptr);
        return 0;
      },
      nullptr);
  if (code != 0) {
    LOG_ERROR("Failed to update collection. code[%d]", code);
  }

  return code;
}

//! Delete collection
int SQLiteMetaStore::delete_collection(const std::string &name) {
  SQLITE_METASTORE_INITIALIZE_CHECK();

  StatementPtr &stmt = statement(SQLITE_SQL_CODE(kDeleteCollection));

  int code = stmt->exec(
      [&name](sqlite3_stmt *s) -> int {
        sqlite3_bind_text(s, 1, name.c_str(), name.length(), nullptr);
        return 0;
      },
      nullptr);
  if (code != 0) {
    LOG_ERROR("Failed to delete collection. code[%d]", code);
  }

  return code;
}

int SQLiteMetaStore::delete_collection_by_uuid(const std::string &uuid) {
  SQLITE_METASTORE_INITIALIZE_CHECK();

  StatementPtr &stmt = statement(SQLITE_SQL_CODE(kDeleteCollectionByUUID));

  int code = stmt->exec(
      [&uuid](sqlite3_stmt *s) -> int {
        sqlite3_bind_text(s, 1, uuid.c_str(), uuid.length(), nullptr);
        return 0;
      },
      nullptr);
  if (code != 0) {
    LOG_ERROR("Failed to delete collection. code[%d]", code);
  }

  return code;
}

namespace {
static std::function<int(sqlite3_stmt *)> BuildFetcher(
    CollectionObject *collection_ptr) {
  return [=](sqlite3_stmt *s) -> int {
    collection_ptr->set_id(sqlite3_column_int64(s, 0));
    collection_ptr->mutable_name()->assign(
        reinterpret_cast<const char *>(sqlite3_column_text(s, 1)));
    collection_ptr->mutable_uid()->assign(
        reinterpret_cast<const char *>(sqlite3_column_text(s, 2)));
    collection_ptr->mutable_uuid()->assign(
        reinterpret_cast<const char *>(sqlite3_column_text(s, 3)));
    collection_ptr->mutable_forward_columns()->assign(
        reinterpret_cast<const char *>(sqlite3_column_text(s, 4)));
    collection_ptr->set_max_docs_per_segment(sqlite3_column_int64(s, 5));
    collection_ptr->set_revision(sqlite3_column_int(s, 6));
    collection_ptr->set_status(sqlite3_column_int(s, 7));
    collection_ptr->set_current(sqlite3_column_int(s, 8));
    collection_ptr->set_io_mode(sqlite3_column_int(s, 9));
    return 0;
  };
}

static std::function<int(sqlite3_stmt *)> BuildFetcher(ColumnObject *column) {
  return [=](sqlite3_stmt *s) -> int {
    column->set_id(sqlite3_column_int64(s, 0));
    column->mutable_collection_uid()->assign(
        reinterpret_cast<const char *>(sqlite3_column_text(s, 1)));
    column->mutable_collection_uuid()->assign(
        reinterpret_cast<const char *>(sqlite3_column_text(s, 2)));
    column->mutable_name()->assign(
        reinterpret_cast<const char *>(sqlite3_column_text(s, 3)));
    column->mutable_uid()->assign(
        reinterpret_cast<const char *>(sqlite3_column_text(s, 4)));
    column->set_dimension(sqlite3_column_int(s, 5));
    column->set_index_type(sqlite3_column_int(s, 6));
    column->set_data_type(sqlite3_column_int(s, 7));
    column->mutable_parameters()->assign(
        reinterpret_cast<const char *>(sqlite3_column_text(s, 8)));
    return 0;
  };
}

static std::function<int(sqlite3_stmt *)> BuildFetcher(
    DatabaseRepositoryObject *repository) {
  return [=](sqlite3_stmt *s) -> int {
    repository->set_id(sqlite3_column_int64(s, 0));
    repository->mutable_name()->assign(
        reinterpret_cast<const char *>(sqlite3_column_text(s, 1)));
    repository->mutable_collection_uid()->assign(
        reinterpret_cast<const char *>(sqlite3_column_text(s, 2)));
    repository->mutable_collection_uuid()->assign(
        reinterpret_cast<const char *>(sqlite3_column_text(s, 3)));
    repository->mutable_table()->assign(
        reinterpret_cast<const char *>(sqlite3_column_text(s, 4)));
    repository->mutable_connection()->assign(
        reinterpret_cast<const char *>(sqlite3_column_text(s, 5)));
    repository->mutable_user()->assign(
        reinterpret_cast<const char *>(sqlite3_column_text(s, 6)));
    repository->mutable_password()->assign(
        reinterpret_cast<const char *>(sqlite3_column_text(s, 7)));
    return 0;
  };
}

}  // namespace

//! Retrieve all collections
int SQLiteMetaStore::list_collections(CollectionAllocator allocator) const {
  SQLITE_METASTORE_INITIALIZE_CHECK();

  StatementPtr &stmt = statement(SQLITE_SQL_CODE(kListAllCollections));

  int code = stmt->exec(nullptr, [&allocator](sqlite3_stmt *s) -> int {
    int fetch_code = BuildFetcher(allocator())(s);
    if (fetch_code != 0) {
      LOG_ERROR("Failed to fetch collection from sqlite statement");
    }
    return fetch_code;
  });

  return code;
}

//! Create column
int SQLiteMetaStore::create_column(const ColumnObject &column) {
  SQLITE_METASTORE_INITIALIZE_CHECK();

  StatementPtr &stmt = statement(SQLITE_SQL_CODE(kCreateColumn));

  int code = stmt->exec(
      [&column](sqlite3_stmt *s) -> int {
        sqlite3_bind_text(s, 1, column.collection_uid().c_str(),
                          column.collection_uid().length(), nullptr);
        sqlite3_bind_text(s, 2, column.collection_uuid().c_str(),
                          column.collection_uuid().length(), nullptr);
        sqlite3_bind_text(s, 3, column.name().c_str(), column.name().length(),
                          nullptr);
        sqlite3_bind_text(s, 4, column.uid().c_str(), column.uid().length(),
                          nullptr);
        sqlite3_bind_int(s, 5, column.dimension());
        sqlite3_bind_int(s, 6, column.index_type());
        sqlite3_bind_int(s, 7, column.data_type());
        sqlite3_bind_text(s, 8, column.parameters().c_str(),
                          column.parameters().length(), nullptr);
        return 0;
      },
      nullptr);

  if (code != 0) {
    LOG_ERROR("Failed to create column. code[%d]", code);
  }
  return code;
}

//! Delete column
int SQLiteMetaStore::delete_columns_by_uid(const std::string &uid) {
  SQLITE_METASTORE_INITIALIZE_CHECK();

  StatementPtr &stmt = statement(SQLITE_SQL_CODE(kDeleteColumn));

  int code = stmt->exec(
      [&uid](sqlite3_stmt *s) -> int {
        sqlite3_bind_text(s, 1, uid.c_str(), uid.length(), nullptr);
        return 0;
      },
      nullptr);
  if (code != 0) {
    LOG_ERROR("Failed to delete column. code[%d]", code);
  }

  return code;
}

//! Delete column
int SQLiteMetaStore::delete_columns_by_uuid(const std::string &uuid) {
  SQLITE_METASTORE_INITIALIZE_CHECK();

  StatementPtr &stmt = statement(SQLITE_SQL_CODE(kDeleteColumnByUUID));

  int code = stmt->exec(
      [&uuid](sqlite3_stmt *s) -> int {
        sqlite3_bind_text(s, 1, uuid.c_str(), uuid.length(), nullptr);
        return 0;
      },
      nullptr);
  if (code != 0) {
    LOG_ERROR("Failed to delete column. code[%d]", code);
  }

  return code;
}

//! Retrieve columns
int SQLiteMetaStore::list_columns(ColumnAllocator allocator) const {
  SQLITE_METASTORE_INITIALIZE_CHECK();

  StatementPtr &stmt = statement(SQLITE_SQL_CODE(kListColumn));

  int code = stmt->exec(nullptr, [&allocator](sqlite3_stmt *s) -> int {
    int fetch_code = BuildFetcher(allocator())(s);
    if (fetch_code != 0) {
      LOG_ERROR("Failed to fetch column from sqlite statement");
    }
    return fetch_code;
  });

  return code;
}

//! Create repository
int SQLiteMetaStore::create_repository(
    const DatabaseRepositoryObject &repository) {
  SQLITE_METASTORE_INITIALIZE_CHECK();

  StatementPtr &stmt = statement(SQLITE_SQL_CODE(kCreateRepository));

  int code = stmt->exec(
      [&repository](sqlite3_stmt *s) -> int {
        sqlite3_bind_text(s, 1, repository.name().c_str(),
                          repository.name().length(), nullptr);
        sqlite3_bind_text(s, 2, repository.collection_uid().c_str(),
                          repository.collection_uid().length(), nullptr);
        sqlite3_bind_text(s, 3, repository.collection_uuid().c_str(),
                          repository.collection_uuid().length(), nullptr);
        sqlite3_bind_text(s, 4, repository.table().c_str(),
                          repository.table().length(), nullptr);
        sqlite3_bind_text(s, 5, repository.connection().c_str(),
                          repository.connection().length(), nullptr);
        sqlite3_bind_text(s, 6, repository.user().c_str(),
                          repository.user().length(), nullptr);
        sqlite3_bind_text(s, 7, repository.password().c_str(),
                          repository.password().length(), nullptr);
        return 0;
      },
      nullptr);

  if (code != 0) {
    LOG_ERROR("Failed to create repository. code[%d]", code);
  }
  return code;
}

//! Delete repositories
int SQLiteMetaStore::delete_repositories_by_uid(const std::string &uid) {
  SQLITE_METASTORE_INITIALIZE_CHECK();

  StatementPtr &stmt = statement(SQLITE_SQL_CODE(kDeleteRepositoriesByUID));

  int code = stmt->exec(
      [&uid](sqlite3_stmt *s) -> int {
        sqlite3_bind_text(s, 1, uid.c_str(), uid.length(), nullptr);
        return 0;
      },
      nullptr);
  if (code != 0) {
    LOG_ERROR("Failed to delete repository. code[%d]", code);
  }

  return code;
}

//! Delete repositories
int SQLiteMetaStore::delete_repositories_by_uuid(const std::string &uuid) {
  SQLITE_METASTORE_INITIALIZE_CHECK();

  StatementPtr &stmt = statement(SQLITE_SQL_CODE(kDeleteRepositoriesByUUID));

  int code = stmt->exec(
      [&uuid](sqlite3_stmt *s) -> int {
        sqlite3_bind_text(s, 1, uuid.c_str(), uuid.length(), nullptr);
        return 0;
      },
      nullptr);
  if (code != 0) {
    LOG_ERROR("Failed to delete repository. code[%d]", code);
  }

  return code;
}

//! Retrieve all repositories
int SQLiteMetaStore::list_repositories(
    DatabaseRepositoryAllocator allocator) const {
  SQLITE_METASTORE_INITIALIZE_CHECK();

  StatementPtr &stmt = statement(SQLITE_SQL_CODE(kListAllRepositories));

  int code = stmt->exec(nullptr, [&allocator](sqlite3_stmt *s) -> int {
    int fetch_code = BuildFetcher(allocator())(s);
    if (fetch_code != 0) {
      LOG_ERROR("Failed to fetch repository from sqlite statement");
    }
    return fetch_code;
  });

  return code;
}

//! Flush all changes to storage
int SQLiteMetaStore::flush() const {
  return 0;
}

int SQLiteMetaStore::do_cleanup() {
  for (auto &stmt : statements_) {
    stmt.second->cleanup();
  }

  statements_.clear();
  initialized_ = false;
  return 0;
}

int SQLiteMetaStore::sync(sqlite3 *handle) {
  static const char *kMetaTable =
      "CREATE TABLE IF NOT EXISTS columns ( \n"
      "    id INTEGER PRIMARY KEY AUTOINCREMENT, \n"
      "    collection_uid TEXT NOT NULL, \n"
      "    collection_uuid TEXT NOT NULL, \n"
      "    name TEXT NOT NULL, \n"
      "    uid TEXT NOT NULL, \n"
      "    dimension INTEGER, \n"
      "    index_type INTEGER, \n"
      "    data_type INTEGER, \n"
      "    parameters TEXT DEFAULT '' \n"
      ");"
      "CREATE TABLE IF NOT EXISTS collections ("
      "    id INTEGER PRIMARY KEY AUTOINCREMENT, \n"
      "    name TEXT NOT NULL, \n"
      "    uid TEXT NOT NULL, \n"
      "    uuid TEXT NOT NULL UNIQUE, \n"
      "    forward_columns TEXT NOT NULL, \n"
      "    max_docs_per_segment INTEGER, \n"
      "    revision INTEGER, \n"
      "    status INTEGER, \n"
      "    current INTEGER, \n"
      "    io_mode INTEGER\n"
      ");"
      "CREATE TABLE IF NOT EXISTS database_repositories ("
      "    id INTEGER PRIMARY KEY AUTOINCREMENT, \n"
      "    name TEXT NOT NULL, \n"
      "    collection_uid TEXT NOT NULL, \n"
      "    collection_uuid TEXT NOT NULL, \n"
      "    table_name TEXT NOT NULL, \n"
      "    connection TEXT NOT NULL, \n"
      "    user TEXT NOT NULL, \n"
      "    password TEXT NOT NULL \n"
      ");";

  int code = sqlite3_exec(handle, kMetaTable, nullptr, nullptr, nullptr);
  if (code != SQLITE_OK) {
    LOG_ERROR("Failed to create table. msg[%s]", sqlite3_errstr(code));
    return PROXIMA_BE_ERROR_CODE(RuntimeError);
  }
  return 0;
}

#define PREPARE_AND_CACHE_SQL(K, DB)                                    \
  {                                                                     \
    int code = put(SQLITE_SQL_CODE(K),                                  \
                   std::make_shared<Statement>(DB, SQLITE_SQL_STR(K))); \
    if (code != 0) {                                                    \
      code = PROXIMA_BE_ERROR_CODE(RuntimeError);                       \
      LOG_ERROR("Failed to prepare sql.");                              \
      return code;                                                      \
    }                                                                   \
  }

int SQLiteMetaStore::init_statements(const std::string &database) {
  PREPARE_AND_CACHE_SQL(kCreateCollection, database)
  PREPARE_AND_CACHE_SQL(kUpdateCollection, database)
  PREPARE_AND_CACHE_SQL(kDeleteCollection, database)
  PREPARE_AND_CACHE_SQL(kDeleteCollectionByUUID, database)
  PREPARE_AND_CACHE_SQL(kListAllCollections, database)

  PREPARE_AND_CACHE_SQL(kCreateColumn, database)
  PREPARE_AND_CACHE_SQL(kDeleteColumn, database)
  PREPARE_AND_CACHE_SQL(kDeleteColumnByUUID, database)
  PREPARE_AND_CACHE_SQL(kListColumn, database)

  PREPARE_AND_CACHE_SQL(kCreateRepository, database)
  PREPARE_AND_CACHE_SQL(kDeleteRepositoriesByUID, database)
  PREPARE_AND_CACHE_SQL(kDeleteRepositoriesByUUID, database)
  PREPARE_AND_CACHE_SQL(kListAllRepositories, database)
  return 0;
}

#undef PREPARE_AND_CACHE_SQL

StatementPtr &SQLiteMetaStore::statement(uint32_t index) const {
  return statements_[index];
}

int SQLiteMetaStore::put(uint32_t hash, const StatementPtr &stmt) {
  if (stmt->initialize() == 0) {
    statements_[hash] = stmt;
    return 0;
  }
  LOG_ERROR("Failed to initialize Statement");
  return PROXIMA_BE_ERROR_CODE(RuntimeError);
}

#undef SQLITE_METASTORE_INITIALIZE_CHECK
#undef DEFINE_SQLITE_SQL
#undef SQLITE_SQL_STR
#undef SQLITE_SQL_CODE

META_FACTORY_REGISTER_INSTANCE_ALIAS(sqlite, SQLiteMetaStore);

}  // namespace sqlite
}  // namespace meta
}  // namespace be
}  // namespace proxima
