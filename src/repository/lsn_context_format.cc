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
 *   \brief    Implementation of lsn context format
 */


#include "lsn_context_format.h"
#include <sstream>
#include "repository/repository_common/error_code.h"

namespace proxima {
namespace be {
namespace repository {

LsnContextFormat::LsnContextFormat(const LsnContextFormat &other) {
  file_name_ = other.file_name_;
  position_ = other.position_;
  seq_id_ = other.seq_id_;
  mode_ = other.mode_;
}

LsnContextFormat &LsnContextFormat::operator=(const LsnContextFormat &other) {
  file_name_ = other.file_name_;
  position_ = other.position_;
  seq_id_ = other.seq_id_;
  mode_ = other.mode_;
  return *this;
}

int LsnContextFormat::parse_from_string(const std::string &lsn_context) {
  if (lsn_context.empty()) {
    return ErrorCode_InvalidLSNContext;
  }
  std::size_t start = 0;
  std::size_t pos = lsn_context.find(";", start);
  if (pos != std::string::npos) {
    file_name_ = lsn_context.substr(start, pos - start);
    start = pos + 1;
  } else {
    return ErrorCode_InvalidLSNContext;
  }

  pos = lsn_context.find(";", start);
  if (pos != std::string::npos) {
    try {
      position_ = std::stoull(lsn_context.substr(start, pos - start));
    } catch (...) {
      return ErrorCode_InvalidLSNContext;
    }
    start = pos + 1;
  } else {
    return ErrorCode_InvalidLSNContext;
  }

  pos = lsn_context.find(";", start);
  if (pos != std::string::npos) {
    try {
      seq_id_ = std::stoull(lsn_context.substr(start, pos - start));
    } catch (...) {
      return ErrorCode_InvalidLSNContext;
    }
    start = pos + 1;
  } else {
    return ErrorCode_InvalidLSNContext;
  }

  unsigned long scan_mode = (unsigned long)ScanMode::FULL;
  try {
    scan_mode = std::stoul(lsn_context.substr(start));
  } catch (...) {
    return ErrorCode_InvalidLSNContext;
  }
  mode_ = static_cast<ScanMode>(scan_mode);
  return 0;
}

std::string LsnContextFormat::convert_to_string() const {
  std::stringstream ss;
  ss << file_name_ << ";" << position_ << ";" << seq_id_ << ";"
     << (uint32_t)mode_;
  return ss.str();
}

const std::string &LsnContextFormat::file_name() const {
  return file_name_;
}

uint64_t LsnContextFormat::position() const {
  return position_;
}

uint64_t LsnContextFormat::seq_id() const {
  return seq_id_;
}

const ScanMode &LsnContextFormat::mode() const {
  return mode_;
}

}  // end namespace repository
}  // namespace be
}  // end namespace proxima
