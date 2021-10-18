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
 *
 *   \author   guonix
 *   \date     Nov 2020
 *   \brief
 */

#pragma once

#include "index/index_service.h"
#include "proto/proxima_be.pb.h"
#include "meta_wrapper.h"

namespace proxima {
namespace be {
namespace query {

/*!
 * Helper object for forward serialization
 */
class ForwardSerializer {
 public:
  //! Deserialized forward buf
  static bool Deserialize(const std::string &buf,
                          proto::GenericValueList *values);

  //! Fill forward field
  static int FillForward(const index::QueryResult &forward,
                         const ColumnNameList &columns, proto::Document *doc);
};


}  // namespace query
}  // namespace be
}  // namespace proxima
