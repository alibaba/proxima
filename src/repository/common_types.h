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

 *   \author   Dianzhang.Chen
 *   \date     Jan 2021
 *   \brief    This file includes some baic & common functions
 */

#pragma once

#include <string>
#include <aitheta2/index_framework.h>
#include "proto/common.pb.h"
#include "proto/proxima_be.pb.h"

namespace proxima {
namespace be {
namespace repository {

const uint64_t UPDATE_INTERVAL{1};

enum class CollectionStateFlag : uint32_t { NORMAL = 0, UPDATE, DROP };
enum class CollectionStatus : uint32_t {
  INIT = 0,
  RUNNING,
  UPDATING,
  FINISHED
};

enum class ScanMode : uint32_t { FULL = 0, INCREMENTAL };
enum RowDataStatus : uint32_t { NORMAL = 0, NO_MORE_DATA, SCHEMA_CHANGED };

#define INVALID_PRIMARY_KEY std::numeric_limits<uint64_t>::max()

using CollectionConfig = proto::CollectionConfig;
using WriteRequest = proto::WriteRequest;

using GenericValue = proto::GenericValue;
using FieldType = proto::GenericValueMeta::FieldType;

using GenericValueMeta = proxima::be::proto::GenericValueMeta;
using GenericValueMetaList =
    google::protobuf::RepeatedPtrField<proxima::be::proto::GenericValueMeta>;

// new
using CollectionInfo = proto::CollectionInfo;

struct LsnContext {
  //! binlog file name
  std::string file_name;
  //! binlog position
  uint64_t position;
  //! table sequence id
  uint64_t seq_id;
  //! row data status
  RowDataStatus status;

  LsnContext()
      : file_name(""), position(4), seq_id(0), status(RowDataStatus::NORMAL) {}
};

}  // end namespace repository
}  // namespace be
}  // end namespace proxima
