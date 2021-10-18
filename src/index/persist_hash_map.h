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

 *   \author   zhongyou.dlx
 *   \date     Jun 2021
 *   \brief    Persist hashmap which is thread-safe for add/del operations
 */

#pragma once

#include <ailego/parallel/lock.h>
#include <ailego/utility/string_helper.h>
#include "common/error_code.h"
#include "common/macro_define.h"
#include "constants.h"
#include "typedef.h"

namespace proxima {
namespace be {
namespace index {

static constexpr uint32_t INVALID_NODE_ID{-1U};

/*
 * A PersistHashMap represents block of hash data in persist storage.
 */
template <typename TKey, typename TValue, typename Hash = std::hash<TKey> >
class PersistHashMap {
  struct BlockHeader {
    uint32_t bucket_count;
    uint32_t node_count;
    uint32_t free_header;
    uint32_t reserved[13];
  };

  static_assert(sizeof(BlockHeader) % 64 == 0,
                "BlockHeader must be aligned with 64 bytes");

  struct NodeType {
    NodeType(TKey key, TValue value)
        : first(key), second(value), next(INVALID_NODE_ID) {}

    TKey first;
    TValue second;
    uint32_t next;
  };

 public:
  PROXIMA_DISALLOW_COPY_AND_ASSIGN(PersistHashMap);

  //! Constructor
  PersistHashMap() = default;

  //! Destructor
  ~PersistHashMap() = default;

 public:
  //! Mount persist storage
  int mount(const IndexStoragePtr &stg) {
    if (!stg) {
      LOG_ERROR("Mount null storage");
      return ErrorCode_RuntimeError;
    }

    storage_ = stg;
    for (int i = 0; /* No Limit */; ++i) {
      std::string block_name = ailego::StringHelper::Concat(DATA_BLOCK, i);
      auto block = storage_->get(block_name);
      if (!block) break;

      const void *data = nullptr;
      if (ailego_unlikely(block->read(0, &data, sizeof(BlockHeader)) !=
                          sizeof(BlockHeader))) {
        LOG_ERROR("Failed to read block header from block idx %d", i);
        return ErrorCode_ReadData;
      }
      BlockHeader block_header = *static_cast<const BlockHeader *>(data);

      size_t bucket_count = block_header.bucket_count;
      size_t node_count = bucket_count * kLoadFactor;
      size_t block_size = sizeof(BlockHeader) +
                          bucket_count * sizeof(uint32_t) +
                          node_count * sizeof(NodeType);

      if (ailego_unlikely(block->capacity() < block_size)) {
        return ErrorCode_ReadData;
      }

      if (ailego_unlikely(block->data_size() != block_size)) {
        LOG_DEBUG("Block need reinit");
        int ret = init_block(block.get(), bucket_count);
        if (ret < 0) {
          return ret;
        }
      }

      blocks_.emplace_back(block);
      blocks_header_.emplace_back(block_header);
    }

    return ErrorCode_Success;
  }

  //! Unmount persist storage
  void unmount() {
    storage_ = nullptr;
    blocks_.clear();
    blocks_header_.clear();
  }

  //! Reserve
  int reserve(size_t bucket_count) {
    ailego::WriteLock wlock(mutex_);
    std::lock_guard<ailego::WriteLock> signal_lock(wlock);
    int ret = ErrorCode_Success;
    if (blocks_.empty()) {
      ret = add_block(bucket_count);
    }
    return ret;
  }

  //! Emplace a key-value pair
  int emplace(const TKey &key, const TValue &val) {
    ailego::WriteLock wlock(mutex_);
    std::lock_guard<ailego::WriteLock> signal_lock(wlock);

    size_t block_idx = -1UL;
    for (int idx = blocks_.size() - 1; idx >= 0; --idx) {
      if (blocks_header_[idx].free_header != INVALID_NODE_ID) {
        block_idx = idx;
        break;
      }
    }

    if (ailego_unlikely(block_idx == -1UL)) {
      int ret = add_block();
      if (ret < 0) {
        return ret;
      }
      block_idx = ret;
    }

    int ret = emplace_in_block(block_idx, key, val);
    return ret;
  }

