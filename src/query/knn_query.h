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

#pragma once

#include <functional>  // for std::reference_wrapper
#include "collection_query.h"
#include "knn_task.h"

namespace proxima {
namespace be {
namespace query {

/*!
 * KNNQuery: handler of knn query
 */
class KNNQuery : public CollectionQuery, public KNNQueryContext {
 public:
  // Alias for reference to Result
  using ResultRef = std::reference_wrapper<const index::QueryResult>;

  // Alias for result reference list, which used to merge result and sort
  using ResultRefList = std::vector<ResultRef>;

  // Comparator for ResultReference
  struct ResultRefCompare {
    bool operator()(const ResultRef &__x, const ResultRef &__y) const {
      return __x.get() < __y.get();
    }
  };

  // Heap of ResultReference
  using ResultRefHeap = ailego::Heap<ResultRef, ResultRefCompare>;

 public:
  //! Constructor
  KNNQuery(uint64_t traceID, const proto::QueryRequest *req,
           index::IndexServicePtr index, MetaWrapperPtr meta_wrapper,
           ExecutorPtr executor_ptr, ProfilerPtr profiler_ptr,
           proto::QueryResponse *resp);

  //! Destructor
  ~KNNQuery() override;

 public:
  //! Validate query object, 0 for valid, otherwise non zero returned
  int validate() const override;

  //! Retrieve IOMode of query
  IOMode mode() const override;

  //! Retrieve the type of query, Readonly
  QueryType type() const override;

  //! Prepare resources, 0 for success, otherwise failed
  int prepare() override;

  //! Evaluate query, and collection feedback
  int evaluate() override;

  //! Finalize query object
  int finalize() override;

  //! Retrieve column name
  const std::string &column() const override;

  //! Retrieve features field
  const std::string &features() const override;

  //! Retrieve batch_count field
  uint32_t batch_count() const override;

  //! Retrieve QueryParams
  const index::QueryParams &query_params() const override;

 private:
  //! Build query param from PB proto
  int build_query_param(const proto::QueryRequest::KnnQueryParam &);

  //! Collect result
  int collect_result();

  //! Feed entity field
  uint32_t feed_entity(const ResultRefList &, proto::QueryResponse::Result *);

  //! transform query features if necessary
  int transform_feature(const proto::QueryRequest::KnnQueryParam &);

 private:
  //! QueryParams handler
  index::QueryParams query_param_{};

  //! KNNTasks, which scheduled by executor
  KNNTaskPtrList tasks_{};

  //! Query features
  std::string features_{};
};


}  // namespace query
}  // namespace be
}  // namespace proxima
