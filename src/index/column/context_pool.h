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
 *   \brief    Reserving context of vector inserting/searching
 */

#pragma once

#include <condition_variable>
#include <memory>
#include <queue>
#include "common/macro_define.h"
#include "../typedef.h"

namespace proxima {
namespace be {
namespace index {

/*
 * Storage of proxima search context
 */
class ContextPool {
 public:
  PROXIMA_DISALLOW_COPY_AND_ASSIGN(ContextPool);

  //! Constructor
  ContextPool() = default;

  //! Destructor
  ~ContextPool() = default;

  //! Emplace a context from pool
  void emplace(IndexContextPtr ctx);

  //! Acauire a context from pool
  IndexContextPtr acquire();

  //! Return a context to pool
  void release(IndexContextPtr ctx);

  //! Clear context pool
  void clear();

 private:
  std::queue<IndexContextPtr> contexts_{};
  std::mutex mutex_{};
  std::condition_variable_any not_empty_cond_{};
};


}  // end namespace index
}  // namespace be
}  // end namespace proxima
