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
 *   \date     Nov 2020
 *   \brief    Implementation of mysql collection
 */

#include "mysql_collection.h"
#include <sstream>
#include <vector>
#include "repository/repository_common/config.h"
#include "repository/repository_common/error_code.h"
#include "lsn_context_format.h"

namespace proxima {
namespace be {
namespace repository {

//! Start process
void MysqlCollection::run() {
  fetch_thread_ = std::thread(&MysqlCollection::fetch_impl, this);
  send_thread_ = std::thread(&MysqlCollection::send_impl, this);
  LOG_INFO("Collection is running. name[%s]",
           config_.collection_name().c_str());
}

//! Update Collection
// todo<cdz>: Current version do not support update collection.
void MysqlCollection::update() {
  collection_state_flag_.store(CollectionStateFlag::UPDATE);
}

//! Drop Collection
void MysqlCollection::drop() {
  collection_state_flag_.store(CollectionStateFlag::DROP);
}

//! Reset Collection back to CONTINUE
int MysqlCollection::reset_collection() {
  // Get lsn information
  int ret = load_lsn_info();
  if (ret != 0) {
    LOG_ERROR("Failed to update lsn information.");
    return ret;
  }

  // LOG_INFO("Init mysql handler successed");
  if (lsn_ == 0) {
    ret = mysql_handler_->get_table_snapshot(&context_.file_name,
                                             &context_.position);
    if (ret != 0) {
      LOG_ERROR("Failed to get full table snapshot.");
      return ret;
    }
    LOG_INFO("Get table snapshot, file_name: %s position: %zu",
             context_.file_name.c_str(), (size_t)context_.position);
  }

  // Reset binlog pull state
  ret = mysql_handler_->reset_status(pull_state_flag_, config_, context_);
  if (ret != 0) {
    LOG_ERROR("Failed to reset mysql handler.");
    return ret;
  }

  // Reset collection state
  clear_fetch_data();
  reset_send_status();
  reset_fetch_status();
  return 0;
}

uint32_t MysqlCollection::schema_revision() const {
  // todo<cdz>: Read from collection info when add update support.
  // return config_.schema_revision();
  return 0;
}

bool MysqlCollection::finished() const {
  return state() == CollectionStatus::FINISHED;
}

CollectionStateFlag MysqlCollection::get_collection_flag() {
  CollectionStateFlag status = CollectionStateFlag::UPDATE;
  // In case update twice
  bool is_update = collection_state_flag_.compare_exchange_strong(
      status, CollectionStateFlag::NORMAL);
  if (is_update) {
    return CollectionStateFlag::UPDATE;
  } else {
    return status;
  }
}

void MysqlCollection::clear_fetch_data() {
  fetch_request_->Clear();
}

void MysqlCollection::reset_fetch_status() {
  prepared_data_size_.store(0);
  reset_.store(true);
}

void MysqlCollection::reset_send_status() {
  ready_.store(false);
}

void MysqlCollection::stop() {
  const std::string &collection_name = config_.collection_name();
  state_.store(CollectionStatus::FINISHED);
  if (fetch_thread_.joinable()) {
    fetch_thread_.join();
  }
  if (send_thread_.joinable()) {
    send_thread_.join();
  }
  LOG_INFO("Stop collection successed. name[%s]", collection_name.c_str());
}

void MysqlCollection::update_action() {
  const std::string &collection_name = config_.collection_name();
  LOG_INFO("Updating collection. name[%s]", collection_name.c_str());
  std::unique_lock<std::mutex> lg(update_mutex_);
  while (is_valid()) {
    int ret = reset_collection();
    if (ret != 0) {
      LOG_ERROR("Failed to reset collection. retry ...");
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      continue;
    }
    state_.store(CollectionStatus::RUNNING);
    break;
  }
}

void MysqlCollection::process_update() {
  switch (state_.load()) {
    case CollectionStatus::INIT:
    case CollectionStatus::RUNNING:
    case CollectionStatus::UPDATING:
      update_action();
      break;
    case CollectionStatus::FINISHED:
      break;
  }
}

void MysqlCollection::process_drop() {
  state_.store(CollectionStatus::FINISHED);
  LOG_INFO("Drop collection. name[%s]", config_.collection_name().c_str());
}

void MysqlCollection::wait_update_command() {
  while (is_valid()) {
    CollectionStateFlag flag = get_collection_flag();
    switch (flag) {
      case CollectionStateFlag::UPDATE:
        return;
      case CollectionStateFlag::DROP:
        process_drop();
        break;
      case CollectionStateFlag::NORMAL:
        std::this_thread::sleep_for(std::chrono::microseconds(10));
    }
  }
}

void MysqlCollection::process_normal() {
  auto current_state = state_.load();
  switch (current_state) {
    case CollectionStatus::INIT:
      state_.store(CollectionStatus::RUNNING);
      break;
    case CollectionStatus::RUNNING:
    case CollectionStatus::FINISHED:
      // If is running, or finished. do nothing
      break;
    case CollectionStatus::UPDATING:
      // If current state is updating. wait until get update command and update
      wait_update_command();
      process_update();
  }
  return;
}

void MysqlCollection::process_event(const CollectionStateFlag &flag) {
  switch (flag) {
    case CollectionStateFlag::UPDATE:
      process_update();
      break;
    case CollectionStateFlag::DROP:
      process_drop();
      break;
    case CollectionStateFlag::NORMAL:
      process_normal();
      break;
  }
}

CollectionStatus MysqlCollection::state() const {
  return state_.load();
}

int MysqlCollection::init_brpc() {
  brpc::ChannelOptions options;
  options.max_retry = max_retry_;
  options.timeout_ms = brpc_timeout_ms_;
  int ret =
      channel_.Init(index_server_uri_.c_str(), load_balance_.c_str(), &options);
  if (ret != 0) {
    LOG_ERROR("Failed to initialize channel. uri[%s]",
              index_server_uri_.c_str());
    return ErrorCode_InitChannel;
  }
  stub_.reset(new (std::nothrow) proto::ProximaService_Stub(&channel_));
  fetch_request_.reset(new (std::nothrow) proto::WriteRequest());
  send_request_.reset(new (std::nothrow) proto::WriteRequest());
  reset_.store(true);
  return 0;
}

int MysqlCollection::init_mysql_module() {
  if (mysql_handler_ == nullptr) {
    LOG_ERROR("Invalid mysql handler");
    return ErrorCode_InvalidMysqlHandler;
  }

  int ret = load_lsn_info(false);
  if (ret != 0) {
    LOG_ERROR("Failed to load lsn map information");
    return ret;
  }
  LOG_INFO("Load lsn info successed");

  ret = mysql_handler_->init(pull_state_flag_);
  if (ret != 0) {
    LOG_ERROR("Failed to init mysql handler");
    return ret;
  }
  LOG_INFO("Init mysql handler successed");

  if (lsn_ == 0) {
    ret = mysql_handler_->get_table_snapshot(&context_.file_name,
                                             &context_.position);
    if (ret != 0) {
      LOG_ERROR("Failed to get full table snapshot.");
      return ret;
    }
    LOG_INFO("Get table snapshot successed, file_name: %s position: %zu",
             context_.file_name.c_str(), (size_t)context_.position);
  }

  ret = mysql_handler_->start(context_);
  if (ret != 0) {
    LOG_ERROR("Failed to start mysql handler.");
    return ret;
  }

  return 0;
}

//! Initialize MySQL Collection
int MysqlCollection::init() {
  load_config();
  //! Init brpc components
  int ret = init_brpc();
  if (ret != 0) {
    LOG_ERROR("Failed to init brpc components");
    return ret;
  }
  LOG_INFO("Init brpc successed");

  //! Init mysql module
  ret = init_mysql_module();
  if (ret != 0) {
    LOG_ERROR("Failed to init mysql module");
    return ret;
  }
  LOG_INFO("Init mysql module successed");
  LOG_INFO("Init mysql collection successed");
  return 0;
}

void MysqlCollection::load_config() {
  batch_size_ = repository::Config::Instance().get_batch_size();
  batch_interval_ = repository::Config::Instance().get_batch_interval();
  index_server_uri_ = repository::Config::Instance().get_index_agent_uri();
  load_balance_ = repository::Config::Instance().get_load_balance();
  max_retry_ = repository::Config::Instance().get_max_retry();
  brpc_timeout_ms_ = repository::Config::Instance().get_timeout_ms();
}

int MysqlCollection::load_lsn_info(bool is_retry) {
  proto::CollectionName request;
  proto::DescribeCollectionResponse response;
  brpc::Controller cntl;
  request.set_collection_name(config_.collection_name());
  while (true) {
    if (!is_valid()) {
      return ErrorCode_Terminate;
    }
    stub_->describe_collection(&cntl, &request, &response, NULL);
    if (cntl.Failed()) {
      LOG_ERROR("Failed to get collection from index agent. msg[%s]",
                cntl.ErrorText().c_str());
      if (is_retry) {
        uint64_t sleep_time = get_sleep_time();
        std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time));
        cntl.Reset();
        continue;
      } else {
        return ErrorCode_RPCFailed;
      }
    }
    break;
  }

