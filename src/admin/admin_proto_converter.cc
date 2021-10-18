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

#include "admin_proto_converter.h"
#include "common/types_helper.h"
namespace proxima {
namespace be {
namespace admin {

namespace {

//! Status Code Book
struct StatusCodeBook {
  //! Convert C++ CollectionStatus to protobuf CollectionStatus
  static proto::CollectionInfo::CollectionStatus Get(
      meta::CollectionStatus status) {
    switch (status) {
      case meta::CollectionStatus::INITIALIZED:
        return proto::CollectionInfo_CollectionStatus_CS_INITIALIZED;
      case meta::CollectionStatus::SERVING:
        return proto::CollectionInfo_CollectionStatus_CS_SERVING;
      case meta::CollectionStatus::DROPPED:
        return proto::CollectionInfo_CollectionStatus_CS_DROPPED;
    }
    return proto::CollectionInfo_CollectionStatus_CS_INITIALIZED;
  }
};

//! Index Type Codebook
struct IndexTypeCodeBook {
  //! convert protobuf IndexType to C++ IndexTypes
  static IndexTypes Get(be::proto::IndexType type) {
    switch (type) {
      case be::proto::IT_PROXIMA_GRAPH_INDEX:
        return IndexTypes::PROXIMA_GRAPH_INDEX;
      default:
        break;
    }
    return IndexTypes::UNDEFINED;
  }

