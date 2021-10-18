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
 *
 *   \author   guonix
 *   \date     Jan 2021
 *   \brief
 */

#include "bthread_task.h"
#include "common/error_code.h"

namespace proxima {
namespace be {
namespace query {

BthreadTask::BthreadTask(std::string name_str)
    : name_(std::move(name_str)),
      status_(Task::Status::INITIALIZED),
      exit_code_(0) {
  bthread_mutex_init(&mutex_, nullptr);
  bthread_cond_init(&cond_, nullptr);
}

BthreadTask::~BthreadTask() {
  bthread_mutex_destroy(&mutex_);
  bthread_cond_destroy(&cond_);
}

const std::string &BthreadTask::name() const {
  return name_;
}

int BthreadTask::exit_code() const {
  return exit_code_;
}

//! Invoke do_run interface
int BthreadTask::run() {
  return run_once();
}

Task::Status BthreadTask::status() const {
  return status_.load();
}

//! Update task status, and notify listener if task have been finished
void BthreadTask::status(Task::Status stat) {
  if (stat == Status::FINISHED) {
    bthread_mutex_lock(&mutex_);
    status_.exchange(stat);
    bthread_cond_signal(&cond_);
    bthread_mutex_unlock(&mutex_);
  } else {
    status_.exchange(stat);
  }
}

bool BthreadTask::running() const {
  return status() == Task::Status::RUNNING;
}

bool BthreadTask::finished() const {
  return status() == Task::Status::FINISHED;
}

int BthreadTask::run_once() {
  Status stat = Status::SCHEDULED;
  if (status_.compare_exchange_strong(stat, Status::RUNNING)) {
    exit_code_ = do_run();
    status(Task::Status::FINISHED);
    return exit_code_;
  } else if (stat == Status::RUNNING) {
    return PROXIMA_BE_ERROR_CODE(TaskIsRunning);
  } else if (stat == Status::FINISHED) {
    return exit_code_;
  }
  return PROXIMA_BE_ERROR_CODE(RuntimeError);
}

//! Wait task until have been finished
bool BthreadTask::wait_finish() {
  bthread_mutex_lock(&mutex_);
  if (!finished()) {
    bthread_cond_wait(&cond_, &mutex_);
  }
  bthread_mutex_unlock(&mutex_);
  return true;
}

}  // namespace query
}  // namespace be
}  // namespace proxima
