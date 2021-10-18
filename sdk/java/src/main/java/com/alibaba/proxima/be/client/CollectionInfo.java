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
 * \brief    CollectionInfo contains information for collection
 */

package com.alibaba.proxima.be.client;

/**
 * Contains information for collection
 */
public class CollectionInfo {
  private final CollectionConfig collectionConfig;
  private final CollectionStatus collectionStatus;
  private final String uuid;
  private final LsnContext latestLsnContext;
  private final long magicNumber;

  private CollectionInfo(Builder builder) {
    this.collectionConfig = builder.collectionConfig;
    this.collectionStatus = builder.collectionStatus;
    this.uuid = builder.uuid;
    this.latestLsnContext = builder.latestLsnContext;
    this.magicNumber = builder.magicNumber;
  }

  public CollectionConfig getCollectionConfig() {
    return collectionConfig;
  }

  public CollectionStatus getCollectionStatus() {
    return collectionStatus;
  }

  public String getUuid() {
    return uuid;
  }

  public LsnContext getLatestLsnContext() {
    return latestLsnContext;
  }

  public long getMagicNumber() {
    return magicNumber;
  }

  // New CollectionInfo builder
  public static Builder newBuilder() {
    return new Builder();
  }

  /**
   * Builder for CollectionInfo
   */
  public static class Builder {
    // required parameters
    private CollectionConfig collectionConfig;
    private CollectionStatus collectionStatus;
    private String uuid;

    // optional parameters
    private LsnContext latestLsnContext = null;
    private long magicNumber = 0;

    /**
     * Constructor without parameters
     */
    public Builder() {
    }

    /**
     * Set collection config
     * @param collectionConfig collection config
     * @return Builder
     */
    public Builder withCollectionConfig(CollectionConfig collectionConfig) {
      this.collectionConfig = collectionConfig;
      return this;
    }

    /**
     * Sset collection status
     * @param collectionStatus collection status
     * @return Builder
     */
    public Builder withCollectionStatus(CollectionStatus collectionStatus) {
      this.collectionStatus = collectionStatus;
      return this;
    }

    /**
     * Set uuid
     * @param uuid unique user id
     * @return Builder
     */
    public Builder withUuid(String uuid) {
      this.uuid = uuid;
      return this;
    }

    /**
     * Set latest lsn context
     * @param latestLsnContext latest lsn context, only use with mysql repository
     * @return Builder
     */
    public Builder withLatestLsnContext(LsnContext latestLsnContext) {
      this.latestLsnContext = latestLsnContext;
      return this;
    }

    /**
     * Set magic number
     * @param magicNumber magic number from server
     * @return Builder
     */
    public Builder withMagicNumber(long magicNumber) {
      this.magicNumber = magicNumber;
      return this;
    }

    /**
     * Build CollectionInfo object
     * @return CollectionInfo
     */
    public CollectionInfo build() {
      return new CollectionInfo(this);
    }
  }

  /**
   * Collection running status
   */
  public enum CollectionStatus {
    /**
     * Collection initialized
     */
    INITIALIZED(0),
    /**
     * Collection serving
     */
    SERVING(1),
    /**
     * Collection dropped
     */
    DROPPED(2),
    /**
     * Unknown status
     */
    UNKNOWN(-1);

    private int value;

    CollectionStatus(int value) {
      this.value = value;
    }

    public int getValue() {
      return this.value;
    }

    public static CollectionStatus valueOf(int value) {
      switch (value) {
        case 0:
          return INITIALIZED;
        case 1:
          return SERVING;
        case 2:
          return DROPPED;
        default:
          return UNKNOWN;
      }
    }
  }
}
