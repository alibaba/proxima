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

#include "admin_agent.h"
#include <utility>
#include "admin/admin_proto_converter.h"
#include "common/logger.h"

namespace proxima {
namespace be {
namespace admin {


/**
 * Proxima BE Admin module
 */
class AdminAgentImpl : public AdminAgent {
 public:
  //! Constructor
  explicit AdminAgentImpl(meta::MetaAgentPtr meta, agent::IndexAgentPtr index,
                          query::QueryAgentPtr query)
      : meta_agent_(std::move(meta)),
        index_agent_(std::move(index)),
        query_agent_(std::move(query)) {}
  //! Destructor
  ~AdminAgentImpl() override = default;

 public:
  //! Init Meta Agent
  int init() override {
    LOG_INFO("AdminAgent initialize complete.");
    return 0;
  }

  //! Clean up object
  int cleanup() override {
    LOG_INFO("AdminAgent cleanup complete.");
    return 0;
  }

  //! Start background service
  int start() override {
    LOG_INFO("AdminAgent start complete.");
    return 0;
  }

  //! Stop background service
  int stop() override {
    LOG_INFO("AdminAgent stopped.");
    return 0;
  }

  //! Create collection
  int create_collection(const proto::CollectionConfig &request) override {
    meta::CollectionBase param;
    int code = AdminProtoConverter::PBToCollectionBase(request, &param);
    if (code != 0) {
      LOG_ERROR("Deserialize collection meta from pb failed.");
      return code;
    }

    meta::CollectionMetaPtr collection;
    code = meta_agent_->create_collection(param, &collection);
    if (code != 0) {
      LOG_ERROR("MetaAgent create collection failed. code[%d] what[%s]", code,
                ErrorCode::What(code));
      return code;
    }

    code = index_agent_->create_collection(collection->name());
    if (code == 0) {
      LOG_INFO("Create collection success. collection_config[%s]",
               request.ShortDebugString().c_str());
      meta_agent_->enable_collection(collection->name(),
                                     collection->revision());
    } else {
      meta_agent_->delete_collection(collection->name());
      LOG_ERROR("IndexAgent create collection failed. code[%d] what[%s]", code,
                ErrorCode::What(code));
    }
    return code;
  }

  //! Describe collection
  int describe_collection(
      const std::string &collection_name,
      proto::DescribeCollectionResponse *collection_info) override {
    auto collection = meta_agent_->get_collection(collection_name);
    if (!collection) {
      LOG_ERROR("Failed to describe collection. collection[%s]",
                collection_name.c_str());
      return PROXIMA_BE_ERROR_CODE(InexistentCollection);
    }
    return fill_collection_info(collection,
                                collection_info->mutable_collection());
  }

  //! Delete collection
  int drop_collection(const std::string &collection_name) override {
    int code = index_agent_->drop_collection(collection_name);
    if (code != 0) {
      LOG_ERROR("IndexAgent delete collection failed. code[%d] what[%s]", code,
                ErrorCode::What(code));
      return code;
    }

    // Continue to remove collection in meta
    code = meta_agent_->delete_collection(collection_name);
    if (code != 0) {
      LOG_ERROR("MetaAgent delete collection failed. code[%d] what[%s]", code,
                ErrorCode::What(code));
      return code;
    }
    return code;
  }

  //! Retrieve collections
  int list_collections(const proto::ListCondition &condition,
                       proto::ListCollectionsResponse *response) override {
    meta::CollectionMetaPtrList collections;
    int code = meta_agent_->list_collections(&collections);
    if (code != 0) {
      LOG_ERROR("Failed to list collections. code[%d] what[%s].", code,
                ErrorCode::What(code));
      return code;
    }
    for (auto &collection : collections) {
      if (condition.repository_name().empty() ||
          (condition.repository_name() == collection->repository_name())) {
        auto *pb_collection = response->add_collections();
        int ret = fill_collection_info(collection, pb_collection);
        if (ret != 0) {
          return ret;
        }
      }
    }

    return 0;
  }

  int stats_collection(const std::string &collection_name,
                       proto::StatsCollectionResponse *stats) override {
    index::CollectionStats collection_stats;
    int code =
        index_agent_->get_collection_stats(collection_name, &collection_stats);
    if (code != 0) {
      LOG_ERROR(
          "Failed to get collection stats, collection[%s] code[%d], "
          "what[%s].",
          collection_name.c_str(), code, ErrorCode::What(code));
      return code;
    }
    AdminProtoConverter::CollectionStatsToPB(collection_stats,
                                             stats->mutable_collection_stats());
    return 0;
  }

  int reload_meta() override {
    return meta_agent_->reload();
  }

  int start_query_service() override {
    return query_agent_->start();
  }

  int stop_query_service() override {
    return query_agent_->stop();
  }

  int get_query_service_status() override {
    return query_agent_->is_running();
  }

 private:
  //! only called in proxy write scenario
  int fill_lsn_context_and_magic_number(
      const std::string &collection_name,
      proto::CollectionInfo *collection_info) {
    collection_info->set_magic_number(index_agent_->get_magic_number());
    std::string context;
    uint64_t lsn;
    int ret = index_agent_->get_latest_lsn(collection_name, &lsn, &context);
    if (ret != 0) {
      LOG_WARN("Get latest lsn failed. collection_name[%s] ret[%d]",
               collection_name.c_str(), ret);
      return ret;
    }
    auto *lsn_context = collection_info->mutable_latest_lsn_context();
    lsn_context->set_lsn(lsn);
    lsn_context->set_context(std::move(context));
    return 0;
  }

  //! fill protobuf collection info
  int fill_collection_info(const meta::CollectionMetaPtr &meta,
                           proto::CollectionInfo *collection) {
    int ret = 0;
    AdminProtoConverter::CollectionMetaToPB(*meta, collection);
    if (meta->repository()) {
      ret = fill_lsn_context_and_magic_number(meta->name(), collection);
    }
    return ret;
  }

 private:
  //! Meta agent
  meta::MetaAgentPtr meta_agent_{nullptr};

  //! Index Agent
  agent::IndexAgentPtr index_agent_{nullptr};

  //! Query agent
  query::QueryAgentPtr query_agent_{nullptr};
};

AdminAgentPtr AdminAgent::Create(const meta::MetaAgentPtr &meta,
                                 const agent::IndexAgentPtr &agent,
                                 const query::QueryAgentPtr &query) {
  return std::make_shared<AdminAgentImpl>(meta, agent, query);
}

}  // namespace admin
}  // namespace be
}  // namespace proxima
