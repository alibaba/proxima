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
 *   \brief    Interface of AiLego Process ID File
 */

#ifndef __AILEGO_IO_PID_FILE_H__
#define __AILEGO_IO_PID_FILE_H__

#include "file_lock.h"

namespace ailego {

/*! Process ID File
 */
class PidFile {
 public:
  //! Constructor
  PidFile(void) {}

  //! Constructor
  PidFile(PidFile &&rhs) : file_(std::move(rhs.file_)) {}

  //! Assignment
  PidFile &operator=(PidFile &&rhs) {
    file_ = std::move(rhs.file_);
    return *this;
  }

  //! Test if the pid file is valid
  bool is_valid(void) const {
    return file_.is_valid();
  }

  //! Open a local file
  bool open(const char *path);

  //! Open a local file
  bool open(const std::string &path) {
    return this->open(path.c_str());
  }

  //! Close the pid file
  void close(void);

 private:
  //! Disable them
  PidFile(const PidFile &) = delete;
  PidFile &operator=(const PidFile &) = delete;

  //! Members
  File file_{};
};

}  // namespace ailego

#endif  // __AILEGO_IO_PID_FILE_H__
