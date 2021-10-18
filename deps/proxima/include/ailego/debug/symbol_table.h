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
 *   \brief    Interface of Bugreport Symbol Table
 */

#ifndef __AILEGO_DEBUG_SYMBOL_TABLE_H__
#define __AILEGO_DEBUG_SYMBOL_TABLE_H__

#include <map>
#include <string>
#include <vector>

namespace ailego {

/*! Symbol Information (same with Dl_info)
 */
struct SymbolInfo {
  const char *path;  // Pathname of shared object that contains address
  void *base;        // Base address at which shared object is loaded
  const char *name;  // Name of symbol whose definition overlaps addr
  void *address;     // Exact address of symbol
};

/*! Symbol Meta
 */
struct SymbolMeta {
  void *address;    // Exact address of symbol
  size_t size;      // Size of symbol
  size_t position;  // Position of symbol name

  //! Constructor
  SymbolMeta(void *addr, size_t len, size_t index)
      : address(addr), size(len), position(index) {}
};

/*! Symbol Table
 */
class SymbolTable {
 public:
  //! Load symbols into a table from a binary file
  bool load(const std::string &path, void *base);

  //! Clear the symbol table
  void clear(void);

  //! Fetch symbol information via address
  bool fetch(const std::string &path, void *base, void *addr, SymbolInfo *out);

  //! Fetch symbol information via address
  bool fetch(void *addr, SymbolInfo *out);

 private:
  //! Members
  std::map<std::string, std::pair<std::vector<SymbolMeta>, std::string>> map_{};
};

}  // namespace ailego

#endif  // __AILEGO_DEBUG_SYMBOL_TABLE_H__
