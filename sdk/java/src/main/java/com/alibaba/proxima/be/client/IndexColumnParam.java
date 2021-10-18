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
 * \brief    IndexColumnParam contains parameters for index column
 */

package com.alibaba.proxima.be.client;

import java.util.HashMap;
import java.util.Map;

/**
 * Contains parameters for index column
 */
public class IndexColumnParam {
  private final String columnName;
  private final IndexType indexType;
  private final DataType dataType;
  private final int dimension;
  private final Map<String, String> extraParams;

  private IndexColumnParam(Builder builder) {
    this.columnName = builder.columnName;
    this.indexType = builder.indexType;
    this.dataType = builder.dataType;
    this.dimension = builder.dimension;
    this.extraParams = builder.extraParams;
  }

  public String getColumnName() {
    return columnName;
  }

  public IndexType getIndexType() {
    return indexType;
  }

  public DataType getDataType() {
    return dataType;
  }

  public int getDimension() {
    return dimension;
  }

  public Map<String, String> getExtraParams() {
    return extraParams;
  }

  /**
   * New IndexColumnParam builder
   * @return Builder
   */
  public static Builder newBuilder() {
    return new Builder();
  }

  /**
   * Builder for IndexColumnParam
   */
  public static class Builder {
    // required parameters
    private String columnName;
    private DataType dataType = DataType.UNDEFINED;
    private int dimension = 0;

    // optional parameters
    private IndexType indexType = IndexType.PROXIMA_GRAPH_INDEX;
    private Map<String, String> extraParams = new HashMap<>();

    /**
     * Empty constructor
     */
    public Builder() {
    }

    /**
     * Constructor with parameters
     * @param columnName index column name
     * @param dataType index data type
     * @param dimension index dimension
     */
    public Builder(String columnName, DataType dataType, int dimension) {
      this.columnName = columnName;
      this.dataType = dataType;
      this.dimension = dimension;
    }

    /**
     * Set index column name
     * @param columnName index column name
     * @return Builder
     */
    public Builder withColumnName(String columnName) {
      this.columnName = columnName;
      return this;
    }

    /**
     * Set index data type
     * @param dataType index data type
     * @return Builder
     */
    public Builder withDataType(DataType dataType) {
      this.dataType = dataType;
      return this;
    }

    // Set dimension

    /**
     * Set index dimension
     * @param dimension index dimension
     * @return Builder
     */
    public Builder withDimension(int dimension) {
      this.dimension = dimension;
      return this;
    }

    /**
     * Set index type
     * @param indexType index type
     * @return Builder
     */
    public Builder withIndexType(IndexType indexType) {
      this.indexType = indexType;
      return this;
    }

    // Set extra params

    /**
     * Set extra parameters
     * @param extraParams extra parameters
     * @return Builder
     */
    public Builder withExtraParams(Map<String, String> extraParams) {
      this.extraParams = extraParams;
      return this;
    }

    /**
     * Add one extra param
     * @param key extra parameter key
     * @param value extra parameter value
     * @return Builder
     */
    public Builder addExtraParam(String key, String value) {
      this.extraParams.put(key, value);
      return this;
    }

    /**
     * Build IndexColumnParam object
     * @return IndexColumnParam
     */
    public IndexColumnParam build() {
      return new IndexColumnParam(this);
    }
  }
}

