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
 * \brief    Contains the specified collection info
 */

package com.alibaba.proxima.be.client;

/**
 * Contains the specified collection information
 */
public class DescribeCollectionResponse {
  private Status status;
  private CollectionInfo collectionInfo;

  /**
   * Constructor with status and collection info
   * @param status request success or failed
   * @param collectionInfo collection info
   */
  public DescribeCollectionResponse(Status status, CollectionInfo collectionInfo) {
    this.status = status;
    this.collectionInfo = collectionInfo;
  }

  /**
   * Constructor with ErrorCode
   * @param code error code
   */
  public DescribeCollectionResponse(Status.ErrorCode code) {
    this.status = new Status(code);
    this.collectionInfo = null;
  }

  /**
   * Constructor with error code and reason
   * @param code error code
   * @param reason error reason
   */
  public DescribeCollectionResponse(Status.ErrorCode code, String reason) {
    this.status = new Status(code, reason);
    this.collectionInfo = null;
  }

  public Status getStatus() {
    return status;
  }

  public CollectionInfo getCollectionInfo() {
    return collectionInfo;
  }

  /**
   * Is response success
   * @return true means success.
   */
  public boolean ok() {
    return this.status.ok();
  }
}
