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

#include <memory>
#include <vector>
#include <ailego/encoding/uri.h>
#include <aitheta2/index_params.h>
#include "common/error_code.h"
#include "common/logger.h"
#include "common/types.h"
#include "common/uuid_helper.h"

namespace proxima {
namespace be {
namespace meta {

// Predefine classes
class RepositoryBase;
class DatabaseRepositoryMeta;
class ColumnMeta;
class CollectionBase;
class CollectionMeta;

// Alias for meta types
using RepositoryBasePtr = std::shared_ptr<RepositoryBase>;
using DatabaseRepositoryMetaPtr = std::shared_ptr<DatabaseRepositoryMeta>;
using ColumnMetaPtr = std::shared_ptr<ColumnMeta>;
using ColumnMetaPtrList = std::vector<ColumnMetaPtr>;
using CollectionMetaPtr = std::shared_ptr<CollectionMeta>;
using CollectionMetaPtrList = std::vector<CollectionMetaPtr>;

//! Alias
using IndexParams = aitheta2::IndexParams;

#define META_VERIFY_ARGUMENTS(COND, CODE, MSG) \
  if ((COND)) {                                \
    LOG_ERROR(MSG);                            \
    return CODE;                               \
  }


/*!
 * ColumnMeta illustrate Column in Collection
 */
class ColumnMeta {
 public:
  //! Constructors
  ColumnMeta() = default;

  explicit ColumnMeta(std::string &&column_name)
      : name_(std::move(column_name)),
        uid_(gen_uuid()),
        index_type_(IndexTypes::PROXIMA_GRAPH_INDEX),
        data_type_(DataTypes::UNDEFINED),
        dimension_(0) {}

  explicit ColumnMeta(const ColumnMeta &param)
      : name_(param.name_),
        uid_(param.uid_),
        index_type_(param.index_type_),
        data_type_(param.data_type_),
        dimension_(param.dimension_),
        parameters_(param.parameters_) {}

  //! Destructor
  ~ColumnMeta() = default;

 public:
  int validate() const {
    META_VERIFY_ARGUMENTS(
        name_.empty(), PROXIMA_BE_ERROR_CODE(EmptyColumnName),
        "Invalid arguments for create collection, Name of collection "
        "can't be empty")

    META_VERIFY_ARGUMENTS(data_type_ == DataTypes::UNDEFINED,
                          PROXIMA_BE_ERROR_CODE(InvalidDataType),
                          "Invalid data types")
    return 0;
  }

  //! Retrieve name field
  const std::string &name() const {
    return name_;
  }

  //! Set name
  void set_name(const std::string &new_name) {
    name_ = new_name;
  }

  //! Retrieve mutable name field
  std::string *mutable_name() {
    return &name_;
  }

  //! Retrieve column uid
  const std::string &uid() const {
    return uid_;
  }

  //! Retrieve mutable column uid
  std::string *mutable_uid() {
    return &uid_;
  }

  //! Set column uid
  void set_uid(const std::string &new_uid) {
    uid_ = new_uid;
  }

  //! Retrieve index type
  IndexTypes index_type() const {
    return index_type_;
  }

  //! Set index type
  void set_index_type(IndexTypes type) {
    index_type_ = type;
  }

  //! Retrieve data type
  DataTypes data_type() const {
    return data_type_;
  }

  //! Set data type
  void set_data_type(DataTypes type) {
    data_type_ = type;
  }

  //! Retrieve index type
  uint32_t dimension() const {
    return dimension_;
  }

  //! Set index type
  void set_dimension(uint32_t new_dimension) {
    dimension_ = new_dimension;
  }

  //! Retrieve params
  const IndexParams &parameters() const {
    return parameters_;
  }

  //! Get mutable params
  IndexParams *mutable_parameters() {
    return &parameters_;
  }

