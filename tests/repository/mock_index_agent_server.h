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
 *   \brief    Definition of mock index agent server
 */

#include <random>
#include <unordered_set>
#include <brpc/channel.h>
#include <brpc/controller.h>
#include <brpc/server.h>
#include <gtest/gtest.h>
#include "proto/proxima_be.pb.h"
#include "repository/repository_common/error_code.h"
#include "repository/repository_common/logger.h"
#include "repository/repository_common/version.h"

using namespace proxima::be;
using namespace ::testing;

/////////////////////////////////////
class MockGeneralProximaServiceImpl
    : public ::proxima::be::proto::ProximaService {
 public:
  void create_collection(google::protobuf::RpcController *controller,
                         const ::proxima::be::proto::CollectionConfig *request,
                         ::proxima::be::proto::Status *response,
                         ::google::protobuf::Closure *done) {
    brpc::ClosureGuard done_guard(done);
    response->set_code(::proxima::be::repository::ErrorCode_Success.value());
  }

  void drop_collection(google::protobuf::RpcController *controller,
                       const ::proxima::be::proto::CollectionName *request,
                       ::proxima::be::proto::Status *response,
                       ::google::protobuf::Closure *done) {
    brpc::ClosureGuard done_guard(done);
    response->set_code(::proxima::be::repository::ErrorCode_Success.value());
  }

  void describe_collection(
      google::protobuf::RpcController *controller,
      const ::proxima::be::proto::CollectionName *request,
      ::proxima::be::proto::DescribeCollectionResponse *response,
      ::google::protobuf::Closure *done) {
    brpc::ClosureGuard done_guard(done);
    std::unique_lock<std::mutex> ul(server_lock_);
    auto *info = response->mutable_collection();
    auto *config = info->mutable_config();
    config->set_collection_name(request->collection_name());
    auto lsn_context = info->mutable_latest_lsn_context();
    if (!mock_context_store_.empty()) {
      auto last_lsn_context = mock_context_store_.back();
      lsn_context->set_lsn(last_lsn_context.lsn());
      lsn_context->set_context(last_lsn_context.context());
    } else {
      lsn_context->set_lsn(0);
      lsn_context->set_context("");
    }
    response->mutable_status()->set_code(
        ::proxima::be::repository::ErrorCode_Success.value());
  }

  void list_collections(google::protobuf::RpcController *controller,
                        const ::proxima::be::proto::ListCondition *request,
                        ::proxima::be::proto::ListCollectionsResponse *response,
                        ::google::protobuf::Closure *done) {
    brpc::ClosureGuard done_guard(done);
    response->mutable_status()->set_code(
        ::proxima::be::repository::ErrorCode_Success.value());
  }

  void stats_collection(google::protobuf::RpcController *controller,
                        const ::proxima::be::proto::CollectionName *request,
                        ::proxima::be::proto::StatsCollectionResponse *response,
                        ::google::protobuf::Closure *done) {
    brpc::ClosureGuard done_guard(done);
    response->mutable_status()->set_code(
        ::proxima::be::repository::ErrorCode_Success.value());
  }

  void write(google::protobuf::RpcController *controller,
             const ::proxima::be::proto::WriteRequest *request,
             ::proxima::be::proto::Status *response,
             ::google::protobuf::Closure *done) {
    brpc::ClosureGuard done_guard(done);
    std::unique_lock<std::mutex> ul(server_lock_);
    LOG_INFO("Mock General ProximaService received request[%s]",
             request->ShortDebugString().c_str());
    auto row_size = request->rows_size();
    for (int i = 0; i < row_size; i++) {
      auto current_row_context = request->rows(i).lsn_context();
      mock_context_store_.push_back(current_row_context);
    }
    std::string request_string = "";
    request->SerializeToString(&request_string);
    request_strings_.push_back(request_string);
    server_called_++;
    response->set_code(::proxima::be::repository::ErrorCode_Success.value());
  }

  void query(google::protobuf::RpcController *controller,
             const ::proxima::be::proto::QueryRequest *request,
             ::proxima::be::proto::QueryResponse *response,
             ::google::protobuf::Closure *done) {
    brpc::ClosureGuard done_guard(done);
    response->mutable_status()->set_code(
        ::proxima::be::repository::ErrorCode_Success.value());
  }

  void get_version(::google::protobuf::RpcController * /* controller */,
                   const proto::GetVersionRequest * /* request */,
                   proto::GetVersionResponse *response,
                   ::google::protobuf::Closure *done) {
    brpc::ClosureGuard done_guard(done);
    response->set_version(repository::Version::String());
    response->mutable_status()->set_code(0);
    response->mutable_status()->set_reason(repository::ErrorCode::What(0));
  }

  std::string get_request_string(int idx) {
    std::unique_lock<std::mutex> ul(server_lock_);
    if ((int)server_called_ <= idx) {
      return "";
    }
    std::string ret = request_strings_[idx];
    return ret;
  }

  size_t get_server_called_count() {
    std::unique_lock<std::mutex> ul(server_lock_);
    return server_called_;
  }

 private:
  std::vector<std::string> request_strings_;
  std::vector<proto::LsnContext> mock_context_store_;
  size_t server_called_{0};
  std::mutex server_lock_;
};
/////////////////////////////////////
// todo: don't use ::
class MockRandomProximaServiceImpl
    : public ::proxima::be::proto::ProximaService {
 public:
  void create_collection(google::protobuf::RpcController *controller,
                         const ::proxima::be::proto::CollectionConfig *request,
                         ::proxima::be::proto::Status *response,
                         ::google::protobuf::Closure *done) {
    brpc::ClosureGuard done_guard(done);
    response->set_code(::proxima::be::repository::ErrorCode_Success.value());
  }

  void drop_collection(google::protobuf::RpcController *controller,
                       const ::proxima::be::proto::CollectionName *request,
                       ::proxima::be::proto::Status *response,
                       ::google::protobuf::Closure *done) {
    brpc::ClosureGuard done_guard(done);
    response->set_code(::proxima::be::repository::ErrorCode_Success.value());
  }

  void describe_collection(
      google::protobuf::RpcController *controller,
      const ::proxima::be::proto::CollectionName *request,
      ::proxima::be::proto::DescribeCollectionResponse *response,
      ::google::protobuf::Closure *done) {
    brpc::ClosureGuard done_guard(done);
    std::unique_lock<std::mutex> ul(server_lock_);
    auto next = (expect_ >> 1);
    auto *info = response->mutable_collection();
    auto *config = info->mutable_config();
    config->set_collection_name(request->collection_name());
    auto lsn_context = info->mutable_latest_lsn_context();
    lsn_context->set_lsn(next);
    for (auto it = mock_context_store_.rbegin();
         it != mock_context_store_.rend(); it++) {
      if (it->lsn() == next) {
        lsn_context->set_context(it->context());
      }
    }
    expect_ = next + 1;
    LOG_INFO("expect_: [%zu]", (size_t)expect_);
    response->mutable_status()->set_code(
        ::proxima::be::repository::ErrorCode_Success.value());
  }

  void list_collections(google::protobuf::RpcController *controller,
                        const ::proxima::be::proto::ListCondition *request,
                        ::proxima::be::proto::ListCollectionsResponse *response,
                        ::google::protobuf::Closure *done) {
    brpc::ClosureGuard done_guard(done);
    response->mutable_status()->set_code(
        ::proxima::be::repository::ErrorCode_Success.value());
  }

  void stats_collection(google::protobuf::RpcController *controller,
                        const ::proxima::be::proto::CollectionName *request,
                        ::proxima::be::proto::StatsCollectionResponse *response,
                        ::google::protobuf::Closure *done) {
    brpc::ClosureGuard done_guard(done);
    response->mutable_status()->set_code(
        ::proxima::be::repository::ErrorCode_Success.value());
  }

  void write(google::protobuf::RpcController *controller,
             const ::proxima::be::proto::WriteRequest *request,
             ::proxima::be::proto::Status *response,
             ::google::protobuf::Closure *done) {
    brpc::ClosureGuard done_guard(done);
    std::unique_lock<std::mutex> ul(server_lock_);
    LOG_INFO("Mock General ProximaService received request[%s]",
             request->ShortDebugString().c_str());
    auto row = request->rows();
    uint64_t last_lsn = 0;
    for (auto it = row.begin(); it != row.end(); it++) {
      if (it == row.begin()) {
        first_lsn_ = it->lsn_context().lsn();
        EXPECT_EQ(it->lsn_context().lsn(), expect_);
      }
      auto current_row_context = it->lsn_context();
      mock_context_store_.push_back(current_row_context);
      last_lsn = current_row_context.lsn();
      records_count_++;
    }
    server_called_ = true;
    expect_ = last_lsn + 1;

    std::mt19937 gen((std::random_device())());
    auto temp = (std::uniform_int_distribution<size_t>(0, 10))(gen);
    int result = 0;
    if (temp < 7) {
      result = ::proxima::be::repository::ErrorCode_Success.value();
    } else if (temp < 9) {
      result = ::proxima::be::repository::ErrorCode_ExceedRateLimit.value();
      expect_ = first_lsn_;
    } else {
      // todo<cdz>: Add ErrorCode_MismatchedSchema when support update
      // result =
      // ::proxima::be::repository::ErrorCode_MismatchedSchema.value();
      result = ::proxima::be::repository::ErrorCode_Success.value();
    }
    LOG_INFO("expect_: [%zu]", (size_t)expect_);
    response->set_code(result);
  }

  void query(google::protobuf::RpcController *controller,
             const ::proxima::be::proto::QueryRequest *request,
             ::proxima::be::proto::QueryResponse *response,
             ::google::protobuf::Closure *done) {
    brpc::ClosureGuard done_guard(done);
    response->mutable_status()->set_code(
        ::proxima::be::repository::ErrorCode_Success.value());
  }

  bool is_server_called() {
    std::unique_lock<std::mutex> ul(server_lock_);
    return server_called_;
  }


  uint64_t get_records_count() {
    std::unique_lock<std::mutex> ul(server_lock_);
    return records_count_;
  }


  void get_version(::google::protobuf::RpcController * /* controller */,
                   const proto::GetVersionRequest * /* request */,
                   proto::GetVersionResponse *response,
                   ::google::protobuf::Closure *done) {
    brpc::ClosureGuard done_guard(done);
    response->set_version(repository::Version::String());
    response->mutable_status()->set_code(0);
    response->mutable_status()->set_reason(repository::ErrorCode::What(0));
  }


 private:
  uint64_t expect_{1};
  uint64_t first_lsn_{1};
  bool server_called_{false};
  uint64_t records_count_{0};
  std::vector<proto::LsnContext> mock_context_store_;
  std::mutex server_lock_;
};

