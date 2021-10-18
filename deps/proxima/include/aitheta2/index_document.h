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
 *   \brief    Interface of AiTheta Index Document
 */

#ifndef __AITHETA2_INDEX_DOCUMENT_H__
#define __AITHETA2_INDEX_DOCUMENT_H__

#include <cstdint>
#include <ailego/container/heap.h>

namespace aitheta2 {

/*! Index Document
 */
class IndexDocument {
 public:
  //! Constructor
  IndexDocument() = default;

  //! Constructor
  IndexDocument(uint64_t k, float v) : key_(k), score_(v) {}

  //! Constructor
  IndexDocument(uint64_t k, float v, uint32_t i)
      : key_(k), score_(v), index_(i) {}

  //! Constructor
  IndexDocument(const IndexDocument &rhs)
      : key_(rhs.key_), score_(rhs.score_), index_(rhs.index_) {}

  //! Assignment
  IndexDocument &operator=(const IndexDocument &rhs) {
    key_ = rhs.key_;
    score_ = rhs.score_;
    index_ = rhs.index_;
    return *this;
  }

  //! Less than
  bool operator<(const IndexDocument &rhs) const {
    return (this->score_ < rhs.score_);
  }

  //! Greater than
  bool operator>(const IndexDocument &rhs) const {
    return (this->score_ > rhs.score_);
  }

  //! Retrieve primary key
  uint64_t key(void) const {
    return key_;
  }

  //! Retrieve score value
  float score(void) const {
    return score_;
  }

  //! Retrieve index id
  uint32_t index(void) const {
    return index_;
  }

  //! Retrieve mutable primary key
  uint64_t *mutable_key(void) {
    return &key_;
  }

  //! Retrieve mutable score value
  float *mutable_score(void) {
    return &score_;
  }

  //! Retrieve mutable index id
  uint32_t *mutable_index(void) {
    return &index_;
  }

  //! Retrieve primary key
  void set_key(uint64_t val) {
    key_ = val;
  }

  //! Retrieve score value
  void set_score(float val) {
    score_ = val;
  }

  //! Retrieve index id
  void set_index(uint32_t val) {
    index_ = val;
  }

 private:
  //! Data members
  uint64_t key_{0u};
  float score_{0.0f};
  uint32_t index_{0u};
};

/*! Index Document Heap
 */
class IndexDocumentHeap : public ailego::Heap<IndexDocument> {
 public:
  //! Constructor
  IndexDocumentHeap(void) : ailego::Heap<IndexDocument>() {}

  //! Constructor
  IndexDocumentHeap(size_t max) : ailego::Heap<IndexDocument>(max) {}

  //! Constructor
  IndexDocumentHeap(size_t max, float val)
      : ailego::Heap<IndexDocument>(max), threshold_(val) {}

  //! Constructor
  IndexDocumentHeap(const IndexDocumentHeap &rhs)
      : ailego::Heap<IndexDocument>(rhs), threshold_(rhs.threshold_) {}

  //! Constructor
  IndexDocumentHeap(IndexDocumentHeap &&rhs)
      : ailego::Heap<IndexDocument>(std::move(rhs)),
        threshold_(rhs.threshold_) {}

  //! Constructor
  IndexDocumentHeap(const std::vector<IndexDocument> &rhs)
      : ailego::Heap<IndexDocument>(rhs) {}

  //! Constructor
  IndexDocumentHeap(std::vector<IndexDocument> &&rhs)
      : ailego::Heap<IndexDocument>(std::move(rhs)) {}

  //! Insert a document into the heap
  void emplace(uint64_t key, float score) {
    if (score <= threshold_) {
      ailego::Heap<IndexDocument>::emplace(key, score);
    }
  }

  //! Insert a document into the heap
  void emplace(uint64_t key, float score, uint32_t index) {
    if (score <= threshold_) {
      ailego::Heap<IndexDocument>::emplace(key, score, index);
    }
  }

  //! Set threshold for RNN
  void set_threshold(float val) {
    threshold_ = val;
  }

  //! Retrieve value of threshold for RNN
  float threshold(void) const {
    return threshold_;
  }

 private:
  //! members
  float threshold_{std::numeric_limits<float>::max()};
};

/*! Index Document List
 */
using IndexDocumentList = std::vector<IndexDocument>;

}  // namespace aitheta2

#endif  // __AITHETA2_INDEX_DOCUMENT_H__
