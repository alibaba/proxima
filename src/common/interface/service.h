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
 *   \brief    Abstract class of service which includes
 *             init->start->stop->cleanup
 */

#pragma once

#include <memory>
#include "common/error_code.h"
#include "common/logger.h"
#include "common/macro_define.h"

namespace proxima {
namespace be {

class Service {
 public:
  //! Destructor
  virtual ~Service() = default;

 public:
  int init() {
    if (status_ != CREATED) {
      LOG_ERROR("Service status error. status[%d] expect[%d]", status_,
                CREATED);
      return ErrorCode_StatusError;
    }

    int ret = this->init_impl();
    if (ret == 0) {
      status_ = INITIALIZED;
    }

    return ret;
  }

  int cleanup() {
    if (status_ != INITIALIZED) {
      LOG_ERROR("Service status error. status[%d] expect[%d]", status_,
                INITIALIZED);
      return ErrorCode_StatusError;
    }

    int ret = this->cleanup_impl();
    if (ret == 0) {
      status_ = CREATED;
    }
    return ret;
  }

  int start() {
    if (status_ != INITIALIZED) {
      LOG_ERROR("Service status error. status[%d] expect[%d]", status_,
                INITIALIZED);
      return ErrorCode_StatusError;
    }

    int ret = this->start_impl();
    if (ret == 0) {
      status_ = STARTED;
    }

    return ret;
  }

  int stop() {
    if (status_ != STARTED) {
      LOG_ERROR("Service status error. status[%d] expect[%d]", status_,
                STARTED);
      return ErrorCode_StatusError;
    }

    int ret = this->stop_impl();
    if (ret == 0) {
      status_ = INITIALIZED;
    }

    return ret;
  }

  int status() const {
    return status_;
  }

 protected:
  virtual int init_impl() = 0;

  virtual int cleanup_impl() = 0;

  virtual int start_impl() = 0;

  virtual int stop_impl() = 0;

 protected:
  enum Status { CREATED = 0, INITIALIZED = 1, STARTED = 2 };

  Status status_{CREATED};
};

using ServicePtr = std::shared_ptr<Service>;

}  // namespace be
}  // end namespace proxima
