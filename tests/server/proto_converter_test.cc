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
 */

#include <ailego/encoding/json.h>
#include <ailego/utility/float_helper.h>
#include <ailego/utility/time_helper.h>

#define private public
#include "server/proto_converter.h"
#undef private
#include <gtest/gtest.h>

using namespace proxima::be;
using namespace proxima::be::server;

class ProtoConverterTest : public testing::Test {
 protected:
  void SetUp() {}

  void TearDown() {}
};

TEST_F(ProtoConverterTest, TestConvertIndexDataSuccessWithJson) {
  std::string index_value("[1,2,3,4,5,6]");
  meta::ColumnMetaPtr meta = std::make_shared<meta::ColumnMeta>();
  meta->set_index_type(IndexTypes::PROXIMA_GRAPH_INDEX);
  meta->set_data_type(DataTypes::VECTOR_FP32);
  meta->set_dimension(6);
  meta->set_name("field1");
  int dimension = 6;
  aitheta2::IndexParams *params = meta->mutable_parameters();
  params->set("dimension", dimension);
  proto::WriteRequest::IndexColumnMeta proto_meta;
  proto_meta.set_dimension(dimension);
  proto_meta.set_data_type(proto::DataType::DT_VECTOR_FP32);
  index::ColumnData column_data;
  int ret = ProtoConverter::ConvertIndexData(index_value, *meta, proto_meta,
                                             false, &column_data);
  ASSERT_EQ(ret, 0);
  ASSERT_EQ(column_data.column_name, "field1");
  ASSERT_EQ(column_data.data_type, DataTypes::VECTOR_FP32);
  ASSERT_EQ(column_data.dimension, 6);
  const float *data = (const float *)(&(column_data.data[0]));
  for (uint32_t i = 1; i <= column_data.dimension; ++i) {
    ASSERT_FLOAT_EQ(1.0f * i, data[i - 1]);
  }
}

TEST_F(ProtoConverterTest, TestConvertIndexDataSuccessWithBytes) {
  std::vector<float> vectors = {1, 2, 3, 4, 5, 6};
  std::string index_value((const char *)vectors.data(),
                          vectors.size() * sizeof(float));
  meta::ColumnMetaPtr meta = std::make_shared<meta::ColumnMeta>();
  meta->set_index_type(IndexTypes::PROXIMA_GRAPH_INDEX);
  meta->set_data_type(DataTypes::VECTOR_FP32);
  int dimension = 6;
  meta->set_dimension(dimension);
  meta->set_name("field1");
  proto::WriteRequest::IndexColumnMeta proto_meta;
  proto_meta.set_dimension(dimension);
  proto_meta.set_data_type(proto::DataType::DT_VECTOR_FP32);
  index::ColumnData column_data;
  int ret = ProtoConverter::ConvertIndexData(index_value, *meta, proto_meta,
                                             true, &column_data);
  ASSERT_EQ(ret, 0);
  ASSERT_EQ(column_data.column_name, "field1");
  ASSERT_EQ(column_data.data_type, DataTypes::VECTOR_FP32);
  ASSERT_EQ(column_data.dimension, 6);
  const float *data = (const float *)(&(column_data.data[0]));
  for (uint32_t i = 1; i <= column_data.dimension; ++i) {
    ASSERT_FLOAT_EQ(1.0f * i, data[i - 1]);
  }
}

TEST_F(ProtoConverterTest, TestConvertIndexDataWithParseFailed) {
  std::string index_value("[1,2,3,4,5]");
  meta::ColumnMetaPtr meta = std::make_shared<meta::ColumnMeta>();
  meta->set_index_type(IndexTypes::PROXIMA_GRAPH_INDEX);
  meta->set_data_type(DataTypes::VECTOR_FP32);
  int dimension = 6;
  meta->set_dimension(6);
  proto::WriteRequest::IndexColumnMeta proto_meta;
  proto_meta.set_dimension(dimension);
  proto_meta.set_data_type(proto::DataType::DT_VECTOR_FP32);
  index::ColumnData column_data;
  int ret = ProtoConverter::ConvertIndexData(index_value, *meta, proto_meta,
                                             false, &column_data);
  ASSERT_EQ(ret, ErrorCode_MismatchedDimension);
}

