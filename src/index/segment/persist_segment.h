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
 *   \brief    Provides search ability for persist segment
 */

#pragma once

#include <unordered_map>
#include <ailego/parallel/lock.h>
#include "common/macro_define.h"
#include "meta/meta.h"
#include "segment.h"
#include "../column/column_reader.h"
#include "../column/forward_reader.h"
#include "../concurrent_hash_map.h"
#include "../delete_store.h"
#include "../id_map.h"
#include "../typedef.h"

namespace proxima {
namespace be {
namespace index {

class PersistSegment;
using PersistSegmentPtr = std::shared_ptr<PersistSegment>;

/*
 * A PersistSegment represents block of index data in persist storage.
 * It transform from MemorySegment which will dump to PersistSegment
 * at some condition.
 * It also provides search abilify, but readonly
 */
class PersistSegment : public Segment {
 public:
  PROXIMA_DISALLOW_COPY_AND_ASSIGN(PersistSegment);

  //! Constructor
  PersistSegment(const std::string &coll_name, const std::string &coll_path,
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
  ~PersistSegment() override;

  //! Create an instance
  static PersistSegmentPtr Create(const std::string &coll_name,
                                  const std::string &coll_path,
                                  const SegmentMeta &seg_meta,
                                  const meta::CollectionMeta *schema_ptr,
                                  const DeleteStore *delete_store_ptr,
                                  const IDMap *id_map_ptr,
                                  uint32_t concurrency_val);

  //! Create and load an instance
  static int CreateAndLoad(const std::string &collection_name,
                           const std::string &collection_path,
                           const SegmentMeta &segment_meta,
                           const meta::CollectionMeta *schema,
                           const DeleteStore *delete_store, const IDMap *id_map,
                           uint32_t concurrency,
                           const ReadOptions &read_options,
                           PersistSegmentPtr *persist_segment);

 public:
  //! Load index from persist storage
  int load(const ReadOptions &read_options);

  //! Unload index
  int unload();

 public:
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
  //! Remove a index column
  int remove_column(const std::string &column_name) override;

  //! Add a index column
  int add_column(const meta::ColumnMetaPtr &column_meta) override;

 public:
  //! Return forward count
  size_t doc_count() const override {
    return forward_reader_->doc_count();
  }

 public:
  //! Get forward reader
  ForwardReaderPtr get_forward_reader() const override {
    return forward_reader_;
  }

  //! Get column reader
  ColumnReaderPtr get_column_reader(
      const std::string &column_name) const override {
    if (!column_readers_.has(column_name)) {
      return ColumnReaderPtr();
    }
    return column_readers_.get(column_name);
  }

 private:
  int load_forward_reader(const ReadOptions &read_options);

  int load_column_readers(const ReadOptions &read_options);

 private:
  static constexpr uint32_t MAX_WAIT_RETRY_COUNT = 60U;

 private:
  const meta::CollectionMeta *schema_{nullptr};
  const DeleteStore *delete_store_{nullptr};
  const IDMap *id_map_{nullptr};
  uint32_t concurrency_{0U};

  ForwardReaderPtr forward_reader_{};
  ConcurrentHashMap<std::string, ColumnReaderPtr> column_readers_{};

  std::atomic<uint64_t> active_search_count_{0U};
  bool loaded_{false};
};


}  // end namespace index
}  // namespace be
}  // end namespace proxima
