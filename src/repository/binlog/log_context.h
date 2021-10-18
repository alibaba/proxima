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

 *   \author   Hongqing.hu
 *   \date     Nov 2020
 *   \brief    Log context interface definition for proxima search engine
 */

#pragma once

#include <memory>
#include "binlog_event.h"

namespace proxima {
namespace be {
namespace repository {

class LogContext;
using LogContextPtr = std::shared_ptr<LogContext>;

/*! Log Context
 */
class LogContext {
 public:
  //! Constructor
  LogContext() = default;

  //! Constructor
  ~LogContext() = default;

  //! Get table map event
  TableMapEventPtr table_map() const {
    return table_map_;
  }

  //! Get Position
  uint64_t position() {
    return position_;
  }

  //! Get file name
  const std::string &file_name() {
    return file_name_;
  }

  //! Update table map event
  void update_table_map(TableMapEventPtr event) {
    table_map_ = std::move(event);
  }

  //! Update lsn
  void update_lsn(const std::string &name, uint64_t pos) {
    position_ = pos;
    file_name_ = name;
  }

 private:
  //! Table map event
  TableMapEventPtr table_map_{};
  //! Bin log position
  uint64_t position_{0};
  //! Bin log file name
  std::string file_name_{};
};

}  // end namespace repository
}  // namespace be
}  // end namespace proxima
