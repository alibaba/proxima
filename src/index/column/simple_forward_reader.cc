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
 *   \brief    Implementation of forward searcher
 */

#include "simple_forward_reader.h"
#include "common/error_code.h"

namespace proxima {
namespace be {
namespace index {

SimpleForwardReader::~SimpleForwardReader() {
  // TODO  can't call virtual function
  if (opened_) {
    close();
  }
}

int SimpleForwardReader::open(const ReadOptions &read_options) {
  CHECK_STATUS(opened_, false);

  int ret = open_proxima_container(read_options);
  CHECK_RETURN(ret, 0);

  ret = open_forward_searcher();
  CHECK_RETURN(ret, 0);

  opened_ = true;
  return 0;
}

int SimpleForwardReader::close() {
  CHECK_STATUS(opened_, true);

  forward_searcher_->unload();
  opened_ = false;
  return 0;
}

int SimpleForwardReader::seek(idx_t doc_id, ForwardData *forward) {
  CHECK_STATUS(opened_, true);

  uint64_t index = doc_id - this->start_doc_id();
  std::string buffer;
  int ret = forward_searcher_->fetch(index, &buffer);
  CHECK_RETURN_WITH_SLOG(ret, 0, "Forward searcher fetch failed.");

  forward->deserialize(buffer);
  return 0;
}

int SimpleForwardReader::open_proxima_container(
    const ReadOptions &read_options) {
  index_file_path_ = FileHelper::MakeFilePath(
      this->collection_path(), FileID::SEGMENT_FILE, this->segment_id());

  if (read_options.use_mmap) {
    container_ = aitheta2::IndexFactory::CreateContainer("MMapFileContainer");
  } else {
    container_ = aitheta2::IndexFactory::CreateContainer("MemoryContainer");
  }

  // Default set warmup flag
  IndexParams container_params;
  container_params.set("proxima.mmap_file.container.memory_warmup", true);

  int ret = container_->init(container_params);
  CHECK_RETURN_WITH_LLOG(ret, 0, "Container init failed. ret[%d]", ret);

  ret = container_->load(index_file_path_);
  CHECK_RETURN_WITH_LLOG(ret, 0, "Container load failed. ret[%d] file[%s]", ret,
                         index_file_path_.c_str());

  return 0;
}

int SimpleForwardReader::open_forward_searcher() {
  forward_searcher_ =
      aitheta2::IndexFactory::CreateImmutableCloset("ChainImmutableCloset");
  if (!forward_searcher_) {
    SLOG_ERROR("Create proxima forward searcher failed.");
    return ErrorCode_RuntimeError;
  }

  auto forward_block = container_->get(FORWARD_DUMP_BLOCK);
  if (!forward_block) {
    LLOG_ERROR("Can't find forward block in index file");
    return ErrorCode_InvalidSegment;
  }

  auto block_container =
      std::make_shared<aitheta2::IndexSegmentContainer>(forward_block);
  int ret = block_container->load();
  CHECK_RETURN_WITH_SLOG(ret, 0, "Load forward container failed.");


  ret = forward_searcher_->load(block_container);
  CHECK_RETURN_WITH_SLOG(ret, 0,
                         "Load proxima forward searcher failed. ret[%d]", ret);

  return 0;
}


}  // end namespace index
}  // namespace be
}  // end namespace proxima
