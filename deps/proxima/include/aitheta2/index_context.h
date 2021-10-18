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
 *   \brief    Interface of AiTheta Index Context
 */

#ifndef __AITHETA2_INDEX_CONTEXT_H__
#define __AITHETA2_INDEX_CONTEXT_H__

#include <memory>
#include "index_document.h"
#include "index_error.h"
#include "index_filter.h"
#include "index_params.h"

namespace aitheta2 {

/*! Index Context
 */
class IndexContext {
 public:
  //! Index Context Pointer
  typedef std::unique_ptr<IndexContext> Pointer;

  //! Destructor
  virtual ~IndexContext(void) {}

  //! Set topk of search result
  virtual void set_topk(uint32_t topk) = 0;

  //! Set mode of debug
  virtual void set_debug_mode(bool /*enable*/) {}

  //! Retrieve search result
  virtual const IndexDocumentList &result(void) const = 0;

  //! Retrieve search result with index
  virtual const IndexDocumentList &result(size_t /*index*/) const {
    return this->result();
  }

  //! Update the parameters of context
  virtual int update(const IndexParams & /*params*/) {
    return IndexError_NotImplemented;
  }

  //! Retrieve mode of debug
  virtual bool debug_mode(void) const {
    return false;
  }

  //! Retrieve debug information
  virtual std::string debug_string(void) const {
    return std::string();
  }

  //! Retrieve magic number
  virtual uint32_t magic(void) const {
    return 0;
  }

  //! Retrieve search filter
  const IndexFilter &filter(void) const {
    return filter_;
  }

  //! Set the filter of context
  template <typename T>
  void set_filter(T &&func) {
    filter_.set(std::forward<T>(func));
  }

  //! Reset the filter of context
  void reset_filter(void) {
    filter_.reset();
  }

  //! Set threshold for RNN
  void set_threshold(float val) {
    threshold_ = val;
  }

  //! Retrieve value of threshold for RNN
  float threshold(void) const {
    return threshold_;
  }

  //! Generate a global magic number
  static uint32_t GenerateMagic(void);

 private:
  //! Members
  IndexFilter filter_{};
  float threshold_{std::numeric_limits<float>::max()};
};

}  // namespace aitheta2

#endif  // __AITHETA2_INDEX_CONTEXT_H__