  int check_updated_field(const ColumnMeta &param) const {
    if (name_ != param.name()) {
      return PROXIMA_BE_ERROR_CODE(UpdateColumnNameField);
    } else if (index_type_ != param.index_type()) {
      return PROXIMA_BE_ERROR_CODE(UpdateIndexTypeField);
    } else if (data_type_ != param.data_type()) {
      return PROXIMA_BE_ERROR_CODE(UpdateDataTypeField);
    } else if (dimension_ != param.dimension()) {
    } else {
      std::string meta_params, params;
      IndexParams::SerializeToBuffer(param.parameters(), &meta_params);
      IndexParams::SerializeToBuffer(parameters_, &params);
      if (params != meta_params) {
        return PROXIMA_BE_ERROR_CODE(UpdateParametersField);
      }
    }
    LOG_INFO("Input column was check_updated_field");
    return 0;
  }

 private:
  //! Column name
  std::string name_{};

  //! Unique ID of column
  std::string uid_{};

  //! The type of column, possible value listed bellow:
  //! 1. PROXIMA_GRAPH_INDEX: vector index, default value
  //! 2. ...
  IndexTypes index_type_{IndexTypes::PROXIMA_GRAPH_INDEX};

  //! Data Types
  DataTypes data_type_{DataTypes::UNDEFINED};

  //! Dimension of data
  uint32_t dimension_{0u};

  //! Dictionary of params
  IndexParams parameters_{};
};


/*!
 * Collection Status
 */
enum class CollectionStatus : uint32_t {
  INITIALIZED = 0,  // Collection has been initialized, ready to serving
  SERVING,          // Collection is serving
  DROPPED,          // Collection has been dropped
};


//! Alias ForwardColumn
using ForwardColumn = std::string;
//! Alias ForwardColumns
using ForwardColumns = std::vector<ForwardColumn>;


/*!
 * Types of Repository
 */
enum class RepositoryTypes : int {
  //! Undefined repo type
  UNDEFINED,
  //! DATABASE
  DATABASE
};

/*!
 * Repository Base Object
 */
class RepositoryBase {
 public:
  //! Constructors
  RepositoryBase() = default;

  explicit RepositoryBase(RepositoryTypes repo_type) : type_(repo_type) {}

  explicit RepositoryBase(const RepositoryBase &repo)
      : name_(repo.name_), type_(repo.type_) {}

  //! Destructor
  virtual ~RepositoryBase() = default;

 public:
  //! Validate repository object
  virtual int validate() const {
    META_VERIFY_ARGUMENTS(
        name_.empty(), PROXIMA_BE_ERROR_CODE(EmptyRepositoryName),
        "Invalid arguments for create collection, empty repository name.")
    return 0;
  }

  //! Retrieve repository name
  const std::string &name() const {
    return name_;
  }

  //! Retrieve mutable repository name
  std::string *mutable_name() {
    return &name_;
  }

  //! Set repository name
  void set_name(const std::string &new_name) {
    name_ = new_name;
  }

  RepositoryTypes type() const {
    return type_;
  }

  void set_type(RepositoryTypes new_type) {
    type_ = new_type;
  }

 private:
  //! Name of repository
  std::string name_{};

  //! Type of repository
  RepositoryTypes type_{RepositoryTypes::UNDEFINED};
};


/*!
 * Repository Meta
 */
class DatabaseRepositoryMeta : public RepositoryBase {
 public:
  //! Constructors
  DatabaseRepositoryMeta() : RepositoryBase(RepositoryTypes::DATABASE) {}

  explicit DatabaseRepositoryMeta(const DatabaseRepositoryMeta &param)
      : RepositoryBase(param),
        connection_(param.connection_),
        user_(param.user_),
        password_(param.password_),
        table_name_(param.table_name_) {}

  //! Destructor
  ~DatabaseRepositoryMeta() override = default;

 public:
  //! Validate repository object
  int validate() const override {
    int code = RepositoryBase::validate();
    if (code != 0) {
      return code;
    }

    ailego::Uri uri(connection_);
    META_VERIFY_ARGUMENTS(!uri.is_valid(), PROXIMA_BE_ERROR_CODE(InvalidURI),
                          "Invalid arguments for create collection, URI is "
                          "invalid.")

    META_VERIFY_ARGUMENTS(
        user_.empty(), PROXIMA_BE_ERROR_CODE(EmptyUserName),
        "Invalid arguments for create collection, empty user name.")

    META_VERIFY_ARGUMENTS(
        password_.empty(), PROXIMA_BE_ERROR_CODE(EmptyPassword),
        "Invalid arguments for create collection, empty password.")

    META_VERIFY_ARGUMENTS(table_name_.empty(),
                          PROXIMA_BE_ERROR_CODE(EmptyRepositoryTable),
                          "Invalid arguments for create collection, "
                          "repository_table can't be empty")
    return 0;
  }

