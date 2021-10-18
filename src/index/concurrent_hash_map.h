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
 *   \date     Oct 2020
 *   \brief    Concurrent hashmap which is thread-safe for add/del operations
 */

#pragma once

#include <unordered_map>
#include <ailego/parallel/lock.h>

namespace proxima {
namespace be {
namespace index {

/*
 * Concurrent hash map for synchronously get/set item.
 */
template <typename TKey, typename TValue>
class ConcurrentHashMap {
 public:
  //! Constructor
  ConcurrentHashMap() = default;

  //! Destructor
  ~ConcurrentHashMap() = default;

 public:
  //! Emplace a key-value pair
  void emplace(TKey key, TValue val) {
    rw_lock_.lock();
    map_.emplace(key, val);
    rw_lock_.unlock();
  }

  //! Get value by key
  const TValue &get(TKey key) const {
    rw_lock_.lock_shared();
    const TValue &val = map_.at(key);
    rw_lock_.unlock_shared();
    return val;
  }

  //! If has key
  bool has(TKey key) const {
    bool found = false;
    rw_lock_.lock_shared();
    if (map_.find(key) != map_.end()) {
      found = true;
    }
    rw_lock_.unlock_shared();
    return found;
  }

  //! Return key-value pair count
  size_t size() const {
    size_t val;
    rw_lock_.lock_shared();
    val = map_.size();
    rw_lock_.unlock_shared();
    return val;
  }

  //! Erase a pair by key
  void erase(TKey key) {
    rw_lock_.lock();
    map_.erase(key);
    rw_lock_.unlock();
  }

  //! Clear all pairs
  void clear() {
    rw_lock_.lock();
    map_.clear();
    rw_lock_.unlock();
  }

  typedef typename std::unordered_map<TKey, TValue>::iterator iterator;
  typedef
      typename std::unordered_map<TKey, TValue>::const_iterator const_iterator;

  // NOTICE: iterator interface is only called when
  // uppper class destruct, and ensure in single thread
  //
  // Return begin iterator
  iterator begin() {
    rw_lock_.lock_shared();
    iterator it = map_.begin();
    rw_lock_.unlock_shared();
    return it;
  }

  //! Return const begin iterator
  const_iterator begin() const {
    rw_lock_.lock_shared();
    const_iterator it = map_.begin();
    rw_lock_.unlock_shared();
    return it;
  }

  //! Return reverse begin iterator
  const_iterator cbegin() const {
    rw_lock_.lock_shared();
    const_iterator it = map_.cbegin();
    rw_lock_.unlock_shared();
    return it;
  }

  //! Return end iterator
  iterator end() {
    rw_lock_.lock_shared();
    iterator it = map_.end();
    rw_lock_.unlock_shared();
    return it;
  }

  //! Return const end iterator
  const_iterator end() const {
    rw_lock_.lock_shared();
    const_iterator it = map_.end();
    rw_lock_.unlock_shared();
    return it;
  }

  //! Return cend iterator
  const_iterator cend() const {
    rw_lock_.lock_shared();
    const_iterator it = map_.cend();
    rw_lock_.unlock_shared();
    return it;
  }

 private:
  mutable ailego::SharedMutex rw_lock_{};
  std::unordered_map<TKey, TValue> map_{};
};


}  // end namespace index
}  // namespace be
}  // end namespace proxima
