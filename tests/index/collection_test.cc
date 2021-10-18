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

#include "index/collection.h"
#include <gtest/gtest.h>

using namespace proxima::be;
using namespace proxima::be::index;

class CollectionTest : public testing::Test {
 protected:
  void SetUp() {
    char cmd_buf[100];
    snprintf(cmd_buf, 100, "rm -rf ./teachers/");
    system(cmd_buf);

    FillSchema();
  }

  void TearDown() {}

  void FillSchema() {
    schema_ = std::make_shared<meta::CollectionMeta>();
    meta::ColumnMetaPtr column_meta = std::make_shared<meta::ColumnMeta>();
    column_meta->set_name("face");
    column_meta->set_index_type(IndexTypes::PROXIMA_GRAPH_INDEX);
    column_meta->set_data_type(DataTypes::VECTOR_FP32);
    column_meta->set_dimension(16);
    column_meta->mutable_parameters()->set("metric_type", "SquaredEuclidean");
    schema_->append(column_meta);
    schema_->set_name("teachers");
    schema_->set_revision(0);
    // Set default value to 0
    schema_->set_max_docs_per_segment(0);
  }

 protected:
  meta::CollectionMetaPtr schema_{};
};

TEST_F(CollectionTest, TestGeneral) {
  index::ThreadPool thread_pool(10, false);
  CollectionPtr collection =
      Collection::Create(schema_->name(), "./", schema_, 10, &thread_pool);
  ASSERT_NE(collection, nullptr);
  ReadOptions read_options;
  read_options.use_mmap = true;
  read_options.create_new = true;
  int ret = collection->open(read_options);
  ASSERT_EQ(ret, 0);

  for (size_t i = 0; i < 1000; i++) {
    CollectionDatasetPtr add_records = std::make_shared<CollectionDataset>(1);
    CollectionDataset::RowData *new_row = add_records->add_row_data();
    new_row->primary_key = i;
    new_row->operation_type = OperationTypes::INSERT;
    new_row->lsn = i;
    new_row->forward_data = "hello";

    CollectionDataset::ColumnData new_column;
    new_column.column_name = "face";
    new_column.data_type = DataTypes::VECTOR_FP32;
    new_column.dimension = 16;

    std::vector<float> fvec(16U);
    for (size_t j = 0; j < 16U; j++) {
      fvec[j] = i * 1.0f;
    }
    std::string vector((char *)fvec.data(), fvec.size() * sizeof(float));
    new_column.data = vector;

    new_row->column_datas.emplace_back(new_column);
    ret = collection->write_records(*add_records);
    ASSERT_EQ(ret, 0);

    uint64_t lsn;
    std::string lsn_context;
    ret = collection->get_latest_lsn(&lsn, &lsn_context);
    ASSERT_EQ(ret, 0);
    ASSERT_EQ(lsn, i);
  }

  // test search query
  std::vector<SegmentPtr> segments;
  ret = collection->get_segments(&segments);
  ASSERT_EQ(ret, 0);
  ASSERT_EQ(segments.size(), 1);

  ASSERT_EQ(segments[0]->collection_name(), "teachers");
  ASSERT_EQ(segments[0]->segment_id(), 0);
  ASSERT_EQ(segments[0]->doc_count(), 1000);

  for (size_t i = 0; i < 1000; i++) {
    std::vector<float> fvec(16U);
    for (size_t j = 0; j < 16U; j++) {
      fvec[j] = i * 1.0f;
    }
    std::string query((char *)fvec.data(), fvec.size() * sizeof(float));
    QueryParams query_params;
    query_params.topk = 10;
    query_params.data_type = DataTypes::VECTOR_FP32;
    query_params.dimension = 16;

    QueryResultList result_list;
    ret = segments[0]->knn_search("face", query, query_params, &result_list);
    ASSERT_EQ(ret, 0);
    ASSERT_EQ(result_list.size(), 10);
    ASSERT_EQ(result_list[0].primary_key, i);
    ASSERT_EQ(result_list[0].score, 0.0f);
    ASSERT_EQ(result_list[0].lsn, i);
  }
}

