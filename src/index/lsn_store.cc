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
 *   \brief    Implementation of lsn store
 */

#include "lsn_store.h"
#include "common/error_code.h"
#include "constants.h"
#include "file_helper.h"

namespace proxima {
namespace be {
namespace index {

LsnStorePtr LsnStore::Create(const std::string &collection_name,
                             const std::string &collection_path) {
  return std::make_shared<LsnStore>(collection_name, collection_path);
}

int LsnStore::CreateAndOpen(const std::string &collection_name,
                            const std::string &collection_path,
                            const ReadOptions &read_options,
                            LsnStorePtr *lsn_store) {
  (*lsn_store) = Create(collection_name, collection_path);

  return (*lsn_store)->open(read_options);
}

LsnStore::~LsnStore() {
  if (opened_) {
    this->close();
  }
}

int LsnStore::open(const ReadOptions &read_options) {
  CHECK_STATUS(opened_, false);

  int ret = Snapshot::CreateAndOpen(collection_path_, FileID::LSN_FILE,
                                    read_options, &snapshot_);
  CHECK_RETURN_WITH_CLOG(ret, 0, "Create snapshot failed.");

  ret = this->mount();
  CHECK_RETURN_WITH_CLOG(ret, 0, "Mount storage failed.");

  opened_ = true;
  CLOG_DEBUG("Opened lsn store.");
  return 0;
}

int LsnStore::flush() {
  CHECK_STATUS(opened_, true);

  return snapshot_->flush();
}

int LsnStore::close() {
  CHECK_STATUS(opened_, true);
  this->unmount();

  int ret = snapshot_->close();
  if (ret != 0) {
    LOG_WARN("LsnStore close snapshot failed.");
  }

  opened_ = false;
  CLOG_DEBUG("Closed lsn store");
  return 0;
}

int LsnStore::append(uint64_t lsn, const std::string &lsn_context) {
  CHECK_STATUS(opened_, true);

  ailego::WriteLock wlock(mutex_);
  std::lock_guard<ailego::WriteLock> lock(wlock);

  uint64_t expect_write_len =
      sizeof(uint64_t) + sizeof(uint64_t) + lsn_context.size();
  if (expect_write_len > kDataBlockSize) {
    return ErrorCode_ExceedLimit;
  }

  uint32_t block_index = header_.tail_block_index;
  auto data_block = data_blocks_[block_index];

  if (data_block->padding_size() < expect_write_len) {
    block_index = (block_index + 1) % 2;
    data_block = data_blocks_[block_index];
    data_block->resize(0);
    header_.tail_block_index = block_index;
    update_header();
  }

  size_t write_len =
      data_block->write(data_block->data_size(), &lsn, sizeof(uint64_t));
  if (write_len != sizeof(uint64_t)) {
    return ErrorCode_WriteData;
  }

  uint64_t lsn_context_len = lsn_context.size();
  write_len = data_block->write(data_block->data_size(), &lsn_context_len,
                                sizeof(uint64_t));
  if (write_len != sizeof(uint64_t)) {
    return ErrorCode_WriteData;
  }

  write_len = data_block->write(data_block->data_size(), lsn_context.data(),
                                lsn_context.size());
  if (write_len != lsn_context.size()) {
    return ErrorCode_WriteData;
  }

  header_.lsn_count++;
  update_header();
  return 0;
}

int LsnStore::shift() {
  CHECK_STATUS(opened_, true);

  ailego::ReadLock rlock(mutex_);
  std::lock_guard<ailego::ReadLock> lock(rlock);

  // find the writing data block
  uint32_t block_index = header_.tail_block_index;
  if (data_blocks_[block_index]->data_size() == 0U) {
    block_index = (block_index + 1) % 2;
  }

  uint64_t expect_shift_len = data_blocks_[block_index]->data_size();
  if (expect_shift_len == 0U) {
    return ErrorCode_ReadData;
  }

  auto src_data_block = data_blocks_[block_index];

  // copy and shift data block
  const void *lsn_data;
  size_t read_len = data_blocks_[block_index]->read(
      0, &lsn_data, data_blocks_[block_index]->data_size());
  if (read_len != data_blocks_[block_index]->data_size()) {
    return ErrorCode_ReadData;
  }

  data_blocks_[2]->resize(0);
  size_t write_len = data_blocks_[2]->write(0, lsn_data, read_len);
  if (write_len != read_len) {
    return ErrorCode_WriteData;
  }

  return 0;
}

int LsnStore::get_latest_lsn(uint64_t *lsn, std::string *lsn_context) {
  CHECK_STATUS(opened_, true);

  ailego::ReadLock rlock(mutex_);
  std::lock_guard<ailego::ReadLock> lock(rlock);

  ailego::Heap<LSN, std::greater<LSN>> lsn_heap(kWindowSize);
  // scan lsn data blocks
  for (size_t i = 0; i < data_blocks_.size(); i++) {
    auto data_block = data_blocks_[i];
    uint64_t offset = 0;
    while (offset < data_block->data_size()) {
      uint64_t lsn_val = 0U;
      size_t read_len = data_block->fetch(offset, &lsn_val, sizeof(uint64_t));
      if (read_len != sizeof(uint64_t)) {
        return ErrorCode_ReadData;
      }
      offset += sizeof(uint64_t);

      uint64_t lsn_context_len;
      read_len = data_block->fetch(offset, &lsn_context_len, sizeof(uint64_t));
      if (read_len != sizeof(uint64_t)) {
        return ErrorCode_ReadData;
      }
      offset += sizeof(uint64_t);

      std::string lsn_context_val;
      lsn_context_val.resize(lsn_context_len);
      read_len = data_block->fetch(offset, (void *)lsn_context_val.data(),
                                   lsn_context_len);
      offset += lsn_context_len;

      lsn_heap.emplace(LSN(lsn_val, lsn_context_val));
    }
  }

  // from small to large
  std::sort(lsn_heap.begin(), lsn_heap.end());

  // find last not continues lsn
  LSN max_lsn;
  bool found = false;
  for (size_t i = 0; i + 1 < lsn_heap.size(); i++) {
    if (lsn_heap[i + 1].lsn > lsn_heap[i].lsn + 1) {
      max_lsn = lsn_heap[i];
      found = true;
      break;
    }
  }

  // if not find, just pick up last lsn
  if (!found && lsn_heap.size() > 0) {
    max_lsn = *(lsn_heap.rbegin());
  }

  *lsn = max_lsn.lsn;
  *lsn_context = max_lsn.lsn_context;
  return 0;
}

int LsnStore::mount() {
  auto &storage = snapshot_->data();

  header_block_ = storage->get(HEADER_BLOCK);
  int ret = 0;
  if (!header_block_) {
    ret = storage->append(HEADER_BLOCK, sizeof(Header));
    CHECK_RETURN(ret, 0);
    header_block_ = storage->get(HEADER_BLOCK);
    ret = update_header();
    CHECK_RETURN(ret, 0);

    for (uint32_t i = 0; i < kDataBlockCount; i++) {
      auto block_id = ailego::StringHelper::Concat(DATA_BLOCK, i);
      ret = storage->append(block_id, kDataBlockSize);
      CHECK_RETURN(ret, 0);
      auto data_block = storage->get(block_id);
      data_blocks_.emplace_back(data_block);
    }
  } else {
    size_t read_len = header_block_->fetch(0, &header_, sizeof(Header));
    if (read_len != sizeof(Header)) {
      return ErrorCode_ReadData;
    }

    for (uint32_t i = 0; i < kDataBlockCount; i++) {
      auto data_block =
          storage->get(ailego::StringHelper::Concat(DATA_BLOCK, i));
      if (!data_block) {
        return ErrorCode_InvalidIndexDataFormat;
      }
      data_blocks_.emplace_back(data_block);
    }
  }

  return 0;
}

void LsnStore::unmount() {
  header_block_.reset();
  data_blocks_.clear();
  memset(&header_, 0, sizeof(header_));
}

int LsnStore::update_header() {
  size_t write_len = header_block_->write(0, &header_, sizeof(Header));
  if (write_len != sizeof(Header)) {
    return ErrorCode_WriteData;
  }

  return 0;
}


}  // end namespace index
}  // namespace be
}  // end namespace proxima