// Use for manager test
class MockProximaServiceImpl : public ::proxima::be::proto::ProximaService {
 public:
  void create_collection(google::protobuf::RpcController *controller,
                         const ::proxima::be::proto::CollectionConfig *request,
                         ::proxima::be::proto::Status *response,
                         ::google::protobuf::Closure *done) {
    brpc::ClosureGuard done_guard(done);
    response->set_code(::proxima::be::repository::ErrorCode_Success.value());
  }

  void drop_collection(google::protobuf::RpcController *controller,
                       const ::proxima::be::proto::CollectionName *request,
                       ::proxima::be::proto::Status *response,
                       ::google::protobuf::Closure *done) {
    brpc::ClosureGuard done_guard(done);
    response->set_code(::proxima::be::repository::ErrorCode_Success.value());
  }

  void describe_collection(
      google::protobuf::RpcController *controller,
      const ::proxima::be::proto::CollectionName *request,
      ::proxima::be::proto::DescribeCollectionResponse *response,
      ::google::protobuf::Closure *done) {
    brpc::ClosureGuard done_guard(done);

    std::unique_lock<std::mutex> ul(server_lock_);
    created_collection_.insert(request->collection_name());
    auto *info = response->mutable_collection();
    auto *config = info->mutable_config();
    config->set_collection_name(request->collection_name());
    response->mutable_status()->set_code(
        ::proxima::be::repository::ErrorCode_Success.value());
  }

