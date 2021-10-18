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

 *   \author   jiliang.ljl
 *   \date     Feb 2021
 *   \brief    implementation of metrics collector
 */

#include "metrics/metrics_collector.h"
#include "common/logger.h"

namespace proxima {
namespace be {
namespace metrics {

std::string MetricsCollector::metrics_name_;
static MetricsCollector &CreateMetrics(const std::string &name) {
  if (name.empty()) {
    static MetricsCollector empty_metrics;
    return empty_metrics;
  }
  static auto obj = ailego::Factory<MetricsCollector>::MakeShared(name.c_str());
  if (obj) {
    LOG_INFO("Create Metrics with name:%s", name.c_str());
    return *obj;
  } else {
    std::string registered;
    for (const auto &s : ailego::Factory<MetricsCollector>::Classes()) {
      registered += s;
      registered += ", ";
    }
    LOG_FATAL("Cannot create Metrics with name=%s, registered names=%s",
              name.c_str(), registered.c_str());
    // In case LOG(FATAL) only print log and do not abort process
    static MetricsCollector empty_metrics;
    return empty_metrics;
  }
}

MetricsCollector &MetricsCollector::GetInstance() {
  static MetricsCollector &obj = CreateMetrics(metrics_name_);
  return obj;
}

int MetricsCollector::CreateAndInitMetrics(
    const proxima::be::proto::MetricsConfig &config) {
  metrics_name_ = config.name();
  auto &obj = GetInstance();
  int ret = 0;
  ret = obj.init(config);
  if (ret != 0) {
    LOG_FATAL("init metrics failed, config=%s", config.DebugString().c_str());
    return ret;
  }
  return ret;
}

METRICS_REGISTER(default, MetricsCollector);

}  // namespace metrics
}  // namespace be
}  // namespace proxima
