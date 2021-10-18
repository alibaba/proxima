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

#define private public
#include "server/write_request_builder.h"
#undef private
#include <gtest/gtest.h>
#include "common/error_code.h"

using namespace ::proxima::be;
using namespace ::proxima::be::server;

class WriteRequestBuilderTest : public testing::Test {
 protected:
  void SetUp() {}

  void TearDown() {}
};

void fill_collection_info(proto::WriteRequest &request,
                          meta::CollectionMetaPtr *meta,
                          agent::ColumnOrderMapPtr *order_map,
                          bool with_repo = true, bool is_bytes = false) {
  request.set_request_id("00000000");
  if (with_repo) {
    request.set_magic_number(140140140);
  }
  int dim = 4;
  request.set_collection_name("mytest");
  auto *row_meta = request.mutable_row_meta();
  auto *column_meta = row_meta->add_index_column_metas();
  column_meta->set_column_name("field1");
  column_meta->set_data_type(proto::DataType::DT_VECTOR_FP32);
  column_meta->set_dimension(dim);
  ;
  row_meta->add_forward_column_names("forward_f1");
  row_meta->add_forward_column_names("forward_f2");
  auto *row1 = request.add_rows();
  row1->set_primary_key(1000);
  row1->set_operation_type(::proxima::be::proto::OP_INSERT);
  if (with_repo) {
    auto *ctx = row1->mutable_lsn_context();
    ctx->set_lsn(1);
    ctx->set_context("binlog:123");
  }
  if (!is_bytes) {
    row1->mutable_index_column_values()->add_values()->set_string_value(
        "[1,2,3,4]");
  } else {
    std::vector<float> vectors = {1.0, 2.0, 3.0, 4.0};
    row1->mutable_index_column_values()->add_values()->set_bytes_value(
        (const void *)vectors.data(), vectors.size() * sizeof(float));
  }
  row1->mutable_forward_column_values()->add_values()->set_float_value(10.0);
  row1->mutable_forward_column_values()->add_values()->set_int32_value(20);

  *meta = std::make_shared<meta::CollectionMeta>();
  (*meta)->set_name("mytest");

  if (with_repo) {
    meta::RepositoryBasePtr repo =
        std::make_shared<meta::DatabaseRepositoryMeta>();
    repo->set_name("mytest");
    (*meta)->set_repository(repo);
  }

  (*meta)->mutable_forward_columns()->emplace_back("forward_f1");
  (*meta)->mutable_forward_columns()->emplace_back("forward_f2");

  auto *index_columns = (*meta)->mutable_index_columns();

  meta::ColumnMetaPtr column_meta1 = std::make_shared<meta::ColumnMeta>();
  column_meta1->set_index_type(IndexTypes::PROXIMA_GRAPH_INDEX);
  column_meta1->set_name("field1");
  column_meta1->set_data_type(DataTypes::VECTOR_FP32);
  column_meta1->set_dimension(4);

  index_columns->emplace_back(column_meta1);

  *order_map = std::make_shared<agent::ColumnOrderMap>();
  (*order_map)->add_column_order(**meta);
}

int create_write_request(const proto::WriteRequest &request,
                         const meta::CollectionMetaPtr &meta,
                         agent::ColumnOrderMapPtr &order_map,
                         agent::WriteRequest *write_request) {
  auto column_order = order_map->get_column_order(request.collection_name());

  return WriteRequestBuilder::build(*meta, *column_order, request,
                                    write_request);
}

