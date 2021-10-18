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
 *   \brief    Format of collection writing requests
 */

#pragma once

#include <memory>
#include <vector>
#include "common/macro_define.h"
#include "common/types.h"
#include "constants.h"

namespace proxima {
namespace be {
namespace index {

class CollectionDataset;
using CollectionDatasetPtr = std::shared_ptr<CollectionDataset>;

/*
 * CollectionDataset represents a batch of insert/update/delete requests.
 * A RowData represents a record, it consists of several ColumnData.
 * A ColumnData contains field name and field value.
 */
class CollectionDataset {
 public:
  //! ColumnData represents index column data
  struct ColumnData {
    std::string column_name{};
    DataTypes data_type{DataTypes::UNDEFINED};
    uint32_t dimension{0U};
    std::string data{};
  };

  //! A RowData contains serveral index columns and forward data
  struct RowData {
    OperationTypes operation_type{OperationTypes::UNDEFINED};
    uint64_t primary_key{0U};
    uint32_t revision{0U};
    uint64_t lsn{0U};
    std::string lsn_context{};
    bool lsn_check{false};
    uint64_t timestamp{0U};
    std::string forward_data{};
    std::vector<ColumnData> column_datas{};

    RowData() {
      primary_key = INVALID_KEY;
    }
  };

 public:
  //! Constructor
  CollectionDataset(uint32_t schema_rev) : schema_revision_(schema_rev) {}

  //! Destructor
  ~CollectionDataset() = default;

 public:
  //! Add a row data
  RowData *add_row_data();

  //! Return a row
  const RowData &get(size_t i) const;

  //! Return row size
  size_t size() const {
    return records_.size();
  }

  //! Return if empty
  bool empty() const {
    return records_.empty();
  }

  //! Clear rows
  void clear() {
    records_.clear();
  }

  //! Get schema revision
  uint32_t get_schema_revision() const {
    return schema_revision_;
  }

 private:
  uint32_t schema_revision_{0U};
  std::vector<RowData> records_{};
};

using Record = CollectionDataset::RowData;
using ColumnData = CollectionDataset::ColumnData;

}  // end namespace index
}  // namespace be
}  // end namespace proxima