TEST_F(CollectionTest, TestDumpSegment) {
  index::ThreadPool thread_pool(10, false);
  CollectionPtr collection =
      Collection::Create(schema_->name(), "./", schema_, 10, &thread_pool);
  ASSERT_NE(collection, nullptr);

  ReadOptions read_options;
  read_options.use_mmap = true;
  read_options.create_new = true;
  int ret = collection->open(read_options);
  ASSERT_EQ(ret, 0);

  // add segment dump document limit
  schema_->set_max_docs_per_segment(900);

  for (size_t i = 0; i < 2000; i++) {
    CollectionDatasetPtr add_records = std::make_shared<CollectionDataset>(1);
    CollectionDataset::RowData *new_row = add_records->add_row_data();
    new_row->primary_key = i;
    new_row->operation_type = OperationTypes::INSERT;
    new_row->lsn = i;
    new_row->forward_data = "hello";

    CollectionDataset::ColumnData new_column;
    new_column.column_name = "face";
    new_column.data_type = DataTypes::VECTOR_FP32;
    new_column.dimension = 16;

    std::vector<float> fvec(16U);
    for (size_t j = 0; j < 16U; j++) {
      fvec[j] = i * 1.0f;
    }
    std::string vector((char *)fvec.data(), fvec.size() * sizeof(float));
    new_column.data = vector;

    new_row->column_datas.emplace_back(new_column);
    ret = collection->write_records(*add_records);
    ASSERT_EQ(ret, 0);

    if (i > 0 && i % 900 == 0) {
      sleep(2);
    }
  }

  CollectionStats stats;
  ret = collection->get_stats(&stats);
  ASSERT_EQ(ret, 0);
  ASSERT_EQ(stats.total_doc_count, 2000);
  ASSERT_EQ(stats.delete_doc_count, 0);
  ASSERT_EQ(stats.total_segment_count, 3);
  ASSERT_EQ(stats.total_index_file_count, 8);
  ASSERT_GT(stats.total_index_file_size, 2000U);
  ASSERT_EQ(stats.segment_stats.size(), 3);

  ASSERT_EQ(stats.segment_stats[0].segment_id, 0);
  ASSERT_EQ(stats.segment_stats[0].state, SegmentState::PERSIST);
  ASSERT_EQ(stats.segment_stats[0].doc_count, 900);
  ASSERT_EQ(stats.segment_stats[0].min_doc_id, 0);
  ASSERT_EQ(stats.segment_stats[0].max_doc_id, 899);
  ASSERT_EQ(stats.segment_stats[0].min_primary_key, 0);
  ASSERT_EQ(stats.segment_stats[0].max_primary_key, 899);
  ASSERT_EQ(stats.segment_stats[0].min_lsn, 0);
  ASSERT_EQ(stats.segment_stats[0].max_lsn, 899);
  ASSERT_EQ(stats.segment_stats[0].index_file_count, 1);
  ASSERT_GT(stats.segment_stats[0].index_file_size, 0);

  ASSERT_EQ(stats.segment_stats[1].segment_id, 1);
  ASSERT_EQ(stats.segment_stats[1].state, SegmentState::PERSIST);
  ASSERT_EQ(stats.segment_stats[1].doc_count, 900);
  ASSERT_EQ(stats.segment_stats[1].min_doc_id, 1899);
  ASSERT_EQ(stats.segment_stats[1].max_doc_id, 2798);
  ASSERT_EQ(stats.segment_stats[1].min_primary_key, 900);
  ASSERT_EQ(stats.segment_stats[1].max_primary_key, 1799);
  ASSERT_EQ(stats.segment_stats[1].min_lsn, 900);
  ASSERT_EQ(stats.segment_stats[1].max_lsn, 1799);
  ASSERT_EQ(stats.segment_stats[1].index_file_count, 1);
  ASSERT_GT(stats.segment_stats[1].index_file_size, 0);

  ASSERT_EQ(stats.segment_stats[2].segment_id, 2);
  ASSERT_EQ(stats.segment_stats[2].state, SegmentState::WRITING);
  ASSERT_EQ(stats.segment_stats[2].doc_count, 200);
  ASSERT_EQ(stats.segment_stats[2].min_doc_id, 3798);
  ASSERT_EQ(stats.segment_stats[2].max_doc_id, 3997);
  ASSERT_EQ(stats.segment_stats[2].min_primary_key, 1800);
  ASSERT_EQ(stats.segment_stats[2].max_primary_key, 1999);
  ASSERT_EQ(stats.segment_stats[2].min_lsn, 1800);
  ASSERT_EQ(stats.segment_stats[2].max_lsn, 1999);
  ASSERT_EQ(stats.segment_stats[2].index_file_count, 2);
  ASSERT_GT(stats.segment_stats[2].index_file_size, 0);

  std::vector<SegmentPtr> segments;
  ret = collection->get_segments(&segments);
  ASSERT_EQ(ret, 0);
  ASSERT_EQ(segments.size(), 3);

  for (size_t i = 0; i < 2000; i++) {
    std::vector<float> fvec(16U);
    for (size_t j = 0; j < 16U; j++) {
      fvec[j] = i * 1.0f;
    }
    std::string query((char *)fvec.data(), fvec.size() * sizeof(float));
    QueryParams query_params;
    query_params.topk = 10;
    query_params.data_type = DataTypes::VECTOR_FP32;
    query_params.dimension = 16;

    QueryResultList all_result;
    for (size_t j = 0; j < segments.size(); j++) {
      QueryResultList result_list;
      ret = segments[j]->knn_search("face", query, query_params, &result_list);
      ASSERT_EQ(ret, 0);
      all_result.insert(all_result.end(), result_list.begin(),
                        result_list.end());
    }
    std::sort(all_result.begin(), all_result.end());
    ASSERT_EQ(all_result[0].primary_key, i);
    ASSERT_EQ(all_result[0].score, 0.0f);
    ASSERT_EQ(all_result[0].lsn, i);
  }
}

