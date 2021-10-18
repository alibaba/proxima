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

#include "meta_cache.h"
#include "common/error_code.h"
#include "common/logger.h"

namespace proxima {
namespace be {
namespace meta {

//! Select all filter
CollectionFilter MetaCache::PassAllFilter =
    [](const CollectionImplPtr &) -> bool { return true; };


//! Current Collection filter
CollectionFilter MetaCache::IsCurrentFilter =
    [](const CollectionImplPtr &collection) -> bool {
  return collection->meta()->is_current();
};

namespace {
// Static empty pointer
static const CollectionImplPtr EmptyCollectionPtr;

static const CollectionImplPtr &FindCollectionIf(
    const CollectionImplPtrList &collections, CollectionFilter filter) {
  static CollectionImplPtr NotFound{nullptr};
  auto collection =
      std::find_if_not(collections.begin(), collections.end(),
                       [&filter](const CollectionImplPtr &c) -> bool {
                         return filter ? !filter(c) : true;
                       });
  return collection != collections.end() ? *collection : NotFound;
}

}  // namespace
void MetaCache::clear() {
  cache_.clear();
  mapping_.clear();
}

void MetaCache::delete_collection(const std::string &name) {
  auto iter = cache_.find(name);
  if (iter != cache_.end()) {
    for (auto &collection : iter->second) {
      mapping_.erase(collection->uuid());
    }
    cache_.erase(iter);
  }
}

bool MetaCache::exist_collection(const std::string &name) const {
  return cache_.find(name) != cache_.end();
}

const CollectionImplPtr &MetaCache::get_collection(
    const std::string &name) const {
  return get_collection(name, IsCurrentFilter);
}

//! Retrieve collection by name
const CollectionImplPtr &MetaCache::get_latest_collection(
    const std::string &name) const {
  auto iter = cache_.find(name);
  if (iter != cache_.end()) {
    return *iter->second.begin();
  }
  return EmptyCollectionPtr;
}

//! Retrieve collection
const CollectionImplPtr &MetaCache::get_collection(
    const std::string &name, const CollectionFilter &filter) const {
  auto iter = cache_.find(name);
  if (iter != cache_.end()) {
    return FindCollectionIf(iter->second, filter);
  }
  return EmptyCollectionPtr;
}

void MetaCache::get_collections(CollectionMetaPtrList *collections) const {
  for (auto &iter : cache_) {
    auto &collection = FindCollectionIf(iter.second, IsCurrentFilter);
    if (collection) {
      collections->push_back(collection->meta());
    } else {
      LOG_WARN("There is collection, which not enabled yet");
    }
  }
}

//! Retrieve collections with specific collection name
void MetaCache::get_collections(const std::string &collection,
                                CollectionMetaPtrList *collections) const {
  auto iter = cache_.find(collection);
  if (iter != cache_.end()) {
    for (auto &collection_ptr : iter->second) {
      collections->push_back(collection_ptr->meta());
    }
  }
}

//! Retrieve all the collections
void MetaCache::get_collections(const CollectionFilter &filter,
                                CollectionMetaPtrList *collections) const {
  for (auto &iter : mapping_) {
    if (!filter || filter(iter.second)) {
      collections->push_back(iter.second->meta());
    }
  }
}

//! Retrieve collections with specific repository
void MetaCache::get_collections_by_repo(
    const std::string &repo, CollectionMetaPtrList *collections) const {
  for (auto &iter : cache_) {
    auto &collection = FindCollectionIf(
        iter.second, [&repo](const CollectionImplPtr &c) -> bool {
          return c->repository() && c->repository()->name() == repo &&
                 c->meta()->is_current() && c->serving();
        });
    if (collection) {
      collections->push_back(collection->meta());
    }
  }
}

int MetaCache::append_collection(const CollectionImplPtr &collection) {
  if (!collection) {
    return PROXIMA_BE_ERROR_CODE(InvalidArgument);
  }
  auto &collections = cache_[collection->name()];
  // Append collection
  collections.push_back(collection);
  // Sort collection
  collections.sort([](const CollectionImplPtr &left,
                      const CollectionImplPtr &right) -> bool {
    return left->revision() > right->revision();
  });

  // Append to uuid -> collection mapping
  mapping_[collection->uuid()] = collection;
  return 0;
}

int MetaCache::append_column(ColumnImplPtr column) {
  if (!column) {
    return PROXIMA_BE_ERROR_CODE(InvalidArgument);
  }

  auto iter = mapping_.find(column->collection_uuid());
  if (iter == mapping_.end()) {
    LOG_WARN("Can't find collection");
    return PROXIMA_BE_ERROR_CODE(InvalidArgument);
  }

  return iter->second->append(column);
}

int MetaCache::append_repository(DatabaseRepositoryImplPtr repository) {
  if (!repository) {
    return PROXIMA_BE_ERROR_CODE(InvalidArgument);
  }

  auto iter = mapping_.find(repository->collection_uuid());
  if (iter == mapping_.end()) {
    LOG_WARN("Can't find collection");
    return PROXIMA_BE_ERROR_CODE(InvalidArgument);
  }

  return iter->second->set_repository(repository);
}

}  // namespace meta
}  // namespace be
}  // namespace proxima
