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
 *   \date     Jan 2021
 *   \brief    Interface of AiLego Utility File Lock
 */

#ifndef __AILEGO_IO_FILE_LOCK_H__
#define __AILEGO_IO_FILE_LOCK_H__

#include "file.h"

namespace ailego {

/*! File Utility
 */
class FileLock {
 public:
  //! Constructor
  FileLock(const File &file) : native_handle_(file.native_handle()) {}

  //! Constructor
  FileLock(File::NativeHandle handle) : native_handle_(handle) {}

  //! Locking
  bool lock(void) const {
    return FileLock::Lock(native_handle_);
  }

  //! Try locking
  bool try_lock(void) const {
    return FileLock::TryLock(native_handle_);
  }

  //! Locking (shared)
  bool lock_shared(void) const {
    return FileLock::LockShared(native_handle_);
  }

  //! Try locking (shared)
  bool try_lock_shared(void) const {
    return FileLock::TryLockShared(native_handle_);
  }

  //! Unlocking
  bool unlock(void) const {
    return FileLock::Unlock(native_handle_);
  }

  //! Locking
  static bool Lock(File::NativeHandle handle);

  //! Try locking
  static bool TryLock(File::NativeHandle handle);

  //! Locking (shared)
  static bool LockShared(File::NativeHandle handle);

  //! Try locking (shared)
  static bool TryLockShared(File::NativeHandle handle);

  //! Unlocking
  static bool Unlock(File::NativeHandle handle);

 private:
  //! Disable them
  FileLock(const FileLock &) = delete;
  FileLock(FileLock &&) = delete;
  FileLock &operator=(const FileLock &) = delete;
  FileLock &operator=(FileLock &&) = delete;

  //! Members
  File::NativeHandle native_handle_;
};

}  // namespace ailego

#endif  // __AILEGO_IO_FILE_LOCK_H__
