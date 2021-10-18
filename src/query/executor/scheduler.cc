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

#include "scheduler.h"
#include <atomic>
#include <thread>
#include "common/error_code.h"
#include "common/logger.h"
#include "bthread_queue.h"

namespace proxima {
namespace be {
namespace query {

//! Alias for TaskQueueVector
using TaskQueueVector = std::vector<TaskQueuePtr>;

/*!
 * Selector
 */
struct Selector {
  //! Pick a number between [0:limit]
  static uint32_t pick(uint32_t /* limit */) {
    return 0;
  }
};

//! Round Robin Selector
struct RoundRobinSelector {
  //! Pick a number between [0:limit]
  uint32_t pick(uint32_t limit) {
    return count++ % limit;
  }

 private:
  std::atomic<uint64_t> count{0};
};

template <typename Selector>
class SchedulerImpl : public Scheduler {
 public:
  //! Constructor
  SchedulerImpl() : concurrency_(0) {}

  //! Destructor
  ~SchedulerImpl() override = default;

 public:
  //! Dispatch task to execution queue
  int schedule(TaskPtr task) override {
    if (!queues_.empty()) {
      uint32_t pos = selector_.pick(concurrency_);
      LOG_DEBUG("Selector return[%u], task[%s]", pos, task->name().c_str());
      return queues_[pos]->put(task);
    }
    return PROXIMA_BE_ERROR_CODE(UnreadyQueue);
  }

  //! Retrieve concurrency field
  uint32_t concurrency() const override {
    return concurrency_;
  }

  //! Set concurrency field
  uint32_t concurrency(uint32_t concurrent) override {
    recycle_queue();
    concurrency_ = resize(concurrent);
    return concurrency_;
  }

 private:
  //! Resize queue buckets
  uint32_t resize(uint32_t size) {
    while (size--) {
      TaskQueuePtr queue = TaskQueuePtr(new BThreadQueue());
      if (queue->start() == 0) {
        LOG_DEBUG("Success to start task execution queue");
        queues_.emplace_back(queue);
      }
    }
    return static_cast<uint32_t>(queues_.size());
  }

  void recycle_queue() {
    for (auto &queue : queues_) {
      queue->stop();
      queue->join();
    }
    queues_.clear();
  }

 private:
  //! Concurrency
  uint32_t concurrency_{0};
  //! Execution queue buckets
  TaskQueueVector queues_{};
  //! Selector, Load balance role
  Selector selector_{};
};

//! Retrieve default scheduler reference
SchedulerPtr Scheduler::Default() {
  static SchedulerPtr kScheduler =
      SchedulerPtr(new SchedulerImpl<RoundRobinSelector>());
  return kScheduler;
}

//! Retrieve hardware concurrency
uint32_t Scheduler::HostConcurrency() {
  return std::thread::hardware_concurrency();
}

}  // namespace query
}  // namespace be
}  // namespace proxima
