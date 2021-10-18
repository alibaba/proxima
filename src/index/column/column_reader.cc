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
 *   \brief    ColumnReader load column index data and provides read ability
 */

#include "column_reader.h"
#include "vector_column_reader.h"

namespace proxima {
namespace be {
namespace index {

ColumnReaderPtr ColumnReader::Create(const std::string &collection_name,
                                     const std::string &collection_path,
                                     SegmentID segment_id,
                                     const std::string &column_name,
                                     IndexTypes index_type) {
  switch (index_type) {
    case IndexTypes::PROXIMA_GRAPH_INDEX:
      return std::make_shared<VectorColumnReader>(
          collection_name, collection_path, segment_id, column_name);
    default:
      return ColumnReaderPtr();
  }
}


}  // end namespace index
}  // namespace be
}  // end namespace proxima
