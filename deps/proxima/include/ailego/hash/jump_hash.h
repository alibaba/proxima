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
 *   \date     Dec 2017
 *   \brief    Interface of Jump Consistent Hash Algorithm
 */

#ifndef __AILEGO_HASH_JUMP_HASH_H__
#define __AILEGO_HASH_JUMP_HASH_H__

#include <ailego/internal/platform.h>

namespace ailego {

//! Jump consistent hash algorithm (https://arxiv.org/pdf/1406.2294.pdf)
static inline int32_t JumpHash(uint64_t key, int32_t num_buckets) {
  int64_t b = 1, j = 0;
  while (j < num_buckets) {
    b = j;
    key = key * 2862933555777941757ULL + 1;
    j = (int64_t)(double(b + 1) *
                  (double(1LL << 31) / double((key >> 33) + 1)));
  }
  return (int32_t)b;
}

}  // namespace ailego

#endif  // __AILEGO_HASH_JUMP_HASH_H__
