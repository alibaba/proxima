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
 *   \brief    Interface of AiTheta Index Realtime Streamer
 */

#ifndef __AITHETA2_INDEX_STREAMER_H__
#define __AITHETA2_INDEX_STREAMER_H__

#include "index_context.h"
#include "index_helper.h"
#include "index_provider.h"
#include "index_stats.h"
#include "index_threads.h"

namespace aitheta2 {

/*! Index Streamer
 */
class IndexStreamer : public IndexModule {
 public:
  //! Index Streamer Pointer
  typedef std::shared_ptr<IndexStreamer> Pointer;

  /*! Index Streamer Stats
   */
  class Stats : public IndexStats {
   public:
    //! Set revision id
    void set_revision_id(size_t rev) {
      revision_id_ = rev;
    }

    //! Set count of documents loaded
    void set_loaded_count(size_t count) {
      loaded_count_ = count;
    }

    //! Set count of documents added
    void set_added_count(size_t count) {
      added_count_ = count;
    }

    //! Set count of documents discarded
    void set_discarded_count(size_t count) {
      discarded_count_ = count;
    }

    //! Set count of documents updated
    void set_updated_count(size_t count) {
      updated_count_ = count;
    }

    //! Set count of documents deleted
    void set_deleted_count(size_t count) {
      deleted_count_ = count;
    }

    //! Set size of index
    void set_index_size(size_t count) {
      index_size_ = count;
    }

    //! Set size of index dumped
    void set_dumped_size(size_t count) {
      dumped_size_ = count;
    }

    //! Retrieve create time
    void set_create_time(uint64_t val) {
      create_time_ = val;
    }

    //! Retrieve update time
    void set_update_time(uint64_t val) {
      update_time_ = val;
    }

    //! Retrieve revision id
    size_t revision_id(void) const {
      return revision_id_;
    }

    //! Retrieve count of documents loaded
    size_t loaded_count(void) const {
      return loaded_count_;
    }

    //! Retrieve count of documents added
    size_t added_count(void) const {
      return added_count_;
    }

    //! Retrieve count of documents discarded
    size_t discarded_count(void) const {
      return discarded_count_;
    }

    //! Retrieve count of documents updated
    size_t updated_count(void) const {
      return updated_count_;
    }

    //! Retrieve count of documents deleted
    size_t deleted_count(void) const {
      return deleted_count_;
    }

    //! Retrieve size of index
    size_t index_size(void) const {
      return index_size_;
    }

    //! Retrieve size of index dumped
    size_t dumped_size(void) const {
      return dumped_size_;
    }

    //! Retrieve check point of index
    uint64_t check_point(void) const {
      return check_point_;
    }

    //! Retrieve create time of index
    uint64_t create_time(void) const {
      return create_time_;
    }

    //! Retrieve update time of index
    uint64_t update_time(void) const {
      return update_time_;
    }

    //! Retrieve count of documents loaded (mutable)
    size_t *mutable_loaded_count(void) {
      return &loaded_count_;
    }

    //! Retrieve count of documents added (mutable)
    size_t *mutable_added_count(void) {
      return &added_count_;
    }

    //! Retrieve count of documents discarded (mutable)
    size_t *mutable_discarded_count(void) {
      return &discarded_count_;
    }

    //! Retrieve count of documents updated (mutable)
    size_t *mutable_updated_count(void) {
      return &updated_count_;
    }

    //! Retrieve count of documents deleted (mutable)
    size_t *mutable_deleted_count(void) {
      return &deleted_count_;
    }

    //! Retrieve size of index (mutable)
    size_t *mutable_index_size(void) {
      return &index_size_;
    }

    //! Retrieve size of index dumped (mutable)
    size_t *mutable_dumped_size(void) {
      return &dumped_size_;
    }

    //! Retrieve check point of index (mutable)
    uint64_t *mutable_check_point(void) {
      return &check_point_;
    }

    //! Retrieve create time of index (mutable)
    uint64_t *mutable_create_time(void) {
      return &create_time_;
    }

    //! Retrieve update time of index (mutable)
    uint64_t *mutable_update_time(void) {
      return &update_time_;
    }

   private:
    //! Members
    size_t revision_id_{0u};
    size_t loaded_count_{0u};
    size_t added_count_{0u};
    size_t discarded_count_{0u};
    size_t updated_count_{0u};
    size_t deleted_count_{0u};
    size_t index_size_{0u};
    size_t dumped_size_{0u};
    uint64_t check_point_{0u};
    uint64_t create_time_{0u};
    uint64_t update_time_{0u};
  };

  /*! Index Streamer Context
   */
  struct Context : public IndexContext {};

  /*! Index Streamer Provider
   */
  struct Provider : public IndexProvider {};

  //! Destructor
  virtual ~IndexStreamer(void) {}

  //! Initialize the streamer
  virtual int init(const IndexMeta &mt, const IndexParams &params) = 0;

  //! Cleanup the streamer
  virtual int cleanup(void) = 0;

  //! Create a context
  virtual Context::Pointer create_context(void) const = 0;

  //! Similarity search
  virtual int search_impl(const void *query, const IndexQueryMeta &qmeta,
                          Context::Pointer &context) const = 0;

  //! Similarity search
  virtual int search_impl(const void *query, const IndexQueryMeta &qmeta,
                          uint32_t count, Context::Pointer &context) const = 0;

  //! Similarity brute force search
  virtual int search_bf_impl(const void *query, const IndexQueryMeta &qmeta,
                             Context::Pointer &context) const = 0;

  //! Similarity brute force search
  virtual int search_bf_impl(const void *query, const IndexQueryMeta &qmeta,
                             uint32_t count,
                             Context::Pointer &context) const = 0;

