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
 */

#include <ailego/encoding/json.h>
#include <ailego/utility/float_helper.h>
#include <ailego/utility/time_helper.h>

#define private public
#include "admin/admin_proto_converter.h"
#undef private
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace std;
using namespace testing;

namespace proxima {
namespace be {
namespace admin {
namespace test {

proto::CollectionConfig GetTestCollectionConfig() {
  proto::CollectionConfig config;
  config.set_collection_name("collection");
  config.set_max_docs_per_segment(1000);
  config.add_forward_column_names("f1");
  config.add_forward_column_names("f2");
  config.add_forward_column_names("f3");
  auto *index = config.add_index_column_params();
  index->set_dimension(32);
  index->set_column_name("index1");
  index->set_data_type(proto::DT_VECTOR_FP16);
  return config;
}

proto::CollectionConfig GetTestCollectionConfigWithDbRepository() {
  auto config = GetTestCollectionConfig();
  auto *repo = config.mutable_repository_config();
  repo->set_repository_name("test_repo");
  repo->set_repository_type(
      proto::CollectionConfig_RepositoryConfig_RepositoryType_RT_DATABASE);
  auto *db = repo->mutable_database();
  db->set_password("password");
  db->set_user("user");
  db->set_connection_uri("url");
  db->set_table_name("table");
  return config;
}


TEST(AdminProtoConverterTest, PBToCollectionBase) {
  auto config = GetTestCollectionConfig();
  meta::CollectionBase c;
  ASSERT_EQ(AdminProtoConverter::PBToCollectionBase(config, &c), 0);
  EXPECT_EQ(c.name(), "collection");
  EXPECT_EQ(c.max_docs_per_segment(), 1000);
  EXPECT_THAT(c.forward_columns(),
              testing::ContainerEq(vector<string>{"f1", "f2", "f3"}));
  EXPECT_FALSE(c.repository());
  auto &index_columns = c.index_columns();
  EXPECT_EQ(index_columns.size(), 1U);
  auto &index = index_columns[0];
  EXPECT_EQ(index->name(), "index1");
  EXPECT_EQ(index->dimension(), 32);
  EXPECT_EQ(index->data_type(), DataTypes::VECTOR_FP16);
};

TEST(AdminProtoConverterTest, PBToCollectionBaseWithRepository) {
  auto config = GetTestCollectionConfigWithDbRepository();
  meta::CollectionBase c;
  ASSERT_EQ(AdminProtoConverter::PBToCollectionBase(config, &c), 0);
  EXPECT_EQ(c.name(), "collection");
  EXPECT_EQ(c.max_docs_per_segment(), 1000);
  EXPECT_THAT(c.forward_columns(),
              testing::ContainerEq(vector<string>{"f1", "f2", "f3"}));
  auto &index_columns = c.index_columns();
  EXPECT_EQ(index_columns.size(), 1U);
  auto &index = index_columns[0];
  EXPECT_EQ(index->name(), "index1");
  EXPECT_EQ(index->dimension(), 32);
  EXPECT_EQ(index->data_type(), DataTypes::VECTOR_FP16);
  EXPECT_TRUE(c.repository());
  auto db_repo =
      dynamic_pointer_cast<meta::DatabaseRepositoryMeta>(c.repository());
  EXPECT_TRUE(db_repo);
  EXPECT_EQ(db_repo->password(), "password");
  EXPECT_EQ(db_repo->user(), "user");
  EXPECT_EQ(db_repo->table_name(), "table");
  EXPECT_EQ(db_repo->connection(), "url");
  EXPECT_EQ(db_repo->name(), "test_repo");
  EXPECT_EQ(c.repository()->type(), meta::RepositoryTypes::DATABASE);
};

TEST(AdminProtoConverterTest, CollectionMetaToPB) {
  auto config = GetTestCollectionConfig();
  meta::CollectionBase c;
  ASSERT_EQ(AdminProtoConverter::PBToCollectionBase(config, &c), 0);
  meta::CollectionMeta meta(c);
  meta.set_status(meta::CollectionStatus::SERVING);
  proto::CollectionInfo info;
  AdminProtoConverter::CollectionMetaToPB(meta, &info);
  EXPECT_EQ(info.status(), proto::CollectionInfo_CollectionStatus_CS_SERVING);
  auto &conf = info.config();
  EXPECT_EQ(conf.max_docs_per_segment(), 1000);
  EXPECT_EQ(conf.forward_column_names_size(), 3);
  EXPECT_EQ(conf.forward_column_names(0), "f1");
  EXPECT_EQ(conf.forward_column_names(1), "f2");
  EXPECT_EQ(conf.forward_column_names(2), "f3");
  EXPECT_EQ(conf.index_column_params_size(), 1);
  auto &index = conf.index_column_params(0);
  EXPECT_EQ(index.dimension(), 32);
  EXPECT_EQ(index.data_type(), proto::DT_VECTOR_FP16);
  EXPECT_EQ(index.column_name(), "index1");
  EXPECT_EQ(conf.collection_name(), "collection");
  EXPECT_FALSE(conf.has_repository_config());
}

TEST(AdminProtoConverterTest, CollectionMetaToPBWithRepository) {
  auto config = GetTestCollectionConfigWithDbRepository();
  meta::CollectionBase c;
  ASSERT_EQ(AdminProtoConverter::PBToCollectionBase(config, &c), 0);
  meta::CollectionMeta meta(c);
  meta.set_status(meta::CollectionStatus::SERVING);
  proto::CollectionInfo info;
  AdminProtoConverter::CollectionMetaToPB(meta, &info);
  auto &conf = info.config();
  EXPECT_EQ(conf.max_docs_per_segment(), 1000);
  EXPECT_EQ(conf.forward_column_names_size(), 3);
  EXPECT_EQ(conf.forward_column_names(0), "f1");
  EXPECT_EQ(conf.forward_column_names(1), "f2");
  EXPECT_EQ(conf.forward_column_names(2), "f3");
  EXPECT_EQ(conf.index_column_params_size(), 1);
  auto &index = conf.index_column_params(0);
  EXPECT_EQ(index.dimension(), 32);
  EXPECT_EQ(index.data_type(), proto::DT_VECTOR_FP16);
  EXPECT_EQ(index.column_name(), "index1");
  EXPECT_EQ(conf.collection_name(), "collection");
  EXPECT_TRUE(conf.has_repository_config());
  auto &repo_config = conf.repository_config();
  EXPECT_EQ(repo_config.repository_name(), "test_repo");
  EXPECT_EQ(
      repo_config.repository_type(),
      proto::CollectionConfig_RepositoryConfig_RepositoryType_RT_DATABASE);
  EXPECT_TRUE(repo_config.has_database());
  auto &db = repo_config.database();
  EXPECT_EQ(db.user(), "user");
  EXPECT_EQ(db.password(), "password");
  EXPECT_EQ(db.table_name(), "table");
  EXPECT_EQ(db.connection_uri(), "url");
}

TEST(AdminProtoConverterTest, CollectionStatsToPB) {
  index::CollectionStats stats;
  stats.collection_name = "collection";
  stats.collection_path = "path";
  stats.total_segment_count = 1000;
  index::SegmentMeta sm;
  sm.segment_id = 11;
  sm.max_doc_id = 3000;
  stats.segment_stats.push_back(index::SegmentStats(sm));
  stats.segment_stats.push_back(index::SegmentStats(sm));
  proto::CollectionStats pb_stats;
  AdminProtoConverter::CollectionStatsToPB(stats, &pb_stats);
  EXPECT_EQ(pb_stats.collection_name(), "collection");
  EXPECT_EQ(pb_stats.collection_path(), "path");
  EXPECT_EQ(pb_stats.total_segment_count(), 1000);
  EXPECT_EQ(pb_stats.segment_stats_size(), 2);
  EXPECT_EQ(pb_stats.segment_stats(0).segment_id(), 11);
  EXPECT_EQ(pb_stats.segment_stats(0).max_doc_id(), 3000);
}

}  // namespace test
}  // namespace admin
}  // namespace be
}  // namespace proxima