  int emplace_or_assign(const TKey &key, const TValue &val) {
    ailego::WriteLock wlock(mutex_);
    std::lock_guard<ailego::WriteLock> signal_lock(wlock);
    int ret = find_key(key, [this, &val](const NodeType *node,
                                         uint32_t node_idx, const NodeType *,
                                         uint32_t, size_t, size_t block_idx) {
      NodeType writable_node = *node;
      writable_node.second = val;
      size_t offset =
          sizeof(BlockHeader) +
          blocks_header_[block_idx].bucket_count * sizeof(uint32_t) +
          node_idx * sizeof(NodeType);
      if (ailego_unlikely(blocks_[block_idx]->write(offset, &writable_node,
                                                    sizeof(NodeType)) !=
                          sizeof(NodeType))) {
        LOG_ERROR("Failed to write node content for block idx %zu", block_idx);
        return ErrorCode_WriteData;
      }
      return ErrorCode_Success;
    });

    if (ret == ErrorCode_InexistentKey) {
      ret = emplace(key, val);
    }
    return ret;
  }

  //! Get value by key
  int get(const TKey &key, TValue *value) const {
    ailego::ReadLock rlock(mutex_);
    std::lock_guard<ailego::ReadLock> signal_lock(rlock);
    int ret =
        find_key(key, [&value](const NodeType *node, uint32_t, const NodeType *,
                               uint32_t, size_t, size_t) {
          *value = node->second;
          return ErrorCode_Success;
        });
    return ret;
  }

  //! If has key
  bool has(const TKey &key) const {
    ailego::ReadLock rlock(mutex_);
    std::lock_guard<ailego::ReadLock> signal_lock(rlock);
    int ret =
        find_key(key, [](const NodeType *, uint32_t, const NodeType *, uint32_t,
                         size_t, size_t) { return ErrorCode_Success; });
    return (ret == ErrorCode_Success);
  }

  //! Return key-value pair count
  size_t size() const {
    ailego::ReadLock rlock(mutex_);
    std::lock_guard<ailego::ReadLock> signal_lock(rlock);
    size_t val = 0;
    for (int i = 0; i < (int)blocks_header_.size(); ++i) {
      val += blocks_header_[i].node_count;
    }
    return val;
  }

  //! Erase a pair by key
  int erase(const TKey &key) {
    ailego::WriteLock wlock(mutex_);
    std::lock_guard<ailego::WriteLock> signal_lock(wlock);
    int ret = find_key(key, [this](const NodeType *node, uint32_t node_idx,
                                   const NodeType *pre_node,
                                   uint32_t pre_node_idx, size_t bucket_offset,
                                   size_t block_idx) {
      auto &block = blocks_[block_idx];

      // free node
      if (pre_node == nullptr) {
        if (ailego_unlikely(
                block->write(bucket_offset, &(node->next), sizeof(uint32_t)) !=
                sizeof(uint32_t))) {
          LOG_ERROR("Failed to write bucket content for block idx %zu",
                    block_idx);
          return ErrorCode_WriteData;
        }
      } else {
        NodeType writable_node = *pre_node;
        writable_node.next = node->next;
        size_t offset =
            sizeof(BlockHeader) +
            blocks_header_[block_idx].bucket_count * sizeof(uint32_t) +
            pre_node_idx * sizeof(NodeType);
        if (ailego_unlikely(
                block->write(offset, &writable_node, sizeof(NodeType)) !=
                sizeof(NodeType))) {
          LOG_ERROR("Failed to write node content for block idx %zu",
                    block_idx);
          return ErrorCode_WriteData;
        }
      }

      // recycle node
      NodeType writable_node = *node;
      writable_node.next = blocks_header_[block_idx].free_header;
      size_t offset =
          sizeof(BlockHeader) +
          blocks_header_[block_idx].bucket_count * sizeof(uint32_t) +
          node_idx * sizeof(NodeType);
      if (ailego_unlikely(block->write(offset, &writable_node,
                                       sizeof(NodeType)) != sizeof(NodeType))) {
        LOG_ERROR("Failed to write node content for block idx %zu", block_idx);
        return ErrorCode_WriteData;
      }

      blocks_header_[block_idx].free_header = node_idx;
      blocks_header_[block_idx].node_count -= 1;
      if (ailego_unlikely(block->write(0, &blocks_header_[block_idx],
                                       sizeof(BlockHeader)) !=
                          sizeof(BlockHeader))) {
        LOG_ERROR("Failed to write block header for block idx %zu", block_idx);
        return ErrorCode_WriteData;
      }

      return ErrorCode_Success;
    });

    return ret;
  }

