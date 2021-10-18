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

 *   \author   Haichao.chc
 *   \date     Oct 2020
 *   \brief    Reserving append data and mmap to persist storage, thread-safe
 */

#pragma once

#include <mutex>
#include <ailego/utility/string_helper.h>
#include "common/error_code.h"
#include "common/macro_define.h"
#include "constants.h"
#include "typedef.h"

namespace proxima {
namespace be {
namespace index {

/*
 * DeltaStore is a kind of linear storage in memory, and
 * it can snapshot to disk at the same time with mmap.
 * It can only increase at the tail part.
 */
template <typename T>
class DeltaStore {
 public:
  /*
   * Header segment struct
   */
  struct Header {
    uint64_t block_count{0U};
    uint64_t total_size{0U};
    uint64_t reserved_[6];
  };

  static_assert(sizeof(Header) % 64 == 0,
                "Header must be aligned with 64 bytes");

 public:
  PROXIMA_DISALLOW_COPY_AND_ASSIGN(DeltaStore);

  //! Constructor
  DeltaStore() = default;

  //! Destructor
  ~DeltaStore() = default;

  //! Mount persist storage
  int mount(const IndexStoragePtr &stg) {
    if (!stg) {
      LOG_ERROR("Mount null storage");
      return ErrorCode_RuntimeError;
    }

    storage_ = stg;
    header_block_ = storage_->get(HEADER_BLOCK);
    int ret = 0;
    if (!header_block_) {
      ret = init_storage();
    } else {
      ret = load_storage();
    }
    CHECK_RETURN(ret, 0);

    return 0;
  }

  //! Unmount persist storage
  void unmount() {
    storage_ = nullptr;
    header_block_.reset();
    data_blocks_.clear();
    memset(&header_, 0, sizeof(header_));
    node_count_ = 0U;
  }

  //! Append an element
  int append(const T &element) {
    std::lock_guard<std::mutex> lock(mutex_);

    size_t block_offset = 0U;
    IndexBlockPtr data_block;

    uint32_t block_index = data_blocks_.size() - 1;
    size_t block_size = kNodeCountPerBlock * sizeof(T);
    if (block_index == -1U ||
        data_blocks_[block_index]->data_size() >= block_size) {
      block_index++;
      std::string new_block_name =
          ailego::StringHelper::Concat(DATA_BLOCK, block_index);

      int ret = storage_->append(new_block_name, block_size);
      CHECK_RETURN(ret, 0);

      header_.block_count++;
      header_.total_size += block_size;
      ret = update_header();
      CHECK_RETURN(ret, 0);

      block_offset = 0U;
      data_block = storage_->get(new_block_name);
      data_blocks_.emplace_back(data_block);
    } else {
      data_block = data_blocks_[block_index];
      block_offset = data_block->data_size();
    }

    size_t write_len = data_block->write(block_offset, &element, sizeof(T));
    if (write_len != sizeof(T)) {
      return ErrorCode_WriteData;
    }

    node_count_++;
    return 0;
  }

  //! Update an element by position
  int update(size_t pos, const T &element) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (pos >= node_count_) {
      return ErrorCode_RuntimeError;
    }

    size_t block_index = pos / kNodeCountPerBlock;
    size_t block_offset = (pos % kNodeCountPerBlock) * sizeof(T);

    std::string block_name =
        ailego::StringHelper::Concat(DATA_BLOCK, block_index);
    IndexBlockPtr data_block = storage_->get(block_name);
    if (!data_block) {
      return ErrorCode_InvalidIndexDataFormat;
    }

    size_t write_len = data_block->write(block_offset, &element, sizeof(T));
    if (write_len != sizeof(T)) {
      return ErrorCode_WriteData;
    }

    return 0;
  }

  //! Get an element by position
  const T *at(size_t pos) const {
    if (pos >= node_count_) {
      return nullptr;
    }

    size_t block_index = pos / kNodeCountPerBlock;
    size_t block_offset = (pos % kNodeCountPerBlock) * sizeof(T);

    std::string block_name =
        ailego::StringHelper::Concat(DATA_BLOCK, block_index);
    IndexBlockPtr data_block = storage_->get(block_name);
    if (!data_block) {
      return nullptr;
    }

    T *element;
    size_t read_len =
        data_block->read(block_offset, (const void **)&element, sizeof(T));
    if (read_len != sizeof(T)) {
      return nullptr;
    }
    return element;
  }

  //! Return node count
  size_t count() const {
    return node_count_;
  }

 private:
  int init_storage() {
    int ret = storage_->append(HEADER_BLOCK, sizeof(Header));
    CHECK_RETURN(ret, 0);

    header_block_ = storage_->get(HEADER_BLOCK);
    header_.block_count = 0U;
    header_.total_size = 0U;
    ret = update_header();

    node_count_ = 0UL;
    return ret;
  }

  int load_storage() {
    header_block_ = storage_->get(HEADER_BLOCK);

    Header *header;
    size_t read_len =
        header_block_->read(0, (const void **)&header, sizeof(Header));
    if (read_len != sizeof(Header)) {
      return ErrorCode_ReadData;
    }
    header_ = *header;

    for (uint32_t i = 0; i < header_.block_count; i++) {
      std::string block_name = ailego::StringHelper::Concat(DATA_BLOCK, i);
      auto data_block = storage_->get(block_name);
      if (!data_block) {
        return ErrorCode_ReadData;
      }
      data_blocks_.emplace_back(data_block);
    }

    if (header_.block_count > 0) {
      node_count_ =
          (header_.block_count - 1) * kNodeCountPerBlock +
          data_blocks_[header_.block_count - 1]->data_size() / sizeof(T);
    } else {
      node_count_ = 0U;
    }

    return 0;
  }

  int update_header() {
    size_t write_len = header_block_->write(0, &header_, sizeof(Header));
    if (write_len != sizeof(Header)) {
      return ErrorCode_WriteData;
    }
    return 0;
  }

 private:
  static constexpr uint32_t kNodeCountPerBlock = 1UL * 1024UL * 1024UL;

 private:
  IndexStoragePtr storage_{};

  IndexBlockPtr header_block_{};
  std::vector<IndexBlockPtr> data_blocks_{};
  Header header_;

  std::mutex mutex_{};
  std::atomic<uint64_t> node_count_{0UL};
};


}  // end namespace index
}  // namespace be
}  // end namespace proxima