  // Debug
  LOG_INFO("get_collection response : %s", response.ShortDebugString().c_str());

  // Get lsn
  auto &info = response.collection();
  config_ = info.config();
  agent_timestamp_ = info.magic_number();
  auto &lsn_context = info.latest_lsn_context();
  lsn_ = lsn_context.lsn();
  if (lsn_ == 0) {
    LOG_INFO("LSN is zero, use initial value.");
    pull_state_flag_ = ScanMode::FULL;
    return 0;
  }

  LsnContextFormat lsn_context_format;
  int ret = lsn_context_format.parse_from_string(lsn_context.context());
  // int ret = string_to_context(lsn_context.context(), &lsn_context_format);
  if (ret != 0) {
    LOG_ERROR("Parse lsn context from string failed.");
    return ret;
  }

  context_.file_name = lsn_context_format.file_name();
  context_.position = lsn_context_format.position();
  context_.seq_id = lsn_context_format.seq_id();
  pull_state_flag_ = lsn_context_format.mode();
  return 0;
}

bool MysqlCollection::is_valid() const {
  if (state_.load() == CollectionStatus::FINISHED ||
      collection_state_flag_.load() == CollectionStateFlag::DROP) {
    return false;
  }
  return true;
}

int MysqlCollection::update_request_meta() {
  if (reset_.load() == true) {
    start_time_ = ailego::Monotime::MilliSeconds();
    int ret = reset_request();
    if (ret != 0) {
      LOG_ERROR("Reset fetch request failed");
      return ret;
    }
    reset_.store(false);
  }
  return 0;
}

int MysqlCollection::get_request_row() {
  int ret = 0;
  LsnContext current_context;
  auto *next_row = fetch_request_->add_rows();
  ret = mysql_handler_->get_next_row_data(next_row, &current_context);
  if (ret != 0) {
    // Not fatal error, continue
    LOG_ERROR("Get next row data failed. code[%d], msg[%s]", ret,
              ErrorCode::What(ret));
    // Remove the last row
    fetch_request_->mutable_rows()->RemoveLast();
    std::this_thread::sleep_for(std::chrono::microseconds(10));
    return ret;
  }
  ret = verify_and_handle(current_context);
  if (ret != 0) {
    return ret;
  }
  update_lsn_map_info(next_row);
  return 0;
}

bool MysqlCollection::must_send(uint64_t start_time) {
  if (ready_.load() == true) {
    return true;
  }
  uint64_t current_time = ailego::Monotime::MicroSeconds();
  if ((current_time - start_time) >= (uint64_t)batch_interval_ &&
      prepared_data_size_.load() != 0) {
    ready_.store(true);
    return true;
  }
  return false;
}

//! Start process
void MysqlCollection::fetch_impl() {
  LOG_INFO("Start fetch thread");
  uint64_t start_time = ailego::Monotime::MicroSeconds();
  start_time_ = start_time;  // start_time_ just use to print info
  while (is_valid()) {
    std::this_thread::sleep_for(std::chrono::microseconds(2));
    std::unique_lock<std::mutex> ul(update_mutex_);
    if (must_send(start_time)) {
      start_time = ailego::Monotime::MicroSeconds();
      continue;
    }

    int ret = update_request_meta();
    if (ret != 0) {
      LOG_ERROR("Update request meta failed. code[%d], msg[%s]", ret,
                ErrorCode::What(ret));
      // If failed, continue
      continue;
    }
    ret = get_request_row();
    if (ret != 0) {
      // Get row data failed, just continue
      continue;
    }

    prepared_data_size_++;
    if (prepared_data_size_ >= batch_size_) {
      ready_.store(true);
    }
  }
}

void MysqlCollection::update_lsn_map_info(
    proto::WriteRequest::Row *row_data) const {
  auto lsn_context = row_data->mutable_lsn_context();
  lsn_context->set_lsn(lsn_);

  LsnContextFormat current_context(context_.file_name, context_.position,
                                   context_.seq_id, pull_state_flag_);
  lsn_context->set_context(current_context.convert_to_string());
}

int MysqlCollection::verify_and_handle(const LsnContext &context) {
  auto current_state = context.status;
  switch (current_state) {
    case RowDataStatus::NO_MORE_DATA:
      handle_no_data();
      return ErrorCode_NoMoreData;
    case RowDataStatus::SCHEMA_CHANGED:
      handle_schema_changed();
      return ErrorCode_SchemaChanged;
    default:
      update_context(context);
      lsn_++;
      return 0;
  }
}

void MysqlCollection::handle_no_data() {
  fetch_request_->mutable_rows()->RemoveLast();
  if (pull_state_flag_ == ScanMode::FULL) {
    LOG_INFO("Scan mode need change");
    pull_state_flag_ = ScanMode::INCREMENTAL;
    context_.seq_id = 0;  // seq_id set to invalid value
    mysql_handler_->reset_status(ScanMode::INCREMENTAL, config_, context_);
    ready_.store(true);
  }
}

void MysqlCollection::update_context(const LsnContext &context) {
  if (pull_state_flag_ == ScanMode::FULL) {
    context_.seq_id = context.seq_id;
  } else {
    context_.file_name = context.file_name;
    context_.position = context.position;
  }
  context_.status = context.status;
}

void MysqlCollection::handle_schema_changed() {
  LOG_INFO("Schema changed");
  fetch_request_->mutable_rows()->RemoveLast();
  ready_.store(true);
}

//! Start process
void MysqlCollection::send_impl() {
  LOG_INFO("Start send thread");
  // Actual work here
  while (!finished()) {
    update_state();
    bool is_ready = wait_prepared_data();
    if (is_ready == false) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
      continue;
    }

    int ret = send_data();
    if (ret != 0) {
      LOG_ERROR("Failed to send data. code[%d], msg[%s]", ret,
                ErrorCode::What(ret));
      continue;
    }
  }
}