 private:
  size_t constrain_hash(size_t hash, size_t block_capacity) const {
    size_t slot = hash % block_capacity;
    return sizeof(BlockHeader) + slot * sizeof(uint32_t);
  }

  int find_key(const TKey &key,
               std::function<int(const NodeType *, uint32_t, const NodeType *,
                                 uint32_t, size_t, size_t)>
                   fun) const {
    const uint64_t hash = hasher_(key);
    for (int idx = blocks_.size() - 1; idx >= 0; --idx) {
      auto &block = blocks_[idx];
      size_t bucket_offset =
          constrain_hash(hash, blocks_header_[idx].bucket_count);
      ailego_assert_with(bucket_offset < block->data_size(), "Invalid Offset");

      const NodeType *pre_node = nullptr;
      uint32_t pre_node_idx = INVALID_NODE_ID;

      const void *data = nullptr;
      if (ailego_unlikely(block->read(bucket_offset, &data, sizeof(uint32_t)) !=
                          sizeof(uint32_t))) {
        LOG_ERROR("Failed to read bucket content from block idx %d", idx);
        return ErrorCode_ReadData;
      }
      uint32_t bucket_idx = *static_cast<const uint32_t *>(data);
      uint32_t next = bucket_idx;
      while (next != INVALID_NODE_ID) {
        size_t offset = sizeof(BlockHeader) +
                        blocks_header_[idx].bucket_count * sizeof(uint32_t) +
                        next * sizeof(NodeType);
        if (ailego_unlikely(block->read(offset, &data, sizeof(NodeType)) !=
                            sizeof(NodeType))) {
          LOG_ERROR("Failed to read node content from block idx %d", idx);
          return ErrorCode_ReadData;
        }
        const NodeType *node = static_cast<const NodeType *>(data);
        if (node->first == key) {
          int ret = fun(node, next, pre_node, pre_node_idx, bucket_offset, idx);
          if (ailego_unlikely(ret < 0)) {
            return ret;
          }
          return ErrorCode_Success;
        } else {
          pre_node_idx = next;
          pre_node = node;
          next = node->next;
        }
      }
    }
    return ErrorCode_InexistentKey;
  }

  int emplace_in_block(size_t block_idx, const TKey &key, const TValue &value) {
    auto &block = blocks_[block_idx];

    uint32_t free_idx = blocks_header_[block_idx].free_header;
    uint32_t bucket_count = blocks_header_[block_idx].bucket_count;

    // alloc Node
    const void *data = nullptr;
    size_t free_offset = sizeof(BlockHeader) + bucket_count * sizeof(uint32_t) +
                         free_idx * sizeof(NodeType);
    if (ailego_unlikely(block->read(free_offset, &data, sizeof(NodeType)) !=
                        sizeof(NodeType))) {
      LOG_ERROR("Failed to read node content from block idx %zu", block_idx);
      return ErrorCode_ReadData;
    }
    const NodeType *free_node = static_cast<const NodeType *>(data);
    blocks_header_[block_idx].free_header = free_node->next;
    blocks_header_[block_idx].node_count += 1;

    // write node
    const uint64_t hash = hasher_(key);
    size_t bucket_offset =
        constrain_hash(hash, blocks_header_[block_idx].bucket_count);
    if (ailego_unlikely(block->read(bucket_offset, &data, sizeof(uint32_t)) !=
                        sizeof(uint32_t))) {
      LOG_ERROR("Failed to read bucket content from block idx %zu", block_idx);
      return ErrorCode_ReadData;
    }
    NodeType node(key, value);
    node.next = *static_cast<const uint32_t *>(data);
    if (ailego_unlikely(block->write(free_offset, &node, sizeof(NodeType)) !=
                        sizeof(NodeType))) {
      LOG_ERROR("Failed to write node content for block idx %zu", block_idx);
      return ErrorCode_WriteData;
    }

    // update bucket hash link
    if (ailego_unlikely(block->write(bucket_offset, &free_idx,
                                     sizeof(uint32_t)) != sizeof(uint32_t))) {
      LOG_ERROR("Failed to write bucket content for block idx %zu", block_idx);
      return ErrorCode_WriteData;
    }

    // update block header
    if (ailego_unlikely(
            block->write(0, &blocks_header_[block_idx], sizeof(BlockHeader)) !=
            sizeof(BlockHeader))) {
      LOG_ERROR("Failed to write block header for block idx %zu", block_idx);
      return ErrorCode_WriteData;
    }

    return ErrorCode_Success;
  }