  //! merge repository
  int merge_repository(DatabaseRepositoryMetaPtr repo) {
    set_name(repo->name());
    connection_.assign(repo->connection());
    user_.assign(repo->user());
    password_.assign(repo->password());
    table_name_.assign(repo->table_name());
    return 0;
  }

  //! Retrieve connection uri
  const std::string &connection() const {
    return connection_;
  }

  //! Retrieve mutable connection uris
  std::string *mutable_connection() {
    return &connection_;
  }

  //! Set connection uri
  void set_connection(const std::string &uri) {
    connection_ = uri;
  }

  //! Retrieve user of repository
  const std::string &user() const {
    return user_;
  }

  //! Retrieve mutable user of repository
  std::string *mutable_user() {
    return &user_;
  }

  //! Set user of repository
  void set_user(const std::string &user_name) {
    user_ = user_name;
  }

  //! Retrieve password of user
  const std::string &password() const {
    return password_;
  }

  //! Retrieve mutable password of user
  std::string *mutable_password() {
    return &password_;
  }

  //! Set password
  void set_password(const std::string &pass) {
    password_ = pass;
  }

  //! Retrieve the table of repository
  const std::string &table_name() const {
    return table_name_;
  }

  //! Retrieve the mutable table of repository
  std::string *mutable_table_name() {
    return &table_name_;
  }

  //! Set repository_table
  void set_table_name(const std::string &new_table_name) {
    table_name_ = new_table_name;
  }

 private:
  //! JDBC Connection URI
  std::string connection_{};

  //! User name
  std::string user_{};

  //! User password
  std::string password_{};

  //! Source table of repository
  std::string table_name_{};
};

//! Repository type helper
struct RepositoryHelper {
  //! Get child pointer from base pointer
  template <typename REPO, typename = typename std::enable_if<std::is_base_of<
                               RepositoryBase, REPO>::value>::type>
  static std::shared_ptr<typename std::remove_cv<REPO>::type> Child(
      RepositoryBasePtr repo) {
    typedef typename std::remove_cv<REPO>::type CHILD;
    return std::dynamic_pointer_cast<CHILD>(repo);
  }

  //! Construct one child from base pointer
  template <typename REPO, typename = typename std::enable_if<std::is_base_of<
                               RepositoryBase, REPO>::value>::type>
  static std::shared_ptr<typename std::remove_cv<REPO>::type> NewChild(
      RepositoryBasePtr repo) {
    typedef typename std::remove_cv<REPO>::type CHILD;
    auto child = RepositoryHelper::Child<CHILD>(repo);
    return std::make_shared<CHILD>(*child);
  }

  //! Copy child repository from base pointer
  static RepositoryBasePtr CopyRepository(RepositoryBasePtr base) {
    if (base) {
      switch (base->type()) {
        case RepositoryTypes::DATABASE:
          return RepositoryHelper::NewChild<DatabaseRepositoryMeta>(base);
        case RepositoryTypes::UNDEFINED:
          LOG_WARN("Ignore undefined repository type");
      }
    }
    return nullptr;
  }
};

/*!
 * CollectionBase object
 */
class CollectionBase {
 public:
  constexpr static uint64_t UNLIMITED_DOCS_PER_SEGMENT =
      std::numeric_limits<uint64_t>::max();

 public:
  //! Constructors
  CollectionBase() = default;

  explicit CollectionBase(const CollectionBase &param)
      : name_(param.name_),
        max_docs_per_segment_(param.max_docs_per_segment_),
        forward_columns_(param.forward_columns_) {
    for (auto column : param.index_columns_) {
      index_columns_.emplace_back(std::make_shared<ColumnMeta>(*column));
    }

    if (param.repository_) {
      repository_ = RepositoryHelper::CopyRepository(param.repository_);
    }
  }

