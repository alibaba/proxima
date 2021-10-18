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
 *   \date     March 2021
 *   \brief    RAII type atomic counter
 */

#include <atomic>

namespace proxima {
namespace be {

/*
 * AutoCounter is a simple RAII type atomic counter
 */
struct AutoCounter {
  AutoCounter(std::atomic<uint64_t> &ctr) : counter_(ctr) {
    counter_++;
  }

  ~AutoCounter() {
    counter_--;
  }

  std::atomic<uint64_t> &counter_;
};


}  // namespace be
}  // end namespace proxima
