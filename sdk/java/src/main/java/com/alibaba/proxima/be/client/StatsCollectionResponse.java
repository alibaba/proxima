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
 * \brief    StatsCollectionResponse contains the statistic info for collection
 */

package com.alibaba.proxima.be.client;

/**
 * Contains statistic information for collection
 */
public class StatsCollectionResponse {
  private Status status;
  private CollectionStats collectionStats;

  /**
   * Constructor with statuss and collection stats
   * @param status status
   * @param collectionStats collection stats
   */
  public StatsCollectionResponse(Status status, CollectionStats collectionStats) {
    this.status = status;
    this.collectionStats = collectionStats;
  }

  /**
   * Constructor with error code
   * @param code error code
   */
  public StatsCollectionResponse(Status.ErrorCode code) {
    this.status = new Status(code);
    this.collectionStats = null;
  }

  /**
   * Constructor with code and reason
   * @param code error code
   * @param reason error message
   */
  public StatsCollectionResponse(Status.ErrorCode code, String reason) {
    this.status = new Status(code, reason);
    this.collectionStats = null;
  }

  public Status getStatus() {
    return status;
  }

  public CollectionStats getCollectionStats() {
    return collectionStats;
  }

  /**
   * Is request success, true means success
   * @return boolean
   */
  public boolean ok() {
    return this.status.ok();
  }
}
