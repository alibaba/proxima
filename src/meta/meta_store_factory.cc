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

#include "meta_store_factory.h"
#include "common/logger.h"

namespace proxima {
namespace be {
namespace meta {

MetaStorePtr MetaStoreFactory::create(const char *name,
                                      const ailego::Uri *uri) {
  auto iter = store_cache_.find(name);
  if (iter != store_cache_.end()) {
    return iter->second;
  }

  if (ailego::Factory<MetaStore>::Has(name)) {
    auto meta = ailego::Factory<MetaStore>::MakeShared(name);
    if (meta->initialize(uri) == 0) {
      store_cache_[name] = meta;
      return meta;
    } else {
      LOG_ERROR("Failed to init meta store");
    }
  }
  return nullptr;
}

}  // namespace meta
}  // namespace be
}  // namespace proxima