TEST_F(WriteRequestBuilderTest, TestCreateSuccessWithProxyWrite) {
  proto::WriteRequest request;
  meta::CollectionMetaPtr meta;
  agent::ColumnOrderMapPtr order_map;
  fill_collection_info(request, &meta, &order_map, true);
  agent::WriteRequest write_request;
  int ret = create_write_request(request, meta, order_map, &write_request);
  ASSERT_EQ(ret, 0);
  ASSERT_EQ(write_request.magic_number(), 140140140);
  ASSERT_EQ(write_request.collection_name(), "mytest");
  auto &records = write_request.get_collection_dataset();
  ASSERT_EQ(records.size(), (size_t)1);
  ASSERT_EQ(records[0]->size(), (size_t)1);
  ASSERT_EQ(records[0]->schema_revision_, 0);
  auto &raw_data = records[0]->get(0);
  ASSERT_EQ(raw_data.primary_key, 1000);
  ASSERT_EQ(raw_data.operation_type, OperationTypes::INSERT);
  ASSERT_EQ(raw_data.lsn, 1);
  ASSERT_EQ(raw_data.lsn_context, "binlog:123");
  ::proxima::be::proto::GenericValueList forward_list;
  forward_list.add_values()->set_float_value(10.0);
  forward_list.add_values()->set_int32_value(20);
  std::string expected_forward_str;
  forward_list.SerializeToString(&expected_forward_str);
  ASSERT_EQ(raw_data.forward_data, expected_forward_str);
  auto &column_datas = raw_data.column_datas;
  ASSERT_EQ(column_datas.size(), (size_t)1);
  ASSERT_EQ(column_datas[0].column_name, "field1");
  ASSERT_EQ(column_datas[0].data_type, DataTypes::VECTOR_FP32);
  ASSERT_EQ(column_datas[0].dimension, (uint32_t)4);
  ASSERT_EQ(column_datas[0].data.size(), (size_t)16);
  const float *vector_data = (const float *)column_datas[0].data.data();
  ASSERT_FLOAT_EQ(vector_data[0], 1.0);
  ASSERT_FLOAT_EQ(vector_data[1], 2.0);
  ASSERT_FLOAT_EQ(vector_data[2], 3.0);
  ASSERT_FLOAT_EQ(vector_data[3], 4.0);
}

TEST_F(WriteRequestBuilderTest, TestCreateSuccessWithDirectWrite) {
  proto::WriteRequest request;
  meta::CollectionMetaPtr meta;
  agent::ColumnOrderMapPtr order_map;
  fill_collection_info(request, &meta, &order_map, false);
  agent::WriteRequest write_request;
  int ret = create_write_request(request, meta, order_map, &write_request);
  ASSERT_EQ(ret, 0);
  ASSERT_EQ(write_request.magic_number(), 0);
  ASSERT_EQ(write_request.collection_name(), "mytest");
  auto &records = write_request.get_collection_dataset();
  ASSERT_EQ(records.size(), (size_t)1);
  auto &record = records[0];
  ASSERT_EQ(record->schema_revision_, 0);
  auto &raw_data = record->get(0);
  ASSERT_EQ(raw_data.primary_key, 1000);
  ASSERT_EQ(raw_data.lsn_check, false);
  ASSERT_EQ(raw_data.operation_type, OperationTypes::INSERT);
  ::proxima::be::proto::GenericValueList forward_list;
  forward_list.add_values()->set_float_value(10.0);
  forward_list.add_values()->set_int32_value(20);
  std::string expected_forward_str;
  forward_list.SerializeToString(&expected_forward_str);
  ASSERT_EQ(raw_data.forward_data, expected_forward_str);
  auto &column_datas = raw_data.column_datas;
  ASSERT_EQ(column_datas.size(), (size_t)1);
  ASSERT_EQ(column_datas[0].column_name, "field1");
  ASSERT_EQ(column_datas[0].data_type, DataTypes::VECTOR_FP32);
  ASSERT_EQ(column_datas[0].dimension, (uint32_t)4);
  ASSERT_EQ(column_datas[0].data.size(), (size_t)16);
  const float *vector_data = (const float *)column_datas[0].data.data();
  ASSERT_FLOAT_EQ(vector_data[0], 1.0);
  ASSERT_FLOAT_EQ(vector_data[1], 2.0);
  ASSERT_FLOAT_EQ(vector_data[2], 3.0);
  ASSERT_FLOAT_EQ(vector_data[3], 4.0);
}

TEST_F(WriteRequestBuilderTest, TestCreateFailedWithDimensionMismatched) {
  proto::WriteRequest request;
  meta::CollectionMetaPtr meta;
  agent::ColumnOrderMapPtr order_map;
  fill_collection_info(request, &meta, &order_map, true);
  request.mutable_row_meta()->mutable_index_column_metas(0)->set_dimension(100);
  agent::WriteRequest write_request;
  int ret = create_write_request(request, meta, order_map, &write_request);
  ASSERT_EQ(ret, ErrorCode_InvalidWriteRequest);
}

TEST_F(WriteRequestBuilderTest, TestCreateFailedWithValidate) {
  proto::WriteRequest request;
  meta::CollectionMetaPtr meta;
  agent::ColumnOrderMapPtr order_map;
  fill_collection_info(request, &meta, &order_map, true);
  agent::WriteRequest write_request;
  proto::WriteRequest tmp_request;
  int ret = create_write_request(tmp_request, meta, order_map, &write_request);
  ASSERT_EQ(ret, ErrorCode_InvalidWriteRequest);
}

