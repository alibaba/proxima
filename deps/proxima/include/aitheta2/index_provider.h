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
 *   \date     Feb 2020
 *   \brief    Interface of AiTheta Index Provider
 */

#ifndef __AITHETA2_INDEX_PROVIDER_H__
#define __AITHETA2_INDEX_PROVIDER_H__

#include "index_holder.h"

namespace aitheta2 {

/*! Index Provider
 */
struct IndexProvider {
  //! Index Provider Pointer
  typedef std::shared_ptr<IndexProvider> Pointer;

  //! Index Provider Iterator
  typedef IndexHolder::Iterator Iterator;

  //! Destructor
  virtual ~IndexProvider(void) {}

  //! Create a new iterator
  virtual Iterator::Pointer create_iterator(void) const = 0;

  //! Retrieve count of elements in provider
  virtual size_t count(void) const = 0;

  //! Retrieve dimension of vector
  virtual size_t dimension(void) const = 0;

  //! Retrieve type of vector
  virtual IndexMeta::FeatureTypes vector_type(void) const = 0;

  //! Retrieve vector size in bytes
  virtual size_t vector_size(void) const = 0;

  //! Retrieve a vector using a primary key
  virtual const void *get_vector(uint64_t key) const = 0;

  //! Retrieve an attachment using a primary key
  virtual const void *get_attachment(uint64_t key, size_t *len) const = 0;

  //! Retrieve the owner class
  virtual const std::string &owner_class(void) const = 0;
};

}  // namespace aitheta2

#endif  // __AITHETA2_INDEX_PROVIDER_H__