  //! Convert C++ IndexTypes to protobuf IndexType
  static be::proto::IndexType Get(IndexTypes type) {
    switch (type) {
      case IndexTypes::PROXIMA_GRAPH_INDEX:
        return be::proto::IT_PROXIMA_GRAPH_INDEX;
      default:
        break;
    }
    return be::proto::IT_UNDEFINED;
  }
};

}  // namespace

void AdminProtoConverter::PBToColumnMeta(
    const proto::CollectionConfig::IndexColumnParam *request,
    meta::ColumnMeta *column) {
  column->set_name(request->column_name());
  column->set_index_type(IndexTypeCodeBook::Get(request->index_type()));
  column->set_data_type(DataTypeCodeBook::Get(request->data_type()));

  IndexParamsHelper::SerializeToParams(request->extra_params(),
                                       column->mutable_parameters());
  column->set_dimension(request->dimension());
}

int AdminProtoConverter::PBToCollectionBase(
    const proto::CollectionConfig &request, meta::CollectionBase *param) {
  param->mutable_name()->assign(request.collection_name());
  if (request.has_repository_config()) {
    const auto &repo_config = request.repository_config();
    switch (repo_config.repository_type()) {
      case proto::
          CollectionConfig_RepositoryConfig_RepositoryType_RT_DATABASE: {
        auto &database = repo_config.database();
        auto db_repo = std::make_shared<meta::DatabaseRepositoryMeta>();
        db_repo->set_connection(database.connection_uri());
        db_repo->set_table_name(database.table_name());
        db_repo->set_password(database.password());
        db_repo->set_user(database.user());
        db_repo->set_name(repo_config.repository_name());
        db_repo->set_type(meta::RepositoryTypes::DATABASE);
        param->set_repository(db_repo);
        break;
      }
      default:
        LOG_ERROR("Invalid repository type. repository_type[%d] collection[%s]",
                  static_cast<int>(repo_config.repository_type()),
                  request.collection_name().c_str());
        return PROXIMA_BE_ERROR_CODE(InvalidRepositoryType);
    }
  }
  // Should we recommend some of default value for users
  param->set_max_docs_per_segment(request.max_docs_per_segment());
  // Serialize forward_columns
  param->mutable_forward_columns()->insert(
      param->forward_columns().begin(), request.forward_column_names().begin(),
      request.forward_column_names().end());
  int columns = request.index_column_params_size();
  while (columns--) {
    auto column = std::make_shared<meta::ColumnMeta>();
    PBToColumnMeta(&request.index_column_params(columns), column.get());
    param->append(std::move(column));
  }
  return 0;
}

void AdminProtoConverter::ColumnMetaToPB(
    const meta::ColumnMetaPtr &column,
    proto::CollectionConfig::IndexColumnParam *param) {
  param->set_column_name(column->name());
  param->set_index_type(IndexTypeCodeBook::Get(column->index_type()));
  param->set_data_type(DataTypeCodeBook::Get(column->data_type()));

  be::IndexParamsHelper::SerializeToPB(column->parameters(),
                                       param->mutable_extra_params());
  param->set_dimension(column->dimension());
}

void AdminProtoConverter::RepositoryToPB(
    std::shared_ptr<meta::RepositoryBase> repo,
    proto::CollectionConfig *config) {
  if (!repo) {
    return;
  }
  auto *repo_config = config->mutable_repository_config();
  repo_config->set_repository_name(repo->name());
  switch (repo->type()) {
    case meta::RepositoryTypes::DATABASE: {
      auto db_repo =
          std::dynamic_pointer_cast<meta::DatabaseRepositoryMeta>(repo);
      if (!db_repo) {
        LOG_ERROR("Mismatched repository. type[%d] name[%s]",
                  static_cast<int>(repo->type()), repo->name().c_str());
        return;
      }
      repo_config->set_repository_type(
          proto::CollectionConfig_RepositoryConfig_RepositoryType_RT_DATABASE);
      auto *db = repo_config->mutable_database();
      db->set_connection_uri(db_repo->connection());
      db->set_table_name(db_repo->table_name());
      db->set_user(db_repo->user());
      db->set_password(db_repo->password());
      break;
    }

    default:
      LOG_FATAL("Unexpected repository type. type[%d]",
                static_cast<int>(repo->type()));
  }
}

void AdminProtoConverter::CollectionMetaToPB(
    const meta::CollectionMeta &collection, proto::CollectionInfo *info) {
  info->set_status(StatusCodeBook::Get(collection.status()));
  info->set_uuid(collection.uid());
  auto *config = info->mutable_config();
  config->set_collection_name(collection.name());
  config->set_max_docs_per_segment(collection.max_docs_per_segment());

  for (auto &forward : collection.forward_columns()) {
    config->add_forward_column_names(forward);
  }
  for (auto &column : collection.index_columns()) {
    auto *column_meta = config->add_index_column_params();
    ColumnMetaToPB(column, column_meta);
  }
  RepositoryToPB(collection.repository(), config);
}

#define SET_STATS_FIELD(name) pb_stats->set_##name(stats.name)
void AdminProtoConverter::SegmentStatsToPB(
    const index::SegmentStats &stats,
    proto::CollectionStats::SegmentStats *pb_stats) {
  SET_STATS_FIELD(segment_id);
  SET_STATS_FIELD(doc_count);
  SET_STATS_FIELD(index_file_count);
  SET_STATS_FIELD(index_file_size);
  SET_STATS_FIELD(min_doc_id);
  SET_STATS_FIELD(max_doc_id);
  SET_STATS_FIELD(min_primary_key);
  SET_STATS_FIELD(max_primary_key);
  SET_STATS_FIELD(min_timestamp);
  SET_STATS_FIELD(max_timestamp);
  SET_STATS_FIELD(min_lsn);
  SET_STATS_FIELD(max_lsn);
  pb_stats->set_state(
      static_cast<proto::CollectionStats_SegmentStats_SegmentState>(
          stats.state));
}

void AdminProtoConverter::CollectionStatsToPB(
    const index::CollectionStats &stats, proto::CollectionStats *pb_stats) {
  SET_STATS_FIELD(collection_name);
  SET_STATS_FIELD(collection_path);
  SET_STATS_FIELD(total_doc_count);
  SET_STATS_FIELD(total_segment_count);
  SET_STATS_FIELD(total_index_file_count);
  SET_STATS_FIELD(total_index_file_size);
  for (const auto &segment : stats.segment_stats) {
    SegmentStatsToPB(segment, pb_stats->add_segment_stats());
  }
}
#undef SET_STATS_FIELD

}  // namespace admin
}  // namespace be
}  // namespace proxima
