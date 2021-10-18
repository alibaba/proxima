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

 *   \author   Hechong.xyf
 *   \date     Mar 2018
 *   \brief    Interface of AiTheta Index Factory
 */

#ifndef __AITHETA2_INDEX_FACTORY_H__
#define __AITHETA2_INDEX_FACTORY_H__

#include <ailego/pattern/factory.h>
#include "index_closet.h"
#include "index_container.h"
#include "index_converter.h"
#include "index_dumper.h"
#include "index_logger.h"
#include "index_measure.h"
#include "index_reformer.h"
#include "index_searcher.h"
#include "index_storage.h"
#include "index_streamer.h"

namespace aitheta2 {

/*! Index Factory
 */
struct IndexFactory {
  //! Create a index measure by name
  static IndexMeasure::Pointer CreateMeasure(const std::string &name);

  //! Test if the measure is exist
  static bool HasMeasure(const std::string &name);

  //! Retrieve all measure classes
  static std::vector<std::string> AllMeasures(void);

  //! Create a index logger by name
  static IndexLogger::Pointer CreateLogger(const std::string &name);

  //! Test if the logger is exist
  static bool HasLogger(const std::string &name);

  //! Create a index dumper by name
  static IndexDumper::Pointer CreateDumper(const std::string &name);

  //! Test if the dumper is exist
  static bool HasDumper(const std::string &name);

  //! Create a index container by name
  static IndexContainer::Pointer CreateContainer(const std::string &name);

  //! Test if the container is exist
  static bool HasContainer(const std::string &name);

  //! Create a index storage by name
  static IndexStorage::Pointer CreateStorage(const std::string &name);

  //! Test if the storage is exist
  static bool HasStorage(const std::string &name);

  //! Create a index converter by name
  static IndexConverter::Pointer CreateConverter(const std::string &name);

  //! Test if the converter is exist
  static bool HasConverter(const std::string &name);

  //! Create a index reformer by name
  static IndexReformer::Pointer CreateReformer(const std::string &name);

  //! Test if the reformer is exist
  static bool HasReformer(const std::string &name);

  //! Create a index searcher by name
  static IndexSearcher::Pointer CreateSearcher(const std::string &name);

  //! Test if the searcher is exist
  static bool HasSearcher(const std::string &name);

  //! Create a index streamer by name
  static IndexStreamer::Pointer CreateStreamer(const std::string &name);

  //! Test if the streamer is exist
  static bool HasStreamer(const std::string &name);

  //! Create a index closet by name
  static IndexCloset::Pointer CreateCloset(const std::string &name);

  //! Test if the closet is exist
  static bool HasCloset(const std::string &name);

  //! Create a index closet searcher by name
  static IndexImmutableCloset::Pointer CreateImmutableCloset(
      const std::string &name);