  //! Destructor
  virtual ~CollectionBase() = default;

 public:
  //! Validate params, 0 for valid, otherwise invalid
  virtual int validate() const {
    META_VERIFY_ARGUMENTS(name_.empty(),
                          PROXIMA_BE_ERROR_CODE(EmptyCollectionName),
                          "Invalid name of collection")
    META_VERIFY_ARGUMENTS(max_docs_per_segment_ == 0,
                          PROXIMA_BE_ERROR_CODE(ZeroDocsPerSegment),
                          "Max doc per segment can't be 0")
    META_VERIFY_ARGUMENTS(index_columns_.empty(),
                          PROXIMA_BE_ERROR_CODE(EmptyColumns), "Empty Columns")

    int code = 0;
    for (auto &index : index_columns_) {
      code = index->validate();
      if (code != 0) {
        break;
      }
    }

    if (code == 0 && repository_) {
      code = repository_->validate();
    }
    return code;
  }

  //! Retrieve name
  const std::string &name() const {
    return name_;
  }

  //! Retrieve mutable name
  std::string *mutable_name() {
    return &name_;
  }

  //! Set name
  void set_name(const std::string &new_name) {
    name_ = new_name;
  }

  //! Retrieve split index size
  uint64_t max_docs_per_segment() const {
    return max_docs_per_segment_;
  }

  //! Set split index size, 0 means unlimited segment size
  void set_max_docs_per_segment(uint64_t count) {
    if (count == 0) {
      count = UNLIMITED_DOCS_PER_SEGMENT;
    }
    max_docs_per_segment_ = count;
  }

  //! Retrieve forward columns
  const ForwardColumns &forward_columns() const {
    return forward_columns_;
  }

  //! Retrieve mutable forward columns
  ForwardColumns *mutable_forward_columns() {
    return &forward_columns_;
  }

  //! Retrieve index columns
  const ColumnMetaPtrList &index_columns() const {
    return index_columns_;
  }

  //! Retrieve mutable index columns
  ColumnMetaPtrList *mutable_index_columns() {
    return &index_columns_;
  }

  //! Append column
  void append(ColumnMetaPtr param) {
    index_columns_.emplace_back(param);
  }

  //! Append column
  void append(const ColumnMeta &param) {
    index_columns_.emplace_back(std::make_shared<ColumnMeta>(param));
  }

  //! Retrieve repository member
  const RepositoryBasePtr repository() const {
    return repository_;
  }

  //! Set repository
  void set_repository(RepositoryBasePtr repo) {
    repository_ = repo;
  }

  const std::string &repository_name() const {
    static const std::string kEmpty = "";
    if (repository_) {
      return repository_->name();
    }
    return kEmpty;
  }

 public:
  //! Update columns
  int update_columns(const ColumnMetaPtrList &columns) {
    ColumnMetaPtrList merged_columns;
    for (auto column : columns) {
      auto ptr = column_by_name(column->name());
      if (ptr) {
        // Does not support update any of fields of column yet, so copy column
        // to newer collection.
        int code = ptr->check_updated_field(*column);
        if (code != 0) {
          return code;
        }
        merged_columns.emplace_back(std::make_shared<ColumnMeta>(*ptr));
      } else {
        auto new_column = std::make_shared<ColumnMeta>(*column);
        new_column->set_uid(gen_uuid());
        merged_columns.emplace_back(new_column);
      }
    }
    index_columns_ = merged_columns;
    return 0;
  }

  //! Find column which named by name
  ColumnMetaPtr column_by_name(const std::string &column_name) const {
    return find_column_by_filter(
        [&column_name](ColumnMetaPtr column_ptr) -> bool {
          return column_ptr->name() == column_name;
        });
  }

 private:
  //! Find column by filter
  ColumnMetaPtr find_column_by_filter(
      std::function<bool(ColumnMetaPtr)> filter) const {
    for (auto &c : index_columns_) {
      if (filter(c)) {
        return c;
      }
    }
    return nullptr;
  }

 private:
  //! The name of collection, unique name
  std::string name_{};

