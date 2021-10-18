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
 *   \date     Oct 2020
 *   \brief    IndexAgent service implementation for proxima search engine
 */

#include "index_agent.h"
#include "common/config.h"

namespace proxima {
namespace be {
namespace agent {

IndexAgentPtr IndexAgent::Create(meta::MetaServicePtr meta_service) {
  return std::make_shared<IndexAgent>(meta_service);
}

IndexAgent::IndexAgent(meta::MetaServicePtr meta_service)
    : meta_service_(std::move(meta_service)) {}

IndexAgent::~IndexAgent() {}

int IndexAgent::create_collection(const std::string &collection_name) {
  // Add counter
  counter_map_->add_counter(collection_name);

  // Add column order
  auto schema = meta_service_->get_current_collection(collection_name);
  if (!schema) {
    LOG_ERROR("Get latest collection for meta service failed. collection[%s]",
              collection_name.c_str());
    return ErrorCode_InexistentCollection;
  }
  column_order_map_->add_column_order(*schema);

  // Create collection
  int ret = index_service_->create_collection(collection_name, schema);
  if (ret != 0) {
    LOG_ERROR("Index service create collection failed. collection[%s]",
              collection_name.c_str());
    return ret;
  }

  return 0;
}

int IndexAgent::update_collection(const std::string &collection_name,
                                  uint32_t revision) {
  // get collection's counter
  CollectionCounterPtr counter = counter_map_->get_counter(collection_name);
  if (!counter) {
    LOG_ERROR("Get collection counter failed. collection[%s]",
              collection_name.c_str());
    return ErrorCode_RuntimeError;
  }

  // wait collection's all records to be processed
  while (true) {
    if (counter->active_count() == 0) {
      break;
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  // get specified revision's collection meta
  auto collection = meta_service_->get_collection(collection_name, revision);
  if (!collection) {
    LOG_ERROR("Meta service update collection failed. collection[%s]",
              collection_name.c_str());
    return ErrorCode_InexistentCollection;
  }

  column_order_map_->update_column_order(*collection);

  // update collection
  int ret = index_service_->update_collection(collection_name, collection);
  if (ret != 0) {
    LOG_ERROR("Index service update collection failed. collection[%s]",
              collection_name.c_str());
    return ret;
  }

  return 0;
}

int IndexAgent::drop_collection(const std::string &collection_name) {
  int ret = index_service_->drop_collection(collection_name);
  if (ret != 0) {
    LOG_ERROR("Index service drop collection failed. collection[%s]",
              collection_name.c_str());
    return ret;
  }

  counter_map_->remove_counter(collection_name);
  column_order_map_->remove_column_order(collection_name);

  return 0;
}

int IndexAgent::get_collection_stats(
    const std::string &name, index::CollectionStats *collection_stats) const {
  int ret = index_service_->get_collection_stats(name, collection_stats);
  if (ret != 0) {
    LOG_ERROR("Index service get collection stats failed. collection[%s]",
              name.c_str());
    return ret;
  }

  return 0;
}

bool IndexAgent::is_collection_suspend(const std::string &collection) {
  meta::CollectionMetaPtr meta =
      meta_service_->get_current_collection(collection);
  if (!meta) {
    LOG_ERROR("Meta service get latest collection failed. collection[%s]",
              collection.c_str());
    return false;
  }

  return !meta->writable();
}

int IndexAgent::write(const WriteRequest &request) {
  // check request empty
  int row_count = request.row_count();
  if (!row_count) {
    return 0;
  }

  // check if suspend
  const std::string &collection = request.collection_name();
  if (is_collection_suspend(collection)) {
    return ErrorCode_SuspendedCollection;
  }

  // check magic number
  bool proxy_request = request.is_proxy_request();
  if (proxy_request && agent_timestamp_ != request.magic_number()) {
    LOG_ERROR("Write request magic number mismatched.");
    return ErrorCode_MismatchedMagicNumber;
  }

  // acquire permits
  if (!rate_limiter_->try_acquire(row_count, acquire_timeout_)) {
    LOG_WARN("Acquire permits failed. count[%d] collection[%s]", row_count,
             collection.c_str());
    return ErrorCode_ExceedRateLimit;
  }

  CollectionCounterPtr counter = counter_map_->get_counter(collection);
  if (!counter) {
    LOG_ERROR("Get collection counter failed. collection[%s]",
              collection.c_str());
    return ErrorCode_InexistentCollection;
  }
  counter->add_active_count(row_count);

  // double check collection is suspended
  if (is_collection_suspend(collection)) {
    counter->sub_active_count(row_count);
    return ErrorCode_SuspendedCollection;
  }

  if (proxy_request) {
    return proxy_write(request, counter.get());
  } else {
    return direct_write(request, counter.get());
  }
}

int IndexAgent::get_latest_lsn(const std::string &collection_name,
                               uint64_t *lsn, std::string *lsn_context) {
  int ret = index_service_->get_latest_lsn(collection_name, lsn, lsn_context);
  if (ret != 0) {
    LOG_ERROR("Index service get collection latest lsn failed. collection[%s]",
              collection_name.c_str());
    return ret;
  }

  return 0;
}

int IndexAgent::init() {
  if (!meta_service_) {
    LOG_ERROR("Meta service is nullptr.");
    return ErrorCode_RuntimeError;
  }

  // init index service
  index_service_ = std::make_shared<index::IndexService>();
  int ret = index_service_->init();
  if (ret != 0) {
    LOG_ERROR("Init index service failed.");
    return ret;
  }

  // init rate limiter
  const Config &config = Config::Instance();
  rate_limiter_ = ailego::RateLimiter::Create(config.get_index_max_build_qps());
  if (!rate_limiter_) {
    LOG_ERROR("Create rate limiter failed.");
    return ErrorCode_RuntimeError;
  }
  // Set to default value
  acquire_timeout_ = 500;

  // init agent stat timestamp
  agent_timestamp_ = ailego::Monotime::MicroSeconds();

  // create counter map
  counter_map_ = std::make_shared<CollectionCounterMap>();

  // create columns order map
  column_order_map_ = std::make_shared<ColumnOrderMap>();

  LOG_INFO("IndexAgent initialzie complete.");
  return 0;
}

int IndexAgent::cleanup() {
  if (index_service_) {
    index_service_->cleanup();
  }

  LOG_INFO("IndexAgent cleanup complete.");
  return 0;
}

int IndexAgent::start() {
  // Start index service
  int ret = index_service_->start();
  if (ret != 0) {
    LOG_ERROR("Start index service failed.");
    return ret;
  }

  ret = load_index_service();
  if (ret != 0) {
    LOG_ERROR("Load index service failed.");
    return ret;
  }

  thread_pool_ = std::make_shared<ailego::ThreadQueue>(
      Config::Instance().get_index_build_thread_count());

  LOG_INFO("IndexAgent start complete.");
  return 0;
}

int IndexAgent::stop() {
  if (thread_pool_) {
    thread_pool_->stop();
  }

  if (index_service_) {
    index_service_->stop();
  }

  LOG_INFO("IndexAgent stopped.");
  return 0;
}

int IndexAgent::proxy_write(const WriteRequest &request,
                            CollectionCounter *counter) {
  auto &collection = request.collection_name();
  int dataset_count = (int)request.collection_dataset_count();
  for (int i = 0; i < dataset_count; ++i) {
    auto &dataset = request.get_collection_dataset(i);
    thread_pool_->execute(dataset->get(0).primary_key, this,
                          &IndexAgent::write_dataset, collection, dataset,
                          counter);
  }

  return 0;
}

int IndexAgent::direct_write(const WriteRequest &request,
                             CollectionCounter *counter) {
  auto &collection = request.collection_name();
  int row_count = request.row_count();
  auto &dataset = request.get_collection_dataset(0);
  int ret = index_service_->write_records(collection, dataset);
  if (ret != 0) {
    counter->sub_active_count(row_count);
    LOG_ERROR("Index service write records failed. collection[%s]",
              collection.c_str());
    return ret;
  }
  counter->sub_active_count(row_count);

  return 0;
}

void IndexAgent::write_dataset(const std::string &collection_name,
                               const index::CollectionDatasetPtr &record,
                               CollectionCounter *counter) {
  int ret = index_service_->write_records(collection_name, record);
  if (ret != 0) {
    LOG_ERROR(
        "Index service write record failed. "
        "code[%d] reason[%s] collection[%s]",
        ret, ErrorCode::What(ret), collection_name.c_str());
  }
  counter->dec_active_count();
}

int IndexAgent::load_index_service() {
  // 1.Get all valid collection schemas
  meta::CollectionMetaPtrList schemas;
  int ret = meta_service_->get_latest_collections(&schemas);
  if (ret != 0) {
    LOG_ERROR("Meta service get latest collections failed.");
    return ret;
  }

  // 2.Get all valid collection names, and create all collections columns order
  std::vector<std::string> collection_names;
  for (auto &schema : schemas) {
    collection_names.emplace_back(schema->name());
    column_order_map_->add_column_order(*schema);
  }

  // 3.Create all collections counter
  for (auto &collection_name : collection_names) {
    counter_map_->add_counter(collection_name);
  }

  // 4.Load index service
  ret = index_service_->load_collections(collection_names, schemas);
  if (ret != 0) {
    LOG_ERROR("Index service load collections failed.");
    return ret;
  }

  return 0;
}

}  // end namespace agent
}  // namespace be
}  // end namespace proxima