TEST_F(WriteRequestBuilderTest, TestCreateFailedWithBuildProxyRequest) {
  proto::WriteRequest request;
  meta::CollectionMetaPtr meta;
  agent::ColumnOrderMapPtr order_map;
  agent::WriteRequest write_request;
  fill_collection_info(request, &meta, &order_map, true);
  request.mutable_rows(0)->clear_lsn_context();
  int ret = create_write_request(request, meta, order_map, &write_request);
  ASSERT_EQ(ret, ErrorCode_EmptyLsnContext);
}

TEST_F(WriteRequestBuilderTest, TestCreateFailedWithBuildDirectRequest) {
  proto::WriteRequest request;
  meta::CollectionMetaPtr meta;
  agent::ColumnOrderMapPtr order_map;
  agent::WriteRequest write_request;
  fill_collection_info(request, &meta, &order_map, false);
  request.mutable_rows(0)
      ->mutable_index_column_values()
      ->mutable_values(0)
      ->set_string_value("invalid vector");
  int ret = create_write_request(request, meta, order_map, &write_request);
  ASSERT_EQ(ret, ErrorCode_MismatchedDimension);
}

TEST_F(WriteRequestBuilderTest, TestGetIndexAndForwardModeWithFullMatch) {
  proto::WriteRequest request;
  meta::CollectionMetaPtr meta;
  agent::ColumnOrderMapPtr order_map;
  agent::WriteRequest write_request;
  fill_collection_info(request, &meta, &order_map, false);
  bool index_full_match;
  bool forward_full_match;
  WriteRequestBuilder::get_index_and_forward_mode(
      request, *meta, &index_full_match, &forward_full_match);
  ASSERT_TRUE(index_full_match);
  ASSERT_TRUE(forward_full_match);
}

TEST_F(WriteRequestBuilderTest, TestGetIndexAndForwardModeWithNotFullMatch) {
  proto::WriteRequest request;
  meta::CollectionMetaPtr meta;
  agent::ColumnOrderMapPtr order_map;
  agent::WriteRequest write_request;
  fill_collection_info(request, &meta, &order_map, false);
  request.mutable_row_meta()->set_forward_column_names(0, "invalid");
  auto *index_meta = request.mutable_row_meta()->mutable_index_column_metas(0);
  index_meta->set_column_name("invalid");

  bool index_full_match;
  bool forward_full_match;
  WriteRequestBuilder::get_index_and_forward_mode(
      request, *meta, &index_full_match, &forward_full_match);
  ASSERT_FALSE(index_full_match);
  ASSERT_FALSE(forward_full_match);
}

TEST_F(WriteRequestBuilderTest, TestValidateRequest) {
  proto::WriteRequest request;
  meta::CollectionMetaPtr meta;
  agent::ColumnOrderMapPtr order_map;
  agent::WriteRequest write_request;
  fill_collection_info(request, &meta, &order_map, true);
  agent::ColumnOrderPtr column_order = order_map->get_column_order("mytest");
  int ret = WriteRequestBuilder::validate_request(request, *meta, *column_order,
                                                  true, true);
  ASSERT_EQ(ret, 0);
}

TEST_F(WriteRequestBuilderTest, TestValidateRequestFailedWithIndexColumnSize) {
  proto::WriteRequest request;
  meta::CollectionMetaPtr meta;
  agent::ColumnOrderMapPtr order_map;
  agent::WriteRequest write_request;
  fill_collection_info(request, &meta, &order_map, true);
  agent::ColumnOrderPtr column_order = order_map->get_column_order("mytest");
  request.mutable_row_meta()->add_index_column_metas();
  int ret = WriteRequestBuilder::validate_request(request, *meta, *column_order,
                                                  true, true);
  ASSERT_EQ(ret, ErrorCode_InvalidWriteRequest);
}

TEST_F(WriteRequestBuilderTest, TestValidateRequestFailedWithIndexColumnName) {
  proto::WriteRequest request;
  meta::CollectionMetaPtr meta;
  agent::ColumnOrderMapPtr order_map;
  agent::WriteRequest write_request;
  fill_collection_info(request, &meta, &order_map, true);
  agent::ColumnOrderPtr column_order = order_map->get_column_order("mytest");
  auto *index_meta = request.mutable_row_meta()->mutable_index_column_metas(0);
  index_meta->set_column_name("invalid");

  int ret = WriteRequestBuilder::validate_request(request, *meta, *column_order,
                                                  false, true);
  ASSERT_EQ(ret, ErrorCode_InvalidWriteRequest);
}

