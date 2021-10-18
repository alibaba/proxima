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
 *   \brief    Writing requests will insert into memory segment firstly.
 *             Then it will dump to persist segment when it's full
 */

#pragma once

#include <ailego/parallel/lock.h>
#include "common/auto_counter.h"
#include "meta/meta.h"
#include "segment.h"
#include "../collection_dataset.h"
#include "../column/column_indexer.h"
#include "../column/forward_indexer.h"
#include "../concurrent_hash_map.h"
#include "../delete_store.h"
#include "../id_map.h"

namespace proxima {
namespace be {
namespace index {

class MemorySegment;
using MemorySegmentPtr = std::shared_ptr<MemorySegment>;

/*
 * A MemorySegment represents block of index data in memory, with
 * streaming insert and search ability at the same time. You can
 * set a docs limit to it, and it will dump to persit storage when
 * full.
 */
class MemorySegment : public Segment {
 public:
  PROXIMA_DISALLOW_COPY_AND_ASSIGN(MemorySegment);

  //! Constructor
  MemorySegment(const std::string &coll_name, const std::string &coll_path,
                const SegmentMeta &seg_meta,
                const meta::CollectionMeta *schema_ptr,
                const DeleteStore *delete_store_ptr, const IDMap *id_map_ptr,
                uint32_t concurrency_val)
      : schema_(schema_ptr),
        delete_store_(delete_store_ptr),
        id_map_(id_map_ptr),
        concurrency_(concurrency_val) {
    this->set_collection_name(coll_name);
    this->set_collection_path(coll_path);
    this->set_segment_meta(seg_meta);
  }


  //! Destructor
  ~MemorySegment() override;

  //! Create an instance and return shared ptr
  static MemorySegmentPtr Create(const std::string &collection_name,
                                 const std::string &collection_path,
                                 const SegmentMeta &segment_meta,
                                 const meta::CollectionMeta *schema,
                                 const DeleteStore *delete_store,
                                 const IDMap *id_map, uint32_t concurrency);

  //! Create and open an instance
  static int CreateAndOpen(const std::string &collection_name,
                           const std::string &collection_path,
                           const SegmentMeta &segment_meta,
                           const meta::CollectionMeta *schema,
                           const DeleteStore *delete_store, const IDMap *id_map,
                           uint32_t concurrency,
                           const ReadOptions &read_options,
                           MemorySegmentPtr *memory_segment);

 public:
  //! Open and initialize memory segment
  int open(const ReadOptions &read_options);

  //! Close and cleanup memory segment
  int close();

  //! Flush memory to persist storage
  int flush();

  //! Dump to another index type to persist storage
  int dump();

  //! Close and remove internal files
  int close_and_remove_files();

 public:
  //! Insert a record & alloc a doc_id
  int insert(const Record &record, idx_t *doc_id);

  //! Remove a record
  int remove(idx_t doc_id);

  //! Optimize memory usage
  int optimize(ThreadPoolPtr pool);

#if 0
  //! Update a record
  int update(idx_t doc_id, const Record &record);
#endif

  //! Knn similar search
  int knn_search(const std::string &column_name, const std::string &query,
                 const QueryParams &query_params,
                 QueryResultList *results) override;

  //! Knn similar search with batch mode
  int knn_search(const std::string &column_name, const std::string &query,
                 const QueryParams &query_params, uint32_t batch_count,
                 std::vector<QueryResultList> *results) override;

  //! Just search forward by doc primary key
  int kv_search(uint64_t primary_key, QueryResult *result) override;

 public:
  //! Remove a column
  int remove_column(const std::string &column_name) override;

  //! Add a column
  int add_column(const meta::ColumnMetaPtr &column_meta) override;

 public:
  //! Set segment state
  void update_state(SegmentState new_state) {
    segment_meta_.state = new_state;
  }

  //! Return forward count
  size_t doc_count() const override {
    return segment_meta_.doc_count;
  }

 public:
  //! Get forward reader
  ForwardReaderPtr get_forward_reader() const override {
    return forward_indexer_;
  }

  //! Get column reader
  ColumnReaderPtr get_column_reader(
      const std::string &column_name) const override {
    if (!column_indexers_.has(column_name)) {
      return ColumnReaderPtr();
    }
    return column_indexers_.get(column_name);
  }

 private:
  int open_forward_indexer(const ReadOptions &read_options);

  int open_column_indexers(const ReadOptions &read_options);

  int dump_forward_indexer(const IndexDumperPtr &dumper);

  int dump_column_indexers(const IndexDumperPtr &dumper);

  void update_stats(const Record &record, idx_t doc_id);

  size_t get_index_file_count();

  size_t get_index_file_size();

 private:
  static constexpr uint32_t MAX_WAIT_RETRY_COUNT = 60U;

 private:
  const meta::CollectionMeta *schema_{nullptr};
  const DeleteStore *delete_store_{nullptr};
  const IDMap *id_map_{nullptr};
  uint32_t concurrency_{0U};

  ForwardIndexerPtr forward_indexer_{};
  ConcurrentHashMap<std::string, ColumnIndexerPtr> column_indexers_{};

  std::mutex mutex_{};
  std::atomic<uint64_t> active_insert_count_{0U};
  std::atomic<uint64_t> active_search_count_{0U};
  bool opened_{false};
};


}  // end namespace index
}  // namespace be
}  // end namespace proxima
