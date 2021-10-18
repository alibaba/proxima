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

#include "meta_impl.h"
#include <ailego/utility/string_helper.h>
#include "common/error_code.h"
#include "common/uuid_helper.h"

namespace proxima {
namespace be {
namespace meta {

ColumnImpl::ColumnImpl() {
  meta_ = std::make_shared<ColumnMeta>();
}

ColumnImpl::ColumnImpl(const ColumnMeta &column_meta) {
  meta_ = std::make_shared<ColumnMeta>(column_meta);

  // Serialize index params to json string
  IndexParams::SerializeToBuffer(column_meta.parameters(), &parameters_);
}

ColumnImpl::ColumnImpl(std::string new_uid, std::string new_uuid,
                       const ColumnMeta &column_meta)
    : collection_uid_(std::move(new_uid)),
      collection_uuid_(std::move(new_uuid)) {
  meta_ = std::make_shared<ColumnMeta>(column_meta);

  // Serialize index params to json string
  IndexParams::SerializeToBuffer(column_meta.parameters(), &parameters_);
}

void ColumnImpl::transform() {
  IndexParams::ParseFromBuffer(parameters_, meta_->mutable_parameters());
}

CollectionImpl::CollectionImpl() {
  // Allocate new memory for meta
  meta_ = std::make_shared<CollectionMeta>();
  meta_->set_revision(0);
  meta_->set_current(true);
  uuid_ = gen_uuid();
}

CollectionImpl::CollectionImpl(const char *collection_name) {
  // Allocate new memory for meta
  meta_ = std::make_shared<CollectionMeta>();
  meta_->set_name(collection_name);
  uuid_ = gen_uuid();
}

CollectionImpl::CollectionImpl(const CollectionMeta &collection) {
  meta_ = std::make_shared<CollectionMeta>(collection);
  uuid_ = gen_uuid();
  init_from_meta();
}

CollectionImpl::CollectionImpl(CollectionMetaPtr collection)
    : meta_(std::move(collection)) {
  uuid_ = gen_uuid();
  init_from_meta();
}

int CollectionImpl::append(ColumnImplPtr &column_ptr, bool force) {
  if (!force && (column_ptr->collection_uuid() != uuid() ||
                 column_ptr->collection_uid() != uid())) {
    return PROXIMA_BE_ERROR_CODE(InvalidArgument);
  }

  // Reset uuid and uid
  column_ptr->set_collection_uuid(uuid());
  column_ptr->set_collection_uid(uid());

  // Link to columns
  columns_.push_back(column_ptr);
  // Collect meta
  meta_->append(column_ptr->meta());
  return 0;
}

int CollectionImpl::set_repository(DatabaseRepositoryImplPtr &repo) {
  if (repo) {
    repository_ = repo;
    meta_->set_repository(repository_->meta());
  }
  return 0;
}

void CollectionImpl::transform() {
  if (!forward_columns_.empty()) {
    ailego::StringHelper::Split(forward_columns_, ",",
                                meta_->mutable_forward_columns());
  }
  for (auto &column : columns()) {
    column->set_collection_uid(uid());
    column->set_collection_uuid(uuid());
  }
}

void CollectionImpl::init_from_meta() {
  // merge forward columns
  for (auto &c : meta_->forward_columns()) {
    forward_columns_.append(c);
    forward_columns_.append(",");
  }
  if (!forward_columns_.empty()) {
    forward_columns_.pop_back();
  }

  // parse columns
  for (auto &c : meta_->index_columns()) {
    columns_.emplace_back(
        std::make_shared<ColumnImpl>(meta_->uid(), uuid_, *c));
  }

  if (meta_->repository()) {
    auto repo =
        RepositoryHelper::Child<DatabaseRepositoryMeta>(meta_->repository());
    repository_ =
        std::make_shared<DatabaseRepositoryImpl>(meta_->uid(), uuid_, *repo);
  }
}

DatabaseRepositoryImpl::DatabaseRepositoryImpl() {
  repository_ = std::make_shared<DatabaseRepositoryMeta>();
}

DatabaseRepositoryImpl::DatabaseRepositoryImpl(
    const DatabaseRepositoryMeta &repo)
    : collection_uid_(gen_uuid()), collection_uuid_(gen_uuid()) {
  repository_ = std::make_shared<DatabaseRepositoryMeta>(repo);
}

DatabaseRepositoryImpl::DatabaseRepositoryImpl(
    const std::string &uid, const std::string &uuid,
    const DatabaseRepositoryMeta &repo)
    : collection_uid_(uid), collection_uuid_(uuid) {
  repository_ = std::make_shared<DatabaseRepositoryMeta>(repo);
}

}  // namespace meta
}  // namespace be
}  // namespace proxima
