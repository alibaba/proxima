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
 *   \brief    Concurrent bitmap which is thread-safe for add/del operations
 */

#pragma once

#include <ailego/container/bitmap.h>
#include <ailego/parallel/lock.h>

namespace proxima {
namespace be {
namespace index {

/*
 * Concurrent bitmap, support test/set in multi-threads
 */
class ConcurrentBitmap {
 public:
  //! Construtor
  ConcurrentBitmap() = default;

  //! Destructor
  ~ConcurrentBitmap() = default;

 public:
  //! Test if num pos bit exist
  bool test(size_t num) const {
    bool found = false;
    // Remove shared lock here!!
    // It will slow down search performance significantly
    found = bitmap_.test(num);
    return found;
  }

  //! Set num pos bit
  void set(size_t num) {
    rw_lock_.lock();
    bitmap_.set(num);
    rw_lock_.unlock();
  }

  //! Reset num pos bit
  void reset(size_t num) {
    rw_lock_.lock();
    bitmap_.reset(num);
    rw_lock_.unlock();
  }

  //! Cleanup all bits
  void clear() {
    rw_lock_.lock();
    bitmap_.clear();
    rw_lock_.unlock();
  }

 private:
  mutable ailego::SharedMutex rw_lock_{};
  ailego::Bitmap bitmap_{};
};


}  // end namespace index
}  // namespace be
}  // end namespace proxima
