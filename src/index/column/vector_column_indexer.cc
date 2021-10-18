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
 *   \brief    Implementation of column indexer
 */

#include "vector_column_indexer.h"
#include "common/defer.h"
#include "common/logger.h"
#include "../file_helper.h"

namespace proxima {
namespace be {
namespace index {

VectorColumnIndexer::~VectorColumnIndexer() {
  // TODO  can't call virtual function
  if (opened_) {
    this->close();
  }
}

int VectorColumnIndexer::open(const meta::ColumnMeta &column_meta,
                              const ReadOptions &read_options) {
  CHECK_STATUS(opened_, false);

  if (!check_column_meta(column_meta)) {
    LLOG_ERROR("Check column meta failed.");
    return ErrorCode_ConfigError;
  }

  // open snapshot
  int ret = Snapshot::CreateAndOpen(
      this->collection_path(), FileID::PROXIMA_FILE, this->segment_id(),
      this->column_name(), read_options, &snapshot_);
  CHECK_RETURN_WITH_LLOG(ret, 0, "Create and open snapshot failed. ret[%d]",
                         ret);

  // open proxima streamer
  ret = open_proxima_streamer();
  CHECK_RETURN_WITH_LLOG(ret, 0, "Open proxima streamer failed.");

  opened_ = true;
  return 0;
}

int VectorColumnIndexer::close() {
  CHECK_STATUS(opened_, true);

  context_pool_.clear();
  proxima_streamer_->cleanup();

  int ret = snapshot_->close();
  if (ret != 0) {
    LLOG_WARN("Close snapshot failed.");
  }

  opened_ = false;
  return ret;
}

int VectorColumnIndexer::flush() {
  CHECK_STATUS(opened_, true);

  return proxima_streamer_->flush(0);
}

int VectorColumnIndexer::dump(IndexDumperPtr dumper) {
  CHECK_STATUS(opened_, true);

  return proxima_streamer_->dump(dumper);
}

int VectorColumnIndexer::insert(idx_t doc_id, const ColumnData &column_data) {
  CHECK_STATUS(opened_, true);

  // Check column data if legal
  IndexQueryMeta query_meta;
  auto feature_type = IndexHelper::GetProximaFeatureType(column_data.data_type);
  auto dimension = column_data.dimension;
  if (feature_type != FeatureTypes::FT_UNDEFINED && dimension != 0) {
    query_meta.set_meta(feature_type, dimension);
  } else {
    query_meta.set_meta(proxima_meta_.type(), proxima_meta_.dimension());
  }

  if (query_meta.type() != proxima_meta_.type() ||
      query_meta.dimension() != proxima_meta_.dimension()) {
    LLOG_ERROR(
        "Invalid record, input query feature type or dimension not matched. "
        "query_feature_type[%d] query_dimension[%u] feature_type[%d] "
        "dimension[%u]",
        query_meta.type(), query_meta.dimension(), proxima_meta_.type(),
        proxima_meta_.dimension());
    return ErrorCode_InvalidRecord;
  }

  const std::string &vector = column_data.data;
  uint32_t expect_size = proxima_meta_.element_size();
  if (vector.size() != expect_size) {
    LLOG_ERROR(
        "Invalid record, vector size mismatch. expect_size[%u] "
        "actual_size[%zu]",
        expect_size, vector.size());
    return ErrorCode_InvalidRecord;
  }

  // Get context and set properties.
  // Notice we must return back to pool in the end.
  auto ctx = context_pool_.acquire();
  Defer defer([&ctx, this] { context_pool_.release(std::move(ctx)); });

  int ret = 0;
  // Check if need to use quantizer
  if (quantize_type_ != QuantizeTypes::UNDEFINED && reformer_ != nullptr) {
    std::string new_vector;
    IndexQueryMeta new_meta;
    ret = reformer_->convert(vector.data(), query_meta, &new_vector, &new_meta);
    CHECK_RETURN_WITH_LLOG(ret, 0, "Reformer transform data failed. ret[%d]",
                           ret);
    ret = proxima_streamer_->add_impl(doc_id, new_vector.data(), new_meta, ctx);
  } else {
    ret = proxima_streamer_->add_impl(doc_id, vector.data(), query_meta, ctx);
  }
  CHECK_RETURN_WITH_LLOG(ret, 0,
                         "Insert proxima streamer failed. ret[%d] reason[%s]",
                         ret, aitheta2::IndexError::What(ret));

  return 0;
}

#if 0
int VectorColumnIndexer::update(idx_t doc_id, const ColumnData &column_data) {
  CHECK_STATUS(opened_, true);

  // Check column data if legal
  IndexQueryMeta query_meta;
  auto feature_type = IndexHelper::GetProximaFeatureType(column_data.data_type);
  auto dimension = column_data.dimension;
  if (feature_type != FeatureTypes::FT_UNDEFINED && dimension != 0) {
    query_meta.set_meta(feature_type, dimension);
  } else {
    query_meta.set_meta(proxima_meta_.type(), proxima_meta_.dimension());
  }

  if (query_meta.type() != proxima_meta_.type() ||
      query_meta.dimension() != proxima_meta_.dimension()) {
    LLOG_ERROR(
        "Invalid record, input query feature type or dimension not matched. "
        "query_feature_type[%d] query_dimension[%u] feature_type[%d] "
        "dimension[%u]",
        query_meta.type(), query_meta.dimension(), proxima_meta_.type(),
        proxima_meta_.dimension());
    return ErrorCode_InvalidRecord;
  }

  const std::string &vector = column_data.data;
  uint32_t expect_size = proxima_meta_.element_size();
  if (vector.size() != expect_size) {
    LLOG_ERROR(
        "Invalid record, vector size mismatch. expect_size[%u] "
        "actual_size[%zu]",
        expect_size, vector.size());
    return ErrorCode_InvalidRecord;
  }

  // Get context and set properties.
  // Notice we must return back to pool in the end.
  auto ctx = context_pool_.acquire();
  Defer defer([&ctx, this] { context_pool_.release(std::move(ctx)); });

  int ret = 0;
  // Check if need to use quantizer
  if (quantize_type_ != QuantizeTypes::UNDEFINED && reformer_ != nullptr) {
    std::string new_vector;
    IndexQueryMeta new_meta;
    ret = reformer_->convert(vector.data(), query_meta, &new_vector, &new_meta);
    CHECK_RETURN_WITH_LLOG(ret, 0, "Reformer transform data failed. ret[%d]",
                           ret);
    ret = proxima_streamer_->update_impl(doc_id, new_vector.data(), new_meta, ctx);
  } else {
    ret = proxima_streamer_->update_impl(doc_id, vector.data(), query_meta, ctx);
  }
  CHECK_RETURN_WITH_LLOG(ret, 0,
                         "Update proxima streamer failed. ret[%d] reason[%s]",
                         ret, aitheta2::IndexError::What(ret));

  return 0;

}
#endif

int VectorColumnIndexer::remove(idx_t doc_id) {
  CHECK_STATUS(opened_, true);

  // HNSW do not need to do remove
  if (engine_type_ == EngineTypes::PROXIMA_OSWG_STREAMER) {
    auto ctx = context_pool_.acquire();
    Defer defer([&ctx, this] { context_pool_.release(std::move(ctx)); });

    int ret = proxima_streamer_->remove_impl(doc_id, ctx);
    CHECK_RETURN_WITH_LLOG(
        ret, 0, "Remove from proxima streamer failed. doc_id[%zu] ret[%d]",
        (size_t)doc_id, ret)
  }
  return 0;
}

int VectorColumnIndexer::optimize(ThreadPoolPtr pool) {
  CHECK_STATUS(opened_, true);

  // HNSW do not need to do optimize
  if (engine_type_ == EngineTypes::PROXIMA_OSWG_STREAMER) {
    ailego::ElapsedTime timer;
    int ret = proxima_streamer_->optimize_impl(ThreadPoolPtr(pool));
    CHECK_RETURN_WITH_LLOG(ret, 0, "Optimize column indexer failed. ret[%d]",
                           ret);

    LLOG_DEBUG("Optmize column indexer complete. cost[%zuus]",
               (size_t)timer.micro_seconds());
  }
  return 0;
}

int VectorColumnIndexer::search(const std::string &query,
                                const QueryParams &query_params,
                                FilterFunction filter,
                                IndexDocumentList *result_list) {
  CHECK_STATUS(opened_, true);

  std::vector<IndexDocumentList> batch_result_list;
  int ret = this->search(query, query_params, 1, filter, &batch_result_list);
  (*result_list) = batch_result_list[0];
  return ret;
}

int VectorColumnIndexer::search(
    const std::string &query, const QueryParams &query_params,
    uint32_t batch_count, FilterFunction filter,
    std::vector<IndexDocumentList> *batch_result_list) {
  CHECK_STATUS(opened_, true);

  // Check if query legal
  IndexQueryMeta query_meta;
  auto feature_type =
      IndexHelper::GetProximaFeatureType(query_params.data_type);
  auto dimension = query_params.dimension;
  if (feature_type != FeatureTypes::FT_UNDEFINED && dimension != 0) {
    query_meta.set_meta(feature_type, dimension);
  } else {
    query_meta.set_meta(proxima_meta_.type(), proxima_meta_.dimension());
  }

  if (query_meta.type() != proxima_meta_.type() ||
      query_meta.dimension() != proxima_meta_.dimension()) {
    LLOG_ERROR(
        "Invalid query, input query feature type or dimension not matched. "
        "query_feature_type[%d] query_dimension[%u] feature_type[%d] "
        "dimension[%u]",
        query_meta.type(), query_meta.type(), proxima_meta_.type(),
        proxima_meta_.dimension());
    return ErrorCode_InvalidQuery;
  }

  uint32_t expect_size = query_meta.element_size() * batch_count;
  if (query.size() != expect_size) {
    LLOG_ERROR(
        "Invalid query, query size mismatch. expect_size[%u] "
        "actual_size[%zu]",
        expect_size, query.size());
    return ErrorCode_InvalidQuery;
  }

  // Get context and set properties.
  // Notice that, we must reset the context
  // when return back into pool.
  auto ctx = context_pool_.acquire();
  ctx->set_topk(query_params.topk);
  if (filter != nullptr) {
    ctx->set_filter(filter);
  }

  // Notice oswg graph do not need to pass filter
  if (engine_type_ == EngineTypes::PROXIMA_OSWG_STREAMER) {
    ctx->set_filter(nullptr);
  }

  if (query_params.radius > 0.0f) {
    ctx->set_threshold(query_params.radius);
  }
  Defer defer([&ctx, this] {
    ctx->set_filter(nullptr);
    ctx->set_threshold(std::numeric_limits<float>::max());
    context_pool_.release(std::move(ctx));
  });

  int ret = 0;
  // Check if need to use quantizer
  if (quantize_type_ != QuantizeTypes::UNDEFINED && reformer_ != nullptr) {
    std::string new_query;
    IndexQueryMeta new_meta;
    ret = reformer_->transform(query.data(), query_meta, &new_query, &new_meta);
    CHECK_RETURN_WITH_LLOG(ret, 0, "Reformer transform data failed. ret[%d]",
                           ret);

    if (query_params.is_linear) {
      ret = proxima_streamer_->search_bf_impl(new_query.data(), new_meta,
                                              batch_count, ctx);
    } else {
      ret = proxima_streamer_->search_impl(new_query.data(), new_meta,
                                           batch_count, ctx);
    }
  } else {
    if (query_params.is_linear) {
      ret = proxima_streamer_->search_bf_impl(query.data(), query_meta,
                                              batch_count, ctx);
    } else {
      ret = proxima_streamer_->search_impl(query.data(), query_meta,
                                           batch_count, ctx);
    }
  }
  CHECK_RETURN_WITH_LLOG(ret, 0,
                         "Search proxima streamer failed. ret[%d] reason[%s]",
                         ret, aitheta2::IndexError::What(ret));

  for (uint32_t i = 0; i < batch_count; i++) {
    auto &result_list = ctx->result(i);
    if (measure_->support_normalize()) {
      for (auto &it : const_cast<IndexDocumentList &>(result_list)) {
        measure_->normalize(it.mutable_score());
      }
    }
    if (reformer_) {
      reformer_->normalize(query.data(), query_meta,
                           const_cast<IndexDocumentList &>(result_list));
    }
    batch_result_list->emplace_back(result_list);
  }

  return 0;
}

bool VectorColumnIndexer::check_column_meta(
    const meta::ColumnMeta &column_meta) {
  auto index_type = column_meta.index_type();
  if (index_type != IndexTypes::PROXIMA_GRAPH_INDEX) {
    LOG_ERROR("Column meta config error, only support PROXIMA_GRAPH_INDEX now");
    return false;
  }

  auto data_type = column_meta.data_type();
  auto feature_type = IndexHelper::GetProximaFeatureType(data_type);
  if (feature_type == FeatureTypes::FT_UNDEFINED) {
    LLOG_ERROR("Column meta config error, unknown data type.");
    return false;
  }

  auto dimension = column_meta.dimension();
  if (dimension == 0U) {
    LLOG_ERROR("Column meta config error, dimension can't be 0.");
    return false;
  }

  auto metric_type = column_meta.parameters().get_as_string("metric_type");
  if (metric_type.empty()) {
    metric_type = "SquaredEuclidean";
  } else if (metric_type == "InnerProduct") {
    metric_type = "MipsSquaredEuclidean";
  }

  auto max_neighbor_count =
      column_meta.parameters().get_as_uint32("max_neighbor_count");
  if (max_neighbor_count > 0U) {
    proxima_params_.set("proxima.hnsw.streamer.max_neighbor_count",
                        max_neighbor_count);
    proxima_params_.set("proxima.oswg.streamer.max_neighbor_count",
                        max_neighbor_count);
  }

  auto ef_construction =
      column_meta.parameters().get_as_uint32("ef_construction");
  if (ef_construction > 0U) {
    proxima_params_.set("proxima.hnsw.streamer.ef_construction",
                        ef_construction);
    proxima_params_.set("proxima.oswg.streamer.ef_construction",
                        ef_construction);
  }

  auto ef_search = column_meta.parameters().get_as_uint32("ef_search");
  if (ef_search > 0U) {
    proxima_params_.set("proxima.hnsw.streamer.ef", ef_search);
    proxima_params_.set("proxima.oswg.streamer.ef", ef_search);
  } else {
    proxima_params_.set("proxima.hnsw.streamer.ef", 200U);
    proxima_params_.set("proxima.oswg.streamer.ef", 200U);
  }

  auto chunk_size = column_meta.parameters().get_as_uint32("chunk_size");
  if (chunk_size > 0U) {
    proxima_params_.set("proxima.hnsw.streamer.chunk_size", chunk_size);
    proxima_params_.set("proxima.oswg.streamer.segment_size", chunk_size);
  } else {
    proxima_params_.set("proxima.hnsw.streamer.chunk_size",
                        64UL * 1024UL * 1024UL);
    proxima_params_.set("proxima.oswg.streamer.segment_size",
                        64UL * 1024UL * 1024UL);
  }

  auto max_scan_ratio = column_meta.parameters().get_as_float("max_scan_ratio");
  if (max_scan_ratio > 0.0f) {
    proxima_params_.set("proxima.hnsw.streamer.max_scan_ratio", max_scan_ratio);
    proxima_params_.set("proxima.oswg.streamer.max_scan_ratio", max_scan_ratio);
  }

  auto visit_bf =
      column_meta.parameters().get_as_bool("visit_bloomfilter_enable");
  if (visit_bf) {
    proxima_params_.set("proxima.hnsw.streamer.visit_bloomfilter_enable",
                        visit_bf);
    proxima_params_.set("proxima.oswg.streamer.visit_bloomfilter_enable",
                        visit_bf);
  }

  // Check quantize type
  auto quantize_type = column_meta.parameters().get_as_string("quantize_type");
  if (!quantize_type.empty()) {
    if (IndexHelper::GetQuantizeType(quantize_type) ==
        QuantizeTypes::UNDEFINED) {
      LLOG_ERROR(
          "Column meta config error, unknown quantize type. quantize_type[%s]",
          quantize_type.c_str());
      return false;
    }

    if (data_type != DataTypes::VECTOR_FP32) {
      LLOG_ERROR(
          "Column meta config error, only FP32 data type can open quantizer");
      return false;
    }

    quantize_type_ = IndexHelper::GetQuantizeType(quantize_type);
  }

  // Default filter duplicate records
  proxima_params_.set("proxima.hnsw.streamer.filter_same_key", true);

  // Set proxima index meta
  proxima_meta_.set_meta(feature_type, dimension);
  proxima_meta_.set_measure(metric_type, 0, IndexParams());

  // Decide which engine to use
  std::string engine = column_meta.parameters().get_as_string("engine");
  if (engine == "OSWG") {
    engine_type_ = EngineTypes::PROXIMA_OSWG_STREAMER;
  } else if (engine == "HNSW") {
    engine_type_ = EngineTypes::PROXIMA_HNSW_STREAMER;
  }

  LLOG_INFO(
      "Show vector column indexer options. index_type[%u] data_type[%u] "
      "dimension[%u] "
      "measure[%s] context_count[%u] max_neighbor_count[%u] "
      "ef_construction[%u] chunk_size[%u] ef_search[%u] max_scan_ratio[%f] "
      "visit_bf[%d] quantize_type[%s] engine_type[%d]",
      index_type, data_type, dimension, metric_type.c_str(),
      this->concurrency(), max_neighbor_count, ef_construction, chunk_size,
      ef_search, max_scan_ratio, visit_bf, quantize_type.c_str(), engine_type_);

  return true;
}

int VectorColumnIndexer::open_proxima_streamer() {
  int ret = 0;
  auto index_meta = proxima_meta_;
  // Check if need to open quantizer
  if (quantize_type_ != QuantizeTypes::UNDEFINED) {
    IndexConverterPtr converter;
    switch (quantize_type_) {
      case QuantizeTypes::VECTOR_INT4:
        converter =
            aitheta2::IndexFactory::CreateConverter("Int4StreamingConverter");
        break;
      case QuantizeTypes::VECTOR_INT8:
        converter =
            aitheta2::IndexFactory::CreateConverter("Int8StreamingConverter");
        break;
      case QuantizeTypes::VECTOR_FP16:
        converter =
            aitheta2::IndexFactory::CreateConverter("HalfFloatConverter");
        break;
      default:
        return ErrorCode_RuntimeError;
    }

    if (!converter) {
      LLOG_ERROR("Create converter failed.");
      return ErrorCode_RuntimeError;
    }

    ret = converter->init(proxima_meta_, IndexParams());
    CHECK_RETURN_WITH_LLOG(ret, 0, "Converter init failed. ret[%d]", ret);
    index_meta = converter->meta();

    reformer_ =
        aitheta2::IndexFactory::CreateReformer(index_meta.reformer_name());
    ret = reformer_->init(IndexParams());
    CHECK_RETURN_WITH_LLOG(ret, 0, "Reformer init failed. ret[%d]", ret);
  }

  // Init measure
  measure_ =
      aitheta2::IndexFactory::CreateMeasure(proxima_meta_.measure_name());
  if (!measure_) {
    LLOG_ERROR("Create measure %s failed",
               proxima_meta_.measure_name().c_str());
    return aitheta2::IndexError_Runtime;
  }
  ret = measure_->init(proxima_meta_, IndexParams());
  CHECK_RETURN_WITH_LLOG(ret, 0, "Reformer init failed. ret[%d]", ret);
  auto query_measure = measure_->query_measure();
  if (query_measure) {
    measure_ = query_measure;
  }

  // Get actual engine name and initialize it with factory
  std::string engine_name = this->get_engine_name();
  proxima_streamer_ = aitheta2::IndexFactory::CreateStreamer(engine_name);
  if (!proxima_streamer_) {
    LLOG_ERROR("Create proxima streamer failed. name[%s]", engine_name.c_str());
    return ErrorCode_RuntimeError;
  }

  //! Notice use new index meta as initialize params
  //! When user config quantize type, it may change its value.
  ret = proxima_streamer_->init(index_meta, proxima_params_);
  CHECK_RETURN_WITH_LLOG(ret, 0, "Init proxima streamer failed. ret[%d]", ret);

  ret = proxima_streamer_->open(snapshot_->data());
  CHECK_RETURN_WITH_LLOG(ret, 0, "Open proxima streamer failed. ret[%d]", ret);

  // Initialize context pool
  for (uint32_t i = 0; i < this->concurrency(); i++) {
    auto ctx = proxima_streamer_->create_context();
    if (!ctx) {
      LLOG_ERROR("Create proxima streamer context failed.");
      return ErrorCode_RuntimeError;
    }
    context_pool_.emplace(std::move(ctx));
  }

  return 0;
}


}  // end namespace index
}  // namespace be
}  // end namespace proxima
