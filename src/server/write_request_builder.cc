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
 *   \brief    Implements write request builder interface
 */

#include "write_request_builder.h"
#include "common/types_helper.h"
#include "proto_converter.h"

namespace proxima {
namespace be {
namespace server {

using RequestType = agent::WriteRequest::RequestType;

int WriteRequestBuilder::build(const meta::CollectionMeta &meta,
                               const agent::ColumnOrder &column_order,
                               const proto::WriteRequest &pb_request,
                               agent::WriteRequest *write_request) {
  // get if indexes and forwards sequence are strictly match collection meta
  bool index_full_match = false;
  bool forward_full_match = false;
  get_index_and_forward_mode(pb_request, meta, &index_full_match,
                             &forward_full_match);
  // validate indexes and forward values size
  int ret = validate_request(pb_request, meta, column_order, index_full_match,
                             forward_full_match);
  if (ret != 0) {
    return ret;
  }

  RequestType request_type =
      meta.repository() ? RequestType::PROXY : RequestType::DIRECT;
  if (request_type == RequestType::PROXY) {
    ret = build_proxy_request(meta, column_order, pb_request, index_full_match,
                              forward_full_match, write_request);
  } else {
    ret = build_direct_request(meta, column_order, pb_request, index_full_match,
                               forward_full_match, write_request);
  }
  if (ret != 0) {
    LOG_ERROR("Build write request failed. collection[%s]",
              pb_request.collection_name().c_str());
    return ret;
  }

  write_request->set_request_type(request_type);

  return 0;
}

void WriteRequestBuilder::get_index_and_forward_mode(
    const proto::WriteRequest &request, const meta::CollectionMeta &meta,
    bool *index_full_match, bool *forward_full_match) {
  // check index columns size
  auto &request_index_columns = request.row_meta().index_column_metas();
  auto &meta_index_columns = meta.index_columns();
  size_t index_column_size = static_cast<size_t>(request_index_columns.size());
  size_t meta_columns_size = meta_index_columns.size();

  // get index column full match
  *index_full_match = false;
  if (meta_columns_size == index_column_size) {
    size_t i = 0;
    for (i = 0; i < meta_columns_size; ++i) {
      if (meta_index_columns[i]->name() !=
          request_index_columns[i].column_name()) {
        break;
      }
    }
    if (i == meta_columns_size) {
      *index_full_match = true;
    }
  }

  // get forward column full match
  auto &request_forward_columns = request.row_meta().forward_column_names();
  auto &meta_forward_columns = meta.forward_columns();
  size_t request_forward_size =
      static_cast<size_t>(request_forward_columns.size());
  size_t meta_forward_size = meta_forward_columns.size();
  *forward_full_match = false;
  if (meta_forward_size == request_forward_size) {
    size_t i = 0;
    for (i = 0; i < meta_forward_size; ++i) {
      if (meta_forward_columns[i] != request_forward_columns[i]) {
        break;
      }
    }
    if (i == meta_forward_size) {
      *forward_full_match = true;
    }
  }
}

int WriteRequestBuilder::validate_request(
    const proto::WriteRequest &request, const meta::CollectionMeta &meta,
    const agent::ColumnOrder &column_order, bool index_full_match,
    bool forward_full_match) {
  auto &collection = request.collection_name();
  auto &request_index_metas = request.row_meta().index_column_metas();
  size_t index_column_size = static_cast<size_t>(request_index_metas.size());
  size_t meta_index_size = meta.index_columns().size();
  // check request is empty
  if (!request.rows_size()) {
    LOG_ERROR("Write request is empty. collection[%s]", collection.c_str());
    return ErrorCode_InvalidWriteRequest;
  }

  // check index
  if (index_column_size != (size_t)request_index_metas.size()) {
    LOG_ERROR(
        "Collection index column meta size mismatched. "
        "meta[%zu] index[%zu] collection[%s]",
        index_column_size, (size_t)request_index_metas.size(),
        collection.c_str());
    return ErrorCode_InvalidWriteRequest;
  }

  if (meta_index_size < index_column_size) {
    LOG_ERROR(
        "Collection index columns size mismatched. meta[%zu] "
        "request[%zu] collection[%s]",
        meta_index_size, index_column_size, collection.c_str());
    return ErrorCode_InvalidWriteRequest;
  }
  if (!index_full_match) {
    auto &index_order = column_order.get_index_order();
    for (size_t i = 0; i < index_column_size; ++i) {
      auto &index_column = request_index_metas[i].column_name();
      auto it = index_order.find(index_column);
      if (it == index_order.end()) {
        LOG_ERROR("Collection index field invalid. request[%s] collection[%s]",
                  index_column.c_str(), collection.c_str());
        return ErrorCode_InvalidWriteRequest;
      }
    }
  }
  for (size_t i = 0; i < index_column_size; ++i) {
    auto &index_column = request_index_metas[i].column_name();
    auto column_meta = meta.column_by_name(index_column);
    if (!column_meta) {
      LOG_ERROR("Invalid index column. name[%s] collection[%s]",
                index_column.c_str(), collection.c_str());
      return ErrorCode_InvalidWriteRequest;
    }
    if (column_meta->dimension() != request_index_metas[i].dimension()) {
      LOG_ERROR(
          "Index column dimension mismatched. "
          "meta[%u] request[%u] column[%s] collection[%s]",
          column_meta->dimension(), request_index_metas[i].dimension(),
          index_column.c_str(), collection.c_str());
      return ErrorCode_InvalidWriteRequest;
    }
  }

  // check forward
  auto &request_forward_columns = request.row_meta().forward_column_names();
  size_t request_forward_size =
      static_cast<size_t>(request_forward_columns.size());
  size_t meta_forward_size = meta.forward_columns().size();
  if (meta_forward_size < request_forward_size) {
    LOG_ERROR(
        "Collection forward columns size mismatched. meta[%zu] "
        "request[%zu] collection[%s]",
        meta_forward_size, request_forward_size, collection.c_str());
    return ErrorCode_InvalidWriteRequest;
  }
  if (!forward_full_match) {
    auto &forward_order = column_order.get_forward_order();
    for (auto &forward_column : request_forward_columns) {
      auto it = forward_order.find(forward_column);
      if (it == forward_order.end()) {
        LOG_ERROR(
            "Collection forward field invalid. request[%s] collection[%s]",
            forward_column.c_str(), collection.c_str());
        return ErrorCode_InvalidWriteRequest;
      }
    }
  }

  // check index&forward data
  for (int i = 0; i < request.rows_size(); ++i) {
    auto &row = request.rows(i);
    if (row.operation_type() == proxima::be::proto::OP_DELETE) {
      continue;
    }
    if (!index_column_size) {
      LOG_ERROR("Row index column names is empty. collection[%s]",
                collection.c_str());
      return ErrorCode_InvalidWriteRequest;
    }
    size_t index_value_size =
        static_cast<size_t>(row.index_column_values().values_size());
    if (index_value_size != index_column_size) {
      LOG_ERROR(
          "Row index columns size mismatched. meta[%zu] "
          "values[%zu] collection[%s]",
          index_column_size, index_value_size, collection.c_str());
      return ErrorCode_InvalidWriteRequest;
    }

    size_t forward_value_size =
        static_cast<size_t>(row.forward_column_values().values_size());
    if (forward_value_size != request_forward_size) {
      LOG_ERROR(
          "Row forward columns size mismatched. meta[%zu] "
          "values[%zu] collection[%s]",
          request_forward_size, forward_value_size, collection.c_str());
      return ErrorCode_InvalidWriteRequest;
    }
  }

  return 0;
}

int WriteRequestBuilder::build_proxy_request(
    const meta::CollectionMeta &meta, const agent::ColumnOrder &column_order,
    const proto::WriteRequest &pb_request, bool index_full_match,
    bool forward_full_match, agent::WriteRequest *write_request) {
  auto &row_meta = pb_request.row_meta();
  auto &collection = pb_request.collection_name();

  for (int i = 0; i < pb_request.rows_size(); ++i) {
    // schema revision default 0
    index::CollectionDatasetPtr record =
        std::make_shared<index::CollectionDataset>(0);
    auto &row = pb_request.rows(i);
    int ret = build_record(row, row_meta, meta, column_order, index_full_match,
                           forward_full_match, record.get());
    if (ret != 0) {
      LOG_ERROR("Build record failed. id[%d] collection[%s]", i,
                collection.c_str());
      return ret;
    }
    write_request->add_collection_dataset(record);
  }

  write_request->set_magic_number(pb_request.magic_number());
  write_request->set_collection_name(collection);

  return 0;
}

int WriteRequestBuilder::build_direct_request(
    const meta::CollectionMeta &meta, const agent::ColumnOrder &column_order,
    const proto::WriteRequest &pb_request, bool index_full_match,
    bool forward_full_match, agent::WriteRequest *write_request) {
  auto &row_meta = pb_request.row_meta();
  auto &collection = pb_request.collection_name();

  // schema revision default 0
  index::CollectionDatasetPtr dataset =
      std::make_shared<index::CollectionDataset>(0);

  for (int i = 0; i < pb_request.rows_size(); ++i) {
    auto &row = pb_request.rows(i);
    int ret = build_record(row, row_meta, meta, column_order, index_full_match,
                           forward_full_match, dataset.get());
    if (ret != 0) {
      LOG_ERROR("Build record failed. id[%d] collection[%s]", i,
                collection.c_str());
      return ret;
    }
  }

  write_request->add_collection_dataset(dataset);
  write_request->set_collection_name(collection);

  return 0;
}

int WriteRequestBuilder::build_record(
    const proto::WriteRequest::Row &row,
    const proto::WriteRequest::RowMeta &row_meta,
    const meta::CollectionMeta &meta, const agent::ColumnOrder &column_order,
    bool index_full_match, bool forward_full_match,
    index::CollectionDataset *dataset) {
  auto *row_data = dataset->add_row_data();
  row_data->primary_key = row.primary_key();

  // set lsn context
  if (meta.repository()) {
    if (!row.has_lsn_context()) {
      LOG_ERROR("Row not set lsn_context field. pk[%zu] collection[%s]",
                (size_t)row.primary_key(), meta.name().c_str());
      return ErrorCode_EmptyLsnContext;
    }
    row_data->lsn_check = true;
    auto &lsn_ctx = row.lsn_context();
    row_data->lsn = lsn_ctx.lsn();
    row_data->lsn_context = lsn_ctx.context();
  } else {
    row_data->lsn_check = false;
  }

  row_data->operation_type = OperationTypesCodeBook::Get(row.operation_type());

  if (row_data->operation_type == OperationTypes::DELETE) {
    return 0;
  }

  // build forwards data
  int ret = build_forwards_data(row, row_meta, column_order, meta,
                                forward_full_match, row_data);
  if (ret != 0) {
    LOG_ERROR("Build forwards data failed. collection[%s]",
              meta.name().c_str());
    return ret;
  }

  // build index data
  ret = build_indexes_data(row, row_meta, meta, index_full_match, row_data);
  if (ret != 0) {
    LOG_ERROR("Build indexes data failed. collection[%s]", meta.name().c_str());
    return ret;
  }

  return 0;
}

int WriteRequestBuilder::build_forwards_data(
    const proto::WriteRequest::Row &row,
    const proto::WriteRequest::RowMeta &row_meta,
    const agent::ColumnOrder &column_order, const meta::CollectionMeta &meta,
    bool forward_full_match, index::CollectionDataset::RowData *row_data) {
  // if forward_full_match is true, direct serialized
  auto *forward_data = &(row_data->forward_data);
  if (forward_full_match) {
    if (!row.forward_column_values().SerializeToString(forward_data)) {
      LOG_ERROR("Forward columns serialize failed. collection[%s]",
                meta.name().c_str());
      return ErrorCode_SerializeError;
    }
    return 0;
  }

  // init the value list
  proto::GenericValueList value_list;
  auto &forward_order = column_order.get_forward_order();
  size_t meta_forward_size = meta.forward_columns().size();
  for (size_t i = 0; i < meta_forward_size; ++i) {
    value_list.add_values();
  }

  // fill the value list
  auto &request_forward_columns = row_meta.forward_column_names();
  auto &forward_values = row.forward_column_values().values();
  for (int i = 0; i < request_forward_columns.size(); ++i) {
    auto &forward_column = request_forward_columns[i];
    auto it = forward_order.find(forward_column);
    if (it != forward_order.end()) {
      if (it->second < meta_forward_size) {
        value_list.mutable_values(it->second)->CopyFrom(forward_values[i]);
      } else {
        LOG_ERROR(
            "Forward order invalid. forward[%s] index[%zu] "
            "max_size[%zu] collection[%s]",
            forward_column.c_str(), it->second, meta_forward_size,
            meta.name().c_str());
        return ErrorCode_RuntimeError;
      }
    } else {
      LOG_ERROR("Find forward order failed. forward[%s] collection[%s]",
                forward_column.c_str(), meta.name().c_str());
      return ErrorCode_InvalidWriteRequest;
    }
  }

  // copy forward data
  if (!value_list.SerializeToString(forward_data)) {
    LOG_ERROR("Forward columns serialize failed. collection[%s]",
              meta.name().c_str());
    return ErrorCode_SerializeError;
  }

  return 0;
}

int WriteRequestBuilder::build_indexes_data(
    const proto::WriteRequest::Row &row,
    const proto::WriteRequest::RowMeta &row_meta,
    const meta::CollectionMeta &meta, bool index_full_match,
    index::CollectionDataset::RowData *row_data) {
  auto &index_column_metas = row_meta.index_column_metas();
  int index_column_size = index_column_metas.size();
  row_data->column_datas.resize(index_column_size);

  auto &index_values = row.index_column_values().values();
  auto &column_meta_list = meta.index_columns();
  for (int i = 0; i < index_column_size; ++i) {
    meta::ColumnMetaPtr column_meta;
    if (index_full_match) {
      column_meta = column_meta_list[i];
    } else {
      column_meta = meta.column_by_name(index_column_metas[i].column_name());
    }
    if (!column_meta) {
      LOG_ERROR("Find index column failed. column[%s] collection[%s]",
                index_column_metas[i].column_name().c_str(),
                meta.name().c_str());
      return ErrorCode_MismatchedIndexColumn;
    }
    auto value_type = index_values[i].value_oneof_case();
    if ((column_meta->index_type() != IndexTypes::PROXIMA_GRAPH_INDEX) ||
        (value_type != proto::GenericValue::ValueOneofCase::kStringValue &&
         value_type != proto::GenericValue::ValueOneofCase::kBytesValue)) {
      LOG_ERROR(
          "Only support PROXIMA_GRAPH_INDEX && (string or bytes) type."
          " collection[%s]",
          meta.name().c_str());
      return ErrorCode_MismatchedIndexColumn;
    }

    int ret = 0;
    auto &column_data = row_data->column_datas[i];
    if (value_type == proto::GenericValue::ValueOneofCase::kStringValue) {
      ret = ProtoConverter::ConvertIndexData(
          index_values[i].string_value(), *column_meta, index_column_metas[i],
          false, &column_data);
    } else {
      ret = ProtoConverter::ConvertIndexData(
          index_values[i].bytes_value(), *column_meta, index_column_metas[i],
          true, &column_data);
    }
    if (ret != 0) {
      LOG_ERROR("Convert collection index data failed. collection[%s]",
                meta.name().c_str());
      return ret;
    }
  }

  return 0;
}


}  // end namespace server
}  // namespace be
}  // end namespace proxima