TEST_F(ProtoConverterTest, TestConvertIndexDataWithUnsupportedIndexType) {
  std::string index_value("[1,2,3,4,5,6]");
  meta::ColumnMetaPtr meta = std::make_shared<meta::ColumnMeta>();
  meta->set_index_type(IndexTypes::UNDEFINED);
  meta->set_data_type(DataTypes::VECTOR_FP32);
  int dimension = 6;
  meta->set_dimension(dimension);
  proto::WriteRequest::IndexColumnMeta proto_meta;
  proto_meta.set_dimension(dimension);
  proto_meta.set_data_type(proto::DataType::DT_VECTOR_FP32);
  index::ColumnData column_data;
  int ret = ProtoConverter::ConvertIndexData(index_value, *meta, proto_meta,
                                             false, &column_data);
  ASSERT_EQ(ret, ErrorCode_InvalidIndexType);
}

TEST_F(ProtoConverterTest, TestParseJsonIndexColumnValueaFp32) {
  std::string index_value("[1,2,3,4,5,6]");
  meta::ColumnMetaPtr meta = std::make_shared<meta::ColumnMeta>();
  meta->set_index_type(IndexTypes::PROXIMA_GRAPH_INDEX);
  meta->set_data_type(DataTypes::VECTOR_FP32);
  uint32_t dimension = 6;
  meta->set_dimension(dimension);
  proto::WriteRequest::IndexColumnMeta proto_meta;
  proto_meta.set_dimension(dimension);
  proto_meta.set_data_type(proto::DataType::DT_VECTOR_FP32);
  std::string output_value;
  int ret = ProtoConverter::ParseJsonIndexColumnValue(
      index_value, *meta, proto_meta, &output_value);
  ASSERT_EQ(ret, 0);
  const float *data = (const float *)(&(output_value[0]));
  for (uint32_t i = 1; i <= dimension; ++i) {
    ASSERT_FLOAT_EQ(1.0f * i, data[i - 1]);
  }
}

TEST_F(ProtoConverterTest,
       TestParseJsonIndexColumnValueaFp32WithTransformNoSupport) {
  std::string index_value("[1,2,3,4,5,6]");
  meta::ColumnMetaPtr meta = std::make_shared<meta::ColumnMeta>();
  meta->set_index_type(IndexTypes::PROXIMA_GRAPH_INDEX);
  meta->set_data_type(DataTypes::VECTOR_FP32);
  uint32_t dimension = 6;
  meta->set_dimension(dimension);
  proto::WriteRequest::IndexColumnMeta proto_meta;
  proto_meta.set_dimension(dimension);
  proto_meta.set_data_type(proto::DataType::DT_VECTOR_INT8);
  std::string output_value;
  int ret = ProtoConverter::ParseJsonIndexColumnValue(
      index_value, *meta, proto_meta, &output_value);
  ASSERT_EQ(ret, ErrorCode_MismatchedDataType);
}

TEST_F(ProtoConverterTest,
       TestParseJsonIndexColumnValueaFp32WithTransformSuccess) {
  std::string index_value("[1,2,3,4,5,6]");
  meta::ColumnMetaPtr meta = std::make_shared<meta::ColumnMeta>();
  meta->set_index_type(IndexTypes::PROXIMA_GRAPH_INDEX);
  meta->set_data_type(DataTypes::VECTOR_FP16);
  uint32_t dimension = 6;
  meta->set_dimension(dimension);
  proto::WriteRequest::IndexColumnMeta proto_meta;
  proto_meta.set_dimension(dimension);
  proto_meta.set_data_type(proto::DataType::DT_VECTOR_FP32);
  std::string output_value;
  int ret = ProtoConverter::ParseJsonIndexColumnValue(
      index_value, *meta, proto_meta, &output_value);
  ASSERT_EQ(ret, 0);
  const uint16_t *data = (const uint16_t *)(&(output_value[0]));
  for (uint32_t i = 1; i <= dimension; ++i) {
    ASSERT_FLOAT_EQ(1.0f * i, ailego::FloatHelper::ToFP32(data[i - 1]));
  }
}