TEST_F(CollectionTest, TestDeleteRecord) {
  index::ThreadPool thread_pool(10, false);
  CollectionPtr collection =
      Collection::Create(schema_->name(), "./", schema_, 10, &thread_pool);
  ASSERT_NE(collection, nullptr);

  ReadOptions read_options;
  read_options.use_mmap = true;
  read_options.create_new = true;
  int ret = collection->open(read_options);
  ASSERT_EQ(ret, 0);

  // insert 1000 records
  for (size_t i = 0; i < 1000; i++) {
    CollectionDatasetPtr add_records = std::make_shared<CollectionDataset>(1);
    CollectionDataset::RowData *new_row = add_records->add_row_data();
    new_row->primary_key = i;
    new_row->operation_type = OperationTypes::INSERT;
    new_row->lsn = i;
    new_row->forward_data = "hello";

    CollectionDataset::ColumnData new_column;
    new_column.column_name = "face";
    new_column.data_type = DataTypes::VECTOR_FP32;
    new_column.dimension = 16;

    std::vector<float> fvec(16U);
    for (size_t j = 0; j < 16U; j++) {
      fvec[j] = i * 1.0f;
    }

    std::string vector((char *)fvec.data(), fvec.size() * sizeof(float));
    new_column.data = vector;

    new_row->column_datas.emplace_back(new_column);
    ret = collection->write_records(*add_records);
    ASSERT_EQ(ret, 0);
  }

  std::vector<SegmentPtr> segments;
  ret = collection->get_segments(&segments);
  ASSERT_EQ(ret, 0);
  ASSERT_EQ(segments.size(), 1);

  // search front 500 records
  for (size_t i = 0; i < 500; i++) {
    std::vector<float> fvec(16U);
    for (size_t j = 0; j < 16U; j++) {
      fvec[j] = i * 1.0f;
    }
    std::string query((char *)fvec.data(), fvec.size() * sizeof(float));
    QueryParams query_params;
    query_params.topk = 10;
    query_params.data_type = DataTypes::VECTOR_FP32;
    query_params.dimension = 16;

    QueryResultList result_list;
    ret = segments[0]->knn_search("face", query, query_params, &result_list);
    ASSERT_EQ(ret, 0);

    ASSERT_EQ(result_list[0].primary_key, i);
    ASSERT_EQ(result_list[0].score, 0.0f);
    ASSERT_EQ(result_list[0].lsn, i);
  }

  // delete front 500 records
  for (size_t i = 0; i < 500; i++) {
    CollectionDatasetPtr del_records = std::make_shared<CollectionDataset>(1);
    CollectionDataset::RowData *new_row = del_records->add_row_data();
    new_row->primary_key = i;
    new_row->operation_type = OperationTypes::DELETE;

    ret = collection->write_records(*del_records);
    ASSERT_EQ(ret, 0);
  }

  // search front 500 records again
  for (size_t i = 0; i < 500; i++) {
    std::vector<float> fvec(16U);
    for (size_t j = 0; j < 16U; j++) {
      fvec[j] = i * 1.0f;
    }
    std::string query((char *)fvec.data(), fvec.size() * sizeof(float));
    QueryParams query_params;
    query_params.topk = 10;
    query_params.data_type = DataTypes::VECTOR_FP32;
    query_params.dimension = 16;

    QueryResultList result_list;
    ret = segments[0]->knn_search("face", query, query_params, &result_list);
    ASSERT_EQ(ret, 0);

    ASSERT_NE(result_list[0].primary_key, i);
    ASSERT_NE(result_list[0].score, 0.0f);
    ASSERT_NE(result_list[0].lsn, i);
  }
}


