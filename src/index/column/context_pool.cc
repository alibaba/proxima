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
 *   \brief    Implementation of context pool
 */

#include "context_pool.h"

namespace proxima {
namespace be {
namespace index {

void ContextPool::emplace(IndexContextPtr ctx) {
  std::lock_guard<std::mutex> lock(mutex_);
  contexts_.push(std::move(ctx));
}

IndexContextPtr ContextPool::acquire() {
  IndexContextPtr ctx;
  std::unique_lock<std::mutex> lock(mutex_);
  not_empty_cond_.wait(lock, [this] { return !contexts_.empty(); });
  ctx = std::move(contexts_.front());
  contexts_.pop();
  return ctx;
}

void ContextPool::release(IndexContextPtr ctx) {
  std::lock_guard<std::mutex> lock(mutex_);
  contexts_.push(std::move(ctx));
  not_empty_cond_.notify_one();
}

void ContextPool::clear() {
  std::lock_guard<std::mutex> lock(mutex_);
  std::queue<IndexContextPtr> empty_q;
  contexts_.swap(empty_q);
}


}  // end namespace index
}  // namespace be
}  // end namespace proxima
