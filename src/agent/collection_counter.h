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

 *   \author   Hongqing.hu
 *   \date     Dec 2020
 *   \brief    Collection counter interface definition for proxima search engine
 */

#pragma once

#include <atomic>
#include <memory>
#include <mutex>
#include <unordered_map>

namespace proxima {
namespace be {
namespace agent {

class CollectionCounter;
class CollectionCounterMap;

using CollectionCounterPtr = std::shared_ptr<CollectionCounter>;
using CollectionCounterMapPtr = std::shared_ptr<CollectionCounterMap>;

/*! CollectionCounter
 */
class CollectionCounter {
 public:
  //! Constructor
  CollectionCounter() = default;

  //! Destructor
  ~CollectionCounter() = default;

  //! Add active count
  void add_active_count(uint32_t count) {
    active_count_.fetch_add(count);
  }

  //! Sub active count
  void sub_active_count(uint32_t count) {
    active_count_.fetch_sub(count);
  }

  //! Sub one active count
  void dec_active_count() {
    --active_count_;
  }

  //! Get active count
  uint32_t active_count() const {
    return active_count_.load();
  }

 private:
  //! Collection active record count
  std::atomic<uint32_t> active_count_{0U};
};

/*! CollectionCounterMap
 */
class CollectionCounterMap {
 public:
  //! Constructor
  CollectionCounterMap() = default;

  //! Destructor
  ~CollectionCounterMap() = default;

  //! Add collection counter
  void add_counter(const std::string &name) {
    CollectionCounterPtr counter = std::make_shared<CollectionCounter>();
    std::lock_guard<std::mutex> lock(mutex_);
    counter_map_[name] = counter;
  }

  //! Remove collection counter
  void remove_counter(const std::string &name) {
    std::lock_guard<std::mutex> lock(mutex_);
    counter_map_.erase(name);
  }

  //! Get collection counter
  CollectionCounterPtr get_counter(const std::string &name) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = counter_map_.find(name);
    if (it != counter_map_.end()) {
      return it->second;
    }
    return nullptr;
  }

 private:
  //! Mutex
  std::mutex mutex_{};
  //! Collection counter map
  std::unordered_map<std::string, CollectionCounterPtr> counter_map_{};
};

}  // end namespace agent
}  // namespace be
}  // end namespace proxima
