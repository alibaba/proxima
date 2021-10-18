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
 *   \date     Dec 2020
 *   \brief
 */

#include <gmock/gmock.h>

#define private public
#define protected public
#include "repository/repository_common/config.h"
#undef private
#undef protected

#include "repository/mysql_collection.h"
#include "mock_index_agent_server.h"
#include "mock_mysql_handler.h"
#include "port_helper.h"

const std::string collection_name = "mysql_collection_test.info";
static int PORT = 8010;
static int PID = 0;

////////////////////////////////////////////////////////////////////
class MysqlCollectionRandomTest2 : public ::testing::Test {
 protected:
  MysqlCollectionRandomTest2() {}
  ~MysqlCollectionRandomTest2() {}
  void SetUp() {
    PortHelper::GetPort(&PORT, &PID);
    std::cout << "Server port: " << PORT << std::endl;
    std::string index_uri = "127.0.0.1:" + std::to_string(PORT);
    proxima::be::repository::Config::Instance()
        .repository_config_.mutable_repository_config()
        ->set_index_agent_addr(index_uri);
    std::cout << "Set index addr: " << index_uri << std::endl;

    proxima::be::repository::Config::Instance()
        .repository_config_.mutable_repository_config()
        ->set_batch_interval(1000000);
    std::cout << "Set batch_interval to 1s" << std::endl;
  }
  void TearDown() {
    PortHelper::RemovePortFile(PID);
  }
};

TEST_F(MysqlCollectionRandomTest2, TestGeneral) {
  brpc::Server server_;
  MockRandomProximaServiceImpl svc_;
  brpc::ServerOptions options;
  ASSERT_EQ(0, server_.AddService(&svc_, brpc::SERVER_DOESNT_OWN_SERVICE));
  ASSERT_EQ(0, server_.Start(PORT, &options));
  {
    proto::CollectionConfig config;
    config.set_collection_name(collection_name);

    CollectionPtr collection{nullptr};
    MockMysqlHandlerPtr mysql_handler =
        std::make_shared<MockMysqlHandler>(config);
    EXPECT_CALL(*mysql_handler, init(_))
        .WillRepeatedly(Return(0))
        .RetiresOnSaturation();
    EXPECT_CALL(*mysql_handler, start(_))
        .WillRepeatedly(Return(0))
        .RetiresOnSaturation();

    EXPECT_CALL(*mysql_handler,
                get_next_row_data(Matcher<proto::WriteRequest::Row *>(_),
                                  Matcher<LsnContext *>(_)))
        .WillRepeatedly(Invoke(
            [](proto::WriteRequest::Row *row_data, LsnContext *context) -> int {
              row_data->set_primary_key(1);
              context->status = RowDataStatus::NORMAL;
              return 0;
            }))
        .RetiresOnSaturation();

    EXPECT_CALL(*mysql_handler,
                reset_status(Matcher<ScanMode>(_),
                             Matcher<const proto::CollectionConfig &>(_),
                             Matcher<const LsnContext &>(_)))
        .WillRepeatedly(Return(0))
        .RetiresOnSaturation();

    EXPECT_CALL(*mysql_handler, get_fields_meta(_))
        .WillRepeatedly(Return(0))
        .RetiresOnSaturation();

    EXPECT_CALL(*mysql_handler, get_table_snapshot(_, _))
        .WillRepeatedly(Return(0))
        .RetiresOnSaturation();

    collection.reset(new (std::nothrow) MysqlCollection(config, mysql_handler));

    int ret = collection->init();
    ASSERT_EQ(ret, 0);
    CollectionStatus current_state = collection->state();
    ASSERT_EQ(current_state, CollectionStatus::INIT);
    collection->run();
    sleep(3);

    // check value
    ASSERT_EQ(svc_.is_server_called(), true);
    LOG_INFO("[test]: Server received records count [%zu]",
             (size_t)svc_.get_records_count());
    current_state = collection->state();
    ASSERT_NE(current_state, CollectionStatus::INIT);
    collection->drop();
    sleep(3);
    current_state = collection->state();
    ASSERT_EQ(current_state, CollectionStatus::FINISHED);
    collection->stop();
    sleep(3);
    LOG_INFO("[test]: Server received records count [%zu]",
             (size_t)svc_.get_records_count());
  }

  ASSERT_EQ(0, server_.Stop(0));
  ASSERT_EQ(0, server_.Join());
}