TEST_F(CollectionTest, TestUpdateRecord) {
  index::ThreadPool thread_pool(10, false);
  CollectionPtr collection =
      Collection::Create(schema_->name(), "./", schema_, 10, &thread_pool);
  ASSERT_NE(collection, nullptr);

  ReadOptions read_options;
  read_options.use_mmap = true;
  read_options.create_new = true;
  int ret = collection->open(read_options);
  ASSERT_EQ(ret, 0);

  // insert 1000 records
  for (size_t i = 0; i < 1000; i++) {
    CollectionDatasetPtr add_records = std::make_shared<CollectionDataset>(1);
    CollectionDataset::RowData *new_row = add_records->add_row_data();
    new_row->primary_key = i;
    new_row->operation_type = OperationTypes::INSERT;
    new_row->lsn = i;
    new_row->lsn_check = true;
    new_row->forward_data = "hello";

    CollectionDataset::ColumnData new_column;
    new_column.column_name = "face";
    new_column.data_type = DataTypes::VECTOR_FP32;
    new_column.dimension = 16;

    std::vector<float> fvec(16U);
    for (size_t j = 0; j < 16U; j++) {
      fvec[j] = i * 1.0f;
    }

    std::string vector((char *)fvec.data(), fvec.size() * sizeof(float));
    new_column.data = vector;

    new_row->column_datas.emplace_back(new_column);
    ret = collection->write_records(*add_records);
    ASSERT_EQ(ret, 0);
  }

  uint64_t lsn;
  std::string lsn_context;
  ret = collection->get_latest_lsn(&lsn, &lsn_context);
  ASSERT_EQ(ret, 0);
  ASSERT_EQ(lsn, 999);

  // update 1000 wrong records
  for (size_t i = 0; i < 1000; i++) {
    CollectionDatasetPtr update_records =
        std::make_shared<CollectionDataset>(1);

    CollectionDataset::RowData *new_row = update_records->add_row_data();
    new_row->primary_key = i;
    new_row->operation_type = OperationTypes::UPDATE;
    new_row->lsn = i;
    new_row->lsn_check = true;
    new_row->forward_data = "hello_update";

    CollectionDataset::ColumnData new_column;
    new_column.column_name = "face";
    new_column.data_type = DataTypes::VECTOR_FP32;
    new_column.dimension = 16;

    std::vector<float> fvec(16U);
    for (size_t j = 0; j < 16U; j++) {
      fvec[j] = i * 1.0f;
    }

    std::string vector((char *)fvec.data(), fvec.size() * sizeof(float));
    new_column.data = vector;

    new_row->column_datas.emplace_back(new_column);
    ret = collection->write_records(*update_records);
    ASSERT_NE(ret, 0);
  }

  // update 1000 right records
  for (size_t i = 0; i < 1000; i++) {
    CollectionDatasetPtr update_records =
        std::make_shared<CollectionDataset>(1);

    CollectionDataset::RowData *new_row = update_records->add_row_data();
    new_row->primary_key = i;
    new_row->operation_type = OperationTypes::UPDATE;
    new_row->lsn = i + 1;
    new_row->lsn_check = true;
    new_row->forward_data = "hello_update";

    CollectionDataset::ColumnData new_column;
    new_column.column_name = "face";
    new_column.data_type = DataTypes::VECTOR_FP32;
    new_column.dimension = 16;

    std::vector<float> fvec(16U);
    for (size_t j = 0; j < 16U; j++) {
      fvec[j] = i * 1.0f;
    }

    std::string vector((char *)fvec.data(), fvec.size() * sizeof(float));
    new_column.data = vector;

    new_row->column_datas.emplace_back(new_column);
    ret = collection->write_records(*update_records);
    ASSERT_EQ(ret, 0);
  }

  ret = collection->get_latest_lsn(&lsn, &lsn_context);
  ASSERT_EQ(ret, 0);
  ASSERT_EQ(lsn, 1000);

  // search 1000 records
  std::vector<SegmentPtr> segments;
  ret = collection->get_segments(&segments);
  ASSERT_EQ(ret, 0);
  ASSERT_EQ(segments.size(), 1);

  for (size_t i = 0; i < 1000; i++) {
    std::vector<float> fvec(16U);
    for (size_t j = 0; j < 16U; j++) {
      fvec[j] = i * 1.0f;
    }
    std::string query((char *)fvec.data(), fvec.size() * sizeof(float));
    QueryParams query_params;
    query_params.topk = 10;
    query_params.data_type = DataTypes::VECTOR_FP32;
    query_params.dimension = 16;

    QueryResultList result_list;
    ret = segments[0]->knn_search("face", query, query_params, &result_list);
    ASSERT_EQ(ret, 0);

    ASSERT_EQ(result_list[0].primary_key, i);
    ASSERT_EQ(result_list[0].score, 0.0f);
    ASSERT_EQ(result_list[0].lsn, i + 1);
    ASSERT_EQ(result_list[0].forward_data, "hello_update");
  }
}

