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
 *   \brief    Interface of Bug Report
 */

#ifndef __AILEGO_DEBUG_BUG_REPORT_H__
#define __AILEGO_DEBUG_BUG_REPORT_H__

#include <map>
#include <mutex>
#include <string>
#include <vector>
#include <ailego/parallel/lock.h>
#include <ailego/pattern/singleton.h>
#include "symbol_table.h"

namespace ailego {

/*! Bug Report (Singleton)
 */
class BugReport : public Singleton<BugReport> {
 public:
  typedef std::map<uint32_t, std::vector<void *>> CallStack;

  //! Constructor
  BugReport(void) {}

  //! Destructor
  ~BugReport(void) {}

  //! Bootstrap the bug report
  void bootstrap(int argc, char *argv[], const char *dir);

  //! Backtrace the current thread
  void backtrace(void);

  //! Retrieve timestamp
  uint64_t timestamp(void) const {
    return timestamp_;
  }

  //! Retrieve application command
  const std::string &command(void) const {
    return command_;
  }

  //! Retrieve application arguments
  const std::string &arguments(void) const {
    return arguments_;
  }

  //! Retrieve log directory
  const std::string &logdir(void) const {
    return logdir_;
  }

  //! Retrieve callstack
  CallStack callstack(void) const {
    ReadLock rdlock(callstack_mutex_);
    std::lock_guard<ReadLock> latch(rdlock);
    return callstack_;
  }

  //! Retrieve symbol table
  SymbolTable *mutable_symbols(void) {
    return &symbols_;
  }

  //! Lock the bug report
  void lock(void) {
    mutex_.lock();
  }

  //! Bootstrap the bug report
  static void Bootstrap(int argc, char *argv[], const char *dir) {
    Instance().bootstrap(argc, argv, dir);
  }

 private:
  uint64_t timestamp_{0};
  std::string command_{};
  std::string arguments_{};
  std::string logdir_{};
  SymbolTable symbols_{};
  CallStack callstack_{};
  mutable SharedMutex callstack_mutex_{};
  mutable std::mutex mutex_{};
};

}  // namespace ailego

#endif  // __AILEGO_DEBUG_BUG_REPORT_H__
