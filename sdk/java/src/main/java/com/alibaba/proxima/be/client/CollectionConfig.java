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
 * \brief    CollectionConfig contains params for collection
 */

package com.alibaba.proxima.be.client;

import java.util.*;

/**
 * Contains config for collection
 */
public class CollectionConfig {
  private final String collectionName;
  private final long maxDocsPerSegment;
  private final List<String> forwardColumnNames;
  private final List<IndexColumnParam> indexColumnParams;
  private final DatabaseRepository databaseRepository;

  private CollectionConfig(Builder builder) {
    this.collectionName = builder.collectionName;
    this.maxDocsPerSegment = builder.maxDocsPerSegment;
    this.forwardColumnNames = builder.forwardColumnNames;
    this.indexColumnParams = builder.indexColumnParams;
    this.databaseRepository = builder.databaseRepository;
  }

  public String getCollectionName() {
    return collectionName;
  }

  public long getMaxDocsPerSegment() {
    return maxDocsPerSegment;
  }

  public List<String> getForwardColumnNames() {
    return forwardColumnNames;
  }

  public List<IndexColumnParam> getIndexColumnParams() {
    return indexColumnParams;
  }

  public DatabaseRepository getDatabaseRepository() {
    return databaseRepository;
  }

  /**
   * New collection config builder object
   * @return Builder
   */
  public static Builder newBuilder() {
    return new Builder();
  }

  /**
   * Builder for CollectionConfig
   */
  public static class Builder {
    // Required parameters
    private String collectionName;
    private List<IndexColumnParam> indexColumnParams = new ArrayList<>();

    // Optional parameters
    private long maxDocsPerSegment = 0;
    private List<String> forwardColumnNames = new ArrayList<>();
    private DatabaseRepository databaseRepository = null;

    /**
     * Constructor without parameters
     */
    public Builder() {
    }

    /**
     * Constructor with collectionName and indexColumnParams
     * @param collectionName collection name
     * @param indexColumnParams index column parameters
     */
    public Builder(String collectionName, List<IndexColumnParam> indexColumnParams) {
      this.collectionName = collectionName;
      this.indexColumnParams = indexColumnParams;
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
     * Optional. Set forward column names
     * @param forwardColumnNames forward column name list
     * @return Builder
     */
    public Builder withForwardColumnNames(List<String> forwardColumnNames) {
      this.forwardColumnNames = forwardColumnNames;
      return this;
    }

    //

    /**
     * Optional. Set max docs per segment. Default 0
     * @param maxDocsPerSegment max docs per segment
     * @return Builder
     */
    public Builder withMaxDocsPerSegment(long maxDocsPerSegment) {
      this.maxDocsPerSegment = maxDocsPerSegment;
      return this;
    }

    // Set index column params list

    /**
     * Set index column parameters
     * @param indexColumnParams index column parameters
     * @return Buildder
     */
    public Builder withIndexColumnParams(List<IndexColumnParam> indexColumnParams) {
      this.indexColumnParams = indexColumnParams;
      return this;
    }

    /**
     * Optional. Set database repository
     * @param databaseRepository mysql database repository
     * @return Builder
     */
    public Builder withDatabaseRepository(DatabaseRepository databaseRepository) {
      this.databaseRepository = databaseRepository;
      return this;
    }

    /**
     * Add one forward column
     * @param forwardColumn forward column name
     * @return Builder
     */
    public Builder addForwardColumn(String forwardColumn) {
      this.forwardColumnNames.add(forwardColumn);
      return this;
    }

    /**
     * Add one column index param
     * @param indexParam index column parameters
     * @return Builder
     */
    public Builder addIndexColumnParam(IndexColumnParam indexParam) {
      this.indexColumnParams.add(indexParam);
      return this;
    }

    /**
     * Add one index column param
     * @param columnName index column name
     * @param dataType index data type
     * @param dimension index dimension
     * @return Builder
     */
    public Builder addIndexColumnParam(String columnName, DataType dataType, int dimension) {
      this.indexColumnParams.add(IndexColumnParam.newBuilder()
              .withColumnName(columnName)
              .withDataType(dataType)
              .withDimension(dimension)
              .build());
      return this;
    }

    /**
     * Build collection config
     * @return CollectionConfig
     */
    public CollectionConfig build() {
      return new CollectionConfig(this);
    }
  }
}
