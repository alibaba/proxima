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

 *   \author   Hechong.xyf
 *   \date     Oct 2019
 *   \brief    Interface of AiTheta Index Searcher
 */

#ifndef __AITHETA2_INDEX_SEARCHER_H__
#define __AITHETA2_INDEX_SEARCHER_H__

#include "index_container.h"
#include "index_context.h"
#include "index_measure.h"
#include "index_provider.h"
#include "index_stats.h"

namespace aitheta2 {

/*! Index Searcher
 */
class IndexSearcher : public IndexModule {
 public:
  //! Index Searcher Pointer
  typedef std::shared_ptr<IndexSearcher> Pointer;

  /*! Index Searcher Stats
   */
  class Stats : public IndexStats {
   public:
    //! Set count of documents loaded
    void set_loaded_count(size_t count) {
      loaded_count_ = count;
    }

    //! Set time cost of documents loaded
    void set_loaded_costtime(uint64_t cost) {
      loaded_costtime_ = cost;
    }

    //! Retrieve count of documents loaded
    size_t loaded_count(void) const {
      return loaded_count_;
    }

    //! Retrieve time cost of documents loaded
    uint64_t loaded_costtime(void) const {
      return loaded_costtime_;
    }

    //! Retrieve count of documents loaded (mutable)
    size_t *mutable_loaded_count(void) {
      return &loaded_count_;
    }

    //! Retrieve time cost of documents loaded (mutable)
    uint64_t *mutable_loaded_costtime(void) {
      return &loaded_costtime_;
    }

   private:
    //! Members
    size_t loaded_count_{0u};
    uint64_t loaded_costtime_{0u};
  };

  /*! Index Searcher Context
   */
  struct Context : public IndexContext {};

  /*! Index Searcher Provider
   */
  struct Provider : public IndexProvider {};

  //! Destructor
  virtual ~IndexSearcher(void) {}

  //! Initialize Searcher
  virtual int init(const IndexParams &params) = 0;

  //! Cleanup Searcher
  virtual int cleanup(void) = 0;

  //! Load index from container
  virtual int load(IndexContainer::Pointer cntr,
                   IndexMeasure::Pointer measure) = 0;

  //! Unload index
  virtual int unload(void) = 0;

  //! Similarity brute force search
  virtual int search_bf_impl(const void *query, const IndexQueryMeta &qmeta,
                             Context::Pointer &context) const = 0;

  //! Similarity search
  virtual int search_impl(const void *query, const IndexQueryMeta &qmeta,
                          Context::Pointer &context) const = 0;

  //! Similarity brute force search
  virtual int search_bf_impl(const void *query, const IndexQueryMeta &qmeta,
                             uint32_t count,
                             Context::Pointer &context) const = 0;

  //! Similarity search
  virtual int search_impl(const void *query, const IndexQueryMeta &qmeta,
                          uint32_t count, Context::Pointer &context) const = 0;

  //! Retrieve statistics
  virtual const Stats &stats(void) const = 0;

  //! Retrieve meta of index
  virtual const IndexMeta &meta(void) const = 0;

  //! Retrieve params of index
  virtual const IndexParams &params(void) const = 0;

  //! Create a searcher context
  virtual Context::Pointer create_context(void) const = 0;

  //! Create a searcher provider
  virtual Provider::Pointer create_provider(void) const {
    return Provider::Pointer();
  }

  //! Similarity search (FP16)
  template <IndexMeta::FeatureTypes FT,
            typename = typename std::enable_if<FT == IndexMeta::FT_FP16>::type>
  int search_bf(const ailego::Float16 *vec, size_t dim,
                Context::Pointer &context) const {
    return this->search_bf_impl(vec, IndexQueryMeta(FT, dim), context);
  }

  //! Similarity search (FP32)
  template <IndexMeta::FeatureTypes FT,
            typename = typename std::enable_if<FT == IndexMeta::FT_FP32>::type>
  int search_bf(const float *vec, size_t dim, Context::Pointer &context) const {
    return this->search_bf_impl(vec, IndexQueryMeta(FT, dim), context);
  }

  //! Similarity search (INT8)
  template <IndexMeta::FeatureTypes FT,
            typename = typename std::enable_if<FT == IndexMeta::FT_INT8>::type>
  int search_bf(const int8_t *vec, size_t dim,
                Context::Pointer &context) const {
    return this->search_bf_impl(vec, IndexQueryMeta(FT, dim), context);
  }

  //! Similarity search (INT4)
  template <IndexMeta::FeatureTypes FT,
            typename = typename std::enable_if<FT == IndexMeta::FT_INT4>::type>
  int search_bf(const uint8_t *vec, size_t dim,
                Context::Pointer &context) const {
    return this->search_bf_impl(vec, IndexQueryMeta(FT, dim), context);
  }

  //! Similarity search (BINARY)
  template <IndexMeta::FeatureTypes FT, typename = typename std::enable_if<
                                            FT == IndexMeta::FT_BINARY32>::type>
  int search_bf(const uint32_t *vec, size_t dim,
                Context::Pointer &context) const {
    return this->search_bf_impl(vec, IndexQueryMeta(FT, dim), context);
  }

  //! Similarity search in batch (FP16)
  template <IndexMeta::FeatureTypes FT,
            typename = typename std::enable_if<FT == IndexMeta::FT_FP16>::type>
  int search_bf(const ailego::Float16 *vec, size_t dim, size_t rows,
                Context::Pointer &context) const {
    return this->search_bf_impl(vec, IndexQueryMeta(FT, dim), rows, context);
  }

