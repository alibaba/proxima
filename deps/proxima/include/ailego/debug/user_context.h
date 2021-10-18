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
 *   \date     Dec 2020
 *   \brief    Interface of AiLego User Thread Context
 */

#ifndef __AILEGO_DEBUG_USER_CONTEXT_H__
#define __AILEGO_DEBUG_USER_CONTEXT_H__

#if defined(__linux) || defined(__linux__)
#include "user_context_linux.h"
#elif defined(__APPLE__) || defined(__MACH__)
#include "user_context_darwin.h"
#endif

#endif  // __AILEGO_DEBUG_USER_CONTEXT_H__
