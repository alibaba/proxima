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
 *   \date     Dec 2020
 *   \brief
 */

#pragma once

#include "meta_service.h"

namespace proxima {
namespace be {
namespace meta {

//! Predefined class
class MetaAgent;
//! Alias Pointer for MetaAgent
using MetaAgentPtr = std::shared_ptr<MetaAgent>;


/*!
 * MetaAgent for meta management
 */
class MetaAgent {
 public:
  //! Create one MetaAgent instance
  static MetaAgentPtr Create(const std::string &uri);

  //! Create MetaAgent from MetaService instance
  static MetaAgentPtr Create(const MetaServicePtr &);

 public:
  //! Destructor
  virtual ~MetaAgent() = default;

 public:
  //! Get Meta Service Instance
  virtual MetaServicePtr get_service() const = 0;

 public:
  //! Init Meta Agent
  virtual int init() = 0;

  //! Clean up object
  virtual int cleanup() = 0;

  //! Start background service
  virtual int start() = 0;

  //! Stop background service
  virtual int stop() = 0;

 public:
  //! Reload meta service
  virtual int reload() = 0;

  //! Create collection, 0 for success, otherwise failed, write the details
  // of collection to @param meta if meta not nullptr.
  //! @param, input parameter
  //! @meta, output parameter
  virtual int create_collection(const CollectionBase &param,
                                CollectionMetaPtr *meta) = 0;

  //! Update collection, 0 for success, otherwise failed, write the details
  // of collection to @param meta if meta not nullptr.
  //! @param, input parameter
  //! @meta, output parameter
  virtual int update_collection(const CollectionBase &param,
                                CollectionMetaPtr *meta) = 0;

  //! Update the status of current used collection
  virtual int update_status(const std::string &collection_name,
                            CollectionStatus status) = 0;

  //! Enable collection
  virtual int enable_collection(const std::string &collection,
                                uint32_t revision) = 0;

  //! Suspend reading requests of collection
  virtual int suspend_collection_read(const std::string &collection_name) = 0;

  //! Resume reading requests of collection
  virtual int resume_collection_read(const std::string &collection_name) = 0;

  //! Suspend writing requests of collection
  virtual int suspend_collection_write(const std::string &collection_name) = 0;

  //! Resume writing requests of collection
  virtual int resume_collection_write(const std::string &collection_name) = 0;

  //! Delete collection
  virtual int delete_collection(const std::string &collection) = 0;

  //! Retrieve collections
  virtual int list_collections(CollectionMetaPtrList *collections) const = 0;

  //! Retrieve collection history by name
  virtual int get_collection_history(
      const std::string &name, CollectionMetaPtrList *collections) const = 0;

  //! Retrieve collection by name
  virtual CollectionMetaPtr get_collection(const std::string &name) const = 0;

  //! Check collection exists
  virtual bool exist_collection(const std::string &name) const = 0;
};


}  // namespace meta
}  // namespace be
}  // namespace proxima
