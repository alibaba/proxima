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

#include <vector>
#include <ailego/encoding/uri.h>
#include "meta_types.h"

namespace proxima {
namespace be {
namespace meta {

//! Predefine class
class MetaStore;
//! Alias for MetaStore
using MetaStorePtr = std::shared_ptr<MetaStore>;


//! Alias for object allocator
using CollectionAllocator = std::function<CollectionObject *()>;
using ColumnAllocator = std::function<ColumnObject *()>;
using DatabaseRepositoryAllocator = std::function<DatabaseRepositoryObject *()>;

/*!
 * Meta store
 */
class MetaStore {
 public:
  //! Destructor
  virtual ~MetaStore() = default;

 public:
  //! initialize metastore
  virtual int initialize(const ailego::Uri *uri) = 0;

  //! cleanup
  virtual int cleanup() = 0;

  /** CRUD of CollectionObject **/
  //! Create the collection
  virtual int create_collection(const CollectionObject &collection) = 0;

  //! Update collection by id, all field should be updated except id(most of
  // them was identical)
  virtual int update_collection(const CollectionObject &collection) = 0;

  //! Delete collection
  virtual int delete_collection(const std::string &name) = 0;

  //! Delete collection
  virtual int delete_collection_by_uuid(const std::string &name) = 0;

  //! Retrieve all collections
  virtual int list_collections(CollectionAllocator allocator) const = 0;

  /** CRUD of ColumnObject **/
  //! Create column
  virtual int create_column(const ColumnObject &column) = 0;

  //! Delete columns
  virtual int delete_columns_by_uid(const std::string &uid) = 0;

  //! Delete columns
  virtual int delete_columns_by_uuid(const std::string &uuid) = 0;

  //! Retrieve all collections
  virtual int list_columns(ColumnAllocator allocator) const = 0;

  /** CRUD of RepositoryObject **/
  //! Create repository
  virtual int create_repository(const DatabaseRepositoryObject &repository) = 0;

  //! Delete repositories
  virtual int delete_repositories_by_uid(const std::string &uid) = 0;

  //! Delete repositories
  virtual int delete_repositories_by_uuid(const std::string &uuid) = 0;

  //! Retrieve all repositories
  virtual int list_repositories(
      DatabaseRepositoryAllocator allocator) const = 0;

  //! Flush all changes to storage
  virtual int flush() const = 0;
};


}  // namespace meta
}  // namespace be
}  // namespace proxima
