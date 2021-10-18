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

#include "common/interface/service.h"
#include "meta.h"

namespace proxima {
namespace be {
namespace meta {

//! Predefined class
class MetaService;
//! MetaStore Alias
using MetaServicePtr = std::shared_ptr<MetaService>;


/*!
 * MetaService implementation
 */
class MetaService : public Service {
 public:
  // Destructor
  ~MetaService() override = default;

 public:
  //! Reload meta service
  virtual int reload() = 0;

  //! Create collection and columns
  virtual int create_collection(const CollectionBase &param,
                                CollectionMetaPtr *collection) = 0;

  //! Update collection and columns, increase revision and copy a new collection
  virtual int update_collection(const CollectionBase &param,
                                CollectionMetaPtr *collection) = 0;

  //! Enable collection
  virtual int enable_collection(const std::string &collection,
                                uint32_t revision, bool enable) = 0;

  //! Update the status of current used collection
  virtual int update_status(const std::string &collection_name,
                            CollectionStatus status) = 0;

  //! Suspend reading requests of collection
  virtual int suspend_collection_read(const std::string &collection_name) = 0;

  //! Resume reading requests of collection
  virtual int resume_collection_read(const std::string &collection_name) = 0;

  //! Suspend writing requests of collection
  virtual int suspend_collection_write(const std::string &collection_name) = 0;

  //! Resume writing requests of collection
  virtual int resume_collection_write(const std::string &collection_name) = 0;

  //! Drop collection
  virtual int drop_collection(const std::string &name) = 0;

  //! Retrieve latest version of collection
  virtual CollectionMetaPtr get_current_collection(
      const std::string &name) const = 0;

  //! Retrieve latest version of collections
  virtual int get_latest_collections(
      CollectionMetaPtrList *collections) const = 0;

  //! Retrieve all of collections
  virtual int get_collections(CollectionMetaPtrList *collections) const = 0;

  //! Retrieve collections with specific repository
  virtual int get_collections_by_repo(
      const std::string &repository,
      CollectionMetaPtrList *collections) const = 0;

  //! Retrieve collections with specific collection name
  virtual int get_collections(const std::string &collection,
                              CollectionMetaPtrList *collections) const = 0;

  //! Retrieve collection
  virtual CollectionMetaPtr get_collection(const std::string &collection,
                                           uint64_t revision) const = 0;

  //! Check collection exists
  virtual bool exist_collection(const std::string &collection) const = 0;

};  // end of MetaService


}  // namespace meta
}  // namespace be
}  // namespace proxima
