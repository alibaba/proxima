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

#include "meta_wrapper.h"
#include "common/error_code.h"
#include "common/logger.h"

namespace proxima {
namespace be {
namespace query {

namespace {
static bool FindIf(const std::string &name,
                   const meta::ColumnMetaPtrList &columns) {
  return std::any_of(columns.begin(), columns.end(),
                     [&name](const meta::ColumnMetaPtr &meta) {
                       return meta->name() == name;
                     });
}

static int CheckCollection(meta::CollectionMetaPtr meta) {
  if (!meta) {
    return PROXIMA_BE_ERROR_CODE(InexistentCollection);
  } else if (!meta->readable()) {
    return PROXIMA_BE_ERROR_CODE(UnreadableCollection);
  }
  return 0;
}

}  // namespace

MetaWrapper::MetaWrapper(meta::MetaServicePtr meta_service)
    : meta_service_(std::move(meta_service)) {}

int MetaWrapper::validate(const std::string &collection,
                          const ColumnNameList &columns) const {
  auto meta = meta_service_->get_current_collection(collection);
  int code = CheckCollection(meta);
  if (code == 0) {
    for (auto &column : columns) {
      if (!FindIf(column, meta->index_columns())) {
        return PROXIMA_BE_ERROR_CODE(InexistentColumn);
      }
    }
  }
  return code;
}

int MetaWrapper::validate_collection(const std::string &collection) const {
  auto meta = meta_service_->get_current_collection(collection);
  return CheckCollection(meta);
}

int MetaWrapper::validate_column(const std::string &collection,
                                 const std::string &column) const {
  auto meta = meta_service_->get_current_collection(collection);
  int code = CheckCollection(meta);
  if (code == 0) {
    if (!FindIf(column, meta->index_columns())) {
      code = PROXIMA_BE_ERROR_CODE(InexistentColumn);
    }
  }
  return code;
}

int MetaWrapper::list_columns(const std::string &collection, uint64_t revision,
                              ColumnNameList *columns) const {
  meta::CollectionMetaPtr meta =
      meta_service_->get_collection(collection, revision);

  if (!meta) {
    LOG_ERROR("Can't get the collection meta with specified revision[%zu]",
              (size_t)revision);
    return PROXIMA_BE_ERROR_CODE(InvalidRevision);
  }

  columns->insert(columns->end(), meta->forward_columns().begin(),
                  meta->forward_columns().end());
  return 0;
}

DataTypes MetaWrapper::get_data_type(const std::string &collection,
                                     const std::string &column_name) {
  auto meta = meta_service_->get_current_collection(collection);
  if (!meta) {
    LOG_ERROR("Can't get the collection meta. collection[%s]",
              collection.c_str());
    return DataTypes::UNDEFINED;
  }

  auto column = meta->column_by_name(column_name);
  if (column) {
    return column->data_type();
  } else {
    LOG_ERROR("Collection has not column. collection[%s] column[%s]",
              collection.c_str(), column_name.c_str());
  }

  return DataTypes::UNDEFINED;
}

}  // namespace query
}  // namespace be
}  // namespace proxima
