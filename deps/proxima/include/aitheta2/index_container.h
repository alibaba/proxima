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
 *   \brief    Interface of AiTheta Index Container
 */

#ifndef __AITHETA2_INDEX_CONTAINER_H__
#define __AITHETA2_INDEX_CONTAINER_H__

#include "index_module.h"
#include "index_unpacker.h"

namespace aitheta2 {

/*! Index Container
 */
class IndexContainer : public IndexModule {
 public:
  //! Index Container Pointer
  typedef std::shared_ptr<IndexContainer> Pointer;

  /*! Index Container Segment Data
   */
  struct SegmentData {
    //! Constructor
    SegmentData(void) : offset(0u), length(0u), data(nullptr) {}

    //! Constructor
    SegmentData(size_t off, size_t len)
        : offset(off), length(len), data(nullptr) {}

    //! Members
    size_t offset;
    size_t length;
    const void *data;
  };

  /*! Index Container Segment
   */
  struct Segment {
    //! Index Container Pointer
    typedef std::shared_ptr<Segment> Pointer;

    //! Destructor
    virtual ~Segment(void) {}

    //! Retrieve size of data
    virtual size_t data_size(void) const = 0;

    //! Retrieve crc of data
    virtual uint32_t data_crc(void) const = 0;

    //! Retrieve size of padding
    virtual size_t padding_size(void) const = 0;

    //! Fetch data from segment (with own buffer)
    virtual size_t fetch(size_t offset, void *buf, size_t len) const = 0;

    //! Read data from segment
    virtual size_t read(size_t offset, const void **data, size_t len) = 0;

    //! Read data from segment
    virtual bool read(SegmentData *iovec, size_t count) = 0;

    //! Clone the segment
    virtual Pointer clone(void) = 0;
  };

  //! Destructor
  virtual ~IndexContainer(void) {}

  //! Initialize container
  virtual int init(const IndexParams &params) = 0;

  //! Cleanup container
  virtual int cleanup(void) = 0;

  //! Load a index file into container
  virtual int load(const std::string &path) = 0;

  //! Load the current index into container
  virtual int load(void) = 0;

  //! Unload all indexes
  virtual int unload(void) = 0;

  //! Retrieve a segment by id
  virtual Segment::Pointer get(const std::string &id) const = 0;

  //! Test if it a segment exists
  virtual bool has(const std::string &id) const = 0;

  //! Retrieve all segments
  virtual std::map<std::string, Segment::Pointer> get_all(void) const = 0;

  //! Retrieve magic number of index
  virtual uint32_t magic(void) const = 0;

  //! Fetch a segment by id with level (0 high, 1 normal, 2 low)
  virtual Segment::Pointer fetch(const std::string &id, int /*level*/) const {
    return this->get(id);
  }
};

/*! Index Segment Container
 */
class IndexSegmentContainer : public IndexContainer {
 public:
  //! Index Segment Container Pointer
  typedef std::shared_ptr<IndexSegmentContainer> Pointer;

  /*! Index Container Segment
   */
  class Segment : public IndexContainer::Segment {
   public:
    //! Index Container Pointer
    typedef std::shared_ptr<Segment> Pointer;

    //! Constructor
    Segment(const Segment &rhs)
        : data_offset_(rhs.data_offset_),
          data_size_(rhs.data_size_),
          padding_size_(rhs.padding_size_),
          region_size_(rhs.region_size_),
          data_crc_(rhs.data_crc_),
          parent_(rhs.parent_->clone()) {}

    //! Constructor
    Segment(const IndexContainer::Segment::Pointer &parent,
            const IndexUnpacker::SegmentMeta &segment)
        : data_offset_(segment.data_offset()),
          data_size_(segment.data_size()),
          padding_size_(segment.padding_size()),
          region_size_(segment.data_size() + segment.padding_size()),
          data_crc_(segment.data_crc()),
          parent_(parent->clone()) {}

    //! Destructor
    virtual ~Segment(void) {}

    //! Retrieve size of data
    size_t data_size(void) const override {
      return data_size_;
    }

