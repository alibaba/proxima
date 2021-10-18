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
 * \brief    Contains the query information
 */

package com.alibaba.proxima.be.client;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;
import java.util.HashMap;
import java.util.Map;

/**
 * Contains query information
 */
public class QueryRequest {
  private final QueryType queryType;
  private final String collectionName;
  private final boolean debugMode;
  private final KnnQueryParam knnQueryParam;

  private QueryRequest(Builder builder) {
    this.queryType = builder.queryType;
    this.collectionName = builder.collectionName;
    this.debugMode = builder.debugMode;
    this.knnQueryParam = builder.knnQueryParam;
  }

  public QueryType getQueryType() {
    return queryType;
  }

  public String getCollectionName() {
    return collectionName;
  }

  public boolean isDebugMode() {
    return debugMode;
  }

  public KnnQueryParam getKnnQueryParam() {
    return knnQueryParam;
  }

  /**
   * New QueryRequest builder
   * @return Builder
   */
  public static Builder newBuilder() {
    return new Builder();
  }

  /**
   * Builder for QueryRequest
   */
  public static class Builder {
    // required parameters
    private String collectionName;
    private KnnQueryParam knnQueryParam;

    // optional parameters
    private QueryType queryType = QueryType.KNN;
    private boolean debugMode = false;

    /**
     * Empty constructor
     */
    public Builder() {
    }

    /**
     * Constructor with collection name and query param
     * @param collectionName collection name
     * @param knnQueryParam knn query parameters
     */
    public Builder(String collectionName, KnnQueryParam knnQueryParam) {
      this.collectionName = collectionName;
      this.knnQueryParam = knnQueryParam;
    }

    /**
     * Set query type
     * @param queryType query type
     * @return Builder
     */
    public Builder withQueryType(QueryType queryType) {
      this.queryType = queryType;
      return this;
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
     * Set debug mode
     * @param debugMode debug mode, true means turnning the debug function
     * @return Builder
     */
    public Builder withDebugMode(boolean debugMode) {
      this.debugMode = debugMode;
      return this;
    }

    /**
     * Set knn query param
     * @param knnQueryParam knn query parameters
     * @return Builder
     */
    public Builder withKnnQueryParam(KnnQueryParam knnQueryParam) {
      this.knnQueryParam = knnQueryParam;
      return this;
    }

    /**
     * Build QueryRequest object
     * @return QueryRequest
     */
    public QueryRequest build() {
      return new QueryRequest(this);
    }
  }

  /**
   * Contains the query parameters
   */
  public static class KnnQueryParam {
    private final String columnName;
    private final int topk;
    private final byte[] features;
    private String matrix;
    private final int batchCount;
    private final int dimension;
    private final DataType dataType;
    private final float radius;
    private final boolean isLinear;
    private final Map<String, String> extraParams;

    private KnnQueryParam(Builder builder) {
      this.columnName = builder.columnName;
      this.topk = builder.topk;
      this.features = builder.features;
      this.matrix = builder.matrix;
      this.batchCount = builder.batchCount;
      this.dimension = builder.dimension;
      this.dataType = builder.dataType;
      this.radius = builder.radius;
      this.isLinear = builder.isLinear;
      this.extraParams = builder.extraParams;
    }

    public String getColumnName() {
      return columnName;
    }

    public int getTopk() {
      return topk;
    }

    public byte[] getFeatures() {
      return features;
    }

    public String getMatrix() {
      return matrix;
    }

    public int getBatchCount() {
      return batchCount;
    }

    public int getDimension() {
      return dimension;
    }

    public DataType getDataType() {
      return dataType;
    }

    public float getRadius() {
      return radius;
    }

    public boolean isLinear() {
      return isLinear;
    }

    public Map<String, String> getExtraParams() {
      return extraParams;
    }

    /**
     * New KnnQueryParam builder
     * @return Builder
     */
    public static Builder newBuilder() {
      return new Builder();
    }

    /**
     * Builder for KnnQueryParam
     */
    public static class Builder {
      // required parameters
      private String columnName;
      private byte[] features = null;
      private String matrix = null;

