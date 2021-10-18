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
 *   \date     Feb 2020
 *   \brief    Interface of AiLego defer operator
 */

#ifndef __AILEGO_PATTERN_DEFER_H__
#define __AILEGO_PATTERN_DEFER_H__

#include "scope_guard.h"

#define AILEGO_DEFER_NAME_(x, y) x##y
#define AILEGO_DEFER_NAME(x) AILEGO_DEFER_NAME_(__ailegoDefer_, x)

//! Defer operator
#define AILEGO_DEFER(...) \
  auto AILEGO_DEFER_NAME(__LINE__) = ailego::ScopeGuard::Make(__VA_ARGS__)

#endif  // __AILEGO_PATTERN_DEFER_H__
