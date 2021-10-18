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
 *   \date     May 2019
 *   \brief    Interface of AiTheta Index Measure
 */

#ifndef __AITHETA2_INDEX_MEASURE_H__
#define __AITHETA2_INDEX_MEASURE_H__

#include "index_meta.h"
#include "index_module.h"

namespace aitheta2 {

/*! Index Measure
 */
struct IndexMeasure : public IndexModule {
  //! Index Measure Pointer
  typedef std::shared_ptr<IndexMeasure> Pointer;

  //! Matrix Distance Function
  typedef void (*MatrixDistanceHandle)(const void *m, const void *q, size_t dim,
                                       float *out);

  //! Matrix Distance Function Object
  using MatrixDistance =
      std::function<void(const void *m, const void *q, size_t dim, float *out)>;

  //! Destructor
  virtual ~IndexMeasure(void) {}

  //! Initialize Measure
  virtual int init(const IndexMeta &meta, const IndexParams &params) = 0;

  //! Cleanup Measure
  virtual int cleanup(void) = 0;

  //! Retrieve if it matched
  virtual bool is_matched(const IndexMeta &meta) const = 0;

  //! Retrieve if it matched
  virtual bool is_matched(const IndexMeta &meta,
                          const IndexQueryMeta &qmeta) const = 0;

  //! Retrieve distance function for query
  virtual MatrixDistance distance(void) const = 0;

  //! Retrieve distance function for index features
  virtual MatrixDistance distance_matrix(size_t m, size_t n) const = 0;

  //! Retrieve params of Measure
  virtual const IndexParams &params(void) const = 0;

  //! Retrieve query measure object of this index measure
  virtual Pointer query_measure(void) const = 0;

  //! Normalize result
  virtual void normalize(float *score) const {
    (void)score;
  }

  //! Retrieve if it supports normalization
  virtual bool support_normalize(void) const {
    return false;
  }

  //! Train the measure
  virtual int train(const void *vec, size_t dim) {
    (void)vec;
    (void)dim;
    return 0;
  }

  //! Retrieve if it supports training
  virtual bool support_train(void) const {
    return false;
  }

  //! Compute the distance between feature and query
  float distance(const void *m, const void *q, size_t dim) const {
    float dist;
    (this->distance())(m, q, dim, &dist);
    return dist;
  }
};

}  // namespace aitheta2

#endif  // __AITHETA2_INDEX_MEASURE_H__
