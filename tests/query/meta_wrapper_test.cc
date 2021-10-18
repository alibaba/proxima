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

#include "query/meta_wrapper.h"
#include <gtest/gtest.h>
#include "meta/meta_impl.h"
#include "meta/mock_meta_service.h"

using namespace proxima::be::query;
using namespace proxima::be::meta;

TEST(MetaWrapperTest, TestValidate) {
  {  // TestMetaService error
    auto meta_service = std::make_shared<MockMetaService>();
    EXPECT_CALL(*meta_service, get_current_collection(_))
        .WillOnce(Return(nullptr))  // Failed
        .RetiresOnSaturation();
    MetaWrapper meta(meta_service);

    std::string collection;
    ColumnNameList columns{"", ""};
    EXPECT_TRUE(meta.validate(collection, columns) != 0);
  }

  {  // TestMetaService success with invalid collection pointer
    auto meta_service = std::make_shared<MockMetaService>();
    EXPECT_CALL(*meta_service, get_current_collection(_))
        .WillOnce(Return(nullptr))  // Failed
        .RetiresOnSaturation();
    MetaWrapper meta(meta_service);

    std::string collection;
    ColumnNameList columns{"", ""};
    EXPECT_TRUE(meta.validate(collection, columns) != 0);
  }

  {  // TestMetaService success with valid collection pointer
    CollectionMeta meta;
    meta.mutable_forward_columns()->push_back("forward1");
    meta.mutable_forward_columns()->push_back("forward2");
    auto column1 = std::make_shared<ColumnMeta>("column1");
    auto column2 = std::make_shared<ColumnMeta>("column2");
    meta.append(column1);
    meta.append(column2);
    CollectionImplPtr collection = std::make_shared<CollectionImpl>(meta);

    auto meta_service = std::make_shared<MockMetaService>();
    EXPECT_CALL(*meta_service, get_current_collection(_))
        .WillRepeatedly(
            Invoke([&collection](const std::string &) -> CollectionMetaPtr {
              return collection->meta();
            }));  // success

    MetaWrapper meta_wrapper(meta_service);
    std::string name{"name"};
    ColumnNameList columns{"column1", "column2"};
    // True
    EXPECT_EQ(meta_wrapper.validate(name, columns), 0);
    // False
    columns.push_back("column3");
    EXPECT_TRUE(meta_wrapper.validate(name, columns) != 0);

    // Test validate_column
    EXPECT_EQ(meta_wrapper.validate_column(name, "column1"), 0);
    EXPECT_TRUE(meta_wrapper.validate_column(name, "column3") != 0);

    // Get collection failed
    EXPECT_CALL(*meta_service, get_collection(_, _))
        .WillOnce(Return(nullptr))
        .WillOnce(Return(collection->meta()));

    columns.clear();
    EXPECT_TRUE(meta_wrapper.list_columns("", 1, &columns) != 0);

    EXPECT_EQ(meta_wrapper.list_columns(name, 1, &columns), 0);
    EXPECT_EQ(columns.size(), 2);
    EXPECT_EQ(columns.front().compare("forward1"), 0);
    EXPECT_EQ(columns.back().compare("forward2"), 0);
  }

  {  // TestMetaService validate collection
    CollectionMeta meta;
    meta.set_name("name");
    auto column1 = std::make_shared<ColumnMeta>();
    auto column2 = std::make_shared<ColumnMeta>();
    meta.append(column1);
    meta.append(column2);
    CollectionImplPtr collection = std::make_shared<CollectionImpl>(meta);

    auto meta_service = std::make_shared<MockMetaService>();
    EXPECT_CALL(*meta_service, get_current_collection("name"))
        .WillOnce(
            Invoke([&collection](const std::string &) -> CollectionMetaPtr {
              return collection->meta();
            }))
        .RetiresOnSaturation();

    MetaWrapper meta_wrapper(meta_service);
    std::string name{"name"};
    // True
    EXPECT_EQ(meta_wrapper.validate_collection(name), 0);

    EXPECT_CALL(*meta_service, get_current_collection(_))
        .WillOnce(Return(nullptr))
        .RetiresOnSaturation();
    EXPECT_TRUE(meta_wrapper.validate_collection(name) != 0);
  }
}
