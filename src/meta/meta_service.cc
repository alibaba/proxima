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

#include "meta_service.h"
#include <mutex>
#include <ailego/encoding/uri.h>
#include <ailego/parallel/lock.h>
#include "common/error_code.h"
#include "meta_cache.h"
#include "meta_impl.h"
#include "meta_service_builder.h"
#include "meta_store_factory.h"

namespace proxima {
namespace be {
namespace meta {

/*! MetaServiceImpl
 */
class MetaServiceImpl : public MetaService {
 public:
  //! Constructor
  //! @param uri: Identifier with database
  explicit MetaServiceImpl(MetaStorePtr store, MetaCachePtr cache);

  //! Disable default constructor and copy constructor
  MetaServiceImpl() = delete;
  MetaServiceImpl(const MetaServiceImpl &) = delete;

  // Destructor
  ~MetaServiceImpl() override;

 public:
  // Interface inherent from Service
  int init_impl() override;

  int cleanup_impl() override;

  int start_impl() override;

  int stop_impl() override;

 public:
  //! Reload meta service
  int reload() override;

  //! Create collection and columns
  int create_collection(const CollectionBase &param,
                        CollectionMetaPtr *meta) override;

  //! Update collection and columns, increase revision and copy a new collection
  int update_collection(const CollectionBase &param,
                        CollectionMetaPtr *meta) override;

  //! Enable collection
  int enable_collection(const std::string &collection, uint32_t revision,
                        bool enable) override;

  //! Update the status of current used collection
  int update_status(const std::string &collection_name,
                    CollectionStatus status) override;

  //! Suspend reading requests of collection
  int suspend_collection_read(const std::string &collection_name) override;

  //! Resume reading requests of collection
  int resume_collection_read(const std::string &collection_name) override;

  //! Suspend writing requests of collection
  int suspend_collection_write(const std::string &collection_name) override;

  //! Resume writing requests of collection
  int resume_collection_write(const std::string &collection_name) override;

  //! Drop collection
  int drop_collection(const std::string &name) override;

  //! Retrieve latest version of collection
  CollectionMetaPtr get_current_collection(
      const std::string &name) const override;

  //! Retrieve latest version of collections
  int get_latest_collections(CollectionMetaPtrList *collections) const override;

  //! Retrieve all of collections
  int get_collections(CollectionMetaPtrList *collections) const override;

  //! Retrieve collections with specific repository
  int get_collections_by_repo(
      const std::string &repository,
      CollectionMetaPtrList *collections) const override;

  //! Retrieve collections with specific collection name
  int get_collections(const std::string &name,
                      CollectionMetaPtrList *collections) const override;

  //! Retrieve collection
  CollectionMetaPtr get_collection(const std::string &name,
                                   uint64_t revision) const override;

  //! Check collection exists
  bool exist_collection(const std::string &collection) const override;

 private:
  //! Retrieve collection
  const CollectionImplPtr &inner_get_collection(const std::string &name,
                                                uint64_t revision) const;

  //! Update current collection
  int update_current_used_collection(
      const std::string collection_name,
      std::function<int(CollectionImplPtr)> handler);

  //! Cleanup cache without lock
  inline void cleanup_cache() {
    cache_->clear();
  }

  //! Load meta store to cache with out lock
  int load_meta_cache();

  //! load collections from meta store
  int load_collections();

  //! load columns from meta store
  int load_columns();

  //! load repositories from meta store
  int load_repositories();

  //! load meta from meta store
  int load_meta_store();

  //! store collection fo meta store
  int store_collection(const CollectionImplPtr &collection);

 private:
  //! Mutex for cache access
  mutable ailego::SharedMutex mutex_{};

  //! Meta Store
  MetaStorePtr store_{nullptr};

