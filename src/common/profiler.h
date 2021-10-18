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
 *   \date     Dec 2020
 *   \brief
 */

#pragma once

#include <memory>
#include <vector>
#include <ailego/encoding/json.h>
#include <ailego/utility/time_helper.h>
#include "error_code.h"
#include "logger.h"

namespace proxima {
namespace be {

//! Predefined class
class Profiler;
//! Alias ProfilerPtr
using ProfilerPtr = std::shared_ptr<Profiler>;

//! Profiler collecting all the latency and other information during query
class Profiler {
 private:
  //! Stage object
  struct Stage {
    //! Constructor
    explicit Stage(ailego::JsonObject *node) : node_(node) {}
    //! Stage node, which stored in JsonTree held by Profiler
    ailego::JsonObject *node_{nullptr};
    //! Stage latency, started when creating Stage object
    ailego::ElapsedTime latency_;
  };

 public:
  //! Constructor
  explicit Profiler(bool enable = false) : enable_(enable) {
    if (enabled()) {
      root_.assign(ailego::JsonObject());
    }
  }

  //! Check enabled
  bool enabled() const {
    return enable_;
  }

  //! Start profiler
  void start() {
    if (enabled() && path_.empty()) {
      path_.emplace_back(Stage(&root_.as_object()));
    }
  }

  //! Stop profiler
  void stop() {
    if (enabled()) {
      if (path_.size() == 1) {
        // Root always held in path_[0]
        close_stage();
      } else {
        LOG_WARN("There are stages have not been closed, stages[%zu]",
                 path_.size());
        // Manually set latency to root, which should not be normal way
        root_["latency"] = path_.begin()->latency_.micro_seconds();
      }
    }
  }

  //! Open stage, start timer of stage
  int open_stage(const std::string &name) {
    if (enabled()) {
      if (path_.empty()) {
        LOG_ERROR("Profiler did not start yet");
        return PROXIMA_BE_ERROR_CODE(RuntimeError);
      }
      if (name.empty()) {
        LOG_ERROR("Can't open stage with empty name");
        return PROXIMA_BE_ERROR_CODE(RuntimeError);
      }
      ailego::JsonString key(name);
      ailego::JsonObject child;

      current_path()->set(key, child);  // add child
      path_.emplace_back(Stage(
          &((*current_path())[name.c_str()].as_object())));  // move to child
    }
    return 0;
  }

  //! Close stage and stop timer of stage(represent by stage.latency)
  int close_stage() {
    if (enabled()) {
      if (path_.empty()) {
        LOG_ERROR("No available stage can be closed");
        return PROXIMA_BE_ERROR_CODE(RuntimeError);
      }
      ailego::JsonValue latency(current()->latency_.micro_seconds());
      current_path()->set("latency", latency);
      path_.pop_back();
    }
    return 0;
  }

  //! add value to profiler
  template <typename VALUE_TYPE>
  int add(const std::string &name, const VALUE_TYPE &v) {
    if (enabled()) {
      if (path_.empty()) {
        return PROXIMA_BE_ERROR_CODE(RuntimeError);
      }

      ailego::JsonString key(name);
      ailego::JsonValue value(v);
      current_path()->set(key, value);
    }
    return 0;
  }

  //! Serialize profiler to string(Json Format)
  std::string as_json_string() const {
    return enabled() ? root_.as_json_string().as_stl_string()
                     : std::string("{}");
  }

 private:
  Stage *current() {
    return path_.rbegin().operator->();
  }

  ailego::JsonObject *current_path() {
    return current()->node_;
  }

 private:
  //! enable flag
  bool enable_{false};

  //! root handler
  ailego::JsonValue root_;

  //! Depth-First paths
  std::vector<Stage> path_;
};

//! Helper for latency
class ScopedLatency {
 public:
  //! Constructor
  explicit ScopedLatency(const char *name, ProfilerPtr profiler)
      : name_(name), profiler_(std::move(profiler)) {}

  //! Destructor
  ~ScopedLatency() {
    profiler_->add(name_, latency_.micro_seconds());
  }

 private:
  //! Name of latency
  const char *name_{nullptr};

  //! Timer handler
  ailego::ElapsedTime latency_;

  //! Profiler handler
  ProfilerPtr profiler_;
};

}  // namespace be
}  // namespace proxima