void do_insert_record(Collection *collection, size_t number) {
  CollectionDatasetPtr add_records = std::make_shared<CollectionDataset>(1);
  CollectionDataset::RowData *new_row = add_records->add_row_data();
  new_row->primary_key = number;
  new_row->operation_type = OperationTypes::INSERT;
  new_row->lsn = number;
  new_row->forward_data = "hello";

  CollectionDataset::ColumnData new_column;
  new_column.column_name = "face";
  new_column.data_type = DataTypes::VECTOR_FP32;
  new_column.dimension = 16;

  std::vector<float> fvec(16U);
  for (size_t j = 0; j < 16U; j++) {
    fvec[j] = number * 1.0f;
  }

  std::string vector((char *)fvec.data(), fvec.size() * sizeof(float));
  new_column.data = vector;

  new_row->column_datas.emplace_back(new_column);
  int ret = collection->write_records(*add_records);
  ASSERT_EQ(ret, 0);
}

void do_search_record(Collection *collection, size_t number,
                      bool expect_found) {
  std::vector<SegmentPtr> segments;
  int ret = collection->get_segments(&segments);
  ASSERT_EQ(ret, 0);
  ASSERT_EQ(segments.size(), 1);

  std::vector<float> fvec(16U);
  for (size_t j = 0; j < 16U; j++) {
    fvec[j] = number * 1.0f;
  }
  std::string query((char *)fvec.data(), fvec.size() * sizeof(float));
  QueryParams query_params;
  query_params.topk = 10;
  query_params.data_type = DataTypes::VECTOR_FP32;
  query_params.dimension = 16;
  query_params.query_id = number;

  QueryResultList result_list;
  ret = segments[0]->knn_search("face", query, query_params, &result_list);
  ASSERT_EQ(ret, 0);

  if (expect_found) {
    ASSERT_EQ(result_list[0].primary_key, number);
    ASSERT_EQ(result_list[0].score, 0.0f);
    ASSERT_EQ(result_list[0].lsn, number);
    ASSERT_EQ(result_list[0].forward_data, "hello");
  } else {
    ASSERT_NE(result_list[0].primary_key, number);
    ASSERT_NE(result_list[0].score, 0.0f);
  }
}

void do_delete_record(Collection *collection, size_t number) {
  CollectionDatasetPtr delete_record = std::make_shared<CollectionDataset>(1);
  CollectionDataset::RowData *new_row = delete_record->add_row_data();
  new_row->primary_key = number;
  new_row->operation_type = OperationTypes::DELETE;
  new_row->lsn = number + 1;
  int ret = collection->write_records(*delete_record);
  ASSERT_EQ(ret, 0);
}

void do_update_record(Collection *collection, size_t number) {
  CollectionDatasetPtr add_records = std::make_shared<CollectionDataset>(1);
  CollectionDataset::RowData *new_row = add_records->add_row_data();
  new_row->primary_key = number;
  new_row->operation_type = OperationTypes::UPDATE;
  new_row->lsn = number + 1;
  new_row->forward_data = "hello_update";

  CollectionDataset::ColumnData new_column;
  new_column.column_name = "face";
  new_column.data_type = DataTypes::VECTOR_FP32;
  new_column.dimension = 16;

  std::vector<float> fvec(16U);
  for (size_t j = 0; j < 16U; j++) {
    fvec[j] = number * 1.0f;
  }

  std::string vector((char *)fvec.data(), fvec.size() * sizeof(float));
  new_column.data = vector;

  new_row->column_datas.emplace_back(new_column);
  int ret = collection->write_records(*add_records);
  ASSERT_EQ(ret, 0);
}

