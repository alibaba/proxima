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
 *   \date     Oct 2020
 *   \brief
 */

#pragma once

#include <ailego/pattern/factory.h>
#include <ailego/pattern/singleton.h>
#include "meta_store.h"

namespace proxima {
namespace be {
namespace meta {

/*!
 * MetaStoreFactory in charging create meta store object
 */
class MetaStoreFactory : public ailego::Singleton<MetaStoreFactory> {
 public:
  //! Alias for Cache & CacheIter
  using Cache = std::map<std::string, MetaStorePtr>;
  using CacheIter = Cache::iterator;

 public:
  //! Return singleton MateStore instance
  MetaStorePtr create(const char *name, const ailego::Uri *uri);

 private:
  //! Meta store cache
  Cache store_cache_{};
};


//! Register MetaStore
#define META_FACTORY_REGISTER_INSTANCE_ALIAS(__NAME__, __IMPL__, ...) \
  AILEGO_FACTORY_REGISTER(__NAME__, MetaStore, __IMPL__, ##__VA_ARGS__)


}  // namespace meta
}  // namespace be
}  // namespace proxima
