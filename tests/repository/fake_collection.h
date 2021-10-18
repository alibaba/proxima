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

 *   \author   Dianzhang.Chen
 *   \date     Jan 2021
 *   \brief    Definition of fake collection
 */

#pragma once

#include <string>
#include <brpc/channel.h>

#define private public
#define protected public
#include "repository/repository_common/config.h"
#undef private
#undef protected

#include "repository/binlog/mysql_handler.h"
#include "repository/collection.h"
#include "repository/common_types.h"
#include "repository/repository_common/error_code.h"

namespace proxima {
namespace be {
namespace repository {

class FakeMysqlCollection;
using FakeMysqlCollectionPtr = std::shared_ptr<FakeMysqlCollection>;

/*! Mysql Collection
 */
class FakeMysqlCollection : public Collection {
 public:
  //! Constructor
  FakeMysqlCollection(const proto::CollectionConfig &config,
                      MysqlHandlerPtr mysql_handler)
      : config_(config), mysql_handler_(std::move(mysql_handler)) {}

  //! Destructor
  virtual ~FakeMysqlCollection() {
    if (work_thread_.joinable()) {
      work_thread_.join();
    }
  }

 public:
  //! Initialize MySQL Collection
  int init() override;

  //! Start Collection
  void run() override;

  //! Update Collection
  void update() override;

  //! Drop Collection
  void drop() override;

  //! Get collection state
  CollectionStatus state() const override;

  //! Stop collection
  void stop() override;

  //! Get collection schema revision
  uint32_t schema_revision() const override;

  //! Check if collection is finished
  bool finished() const override;


 protected:
  int init_brpc();

  void work_impl();

  bool is_valid();

 private:
  std::atomic<CollectionStatus> state_{CollectionStatus::INIT};
  proto::CollectionConfig config_;
  MysqlHandlerPtr mysql_handler_;
  brpc::Channel channel_;
  std::shared_ptr<proto::ProximaService_Stub> stub_;
  std::thread work_thread_;
};

void FakeMysqlCollection::run() {
  LOG_INFO("Start Fake Mysql Collection. name[%s]",
           config_.collection_name().c_str());
  work_thread_ = std::thread(&FakeMysqlCollection::work_impl, this);
}

void FakeMysqlCollection::work_impl() {
  while (is_valid()) {
    std::this_thread::sleep_for(std::chrono::microseconds(1));
    proto::Status response;
    proto::WriteRequest request;
    request.set_collection_name(config_.collection_name());
    brpc::Controller cntl;
    stub_->write(&cntl, &request, &response, NULL);
  }
}

bool FakeMysqlCollection::is_valid() {
  if (state_.load() == CollectionStatus::FINISHED) {
    return false;
  }
  return true;
}

bool FakeMysqlCollection::finished() const {
  return state() == CollectionStatus::FINISHED;
}


void FakeMysqlCollection::update() {
  state_.store(CollectionStatus::UPDATING);
  // Just fake: ++schema_revision when update collection
  // real logic should update from brpc
  // auto current_schema_revision = config_.schema_revision();
  // config_.set_schema_revision(++current_schema_revision);
}

void FakeMysqlCollection::drop() {
  state_.store(CollectionStatus::FINISHED);
}

int FakeMysqlCollection::init() {
  LOG_INFO("Init Fake Mysql Collection. name[%s]",
           config_.collection_name().c_str());
  int ret = init_brpc();
  if (ret != 0) {
    return ret;
  }
  proto::CollectionName request;
  proto::DescribeCollectionResponse response;
  brpc::Controller cntl;
  request.set_collection_name(config_.collection_name());
  stub_->describe_collection(&cntl, &request, &response, NULL);
  return 0;
}

int FakeMysqlCollection::init_brpc() {
  brpc::ChannelOptions options;
  auto index_uri =
      proxima::be::repository::Config::Instance().get_index_agent_uri();
  int ret = channel_.Init(index_uri.c_str(), "", &options);
  if (ret != 0) {
    LOG_ERROR("Failed to initialize channel");
    return ErrorCode_InitChannel;
  }
  stub_.reset(new (std::nothrow) proto::ProximaService_Stub(&channel_));
  return 0;
}

CollectionStatus FakeMysqlCollection::state() const {
  CollectionStatus state = state_.load();
  return state;
}

void FakeMysqlCollection::stop() {
  LOG_INFO("Stop Fake Mysql Collection. name[%s]",
           config_.collection_name().c_str());
  state_.store(CollectionStatus::FINISHED);
  if (work_thread_.joinable()) {
    work_thread_.join();
  }
}

uint32_t FakeMysqlCollection::schema_revision() const {
  return 0;
}

}  // end namespace repository
}  // namespace be
}  // end namespace proxima