TEST_F(ProtoConverterTest, TestParseJsonIndexColumnValueaFp16) {
  std::string index_value("[1,2,3,4,5,6]");
  meta::ColumnMetaPtr meta = std::make_shared<meta::ColumnMeta>();
  meta->set_index_type(IndexTypes::PROXIMA_GRAPH_INDEX);
  meta->set_data_type(DataTypes::VECTOR_FP16);
  uint32_t dimension = 6;
  meta->set_dimension(dimension);
  proto::WriteRequest::IndexColumnMeta proto_meta;
  proto_meta.set_dimension(dimension);
  proto_meta.set_data_type(proto::DataType::DT_VECTOR_FP16);
  std::string output_value;
  int ret = ProtoConverter::ParseJsonIndexColumnValue(
      index_value, *meta, proto_meta, &output_value);
  ASSERT_EQ(ret, 0);
  const uint16_t *data = (const uint16_t *)(&(output_value[0]));
  for (uint32_t i = 1; i <= dimension; ++i) {
    ASSERT_FLOAT_EQ(1.0f * i, ailego::FloatHelper::ToFP32(data[i - 1]));
  }
}

TEST_F(ProtoConverterTest, TestParseJsonIndexColumnValueaInt16) {
  std::string index_value("[1,2,3,4,5,6]");
  meta::ColumnMetaPtr meta = std::make_shared<meta::ColumnMeta>();
  meta->set_index_type(IndexTypes::PROXIMA_GRAPH_INDEX);
  meta->set_data_type(DataTypes::VECTOR_INT16);
  uint32_t dimension = 6;
  meta->set_dimension(dimension);
  proto::WriteRequest::IndexColumnMeta proto_meta;
  proto_meta.set_dimension(dimension);
  proto_meta.set_data_type(proto::DataType::DT_VECTOR_INT16);
  std::string output_value;
  int ret = ProtoConverter::ParseJsonIndexColumnValue(
      index_value, *meta, proto_meta, &output_value);
  ASSERT_EQ(ret, 0);
  const int16_t *data = (const int16_t *)(&(output_value[0]));
  for (uint32_t i = 1; i <= dimension; ++i) {
    ASSERT_FLOAT_EQ((int16_t)i, data[i - 1]);
  }
}

TEST_F(ProtoConverterTest, TestParseJsonIndexColumnValueaInt8) {
  std::string index_value("[1,2,3,4,5,6]");
  meta::ColumnMetaPtr meta = std::make_shared<meta::ColumnMeta>();
  meta->set_index_type(IndexTypes::PROXIMA_GRAPH_INDEX);
  meta->set_data_type(DataTypes::VECTOR_INT8);
  uint32_t dimension = 6;
  meta->set_dimension(dimension);
  proto::WriteRequest::IndexColumnMeta proto_meta;
  proto_meta.set_dimension(dimension);
  proto_meta.set_data_type(proto::DataType::DT_VECTOR_INT8);
  std::string output_value;
  int ret = ProtoConverter::ParseJsonIndexColumnValue(
      index_value, *meta, proto_meta, &output_value);
  ASSERT_EQ(ret, 0);
  const int8_t *data = (const int8_t *)(&(output_value[0]));
  for (uint32_t i = 1; i <= dimension; ++i) {
    ASSERT_FLOAT_EQ((int8_t)i, data[i - 1]);
  }
}

TEST_F(ProtoConverterTest, TestParseJsonIndexColumnValueaInt4) {
  std::string index_value("[1,2,3,4,5,6]");
  meta::ColumnMetaPtr meta = std::make_shared<meta::ColumnMeta>();
  meta->set_index_type(IndexTypes::PROXIMA_GRAPH_INDEX);
  meta->set_data_type(DataTypes::VECTOR_INT4);
  uint32_t dimension = 6;
  meta->set_dimension(dimension);
  proto::WriteRequest::IndexColumnMeta proto_meta;
  proto_meta.set_dimension(dimension);
  proto_meta.set_data_type(proto::DataType::DT_VECTOR_INT4);
  std::string output_value;
  int ret = ProtoConverter::ParseJsonIndexColumnValue(
      index_value, *meta, proto_meta, &output_value);
  ASSERT_EQ(ret, 0);
  const uint8_t *data = (const uint8_t *)(&(output_value[0]));
  for (uint32_t i = 1; i <= dimension / 2; ++i) {
    ASSERT_FLOAT_EQ((int8_t)(2 * i - 1), (int8_t)(data[i - 1] & 0xf));
    ASSERT_FLOAT_EQ((uint8_t)(2 * i), (int8_t)(data[i - 1] >> 4));
  }
}