void do_hybrid_ops(Collection *collection, size_t number) {
  do_insert_record(collection, number);
  do_search_record(collection, number, true);

  do_delete_record(collection, number);
  do_search_record(collection, number, false);
}

TEST_F(CollectionTest, TestMultiThread) {
  index::ThreadPool thread_pool(10, false);
  CollectionPtr collection =
      Collection::Create(schema_->name(), "./", schema_, 10, &thread_pool);
  ASSERT_NE(collection, nullptr);

  ReadOptions read_options;
  read_options.use_mmap = true;
  read_options.create_new = true;
  int ret = collection->open(read_options);
  ASSERT_EQ(ret, 0);

  // test multithread insert records
  auto group = thread_pool.make_group();
  for (size_t i = 0; i < 1000; i++) {
    group->submit(ailego::Closure::New(&do_insert_record, collection.get(), i));
  }
  group->wait_finish();

  // test multithread search records
  for (size_t i = 0; i < 1000; i++) {
    group->submit(
        ailego::Closure::New(&do_search_record, collection.get(), i, true));
  }
  group->wait_finish();

  // test multithread delete records
  for (size_t i = 0; i < 500; i++) {
    group->submit(ailego::Closure::New(&do_delete_record, collection.get(), i));
  }
  group->wait_finish();

  // test multithread update records
  for (size_t i = 500; i < 1000; i++) {
    group->submit(ailego::Closure::New(&do_update_record, collection.get(), i));
  }
  group->wait_finish();

  // test multithread hybrid operations
  for (size_t i = 1000; i < 2000; i++) {
    group->submit(ailego::Closure::New(&do_hybrid_ops, collection.get(), i));
  }
  group->wait_finish();
}

TEST_F(CollectionTest, TestUpdateSchema) {
  index::ThreadPool thread_pool(10, false);
  CollectionPtr collection =
      Collection::Create(schema_->name(), "./", schema_, 10, &thread_pool);
  ASSERT_NE(collection, nullptr);

  ReadOptions read_options;
  read_options.use_mmap = true;
  read_options.create_new = true;
  int ret = collection->open(read_options);
  ASSERT_EQ(ret, 0);

  schema_->set_max_docs_per_segment(900);

  // create 2 persist segment and 1 memory segment
  for (size_t i = 0; i < 2000; i++) {
    CollectionDatasetPtr add_records = std::make_shared<CollectionDataset>(1);
    CollectionDataset::RowData *new_row = add_records->add_row_data();
    new_row->primary_key = i;
    new_row->operation_type = OperationTypes::INSERT;
    new_row->lsn = i;
    new_row->forward_data = "hello";

    CollectionDataset::ColumnData new_column;
    new_column.column_name = "face";
    new_column.data_type = DataTypes::VECTOR_FP32;
    new_column.dimension = 16;

    std::vector<float> fvec(16U);
    for (size_t j = 0; j < 16U; j++) {
      fvec[j] = i * 1.0f;
    }
    std::string vector((char *)fvec.data(), fvec.size() * sizeof(float));
    new_column.data = vector;

    new_row->column_datas.emplace_back(new_column);
    ret = collection->write_records(*add_records);
    ASSERT_EQ(ret, 0);
  }
  sleep(3);

  auto new_schema = std::make_shared<meta::CollectionMeta>();
  meta::ColumnMetaPtr column_meta = std::make_shared<meta::ColumnMeta>();
  column_meta->set_name("face1");
  column_meta->set_index_type(IndexTypes::PROXIMA_GRAPH_INDEX);
  column_meta->set_data_type(DataTypes::VECTOR_FP32);
  column_meta->set_dimension(16);
  column_meta->mutable_parameters()->set("metric_type", "SquaredEuclidean");
  new_schema->append(column_meta);
  new_schema->set_name("teachers");
  new_schema->set_revision(0);

  ret = collection->update_schema(new_schema);
  ASSERT_NE(ret, 0);

  new_schema->set_revision(1);
  ret = collection->update_schema(new_schema);
  ASSERT_EQ(ret, 0);
}
