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
 *   \brief    Definitions of some constants
 */

#pragma once

#include <string>

namespace proxima {
namespace be {
namespace index {

const std::string HEADER_BLOCK("HeaderBlock");
const std::string DATA_BLOCK("DataBlock");

const std::string SUMMARY_BLOCK("SummaryBlock");
const std::string VERSION_BLOCK("VersionBlock");
const std::string SEGMENT_BLOCK("SegmentBlock");

const std::string FORWARD_DUMP_BLOCK("ForwardIndex");
const std::string COLUMN_DUMP_BLOCK("ColumnIndex");

const uint64_t INVALID_KEY = -1UL;
const uint64_t INVALID_DOC_ID = -1UL;
const uint32_t INVALID_SEGMENT_ID = -1U;

}  // end namespace index
}  // namespace be
}  // end namespace proxima
