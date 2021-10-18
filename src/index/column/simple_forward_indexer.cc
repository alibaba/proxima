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
 *   \brief    Implementation of simple forward indexer
 */

#include "simple_forward_indexer.h"
#include "common/error_code.h"
#include "forward_indexer.h"
#include "../file_helper.h"

namespace proxima {
namespace be {
namespace index {

SimpleForwardIndexer::~SimpleForwardIndexer() {
  if (opened_) {
    this->close();
  }
}

int SimpleForwardIndexer::open(const ReadOptions &read_options) {
  CHECK_STATUS(opened_, false);

  int ret =
      Snapshot::CreateAndOpen(this->collection_path(), FileID::FORWARD_FILE,
                              this->segment_id(), read_options, &snapshot_);
  CHECK_RETURN_WITH_SLOG(ret, 0, "Create snapshot failed.");

  ret = open_proxima_forward();
  CHECK_RETURN_WITH_SLOG(ret, 0, "Open proxima forward failed.");

  opened_ = true;
  return 0;
}

int SimpleForwardIndexer::close() {
  CHECK_STATUS(opened_, true);

  proxima_forward_->close();
  int ret = snapshot_->close();
  if (ret != 0) {
    SLOG_WARN("Close snapshot failed.");
  }

  opened_ = false;
  return ret;
}

int SimpleForwardIndexer::flush() {
  CHECK_STATUS(opened_, true);

  return proxima_forward_->flush(0);
}

int SimpleForwardIndexer::dump(IndexDumperPtr dumper) {
  CHECK_STATUS(opened_, true);

  return proxima_forward_->dump(dumper);
}

int SimpleForwardIndexer::insert(const ForwardData &forward_data,
                                 idx_t *doc_id) {
  CHECK_STATUS(opened_, true);

  uint64_t index = 0U;
  std::string buffer;
  forward_data.serialize(&buffer);
  uint64_t key = forward_data.header.primary_key;
  int ret = proxima_forward_->append(buffer.data(), buffer.size(), &index);
  CHECK_RETURN_WITH_SLOG(ret, 0, "Append forward failed. key[%zu] ret[%d]",
                         (size_t)key, ret);

  *doc_id = this->start_doc_id() + index;
  return 0;
}

#if 0
int SimpleForwardIndexer::update(idx_t doc_id,
                                 const ForwardData &forward_data) {
  CHECK_STATUS(opened_, true);

  uint64_t key = forward_data.header.primary_key;
  uint64_t index = doc_id - this->start_doc_id();
  std::string buffer;
  forward_data.serialize(&buffer);
  int ret = proxima_forward_->update(index, buffer.data(), buffer.size());
  CHECK_RETURN_WITH_SLOG(
      ret, 0,
      "Update forward data failed. key[%zu] doc_id[%zu] index[%zu] ret[%d]",
      (size_t)key, (size_t)doc_id, (size_t)index, ret);

  return 0;
}
#endif

int SimpleForwardIndexer::remove(idx_t doc_id) {
  CHECK_STATUS(opened_, true);

  uint64_t index = doc_id - this->start_doc_id();
  int ret = proxima_forward_->erase(index);
  CHECK_RETURN_WITH_SLOG(
      ret, 0, "Remove forward data failed. doc_id[%zu] index[%zu] ret[%d]",
      (size_t)doc_id, (size_t)index, ret);

  return 0;
}

int SimpleForwardIndexer::seek(idx_t doc_id, ForwardData *forward_data) {
  CHECK_STATUS(opened_, true);

  uint64_t index = doc_id - this->start_doc_id();
  std::string buffer;
  int ret = proxima_forward_->fetch(index, &buffer);
  CHECK_RETURN_WITH_SLOG(
      ret, 0, "Forward store fetch failed. doc_id[%zu] index[%zu] ret[%d]",
      (size_t)doc_id, (size_t)index, ret);

  forward_data->deserialize(buffer);
  return 0;
}

int SimpleForwardIndexer::open_proxima_forward() {
  proxima_forward_ = aitheta2::IndexFactory::CreateCloset("ChainCloset");
  if (!proxima_forward_) {
    SLOG_ERROR("Create proxima forward failed.");
    return ErrorCode_RuntimeError;
  }

  aitheta2::IndexParams params;
  params.set("proxima.chain.closet.slot_size", 128);
  int ret = proxima_forward_->init(params);
  CHECK_RETURN_WITH_SLOG(ret, 0, "Init proxima forward failed.");

  ret = proxima_forward_->open(snapshot_->data());
  CHECK_RETURN_WITH_SLOG(ret, 0, "Open proxima forward failed. ret[%d]", ret);

  return 0;
}


}  // end namespace index
}  // namespace be
}  // end namespace proxima
