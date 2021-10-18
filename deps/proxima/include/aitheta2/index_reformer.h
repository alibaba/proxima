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
 *   \brief    Interface of AiTheta Index Reformer
 */

#ifndef __AITHETA2_INDEX_REFORMER_H__
#define __AITHETA2_INDEX_REFORMER_H__

#include "index_container.h"
#include "index_document.h"

namespace aitheta2 {

/*! Index Reformer
 */
class IndexReformer : public IndexModule {
 public:
  //! Index Reformer Pointer
  typedef std::shared_ptr<IndexReformer> Pointer;

  //! Destructor
  virtual ~IndexReformer(void) {}

  //! Initialize Reformer
  virtual int init(const IndexParams &params) = 0;

  //! Cleanup Reformer
  virtual int cleanup(void) = 0;

  //! Load index from container
  virtual int load(IndexContainer::Pointer cntr) = 0;

  //! Unload index
  virtual int unload(void) = 0;

  //! Transform a query
  virtual int transform(const void *query, const IndexQueryMeta &qmeta,
                        std::string *out, IndexQueryMeta *ometa) const = 0;

  //! Transform queries
  virtual int transform(const void *query, const IndexQueryMeta &qmeta,
                        uint32_t count, std::string *out,
                        IndexQueryMeta *ometa) const = 0;

  //! Convert a record
  virtual int convert(const void *record, const IndexQueryMeta &rmeta,
                      std::string *out, IndexQueryMeta *ometa) const {
    return this->transform(record, rmeta, out, ometa);
  }

  //! Convert records
  virtual int convert(const void *records, const IndexQueryMeta &rmeta,
                      uint32_t count, std::string *out,
                      IndexQueryMeta *ometa) const {
    return this->transform(records, rmeta, count, out, ometa);
  }

  //! Normalize results
  virtual int normalize(const void *query, const IndexQueryMeta &qmeta,
                        IndexDocumentList &result) const = 0;
};

}  // namespace aitheta2

#endif  // __AITHETA2_INDEX_REFORMER_H__
