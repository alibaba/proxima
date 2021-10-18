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
 *   \date     Mar 2021
 *   \brief    Interface of lsn context format
 */

#pragma once

#include "common_types.h"

namespace proxima {
namespace be {
namespace repository {

/*! Lsn Context Format
 */
class LsnContextFormat {
 public:
  //! Constructor
  LsnContextFormat() = default;

  //! Constructor
  LsnContextFormat(const std::string &name, uint64_t pos, uint64_t id,
                   const ScanMode &scan_mode)
      : file_name_(name), position_(pos), seq_id_(id), mode_(scan_mode) {}

  //! Destructor
  ~LsnContextFormat() = default;

  //! Copy constructor
  LsnContextFormat(const LsnContextFormat &);

  //! Assignment constructor
  LsnContextFormat &operator=(const LsnContextFormat &);

  //! Parse lsn context from string
  int parse_from_string(const std::string &lsn_context);

  //! Convert lsn context store to string
  std::string convert_to_string() const;

  //! Get binlog file name
  const std::string &file_name() const;

  //! Get binlog position
  uint64_t position() const;

  //! Get sequence id
  uint64_t seq_id() const;

  //! Get scan mode
  const ScanMode &mode() const;

 private:
  std::string file_name_{};
  uint64_t position_{0};
  uint64_t seq_id_{0};
  ScanMode mode_{ScanMode::FULL};
};

}  // end namespace repository
}  // namespace be
}  // end namespace proxima
