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

#include <mutex>
#include <unordered_map>
#include "meta.h"
#include "meta_impl.h"

namespace proxima {
namespace be {
namespace meta {

//! Predefined class
class MetaCache;
//! Alias for MetaCache
using MetaCachePtr = std::shared_ptr<MetaCache>;


//! Alias for CollectionMetaMap
using CollectionMetaMap =
    std::unordered_map<std::string, CollectionImplPtrList>;
using CollectionImplMap = std::unordered_map<std::string, CollectionImplPtr>;


//! Alias CollectionFilter
using CollectionFilter = std::function<bool(const CollectionImplPtr &)>;


/*!
 * MetaCache implementation
 */
class MetaCache {
 public:
  //! Default Filters
  static CollectionFilter PassAllFilter;
  static CollectionFilter IsCurrentFilter;

 public:
  //! Constructor
  MetaCache() = default;

  //! Destructor
  ~MetaCache() = default;

 public:
  //! Clear cache
  void clear();

  //! Delete collections
  void delete_collection(const std::string &name);

  //! Has collection
  bool exist_collection(const std::string &name) const;

  //! Retrieve collection by name
  const CollectionImplPtr &get_collection(const std::string &name) const;

  //! Retrieve collection by name, with max revision id
  const CollectionImplPtr &get_latest_collection(const std::string &name) const;

  //! Retrieve collection
  const CollectionImplPtr &get_collection(const std::string &name,
                                          const CollectionFilter &filter) const;

  //! Retrieve all the latest collections
  void get_collections(CollectionMetaPtrList *collections) const;

  //! Retrieve collections with specific collection name
  void get_collections(const std::string &collection,
                       CollectionMetaPtrList *collections) const;

  //! Retrieve all the collections
  void get_collections(const CollectionFilter &filter,
                       CollectionMetaPtrList *collections) const;

  //! Retrieve collections with specific repository
  void get_collections_by_repo(const std::string &repo,
                               CollectionMetaPtrList *collections) const;


  //! Append collection to the cache
  int append_collection(const CollectionImplPtr &collection);

  //! Append column to the cache
  int append_column(ColumnImplPtr column);

  //! Append repository to collection
  int append_repository(DatabaseRepositoryImplPtr repository);

 private:
  //! Collection Cache
  CollectionMetaMap cache_{};

  //! Collection mapping, which help to allocate column with collection
  CollectionImplMap mapping_{};
};


}  // namespace meta
}  // namespace be
}  // namespace proxima
