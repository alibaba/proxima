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

 *   \author   hongqing.hu
 *   \date     Nov 2020
 *   \brief    Interface of Rate Limiter algorithm
 */

#ifndef __AILEGO_ALGORITHM_RATE_LIMITER_H__
#define __AILEGO_ALGORITHM_RATE_LIMITER_H__

#include <chrono>
#include <memory>
#include <mutex>

namespace ailego {

/*! RateLimiter abstract class
 */
struct RateLimiter {
  //! Rate Limiter Shared Pointer
  using Pointer = std::shared_ptr<RateLimiter>;

  //! RateLimiter Types
  enum LimiterTypes {
    LIMITER_BURSTY = 0,
  };

  //! Destructor
  virtual ~RateLimiter(void) {}

  //! Acquire batch permits
  virtual double acquire(int permits) = 0;

  //! Acquire one permit
  virtual double acquire(void) = 0;

  //! Try acquire some permits within timeout ms
  virtual bool try_acquire(int permits, int timeout_ms) = 0;

  //! Try acquire one permit, return at once
  virtual bool try_acquire(void) = 0;

  //! Set permits per second
  virtual void set_rate(double permits_per_second) = 0;

  //! Get permits per second
  virtual double get_rate(void) const = 0;

  //! Create a rate limiter with type
  static Pointer Create(double permits_per_second, LimiterTypes type);

  //! Create a default rate limiter
  static Pointer Create(double permits_per_second) {
    return RateLimiter::Create(permits_per_second, LIMITER_BURSTY);
  }
};

/*! Bursty Rate Limiter
 */
class BurstyRateLimiter : public RateLimiter {
 public:
  //! Constructor
  explicit BurstyRateLimiter(double max_bursty_seconds)
      : max_bursty_seconds_(max_bursty_seconds) {}

  //! Destructor
  virtual ~BurstyRateLimiter(void) {}

  //! Acquire batch permits
  double acquire(int permits) override;

  //! Try acquire some permits with timeout
  bool try_acquire(int permits, int timeout_ms) override;

  //! Set permits per second
  void set_rate(double permits_per_second) override;

  //! Get permits per second
  double get_rate(void) const override {
    return MICROSECONDS_PER_SECOND / interval_;
  }

  //! Acquire one permit
  double acquire(void) override {
    return this->acquire(1);
  }

  //! Try acquire one permit, return at once
  bool try_acquire(void) override {
    return this->try_acquire(1, 0);
  }

 protected:
  //! Resync the information
  void update_stored_permits(int64_t now_usec);

  //! Compute the wait time
  int64_t compute_wait_usec(int required_permits, int64_t now_usec);

 private:
  //! Disable them
  BurstyRateLimiter(const BurstyRateLimiter &) = delete;
  BurstyRateLimiter(BurstyRateLimiter &&) = delete;
  BurstyRateLimiter &operator=(const BurstyRateLimiter &) = delete;

  //! Members
  double max_bursty_seconds_{0.0};
  double max_permits_{0.0};
  double stored_permits_{0.0};
  double interval_{0.0};
  double permits_per_usec_{0.0};
  double next_free_time_{0.0};
  std::mutex mutex_{};

  constexpr static double MICROSECONDS_PER_SECOND = 1000000.0;
};

}  // namespace ailego

#endif  // __AILEGO_ALGORITHM_RATE_LIMITER_H__
