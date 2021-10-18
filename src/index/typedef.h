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
 *   \brief    Includes some micro definitions and alias using names
 */

#pragma once

#include <memory>
#include <string>
#include <aitheta2/index_framework.h>
#include "common/error_code.h"
#include "common/logger.h"
#include "common/types.h"

namespace proxima {
namespace be {
namespace index {

using idx_t = uint64_t;
using SegmentID = uint32_t;

using IndexStoragePtr = aitheta2::IndexStorage::Pointer;
using IndexBlockPtr = aitheta2::IndexStorage::Segment::Pointer;
using IndexDumperPtr = aitheta2::IndexDumper::Pointer;
using IndexContainerPtr = aitheta2::IndexContainer::Pointer;
using IndexContextPtr = aitheta2::IndexContext::Pointer;
using IndexSearcherPtr = aitheta2::IndexSearcher::Pointer;
using IndexContainerBlockPtr = aitheta2::IndexContainer::Segment::Pointer;
using IndexStreamerPtr = aitheta2::IndexStreamer::Pointer;
using ThreadPoolPtr = std::shared_ptr<aitheta2::SingleQueueIndexThreads>;
using IndexReformerPtr = std::shared_ptr<aitheta2::IndexReformer>;
using IndexMeasurePtr = std::shared_ptr<aitheta2::IndexMeasure>;
using IndexConverterPtr = std::shared_ptr<aitheta2::IndexConverter>;
using IndexClosetPtr = std::shared_ptr<aitheta2::IndexCloset>;
using IndexImmutableClosetPtr = std::shared_ptr<aitheta2::IndexImmutableCloset>;

using FeatureTypes = aitheta2::IndexMeta::FeatureTypes;
using IndexBlock = aitheta2::IndexStorage::Segment;
using IndexMeta = aitheta2::IndexMeta;
using IndexQueryMeta = aitheta2::IndexQueryMeta;
using IndexParams = aitheta2::IndexParams;
using IndexDocumentList = aitheta2::IndexDocumentList;
using IndexSegmentDumper = aitheta2::IndexSegmentDumper;
using IndexFactory = aitheta2::IndexFactory;
using ThreadPool = aitheta2::SingleQueueIndexThreads;
using IndexStorage = aitheta2::IndexStorage;

}  // end namespace index
}  // namespace be
}  // end namespace proxima

#define COLLECTION_FORMAT " collection[%s] "

#define CLOG_DEBUG(format, ...) \
  LOG_DEBUG(format COLLECTION_FORMAT, ##__VA_ARGS__, collection_name().c_str())

#define CLOG_INFO(format, ...) \
  LOG_INFO(format COLLECTION_FORMAT, ##__VA_ARGS__, collection_name().c_str())

#define CLOG_WARN(format, ...) \
  LOG_WARN(format COLLECTION_FORMAT, ##__VA_ARGS__, collection_name().c_str())

#define CLOG_ERROR(format, ...) \
  LOG_ERROR(format COLLECTION_FORMAT, ##__VA_ARGS__, collection_name().c_str())

#define CLOG_FATAL(format, ...) \
  LOG_FATAL(format COLLECTION_FORMAT, ##__VA_ARGS__, collection_name().c_str())


#define SEGMENT_FORMAT " segment[%zu] collection[%s] "

#define SLOG_DEBUG(format, ...)                                         \
  LOG_DEBUG(format SEGMENT_FORMAT, ##__VA_ARGS__, (size_t)segment_id(), \
            collection_name().c_str())

#define SLOG_INFO(format, ...)                                         \
  LOG_INFO(format SEGMENT_FORMAT, ##__VA_ARGS__, (size_t)segment_id(), \
           collection_name().c_str())

#define SLOG_WARN(format, ...)                                         \
  LOG_WARN(format SEGMENT_FORMAT, ##__VA_ARGS__, (size_t)segment_id(), \
           collection_name().c_str())

#define SLOG_ERROR(format, ...)                                         \
  LOG_ERROR(format SEGMENT_FORMAT, ##__VA_ARGS__, (size_t)segment_id(), \
            collection_name().c_str())

#define SLOG_FATAL(format, ...)                                         \
  LOG_FATAL(format SEGMENT_FORMAT, ##__VA_ARGS__, (size_t)segment_id(), \
            collection_name().c_str())

#define COLUMN_FORMAT " column[%s] segment[%zu] collection[%s] "

#define LLOG_DEBUG(format, ...)                                         \
  LOG_DEBUG(format COLUMN_FORMAT, ##__VA_ARGS__, column_name().c_str(), \
            (size_t)segment_id(), collection_name().c_str())

#define LLOG_INFO(format, ...)                                         \
  LOG_INFO(format COLUMN_FORMAT, ##__VA_ARGS__, column_name().c_str(), \
           (size_t)segment_id(), collection_name().c_str())

#define LLOG_WARN(format, ...)                                         \
  LOG_WARN(format COLUMN_FORMAT, ##__VA_ARGS__, column_name().c_str(), \
           (size_t)segment_id(), collection_name().c_str())

#define LLOG_ERROR(format, ...)                                         \
  LOG_ERROR(format COLUMN_FORMAT, ##__VA_ARGS__, column_name().c_str(), \
            (size_t)segment_id(), collection_name().c_str())

#define LLOG_FATAL(format, ...)                                         \
  LOG_FATAL(format COLUMN_FORMAT, ##__VA_ARGS__, column_name().c_str(), \
            (size_t)segment_id(), collection_name().c_str())

#define CHECK_STATUS(status, expect)                                         \
  if (status != expect) {                                                    \
    LOG_ERROR("Check status failed. status[%d] expect[%d]", status, expect); \
    return ErrorCode_StatusError;                                            \
  }

#define CHECK_RETURN(ret, expect_ret) \
  if (ret != expect_ret) {            \
    return ret;                       \
  }

#define CHECK_RETURN_WITH_LOG(ret, expect_ret, format, ...) \
  if (ret != expect_ret) {                                  \
    LOG_ERROR(format, ##__VA_ARGS__);                       \
    return ret;                                             \
  }

#define CHECK_RETURN_WITH_CLOG(ret, expect_ret, format, ...) \
  if (ret != expect_ret) {                                   \
    CLOG_ERROR(format, ##__VA_ARGS__);                       \
    return ret;                                              \
  }

#define CHECK_RETURN_WITH_SLOG(ret, expect_ret, format, ...) \
  if (ret != expect_ret) {                                   \
    SLOG_ERROR(format, ##__VA_ARGS__);                       \
    return ret;                                              \
  }

#define CHECK_RETURN_WITH_LLOG(ret, expect_ret, format, ...) \
  if (ret != expect_ret) {                                   \
    LLOG_ERROR(format, ##__VA_ARGS__);                       \
    return ret;                                              \
  }
