/**
 * Copyright 2021 Alibaba, Inc. and its affiliates. All Rights Reserved.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 
 * <p>
 * \author   Hongqing.hu
 * \date     Mar 2021
 * \brief    QueryResponse contains the query documents and status
 */

package com.alibaba.proxima.be.client;

import java.util.List;

/**
 * Contains query documents and status
 */
public class QueryResponse {
  private Status status;
  private long latencyUs;
  private String debugInfo;
  private List<QueryResult> queryResults;

  public QueryResponse(Status status, long latencyMs,
                       String debugInfo, List<QueryResult> queryResults) {
    this.status = status;
    this.latencyUs = latencyMs;
    this.debugInfo = debugInfo;
    this.queryResults = queryResults;
  }

  public QueryResponse(Status.ErrorCode code) {
    this.status = new Status(code);
    this.latencyUs = 0;
    this.debugInfo = null;
    this.queryResults = null;
  }

  public QueryResponse(Status.ErrorCode code, String reason) {
    this.status = new Status(code, reason);
    this.latencyUs = 0;
    this.debugInfo = null;
    this.queryResults = null;
  }

  public Status getStatus() {
    return status;
  }

  public long getLatencyUs() {
    return latencyUs;
  }

  public String getDebugInfo() {
    return debugInfo;
  }

  public List<QueryResult> getQueryResults() {
    return queryResults;
  }

  /**
   * Get query result batch count
   * @return int
   */
  public int getQueryResultCount() {
    if (this.queryResults != null) {
      return this.queryResults.size();
    }
    return 0;
  }

  /**
   * Get specified query result
   * @param index query result index
   * @return QueryResult
   */
  public QueryResult getQueryResult(int index) {
    return this.queryResults.get(index);
  }

  /**
   * Is query request success
   * @return boolean
   */
  public boolean ok() {
    return this.status.ok();
  }
}
