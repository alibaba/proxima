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
 *   \date     Nov 2020
 *   \brief
 */

#include "bthread_queue.h"
#include <cstring>
#include "common/error_code.h"
#include "common/logger.h"

namespace proxima {
namespace be {
namespace query {

BThreadQueue::BThreadQueue() : status_(TaskQueue::Status::INITIALIZED) {}

BThreadQueue::~BThreadQueue() = default;

namespace {

//! Runner routine with execution_queue_start, first arguments not used
static int Execute(void *meta, bthread::TaskIterator<TaskPtr> &iter) {
  auto queue = static_cast<BThreadQueue *>(meta);
  if (iter.is_queue_stopped()) {
    LOG_INFO("Bthread Queue has been stopped.");
    return 0;
  }
  auto queue_id = iter ? queue->id() : 0;
  LOG_DEBUG("Bthread Queue [%zu] start run tasks", (size_t)queue_id);
  // invoke task run interface
  for (; iter; ++iter) {
    LOG_DEBUG("Task [%s] is ready to run on queue[%zu]",
              (*iter)->name().c_str(), (size_t)queue->id());
    (*iter)->run();
    LOG_DEBUG("Task [%s] has been finished on queue[%zu]",
              (*iter)->name().c_str(), (size_t)queue->id());
  }
  LOG_DEBUG("Bthread Queue [%zu] finished to run tasks", (size_t)queue_id);
  return 0;
}

}  // namespace

uint64_t BThreadQueue::id() const {
  return queue_id_.value;
}

int BThreadQueue::start() {
  if (status_ == Status::INITIALIZED) {
    if (bthread::execution_queue_start(&queue_id_, &options_, Execute, this) ==
        0) {
      status_ = Status::STARTED;
      LOG_DEBUG("BThreadQueue success to start, queue_id_[%zu]",
                (size_t)queue_id_.value);
      return 0;
    }
    LOG_ERROR("Failed to start BThreadQueue, queue_id_[%zu]",
              (size_t)queue_id_.value);
    clear_context();
  } else {
    LOG_ERROR("Failed to start BThreadQueue, which has not been initialized");
  }
  return PROXIMA_BE_ERROR_CODE(RuntimeError);
}

int BThreadQueue::stop() {
  if (status_ != Status::STARTED) {
    LOG_DEBUG("Can't stop an queue, which does not started yet. queue_id_[%zu]",
              (size_t)queue_id_.value);
    return PROXIMA_BE_ERROR_CODE(RuntimeError);
  }
  int code = bthread::execution_queue_stop(queue_id_);
  if (code == 0) {
    LOG_INFO("BThreadQueue stopped, queue_id_[%zu]", (size_t)queue_id_.value);
    status_ = Status::STOPPED;
    return 0;
  }

  LOG_ERROR("Failed to stop Bthread, ret code[%d]", code);
  return PROXIMA_BE_ERROR_CODE(RuntimeError);
}

int BThreadQueue::join() {
  if (status_ != Status::STOPPED) {
    LOG_ERROR("Can't join an queue, which did not stop yet, queue_id_[%zu]",
              (size_t)queue_id_.value);
    return PROXIMA_BE_ERROR_CODE(RuntimeError);
  }
  int code = bthread::execution_queue_join(queue_id_);
  if (code == 0) {
    status_ = Status::JOINED;
    return 0;
  }
  LOG_ERROR("Bthread join failed, ret code[%d]", code);
  return PROXIMA_BE_ERROR_CODE(RuntimeError);
}

int BThreadQueue::put(const TaskPtr &task) {
  if (task && started()) {
    task->status(Task::Status::SCHEDULED);
    if (bthread::execution_queue_execute(queue_id_, task) == 0) {
      LOG_DEBUG("Scheduled task[%s]", task->name().c_str());
      return 0;
    } else {
      task->status(Task::Status::INITIALIZED);
      LOG_ERROR("Scheduled task[%s] failed", task->name().c_str());
    }
  }
  LOG_ERROR("Failed to schedule task");
  return PROXIMA_BE_ERROR_CODE(RuntimeError);
}

bool BThreadQueue::started() const {
  return status_ == Status::STARTED;
}

void BThreadQueue::clear_context() {
  status_ = Status::INITIALIZED;
  std::memset(static_cast<void *>(&queue_id_), 0, sizeof(queue_id_));
  std::memset(static_cast<void *>(&options_), 0, sizeof(options_));
}

}  // namespace query
}  // namespace be
}  // namespace proxima