      // optional parameters
      private int topk = 100;
      private int batchCount = 1;
      private int dimension = 0;
      private DataType dataType = DataType.UNDEFINED;
      private float radius = 0.0f;
      private boolean isLinear = false;
      private Map<String, String> extraParams = new HashMap<>();

      /**
       * Empty constructor
       */
      public Builder() {
      }

      /**
       * Constructor with column name
       * @param columnName index column name
       */
      public Builder(String columnName) {
        this.columnName = columnName;
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
       * Set topk
       * @param topk return result number
       * @return Builder
       */
      public Builder withTopk(int topk) {
        this.topk = topk;
        return this;
      }

      /**
       * Set radius
       * @param radius score threshold, result that score less than radius can be return
       * @return Builder
       */
      public Builder withRadius(float radius) {
        this.radius = radius;
        return this;
      }

      // Set use linear

      /**
       * Set use linear search flag
       * @param linear use linear
       * @return Builder
       */
      public Builder withLinear(boolean linear) {
        isLinear = linear;
        return this;
      }

      /**
       * Set extra params
       * @param extraParams extra parameters
       * @return Builder
       */
      public Builder withExtraParams(Map<String, String> extraParams) {
        this.extraParams = extraParams;
        return this;
      }

      /**
       * Set bytes features
       * @param features bytes features
       * @param dataType data type
       * @param dimension dimension
       * @param batchCount query batch count
       * @return Builder
       */
      public Builder withFeatures(byte[] features, DataType dataType, int dimension, int batchCount) {
        if (features == null || features.length == 0) {
          return this;
        }

        this.features = features;

        this.dataType = dataType;
        this.dimension = dimension;
        this.batchCount = batchCount;

        return this;
      }

      /**
       * Set features with json matrix
       * @param jsonMatrix json format matrix
       * @param dataType data type
       * @param dimension dimension
       * @param batchCount query batch count
       * @return Builder
       */
      public Builder withFeatures(String jsonMatrix, DataType dataType, int dimension, int batchCount) {
        if (jsonMatrix == null || jsonMatrix.isEmpty()) {
          return this;
        }
        this.matrix = jsonMatrix;

        this.dataType = dataType;
        this.dimension = dimension;
        this.batchCount = batchCount;

        return this;
      }

      /**
       * Set float type features
       * @param features query features
       * @return Builder
       */
      public Builder withFeatures(float[] features) {
        // pack float features
        if (features == null || features.length == 0) {
          return this;
        }

        ByteBuffer bf = ByteBuffer.allocate(features.length * 4).order(ByteOrder.LITTLE_ENDIAN);
        bf.asFloatBuffer().put(features);
        this.features = bf.array();

        this.dataType = DataType.VECTOR_FP32;
        this.dimension = features.length;
        this.batchCount = 1;

        return this;
      }

      /**
       * Set two dimension float type features
       * @param features query features
       * @return Builder
       */
      public Builder withFeatures(float[][] features) {
        if (features == null || features.length == 0) {
          return this;
        }
        int dim = features[0].length;
        for (int i = 0; i < features.length; ++i) {
          if (features[i].length != dim) {
            return this;
          }
        }

        ByteBuffer bf = ByteBuffer.allocate(features.length * dim * 4).order(ByteOrder.LITTLE_ENDIAN);
        FloatBuffer floatBuffer = bf.asFloatBuffer();
        for (int i = 0; i < features.length; ++i) {
          floatBuffer.put(features[i]);
        }
        this.features = bf.array();

        this.dataType = DataType.VECTOR_FP32;
        this.batchCount = features.length;
        this.dimension = dim;

        return this;
      }

      /**
       * Add one extra parameter
       * @param key extra parameter key
       * @param value extra parameter value
       * @return Builder
       */
      public Builder addExtraParam(String key, String value) {
        this.extraParams.put(key, value);
        return this;
      }

      /**
       * Build KnnQueryParam object
       * @return KnnQueryBuilder
       */
      public KnnQueryParam build() {
        return new KnnQueryParam(this);
      }
    }
  }

  /**
   * Query type
   */
  public enum QueryType {
    /**
     * Knn query type
     */
    KNN(0);

    private int value;

    QueryType(int value) {
      this.value = value;
    }

    public int getValue() {
      return this.value;
    }
  }
}
