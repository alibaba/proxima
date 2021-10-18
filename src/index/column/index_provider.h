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
 *   \date     Jun 2021
 *   \brief    IndexProvider provides column index information
 */

#pragma once

#include "meta/meta.h"
#include "../constants.h"
#include "../typedef.h"

namespace proxima {
namespace be {
namespace index {


/*
 * IndexProvider provides some detail info about the column or forward index.
 */
class IndexProvider {
 public:
  //! Destructor
  virtual ~IndexProvider() = default;

 public:
  //! Return collection name
  const std::string &collection_name() const {
    return collection_name_;
  }

  //! Return collection path
  const std::string &collection_path() const {
    return collection_path_;
  }

  //! Return segment id
  SegmentID segment_id() const {
    return segment_id_;
  }

  //! Return column name
  //! Forward will return ""
  const std::string &column_name() const {
    return column_name_;
  }

  //! Return document count
  virtual size_t doc_count() const = 0;

  //! Return index file path
  virtual std::string index_file_path() const = 0;

 protected:
  void set_collection_name(const std::string &val) {
    collection_name_ = val;
  }

  void set_collection_path(const std::string &val) {
    collection_path_ = val;
  }

  void set_segment_id(SegmentID val) {
    segment_id_ = val;
  }

  void set_column_name(const std::string &val) {
    column_name_ = val;
  }

 private:
  std::string collection_name_{};
  std::string collection_path_{};
  SegmentID segment_id_{INVALID_SEGMENT_ID};
  std::string column_name_{};
};


}  // end namespace index
}  // namespace be
}  // end namespace proxima
