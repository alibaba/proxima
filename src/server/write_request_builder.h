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
 *   \date     Mar 2021
 *   \brief    Write request builder interface definition
 */

#pragma once

#include "agent/column_order.h"
#include "agent/write_request.h"
#include "proto/proxima_be.pb.h"

namespace proxima {
namespace be {
namespace server {

/*! WriteRequestBuilder
 */
class WriteRequestBuilder {
 public:
  //! build write request
  static int build(const meta::CollectionMeta &meta,
                   const agent::ColumnOrder &column_order,
                   const proto::WriteRequest &pb_request,
                   agent::WriteRequest *write_request);

 private:
  static void get_index_and_forward_mode(const proto::WriteRequest &request,
                                         const meta::CollectionMeta &meta,
                                         bool *index_full_match,
                                         bool *forward_full_match);

  static int validate_request(const proto::WriteRequest &request,
                              const meta::CollectionMeta &meta,
                              const agent::ColumnOrder &column_order,
                              bool index_full_match, bool forward_full_match);


  static int build_proxy_request(const meta::CollectionMeta &meta,
                                 const agent::ColumnOrder &column_order,
                                 const proto::WriteRequest &pb_request,
                                 bool index_full_match, bool forward_full_match,
                                 agent::WriteRequest *write_request);

  static int build_direct_request(const meta::CollectionMeta &meta,
                                  const agent::ColumnOrder &column_order,
                                  const proto::WriteRequest &pb_request,
                                  bool index_full_match,
                                  bool forward_full_match,
                                  agent::WriteRequest *write_request);

  static int build_record(const proto::WriteRequest::Row &row,
                          const proto::WriteRequest::RowMeta &row_meta,
                          const meta::CollectionMeta &meta,
                          const agent::ColumnOrder &column_order,
                          bool index_full_match, bool forward_full_match,
                          index::CollectionDataset *dataset);

  static int build_forwards_data(const proto::WriteRequest::Row &row,
                                 const proto::WriteRequest::RowMeta &row_meta,
                                 const agent::ColumnOrder &column_order,
                                 const meta::CollectionMeta &meta,
                                 bool forward_full_match,
                                 index::CollectionDataset::RowData *row_data);

  static int build_indexes_data(const proto::WriteRequest::Row &row,
                                const proto::WriteRequest::RowMeta &row_meta,
                                const meta::CollectionMeta &meta,
                                bool index_full_match,
                                index::CollectionDataset::RowData *row_data);
};


}  // end namespace server
}  // namespace be
}  // end namespace proxima