  //! The max document count of segments, optional field, default is Max
  // value of system.
  uint64_t max_docs_per_segment_{0};

  //! Forward columns
  ForwardColumns forward_columns_{};

  //! Indices
  ColumnMetaPtrList index_columns_{};

  //! Repository of collection, Optional field
  RepositoryBasePtr repository_{nullptr};
};


/*!
 * CollectionMeta
 */
class CollectionMeta : public CollectionBase {
 public:
  //! InvalidRevision
  constexpr static uint32_t INVALID_REVISION =
      std::numeric_limits<uint32_t>::max();

 public:
  //! Constructors
  CollectionMeta()
      : CollectionBase(),
        uid_(gen_uuid()),
        readable_(true),
        writable_(true),
        current_(true) {}

  explicit CollectionMeta(const CollectionMeta &meta)
      : CollectionBase(meta),
        uid_(meta.uid_),
        readable_(meta.readable_),
        writable_(meta.writable_),
        revision_(meta.revision_),
        status_(meta.status_),
        current_(meta.current_) {}

  explicit CollectionMeta(const CollectionBase &param)
      : CollectionBase(param),
        uid_(gen_uuid()),
        readable_(true),
        writable_(true),
        current_(true) {}

  //! Destructor
  ~CollectionMeta() override = default;

 public:
  //! Retrieve collection uid, indicate a group of collections
  const std::string &uid() const {
    return uid_;
  }

  //! Retrieve mutable collection uid, indicate a group of collections
  std::string *mutable_uid() {
    return &uid_;
  }

  //! Set collection uid
  void set_uid(const std::string &new_uid) {
    uid_ = new_uid;
  }

  //! Check readable
  bool readable() const {
    return readable_;
  }

  //! Set readable
  void set_readable(bool flag) {
    readable_ = flag;
  }

  //! Check writable
  bool writable() const {
    return writable_;
  }

  //! Set writable
  void set_writable(bool flag) {
    writable_ = flag;
  }

  //! Retrieve revision
  uint32_t revision() const {
    return revision_;
  }

  //! Set revision
  void set_revision(uint32_t new_revision) {
    revision_ = new_revision;
  }

  //! Invalid revision
  bool invalid_revision() const {
    return revision_ == INVALID_REVISION;
  }

  //! Increase revision number
  void increase_revision(uint32_t step = 1) {
    revision_ += step;
  }

  //! Retrieve status
  CollectionStatus status() const {
    return status_;
  }

  //! Check is INITIALIZED
  bool initialized() const {
    return status_ == CollectionStatus::INITIALIZED;
  }

  //! Check is serving
  bool serving() const {
    return status_ == CollectionStatus::SERVING;
  }

  //! Set status
  void set_status(CollectionStatus new_status) {
    status_ = new_status;
  }

  //! Check current flag
  bool is_current() const {
    return current_;
  }

  //! Set current flag
  void set_current(bool flag = true) {
    current_ = flag;
  }

  //! Merge update param
  int merge_update_param(const CollectionBase &param) {
    set_max_docs_per_segment(param.max_docs_per_segment());
    mutable_forward_columns()->assign(param.forward_columns().begin(),
                                      param.forward_columns().end());

    if (repository() && param.repository()) {
      auto repo = RepositoryHelper::Child<DatabaseRepositoryMeta>(repository());
      repo->merge_repository(
          RepositoryHelper::Child<DatabaseRepositoryMeta>(param.repository()));
    } else if (!repository() && !param.repository()) {
    } else {
      return PROXIMA_BE_ERROR_CODE(UpdateRepositoryTypeField);
    }

    return update_columns(param.index_columns());
  }

 private:
  //! Unique id of collection
  std::string uid_{};

  //! Readable flag, default is true
  bool readable_{false};

  //! Writable flag, default is true
  bool writable_{false};

  //! Revision number
  uint32_t revision_{0};

  //! One of values: initialized, serving ...
  CollectionStatus status_{CollectionStatus::INITIALIZED};

  //! True for indicated current used version, otherwise false
  bool current_{false};
};

#undef META_VERIFY_ARGUMENTS

}  // namespace meta
}  // namespace be
}  // namespace proxima
