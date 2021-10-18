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
 *   \date     Nov 2020
 *   \brief    Event fetcher interface definition for proxima search engine
 */

#pragma once

#include "binlog_event.h"
#include "mysql_connector.h"

namespace proxima {
namespace be {
namespace repository {

class EventFetcher;
using EventFetcherPtr = std::shared_ptr<EventFetcher>;

/*! Event Fetcher
 */
class EventFetcher : public MysqlConnectorProxy {
 public:
  //! Constructor
  EventFetcher(MysqlConnectorManagerPtr mgr);

  //! Destructor
  ~EventFetcher() = default;

  //! Init event fetcher
  int init(const std::string &file_name, uint64_t position);

  //! Fetch next event
  int fetch(BasicEventPtr *next_event);

 private:
  //! Set Check Sum
  int turnoff_checksum();

  //! Send dump request to master
  int request_dump(const std::string &file_name, uint64_t position);

  //! Update lsn info
  int update_lsn_info(const std::string &file_name, uint64_t position);

  //! Get latest lsn
  int get_latest_lsn(const std::string &file_name, MysqlResultWrapper &result);

  //! Generate Server id
  uint32_t generate_server_id();

  //! Read data
  int read_data(unsigned long *len);

  //! Update rotate info
  void update_rotate_info(BasicEventPtr event);

 private:
  //! Binlog event idx
  uint64_t event_idx_{0};
  //! Binlog file name
  std::string file_name_{};
  //! Binlog position
  uint64_t position_{0};
  //! Connector need reconnect
  bool need_reconnect_{false};
  //! Server id
  uint32_t server_id_{0};
  //! Uri
  ailego::Uri uri_{};

  static const size_t MAX_PASSWORD_LENGTH = 32;
  static const int BINLOG_DUMP_NON_BLOCK = (1 << 0);
};

}  // end namespace repository
}  // namespace be
}  // end namespace proxima