  //! Similarity search in batch (FP32)
  template <IndexMeta::FeatureTypes FT,
            typename = typename std::enable_if<FT == IndexMeta::FT_FP32>::type>
  int search_bf(const float *vec, size_t dim, size_t rows,
                Context::Pointer &context) const {
    return this->search_bf_impl(vec, IndexQueryMeta(FT, dim), rows, context);
  }

  //! Similarity search in batch (INT8)
  template <IndexMeta::FeatureTypes FT,
            typename = typename std::enable_if<FT == IndexMeta::FT_INT8>::type>
  int search_bf(const int8_t *vec, size_t dim, size_t rows,
                Context::Pointer &context) const {
    return this->search_bf_impl(vec, IndexQueryMeta(FT, dim), rows, context);
  }

  //! Similarity search in batch (INT4)
  template <IndexMeta::FeatureTypes FT,
            typename = typename std::enable_if<FT == IndexMeta::FT_INT4>::type>
  int search_bf(const uint8_t *vec, size_t dim, size_t rows,
                Context::Pointer &context) const {
    return this->search_bf_impl(vec, IndexQueryMeta(FT, dim), rows, context);
  }

  //! Similarity Search in batch (BINARY)
  template <IndexMeta::FeatureTypes FT, typename = typename std::enable_if<
                                            FT == IndexMeta::FT_BINARY32>::type>
  int search_bf(const uint32_t *vec, size_t dim, size_t rows,
                Context::Pointer &context) const {
    return this->search_bf_impl(vec, IndexQueryMeta(FT, dim), rows, context);
  }

  //! Similarity search (FP16)
  template <IndexMeta::FeatureTypes FT,
            typename = typename std::enable_if<FT == IndexMeta::FT_FP16>::type>
  int search(const ailego::Float16 *vec, size_t dim,
             Context::Pointer &context) const {
    return this->search_impl(vec, IndexQueryMeta(FT, dim), context);
  }

  //! Similarity search (FP32)
  template <IndexMeta::FeatureTypes FT,
            typename = typename std::enable_if<FT == IndexMeta::FT_FP32>::type>
  int search(const float *vec, size_t dim, Context::Pointer &context) const {
    return this->search_impl(vec, IndexQueryMeta(FT, dim), context);
  }

  //! Similarity search (INT8)
  template <IndexMeta::FeatureTypes FT,
            typename = typename std::enable_if<FT == IndexMeta::FT_INT8>::type>
  int search(const int8_t *vec, size_t dim, Context::Pointer &context) const {
    return this->search_impl(vec, IndexQueryMeta(FT, dim), context);
  }

  //! Similarity search (INT4)
  template <IndexMeta::FeatureTypes FT,
            typename = typename std::enable_if<FT == IndexMeta::FT_INT4>::type>
  int search(const uint8_t *vec, size_t dim, Context::Pointer &context) const {
    return this->search_impl(vec, IndexQueryMeta(FT, dim), context);
  }

  //! Similarity search (BINARY32)
  template <IndexMeta::FeatureTypes FT, typename = typename std::enable_if<
                                            FT == IndexMeta::FT_BINARY32>::type>
  int search(const uint32_t *vec, size_t dim, Context::Pointer &context) const {
    return this->search_impl(vec, IndexQueryMeta(FT, dim), context);
  }

  //! Similarity search in batch (FP16)
  template <IndexMeta::FeatureTypes FT,
            typename = typename std::enable_if<FT == IndexMeta::FT_FP16>::type>
  int search(const ailego::Float16 *vec, size_t dim, size_t rows,
             Context::Pointer &context) const {
    return this->search_impl(vec, IndexQueryMeta(FT, dim), rows, context);
  }

  //! Similarity search in batch (FP32)
  template <IndexMeta::FeatureTypes FT,
            typename = typename std::enable_if<FT == IndexMeta::FT_FP32>::type>
  int search(const float *vec, size_t dim, size_t rows,
             Context::Pointer &context) const {
    return this->search_impl(vec, IndexQueryMeta(FT, dim), rows, context);
  }

  //! Similarity search in batch (INT8)
  template <IndexMeta::FeatureTypes FT,
            typename = typename std::enable_if<FT == IndexMeta::FT_INT8>::type>
  int search(const int8_t *vec, size_t dim, size_t rows,
             Context::Pointer &context) const {
    return this->search_impl(vec, IndexQueryMeta(FT, dim), rows, context);
  }

  //! Similarity Search in batch (INT4)
  template <IndexMeta::FeatureTypes FT,
            typename = typename std::enable_if<FT == IndexMeta::FT_INT4>::type>
  int search(const uint8_t *vec, size_t dim, size_t rows,
             Context::Pointer &context) const {
    return this->search_impl(vec, IndexQueryMeta(FT, dim), rows, context);
  }

  //! Similarity Search in batch (BINARY)
  template <IndexMeta::FeatureTypes FT, typename = typename std::enable_if<
                                            FT == IndexMeta::FT_BINARY32>::type>
  int search(const uint32_t *vec, size_t dim, size_t rows,
             Context::Pointer &context) const {
    return this->search_impl(vec, IndexQueryMeta(FT, dim), rows, context);
  }
};

}  // namespace aitheta2

#endif  // __AITHETA2_INDEX_SEARCHER_H__
