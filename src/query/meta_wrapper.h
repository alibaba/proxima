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

#include "meta/meta_service.h"

namespace proxima {
namespace be {
namespace query {

//! Predefine class
class MetaWrapper;

//! Alias for ColumnNameList
using ColumnNameList = std::vector<std::string>;
using MetaWrapperPtr = std::shared_ptr<MetaWrapper>;


/*!
 * MetaWrapper
 */
class MetaWrapper {
 public:
  //! Constructor
  explicit MetaWrapper(meta::MetaServicePtr meta_service);

 public:
  //! validate collection and columns
  int validate(const std::string &collection,
               const ColumnNameList &columns) const;

  //! validate collection
  int validate_collection(const std::string &collection) const;

  //! validate collection and column
  int validate_column(const std::string &collection,
                      const std::string &column) const;

  //! list all the columns
  int list_columns(const std::string &collection, uint64_t revision,
                   ColumnNameList *columns) const;

  //! get data type by column
  DataTypes get_data_type(const std::string &collection,
                          const std::string &column);

 private:
  //! Meta service handler
  meta::MetaServicePtr meta_service_{nullptr};
};


}  // namespace query
}  // namespace be
}  // namespace proxima