    //! Retrieve crc of data
    uint32_t data_crc(void) const override {
      return data_crc_;
    }

    //! Retrieve size of padding
    size_t padding_size(void) const override {
      return padding_size_;
    }

    //! Fetch data from segment (with own buffer)
    size_t fetch(size_t offset, void *buf, size_t len) const override {
      return parent_->fetch(data_offset_ + offset, buf, len);
    }

    //! Read data from segment
    size_t read(size_t offset, const void **data, size_t len) override {
      return parent_->read(data_offset_ + offset, data, len);
    }

    //! Read data from segment
    bool read(SegmentData *iovec, size_t count) override {
      for (SegmentData *it = iovec, *end = iovec + count; it != end; ++it) {
        it->offset += data_offset_;
      }
      bool success = parent_->read(iovec, count);
      for (SegmentData *it = iovec, *end = iovec + count; it != end; ++it) {
        it->offset -= data_offset_;
      }
      return success;
    }

    //! Clone the segment
    IndexContainer::Segment::Pointer clone(void) override {
      return std::make_shared<Segment>(*this);
    }

   private:
    size_t data_offset_{0u};
    size_t data_size_{0u};
    size_t padding_size_{0u};
    size_t region_size_{0u};
    uint32_t data_crc_{0u};
    IndexContainer::Segment::Pointer parent_{nullptr};
  };

  //! Constructor
  IndexSegmentContainer(IndexContainer::Segment::Pointer &&seg)
      : parent_(std::move(seg)) {}

  //! Constructor
  IndexSegmentContainer(const IndexContainer::Segment::Pointer &seg)
      : parent_(seg) {}

  //! Destructor
  virtual ~IndexSegmentContainer(void) {}

  //! Initialize container
  int init(const IndexParams &) override {
    return 0;
  }

  //! Cleanup container
  int cleanup(void) override {
    return 0;
  }

  //! Load the current index into container
  int load(void) override {
    if (!parent_) {
      LOG_ERROR("Failed to load an empty segment");
      return IndexError_NoReady;
    }

    auto read_data = [this](size_t offset, const void **data, size_t len) {
      return this->parent_->read(offset, data, len);
    };

    IndexUnpacker unpacker;
    if (!unpacker.unpack(read_data, parent_->data_size(), false)) {
      LOG_ERROR("Failed to unpack segment data");
      return IndexError_UnpackIndex;
    }
    segments_ = std::move(*unpacker.mutable_segments());
    magic_ = unpacker.magic();
    return 0;
  }

  //! Load the current segment, ignore path
  int load(const std::string &) override {
    return this->load();
  }

  //! Retrieve a segment by id
  IndexContainer::Segment::Pointer get(const std::string &id) const override {
    if (!parent_) {
      return IndexContainer::Segment::Pointer();
    }
    auto it = segments_.find(id);
    if (it == segments_.end()) {
      return IndexContainer::Segment::Pointer();
    }
    return std::make_shared<IndexSegmentContainer::Segment>(parent_,
                                                            it->second);
  }

  //! Test if it a segment exists
  bool has(const std::string &id) const override {
    return (segments_.find(id) != segments_.end());
  }

  //! Retrieve all segments
  std::map<std::string, IndexContainer::Segment::Pointer> get_all(
      void) const override {
    std::map<std::string, IndexContainer::Segment::Pointer> result;
    if (parent_) {
      for (const auto &it : segments_) {
        result.emplace(it.first,
                       std::make_shared<IndexSegmentContainer::Segment>(
                           parent_, it.second));
      }
    }
    return result;
  }

  //! Unload all indexes
  int unload(void) override {
    parent_ = nullptr;
    segments_.clear();
    return 0;
  }

  //! Retrieve magic number of index
  uint32_t magic(void) const override {
    return magic_;
  }

 private:
  uint32_t magic_{0};
  std::map<std::string, IndexUnpacker::SegmentMeta> segments_{};
  IndexContainer::Segment::Pointer parent_{};
};

}  // namespace aitheta2

#endif  // __AITHETA2_INDEX_CONTAINER_H__
