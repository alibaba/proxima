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
 * \brief    Get single document by primary key
 */

package com.alibaba.proxima.be.client;

/**
 * Get document request
 */
public class GetDocumentRequest {
  private final String collectionName;
  private final long primaryKey;
  private boolean debugMode;

  private GetDocumentRequest(Builder builder) {
    this.collectionName = builder.collectionName;
    this.primaryKey = builder.primaryKey;
    this.debugMode = builder.debugMode;
  }

  public String getCollectionName() {
    return collectionName;
  }

  public long getPrimaryKey() {
    return primaryKey;
  }

  public boolean isDebugMode() {
    return debugMode;
  }

  /**
   * New GetDocumentRequest builder
   * @return Builder
   */
  public static Builder newBuilder() {
    return new Builder();
  }

  /**
   * Builder for GetDocumentRequest
   */
  public static class Builder {
    // required parameters
    private String collectionName;
    private long primaryKey;

    // optional parameters
    private boolean debugMode = false;

    /**
     * Empty constructor
     */
    public Builder() {
    }

    /**
     * Constructor with collection name and primary key
     * @param collectionName collection name
     * @param primaryKey primary key
     */
    public Builder(String collectionName, long primaryKey) {
      this.collectionName = collectionName;
      this.primaryKey = primaryKey;
    }

    /**
     * Set collection name
     * @param collectionName collection name
     * @return Builder
     */
    public Builder withCollectionName(String collectionName) {
      this.collectionName = collectionName;
      return this;
    }

    /**
     * Set primary key
     * @param primaryKey primary key to query
     * @return Builder
     */
    public Builder withPrimaryKey(long primaryKey) {
      this.primaryKey = primaryKey;
      return this;
    }

    /**
     * Set debug mode
     * @param debugMode is debug mode, true means debug
     * @return Builder
     */
    public Builder withDebugMode(boolean debugMode) {
      this.debugMode = debugMode;
      return this;
    }

    /**
     * Build get document request object
     * @return GetDocumentRequest
     */
    public GetDocumentRequest build() {
      return new GetDocumentRequest(this);
    }
  }
}