  //! Meta Cache
  MetaCachePtr cache_{nullptr};
};  // end of MetaServiceImpl


MetaServicePtr MetaServiceBuilder::Create(const std::string &uri_str) {
  ailego::Uri uri(uri_str);
  if (!uri.is_valid()) {
    LOG_ERROR("Failed to parse uri, initialize MetaServiceImpl failed. uri[%s]",
              uri_str.c_str());
    return nullptr;
  }

  MetaStorePtr store =
      MetaStoreFactory::Instance().create(uri.scheme().c_str(), &uri);
  if (!store) {
    LOG_ERROR(
        "MetaServiceImpl init failed. reason[can't get meta store instance]");
    return nullptr;
  }

  return std::make_shared<MetaServiceImpl>(store,
                                           std::make_shared<MetaCache>());
}

int MetaServiceImpl::load_collections() {
  CollectionImplPtr collection{nullptr};
  // Load collection to cache
  int code =
      store_->list_collections([this, &collection]() -> CollectionObject * {
        if (collection) {  // Append collection to cache
          collection->transform();
          cache_->append_collection(collection);
          collection.reset();
        }
        collection = std::make_shared<CollectionImpl>();
        return collection.get();
      });
  if (code == 0 && collection) {  // Append last collection
    collection->transform();
    cache_->append_collection(collection);
  }
  if (code != 0) {  // Failed
    LOG_ERROR("Failed to load collection from meta store. code[%d]", code);
    return PROXIMA_BE_ERROR_CODE(RuntimeError);
  }
  return code;
}

int MetaServiceImpl::load_columns() {
  ColumnImplPtr column{nullptr};
  // Load columns into cache
  int code = store_->list_columns([this, &column]() -> ColumnObject * {
    if (column) {  // Append column to cache
      column->transform();
      cache_->append_column(column);
      column.reset();
    }
    column = std::make_shared<ColumnImpl>();
    return column.get();
  });
  if (code == 0 && column) {  // Append last column to cache
    column->transform();
    cache_->append_column(column);
  }
  if (code != 0) {
    LOG_ERROR("Failed to load columns from meta store. code[%d]", code);
    return PROXIMA_BE_ERROR_CODE(RuntimeError);
  }
  return code;
}

int MetaServiceImpl::load_repositories() {
  DatabaseRepositoryImplPtr repository{nullptr};
  // Load columns into cache
  int code = store_->list_repositories(
      [this, &repository]() -> DatabaseRepositoryObject * {
        if (repository) {  // Attach to collection
          cache_->append_repository(repository);
        }
        repository = std::make_shared<DatabaseRepositoryImpl>();
        return repository.get();
      });
  if (code == 0 && repository) {  // Append last repository to cache
    cache_->append_repository(repository);
  }
  if (code != 0) {
    LOG_ERROR("Failed to load repository from meta store. code[%d]", code);
    return PROXIMA_BE_ERROR_CODE(RuntimeError);
  }
  return code;
}

int MetaServiceImpl::load_meta_store() {
  int code = load_collections();
  if (code == 0) {
    code = load_columns();
    if (code == 0) {
      code = load_repositories();
    }
  }
  if (code != 0) {
    cache_->clear();
  }
  return code;
}

int MetaServiceImpl::store_collection(const CollectionImplPtr &collection) {
  int code = store_->create_collection(*collection);
  if (code != 0) {
    return code;
  }

  if (collection->repository()) {
    code = store_->create_repository(*collection->repository());
    if (code != 0) {
      store_->delete_repositories_by_uuid(collection->uuid());
      store_->delete_collection_by_uuid(collection->uuid());
      return code;
    }
  }

  for (auto &column : collection->columns()) {
    code = store_->create_column(*column);
    if (code != 0) {
      store_->delete_columns_by_uuid(collection->uuid());
      store_->delete_repositories_by_uuid(collection->uuid());
      store_->delete_collection_by_uuid(collection->uuid());
      break;
    }
  }
  return code;
}

#define META_WRITE_LOCK_GUARD(MUTEX, GUARD) \
  ailego::WriteLock write_lock(MUTEX);      \
  std::lock_guard<ailego::WriteLock> GUARD(write_lock)

#define META_READ_LOCK_GUARD(MUTEX, GUARD) \
  ailego::ReadLock read_lock(MUTEX);       \
  std::lock_guard<ailego::ReadLock> GUARD(read_lock)


MetaServiceImpl::MetaServiceImpl(MetaStorePtr store, MetaCachePtr cache)
    : store_(std::move(store)), cache_(std::move(cache)) {}

MetaServiceImpl::~MetaServiceImpl() = default;

// Interface inherent from Service
int MetaServiceImpl::init_impl() {
  META_WRITE_LOCK_GUARD(mutex_, guard);
  return load_meta_cache();
}

int MetaServiceImpl::cleanup_impl() {
  META_WRITE_LOCK_GUARD(mutex_, guard);
  cleanup_cache();
  return 0;
}

int MetaServiceImpl::start_impl() {
  return 0;
}

int MetaServiceImpl::stop_impl() {
  return 0;
}

int MetaServiceImpl::reload() {
  LOG_INFO("Reload meta service.");
  META_WRITE_LOCK_GUARD(mutex_, guard);

  LOG_DEBUG("Cleanup meta cache.");
  cleanup_cache();
  LOG_DEBUG("Reload meta cache.");
  int code = load_meta_cache();
  if (code == 0) {
    LOG_INFO("Reload meta service successes.");
  } else {
    LOG_INFO("Reload meta service failed. code[%d] what[%s]", code,
             ErrorCode::What(code));
  }
  return code;
}

int MetaServiceImpl::create_collection(const CollectionBase &param,
                                       CollectionMetaPtr *out) {
  META_WRITE_LOCK_GUARD(mutex_, guard);

  if (cache_->exist_collection(param.name())) {
    return PROXIMA_BE_ERROR_CODE(DuplicateCollection);
  }

  CollectionMetaPtr meta = std::make_shared<CollectionMeta>(param);
  int code = meta->validate();
  if (code != 0) {
    LOG_ERROR("Meta was invalid. code[%d] what[%s]", code,
              ErrorCode::What(code));
    return code;
  }
  CollectionImplPtr collection = std::make_shared<CollectionImpl>(meta);
  collection->transform();

  code = store_collection(collection);
  if (code == 0) {
    cache_->append_collection(collection);
  } else {
    LOG_ERROR("Failed to store collection. code[%d]", code);
  }

  if (code == 0 && out != nullptr) {
    *out = meta;
  }
  return code;
}

//! Update collection and columns, increase revision and copy a new collection
int MetaServiceImpl::update_collection(const CollectionBase &param,
                                       CollectionMetaPtr *out) {
  META_WRITE_LOCK_GUARD(mutex_, guard);

  CollectionImplPtr latest = cache_->get_latest_collection(param.name());
  if (!latest) {
    return PROXIMA_BE_ERROR_CODE(InexistentCollection);
  }

  // Create new meta from latest revision
  CollectionMetaPtr meta = std::make_shared<CollectionMeta>(*latest->meta());
  int code = meta->merge_update_param(param);
  if (code != 0) {
    LOG_ERROR("Readonly field updated. code[%d] what[%s]", code,
              ErrorCode::What(code));
    return code;
  }
  code = meta->validate();
  if (code != 0) {
    LOG_ERROR("Update collection failed. code[%d] what[%s]", code,
              ErrorCode::What(code));
    return code;
  }
  // Increase revision of collection
  meta->increase_revision();
  meta->set_current(false);

  // Copy to Collection Impl
  CollectionImplPtr next = std::make_shared<CollectionImpl>(meta);
  next->transform();

  // Store collection and append to cache
  code = store_collection(next);
  if (code != 0) {
    LOG_ERROR("Failed to update collection. code[%d]", code);
  } else {
    cache_->append_collection(next);
  }

  // Write back of results
  if (code == 0 && out != nullptr) {
    *out = meta;
  }

  return code;
}

//! Enable collection
int MetaServiceImpl::enable_collection(const std::string &name,
                                       uint32_t revision, bool) {
  META_WRITE_LOCK_GUARD(mutex_, guard);

  int code = PROXIMA_BE_ERROR_CODE(InexistentCollection);
  CollectionImplPtr current = cache_->get_collection(name);
  auto &next = inner_get_collection(name, revision);
  if (!current || !next) {
    LOG_ERROR("Can't get collection by name or by revision.");
  } else {
    if (current != next) {  // Not same
      current->meta()->set_readable(false);
      current->meta()->set_writable(false);
      current->meta()->set_current(false);
      store_->update_collection(*current);
    }

    next->meta()->set_status(CollectionStatus::SERVING);
    next->meta()->set_current(true);
    code = store_->update_collection(*next);
  }
  return code;
}

//! Update the status of current used collection
int MetaServiceImpl::update_status(const std::string &name,
                                   CollectionStatus stat) {
  META_WRITE_LOCK_GUARD(mutex_, guard);

  return update_current_used_collection(
      name, [&stat](CollectionImplPtr current) -> int {
        current->meta()->set_status(stat);
        return 0;
      });
}

//! Suspend reading requests of collection
int MetaServiceImpl::suspend_collection_read(
    const std::string &collection_name) {
  META_WRITE_LOCK_GUARD(mutex_, guard);

  return update_current_used_collection(collection_name,
                                        [](CollectionImplPtr current) -> int {
                                          current->meta()->set_readable(false);
                                          return 0;
                                        });
}

//! Resume reading requests of collection
int MetaServiceImpl::resume_collection_read(
    const std::string &collection_name) {
  META_WRITE_LOCK_GUARD(mutex_, guard);

  return update_current_used_collection(collection_name,
                                        [](CollectionImplPtr current) -> int {
                                          current->meta()->set_readable(true);
                                          return 0;
                                        });
}

//! Suspend writing requests of collection
int MetaServiceImpl::suspend_collection_write(
    const std::string &collection_name) {
  META_WRITE_LOCK_GUARD(mutex_, guard);

  return update_current_used_collection(collection_name,
                                        [](CollectionImplPtr current) -> int {
                                          current->meta()->set_writable(false);
                                          return 0;
                                        });
}

//! Resume writing requests of collection
int MetaServiceImpl::resume_collection_write(
    const std::string &collection_name) {
  META_WRITE_LOCK_GUARD(mutex_, guard);

  return update_current_used_collection(collection_name,
                                        [](CollectionImplPtr current) -> int {
                                          current->meta()->set_writable(true);
                                          return 0;
                                        });
}

//! Drop collection
int MetaServiceImpl::drop_collection(const std::string &name) {
  META_WRITE_LOCK_GUARD(mutex_, guard);

  CollectionImplPtr current = cache_->get_collection(name);
  if (current) {
    cache_->delete_collection(name);
    int code = store_->delete_collection(name);
    if (code == 0) {
      code = store_->delete_columns_by_uid(current->uid());
      if (code == 0) {
        code = store_->delete_repositories_by_uid(current->uid());
      }
    }
    return code;
  }
  return 0;
}

//! Retrieve latest version of collection
CollectionMetaPtr MetaServiceImpl::get_current_collection(
    const std::string &name) const {
  META_READ_LOCK_GUARD(mutex_, guard);

  auto current = cache_->get_collection(name);
  return current ? current->meta() : nullptr;
}

//! Retrieve latest version of collections
int MetaServiceImpl::get_latest_collections(
    CollectionMetaPtrList *collections) const {
  if (!collections) {
    return PROXIMA_BE_ERROR_CODE(InvalidArgument);
  }

  META_READ_LOCK_GUARD(mutex_, guard);

  cache_->get_collections(collections);
  return 0;
}

//! Retrieve all of collections
int MetaServiceImpl::get_collections(CollectionMetaPtrList *collections) const {
  META_READ_LOCK_GUARD(mutex_, guard);

  cache_->get_collections(MetaCache::PassAllFilter, collections);
  return 0;
}

//! Retrieve collections with specific repository
int MetaServiceImpl::get_collections_by_repo(
    const std::string &repo, CollectionMetaPtrList *collections) const {
  META_READ_LOCK_GUARD(mutex_, guard);

  cache_->get_collections_by_repo(repo, collections);
  return 0;
}

//! Retrieve collections with specific collection name
int MetaServiceImpl::get_collections(const std::string &name,
                                     CollectionMetaPtrList *collections) const {
  META_READ_LOCK_GUARD(mutex_, guard);

  cache_->get_collections(name, collections);
  return collections->empty() ? PROXIMA_BE_ERROR_CODE(InexistentCollection) : 0;
}

//! Retrieve collection
CollectionMetaPtr MetaServiceImpl::get_collection(const std::string &name,
                                                  uint64_t revision) const {
  META_READ_LOCK_GUARD(mutex_, guard);

  auto &impl = inner_get_collection(name, revision);
  return impl ? impl->meta() : nullptr;
}

//! Check collection exists
bool MetaServiceImpl::exist_collection(const std::string &collection) const {
  META_READ_LOCK_GUARD(mutex_, guard);

  return cache_->exist_collection(collection);
}

const CollectionImplPtr &MetaServiceImpl::inner_get_collection(
    const std::string &name, uint64_t revision) const {
  return cache_->get_collection(
      name, [&revision](const CollectionImplPtr &collection) -> bool {
        return collection->revision() == revision;
      });
}

int MetaServiceImpl::update_current_used_collection(
    const std::string collection_name,
    std::function<int(CollectionImplPtr)> handler) {
  int code = PROXIMA_BE_ERROR_CODE(InexistentCollection);
  CollectionImplPtr current = cache_->get_collection(collection_name);
  if (current) {
    code = handler(current);
    if (code == 0) {
      store_->update_collection(*current);
    } else {
      LOG_ERROR("Update collection failed. collection[%s]",
                current->name().c_str());
    }
  }
  return code;
}

int MetaServiceImpl::load_meta_cache() {
  // Load all the collections from meta store
  int code = load_meta_store();
  if (code != 0) {
    cleanup_impl();
    LOG_ERROR("Failed to load collection into cache.");
    code = PROXIMA_BE_ERROR_CODE(RuntimeError);
  }
  return code;
}

#undef META_WRITE_LOCK_GUARD
#undef META_READ_LOCK_GUARD

}  // namespace meta
}  // namespace be
}  // namespace proxima
