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

#pragma once

#include "meta.h"
#include "meta_types.h"

namespace proxima {
namespace be {
namespace meta {

//! Predefine class
class ColumnImpl;
class CollectionImpl;
class DatabaseRepositoryImpl;

//! Alias Pointers
using ColumnImplPtr = std::shared_ptr<ColumnImpl>;
using ColumnImplPtrList = std::list<ColumnImplPtr>;
using CollectionImplPtr = std::shared_ptr<CollectionImpl>;
using CollectionImplPtrList = std::list<CollectionImplPtr>;
using DatabaseRepositoryImplPtr = std::shared_ptr<DatabaseRepositoryImpl>;


/*!
 * Column Implementation
 */
class ColumnImpl : public ColumnObject {
 public:
  //! Constructors
  ColumnImpl();
  explicit ColumnImpl(const ColumnMeta &column_meta);
  ColumnImpl(std::string uid, std::string uuid, const ColumnMeta &meta);

  //! Destructor
  ~ColumnImpl() override = default;

 public:
  //! Retrieve id
  uint64_t id() const override {
    return id_;
  }

  //! Set id
  void set_id(uint64_t new_id) override {
    id_ = new_id;
  }

  //! Retrieve collection uid
  const std::string &collection_uid() const override {
    return collection_uid_;
  }

  //! Retrieve mutable collection uid
  std::string *mutable_collection_uid() override {
    return &collection_uid_;
  }

  //! Set collection uid
  void set_collection_uid(const std::string &new_collection_uid) override {
    collection_uid_ = new_collection_uid;
  }

  //! Retrieve collection uuid
  const std::string &collection_uuid() const override {
    return collection_uuid_;
  }

  //! Retrieve mutable collection uuid
  std::string *mutable_collection_uuid() override {
    return &collection_uuid_;
  }

  //! Set collection uuid
  void set_collection_uuid(const std::string &new_collection_uuid) override {
    collection_uuid_ = new_collection_uuid;
  }

  //! Retrieve column name
  const std::string &name() const override {
    return meta_->name();
  }

  //! Retrieve mutable column name
  std::string *mutable_name() override {
    return meta_->mutable_name();
  }

  //! Set column name
  void set_name(const std::string &new_name) override {
    meta_->set_name(new_name);
  }

  //! Retrieve column uid
  const std::string &uid() const override {
    return meta_->uid();
  }

  //! Retrieve mutable column uid
  std::string *mutable_uid() override {
    return meta_->mutable_uid();
  }

  //! Set column uid
  void set_uid(const std::string &new_uid) override {
    meta_->set_uid(new_uid);
  }

  //! Retrieve dimension of column
  uint32_t dimension() const override {
    return meta_->dimension();
  }

  //! Set dimension of column
  void set_dimension(uint32_t new_dimension) override {
    return meta_->set_dimension(new_dimension);
  }

  //! Retrieve column index type
  uint32_t index_type() const override {
    return static_cast<uint32_t>(meta_->index_type());
  }

  //! Set column index type
  void set_index_type(uint32_t type) override {
    meta_->set_index_type(static_cast<IndexTypes>(type));
  }

  //! Retrieve column data type
  uint32_t data_type() const override {
    return static_cast<uint32_t>(meta_->data_type());
  }

  //! Set column index type
  void set_data_type(uint32_t type) override {
    meta_->set_data_type(static_cast<DataTypes>(type));
  }

  //! Retrieve column parameters
  const std::string &parameters() const override {
    return parameters_;
  }

  //! Retrieve mutable column parameters
  std::string *mutable_parameters() override {
    return &parameters_;
  }

  //! Set column parameters
  void set_parameters(const std::string &params) override {
    parameters_ = params;
  }

  //! Retrieve mutable meta
  const ColumnMetaPtr &meta() const {
    return meta_;
  }

  ////! Retrieve mutable meta
  // ColumnMetaPtr *mutable_meta() {
  //  return &meta_;
  //}

  //! Transform
  void transform();

 private:
  //! Meta
  ColumnMetaPtr meta_{nullptr};

  //! Primary key in database
  uint64_t id_{0};

  //! Uid of column
  std::string uid_{};

  //! Collection uid, indicate which collection group belong to
  std::string collection_uid_{};

  //! Collection uuid, indicate which specific collection belong to
  std::string collection_uuid_{};

