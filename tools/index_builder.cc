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

 *   \author   Haichao.chc
 *   \date     Jun 2021
 *   \brief    Index builder tool can create collection index
 *             from text or vecs file
 */

#include <fstream>
#include <iostream>
#include <gflags/gflags.h>
#include "common/logger.h"
#include "common/protobuf_helper.h"
#include "common/types.h"
#include "common/version.h"
#include "index/collection.h"
#include "index/typedef.h"
#include "meta/meta.h"
#include "proto/proxima_be.pb.h"
#include "vecs_reader.h"

using namespace proxima::be;

DEFINE_string(schema, "", "Specify the schema of collection");
DEFINE_string(file, "", "Specify input data file");
DEFINE_string(output, "./", "Sepecify output index directory");
DEFINE_uint32(concurrency, 10, "Threads count for building index");

static bool ValidateNotEmpty(const char *flagname, const std::string &value) {
  return !value.empty();
}

DEFINE_validator(schema, ValidateNotEmpty);
DEFINE_validator(file, ValidateNotEmpty);

struct Record {
  uint64_t key;
  std::string vector;
  std::string attributes;
  uint32_t dimension;
};

meta::CollectionMetaPtr g_collection_meta;

static inline void PrintUsage() {
  std::cout << "Usage:" << std::endl;
  std::cout << " index_builder <args>" << std::endl << std::endl;
  std::cout << "Args: " << std::endl;
  std::cout << " --schema           Specify the schema of collection"
            << std::endl;
  std::cout << " --file             Specify input data file" << std::endl;
  std::cout << " --output           Sepecify output index directory(default ./)"
            << std::endl;
  std::cout << " --concurrency      Sepecify threads count for building "
               "index(default 10)"
            << std::endl;
  std::cout << " --help, -h         Dipslay help info" << std::endl;
  std::cout << " --version, -v      Dipslay version info" << std::endl;
}

static bool ParseSchema() {
  // Parse protobuf object from input json
  ProtobufHelper::JsonParseOptions options;
  options.ignore_unknown_fields = true;

  proto::CollectionConfig collection_config;
  if (!ProtobufHelper::JsonToMessage(FLAGS_schema, &collection_config)) {
    LOG_ERROR("JsonToMessage failed. schema[%s]", FLAGS_schema.c_str());
    return false;
  }

  std::string converted_json;
  ProtobufHelper::MessageToJson(collection_config, &converted_json);

  // Check input schema format
  if (collection_config.collection_name().empty()) {
    LOG_ERROR("Collection name can't be empty. schema[%s]",
              converted_json.c_str());
    return false;
  }

  if (collection_config.index_column_params_size() != 1) {
    LOG_ERROR("Schema must contain an index column. schema[%s]",
              converted_json.c_str());
    return false;
  }

  auto *index_column_schema = collection_config.mutable_index_column_params(0);
  if (index_column_schema->column_name().empty()) {
    LOG_ERROR("Schema index column name can't be empty. schema[%s]",
              converted_json.c_str());
    return false;
  }

  if (index_column_schema->index_type() == proto::IndexType::IT_UNDEFINED) {
    index_column_schema->set_index_type(
        proto::IndexType::IT_PROXIMA_GRAPH_INDEX);
  }

  if (index_column_schema->data_type() == proto::DataType::DT_UNDEFINED) {
    index_column_schema->set_data_type(proto::DataType::DT_VECTOR_FP32);
  }

  if (index_column_schema->dimension() == 0U) {
    LOG_ERROR("Schema index column dimension must be set. schema[%s]",
              converted_json.c_str());
    return false;
  }

  if (collection_config.forward_column_names_size() > 1) {
    LOG_ERROR("Schema can contain a forward column at most. schema[%s]",
              converted_json.c_str());
    return false;
  }

  // Generate collection meta from schema
  g_collection_meta = std::make_shared<meta::CollectionMeta>();
  g_collection_meta->set_name(collection_config.collection_name());

  // Set forward column
  if (collection_config.forward_column_names_size() > 0) {
    g_collection_meta->mutable_forward_columns()->emplace_back(
        collection_config.forward_column_names(0));
  }

  // Set index column
  auto &column_param = collection_config.index_column_params(0);
  auto new_column_meta = std::make_shared<meta::ColumnMeta>();
  new_column_meta->set_name(column_param.column_name());
  new_column_meta->set_index_type((IndexTypes)column_param.index_type());
  new_column_meta->set_data_type((DataTypes)column_param.data_type());
  new_column_meta->set_dimension(column_param.dimension());
  for (int j = 0; j < column_param.extra_params_size(); j++) {
    auto &kvpair = column_param.extra_params(j);
    new_column_meta->mutable_parameters()->set(kvpair.key(), kvpair.value());
  }
  g_collection_meta->append(new_column_meta);

  std::cout << "Parse collection schema success. schema[" << FLAGS_schema << "]"
            << std::endl;
  return true;
}