TEST_F(ProtoConverterTest, TestParseJsonIndexColumnValueaBinary32) {
  std::string index_value("[1,2,3,4,5,6,7,8]");
  meta::ColumnMetaPtr meta = std::make_shared<meta::ColumnMeta>();
  meta->set_index_type(IndexTypes::PROXIMA_GRAPH_INDEX);
  meta->set_data_type(DataTypes::VECTOR_BINARY32);
  uint32_t dimension = 256;
  meta->set_dimension(dimension);
  proto::WriteRequest::IndexColumnMeta proto_meta;
  proto_meta.set_dimension(dimension);
  proto_meta.set_data_type(proto::DataType::DT_VECTOR_BINARY32);
  std::string output_value;
  int ret = ProtoConverter::ParseJsonIndexColumnValue(
      index_value, *meta, proto_meta, &output_value);
  ASSERT_EQ(ret, 0);
  const uint32_t *data = (const uint32_t *)(&(output_value[0]));
  for (uint32_t i = 1; i <= dimension / 32; ++i) {
    ASSERT_FLOAT_EQ((uint32_t)i, data[i - 1]);
  }
}

TEST_F(ProtoConverterTest, TestParseJsonIndexColumnValueaBinary64) {
  std::string index_value("[1,2,3,4,5,6,7,8]");
  meta::ColumnMetaPtr meta = std::make_shared<meta::ColumnMeta>();
  meta->set_index_type(IndexTypes::PROXIMA_GRAPH_INDEX);
  meta->set_data_type(DataTypes::VECTOR_BINARY64);
  uint32_t dimension = 512;
  meta->set_dimension(dimension);
  proto::WriteRequest::IndexColumnMeta proto_meta;
  proto_meta.set_dimension(dimension);
  proto_meta.set_data_type(proto::DataType::DT_VECTOR_BINARY64);
  std::string output_value;
  int ret = ProtoConverter::ParseJsonIndexColumnValue(
      index_value, *meta, proto_meta, &output_value);
  ASSERT_EQ(ret, 0);
  const uint64_t *data = (const uint64_t *)(&(output_value[0]));
  for (uint32_t i = 1; i <= dimension / 64; ++i) {
    ASSERT_FLOAT_EQ((uint64_t)i, data[i - 1]);
  }
}

TEST_F(ProtoConverterTest, TestParseBytesIndexColumnValueWithoutTransform) {
  std::vector<float> vectors = {1, 2, 3, 4, 5, 6};
  std::string index_value((const char *)vectors.data(),
                          vectors.size() * sizeof(float));
  meta::ColumnMetaPtr meta = std::make_shared<meta::ColumnMeta>();
  meta->set_index_type(IndexTypes::PROXIMA_GRAPH_INDEX);
  meta->set_data_type(DataTypes::VECTOR_FP32);
  uint32_t dimension = 6;
  meta->set_dimension(dimension);
  proto::WriteRequest::IndexColumnMeta proto_meta;
  proto_meta.set_dimension(dimension);
  proto_meta.set_data_type(proto::DataType::DT_VECTOR_FP32);
  std::string output_value;
  int ret = ProtoConverter::ParseBytesIndexColumnValue(
      index_value, *meta, proto_meta, &output_value);
  ASSERT_EQ(ret, 0);
  const float *data = (const float *)(&(output_value[0]));
  for (uint32_t i = 1; i <= dimension; ++i) {
    ASSERT_FLOAT_EQ(1.0f * i, data[i - 1]);
  }
}

TEST_F(ProtoConverterTest, TestParseBytesIndexColumnValueWithTransform) {
  std::vector<float> vectors = {1, 2, 3, 4, 5, 6};
  std::string index_value((const char *)vectors.data(),
                          vectors.size() * sizeof(float));
  meta::ColumnMetaPtr meta = std::make_shared<meta::ColumnMeta>();
  meta->set_index_type(IndexTypes::PROXIMA_GRAPH_INDEX);
  meta->set_data_type(DataTypes::VECTOR_FP16);
  uint32_t dimension = 6;
  meta->set_dimension(dimension);
  proto::WriteRequest::IndexColumnMeta proto_meta;
  proto_meta.set_dimension(dimension);
  proto_meta.set_data_type(proto::DataType::DT_VECTOR_FP32);
  std::string output_value;
  int ret = ProtoConverter::ParseBytesIndexColumnValue(
      index_value, *meta, proto_meta, &output_value);
  ASSERT_EQ(ret, 0);
  const uint16_t *data = (const uint16_t *)(&(output_value[0]));
  for (uint32_t i = 1; i <= dimension; ++i) {
    ASSERT_FLOAT_EQ(1.0f * i, ailego::FloatHelper::ToFP32(data[i - 1]));
  }
}

