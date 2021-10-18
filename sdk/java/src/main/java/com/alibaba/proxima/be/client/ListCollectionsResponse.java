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
 * \brief    ListCollectionsResponse contains collections info and status
 */

package com.alibaba.proxima.be.client;

import java.util.List;

/**
 * Contains collections information
 */
public class ListCollectionsResponse {
  private Status status;
  private List<CollectionInfo> collections;

  // Constructor

  /**
   * Constructor with status and collections
   * @param status status
   * @param collections collection list
   */
  public ListCollectionsResponse(Status status, List<CollectionInfo> collections) {
    this.status = status;
    this.collections = collections;
  }

  /**
   * Constructor with error code
   * @param code error code
   */
  public ListCollectionsResponse(Status.ErrorCode code) {
    this.status = new Status(code);
    this.collections = null;
  }

  /**
   * Constructor with code and reason
   * @param code error code
   * @param reason error message
   */
  public ListCollectionsResponse(Status.ErrorCode code, String reason) {
    this.status = new Status(code, reason);
    this.collections = null;
  }

  /**
   * Get collection count
   * @return int
   */
  public int getCollectionCount() {
    if (this.collections != null) {
      return this.collections.size();
    }
    return 0;
  }

  /**
   * Get specified collection info
   * @param index the collection index
   * @return CollectionInfo
   */
  public CollectionInfo getCollection(int index) {
    return this.collections.get(index);
  }

  /**
   * Get status
   * @return Status
   */
  public Status getStatus() {
    return this.status;
  }

  /**
   * Is request success, true means success
   * @return boolean
   */
  public boolean ok() {
    return this.status.ok();
  }
}
