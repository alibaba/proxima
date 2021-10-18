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

 *   \author   Hongqing.hu
 *   \date     Oct 2020
 *   \brief    Index agent interface definition for proxima search engine
 */

#pragma once

#include <ailego/algorithm/rate_limiter.h>
#include <ailego/parallel/thread_queue.h>
#include "index/index_service.h"
#include "meta/meta_service.h"
#include "collection_counter.h"
#include "column_order.h"
#include "write_request.h"

namespace proxima {
namespace be {
namespace agent {

class IndexAgent;
using IndexAgentPtr = std::shared_ptr<IndexAgent>;

/*! Index Agent
 */
class IndexAgent {
 public:
  //! Create index agent
  static IndexAgentPtr Create(meta::MetaServicePtr meta_service);

  //! Constructor
  IndexAgent(meta::MetaServicePtr meta_service);

  //! Destructor
  ~IndexAgent();

  //! Create collection with schema
  int create_collection(const std::string &collection_name);

  //! Update collection schema
  int update_collection(const std::string &collection_name, uint32_t revision);

  //! Drop collection by name
  int drop_collection(const std::string &collection_name);

  //! Get collection statstics
  int get_collection_stats(const std::string &collection_name,
                           index::CollectionStats *collection_stats) const;

  //! Write records
  int write(const WriteRequest &request);

  // Get latest lsn
  int get_latest_lsn(const std::string &collection_name, uint64_t *lsn,
                     std::string *lsn_context);

  //! Get Index Service
  const index::IndexServicePtr &get_service() const {
    return index_service_;
  }

  //! Get magic number
  uint64_t get_magic_number() const {
    return agent_timestamp_;
  }

  //! Get collection column order
  ColumnOrderPtr get_column_order(const std::string &collection_name) const {
    return column_order_map_->get_column_order(collection_name);
  }

  //! Get collection meta
  meta::CollectionMetaPtr get_collection_meta(
      const std::string &collection_name) const {
    return meta_service_->get_current_collection(collection_name);
  }

 public:
  //! Init index agent
  int init();

  //! Cleanup index agent
  int cleanup();

  //! Start index agent
  int start();

  //! Stop index agent
  int stop();

 private:
  //! Load index service
  int load_index_service();

  //! Is Index Agent Suspended
  bool is_collection_suspend(const std::string &collection);

  //! Proxy write
  int proxy_write(const WriteRequest &request, CollectionCounter *counter);

  //! Direct write
  int direct_write(const WriteRequest &request, CollectionCounter *counter);

  //! Write the CollectionDataset to index service
  void write_dataset(const std::string &collection_name,
                     const index::CollectionDatasetPtr &records,
                     CollectionCounter *counter);

 private:
  IndexAgent(const IndexAgent &) = delete;
  IndexAgent &operator=(const IndexAgent &) = delete;

 private:
  //! Agent start timestamp
  uint64_t agent_timestamp_{0};
  //! Request acquire timeout ms
  int32_t acquire_timeout_{0};
  //! Meta service pointer
  meta::MetaServicePtr meta_service_{};
  //! Index service pointer
  index::IndexServicePtr index_service_{};
  //! Rate limiter ptr
  ailego::RateLimiter::Pointer rate_limiter_{};
  //! Work thread queue
  std::shared_ptr<ailego::ThreadQueue> thread_pool_{};
  //! Collection counter map
  CollectionCounterMapPtr counter_map_{};
  //! Collection columns order map
  ColumnOrderMapPtr column_order_map_{};
};

}  // end namespace agent
}  // namespace be
}  // end namespace proxima