TEST_F(ProtoConverterTest, TestParseBytesIndexColumnValueWithTransformFailed) {
  std::vector<float> vectors = {1, 2, 3, 4, 5, 6};
  std::string index_value((const char *)vectors.data(),
                          vectors.size() * sizeof(float));
  meta::ColumnMetaPtr meta = std::make_shared<meta::ColumnMeta>();
  meta->set_index_type(IndexTypes::PROXIMA_GRAPH_INDEX);
  meta->set_data_type(DataTypes::VECTOR_FP16);
  uint32_t dimension = 6;
  meta->set_dimension(dimension);
  proto::WriteRequest::IndexColumnMeta proto_meta;
  proto_meta.set_dimension(dimension);
  proto_meta.set_data_type(proto::DataType::DT_VECTOR_INT16);
  std::string output_value;
  int ret = ProtoConverter::ParseBytesIndexColumnValue(
      index_value, *meta, proto_meta, &output_value);
  ASSERT_EQ(ret, ErrorCode_MismatchedDataType);
}

TEST_F(ProtoConverterTest, TestCopyBytesIndexColumnValueFp32) {
  std::vector<float> vectors = {1, 2, 3, 4, 5, 6};
  std::string index_value((const char *)vectors.data(),
                          vectors.size() * sizeof(float));
  meta::ColumnMetaPtr meta = std::make_shared<meta::ColumnMeta>();
  meta->set_index_type(IndexTypes::PROXIMA_GRAPH_INDEX);
  meta->set_data_type(DataTypes::VECTOR_FP32);
  uint32_t dimension = 6;
  meta->set_dimension(dimension);
  std::string output_value;
  int ret = ProtoConverter::CopyBytesIndexColumnValue(index_value, *meta,
                                                      &output_value);
  ASSERT_EQ(ret, 0);
  const float *data = (const float *)(&(output_value[0]));
  for (uint32_t i = 1; i <= dimension; ++i) {
    ASSERT_FLOAT_EQ(1.0f * i, data[i - 1]);
  }
}

TEST_F(ProtoConverterTest, TestCopyBytesIndexColumnValueaFp16) {
  std::vector<float> vectors = {1, 2, 3, 4, 5, 6};
  std::string index_value;
  index_value.resize(12);
  ailego::FloatHelper::ToFP16(reinterpret_cast<const float *>(vectors.data()),
                              vectors.size(),
                              reinterpret_cast<uint16_t *>(&index_value[0]));
  meta::ColumnMetaPtr meta = std::make_shared<meta::ColumnMeta>();
  meta->set_index_type(IndexTypes::PROXIMA_GRAPH_INDEX);
  meta->set_data_type(DataTypes::VECTOR_FP16);
  uint32_t dimension = 6;
  meta->set_dimension(dimension);
  std::string output_value;
  int ret = ProtoConverter::CopyBytesIndexColumnValue(index_value, *meta,
                                                      &output_value);
  ASSERT_EQ(ret, 0);
  const uint16_t *data = (const uint16_t *)(&(output_value[0]));
  for (uint32_t i = 1; i <= dimension; ++i) {
    ASSERT_FLOAT_EQ(1.0f * i, ailego::FloatHelper::ToFP32(data[i - 1]));
  }
}

TEST_F(ProtoConverterTest, TestCopyBytesIndexColumnValueaInt16) {
  std::vector<int16_t> vectors = {1, 2, 3, 4, 5, 6};
  std::string index_value((const char *)vectors.data(),
                          vectors.size() * sizeof(int16_t));
  meta::ColumnMetaPtr meta = std::make_shared<meta::ColumnMeta>();
  meta->set_index_type(IndexTypes::PROXIMA_GRAPH_INDEX);
  meta->set_data_type(DataTypes::VECTOR_INT16);
  uint32_t dimension = 6;
  meta->set_dimension(dimension);
  std::string output_value;
  int ret = ProtoConverter::CopyBytesIndexColumnValue(index_value, *meta,
                                                      &output_value);
  ASSERT_EQ(ret, 0);
  const int16_t *data = (const int16_t *)(&(output_value[0]));
  for (uint32_t i = 1; i <= dimension; ++i) {
    ASSERT_FLOAT_EQ((int16_t)i, data[i - 1]);
  }
}

