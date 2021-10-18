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

 *   \author   Dianzhang.Chen
 *   \date     Oct 2020
 *   \brief    Interface of collection manager
 */

#pragma once

#include <unordered_map>
#include <brpc/channel.h>
#include "collection_creator.h"
#include "mysql_collection.h"

namespace proxima {
namespace be {
namespace repository {

class CollectionManager;
using CollectionManagerPtr = std::shared_ptr<CollectionManager>;

/*! Collection Manager
 */
// class CollectionManager {
class CollectionManager {
 public:
  //! Constructor
  explicit CollectionManager(CollectionCreatorPtr collection_creator)
      : collection_creator_(std::move(collection_creator)) {}

  //! Destructor
  ~CollectionManager() = default;

 public:
  //! Init Collection Manager
  int init();

  //! Start repository
  void start();

  //! Stop repository
  int stop();

  //！Cleanup repository
  int cleanup();

 private:
  //! Create Collection
  int create_collection(const CollectionInfo &info);

  //! Update Collection
  int update_collection(const std::string &uuid);

  //! Drop Collection
  int drop_collection(const std::string &uuid);

  //！ Create Collections
  void create_collections(const std::vector<CollectionInfo> &infos);

  //! Update Collections
  void update_collections(const std::vector<std::string> &uuids);

  //! Drop Collections
  void drop_collections(const std::vector<std::string> &uuids);

  //! Classify Collections
  void classify_collections(
      const std::vector<CollectionInfo> &infos,
      std::vector<CollectionInfo> *new_collection_infos,
      std::vector<std::string> *old_collection_uuids,
      std::vector<std::string> *expired_collection_uuids) const;

  //! Load configs
  void load_config();

  //! Clean invalid collections
  void clean_invalid_collections();

  //! Stop all collections
  void stop_collections();

  //! Get all collections from index agent
  int get_all_collections(std::vector<CollectionInfo> *infos);

  //! Get sleep time
  uint64_t get_sleep_time() const;

  //! Check if collection is old
  bool is_old_collection(const std::string &uuid,
                         uint32_t new_schema_revision) const;

  //! Filter collections by collection status
  void filter_collections(const std::vector<CollectionInfo> &infos,
                          std::vector<CollectionInfo> *valid_infos) const;

  //! Check server version
  int check_server_version();

 private:
  //! Disable copy constructor
  CollectionManager(const CollectionManager &) = delete;

  //! Disable assignment operator
  CollectionManager &operator=(const CollectionManager &) = delete;

 private:
  //! Members of Collections
  std::unordered_map<std::string, CollectionPtr> collections_{};
  std::unordered_map<std::string, std::string> uuid_name_map_{};

  std::atomic<bool> stop_{false};
  //! Mutex
  std::mutex mutex_{};

  //! Members of brpc
  brpc::Channel channel_{};
  brpc::ChannelOptions options_{};
  std::string index_server_uri_{};
  int max_retry_{0};
  int timeout_ms_{0};
  std::string load_balance_{};
  std::string index_agent_addr_{};
  std::string repository_name_{};

  CollectionCreatorPtr collection_creator_{};
};

}  // end namespace repository
}  // namespace be
}  // end namespace proxima
