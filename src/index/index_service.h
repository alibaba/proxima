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

 *   \author   Haichao.chc
 *   \date     Oct 2020
 *   \brief    IndexService mainly provides collection management and writing
 *             records
 */

#pragma once

#include <ailego/parallel/thread_pool.h>
#include "common/config.h"
#include "common/interface/service.h"
#include "common/wait_notifier.h"
#include "collection.h"
#include "concurrent_hash_map.h"
#include "typedef.h"

namespace proxima {
namespace be {
namespace index {

class IndexService;
using IndexServicePtr = std::shared_ptr<IndexService>;

/*
 * IndexService is mainly responsible for collection management
 * and record indexing.
 * Collection operations includes create/drop/update .etc
 * Record operations includes insert/delete/query/update .etc
 * And it regularly do snapshot from collection to persist storage.
 */
class IndexService : public Service {
 public:
  PROXIMA_DISALLOW_COPY_AND_ASSIGN(IndexService);

  //! Constructor
  IndexService() = default;

  //! Destructor
  virtual ~IndexService();

 public:
  //! Create collection with schema
  virtual int create_collection(const std::string &collection_name,
                                const meta::CollectionMetaPtr &schema);

  //! Drop collection by name
  virtual int drop_collection(const std::string &collection_name);

  //! Update collection schema
  virtual int update_collection(const std::string &collection_name,
                                const meta::CollectionMetaPtr &new_schema);

  //! Check if collection exist
  virtual bool has_collection(const std::string &collection_name);

  //! Load collections from storage
  virtual int load_collections(
      const std::vector<std::string> &collection_names,
      const std::vector<meta::CollectionMetaPtr> &schemas);

  //! List all collection names
  virtual int list_collections(std::vector<std::string> *collection_names);

  //! Get collection statistics
  virtual int get_collection_stats(const std::string &collection_name,
                                   CollectionStats *collection_stats);

  //! List all collection segments
  virtual int list_segments(const std::string &collection_name,
                            std::vector<SegmentPtr> *segments);

  //! Get collection latest lsn and context
  virtual int get_latest_lsn(const std::string &collection_name, uint64_t *lsn,
                             std::string *lsn_context);

  //! Write records to some collection
  virtual int write_records(const std::string &collection_name,
                            const CollectionDatasetPtr &records);

 protected:
  //! Initialize inner members
  int init_impl() override;

  //! Cleanup and destroy objects
  int cleanup_impl() override;

  //! Start worker thread
  int start_impl() override;

  //! Stop worker thread
  int stop_impl() override;

 private:
  bool load_config();

  void do_routine_flush();

  void do_routine_optimize();

 private:
  ThreadPoolPtr thread_pool_{};
  ConcurrentHashMap<std::string, CollectionPtr> collections_{};

  std::string index_directory_{};
  uint32_t thread_count_{0U};
  uint32_t flush_internal_{0U};
  uint32_t optimize_internal_{0U};
  uint32_t concurrency_{0U};
  bool use_mmap_read_{false};

  WaitNotifier flush_notifier_{};
  std::atomic<bool> flush_flag_{false};

  WaitNotifier optimize_notifier_{};
  std::atomic<bool> optimize_flag_{false};
};


}  // end namespace index
}  // namespace be
}  // end namespace proxima