  void list_collections(google::protobuf::RpcController *controller,
                        const ::proxima::be::proto::ListCondition *request,
                        ::proxima::be::proto::ListCollectionsResponse *response,
                        ::google::protobuf::Closure *done) {
    brpc::ClosureGuard done_guard(done);

    for (size_t i = 0; i < collections_name_.size(); i++) {
      auto *current_collection_info = response->add_collections();
      //   current_config->set_collection_name(collections_name[i]);
      current_collection_info->set_uuid(collections_uuid_[i]);
      current_collection_info->set_status(
          proxima::be::proto::CollectionInfo::CS_SERVING);
      auto *current_config = current_collection_info->mutable_config();
      current_config->set_collection_name(collections_name_[i]);
      //   current_config->set_schema_revision(1);
    }
    response->mutable_status()->set_code(
        ::proxima::be::repository::ErrorCode_Success.value());
  }

  void stats_collection(google::protobuf::RpcController *controller,
                        const ::proxima::be::proto::CollectionName *request,
                        ::proxima::be::proto::StatsCollectionResponse *response,
                        ::google::protobuf::Closure *done) {
    brpc::ClosureGuard done_guard(done);
    response->mutable_status()->set_code(
        ::proxima::be::repository::ErrorCode_Success.value());
  }

  void write(google::protobuf::RpcController *controller,
             const ::proxima::be::proto::WriteRequest *request,
             ::proxima::be::proto::Status *response,
             ::google::protobuf::Closure *done) {
    brpc::ClosureGuard done_guard(done);
    response->set_code(::proxima::be::repository::ErrorCode_Success.value());
  }

  void query(google::protobuf::RpcController *controller,
             const ::proxima::be::proto::QueryRequest *request,
             ::proxima::be::proto::QueryResponse *response,
             ::google::protobuf::Closure *done) {
    brpc::ClosureGuard done_guard(done);
    response->mutable_status()->set_code(
        ::proxima::be::repository::ErrorCode_Success.value());
  }

  void get_version(::google::protobuf::RpcController * /* controller */,
                   const proto::GetVersionRequest * /* request */,
                   proto::GetVersionResponse *response,
                   ::google::protobuf::Closure *done) {
    brpc::ClosureGuard done_guard(done);
    response->set_version(repository::Version::String());
    response->mutable_status()->set_code(0);
    response->mutable_status()->set_reason(repository::ErrorCode::What(0));
  }

  std::unordered_set<std::string> get_created_collections() {
    std::unique_lock<std::mutex> ul(server_lock_);
    return created_collection_;
  }

  std::vector<std::string> get_collections_name() {
    return collections_name_;
  }

 private:
  std::vector<std::string> collections_name_{"collection1", "collection2",
                                             "collection3"};
  std::vector<std::string> collections_uuid_{
      "collection1-uuid", "collection2-uuid", "collection3-uuid"};
  std::unordered_set<std::string> created_collection_{};
  std::mutex server_lock_;
};