void MysqlCollection::update_state() {
  CollectionStateFlag flag = get_collection_flag();
  process_event(flag);
}

uint64_t MysqlCollection::get_sleep_time() const {
  std::mt19937 gen((std::random_device())());
  return (std::uniform_int_distribution<uint64_t>(500, 1000))(gen);
}

void MysqlCollection::print_send_data_info() const {
  uint64_t current_time = ailego::Monotime::MilliSeconds();
  uint64_t cost = current_time - start_time_;
  auto row_size = send_request_->rows_size();
  auto lsn_min = send_request_->rows(0).lsn_context().lsn();
  auto lsn_max = send_request_->rows(row_size - 1).lsn_context().lsn();
  LOG_INFO("Send request. size[%d], cost[%zums], lsn_min[%zu], lsn_max[%zu]",
           row_size, (size_t)cost, (size_t)lsn_min, (size_t)lsn_max);
}

int MysqlCollection::send_data() {
  int ret = 0;
  proto::Status response;
  brpc::Controller cntl;
  while (is_valid()) {
    print_send_data_info();
    stub_->write(&cntl, send_request_.get(), &response, NULL);
    if (cntl.Failed()) {
      ret = ErrorCode_RPCFailed;
      LOG_ERROR("Failed RPC. msg[%s].", cntl.ErrorText().c_str());
      uint64_t sleep_time = get_sleep_time();
      std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time));
      cntl.Reset();
      continue;
    }

    ret = (int)response.code();
    if (ret == ErrorCode_Success.value()) {
      // If successed
      return 0;
    } else if (ret == ErrorCode_ExceedRateLimit.value()) {
      // If exceed ratelimited, retry
      LOG_INFO("Exceed rate limite. Retry ...");
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      cntl.Reset();
      continue;
    } else if (ret == ErrorCode_MismatchedSchema.value()) {
      // If schema revision mismatched, change state to updating, and wait
      // update command later
      LOG_INFO("Schema revision mismatch");
      state_.store(CollectionStatus::UPDATING);
      return 0;
    } else if (ret == ErrorCode_MismatchedMagicNumber.value()) {
      // If agent timestamp mismatch, update now
      LOG_INFO("Agent timestamp mismatch");
      state_.store(CollectionStatus::UPDATING);
      process_update();
      return 0;
    } else if (ret == ErrorCode_CollectionNotExist.value()) {
      // If collection not exit
      LOG_INFO("Collection not exist");
      return ret;
    } else {
      // Other unknown response, just retry
      LOG_ERROR("Send data failed, unknown response. response_code[%d]", ret);
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      cntl.Reset();
      continue;
    }
  }
  return ret;
}

