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
 *   \brief    Format of collection statistics
 */

#pragma once

#include "segment/segment.h"

namespace proxima {
namespace be {
namespace index {

/* SegmentStats looks like SegmentMeta, but it will expand
 * many fields for statistics, so we just split it out.
 * And SegmentStats is allowed to store many not-POD type data,
 * and provide debug string interface.
 */
struct SegmentStats {
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

  SegmentStats(const SegmentMeta &segment_meta) {
    segment_id = segment_meta.segment_id;
    state = segment_meta.state;
    doc_count = segment_meta.doc_count;
    index_file_count = segment_meta.index_file_count;
    index_file_size = segment_meta.index_file_size;
    min_doc_id = segment_meta.min_doc_id;
    max_doc_id = segment_meta.max_doc_id;
    min_primary_key = segment_meta.min_primary_key;
    max_primary_key = segment_meta.max_primary_key;
    min_timestamp = segment_meta.min_timestamp;
    max_timestamp = segment_meta.max_timestamp;
    min_lsn = segment_meta.min_lsn;
    max_lsn = segment_meta.max_lsn;
  }
};

/*
 * CollectionStats contains some important metrics of collection.
 * It contains serveral segments, at least one.
 */
struct CollectionStats {
  std::string collection_name{};
  std::string collection_path{};
  uint64_t total_doc_count{0U};
  uint64_t delete_doc_count{0U};
  uint64_t total_segment_count{0U};
  uint64_t total_index_file_count{0U};
  uint64_t total_index_file_size{0U};
  std::vector<SegmentStats> segment_stats{};
};

}  // end namespace index
}  // namespace be
}  // end namespace proxima
