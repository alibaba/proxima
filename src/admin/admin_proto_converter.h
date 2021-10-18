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

 *   \author   Jiliang.ljl
 *   \date     Mar 2021
 *   \brief
 *   \detail
 */

#pragma once

#include "index/collection_stats.h"
#include "meta/meta.h"
#include "proto/proxima_be.pb.h"

namespace proxima {
namespace be {
namespace admin {

class AdminProtoConverter {
 public:
  //! Helper function for serialize PB to CollectionMeta
  static int PBToCollectionBase(const proto::CollectionConfig &request,
                                meta::CollectionBase *param);

  //! Convert CollectionMeta to PB
  static void CollectionMetaToPB(const meta::CollectionMeta &collection,
                                 proto::CollectionInfo *info);

  //! Convert CollectionStats to PB
  static void CollectionStatsToPB(const index::CollectionStats &stats,
                                  proto::CollectionStats *pb_stats);

 private:
  //! Convert SegmentStats to PB
  static void SegmentStatsToPB(const index::SegmentStats &stats,
                               proto::CollectionStats::SegmentStats *pb_stats);

  //! Convert Repository to PB
  static void RepositoryToPB(std::shared_ptr<meta::RepositoryBase> repo,
                             proto::CollectionConfig *config);

  //! Convert ColumnMeta to PB
  static void ColumnMetaToPB(const meta::ColumnMetaPtr &column,
                             proto::CollectionConfig::IndexColumnParam *param);

  //! Helper function for serializing PB to IndexColumnParam
  static void PBToColumnMeta(
      const proto::CollectionConfig::IndexColumnParam *request,
      meta::ColumnMeta *column);
};

}  // namespace admin
}  // namespace be
}  // namespace proxima