  //! Index params, Json string, which parsed to meta_.parameters field
  std::string parameters_{};
};


/*!
 * Collection implementation
 */
class CollectionImpl : public CollectionObject {
 public:
  //! Constructors
  CollectionImpl();
  CollectionImpl(const char *coll_name);
  CollectionImpl(const CollectionMeta &coll_meta);
  CollectionImpl(CollectionMetaPtr meta_ptr);

  //! Destructor
  ~CollectionImpl() override = default;

 public:
  //! Retrieve id
  uint64_t id() const override {
    return id_;
  }

  //! Set collection id
  void set_id(uint64_t new_id) override {
    id_ = new_id;
  }

  //! Retrieve collection name
  const std::string &name() const override {
    return meta_->name();
  }

  //! Retrieve mutable collection name
  std::string *mutable_name() override {
    return meta_->mutable_name();
  }

  //! Set collection name
  void set_name(const std::string &new_name) override {
    meta_->set_name(new_name);
  }

  //! Retrieve collection uid, indicate a group of collections
  const std::string &uid() const override {
    return meta_->uid();
  }

  //! Retrieve mutable collection uid, indicate a group of collections
  std::string *mutable_uid() override {
    return meta_->mutable_uid();
  }

  //! Set collection uid
  void set_uid(const std::string &new_uid) override {
    meta_->set_uid(new_uid);
  }

  //! Retrieve collection uuid, unique field
  const std::string &uuid() const override {
    return uuid_;
  }

  //! Retrieve mutable collection uuid, unique field
  std::string *mutable_uuid() override {
    return &uuid_;
  }

  //! Set collection uuid
  void set_uuid(const std::string &new_uuid) override {
    uuid_ = new_uuid;
  }

  //! Retrieve forward columns, separated by character ','
  const std::string &forward_columns() const override {
    return forward_columns_;
  }

  //! Retrieve mutable forward columns, separated by character ','
  std::string *mutable_forward_columns() override {
    return &forward_columns_;
  }

  //! Set forward columns
  void set_forward_columns(const std::string &new_forward_columns) override {
    forward_columns_ = new_forward_columns;
  }

  //! Retrieve split index size
  uint64_t max_docs_per_segment() const override {
    return meta_->max_docs_per_segment();
  }

  //! Set split index size
  void set_max_docs_per_segment(uint64_t count) override {
    meta_->set_max_docs_per_segment(count);
  }

  //! Retrieve collection revision
  uint32_t revision() const override {
    return meta_->revision();
  }

  //! Set collection revision
  void set_revision(uint32_t next_revision) override {
    meta_->set_revision(next_revision);
  }

  //! Retrieve collection status
  uint32_t status() const override {
    return static_cast<uint32_t>(meta_->status());
  }

  //! Check is serving
  bool serving() const {
    return meta_->serving();
  }

  //! Set collection status
  void set_status(uint32_t new_status) override {
    meta_->set_status(static_cast<CollectionStatus>(new_status));
  }

  //! Retrieve collection current flag
  uint32_t current() const override {
    return meta_->is_current() ? 1 : 0;
  }

  //! Set collection current flag
  void set_current(uint32_t flag) override {
    meta_->set_current(flag != 0);
  }

  //! Retrieve io_mode of collection
  uint32_t io_mode() const override {
    uint32_t mode = 0;
    if (meta_->readable()) {
      mode |= 0x1;
    }
    if (meta_->writable()) {
      mode |= 0x2;
    }
    return mode;
  }

  //! Set io_mode of collection
  void set_io_mode(uint32_t mode) override {
    meta_->set_readable(mode & 0x1);
    meta_->set_writable(mode & 0x2);
  }

  //! Retrieve meta
  CollectionMetaPtr meta() const {
    return meta_;
  }

  //! Retrieve columns
  const ColumnImplPtrList &columns() const {
    return columns_;
  }

  //! Retrieve column with name
  const ColumnImplPtr &get_column(const std::string &column_name) const {
    return get_column([&column_name](const ColumnImplPtr &c) -> bool {
      return column_name == c->name();
    });
  }

  ////! Check valid column
  // bool has_column(const std::string &column_name) const {
  //  return get_column(column_name).operator bool();
  //}

  //! Append column, 0 for success, otherwise failed
  //! @param do not check uuid and uid if force equals true,
  int append(ColumnImplPtr &, bool force = true);

  //! Retrieve repository
  DatabaseRepositoryImplPtr repository() const {
    return repository_;
  }

