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

#include <gmock/gmock.h>
#include "meta/meta_service.h"

using namespace proxima::be;
using namespace proxima::be::meta;
using namespace ::testing;  // import ::testing::*

//! Predefined class
class MockMetaService;
//! Alias for MockMetaService
using MockMetaServicePtr = std::shared_ptr<MockMetaService>;

//! Mock MetaService Interface
class MockMetaService : public MetaService {
 public:
  // Destructor
  ~MockMetaService() override = default;

 public:  // Interfaces in Service
  MOCK_METHOD(int, init_impl, (), (override));
  MOCK_METHOD(int, cleanup_impl, (), (override));
  MOCK_METHOD(int, start_impl, (), (override));
  MOCK_METHOD(int, stop_impl, (), (override));

 public:  // Interfaces in MetaService
  //! Reload meta service
  MOCK_METHOD(int, reload, (), (override));

  //! Create collection and columns
  MOCK_METHOD(int, create_collection,
              (const CollectionBase &param, CollectionMetaPtr *collection),
              (override));

  //! Update collection and columns, increase revision and copy a new collection
  MOCK_METHOD(int, update_collection,
              (const CollectionBase &param, CollectionMetaPtr *collection),
              (override));

  //! Enable collection
  MOCK_METHOD(int, enable_collection,
              (const std::string &collection, uint32_t revision, bool enable),
              (override));

  //! Update the status of current used collection
  MOCK_METHOD(int, update_status,
              (const std::string &collection_name, CollectionStatus status),
              (override));

  //! Suspend reading requests of collection
  MOCK_METHOD(int, suspend_collection_read,
              (const std::string &collection_name), (override));

  //! Resume reading requests of collection
  MOCK_METHOD(int, resume_collection_read, (const std::string &collection_name),
              (override));

  //! Suspend writing requests of collection
  MOCK_METHOD(int, suspend_collection_write,
              (const std::string &collection_name), (override));

  //! Resume writing requests of collection
  MOCK_METHOD(int, resume_collection_write,
              (const std::string &collection_name), (override));

  //! Drop collection
  MOCK_METHOD(int, drop_collection, (const std::string &name), (override));

  //! Retrieve latest version of collection
  MOCK_METHOD(CollectionMetaPtr, get_current_collection,
              (const std::string &name), (const, override));

  //! Retrieve latest version of collections
  MOCK_METHOD(int, get_latest_collections,
              (CollectionMetaPtrList * collections), (const, override));

  //! Retrieve all of collections
  MOCK_METHOD(int, get_collections, (CollectionMetaPtrList * collections),
              (const, override));

  //! Retrieve collections with specific repository
  MOCK_METHOD(int, get_collections_by_repo,
              (const std::string &repository,
               CollectionMetaPtrList *collections),
              (const, override));

  //! Retrieve collections with specific collection name
  MOCK_METHOD(int, get_collections,
              (const std::string &collection,
               CollectionMetaPtrList *collections),
              (const, override));

  //! Retrieve collection
  MOCK_METHOD(CollectionMetaPtr, get_collection,
              (const std::string &collection, uint64_t revision),
              (const, override));

  //! Check collection exists
  MOCK_METHOD(bool, exist_collection, (const std::string &collection),
              (const, override));


};  // end of MetaService
