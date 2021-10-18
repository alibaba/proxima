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
 *   \date     Nov 2020
 *   \brief    Scope guard like golang defer
 */

#pragma once

#include <functional>
#include <vector>

namespace proxima {
namespace be {

/*
 * RAII style scope guard to ensure release resource
 */
class Defer {
 public:
  using Func = std::function<void(void)>;

  Defer() = default;

  Defer(Func f) {
    funcs_.emplace_back(f);
  }

  Defer(const Defer &) = delete;

  Defer &operator=(const Defer &) = delete;

  ~Defer() {
    for (auto func : funcs_) {
      func();
    }

    funcs_.clear();
  }

 public:
  void operator()(Func f) {
    funcs_.emplace_back(f);
  }

 private:
  std::vector<Func> funcs_;
};

}  // namespace be
}  // end namespace proxima