  //! Attach to repository
  int set_repository(DatabaseRepositoryImplPtr &);

  //! Transform
  void transform();

 private:
  //! Init member
  void init_from_meta();

  //! Retrieve column with filter
  const ColumnImplPtr &get_column(
      std::function<bool(const ColumnImplPtr &)> filter) const {
    static ColumnImplPtr DefaultColumn;
    auto iter = std::find_if_not(
        columns_.begin(), columns_.end(),
        [&filter](const ColumnImplPtr &c) -> bool { return !filter(c); });
    return iter != columns_.end() ? *iter : DefaultColumn;
  }

 private:
  //! Meta
  CollectionMetaPtr meta_{nullptr};

  //! ID
  uint64_t id_{0};

  //! Uniq id
  std::string uuid_{};

  //! Forward columns, separated by character ','
  std::string forward_columns_{};

  //! Columns
  ColumnImplPtrList columns_{};

  DatabaseRepositoryImplPtr repository_{};
};


/*! Repository implementation
 */
class DatabaseRepositoryImpl : public DatabaseRepositoryObject {
 public:
  //! Constructor
  DatabaseRepositoryImpl();
  explicit DatabaseRepositoryImpl(const DatabaseRepositoryMeta &);
  DatabaseRepositoryImpl(const std::string &, const std::string &,
                         const DatabaseRepositoryMeta &);

  //! Destructor
  ~DatabaseRepositoryImpl() override = default;

 public:
  //! Retrieve id
  uint64_t id() const override {
    return id_;
  }

  //! Set repository id
  void set_id(uint64_t repo_id) override {
    id_ = repo_id;
  }

  //! Retrieve repository name
  const std::string &name() const override {
    return repository_->name();
  }

  //! Retrieve mutable repository name
  std::string *mutable_name() override {
    return repository_->mutable_name();
  }

  //! Set repository name
  void set_name(const std::string &repo_name) override {
    repository_->set_name(repo_name);
  }

  //! Retrieve collection uid
  const std::string &collection_uid() const override {
    return collection_uid_;
  }

  //! Retrieve mutable collection uid
  std::string *mutable_collection_uid() override {
    return &collection_uid_;
  }

  //! Set collection uid
  void set_collection_uid(const std::string &new_connection_uid) override {
    collection_uid_ = new_connection_uid;
  }

  //! Retrieve collection uuid
  const std::string &collection_uuid() const override {
    return collection_uuid_;
  }

  //! Retrieve mutable collection uuid
  std::string *mutable_collection_uuid() override {
    return &collection_uuid_;
  }

  //! Set collection uuid
  void set_collection_uuid(const std::string &new_collection_uuid) override {
    collection_uuid_ = new_collection_uuid;
  }

  //! Retrieve connection uri
  const std::string &connection() const override {
    return repository_->connection();
  }

  //! Retrieve mutable connection uri
  std::string *mutable_connection() override {
    return repository_->mutable_connection();
  }

  //! Set connection uri
  void set_connection(const std::string &uri) override {
    repository_->set_connection(uri);
  }

  //! Retrieve user name of connection
  const std::string &user() const override {
    return repository_->user();
  }

  //! Retrieve mutable user name of connection
  std::string *mutable_user() override {
    return repository_->mutable_user();
  }

  //! Set user name of connection
  void set_user(const std::string &user_name) override {
    repository_->set_user(user_name);
  }

  //! Retrieve password of user
  const std::string &password() const override {
    return repository_->password();
  }

  //! Retrieve mutable password of user
  std::string *mutable_password() override {
    return repository_->mutable_password();
  }

  //! Set password of user
  void set_password(const std::string &new_password) override {
    repository_->set_password(new_password);
  }

  //! Retrieve repository table name
  const std::string &table() const override {
    return repository_->table_name();
  }

  //! Retrieve mutable repository table name
  std::string *mutable_table() override {
    return repository_->mutable_table_name();
  }

  //! Set repository table name
  void set_table(const std::string &table_name) override {
    repository_->set_table_name(table_name);
  }

  const DatabaseRepositoryMetaPtr meta() const {
    return repository_;
  }

 private:
  //! ID
  uint64_t id_{0u};

  //! Collection uid
  std::string collection_uid_{};

  //! Collection uuid
  std::string collection_uuid_{};

  //! Repository handler
  DatabaseRepositoryMetaPtr repository_{nullptr};
};


}  // namespace meta
}  // namespace be
}  // namespace proxima
