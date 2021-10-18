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
 *   \date     Apr 2021
 *   \brief
 */

#define private public
#include "common/transformer.h"
#undef private
#include <gtest/gtest.h>

using namespace proxima::be;

class TransformerTest : public testing::Test {
 protected:
  void SetUp() {}

  void TearDown() {}
};

TEST_F(TransformerTest, TestTransformJsonVector) {
  size_t dimension = 512;
  std::ostringstream oss;
  oss << "[";
  for (size_t i = 0; i < 512; ++i) {
    if (i % 2 == 0)
      oss << (i + 1) / 512.0 << ",";
    else
      oss << (i + 1) / (-512.0) << ",";
  }
  oss << "]";
  std::string index_value = oss.str();
  std::string output_value;
  std::vector<float> vectors;
  auto ret = Transformer::Transform(index_value, nullptr, &vectors);
  ASSERT_EQ(ret, dimension);
  ASSERT_EQ(vectors.size(), dimension);
}

TEST_F(TransformerTest, TestExpectedSize) {
  std::ostringstream oss;
  oss << "[";
  for (size_t i = 0; i < 512; ++i) {
    if (i % 2 == 0)
      oss << (i + 1) / 512.0 << ",";
    else
      oss << (i + 1) / (-512.0) << ",";
  }
  oss << "]";
  std::string index_value = oss.str();
  std::string output_value;
  std::vector<float> vectors;
  auto ret = Transformer::Transform(index_value, nullptr, &vectors);
  ASSERT_EQ(ret, 512);
  ASSERT_EQ(vectors.size(), 512);
}

TEST_F(TransformerTest, TestInvalidVectorFormat) {
  std::ostringstream oss;
  oss << "{\"a\":1}";
  std::string index_value = oss.str();
  std::string output_value;
  std::vector<float> vectors;
  auto ret = Transformer::Transform(index_value, nullptr, &vectors);
  ASSERT_EQ(ret, PROXIMA_BE_ERROR_CODE(InvalidVectorFormat));
}

TEST_F(TransformerTest, TestParseJsonVectorFailedWithInvalidType) {
  std::ostringstream oss;
  oss << "[";
  for (size_t i = 0; i < 512; ++i) {
    if (i % 2 == 0)
      oss << (i + 1) / 512.0 << ",";
    else
      oss << (i + 1) / (-512.0) << ",";
  }
  std::string index_value = oss.str();
  std::string output_value;
  std::vector<float> vectors;
  auto ret = Transformer::Transform(index_value, nullptr, &vectors);

  ASSERT_EQ(ret, PROXIMA_BE_ERROR_CODE(InvalidVectorFormat));
}


TEST_F(TransformerTest, TestInt82Int4) {
  std::string index_value("[1,2,3,4,5,6]");
  std::string output_value;
  std::vector<int8_t> values;
  Transformer::Transform(index_value, nullptr, &values);
  Primary2Bytes::Bytes<int8_t, DataTypes::VECTOR_INT4>(values, &output_value);
  const uint8_t *data = (const uint8_t *)(&(output_value[0]));
  for (uint32_t i = 1; i <= 3; ++i) {
    ASSERT_FLOAT_EQ((int8_t)(2 * i - 1), (int8_t)(data[i - 1] & 0xf));
    ASSERT_FLOAT_EQ((uint8_t)(2 * i), (int8_t)(data[i - 1] >> 4));
  }
}

TEST_F(TransformerTest, TestFP32ToFP16) {
  std::string index_value("[1,2,3,4,5,6]");
  uint32_t dimension = 6;
  std::string output_value;
  std::vector<float> values;
  Transformer::Transform(index_value, nullptr, &values);
  Primary2Bytes::Bytes<float, DataTypes::VECTOR_FP16>(values, &output_value);
  const uint16_t *data = (const uint16_t *)(&(output_value[0]));
  for (uint32_t i = 1; i <= dimension; ++i) {
    ASSERT_FLOAT_EQ(1.0f * i, ailego::FloatHelper::ToFP32(data[i - 1]));
  }
}