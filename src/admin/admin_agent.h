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

#include "agent/index_agent.h"
#include "meta/meta_agent.h"
#include "query/query_agent.h"

namespace proxima {
namespace be {
namespace admin {

class AdminAgent;
//! Alias Pointer for AdminAgent
using AdminAgentPtr = std::shared_ptr<AdminAgent>;

/**
 * Proxima BE Admin module
 */
class AdminAgent {
 public:
  //! Create AdminAgent Object
  static AdminAgentPtr Create(const meta::MetaAgentPtr &meta,
                              const agent::IndexAgentPtr &index,
                              const query::QueryAgentPtr &query);

 public:
  //! Destructor
  virtual ~AdminAgent() = default;

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
  //! Create collection
  virtual int create_collection(const proto::CollectionConfig &request) = 0;

  //! Describe collection
  virtual int describe_collection(
      const std::string &collection_name,
      proto::DescribeCollectionResponse *collection_info) = 0;

  //! Drop collection
  virtual int drop_collection(const std::string &collection_name) = 0;

  //! Retrieve collections
  virtual int list_collections(const proto::ListCondition &condition,
                               proto::ListCollectionsResponse *response) = 0;

  //! Retrieve collection stats
  virtual int stats_collection(const std::string &collection_name,
                               proto::StatsCollectionResponse *stats) = 0;

  //! Reload meta from meta store
  virtual int reload_meta() = 0;

  //! Start query service
  virtual int start_query_service() = 0;

  //! Stop query service
  virtual int stop_query_service() = 0;

  //! Get query service status
  virtual int get_query_service_status() = 0;
};

}  // namespace admin
}  // namespace be
}  // namespace proxima
