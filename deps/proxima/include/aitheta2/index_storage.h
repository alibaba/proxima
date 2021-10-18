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
 *   \date     Jan 2020
 *   \brief    Interface of AiTheta Index Storage
 */

#ifndef __AITHETA2_INDEX_STORAGE_H__
#define __AITHETA2_INDEX_STORAGE_H__

#include "index_module.h"
#include "index_params.h"

namespace aitheta2 {

/*! Index Storage
 */
class IndexStorage : public IndexModule {
 public:
  //! Index Storage Pointer
  typedef std::shared_ptr<IndexStorage> Pointer;

  /*! Index Storage Segment
   */
  struct Segment {
    //! Index Storage Pointer
    typedef std::shared_ptr<Segment> Pointer;

    //! Destructor
    virtual ~Segment(void) {}

    //! Retrieve size of data
    virtual size_t data_size(void) const = 0;

    //! Retrieve crc of data
    virtual uint32_t data_crc(void) const = 0;

    //! Retrieve size of padding
    virtual size_t padding_size(void) const = 0;

    //! Retrieve capacity of segment
    virtual size_t capacity(void) const = 0;

    //! Fetch data from segment (with own buffer)
    virtual size_t fetch(size_t offset, void *buf, size_t len) const = 0;

    //! Read data from segment
    virtual size_t read(size_t offset, const void **data, size_t len) = 0;

    //! Write data into the storage with offset
    virtual size_t write(size_t offset, const void *data, size_t len) = 0;

    //! Resize size of data
    virtual size_t resize(size_t size) = 0;

    //! Update crc of data
    virtual void update_data_crc(uint32_t crc) = 0;

    //! Clone the segment
    virtual Pointer clone(void) = 0;
  };

  //! Destructor
  virtual ~IndexStorage(void) {}

  //! Initialize storage
  virtual int init(const IndexParams &params) = 0;

  //! Cleanup storage
  virtual int cleanup(void) = 0;

  //! Open storage
  virtual int open(const std::string &path, bool create) = 0;

  //! Flush storage
  virtual int flush(void) = 0;

  //! Close storage
  virtual int close(void) = 0;

  //! Append a segment into storage
  virtual int append(const std::string &id, size_t size) = 0;

  //! Refresh meta information (checksum, update time, etc.)
  virtual void refresh(uint64_t check_point) = 0;

  //! Retrieve check point of storage
  virtual uint64_t check_point(void) const = 0;

  //! Retrieve a segment by id
  virtual Segment::Pointer get(const std::string &id) = 0;

  //! Test if it a segment exists
  virtual bool has(const std::string &id) const = 0;

  //! Retrieve magic number of index
  virtual uint32_t magic(void) const = 0;
};

}  // namespace aitheta2

#endif  // __AITHETA2_INDEX_STORAGE_H__