TEST_F(WriteRequestBuilderTest,
       TestValidateRequestFailedWithForwardColumnSize) {
  proto::WriteRequest request;
  meta::CollectionMetaPtr meta;
  agent::ColumnOrderMapPtr order_map;
  agent::WriteRequest write_request;
  fill_collection_info(request, &meta, &order_map, true);
  agent::ColumnOrderPtr column_order = order_map->get_column_order("mytest");
  request.mutable_row_meta()->add_forward_column_names("invalid");
  int ret = WriteRequestBuilder::validate_request(request, *meta, *column_order,
                                                  true, false);
  ASSERT_EQ(ret, ErrorCode_InvalidWriteRequest);
}

TEST_F(WriteRequestBuilderTest,
       TestValidateRequestFailedWithForwardColumnName) {
  proto::WriteRequest request;
  meta::CollectionMetaPtr meta;
  agent::ColumnOrderMapPtr order_map;
  agent::WriteRequest write_request;
  fill_collection_info(request, &meta, &order_map, true);
  agent::ColumnOrderPtr column_order = order_map->get_column_order("mytest");
  request.mutable_row_meta()->set_forward_column_names(0, "invalid");
  int ret = WriteRequestBuilder::validate_request(request, *meta, *column_order,
                                                  true, false);
  ASSERT_EQ(ret, ErrorCode_InvalidWriteRequest);
}

TEST_F(WriteRequestBuilderTest,
       TestValidateRequestFailedWithIndexColumnSizeZero) {
  proto::WriteRequest request;
  meta::CollectionMetaPtr meta;
  agent::ColumnOrderMapPtr order_map;
  agent::WriteRequest write_request;
  fill_collection_info(request, &meta, &order_map, true);
  agent::ColumnOrderPtr column_order = order_map->get_column_order("mytest");
  request.mutable_row_meta()->clear_index_column_metas();
  int ret = WriteRequestBuilder::validate_request(request, *meta, *column_order,
                                                  true, false);
  ASSERT_EQ(ret, ErrorCode_InvalidWriteRequest);
}

TEST_F(WriteRequestBuilderTest,
       TestValidateRequestFailedWithIndexColumnSizeMismatched) {
  proto::WriteRequest request;
  meta::CollectionMetaPtr meta;
  agent::ColumnOrderMapPtr order_map;
  agent::WriteRequest write_request;
  fill_collection_info(request, &meta, &order_map, true);
  agent::ColumnOrderPtr column_order = order_map->get_column_order("mytest");
  request.mutable_rows(0)->clear_index_column_values();
  int ret = WriteRequestBuilder::validate_request(request, *meta, *column_order,
                                                  true, false);
  ASSERT_EQ(ret, ErrorCode_InvalidWriteRequest);
}

TEST_F(WriteRequestBuilderTest,
       TestValidateRequestFailedWithForwardColumnSizeMismatched) {
  proto::WriteRequest request;
  meta::CollectionMetaPtr meta;
  agent::ColumnOrderMapPtr order_map;
  agent::WriteRequest write_request;
  fill_collection_info(request, &meta, &order_map, true);
  agent::ColumnOrderPtr column_order = order_map->get_column_order("mytest");
  request.mutable_rows(0)->clear_forward_column_values();
  int ret = WriteRequestBuilder::validate_request(request, *meta, *column_order,
                                                  true, false);
  ASSERT_EQ(ret, ErrorCode_InvalidWriteRequest);
}

TEST_F(WriteRequestBuilderTest, TestBuildForwardsDataWithFull) {
  proto::WriteRequest request;
  meta::CollectionMetaPtr meta;
  agent::ColumnOrderMapPtr order_map;
  fill_collection_info(request, &meta, &order_map, true);
  agent::ColumnOrderPtr column_order = order_map->get_column_order("mytest");

  auto *row = request.mutable_rows(0);
  auto &row_meta = request.row_meta();
  index::CollectionDataset::RowData row_data;
  bool forward_full_match = true;
  int ret = WriteRequestBuilder::build_forwards_data(
      *row, row_meta, *column_order, *meta, forward_full_match, &row_data);
  ASSERT_EQ(ret, 0);
}

TEST_F(WriteRequestBuilderTest, TestBuildForwardsDataWithNotFull) {
  proto::WriteRequest request;
  meta::CollectionMetaPtr meta;
  agent::ColumnOrderMapPtr order_map;
  fill_collection_info(request, &meta, &order_map, true);
  agent::ColumnOrderPtr column_order = order_map->get_column_order("mytest");

  auto *row = request.mutable_rows(0);
  auto &row_meta = request.row_meta();
  index::CollectionDataset::RowData row_data;
  bool forward_full_match = false;
  int ret = WriteRequestBuilder::build_forwards_data(
      *row, row_meta, *column_order, *meta, forward_full_match, &row_data);
  ASSERT_EQ(ret, 0);
}

