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
 *   \brief    Segment represents piece of collection data.
 *             This class describes the data struct and meta info
 */

#pragma once

#include <memory>
#include "common/macro_define.h"
#include "common/types.h"
#include "meta/meta.h"
#include "../collection_dataset.h"
#include "../collection_query.h"
#include "../column/column_reader.h"
#include "../column/forward_reader.h"
#include "../constants.h"
#include "../typedef.h"

namespace proxima {
namespace be {
namespace index {

class Segment;
using SegmentPtr = std::shared_ptr<Segment>;
using SegmentPtrList = std::vector<SegmentPtr>;

/*
 * Segment state
 */
enum SegmentState : uint32_t {
  CREATED = 0,
  WRITING,
  DUMPING,
  COMPACTING,
  PERSIST
};

/*
 * Segment meta info, records basic stats of a segment.
 */
struct SegmentMeta {
  uint32_t segment_id{0U};
  uint32_t state{0U};
  uint64_t doc_count{0U};
  uint64_t index_file_count{0U};
  uint64_t index_file_size{0U};
  uint64_t min_doc_id{0U};
  uint64_t max_doc_id{0U};
  uint64_t min_primary_key{0U};
  uint64_t max_primary_key{0U};
  uint64_t min_timestamp{0U};
  uint64_t max_timestamp{0U};
  uint64_t min_lsn{0U};
  uint64_t max_lsn{0U};
  uint32_t reserved_[8];

  SegmentMeta() {
    min_primary_key = INVALID_KEY;
    min_timestamp = -1UL;
    min_lsn = -1UL;
  }
};

static_assert(sizeof(SegmentMeta) % 64 == 0,
              "SegmentMeta must be aligned with 64 bytes");

/*
 * SegmentProvider provides interfaces which can get/set property
 */
class SegmentProvider {
 public:
  //! Destructor
  virtual ~SegmentProvider() = default;

 public:
  //! Return collection name
  const std::string &collection_name() const {
    return collection_name_;
  }

  //! Return collection path
  const std::string &collection_path() const {
    return collection_path_;
  }

  //! Return segment id
  SegmentID segment_id() const {
    return segment_meta_.segment_id;
  }

  //! Return segment state
  SegmentState state() const {
    return (SegmentState)segment_meta_.state;
  }

  //! Return segment min doc id
  idx_t min_doc_id() const {
    return segment_meta_.min_doc_id;
  }

  //! Return segment meta
  const SegmentMeta &segment_meta() const {
    return segment_meta_;
  }

  bool is_in_range(idx_t doc_id) const {
    if (doc_id >= segment_meta_.min_doc_id &&
        doc_id <= segment_meta_.max_doc_id) {
      return true;
    }

    return false;
  }

  //! Return document count
  virtual size_t doc_count() const = 0;

 protected:
  void set_collection_name(const std::string &val) {
    collection_name_ = val;
  }

  void set_collection_path(const std::string &val) {
    collection_path_ = val;
  }

  void set_segment_meta(const SegmentMeta &val) {
    segment_meta_ = val;
  }

 protected:
  std::string collection_name_{};
  std::string collection_path_{};
  SegmentMeta segment_meta_{};
};

/*
 * Segment can search records in the segment, it also
 * provides much information about the segment.
 */
class Segment : public SegmentProvider {
 public:
  //! Destructor
  virtual ~Segment() = default;

 public:
  //! Return forward reader
  virtual ForwardReaderPtr get_forward_reader() const = 0;

  //! Return column reader
  virtual ColumnReaderPtr get_column_reader(
      const std::string &column_name) const = 0;

 public:
  //! Knn search
  virtual int knn_search(const std::string &column_name,
                         const std::string &query,
                         const QueryParams &query_params,
                         QueryResultList *results) = 0;

  //! Knn search with batch mode
  virtual int knn_search(const std::string &column_name,
                         const std::string &query,
                         const QueryParams &query_params, uint32_t batch_count,
                         std::vector<QueryResultList> *results) = 0;

  //! Kv search some document
  virtual int kv_search(uint64_t primary_key, QueryResult *result) = 0;

 public:
  //! Add a column
  virtual int add_column(const meta::ColumnMetaPtr &column_meta) = 0;

  //! Remove a column
  virtual int remove_column(const std::string &column_name) = 0;
};


}  // end namespace index
}  // namespace be
}  // end namespace proxima