static void DoInsertCollection(index::Collection *collection,
                               const Record &record) {
  index::CollectionDataset records(0);
  auto *row = records.add_row_data();
  row->operation_type = OperationTypes::INSERT;
  row->primary_key = record.key;

  // Serialize forward data to pb format
  if (g_collection_meta->forward_columns().size() > 0) {
    proto::GenericValueList value_list;
    auto *value = value_list.add_values();
    value->set_string_value(record.attributes);
    value_list.SerializeToString(&row->forward_data);
  }

  // Append index column data
  auto &index_column_schema = g_collection_meta->index_columns().at(0);
  index::ColumnData index_column;
  index_column.column_name = index_column_schema->name();
  index_column.data_type = (DataTypes)index_column_schema->data_type();
  index_column.dimension = record.dimension;
  index_column.data = record.vector;
  row->column_datas.emplace_back(index_column);

  collection->write_records(records);
}

static bool LoadFromVecsFile(aitheta2::IndexThreads::TaskGroup *group,
                             index::Collection *collection) {
  tools::VecsReader reader;
  if (!reader.load(FLAGS_file)) {
    LOG_ERROR("Load vecs file failed.");
    return false;
  }

  for (uint32_t i = 0; i < reader.num_vecs(); i++) {
    Record record;
    uint64_t key = reader.get_key(i);
    const char *feature = (const char *)reader.get_vector(i);

    record.key = key;
    record.vector.append(feature, reader.index_meta().element_size());
    record.dimension = reader.index_meta().dimension();
    group->submit(ailego::Closure::New(DoInsertCollection, collection, record));
  }
  return true;
}