  //! Test if the closet searcher exists
  static bool HasImmutableCloset(const std::string &name);
};

//! Register Index Measure
#define INDEX_FACTORY_REGISTER_MEASURE_ALIAS(__NAME__, __IMPL__, ...) \
  AILEGO_FACTORY_REGISTER(__NAME__, aitheta2::IndexMeasure, __IMPL__, \
                          ##__VA_ARGS__)

//! Register Index Measure
#define INDEX_FACTORY_REGISTER_MEASURE(__IMPL__, ...) \
  INDEX_FACTORY_REGISTER_MEASURE_ALIAS(__IMPL__, __IMPL__, ##__VA_ARGS__)

//! Register Index Logger
#define INDEX_FACTORY_REGISTER_LOGGER_ALIAS(__NAME__, __IMPL__, ...) \
  AILEGO_FACTORY_REGISTER(__NAME__, aitheta2::IndexLogger, __IMPL__, \
                          ##__VA_ARGS__)

//! Register Index Logger
#define INDEX_FACTORY_REGISTER_LOGGER(__IMPL__, ...) \
  INDEX_FACTORY_REGISTER_LOGGER_ALIAS(__IMPL__, __IMPL__, ##__VA_ARGS__)

//! Register Index Dumper
#define INDEX_FACTORY_REGISTER_DUMPER_ALIAS(__NAME__, __IMPL__, ...) \
  AILEGO_FACTORY_REGISTER(__NAME__, aitheta2::IndexDumper, __IMPL__, \
                          ##__VA_ARGS__)

//! Register Index Dumper
#define INDEX_FACTORY_REGISTER_DUMPER(__IMPL__, ...) \
  INDEX_FACTORY_REGISTER_DUMPER_ALIAS(__IMPL__, __IMPL__, ##__VA_ARGS__)

//! Register Index Container
#define INDEX_FACTORY_REGISTER_CONTAINER_ALIAS(__NAME__, __IMPL__, ...) \
  AILEGO_FACTORY_REGISTER(__NAME__, aitheta2::IndexContainer, __IMPL__, \
                          ##__VA_ARGS__)

//! Register Index Container
#define INDEX_FACTORY_REGISTER_CONTAINER(__IMPL__, ...) \
  INDEX_FACTORY_REGISTER_CONTAINER_ALIAS(__IMPL__, __IMPL__, ##__VA_ARGS__)

//! Register Index Storage
#define INDEX_FACTORY_REGISTER_STORAGE_ALIAS(__NAME__, __IMPL__, ...) \
  AILEGO_FACTORY_REGISTER(__NAME__, aitheta2::IndexStorage, __IMPL__, \
                          ##__VA_ARGS__)

//! Register Index Storage
#define INDEX_FACTORY_REGISTER_STORAGE(__IMPL__, ...) \
  INDEX_FACTORY_REGISTER_STORAGE_ALIAS(__IMPL__, __IMPL__, ##__VA_ARGS__)

//! Register Index Converter
#define INDEX_FACTORY_REGISTER_CONVERTER_ALIAS(__NAME__, __IMPL__, ...) \
  AILEGO_FACTORY_REGISTER(__NAME__, aitheta2::IndexConverter, __IMPL__, \
                          ##__VA_ARGS__)

//! Register Index Converter
#define INDEX_FACTORY_REGISTER_CONVERTER(__IMPL__, ...) \
  INDEX_FACTORY_REGISTER_CONVERTER_ALIAS(__IMPL__, __IMPL__, ##__VA_ARGS__)

//! Register Index Reformer
#define INDEX_FACTORY_REGISTER_REFORMER_ALIAS(__NAME__, __IMPL__, ...) \
  AILEGO_FACTORY_REGISTER(__NAME__, aitheta2::IndexReformer, __IMPL__, \
                          ##__VA_ARGS__)

//! Register Index Reformer
#define INDEX_FACTORY_REGISTER_REFORMER(__IMPL__, ...) \
  INDEX_FACTORY_REGISTER_REFORMER_ALIAS(__IMPL__, __IMPL__, ##__VA_ARGS__)

//! Register Index Searcher
#define INDEX_FACTORY_REGISTER_SEARCHER_ALIAS(__NAME__, __IMPL__, ...) \
  AILEGO_FACTORY_REGISTER(__NAME__, aitheta2::IndexSearcher, __IMPL__, \
                          ##__VA_ARGS__)

//! Register Index Searcher
#define INDEX_FACTORY_REGISTER_SEARCHER(__IMPL__, ...) \
  INDEX_FACTORY_REGISTER_SEARCHER_ALIAS(__IMPL__, __IMPL__, ##__VA_ARGS__)

//! Register Index Streamer
#define INDEX_FACTORY_REGISTER_STREAMER_ALIAS(__NAME__, __IMPL__, ...) \
  AILEGO_FACTORY_REGISTER(__NAME__, aitheta2::IndexStreamer, __IMPL__, \
                          ##__VA_ARGS__)

//! Register Index Streamer
#define INDEX_FACTORY_REGISTER_STREAMER(__IMPL__, ...) \
  INDEX_FACTORY_REGISTER_STREAMER_ALIAS(__IMPL__, __IMPL__, ##__VA_ARGS__)

//! Register Index Closet
#define INDEX_FACTORY_REGISTER_CLOSET_ALIAS(__NAME__, __IMPL__, ...) \
  AILEGO_FACTORY_REGISTER(__NAME__, aitheta2::IndexCloset, __IMPL__, \
                          ##__VA_ARGS__)

//! Register Index Closet
#define INDEX_FACTORY_REGISTER_CLOSET(__IMPL__, ...) \
  INDEX_FACTORY_REGISTER_CLOSET_ALIAS(__IMPL__, __IMPL__, ##__VA_ARGS__)

//! Register Index Closet Searcher
#define INDEX_FACTORY_REGISTER_IMMUTABLE_CLOSET_ALIAS(__NAME__, __IMPL__, ...) \
  AILEGO_FACTORY_REGISTER(__NAME__, aitheta2::IndexImmutableCloset, __IMPL__,  \
                          ##__VA_ARGS__)

//! Register Index Closet Searcher
#define INDEX_FACTORY_REGISTER_IMMUTABLE_CLOSET(__IMPL__, ...)      \
  INDEX_FACTORY_REGISTER_IMMUTABLE_CLOSET_ALIAS(__IMPL__, __IMPL__, \
                                                ##__VA_ARGS__)

}  // namespace aitheta2

#endif  // __AITHETA2_INDEX_FACTORY_H__
