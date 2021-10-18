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
 *   \author   Dianzhang.Chen
 *   \date     Nov 2020
 *   \brief    Interface of mysql collection
 */

#pragma once

#include <mutex>
#include <brpc/channel.h>
#include "binlog/mysql_handler.h"
#include "common/macro_define.h"
#include "collection.h"

namespace proxima {
namespace be {
namespace repository {

class MysqlCollection;
using MysqlCollectionPtr = std::shared_ptr<MysqlCollection>;

/*! Mysql Collection
 */
class MysqlCollection : public Collection {
 public:
  //! Constructor
  MysqlCollection(const proto::CollectionConfig &config,
                  MysqlHandlerPtr mysql_handler)
      : config_(config), mysql_handler_(std::move(mysql_handler)) {}

  //! Destructor
  ~MysqlCollection() = default;

 public:
  //! Initialize MySQL collection
  int init() override;

  //! Stop collection
  void stop() override;

  //! Start Collection
  void run() override;

  //! Update collection
  void update() override;

  //! Drop collection
  void drop() override;

  //! If collection is finished
  bool finished() const override;

  //! Get collection state
  CollectionStatus state() const override;

  //! Get schema revision
  uint32_t schema_revision() const override;

 private:
  //! Implementation of fetch data
  void fetch_impl();

  //! Implementation of send data
  void send_impl();

  //! Check if collection should terminate
  bool is_valid() const;

  //! Clear data queue
  void clear_fetch_data();

  //! Send data to index agent
  int send_data();

  // Load lsn information
  int load_lsn_info(bool is_retry = true);

  //! Reset collection
  int reset_collection();

  //! Get collection state
  CollectionStateFlag get_collection_flag();

  //! Process event acording current collection state flag
  void process_event(const CollectionStateFlag &flag);

  //! Update collection
  void process_update();

  //! Drop collection
  void process_drop();

  //! Action when state flag is normal
  void process_normal();

  //! Load configs
  void load_config();

  //! Check if data is illegal and handle illegal data
  int verify_and_handle(const LsnContext &context);

  //! Get sending request
  void get_sending_request();

  //! Wait until prepared data is ready to send
  bool wait_prepared_data();

  //! Init brpc
  int init_brpc();

  //! Init mysql module
  int init_mysql_module();

  //！ Handle no data case
  void handle_no_data();

  //！ Handle schema changed case
  void handle_schema_changed();

  //！ Update context
  void update_context(const LsnContext &context);

  //! Reset brpc request
  int reset_request();

  //! Update request meta information
  int update_request_meta();

  //! Check and update state
  void update_state();

  //! Check if is ready to send data
  bool ready() const;

  //! Get sleep time
  uint64_t get_sleep_time() const;

  //! Update action
  void update_action();

  //! Wait update command from manager
  void wait_update_command();

  //! Generate request id
  std::string generate_request_id() const;

  //! Update lsn map information of row data
  void update_lsn_map_info(proto::WriteRequest::Row *row_data) const;

  //! Check whether send request is empty
  bool is_send_request_empty() const;

  //! Reset fetch status
  void reset_fetch_status();

  //! Reset send status
  void reset_send_status();

  //! Check if need to wait until send thread finish send data
  bool must_send(uint64_t start_time);

  //! Get rod data from binlog handler
  int get_request_row();

  //! Print information of send data
  void print_send_data_info() const;

 private:
  //! Disable copy constructor
  MysqlCollection(const MysqlCollection &) = delete;

  //! Disable assignment operator
  MysqlCollection &operator=(const MysqlCollection &) = delete;

 private:
  uint32_t batch_size_{0};
  uint32_t batch_interval_{0};

  proto::CollectionConfig config_{};
  uint64_t agent_timestamp_{0};

  //! Members for state
  std::thread fetch_thread_{};
  std::thread send_thread_{};
  std::atomic<CollectionStatus> state_{CollectionStatus::INIT};

  std::atomic<uint32_t> prepared_data_size_{0};
  std::atomic<bool> ready_{false};
  std::atomic<bool> reset_{false};

  std::mutex update_mutex_{};  // use for update collection
  std::unique_ptr<proto::WriteRequest> fetch_request_{};
  std::unique_ptr<proto::WriteRequest> send_request_{};

  ScanMode pull_state_flag_{ScanMode::FULL};
  std::atomic<CollectionStateFlag> collection_state_flag_{
      CollectionStateFlag::NORMAL};

  //! Members for brpc
  int max_retry_{0};
  int brpc_timeout_ms_{0};

  std::string index_server_uri_{};
  std::string load_balance_{};
  brpc::Channel channel_{};
  std::shared_ptr<proto::ProximaService_Stub> stub_{};

  //! Members for lsn
  uint64_t lsn_{0};
  LsnContext context_{};
  uint64_t start_time_{0};
  MysqlHandlerPtr mysql_handler_{};
};

}  // end namespace repository
}  // namespace be
}  // end namespace proxima