static bool LoadFromTextFile(aitheta2::IndexThreads::TaskGroup *group,
                             index::Collection *collection) {
  std::ifstream file_stream(FLAGS_file);
  if (!file_stream.is_open()) {
    LOG_ERROR("Can't open input file[%s]", FLAGS_file.c_str());
    return false;
  }

  std::string line;
  while (std::getline(file_stream, line)) {
    line.erase(line.find_last_not_of('\n') + 1);
    if (line.empty()) {
      continue;
    }
    std::vector<std::string> res;
    ailego::StringHelper::Split(line, ';', &res);
    if (res.size() < 2) {
      LOG_ERROR("Bad input line, format[key;vector(1 2 3 4...);attributes]");
      continue;
    }

    Record record;
    // Parse key
    uint64_t key = std::stoull(res[0]);
    record.key = key;
    // Parse feature
    if (res.size() >= 2) {
      auto data_type = g_collection_meta->index_columns().at(0)->data_type();
      if (data_type == DataTypes::VECTOR_BINARY32) {
        std::vector<uint8_t> vec;
        ailego::StringHelper::Split(res[1], ' ', &vec);
        if (vec.size() == 0 || vec.size() % 32 != 0) {
          LOG_ERROR("Bad feature field");
          continue;
        }

        std::vector<uint8_t> tmp;
        for (size_t i = 0; i < vec.size(); i += 8) {
          uint8_t v = 0;
          v |= (vec[i] & 0x01) << 7;
          v |= (vec[i + 1] & 0x01) << 6;
          v |= (vec[i + 2] & 0x01) << 5;
          v |= (vec[i + 3] & 0x01) << 4;
          v |= (vec[i + 4] & 0x01) << 3;
          v |= (vec[i + 5] & 0x01) << 2;
          v |= (vec[i + 6] & 0x01) << 1;
          v |= (vec[i + 7] & 0x01) << 0;
          tmp.push_back(v);
        }

        record.vector =
            std::string((const char *)tmp.data(), tmp.size() * sizeof(uint8_t));
        record.dimension = vec.size();
      } else {
        if (data_type == DataTypes::VECTOR_FP32) {
          std::vector<float> feature;
          ailego::StringHelper::Split(res[1], ' ', &feature);
          if (feature.size() == 0) {
            LOG_ERROR("Bad feature field");
            continue;
          }
          record.vector = std::string((const char *)feature.data(),
                                      feature.size() * sizeof(float));
          record.dimension = feature.size();
        } else if (data_type == DataTypes::VECTOR_INT8) {
          std::vector<int8_t> feature;
          ailego::StringHelper::Split(res[1], ' ', &feature);
          if (feature.size() == 0) {
            LOG_ERROR("Bad feature field");
            continue;
          }
          record.vector = std::string((const char *)feature.data(),
                                      feature.size() * sizeof(int8_t));
          record.dimension = feature.size();
        }
      }
    }
    // Parse attributes
    if (res.size() >= 3) {
      record.attributes = res[2];
    }

    group->submit(ailego::Closure::New(DoInsertCollection, collection, record));
  }
  file_stream.close();

  return true;
}

static bool BuildIndex() {
  index::ThreadPool thread_pool(FLAGS_concurrency, false);

  // Create and open new collection
  index::CollectionPtr new_collection;
  index::ReadOptions read_options;
  read_options.use_mmap = true;
  read_options.create_new = true;
  int ret = index::Collection::CreateAndOpen(
      g_collection_meta->name(), FLAGS_output, g_collection_meta,
      FLAGS_concurrency, &thread_pool, read_options, &new_collection);
  if (ret != 0) {
    return false;
  }
  std::cout << "Create collection complete. collection["
            << g_collection_meta->name() << "]" << std::endl;

  // Writing into collection in parallel
  auto group = thread_pool.make_group();

  if (FLAGS_file.find(".vecs") != std::string::npos) {
    if (!LoadFromVecsFile(group.get(), new_collection.get())) {
      return false;
    }
  } else {
    if (!LoadFromTextFile(group.get(), new_collection.get())) {
      return false;
    }
  }

  group->wait_finish();
  std::cout << "Build index complete. collection[" << g_collection_meta->name()
            << "]" << std::endl;

  // Dump to disk
  ret = new_collection->dump();
  if (ret != 0) {
    return false;
  }

  ret = new_collection->close();
  if (ret != 0) {
    return false;
  }
  std::cout << "Dump index complete. collection[" << g_collection_meta->name()
            << "]" << std::endl;

  return true;
}

int main(int argc, char **argv) {
  // Parse arguments
  for (int i = 1; i < argc; ++i) {
    const char *arg = argv[i];
    if (!strcmp(arg, "-help") || !strcmp(arg, "--help") || !strcmp(arg, "-h")) {
      PrintUsage();
      exit(0);
    } else if (!strcmp(arg, "-version") || !strcmp(arg, "--version") ||
               !strcmp(arg, "-v")) {
      std::cout << proxima::be::Version::Details() << std::endl;
      exit(0);
    }
  }
  gflags::ParseCommandLineNonHelpFlags(&argc, &argv, false);

  // Adjust log level to prevent print too many logs
  aitheta2::IndexLoggerBroker::SetLevel(aitheta2::IndexLogger::LEVEL_WARN);

  // Parse schema
  if (!ParseSchema()) {
    LOG_ERROR("Parse schema failed.");
    exit(1);
  }

  // Start to build index
  if (!BuildIndex()) {
    LOG_ERROR("Build index error.");
    exit(1);
  }
}