  int add_block(size_t bucket_count = 0) {
    uint32_t block_idx = blocks_.size();
    std::string block_name =
        ailego::StringHelper::Concat(DATA_BLOCK, block_idx);
    if (bucket_count == 0) {
      bucket_count = (block_idx == 0)
                         ? kInitBucketCount
                         : blocks_header_[block_idx - 1].bucket_count * 2;
    } else {
      bucket_count = pow(2, ceil(log2(bucket_count)));
    }
    size_t node_count = bucket_count * kLoadFactor;

    if (node_count >= INVALID_NODE_ID) {
      bucket_count = pow(2, floor(log2(INVALID_NODE_ID / kLoadFactor)));
      node_count = bucket_count * kLoadFactor;
    }

    size_t block_size = sizeof(BlockHeader) + bucket_count * sizeof(uint32_t) +
                        node_count * sizeof(NodeType);
    int ret = storage_->append(block_name, block_size);
    if (ret != 0) {
      LOG_ERROR("Failed to append block %s for %s, size %zu",
                block_name.c_str(), aitheta2::IndexError::What(ret),
                block_size);
      return ret;
    }
    IndexBlockPtr block = storage_->get(block_name);

    ret = init_block(block.get(), bucket_count);
    if (ret != 0) {
      LOG_ERROR("Failed to init new block");
      return ret;
    }
    blocks_.emplace_back(block);

    BlockHeader block_header;
    block_header.bucket_count = bucket_count;
    block_header.node_count = 0;
    block_header.free_header = 0;
    blocks_header_.emplace_back(block_header);

    LOG_DEBUG("Add new block with bucket count[%zu]", bucket_count);
    return block_idx;
  }

  int init_block(IndexBlock *block, uint32_t bucket_count) {
    if (block == nullptr) {
      return ErrorCode_WriteData;
    }
    LOG_DEBUG("Init block with bucket count[%u]", bucket_count);

    BlockHeader block_header;
    block_header.bucket_count = bucket_count;
    block_header.node_count = 0;
    block_header.free_header = 0;
    if (ailego_unlikely(block->write(0, &block_header, sizeof(BlockHeader)) !=
                        sizeof(BlockHeader))) {
      LOG_ERROR("Failed to fill block header");
      return ErrorCode_WriteData;
    }
    std::vector<uint32_t> buckets(bucket_count, INVALID_NODE_ID);
    size_t buckets_size = buckets.size() * sizeof(uint32_t);
    if (ailego_unlikely(block->write(sizeof(BlockHeader), buckets.data(),
                                     buckets_size) != buckets_size)) {
      LOG_ERROR("Failed to fill block buckets");
      return ErrorCode_WriteData;
    }

    size_t offset = sizeof(BlockHeader) + buckets_size;
    NodeType empty(0, TValue());
    size_t empty_size = sizeof(NodeType);
    size_t node_count = bucket_count * kLoadFactor;
    for (size_t i = 0; i < node_count; ++i) {
      empty.next = i == (node_count - 1) ? INVALID_NODE_ID : i + 1;
      if (ailego_unlikely(block->write(offset, &empty, empty_size) !=
                          empty_size)) {
        LOG_ERROR("Failed to fill block nodes");
        return ErrorCode_WriteData;
      };
      offset += empty_size;
    }

    return ErrorCode_Success;
  }

 private:
  static constexpr uint32_t kInitBucketCount{1U * 1024U};
  static constexpr double kLoadFactor{1.0};

 private:
  mutable ailego::SharedMutex mutex_{};

  Hash hasher_{};
  IndexStoragePtr storage_{};
  std::vector<IndexBlockPtr> blocks_{};
  std::vector<BlockHeader> blocks_header_{};
};


}  // end namespace index
}  // namespace be
}  // end namespace proxima
