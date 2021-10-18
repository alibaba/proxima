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
#include "index/index_service.h"

using namespace proxima::be;
using namespace proxima::be::index;
using namespace ::testing;  // import ::testing::*

//! Mock IndexService
class MockIndexService : public IndexService {
 public:
  ~MockIndexService() = default;

 public:
  //! Create collection with schema
  MOCK_METHOD(int, create_collection,
              (const std::string &collection_name,
               const meta::CollectionMetaPtr &schema),
              (override));

  //! Drop collection by name
  MOCK_METHOD(int, drop_collection, (const std::string &collection_name),
              (override));

  //! Update collection schema
  MOCK_METHOD(int, update_collection,
              (const std::string &collection_name,
               const meta::CollectionMetaPtr &new_schema),
              (override));

  //! Check if collection exist
  MOCK_METHOD(bool, has_collection, (const std::string &collection_name),
              (override));

  //! Load collections from storage
  MOCK_METHOD(int, load_collections,
              (const std::vector<std::string> &collection_names,
               const std::vector<meta::CollectionMetaPtr> &schemas),
              (override));

  //! List all collection names
  MOCK_METHOD(int, list_collections,
              (std::vector<std::string> * collection_names), (override));

  //! List all collection segments
  MOCK_METHOD(int, list_segments,
              (const std::string &collection_name,
               std::vector<SegmentPtr> *segments),
              (override));

  //! Get collection latest lsn
  MOCK_METHOD(int, get_latest_lsn,
              (const std::string &collection_name, uint64_t *lsn,
               std::string *lsn_context),
              (override));

  //! Write records to some collection
  MOCK_METHOD(int, write_records,
              (const std::string &collection_name,
               const CollectionDatasetPtr &records),
              (override));

 protected:
  //! Initialize inner members
  MOCK_METHOD(int, init_impl, (), (override));

  //! Cleanup and destroy objects
  MOCK_METHOD(int, cleanup_impl, (), (override));

  //! Start worker thread
  MOCK_METHOD(int, start_impl, (), (override));

  //! Stop worker thread
  MOCK_METHOD(int, stop_impl, (), (override));
};

using MockIndexServicePtr = std::shared_ptr<MockIndexService>;