  //! Add a vector into index
  virtual int add_impl(uint64_t key, const void *query,
                       const IndexQueryMeta &qmeta,
                       Context::Pointer &context) = 0;

  //! Update the vector in index
  virtual int update_impl(uint64_t /*key*/, const void * /*query*/,
                          const IndexQueryMeta & /*qmeta*/,
                          Context::Pointer & /*context*/) {
    return IndexError_NotImplemented;
  }

  //! Delete the vector in index
  virtual int remove_impl(uint64_t /*key*/, Context::Pointer & /*context*/) {
    return IndexError_NotImplemented;
  }

  //! Optimize the index
  virtual int optimize_impl(aitheta2::IndexThreads::Pointer) {
    return aitheta2::IndexError_NotImplemented;
  }

  //! Open a index from storage
  virtual int open(IndexStorage::Pointer stg) = 0;

  //! Flush index
  virtual int flush(uint64_t check_point) = 0;

  //! Close index
  virtual int close(void) = 0;

  //! Dump index into storage
  virtual int dump(const IndexDumper::Pointer &dumper) = 0;

  //! Retrieve statistics
  virtual const Stats &stats(void) const = 0;

  //! Retrieve meta of index
  virtual const IndexMeta &meta(void) const = 0;

  //! Initialize the streamer with container
  virtual int init(IndexContainer::Pointer cntr, const IndexParams &params) {
    IndexMeta mt;
    int ret = IndexHelper::DeserializeFromContainer(cntr.get(), &mt);
    if (ret == 0) {
      ret = this->init(mt, params);
    }
    return ret;
  }

  //! Create a streamer provider
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

  //! Similarity Search in batch (INT4)
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

  //! Add a vector into index (FP16)
  template <IndexMeta::FeatureTypes FT,
            typename = typename std::enable_if<FT == IndexMeta::FT_FP16>::type>
  int add(uint64_t key, const ailego::Float16 *vec, size_t dim,
          Context::Pointer &context) {
    return this->add_impl(key, vec, IndexQueryMeta(FT, dim), context);
  }

  //! Add a vector into index (FP32)
  template <IndexMeta::FeatureTypes FT,
            typename = typename std::enable_if<FT == IndexMeta::FT_FP32>::type>
  int add(uint64_t key, const float *vec, size_t dim,
          Context::Pointer &context) {
    return this->add_impl(key, vec, IndexQueryMeta(FT, dim), context);
  }

  //! Add a vector into index (INT8)
  template <IndexMeta::FeatureTypes FT,
            typename = typename std::enable_if<FT == IndexMeta::FT_INT8>::type>
  int add(uint64_t key, const int8_t *vec, size_t dim,
          Context::Pointer &context) {
    return this->add_impl(key, vec, IndexQueryMeta(FT, dim), context);
  }

  //! Add a vector into index (INT4)
  template <IndexMeta::FeatureTypes FT,
            typename = typename std::enable_if<FT == IndexMeta::FT_INT4>::type>
  int add(uint64_t key, const uint8_t *vec, size_t dim,
          Context::Pointer &context) {
    return this->add_impl(key, vec, IndexQueryMeta(FT, dim), context);
  }

  //! Add a vector into index (BINARY)
  template <IndexMeta::FeatureTypes FT, typename = typename std::enable_if<
                                            FT == IndexMeta::FT_BINARY32>::type>
  int add(uint64_t key, const uint32_t *vec, size_t dim,
          Context::Pointer &context) {
    return this->add_impl(key, vec, IndexQueryMeta(FT, dim), context);
  }

  //! Update the vector to index (FP16)
  template <IndexMeta::FeatureTypes FT,
            typename = typename std::enable_if<FT == IndexMeta::FT_FP16>::type>
  int update(uint64_t key, const ailego::Float16 *vec, size_t dim,
             Context::Pointer &context) {
    return this->update_impl(key, vec, IndexQueryMeta(FT, dim), context);
  }

  //! Update the vector to index (FP32)
  template <IndexMeta::FeatureTypes FT,
            typename = typename std::enable_if<FT == IndexMeta::FT_FP32>::type>
  int update(uint64_t key, const float *vec, size_t dim,
             Context::Pointer &context) {
    return this->update_impl(key, vec, IndexQueryMeta(FT, dim), context);
  }

  //! Update the vector to index (INT8)
  template <IndexMeta::FeatureTypes FT,
            typename = typename std::enable_if<FT == IndexMeta::FT_INT8>::type>
  int update(uint64_t key, const int8_t *vec, size_t dim,
             Context::Pointer &context) {
    return this->update_impl(key, vec, IndexQueryMeta(FT, dim), context);
  }

  //! Update the vector in index (INT4)
  template <IndexMeta::FeatureTypes FT,
            typename = typename std::enable_if<FT == IndexMeta::FT_INT4>::type>
  int update(uint64_t key, const uint8_t *vec, size_t dim,
             Context::Pointer &context) {
    return this->update_impl(key, vec, IndexQueryMeta(FT, dim), context);
  }

  //! Update the vector in index (BINARY)
  template <IndexMeta::FeatureTypes FT, typename = typename std::enable_if<
                                            FT == IndexMeta::FT_BINARY32>::type>
  int update(uint64_t key, const uint32_t *vec, size_t dim,
             Context::Pointer &context) {
    return this->update_impl(key, vec, IndexQueryMeta(FT, dim), context);
  }

  //! Delete the vector in index
  int remove(uint64_t key, Context::Pointer &context) {
    return this->remove_impl(key, context);
  }

  //! Optimize the index
  int optimize(aitheta2::IndexThreads::Pointer threads) {
    return this->optimize_impl(threads);
  }
};

}  // namespace aitheta2

#endif  // __AITHETA2_INDEX_STREAMER_H__