TEST_F(ProtoConverterTest, TestCopyBytesIndexColumnValueaInt8) {
  std::vector<int8_t> vectors = {1, 2, 3, 4, 5, 6};
  std::string index_value((const char *)vectors.data(),
                          vectors.size() * sizeof(int8_t));
  meta::ColumnMetaPtr meta = std::make_shared<meta::ColumnMeta>();
  meta->set_index_type(IndexTypes::PROXIMA_GRAPH_INDEX);
  meta->set_data_type(DataTypes::VECTOR_INT8);
  uint32_t dimension = 6;
  meta->set_dimension(dimension);
  std::string output_value;
  int ret = ProtoConverter::CopyBytesIndexColumnValue(index_value, *meta,
                                                      &output_value);
  ASSERT_EQ(ret, 0);
  const int8_t *data = (const int8_t *)(&(output_value[0]));
  for (uint32_t i = 1; i <= dimension; ++i) {
    ASSERT_FLOAT_EQ((int8_t)i, data[i - 1]);
  }
}

TEST_F(ProtoConverterTest, TestCopyBytesIndexColumnValueaInt4) {
  std::vector<int8_t> vectors = {1, 2, 3, 4, 5, 6};
  std::string index_value;
  index_value.resize(3);
  uint8_t *out = reinterpret_cast<uint8_t *>(&index_value[0]);
  for (size_t i = 0; i < vectors.size(); i += 2) {
    out[i / 2] = (static_cast<uint8_t>(vectors[i + 1]) << 4) |
                 (static_cast<uint8_t>(vectors[i]) & 0xF);
  }
  meta::ColumnMetaPtr meta = std::make_shared<meta::ColumnMeta>();
  meta->set_index_type(IndexTypes::PROXIMA_GRAPH_INDEX);
  meta->set_data_type(DataTypes::VECTOR_INT4);
  uint32_t dimension = 6;
  meta->set_dimension(dimension);
  std::string output_value;
  int ret = ProtoConverter::CopyBytesIndexColumnValue(index_value, *meta,
                                                      &output_value);
  ASSERT_EQ(ret, 0);
  const uint8_t *data = (const uint8_t *)(&(output_value[0]));
  for (uint32_t i = 1; i <= dimension / 2; ++i) {
    ASSERT_FLOAT_EQ((int8_t)(2 * i - 1), (int8_t)(data[i - 1] & 0xf));
    ASSERT_FLOAT_EQ((uint8_t)(2 * i), (int8_t)(data[i - 1] >> 4));
  }
}

TEST_F(ProtoConverterTest, TestCopyBytesIndexColumnValueaBinary32) {
  std::vector<uint32_t> vectors = {1, 2, 3, 4, 5, 6, 7, 8};
  std::string index_value((const char *)vectors.data(),
                          vectors.size() * sizeof(uint32_t));
  meta::ColumnMetaPtr meta = std::make_shared<meta::ColumnMeta>();
  meta->set_index_type(IndexTypes::PROXIMA_GRAPH_INDEX);
  meta->set_data_type(DataTypes::VECTOR_BINARY32);
  uint32_t dimension = 256;
  meta->set_dimension(dimension);
  std::string output_value;
  int ret = ProtoConverter::CopyBytesIndexColumnValue(index_value, *meta,
                                                      &output_value);
  ASSERT_EQ(ret, 0);
  const uint32_t *data = (const uint32_t *)(&(output_value[0]));
  for (uint32_t i = 1; i <= dimension / 32; ++i) {
    ASSERT_FLOAT_EQ((uint32_t)i, data[i - 1]);
  }
}

TEST_F(ProtoConverterTest, TestCopyBytesIndexColumnValueaBinary64) {
  std::vector<uint64_t> vectors = {1, 2, 3, 4, 5, 6, 7, 8};
  std::string index_value((const char *)vectors.data(),
                          vectors.size() * sizeof(uint64_t));
  meta::ColumnMetaPtr meta = std::make_shared<meta::ColumnMeta>();
  meta->set_index_type(IndexTypes::PROXIMA_GRAPH_INDEX);
  meta->set_data_type(DataTypes::VECTOR_BINARY64);
  uint32_t dimension = 512;
  meta->set_dimension(dimension);
  std::string output_value;
  int ret = ProtoConverter::CopyBytesIndexColumnValue(index_value, *meta,
                                                      &output_value);
  ASSERT_EQ(ret, 0);
  const uint64_t *data = (const uint64_t *)(&(output_value[0]));
  for (uint32_t i = 1; i <= dimension / 64; ++i) {
    ASSERT_FLOAT_EQ((uint64_t)i, data[i - 1]);
  }
}

