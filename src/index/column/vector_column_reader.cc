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
 *   \brief    Implementation of vector column reader.
 */

#include "vector_column_reader.h"
#include "common/defer.h"
#include "common/error_code.h"
#include "constants.h"
#include "typedef.h"

namespace proxima {
namespace be {
namespace index {

VectorColumnReader::~VectorColumnReader() {
  // TODO can't call virtual function
  if (opened_) {
    close();
  }
}

int VectorColumnReader::open(const meta::ColumnMeta &column_meta,
                             const ReadOptions &read_options) {
  CHECK_STATUS(opened_, false);

  if (!check_column_meta(column_meta)) {
    LLOG_ERROR("Check column meta failed.");
    return ErrorCode_ConfigError;
  }

  int ret = open_proxima_container(read_options);
  CHECK_RETURN_WITH_LOG(ret, 0, "Open proxima container failed.");

  ret = open_proxima_searcher();
  CHECK_RETURN_WITH_LOG(ret, 0, "Open proxima searcher failed.");

  opened_ = true;
  LLOG_DEBUG("Opened column searcher.");
  return 0;
}

int VectorColumnReader::close() {
  context_pool_.clear();
  proxima_searcher_->unload();
  proxima_searcher_->cleanup();

  opened_ = false;
  LLOG_DEBUG("Unloaded column searcher");

  return 0;
}

int VectorColumnReader::search(const std::string &query,
                               const QueryParams &query_params,
                               FilterFunction filter,
                               IndexDocumentList *results) {
  CHECK_STATUS(opened_, true);

  std::vector<IndexDocumentList> batch_results;
  int ret = this->search(query, query_params, 1, filter, &batch_results);
  (*results) = batch_results[0];
  return ret;
}


int VectorColumnReader::search(
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
      ret = proxima_searcher_->search_bf_impl(new_query.data(), new_meta,
                                              batch_count, ctx);
    } else {
      ret = proxima_searcher_->search_impl(new_query.data(), new_meta,
                                           batch_count, ctx);
    }
  } else {
    if (query_params.is_linear) {
      ret = proxima_searcher_->search_bf_impl(query.data(), query_meta,
                                              batch_count, ctx);
    } else {
      ret = proxima_searcher_->search_impl(query.data(), query_meta,
                                           batch_count, ctx);
    }
  }

  CHECK_RETURN_WITH_LLOG(ret, 0,
                         "Search proxima searcher failed. ret[%d] reason[%s]",
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

bool VectorColumnReader::check_column_meta(
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
  }

  auto ef_search = column_meta.parameters().get_as_uint32("ef_search");
  if (ef_search > 0U) {
    proxima_params_.set("proxima.hnsw.searcher.ef", ef_search);
  } else {
    proxima_params_.set("proxima.hnsw.searcher.ef", 200U);
  }

  auto max_scan_ratio = column_meta.parameters().get_as_float("max_scan_ratio");
  if (max_scan_ratio > 0.0f) {
    proxima_params_.set("proxima.hnsw.searcher.max_scan_ratio", max_scan_ratio);
  }

  auto visit_bf =
      column_meta.parameters().get_as_bool("visit_bloomfilter_enable");
  if (visit_bf) {
    proxima_params_.set("proxima.hnsw.searcher.visit_bloomfilter_enable",
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

  // Set proxima index meta
  proxima_meta_.set_meta(feature_type, dimension);
  proxima_meta_.set_measure(metric_type, 0, IndexParams());

  LLOG_INFO(
      "Show vector column searcher options. index_type[%u] data_type[%u] "
      "dimension[%u] measure[%s] context_count[%u] ef_search[%u] "
      "max_scan_ratio[%f] visit_bf[%d] quantize_type[%s] ",
      index_type, data_type, dimension, metric_type.c_str(),
      this->concurrency(), ef_search, max_scan_ratio, visit_bf,
      quantize_type.c_str());

  return true;
}

int VectorColumnReader::open_proxima_container(
    const ReadOptions &read_options) {
  index_file_path_ = FileHelper::MakeFilePath(
      this->collection_path(), FileID::SEGMENT_FILE, this->segment_id());

  if (read_options.use_mmap) {
    container_ = aitheta2::IndexFactory::CreateContainer("MMapFileContainer");
  } else {
    container_ = aitheta2::IndexFactory::CreateContainer("MemoryContainer");
  }

  // Default set warmup flag
  IndexParams container_params;
  container_params.set("proxima.mmap_file.container.memory_warmup", true);

  int ret = container_->init(container_params);
  CHECK_RETURN_WITH_LLOG(ret, 0, "Container init failed. ret[%d]", ret);

  ret = container_->load(index_file_path_);
  CHECK_RETURN_WITH_LLOG(ret, 0, "Container load failed. ret[%d] file[%s]", ret,
                         index_file_path_.c_str());

  return 0;
}

int VectorColumnReader::open_proxima_searcher() {
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

  // Init proxima searcher
  proxima_searcher_ = aitheta2::IndexFactory::CreateSearcher("HnswSearcher");
  if (!proxima_searcher_) {
    LLOG_ERROR("Create proxima searcher failed. name[HnswSearcher]");
    return ErrorCode_RuntimeError;
  }

  ret = proxima_searcher_->init(proxima_params_);
  CHECK_RETURN_WITH_LOG(ret, 0, "Init proxima searcher failed.");

  auto column_block = container_->get(COLUMN_DUMP_BLOCK + this->column_name());
  if (!column_block) {
    LLOG_INFO("Can't find column block in index file.");
    return ErrorCode_InvalidSegment;
  }
  auto block_container =
      std::make_shared<aitheta2::IndexSegmentContainer>(column_block);
  ret = block_container->load();
  CHECK_RETURN_WITH_LLOG(ret, 0, "Column block load failed.");

  ret = proxima_searcher_->load(block_container, nullptr);
  CHECK_RETURN_WITH_LLOG(ret, 0, "Load container failed.");

  // Init context pool
  for (uint32_t i = 0; i < this->concurrency(); i++) {
    auto ctx = proxima_searcher_->create_context();
    if (!ctx) {
      LLOG_ERROR("Create context for proxima searcher failed.");
      return ErrorCode_RuntimeError;
    }
    context_pool_.emplace(std::move(ctx));
  }

  return 0;
}


}  // end namespace index
}  // namespace be
}  // end namespace proxima
