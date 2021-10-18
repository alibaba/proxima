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
 *   \brief    Implementation of collection manager
 */

#include "collection_manager.h"
#include <chrono>
#include <random>
#include <thread>
#include <ailego/utility/string_helper.h>
#include "binlog/mysql_handler.h"
#include "repository/repository_common/config.h"
#include "repository/repository_common/error_code.h"
#include "repository/repository_common/version.h"

namespace proxima {
namespace be {
namespace repository {

//! Create collection
int CollectionManager::create_collection(const CollectionInfo &info) {
  const std::string &collection_name = info.config().collection_name();
  const std::string &uuid = info.uuid();

  if (collections_.find(uuid) != collections_.end()) {
    LOG_ERROR("Create collection failed. uuid[%s]", uuid.c_str());
    return ErrorCode_DuplicateCollection;
  }

  LOG_INFO("Start to create a new collection. name[%s], uuid[%s]",
           collection_name.c_str(), uuid.c_str());

  CollectionPtr current_collection = collection_creator_->create(info);
  if (!current_collection) {
    LOG_ERROR("Create Mysql collection object failed");
    return ErrorCode_RuntimeError;
  }

  int ret = current_collection->init();
  if (ret != 0) {
    LOG_ERROR("Init collection failed. name[%s]", collection_name.c_str());
    return ret;
  }

  collections_[uuid] = current_collection;
  uuid_name_map_[uuid] = collection_name;
  current_collection->run();
  LOG_INFO("Create a new collection successfully. name[%s], uuid[%s]",
           collection_name.c_str(), uuid.c_str());
  return 0;
}

//! Update collection
int CollectionManager::update_collection(const std::string &uuid) {
  auto it = collections_.find(uuid);
  if (it == collections_.end()) {
    LOG_ERROR("Can't update not exist collection. uuid[%s]", uuid.c_str());
    return ErrorCode_CollectionNotExist;
  }
  it->second->update();
  return 0;
}

//! Drop collection
int CollectionManager::drop_collection(const std::string &uuid) {
  auto it = collections_.find(uuid);
  if (it == collections_.end()) {
    LOG_ERROR("Can't drop not exist collection. uuid[%s]", uuid.c_str());
    return ErrorCode_CollectionNotExist;
  }
  it->second->drop();
  return 0;
}

void CollectionManager::clean_invalid_collections() {
  std::vector<std::string> finished_collections;
  for (auto &collection : collections_) {
    if (collection.second->finished()) {
      collection.second->stop();
      finished_collections.emplace_back(collection.first);
    }
  }
  for (auto &finished_collection : finished_collections) {
    LOG_INFO("Clean invalid collection. uuid[%s], name[%s]",
             finished_collection.c_str(),
             uuid_name_map_[finished_collection].c_str());
    collections_.erase(finished_collection);
    uuid_name_map_.erase(finished_collection);
  }
}

void CollectionManager::stop_collections() {
  for (auto &collection : collections_) {
    LOG_INFO("Stopping Collection. uuid[%s], name[%s]",
             collection.first.c_str(),
             uuid_name_map_[collection.first].c_str());
    collection.second->stop();
  }
}

void CollectionManager::load_config() {
  index_server_uri_ = repository::Config::Instance().get_index_agent_uri();
  max_retry_ = repository::Config::Instance().get_max_retry();
  timeout_ms_ = repository::Config::Instance().get_timeout_ms();
  repository_name_ = repository::Config::Instance().get_repository_name();
  load_balance_ = repository::Config::Instance().get_load_balance();
}

int CollectionManager::cleanup() {
  std::unique_lock<std::mutex> ul(mutex_);
  stop_collections();
  collections_.clear();
  return 0;
}

int CollectionManager::stop() {
  LOG_INFO("Stopping Collection Manager.");
  stop_.store(true);
  cleanup();
  return 0;
}

int CollectionManager::get_all_collections(
    std::vector<CollectionInfo> *collections) {
  proto::ListCondition request;
  proto::ListCollectionsResponse response;
  proto::ProximaService_Stub stub(&channel_);
  brpc::Controller cntl;

  request.set_repository_name(repository_name_);

  stub.list_collections(&cntl, &request, &response, NULL);
  if (cntl.Failed()) {
    LOG_ERROR("list_collections rpc failed. reason[%s]",
              cntl.ErrorText().c_str());
    return ErrorCode_RPCFailed;
  }
  // Check status
  if (response.status().code() != 0) {
    LOG_ERROR("Failed to get all collections. reason[%s]",
              response.status().reason().c_str());
    return response.status().code();
  }

  //   auto &entity = response.entity();
  collections->insert(collections->end(), response.collections().begin(),
                      response.collections().end());
  return 0;
}

void CollectionManager::create_collections(
    const std::vector<CollectionInfo> &infos) {
  for (auto &info : infos) {
    int ret = create_collection(info);
    if (ret != 0) {
      // one collection error, just print error message and continue
      LOG_ERROR(
          "Failed to create collection: name[%s], uuid[%s], code[%d], msg[%s]",
          info.config().collection_name().c_str(), info.uuid().c_str(), ret,
          ErrorCode::What(ret));
    }
  }
}

void CollectionManager::update_collections(
    const std::vector<std::string> &uuids) {
  for (auto &uuid : uuids) {
    int ret = update_collection(uuid);
    if (ret != 0) {
      // one collection error, just print error message and continue
      LOG_ERROR("Failed to update collection: uuid[%s], code[%d], msg[%s]",
                uuid.c_str(), ret, ErrorCode::What(ret));
    }
  }
}

void CollectionManager::drop_collections(
    const std::vector<std::string> &uuids) {
  for (auto &uuid : uuids) {
    int ret = drop_collection(uuid);
    if (ret != 0) {
      // one collection error, just print error message and continue
      LOG_ERROR("Failed to drop collection: uuid[%s], code[%d], msg[%s]",
                uuid.c_str(), ret, ErrorCode::What(ret));
    }
  }
}

bool CollectionManager::is_old_collection(const std::string &uuid,
                                          uint32_t new_schema_revision) const {
  auto it = collections_.find(uuid);
  if (it == collections_.end()) {
    return false;
  }
  CollectionPtr current_collection = it->second;
  uint32_t schema_revision = current_collection->schema_revision();
  return schema_revision < new_schema_revision;
}

void CollectionManager::classify_collections(
    const std::vector<CollectionInfo> &infos,
    std::vector<CollectionInfo> *new_collection_infos,
    std::vector<std::string> *old_collection_uuids,
    std::vector<std::string> *expired_collection_uuids) const {
  for (auto &info : infos) {
    const std::string &uuid = info.uuid();
    // todo<cdz>: Read schema revision from collection info when support update.
    // current just send 0;
    // uint32_t schema_revision = collection.schema_revision();
    uint32_t schema_revision = 0;
    if (collections_.find(uuid) == collections_.end()) {
      new_collection_infos->emplace_back(info);
    } else if (is_old_collection(uuid, schema_revision)) {
      old_collection_uuids->emplace_back(uuid);
    }
  }
  for (auto &exist_collection : collections_) {
    const std::string &uuid = exist_collection.first;
    if (std::none_of(
            infos.begin(), infos.end(),
            [&](const CollectionInfo &info) { return info.uuid() == uuid; })) {
      expired_collection_uuids->emplace_back(uuid);
    }
  }
}

uint64_t CollectionManager::get_sleep_time() const {
  std::mt19937 gen((std::random_device())());
  return (std::uniform_int_distribution<uint64_t>(0, 1000))(gen);
}

void CollectionManager::filter_collections(
    const std::vector<CollectionInfo> &collections,
    std::vector<CollectionInfo> *valid_collections) const {
  for (auto &collection : collections) {
    if (collection.status() == proto::CollectionInfo::CS_SERVING) {
      valid_collections->push_back(collection);
    }
  }
}

//! Deal collections periodicity
void CollectionManager::start() {
  LOG_INFO("Start Collection Manager.");
  std::vector<CollectionInfo> collection_infos;
  std::vector<CollectionInfo> valid_collection_infos;
  std::vector<CollectionInfo> new_collection_infos;
  std::vector<std::string> old_collection_uuids;
  std::vector<std::string> expired_collection_uuids;
  while (true) {
    collection_infos.clear();
    valid_collection_infos.clear();
    new_collection_infos.clear();
    old_collection_uuids.clear();
    expired_collection_uuids.clear();
    int ret = get_all_collections(&collection_infos);
    if (ret != 0) {
      // Retry
      uint64_t sleep_time = get_sleep_time();
      std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time));
      if (stop_.load()) {
        return;
      }
      continue;
    }
    filter_collections(collection_infos, &valid_collection_infos);
    classify_collections(valid_collection_infos, &new_collection_infos,
                         &old_collection_uuids, &expired_collection_uuids);
    std::unique_lock<std::mutex> ul(mutex_);
    if (stop_.load()) {
      return;
    }
    create_collections(new_collection_infos);
    update_collections(old_collection_uuids);
    drop_collections(expired_collection_uuids);
    clean_invalid_collections();
    ul.unlock();
    std::this_thread::sleep_for(std::chrono::seconds(UPDATE_INTERVAL));
  }
}

//! Init Collection Manager
int CollectionManager::init() {
  // Load configurations
  load_config();
  options_ = brpc::ChannelOptions();
  options_.max_retry = max_retry_;
  options_.timeout_ms = timeout_ms_;

  repository_name_ = Config::Instance().get_repository_name();
  if (repository_name_.empty()) {
    LOG_ERROR("Repository name is empty.");
    return ErrorCode_ConfigError;
  }

  int ret = channel_.Init(index_server_uri_.c_str(), load_balance_.c_str(),
                          &options_);
  if (ret != 0) {
    LOG_ERROR("Failed to initialize channel. uri[%s]",
              index_server_uri_.c_str());
    return ErrorCode_InitChannel;
  }

  // Check Proxima BE version
  ret = check_server_version();
  if (ret != 0) {
    LOG_ERROR("Check Proxima BE server version failed.");
    return ret;
  }

  return 0;
}

int CollectionManager::check_server_version() {
  proto::ProximaService_Stub stub(&channel_);
  brpc::Controller cntl;
  proto::GetVersionRequest request;
  proto::GetVersionResponse response;

  stub.get_version(&cntl, &request, &response, nullptr);
  if (cntl.Failed()) {
    LOG_ERROR("Get Proxima BE version rpc failed. reason[%s]",
              cntl.ErrorText().c_str());
    return ErrorCode_RPCFailed;
  }

  if (response.status().code() != 0) {
    LOG_ERROR("Get Proxima BE version failed. reason[%s]",
              response.status().reason().c_str());
    return response.status().code();
  }

  std::string server_version = response.version();
  std::string client_version = Version::String();
  LOG_INFO("server_version: %s", server_version.c_str());
  LOG_INFO("mysql_repository_version: %s", Version::String());
  if (server_version == client_version) {
    return 0;
  }

  // TODO @Dianzhang.Chen
  // Temporarily we just use first two seq number of  version string to compare
  // For exp: version[0.1.2] match version[0.1.3] with "0.1"
  std::vector<std::string> server_sub_seqs;
  ailego::StringHelper::Split(server_version, '.', &server_sub_seqs);
  std::vector<std::string> client_sub_seqs;
  ailego::StringHelper::Split(client_version, '.', &client_sub_seqs);

  int compare_count = 2;
  for (int i = 0; i < compare_count; i++) {
    if (client_sub_seqs[i] != server_sub_seqs[i]) {
      LOG_ERROR("Mysql repository version: %s not match server version: %s",
                Version::String(), server_version.c_str());
      return ErrorCode_MismatchedVersion;
    }
  }

  return 0;
}

}  // end namespace repository
}  // namespace be
}  // end namespace proxima