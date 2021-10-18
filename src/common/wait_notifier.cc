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
 *   \brief    Implementation of wait notifier
 */

#include "wait_notifier.h"

namespace proxima {
namespace be {

void WaitNotifier::wait() {
  std::unique_lock<std::mutex> lck(mutex_);
  if (!notified_) {
    cv_.wait(lck);
  }
  notified_ = false;
}

void WaitNotifier::wait_until(
    const std::chrono::system_clock::time_point &tm_point) {
  std::unique_lock<std::mutex> lck(mutex_);
  if (!notified_) {
    cv_.wait_until(lck, tm_point);
  }
  notified_ = false;
}

void WaitNotifier::wait_for(
    const std::chrono::system_clock::duration &tm_duration) {
  std::unique_lock<std::mutex> lck(mutex_);
  if (!notified_) {
    cv_.wait_for(lck, tm_duration);
  }
  notified_ = false;
}

void WaitNotifier::notify() {
  std::unique_lock<std::mutex> lck(mutex_);
  notified_ = true;
  lck.unlock();
  cv_.notify_one();
}

}  // namespace be
}  // end namespace proxima
