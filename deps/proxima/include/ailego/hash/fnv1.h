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
 *   \date     Aug 2019
 *   \brief    Interface of FNV-1 and FNV-1a Hash
 */

#ifndef __AILEGO_HASH_FNV1_H__
#define __AILEGO_HASH_FNV1_H__

#include <ailego/internal/platform.h>

namespace ailego {

/*! FNV-1 Hash Helper (compile time)
 */
template <size_t N>
struct Fnv1Helper {
  //! Compute FNV-1 32bit hash
  static inline constexpr uint32_t Hash32(const char *str) {
    return (Fnv1Helper<N - 1>::Hash32(str) * 0x1000193U) ^ (uint32_t)str[N - 1];
  }

  //! Compute FNV-1 64bit hash
  static inline constexpr uint64_t Hash64(const char *str) {
    return (Fnv1Helper<N - 1>::Hash64(str) * 0x100000001B3LLU) ^
           (uint64_t)str[N - 1];
  }
};

/*! FNV-1 Hash Helper (compile time)
 */
template <>
struct Fnv1Helper<0> {
  //! Compute FNV-1 32bit hash
  static inline constexpr uint32_t Hash32(const char *) {
    return 0x811c9dc5U;
  }

  //! Compute FNV-1 64bit hash
  static inline constexpr uint64_t Hash64(const char *) {
    return 0xcbf29ce484222325LLU;
  }
};

/*! FNV-1a Hash Helper (compile time)
 */
template <size_t N>
struct Fnv1aHelper {
  //! Compute FNV-1 32bit hash
  static inline constexpr uint32_t Hash32(const char *str) {
    return (Fnv1aHelper<N - 1>::Hash32(str) ^ (uint32_t)str[N - 1]) *
           0x1000193U;
  }

  //! Compute FNV-1 64bit hash
  static inline constexpr uint64_t Hash64(const char *str) {
    return (Fnv1aHelper<N - 1>::Hash64(str) ^ (uint64_t)str[N - 1]) *
           0x100000001B3LLU;
  }
};

/*! FNV-1a Hash Helper (compile time)
 */
template <>
struct Fnv1aHelper<0> {
  //! Compute FNV-1 32bit hash
  static inline constexpr uint32_t Hash32(const char *) {
    return 0x811c9dc5U;
  }

  //! Compute FNV-1 64bit hash
  static inline constexpr uint64_t Hash64(const char *) {
    return 0xcbf29ce484222325LLU;
  }
};

/*! FNV-1 Hash
 */
struct Fnv1 {
  //! Compute FNV-1 32bit hash in compile time
  template <size_t N>
  static inline constexpr uint32_t Hash32(const char (&str)[N]) {
    return Fnv1Helper<N>::Hash32(str);
  }

  //! Compute FNV-1 64bit hash in compile time
  template <size_t N>
  static inline constexpr uint64_t Hash64(const char (&str)[N]) {
    return Fnv1Helper<N>::Hash64(str);
  }

  //! Compute FNV-1 32bit hash for the source data buffer
  static inline uint32_t Hash32(const void *data, size_t len, uint32_t sum) {
    const uint8_t *iter = (const uint8_t *)data;
    const uint8_t *end = (const uint8_t *)data + len;

    while (iter < end) {
      // Multiply by the 32 bit FNV magic prime mod 2^32
      sum += (sum << 1) + (sum << 4) + (sum << 7) + (sum << 8) + (sum << 24);
      // Xor the bottom with the current octet
      sum ^= (uint32_t)*iter++;
    }
    return sum;
  }

  //! Compute FNV-1 32bit hash for the source data buffer
  static inline uint32_t Hash32(const void *data, size_t len) {
    return Hash32(data, len, 0x811c9dc5U);
  }

  //! Compute FNV-1 64bit hash for the source data buffer
  static inline uint64_t Hash64(const void *data, size_t len, uint64_t sum) {
    const uint8_t *iter = (const uint8_t *)data;
    const uint8_t *end = (const uint8_t *)data + len;

    while (iter < end) {
      // Multiply by the 64 bit FNV magic prime mod 2^64
      sum += (sum << 1) + (sum << 4) + (sum << 5) + (sum << 7) + (sum << 8) +
             (sum << 40);
      // Xor the bottom with the current octet
      sum ^= (uint64_t)*iter++;
    }
    return sum;
  }

  //! Compute FNV-1 64bit hash for the source data buffer
  static inline uint64_t Hash64(const void *data, size_t len) {
    return Hash64(data, len, 0xcbf29ce484222325LLU);
  }
};

/*! FNV-1a Hash
 */
struct Fnv1a {
  //! Compute FNV-1a 32bit hash in compile time
  template <size_t N>
  static inline constexpr uint32_t Hash32(const char (&str)[N]) {
    return Fnv1aHelper<N>::Hash32(str);
  }

  //! Compute FNV-1a 64bit hash in compile time
  template <size_t N>
  static inline constexpr uint64_t Hash64(const char (&str)[N]) {
    return Fnv1aHelper<N>::Hash64(str);
  }

  //! Compute FNV-1a 32bit hash for the source data buffer
  static inline uint32_t Hash32(const void *data, size_t len, uint32_t sum) {
    const uint8_t *iter = (const uint8_t *)data;
    const uint8_t *end = (const uint8_t *)data + len;

    while (iter < end) {
      // Xor the bottom with the current octet
      sum ^= (uint32_t)*iter++;
      // Multiply by the 32 bit FNV magic prime mod 2^32
      sum += (sum << 1) + (sum << 4) + (sum << 7) + (sum << 8) + (sum << 24);
    }
    return sum;
  }

  //! Compute FNV-1a 32bit hash for the source data buffer
  static inline uint32_t Hash32(const void *data, size_t len) {
    return Hash32(data, len, 0x811c9dc5U);
  }

  //! Compute FNV-1a 64bit hash for the source data buffer
  static inline uint64_t Hash64(const void *data, size_t len, uint64_t sum) {
    const uint8_t *iter = (const uint8_t *)data;
    const uint8_t *end = (const uint8_t *)data + len;

    while (iter < end) {
      // Xor the bottom with the current octet
      sum ^= (uint64_t)*iter++;
      // Multiply by the 64 bit FNV magic prime mod 2^64
      sum += (sum << 1) + (sum << 4) + (sum << 5) + (sum << 7) + (sum << 8) +
             (sum << 40);
    }
    return sum;
  }

  //! Compute FNV-1a 64bit hash for the source data buffer
  static inline uint64_t Hash64(const void *data, size_t len) {
    return Hash64(data, len, 0xcbf29ce484222325LLU);
  }
};

}  // namespace ailego

#endif  // __AILEGO_HASH_FNV1_H__
