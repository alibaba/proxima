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
 *   \brief    Implemention of index service
 */

#include "index_service.h"
#include "common/error_code.h"

namespace proxima {
namespace be {
namespace index {

IndexService::~IndexService() {
  if (status_ == STARTED) {
    this->stop_impl();
  }
  this->cleanup_impl();
}

int IndexService::create_collection(const std::string &collection_name,
                                    const meta::CollectionMetaPtr &schema) {
  CHECK_STATUS(status_, STARTED);

  if (this->has_collection(collection_name)) {
    LOG_ERROR("Collection already exists, create failed. collection[%s]",
              collection_name.c_str());
    return ErrorCode_DuplicateCollection;
  }

  ReadOptions read_options;
  read_options.use_mmap = use_mmap_read_;

  /// Notice we add a check action here:
  /// a. the collection index file exists, then we just load it.
  /// b. the collection index not exist, then we just create new one.
  std::string collection_path = index_directory_ + "/" + collection_name;
  std::string manifest_file_path =
      FileHelper::MakeFilePath(collection_path, FileID::MANIFEST_FILE);
  if (FileHelper::FileExists(manifest_file_path)) {
    read_options.create_new = false;
  } else {
    read_options.create_new = true;
  }

  CollectionPtr collection;
  int ret = Collection::CreateAndOpen(collection_name, index_directory_, schema,
                                      concurrency_, thread_pool_.get(),
                                      read_options, &collection);
  CHECK_RETURN_WITH_LOG(ret, 0,
                        "Create and open new collection failed. collection[%s]",
                        collection_name.c_str());

  collections_.emplace(collection_name, collection);
  LOG_INFO("Create new collection success. collection[%s]",
           collection_name.c_str());

  return 0;
}

int IndexService::update_collection(const std::string &collection_name,
                                    const meta::CollectionMetaPtr &new_schema) {
  CHECK_STATUS(status_, STARTED);

  if (!this->has_collection(collection_name)) {
    LOG_ERROR("Collection not exist, update failed. collection[%s]",
              collection_name.c_str());
    return ErrorCode_InexistentCollection;
  }

  return collections_.get(collection_name)->update_schema(new_schema);
}

bool IndexService::has_collection(const std::string &collection_name) {
  CHECK_STATUS(status_, STARTED);

  return collections_.has(collection_name);
}

int IndexService::load_collections(
    const std::vector<std::string> &collection_names,
    const std::vector<meta::CollectionMetaPtr> &schemas) {
  CHECK_STATUS(status_, STARTED);

  ReadOptions read_options;
  read_options.use_mmap = use_mmap_read_;
  read_options.create_new = false;

  int ret = 0;
  for (size_t i = 0; i < collection_names.size() && i < schemas.size(); i++) {
    CollectionPtr collection;
    ret = Collection::CreateAndOpen(
        collection_names[i], index_directory_, schemas[i], concurrency_,
        thread_pool_.get(), read_options, &collection);
    CHECK_RETURN_WITH_LOG(ret, 0, "Load collection failed. collection[%s]",
                          collection_names[i].c_str());

    collections_.emplace(collection_names[i], collection);
    LOG_INFO("Load collection success. collectoin[%s]",
             collection_names[i].c_str());
  }

  return 0;
}

int IndexService::drop_collection(const std::string &collection_name) {
  CHECK_STATUS(status_, STARTED);

  if (!this->has_collection(collection_name)) {
    LOG_ERROR("Collection not exist, drop failed. collection[%s]",
              collection_name.c_str());
    return ErrorCode_InexistentCollection;
  }

  collections_.get(collection_name)->close_and_cleanup();
  collections_.erase(collection_name);

  LOG_INFO("Drop collection success. collection[%s]", collection_name.c_str());
  return 0;
}

int IndexService::list_collections(std::vector<std::string> *collection_names) {
  CHECK_STATUS(status_, STARTED);

  for (auto &it : collections_) {
    collection_names->emplace_back(it.first);
  }
  return 0;
}

int IndexService::get_collection_stats(const std::string &collection_name,
                                       CollectionStats *collection_stats) {
  CHECK_STATUS(status_, STARTED);

  if (!this->has_collection(collection_name)) {
    LOG_ERROR("Collection not exist, get statistics failed. collection[%s]",
              collection_name.c_str());
    return ErrorCode_InexistentCollection;
  }

  return collections_.get(collection_name)->get_stats(collection_stats);
}

int IndexService::list_segments(const std::string &collection_name,
                                std::vector<SegmentPtr> *segments) {
  CHECK_STATUS(status_, STARTED);

  if (!this->has_collection(collection_name)) {
    LOG_ERROR("Collection not exist, list segments failed. collection[%s]",
              collection_name.c_str());
    return ErrorCode_InexistentCollection;
  }

  return collections_.get(collection_name)->get_segments(segments);
}

int IndexService::get_latest_lsn(const std::string &collection_name,
                                 uint64_t *lsn, std::string *lsn_context) {
  CHECK_STATUS(status_, STARTED);

  if (!this->has_collection(collection_name)) {
    LOG_ERROR("Collection not exist, get latest lsn failed. collection[%s]",
              collection_name.c_str());
    return ErrorCode_InexistentCollection;
  }

  return collections_.get(collection_name)->get_latest_lsn(lsn, lsn_context);
}

int IndexService::write_records(const std::string &collection_name,
                                const CollectionDatasetPtr &records) {
  CHECK_STATUS(status_, STARTED);

  if (!this->has_collection(collection_name)) {
    LOG_ERROR("Collection not exist, write records failed. collection[%s]",
              collection_name.c_str());
    return ErrorCode_InexistentCollection;
  }

  return collections_.get(collection_name)->write_records(*records);
}

int IndexService::init_impl() {
  if (!load_config()) {
    LOG_ERROR("Load config failed.");
    return ErrorCode_LoadConfig;
  }

  thread_pool_ = std::make_shared<ThreadPool>(thread_count_, false);
  if (!thread_pool_) {
    LOG_ERROR("Create thread pool failed.");
    return ErrorCode_RuntimeError;
  }

  LOG_INFO("IndexService initialize complete.");
  return 0;
}

int IndexService::cleanup_impl() {
  thread_count_ = 0U;
  index_directory_ = "";
  flush_internal_ = 0U;
  concurrency_ = 0U;
  use_mmap_read_ = false;

  LOG_INFO("IndexService cleanup complete.");
  return 0;
}

int IndexService::start_impl() {
  if (flush_internal_ > 0U) {
    thread_pool_->submit(
        ailego::Closure::New(this, &IndexService::do_routine_flush));
  }

  if (optimize_internal_ > 0U) {
    thread_pool_->submit(
        ailego::Closure::New(this, &IndexService::do_routine_optimize));
  }

  LOG_INFO("IndexService start complete.");
  return 0;
}

int IndexService::stop_impl() {
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  flush_flag_ = false;
  flush_notifier_.notify();

  optimize_flag_ = false;
  optimize_notifier_.notify();

  thread_pool_->stop();

  for (auto &it : collections_) {
    it.second->close();
  }
  collections_.clear();

  LOG_INFO("IndexService stopped.");
  return 0;
}

bool IndexService::load_config() {
  auto &config = Config::Instance();
  thread_count_ = config.get_index_dump_thread_count();
  index_directory_ = config.get_index_directory();
  flush_internal_ = config.get_index_flush_internal();
  optimize_internal_ = config.get_index_optimize_internal();
  concurrency_ =
      config.get_index_build_thread_count() + config.get_query_thread_count();

  use_mmap_read_ = true;
  return true;
}

void IndexService::do_routine_flush() {
  flush_flag_ = true;

  while (true) {
    if (!flush_flag_) {
      LOG_INFO("Exited flush thread");
      break;
    }

    for (auto it : collections_) {
      it.second->flush();
    }

    flush_notifier_.wait_for(std::chrono::seconds(flush_internal_));
  }
}

void IndexService::do_routine_optimize() {
  optimize_flag_ = true;

  while (true) {
    if (!optimize_flag_) {
      LOG_INFO("Exited optimize thread");
      break;
    }

    for (auto it : collections_) {
      it.second->optimize(thread_pool_);
    }

    optimize_notifier_.wait_for(std::chrono::seconds(optimize_internal_));
  }
}


}  // end namespace index
}  // namespace be
}  // end namespace proxima
