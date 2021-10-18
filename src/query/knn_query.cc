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
 *
 *   \author   guonix
 *   \date     Nov 2020
 *   \brief
 */

#include "knn_query.h"
#include "common/error_code.h"
#include "common/logger.h"
#include "common/transformer.h"
#include "common/types_helper.h"

namespace proxima {
namespace be {
namespace query {

//! Constructor
KNNQuery::KNNQuery(uint64_t traceID, const proto::QueryRequest *req,
                   index::IndexServicePtr index, MetaWrapperPtr meta_wrapper,
                   ExecutorPtr executor_ptr, ProfilerPtr profiler_ptr,
                   proto::QueryResponse *resp)
    : CollectionQuery(traceID, req, std::move(index), std::move(meta_wrapper),
                      std::move(executor_ptr), std::move(profiler_ptr), resp) {}

//! Destructor
KNNQuery::~KNNQuery() = default;

//! Validate query object, 0 for valid, otherwise non zero returned
int KNNQuery::validate() const {
  ScopedLatency latency("validate", profiler());
  int code = CollectionQuery::validate();
  if (code == 0) {
    if (valid_response() && valid_executor()) {
      code = meta()->validate_column(collection(), column());
    } else {
      LOG_WARN("Invalid response or executor passed to KNNQuery");
      code = PROXIMA_BE_ERROR_CODE(InvalidArgument);
    }
  }
  return code;
}

//! Retrieve IOMode of query
IOMode KNNQuery::mode() const {
  return IOMode::READONLY;
}

//! Retrieve the type of query, Readonly
QueryType KNNQuery::type() const {
  return QueryType::KNN;
}

//! Prepare resources, 0 for success, otherwise failed
int KNNQuery::prepare() {
  ScopedLatency latency("prepare", profiler());
  index::SegmentPtrList segments;
  int code = list_segments(&segments);
  if (code != 0) {
    return code;
  }

  for (auto &segment : segments) {
    std::string knn_name("knn_task_");
    knn_name.append(std::to_string(segment->segment_id()));
    knn_name.append("_");
    knn_name.append(std::to_string(id()));
    tasks_.emplace_back(std::make_shared<KNNTask>(knn_name, segment, this));
  }
  code = build_query_param(request()->knn_param());
  if (code != 0) {
    LOG_ERROR("Failed build query param from request");
    return code;
  }

  code = transform_feature(request()->knn_param());
  if (code != 0) {
    LOG_ERROR("Failed transform features. code[%d] what[%s]", code,
              ErrorCode::What(code));
  }
  return code;
}

//! Evaluate query, and collection feedback
int KNNQuery::evaluate() {
  profiler()->open_stage("evaluate");
  TaskPtrList tasks(tasks_.begin(), tasks_.end());
  int code;
  {
    ScopedLatency execute_latency("execute", profiler());
    code = executor()->execute_tasks(tasks);
  }
  if (code == 0) {
    ScopedLatency merge_and_sort_latency("merge_and_sort", profiler());
    // Merge Result
    code = collect_result();
  }
  profiler()->close_stage();
  return code;
}

//! Finalize query object
int KNNQuery::finalize() {
  return 0;
}

namespace {

static int CollectBatchResult(const KNNTaskPtrList &tasks, uint32_t batch,
                              KNNQuery::ResultRefHeap *results) {
  for (auto &task : tasks) {
    if (static_cast<size_t>(batch) < task->result().size()) {
      for (const auto &iter : task->result()[batch]) {
        results->push(iter);
        // Optimization: skip remained result, which more lower than last one
        // in target heap
        if (results->begin()->get() < iter) {
          break;
        }
      }
    } else {
      return PROXIMA_BE_ERROR_CODE(OutOfBoundsResult);
    }
  }
  return 0;
}

}  // namespace

int KNNQuery::collect_result() {
  if (tasks_.empty()) {
    return 0;
  }

  int code = 0;
  for (uint32_t batch = 0; batch < batch_count(); batch++) {
    ResultRefHeap results;
    results.limit(query_param_.topk);
    // Gather all the reference into list
    code = CollectBatchResult(tasks_, batch, &results);
    if (code == 0) {
      // Transform heap to sorted vector
      results.sort();
      // Feed entity field
      if (feed_entity(results, mutable_response()->add_results()) !=
          query_param_.topk) {
        LOG_DEBUG("No enough results to fill response");
      }
    } else {
      LOG_ERROR("Collect result failed");
      break;
    }
  }

  return code;
}

//! Retrieve column name
const std::string &KNNQuery::column() const {
  return request()->knn_param().column_name();
}

const std::string &KNNQuery::features() const {
  return features_;
}

uint32_t KNNQuery::batch_count() const {
  return request()->knn_param().batch_count();
}

const index::QueryParams &KNNQuery::query_params() const {
  return query_param_;
}

int KNNQuery::build_query_param(
    const proto::QueryRequest::KnnQueryParam &param) {
  query_param_.query_id = id();
  query_param_.topk = param.topk();
  query_param_.data_type = be::DataTypeCodeBook::Get(param.data_type());
  query_param_.dimension = param.dimension();
  query_param_.radius = param.radius();
  query_param_.is_linear = param.is_linear();
  be::IndexParamsHelper::SerializeToParams(param.extra_params(),
                                           &query_param_.extra_params);
  return 0;
}

uint32_t KNNQuery::feed_entity(const ResultRefList &results,
                               proto::QueryResponse::Result *result) {
  for (const auto &iter : results) {
    proto::Document *doc = result->add_documents();
    doc->set_primary_key(iter.get().primary_key);
    doc->set_score(iter.get().score);
    // Fill forward for document
    fill_forward(iter.get(), doc);
  }
  return results.size();
}

int KNNQuery::transform_feature(
    const proto::QueryRequest::KnnQueryParam &param) {
  int code = PROXIMA_BE_ERROR_CODE(InvalidQuery);
  auto data_type = meta()->get_data_type(collection(), column());
  auto value_case = param.features_value_case();
  if (value_case == proto::QueryRequest_KnnQueryParam::kFeatures) {
    code = Transformer::Transform(query_param_.data_type, param.features(),
                                  data_type, &features_);
  } else if (value_case == proto::QueryRequest_KnnQueryParam::kMatrix) {
    std::function<int(const ailego::JsonValue &)> validator =
        [this](const ailego::JsonValue &node) -> int {
      int ret = PROXIMA_BE_ERROR_CODE(InvalidVectorFormat);
      if (node.is_array()) {
        auto &array = node.as_array();
        if (!array.empty()) {
          if (array.begin()->is_array()) {
            ret = 0;
            for (auto it = array.begin(); it != array.end(); ++it) {
              if (!it->is_array() ||
                  it->as_array().size() != query_param_.dimension) {
                ret |= PROXIMA_BE_ERROR_CODE(InvalidVectorFormat);
                break;
              }
            }
          } else if (array.size() == query_param_.dimension) {
            ret = 0;
          }
        }
      }
      return ret;
    };
    switch (data_type) {
      case DataTypes::VECTOR_FP32: {
        std::vector<float> values;
        Transformer::Transform(param.matrix(), &validator, &values);
        Primary2Bytes::Bytes<float, DataTypes::VECTOR_FP32>(values, &features_);
        code = 0;
        break;
      }

      case DataTypes::VECTOR_FP16: {
        std::vector<float> values;
        Transformer::Transform(param.matrix(), &validator, &values);
        Primary2Bytes::Bytes<float, DataTypes::VECTOR_FP16>(values, &features_);
        code = 0;
        break;
      }
      case DataTypes::VECTOR_INT16: {
        std::vector<int16_t> values;
        Transformer::Transform(param.matrix(), &validator, &values);
        Primary2Bytes::Bytes<int16_t, DataTypes::VECTOR_INT16>(values,
                                                               &features_);
        code = 0;
        break;
      }
      case DataTypes::VECTOR_INT8: {
        std::vector<int8_t> values;
        Transformer::Transform(param.matrix(), &validator, &values);
        Primary2Bytes::Bytes<int8_t, DataTypes::VECTOR_INT8>(values,
                                                             &features_);
        code = 0;
        break;
      }
      case DataTypes::VECTOR_INT4: {
        std::vector<int8_t> values;
        Transformer::Transform(param.matrix(), &validator, &values);
        Primary2Bytes::Bytes<int8_t, DataTypes::VECTOR_INT4>(values,
                                                             &features_);
        code = 0;
        break;
      }
      case DataTypes::VECTOR_BINARY32: {
        std::vector<uint32_t> values;
        Transformer::Transform(param.matrix(), &validator, &values);
        Primary2Bytes::Bytes<uint32_t, DataTypes::VECTOR_BINARY32>(values,
                                                                   &features_);
        code = 0;
        break;
      }
      case DataTypes::VECTOR_BINARY64: {
        std::vector<uint64_t> values;
        Transformer::Transform(param.matrix(), &validator, &values);
        Primary2Bytes::Bytes<uint64_t, DataTypes::VECTOR_BINARY64>(values,
                                                                   &features_);
        code = 0;
        break;
      }
      default:
        LOG_ERROR("Unsupported data type %u.", (uint32_t)data_type);
        code = PROXIMA_BE_ERROR_CODE(InvalidDataType);
    }
  }
  return code;
}

}  // namespace query
}  // namespace be
}  // namespace proxima