std::string MysqlCollection::generate_request_id() const {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<uint64_t> distrib(0, UINT64_MAX);
  return std::to_string(distrib(gen));
}

int MysqlCollection::reset_request() {
  clear_fetch_data();  // make sure fetch request is clean
  fetch_request_->set_request_id(generate_request_id());
  // todo<cdz>: Set schema revision when support update
  // fetch_request_->set_schema_revision(config_.schema_revision());
  fetch_request_->set_magic_number(agent_timestamp_);
  fetch_request_->set_collection_name(config_.collection_name());
  auto meta = fetch_request_->mutable_row_meta();
  int ret = mysql_handler_->get_fields_meta(meta);
  if (ret != 0) {
    LOG_ERROR("Failed to get fields meta. code[%d] msg[%s]", ret,
              ErrorCode::What(ret));
    return ret;
  }
  return 0;
}

void MysqlCollection::get_sending_request() {
  send_request_.swap(fetch_request_);
  clear_fetch_data();
  reset_fetch_status();  // inform fetch thread reset request
  reset_send_status();   // mark send thread get ready data
}

bool MysqlCollection::ready() const {
  return ready_.load() == true;
}

bool MysqlCollection::wait_prepared_data() {
  if (!ready()) {
    return false;
  }
  get_sending_request();
  if (is_send_request_empty()) {
    return false;
  }
  return true;
}

bool MysqlCollection::is_send_request_empty() const {
  auto row_size = send_request_->rows_size();
  return (row_size == 0);
}

}  // namespace repository
}  // namespace be
}  // namespace proxima
