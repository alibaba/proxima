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

#include "common/types_helper.h"
#include <gtest/gtest.h>

using namespace proxima::be;

TEST(DataTypeCodeBookTest, TestGet) {
  ASSERT_EQ(DataTypeCodeBook::Get(proto::DataType::DT_UNDEFINED),
            DataTypes::UNDEFINED);

  ASSERT_EQ(DataTypeCodeBook::Get(proto::DataType::DT_VECTOR_BINARY32),
            DataTypes::VECTOR_BINARY32);

  ASSERT_EQ(DataTypeCodeBook::Get(proto::DataType::DT_VECTOR_BINARY64),
            DataTypes::VECTOR_BINARY64);

  ASSERT_EQ(DataTypeCodeBook::Get(proto::DataType::DT_VECTOR_FP16),
            DataTypes::VECTOR_FP16);

  ASSERT_EQ(DataTypeCodeBook::Get(proto::DataType::DT_VECTOR_FP32),
            DataTypes::VECTOR_FP32);

  ASSERT_EQ(DataTypeCodeBook::Get(proto::DataType::DT_VECTOR_FP64),
            DataTypes::VECTOR_FP64);

  ASSERT_EQ(DataTypeCodeBook::Get(proto::DataType::DT_VECTOR_INT8),
            DataTypes::VECTOR_INT8);

  ASSERT_EQ(DataTypeCodeBook::Get(proto::DataType::DT_VECTOR_INT16),
            DataTypes::VECTOR_INT16);

  ASSERT_EQ(DataTypeCodeBook::Get(proto::DataType::DT_VECTOR_INT4),
            DataTypes::VECTOR_INT4);
}


TEST(ProtoConverterTest, TestConvertOperationType) {
  ASSERT_EQ(OperationTypesCodeBook::Get(proto::OP_INSERT),
            OperationTypes::INSERT);
  ASSERT_EQ(OperationTypesCodeBook::Get(proto::OP_UPDATE),
            OperationTypes::UPDATE);
  ASSERT_EQ(OperationTypesCodeBook::Get(proto::OP_DELETE),
            OperationTypes::DELETE);
}