TEST_F(ProtoConverterTest, TestParseIndexColumnFailedWithParseJsonVector) {
  std::string index_value("[1,2,3,4,5,6,7,8");
  meta::ColumnMetaPtr meta = std::make_shared<meta::ColumnMeta>();
  meta->set_index_type(IndexTypes::PROXIMA_GRAPH_INDEX);
  meta->set_data_type(DataTypes::VECTOR_BINARY64);
  uint32_t dimension = 512;
  meta->set_dimension(dimension);
  proto::WriteRequest::IndexColumnMeta proto_meta;
  proto_meta.set_dimension(dimension);
  proto_meta.set_data_type(proto::DataType::DT_VECTOR_BINARY64);
  std::string output_value;
  int ret = ProtoConverter::ParseJsonIndexColumnValue(
      index_value, *meta, proto_meta, &output_value);
  ASSERT_EQ(ret, ErrorCode_MismatchedDimension);
}

// TEST_F(ProtoConverterTest, TestCompare) {
//     std::vector<std::string> vectors;
//     size_t count = 100000;
//     size_t dimension = 512;

//     std::cout << "Begin generate data: " << std::endl;
//     for (size_t i = 0; i < count; ++i) {
//         std::ostringstream oss;
//         oss << 1.0 / (i + 1);
//         for (size_t j = 1; j < dimension; ++j) {
//             oss << "," << 1.0 / (i + 1 + j);
//         }
//         vectors.emplace_back(oss.str());
//     }
//     std::cout << "End generate data: " << std::endl;

//     std::cout << "Begin process data: " << std::endl;
//     uint64_t start = ailego::Monotime::MilliSeconds();
//     for (size_t i = 0; i < count; ++i) {
//         std::vector<float> vector;
//         ailego::StringHelper::Split(vectors[i], ",", &vector);
//         if (vector.empty()) {
//             std::cout << "Failed " << i << std::endl;
//             continue;
//         }
//     }
//     uint64_t end = ailego::Monotime::MilliSeconds();
//     std::cout << "End process data: " << std::endl;

//     float qps = count * 1000.0 / (end - start);
//     std::cout << "total cost:" << (end - start) / 1000.0 << "s, qps: " << qps
//     << std::endl;
// }

// TEST_F(ProtoConverterTest, TestCompare1) {
//     std::vector<std::string> vectors;
//     size_t count = 100000;
//     size_t dimension = 512;

//     std::cout << "Begin generate data: " << std::endl;
//     for (size_t i = 0; i < count; ++i) {
//         std::ostringstream oss;
//         oss << "[" << 1.0 / (i + 1);
//         for (size_t j = 1; j < dimension; ++j) {
//             oss << "," << 1.0 / (i + 1 + j);
//         }
//         oss << "]";
//         vectors.emplace_back(oss.str());
//     }
//     std::cout << "End generate data: " << std::endl;

//     std::cout << "Begin process data: " << std::endl;
//     uint64_t start = ailego::Monotime::MilliSeconds();
//     for (size_t i = 0; i < count; ++i) {
//         std::vector<float> vector;
//         ailego::JsonValue root_node;
//         if (!root_node.parse(vectors[i].c_str())) {
//             std::cout << "Parse failed " << i << std::endl;
//             continue;
//         }
//         ailego::JsonArray &array = root_node.as_array();
//         for (auto it = array.begin(); it != array.end(); ++it) {
//             vector.emplace_back(it->as_float());
//         }
//         if (vector.size() != dimension) {
//             std::cout << "Failed " << i << std::endl;
//             continue;
//         }
//     }
//     uint64_t end = ailego::Monotime::MilliSeconds();
//     std::cout << "End process data: " << std::endl;

//     float qps = count * 1000.0 / (end - start);
//     std::cout << "total cost:" << (end - start) / 1000.0 << "s, qps: " << qps
//     << std::endl;
// }
