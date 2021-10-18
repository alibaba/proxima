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

#include <list>
#include "meta.h"

namespace proxima {
namespace be {
namespace meta {

//! Predefine class
class ColumnObject;
class CollectionObject;
class DatabaseRepositoryObject;

//! Alias pointer
using ColumnObjectPtr = std::shared_ptr<ColumnObject>;
using CollectionObjectPtr = std::shared_ptr<CollectionObject>;
using CollectionObjectList = std::list<CollectionObject>;
using CollectionListConstIter = CollectionObjectList::const_iterator;
using DatabaseRepositoryObjectPtr = std::shared_ptr<DatabaseRepositoryObject>;


/*!
 * Column interface, transform meta between meta_store and meta_service
 */
class ColumnObject {
 public:
  //! Destructor
  virtual ~ColumnObject() = default;

 public:
  //! Retrieve id
  virtual uint64_t id() const = 0;

  //! Set id
  virtual void set_id(uint64_t) = 0;

  //! Retrieve collection uid
  virtual const std::string &collection_uid() const = 0;

  //! Retrieve mutable collection uid
  virtual std::string *mutable_collection_uid() = 0;

  //! Set collection uid
  virtual void set_collection_uid(const std::string &) = 0;

  //! Retrieve collection uuid
  virtual const std::string &collection_uuid() const = 0;

  //! Retrieve mutable collection uuid
  virtual std::string *mutable_collection_uuid() = 0;

  //! Set collection uuid
  virtual void set_collection_uuid(const std::string &) = 0;

  //! Retrieve column name
  virtual const std::string &name() const = 0;

  //! Retrieve mutable column name
  virtual std::string *mutable_name() = 0;

  //! Set column name
  virtual void set_name(const std::string &) = 0;

  //! Retrieve column uid
  virtual const std::string &uid() const = 0;

  //! Retrieve mutable column uid
  virtual std::string *mutable_uid() = 0;

  //! Set column uid
  virtual void set_uid(const std::string &) = 0;

  //! Retrieve dimension of column
  virtual uint32_t dimension() const = 0;

  //! Set dimension of column
  virtual void set_dimension(uint32_t) = 0;

  //! Retrieve column index type
  virtual uint32_t index_type() const = 0;

  //! Set column index type
  virtual void set_index_type(uint32_t) = 0;

  //! Retrieve column data type
  virtual uint32_t data_type() const = 0;

  //! Set column index type
  virtual void set_data_type(uint32_t) = 0;

  //! Retrieve column parameters
  virtual const std::string &parameters() const = 0;

  //! Retrieve mutable column parameters
  virtual std::string *mutable_parameters() = 0;

  //! Set column parameters
  virtual void set_parameters(const std::string &) = 0;
};


/*!
 * Collection interface, transform meta between meta_store and
 * meta_service
 */
class CollectionObject {
 public:
  //! Destructor
  virtual ~CollectionObject() = default;

 public:
  //! Retrieve id
  virtual uint64_t id() const = 0;

  //! Set collection id
  virtual void set_id(uint64_t) = 0;

  //! Retrieve collection name
  virtual const std::string &name() const = 0;

  //! Retrieve mutable collection name
  virtual std::string *mutable_name() = 0;

  //! Set collection name
  virtual void set_name(const std::string &) = 0;

  //! Retrieve collection uid, indicate a group of collections
  virtual const std::string &uid() const = 0;

  //! Retrieve mutable collection uid, indicate a group of collections
  virtual std::string *mutable_uid() = 0;

  //! Set collection uid
  virtual void set_uid(const std::string &) = 0;

  //! Retrieve collection uuid, unique field
  virtual const std::string &uuid() const = 0;

  //! Retrieve mutable collection uuid, unique field
  virtual std::string *mutable_uuid() = 0;

  //! Set collection uuid
  virtual void set_uuid(const std::string &) = 0;

  //! Retrieve forward columns, separated by character ','
  virtual const std::string &forward_columns() const = 0;

  //! Retrieve mutable forward columns, separated by character ','
  virtual std::string *mutable_forward_columns() = 0;

  //! Set forward columns
  virtual void set_forward_columns(const std::string &) = 0;

  //! Retrieve split document count
  virtual uint64_t max_docs_per_segment() const = 0;

  //! Set split document count
  virtual void set_max_docs_per_segment(uint64_t) = 0;

  //! Retrieve collection revision
  virtual uint32_t revision() const = 0;

  //! Set collection revision
  virtual void set_revision(uint32_t) = 0;

  //! Retrieve collection status
  virtual uint32_t status() const = 0;

  //! Set collection status
  virtual void set_status(uint32_t) = 0;

  //! Retrieve collection current flag
  virtual uint32_t current() const = 0;

  //! Set collection current flag
  virtual void set_current(uint32_t) = 0;

  //! Retrieve io_mode of collection
  virtual uint32_t io_mode() const = 0;

  //! Set io_mode of collection
  virtual void set_io_mode(uint32_t mode) = 0;
};


/*!
 * Repository Object
 */
class DatabaseRepositoryObject {
 public:
  //! Destructor
  virtual ~DatabaseRepositoryObject() = default;

 public:
  //! Retrieve id
  virtual uint64_t id() const = 0;

  //! Set repository id
  virtual void set_id(uint64_t) = 0;

  //! Retrieve repository name
  virtual const std::string &name() const = 0;

  //! Retrieve mutable repository name
  virtual std::string *mutable_name() = 0;

  //! Set repository name
  virtual void set_name(const std::string &) = 0;

  //! Retrieve collection uid
  virtual const std::string &collection_uid() const = 0;

  //! Retrieve mutable collection uid
  virtual std::string *mutable_collection_uid() = 0;

  //! Set collection uid
  virtual void set_collection_uid(const std::string &) = 0;

  //! Retrieve collection uuid
  virtual const std::string &collection_uuid() const = 0;

  //! Retrieve mutable collection uuid
  virtual std::string *mutable_collection_uuid() = 0;

  //! Set collection uuid
  virtual void set_collection_uuid(const std::string &) = 0;

  //! Retrieve connection uri
  virtual const std::string &connection() const = 0;

  //! Retrieve mutable connection uri
  virtual std::string *mutable_connection() = 0;

  //! Set connection uri
  virtual void set_connection(const std::string &) = 0;

  //! Retrieve user name of connection
  virtual const std::string &user() const = 0;

  //! Retrieve mutable user name of connection
  virtual std::string *mutable_user() = 0;

  //! Set user name of connection
  virtual void set_user(const std::string &) = 0;

  //! Retrieve password of user
  virtual const std::string &password() const = 0;

  //! Retrieve mutable password of user
  virtual std::string *mutable_password() = 0;

  //! Set password of user
  virtual void set_password(const std::string &) = 0;

  //! Retrieve repository table name
  virtual const std::string &table() const = 0;

  //! Retrieve mutable repository table name
  virtual std::string *mutable_table() = 0;

  //! Set repository table name
  virtual void set_table(const std::string &) = 0;
};


}  // namespace meta
}  // namespace be
}  // namespace proxima
