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
 *   \brief    Collection provides add/delete/modify operations of single
 *             collection
 */

#pragma once

#include "common/macro_define.h"
#include "meta/meta.h"
#include "segment/memory_segment.h"
#include "segment/persist_segment_manager.h"
#include "collection_dataset.h"
#include "collection_stats.h"
#include "delete_store.h"
#include "id_map.h"
#include "lsn_store.h"
#include "version_manager.h"

namespace proxima {
namespace be {
namespace index {

class Collection;
using CollectionPtr = std::shared_ptr<Collection>;

/*
 * Collection is responsible for data storage, building index,
 * and searching index. It use traditional LSM structure for
 * data processing and storing. Data will insert into a memory
 * segment, when full it will dump to a persist segment.
 */
class Collection {
 public:
  PROXIMA_DISALLOW_COPY_AND_ASSIGN(Collection);

  //! Constructor
  Collection(const std::string &collection_name, const std::string &prefix_path,
             meta::CollectionMetaPtr schema, uint32_t concurrency,
             ThreadPool *thread_pool);

  //! Destructor
  ~Collection();

  //! Create an instance
  static CollectionPtr Create(const std::string &collection_name,
                              const std::string &prefix_path,
                              meta::CollectionMetaPtr schema,
                              uint32_t concurrency, ThreadPool *thread_pool);

  //! Crate an instance and open
  static int CreateAndOpen(const std::string &collection_name,
                           const std::string &prefix_path,
                           meta::CollectionMetaPtr schema, uint32_t concurrency,
                           ThreadPool *thread_pool,
                           const ReadOptions &read_options,
                           CollectionPtr *collection);

 public:
  //! Open and initialize collection
  int open(const ReadOptions &read_options);

  //! Close collection
  int close();

  //! Close collection and cleanup files
  int close_and_cleanup();

  //! Flush collection's memory to persist storage
  int flush();

  //! Dump collection's memory segment to persist segment
  int dump();

  //! Optimize collection memory usage
  int optimize(ThreadPoolPtr pool);

 public:
  //! Batch write records
  int write_records(const CollectionDataset &records);

  //! Get latest lsn context of record
  int get_latest_lsn(uint64_t *lsn, std::string *lsn_context);

  //! Get all segments
  int get_segments(std::vector<SegmentPtr> *segments);

  //! Get statistics
  int get_stats(CollectionStats *stats);

 public:
  //! Update schema
  int update_schema(meta::CollectionMetaPtr new_schema);

 public:
  //! Get collection name
  const std::string &collection_name() const {
    return collection_name_;
  }

  //! Get collection disk path
  const std::string &dir_path() const {
    return dir_path_;
  }

  //! Get collection schema
  const meta::CollectionMetaPtr &schema() {
    return schema_;
  }

 private:
  int recover_from_snapshot(const ReadOptions &read_options);

  int remove_files();

  int open_memory_segment(const SegmentMeta &segment_meta,
                          const ReadOptions &read_options,
                          MemorySegmentPtr *new_segment);

  int load_persist_segment(const SegmentMeta &segment_meta,
                           const ReadOptions &read_options,
                           PersistSegmentPtr *new_segment);

  int drive_dump_segment();

  int do_dump_segment();

  void diff_schema(const meta::CollectionMeta &new_schema,
                   const meta::CollectionMeta &current_schema,
                   std::vector<meta::ColumnMetaPtr> *add_columns,
                   std::vector<meta::ColumnMetaPtr> *delete_columns);

  int insert_record(const Record &record);

  int delete_record(uint64_t primary_key);

  int update_record(const Record &record);

  bool has_record(uint64_t primary_key);

  int search_record(uint64_t primary_key, Record *record);

 private:
  static constexpr uint32_t DOC_ID_INCREASE_COUNT = 1000;

 private:
  std::string collection_name_{};
  std::string prefix_path_{};
  std::string dir_path_{};
  meta::CollectionMetaPtr schema_{};
  uint32_t concurrency_{0U};
  ThreadPool *thread_pool_{nullptr};

  IDMapPtr id_map_{};
  DeleteStorePtr delete_store_{};
  LsnStorePtr lsn_store_{};
  MemorySegmentPtr writing_segment_{};
  MemorySegmentPtr dumping_segment_{};
  VersionManagerPtr version_manager_{};
  PersistSegmentManagerPtr persist_segment_mgr_{};

  std::mutex schema_mutex_{};
  std::atomic<bool> is_dumping_{false};
  std::atomic<bool> is_flushing_{false};
  std::atomic<bool> is_optimizing_{false};

  bool opened_{false};
};


}  // end namespace index
}  // namespace be
}  // end namespace proxima
