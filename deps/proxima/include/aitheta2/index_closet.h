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

 *   \author   daibing.db
 *   \date     Jun 2021
 *   \brief    Interface of AiTheta Index Closet
 */

#ifndef __AITHETA2_INDEX_SUMMARY_H__
#define __AITHETA2_INDEX_SUMMARY_H__

#include "index_context.h"
#include "index_helper.h"
#include "index_provider.h"
#include "index_stats.h"

namespace aitheta2 {

/*! Index Closet
 */
class IndexCloset : public IndexModule {
 public:
  //! Index Closet Pointer
  typedef std::shared_ptr<IndexCloset> Pointer;

  //! Destructor
  virtual ~IndexCloset(void) {}

  //! Initialize the closet
  virtual int init(const IndexParams &params) = 0;

  //! Cleanup the closet
  virtual int cleanup(void) = 0;

  //! Open a closet index from storage
  virtual int open(IndexStorage::Pointer stg) = 0;

  //! Close closet index
  virtual int close(void) = 0;

  //! Flush closet index
  virtual int flush(uint64_t check_point) = 0;

  //! Put a document and retrieve the local index
  virtual int append(const void *data, size_t len, uint64_t *index) = 0;

  //! Delete the document by local index
  virtual int erase(uint64_t index) = 0;

  //! Fetch a document via a local index
  virtual int fetch(uint64_t index, std::string *out) const = 0;

  //! Update the document via a local index
  virtual int update(uint64_t index, const void *data, size_t len) = 0;

  //! Retrieve the count of index
  virtual uint64_t count(void) const = 0;

  //! Dump closet index into storage
  virtual int dump(const IndexDumper::Pointer &dumper) = 0;
};

/*! Index Immutable Closet
 */
class IndexImmutableCloset : public IndexModule {
 public:
  //! Index Closet Searcher Pointer
  typedef std::shared_ptr<IndexImmutableCloset> Pointer;

  //! Destructor
  virtual ~IndexImmutableCloset(void) {}

  //! Initialize the closet
  virtual int init(const IndexParams &params) = 0;

  //! Cleanup the closet
  virtual int cleanup(void) = 0;

  //! Open a closet index from storage
  virtual int load(IndexContainer::Pointer cntr) = 0;

  //! Close closet index
  virtual int unload(void) = 0;

  //! Fetch a document via a local index
  virtual int fetch(uint64_t index, std::string *out) const = 0;

  //! Retrieve the count of index
  virtual uint64_t count(void) const = 0;
};


}  // namespace aitheta2

#endif  // __AITHETA2_INDEX_CLOSET_H__
