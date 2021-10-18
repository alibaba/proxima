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

#include "fake_collection.h"
#include "mock_collection_creator.h"
#include "port_helper.h"
#define private public
#define protected public
#include "repository/collection_manager.h"
#include "repository/repository_common/config.h"
#undef private
#undef protected

#include "mock_index_agent_server.h"

using namespace proxima::be::repository;

static int PORT = 8010;
static int PID = 0;

int main(int argc, char *argv[]) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

class CollectionManagerCreateTest : public ::testing::Test {
 protected:
  CollectionManagerCreateTest() {}

  ~CollectionManagerCreateTest() {}
  void SetUp() {
    PortHelper::GetPort(&PORT, &PID);
    std::cout << "Server port: " << PORT << std::endl;
    std::string index_uri = "127.0.0.1:" + std::to_string(PORT);
    proxima::be::repository::Config::Instance()
        .repository_config_.mutable_repository_config()
        ->set_index_agent_addr(index_uri);
    std::cout << "Set index addr: " << index_uri << std::endl;
  }
  void TearDown() {
    PortHelper::RemovePortFile(PID);
  }
};


TEST_F(CollectionManagerCreateTest, TestCreate) {
  brpc::Server server_;
  MockProximaServiceImpl svc_;
  brpc::ServerOptions options;
  ASSERT_EQ(0, server_.AddService(&svc_, brpc::SERVER_DOESNT_OWN_SERVICE));
  ASSERT_EQ(0, server_.Start(PORT, &options));
  {
    MockCollectionCreatorPtr collection_creator =
        std::make_shared<MockCollectionCreator>();
    EXPECT_CALL(*collection_creator, create(_))
        .WillRepeatedly(Invoke([](const CollectionInfo &info) -> CollectionPtr {
          std::string collection_name = info.config().collection_name();
          MysqlHandlerPtr mysql_handler =
              std::make_shared<MysqlHandler>(info.config());
          return std::make_shared<FakeMysqlCollection>(info.config(),
                                                       mysql_handler);
        }));

    CollectionManagerPtr collection_manager =
        std::make_shared<CollectionManager>(collection_creator);
    int ret = collection_manager->init();
    ASSERT_EQ(0, ret);

    CollectionInfo info1;
    CollectionInfo info2;
    CollectionInfo info3;
    info1.mutable_config()->set_collection_name("collection1");
    info1.set_uuid("collection1-uuid");
    info2.mutable_config()->set_collection_name("collection2");
    info2.set_uuid("collection2-uuid");
    info3.mutable_config()->set_collection_name("collection3");
    info3.set_uuid("collection3-uuid");
    std::vector<CollectionInfo> collection_infos{info1, info2, info3};
    std::vector<CollectionInfo> new_collections;
    std::vector<std::string> old_collections;
    std::vector<std::string> expired_collections;
    collection_manager->classify_collections(collection_infos, &new_collections,
                                             &old_collections,
                                             &expired_collections);
    EXPECT_EQ(3, new_collections.size());
    EXPECT_EQ(0, old_collections.size());
    EXPECT_EQ(0, expired_collections.size());
    EXPECT_EQ(std::any_of(new_collections.begin(), new_collections.end(),
                          [](const CollectionInfo &info) {
                            return info.config().collection_name() ==
                                       "collection1" &&
                                   info.uuid() == "collection1-uuid";
                          }),
              true);
    EXPECT_EQ(std::any_of(new_collections.begin(), new_collections.end(),
                          [](const CollectionInfo &info) {
                            return info.config().collection_name() ==
                                       "collection2" &&
                                   info.uuid() == "collection2-uuid";
                          }),
              true);
    EXPECT_EQ(std::any_of(new_collections.begin(), new_collections.end(),
                          [](const CollectionInfo &info) {
                            return info.config().collection_name() ==
                                       "collection3" &&
                                   info.uuid() == "collection3-uuid";
                          }),
              true);

    collection_manager->create_collections(new_collections);
    size_t count = 0;
    auto created_collection = svc_.get_created_collections();
    auto collections_name = svc_.get_collections_name();
    while (count < 5) {  // in order to sleep enough time
      if (created_collection.size() == collections_name.size()) {
        break;
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      created_collection = svc_.get_created_collections();
      collections_name = svc_.get_collections_name();
      count++;
    }
    EXPECT_EQ(created_collection.size(),
              collection_manager->collections_.size());
    EXPECT_EQ(3, collection_manager->collections_.size());
    EXPECT_EQ(1, collection_manager->collections_.count("collection1-uuid"));
    EXPECT_EQ(1, collection_manager->collections_.count("collection2-uuid"));
    EXPECT_EQ(1, collection_manager->collections_.count("collection3-uuid"));
    ret = collection_manager->stop();
    EXPECT_EQ(ret, 0);
  }
  ASSERT_EQ(0, server_.Stop(0));
  ASSERT_EQ(0, server_.Join());
}