TEST_F(WriteRequestBuilderTest, TestBuildForwardsDataWithInvalidForwardColumn) {
  proto::WriteRequest request;
  meta::CollectionMetaPtr meta;
  agent::ColumnOrderMapPtr order_map;
  fill_collection_info(request, &meta, &order_map, true);
  agent::ColumnOrderPtr column_order = order_map->get_column_order("mytest");

  auto *row = request.mutable_rows(0);
  auto *row_meta = request.mutable_row_meta();
  row_meta->set_forward_column_names(0, "invalid");
  index::CollectionDataset::RowData row_data;
  bool forward_full_match = false;
  int ret = WriteRequestBuilder::build_forwards_data(
      *row, *row_meta, *column_order, *meta, forward_full_match, &row_data);
  ASSERT_EQ(ret, ErrorCode_InvalidWriteRequest);
}

TEST_F(WriteRequestBuilderTest, TestBuildIndexesDataWithFull) {
  proto::WriteRequest request;
  meta::CollectionMetaPtr meta;
  agent::ColumnOrderMapPtr order_map;
  fill_collection_info(request, &meta, &order_map, true);

  auto *row = request.mutable_rows(0);
  auto &row_meta = request.row_meta();
  index::CollectionDataset::RowData row_data;
  bool index_full_match = true;
  int ret = WriteRequestBuilder::build_indexes_data(
      *row, row_meta, *meta, index_full_match, &row_data);
  ASSERT_EQ(ret, 0);
}

TEST_F(WriteRequestBuilderTest, TestBuildIndexesDataWithNotFull) {
  proto::WriteRequest request;
  meta::CollectionMetaPtr meta;
  agent::ColumnOrderMapPtr order_map;
  fill_collection_info(request, &meta, &order_map, true);

  auto *row = request.mutable_rows(0);
  auto &row_meta = request.row_meta();
  index::CollectionDataset::RowData row_data;
  bool index_full_match = false;
  int ret = WriteRequestBuilder::build_indexes_data(
      *row, row_meta, *meta, index_full_match, &row_data);
  ASSERT_EQ(ret, 0);
}

TEST_F(WriteRequestBuilderTest,
       TestBuildIndexesDataWithIndexColumnNameInvalid) {
  proto::WriteRequest request;
  meta::CollectionMetaPtr meta;
  agent::ColumnOrderMapPtr order_map;
  fill_collection_info(request, &meta, &order_map, true);

  auto *row = request.mutable_rows(0);
  auto *row_meta = request.mutable_row_meta();
  auto *index_meta = request.mutable_row_meta()->mutable_index_column_metas(0);
  index_meta->set_column_name("invalid");

  index::CollectionDataset::RowData row_data;
  bool index_full_match = false;
  int ret = WriteRequestBuilder::build_indexes_data(
      *row, *row_meta, *meta, index_full_match, &row_data);
  ASSERT_EQ(ret, ErrorCode_MismatchedIndexColumn);
}

TEST_F(WriteRequestBuilderTest,
       TestBuildIndexesDataWithIndexColumnTypeInvalid) {
  proto::WriteRequest request;
  meta::CollectionMetaPtr meta;
  agent::ColumnOrderMapPtr order_map;
  fill_collection_info(request, &meta, &order_map, true);

  auto *row = request.mutable_rows(0);
  auto *row_meta = request.mutable_row_meta();
  row->mutable_index_column_values()->mutable_values(0)->set_float_value(
      123.456);

  index::CollectionDataset::RowData row_data;
  bool index_full_match = false;
  int ret = WriteRequestBuilder::build_indexes_data(
      *row, *row_meta, *meta, index_full_match, &row_data);
  ASSERT_EQ(ret, ErrorCode_MismatchedIndexColumn);
}

TEST_F(WriteRequestBuilderTest, TestBuildIndexesDataWithIndexTypeBytes) {
  proto::WriteRequest request;
  meta::CollectionMetaPtr meta;
  agent::ColumnOrderMapPtr order_map;
  fill_collection_info(request, &meta, &order_map, true, true);

  auto *row = request.mutable_rows(0);
  auto *row_meta = request.mutable_row_meta();
  index::CollectionDataset::RowData row_data;
  bool index_full_match = false;
  int ret = WriteRequestBuilder::build_indexes_data(
      *row, *row_meta, *meta, index_full_match, &row_data);
  ASSERT_EQ(ret, 0);
}
