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
 * \brief    Contains the rows information for collection
 */

package com.alibaba.proxima.be.client;

import com.alibaba.proxima.be.grpc.GenericValue;
import com.alibaba.proxima.be.grpc.GenericValueList;
import com.google.protobuf.ByteString;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.ArrayList;
import java.util.List;

/**
 * Contains the write rows information
 */
public class WriteRequest {
  private final String collectionName;
  private final RowMeta rowMeta;
  private final List<Row> rows;
  private final String requestId;
  private final long magicNumber;

  private WriteRequest(Builder builder) {
    this.collectionName = builder.collectionName;
    this.rowMeta = builder.rowMetaBuilder.build();
    this.rows = builder.rows;
    this.requestId = builder.requestId;
    this.magicNumber = builder.magicNumber;
  }

  public String getCollectionName() {
    return collectionName;
  }

  public RowMeta getRowMeta() {
    return rowMeta;
  }

  public List<Row> getRows() {
    return rows;
  }

  public String getRequestId() {
    return requestId;
  }

  public long getMagicNumber() {
    return magicNumber;
  }

  /**
   * New WriteRequest builder
   * @return Builder
   */
  public static Builder newBuilder() {
    return new Builder();
  }

  /**
   * Builder for WriteRequest
   */
  public static class Builder {
    // required parameters
    private String collectionName;
    private RowMeta.Builder rowMetaBuilder = RowMeta.newBuilder();
    private List<Row> rows = new ArrayList<>();

    // optional parameters
    private String requestId = "";
    private long magicNumber = 0;

    /**
     * Constructor without parameters
     */
    public Builder() {
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
     * Set row list
     * @param rows write row list
     * @return Builder
     */
    public Builder withRows(List<Row> rows) {
      this.rows = rows;
      return this;
    }

    /**
     * Set Request id
     * @param requestId request id
     * @return Builder
     */
    public Builder withRequestId(String requestId) {
      this.requestId = requestId;
      return this;
    }

    /**
     * Set magic number
     * @param magicNumber magic number
     * @return Builder
     */
    public Builder withMagicNumber(long magicNumber) {
      this.magicNumber = magicNumber;
      return this;
    }

    /**
     * Set forward column list
     * @param forwardColumnList forward column list
     * @return Builder
     */
    public Builder withForwardColumnList(List<String> forwardColumnList) {
      for (String columnName : forwardColumnList) {
        this.rowMetaBuilder.addForwardColumnName(columnName);
      }
      return this;
    }

    /**
     * Set index column meta list
     * @param indexColumnMetaList index column meta list
     * @return Builder
     */
    public Builder withIndexColumnMetaList(List<IndexColumnMeta> indexColumnMetaList) {
      this.rowMetaBuilder.withIndexColumnMetas(indexColumnMetaList);
      return this;
    }

    /**
     * Add one index column meta information
     * @param columnName index column name
     * @param dataType index data type
     * @param dimension index dimension
     * @return Builder
     */
    public Builder addIndexColumnMeta(String columnName, DataType dataType, int dimension) {
      this.rowMetaBuilder.addIndexColumnMeta(
              new IndexColumnMeta.Builder(columnName, dataType, dimension).build());
      return this;
    }

    /**
     * Add one forward column
     * @param columnName forward column name
     * @return Builder
     */
    public Builder addForwardColumn(String columnName) {
      this.rowMetaBuilder.addForwardColumnName(columnName);
      return this;
    }

    /**
     * Add one row
     * @param row write row information
     * @return Builder
     */
    public Builder addRow(Row row) {
      this.rows.add(row);
      return this;
    }

    /**
     * Build WriteRequest object
     * @return WriteRequest
     */
    public WriteRequest build() {
      return new WriteRequest(this);
    }
  }

  /**
   * Document operation type
   */
  public enum OperationType {
    /**
     * Insert record
     */
    INSERT(0),
    /**
     * Update record
     */
    UPDATE(1),
    /**
     * Delete record
     */
    DELETE(2);

    private int value;

    OperationType(int value) {
      this.value = value;
    }

    public int getValue() {
      return this.value;
    }
  }

  /**
   * Contains index and forward writing information
   */
  public static class Row {
    private final long primaryKey;
    private final OperationType operationType;
    private final GenericValueList indexValues;
    private final GenericValueList forwardValues;
    private final LsnContext lsnContext;

    private Row(Builder builder) {
      this.primaryKey = builder.primaryKey;
      this.operationType = builder.operationType;
      this.indexValues = builder.indexColumnBuilder.build();
      this.forwardValues = builder.forwardColumnBuilder.build();
      this.lsnContext = builder.lsnContext;
    }

    public long getPrimaryKey() {
      return primaryKey;
    }

    public OperationType getOperationType() {
      return operationType;
    }

    public GenericValueList getIndexValues() {
      return indexValues;
    }

    public GenericValueList getForwardValues() {
      return forwardValues;
    }

    public LsnContext getLsnContext() {
      return lsnContext;
    }

    /**
     * New row builder object
     * @return Builder
     */
    public static Builder newBuilder() {
      return new Builder();
    }

    /**
     * Builder for Row
     */
    public static class Builder {
      // required parameters
      private long primaryKey;
      private OperationType operationType;
      private GenericValueList.Builder indexColumnBuilder = GenericValueList.newBuilder();
      // optional parameters
      private GenericValueList.Builder forwardColumnBuilder = GenericValueList.newBuilder();
      private LsnContext lsnContext = null;

      /**
       * Empty constructor
       */
      public Builder() {
      }

      /**
       * Constructor with primary key and operation type
       * @param primaryKey primary key
       * @param operationType operation type
       */
      public Builder(long primaryKey, OperationType operationType) {
        this.primaryKey = primaryKey;
        this.operationType = operationType;
      }

      /**
       * Set primary key
       * @param primaryKey primary key
       * @return Builder
       */
      public Builder withPrimaryKey(long primaryKey) {
        this.primaryKey = primaryKey;
        return this;
      }

      /**
       * Set operation type
       * @param operationType operation type
       * @return Builder
       */
      public Builder withOperationType(OperationType operationType) {
        this.operationType = operationType;
        return this;
      }

      /**
       * Set lsn context
       * @param lsnContext lsn context information
       * @return Builder
       */
      public Builder withLsnContext(LsnContext lsnContext) {
        this.lsnContext = lsnContext;
        return this;
      }

      /**
       * Add string type index column value
       * @param indexValue string index column value
       * @return Builder
       */
      public Builder addIndexValue(String indexValue) {
        indexColumnBuilder.addValues(
                GenericValue.newBuilder().setStringValue(indexValue).build()
        );
        return this;
      }

      /**
       * Add bytes type index column value
       * @param indexValue bytes index column value
       * @return Builder
       */
      public Builder addIndexValue(byte[] indexValue) {
        indexColumnBuilder.addValues(
                GenericValue.newBuilder().setBytesValue(ByteString.copyFrom(indexValue))
        );
        return this;
      }

      /**
       * Add float type index column value
       * @param indexValue float index column value
       * @return Builder
       */
      public Builder addIndexValue(float[] indexValue) {
        if (indexValue == null || indexValue.length == 0) {
          return this;
        }

        ByteBuffer bf = ByteBuffer.allocate(indexValue.length * 4).order(ByteOrder.LITTLE_ENDIAN);
        bf.asFloatBuffer().put(indexValue);
        indexColumnBuilder.addValues(
                GenericValue.newBuilder().setBytesValue(ByteString.copyFrom(bf.array())));

        return this;
      }

      /**
       * Add byte type forward column value
       * @param forwardValue bytes forward column value
       * @return Builder
       */
      public Builder addForwardValue(byte[] forwardValue) {
        forwardColumnBuilder.addValues(
                GenericValue.newBuilder().setBytesValue(ByteString.copyFrom(forwardValue))
        );
        return this;
      }

      /**
       * Add string type forward column value
       * @param forwardValuey string forward value
       * @return Builder
       */
      public Builder addForwardValue(String forwardValuey) {
        forwardColumnBuilder.addValues(
                GenericValue.newBuilder().setStringValue(forwardValuey)
        );
        return this;
      }

      /**
       * Add boolean type forward column value
       * @param forwardValuey boolean forward value
       * @return Builder
       */
      public Builder addForwardValue(boolean forwardValuey) {
        forwardColumnBuilder.addValues(
                GenericValue.newBuilder().setBoolValue(forwardValuey)
        );
        return this;
      }

      /**
       * Add int type forward column value
       * @param forwardValuey int forward value
       * @return Builder
       */
      public Builder addForwardValue(int forwardValuey) {
        forwardColumnBuilder.addValues(
                GenericValue.newBuilder().setInt32Value(forwardValuey)
        );
        return this;
      }

      /**
       * Add long type forward column value
       * @param forwardValuey long forward value
       * @return Builder
       */
      public Builder addForwardValue(long forwardValuey) {
        forwardColumnBuilder.addValues(
                GenericValue.newBuilder().setInt64Value(forwardValuey)
        );
        return this;
      }

      /**
       * Add float type forward column value
       * @param forwardValuey float forward value
       * @return Builder
       */
      public Builder addForwardValue(float forwardValuey) {
        forwardColumnBuilder.addValues(
                GenericValue.newBuilder().setFloatValue(forwardValuey)
        );
        return this;
      }

      /**
       * Add double type forward column value
       * @param forwardValuey double forward value
       * @return Builder
       */
      public Builder addForwardValue(double forwardValuey) {
        forwardColumnBuilder.addValues(
                GenericValue.newBuilder().setDoubleValue(forwardValuey)
        );
        return this;
      }

      /**
       * Build Row object
       * @return Row
       */
      public Row build() {
        return new Row(this);
      }
    }
  }

  /**
   * Contains the index column information
   */
  public static class IndexColumnMeta {
    private final String columnName;
    private final DataType dataType;
    private final int dimension;

    private IndexColumnMeta(Builder builder) {
      this.columnName = builder.columnName;
      this.dataType = builder.dataType;
      this.dimension = builder.dimension;
    }

    public String getColumnName() {
      return columnName;
    }

    public DataType getDataType() {
      return dataType;
    }

    public int getDimension() {
      return dimension;
    }

    /**
     * New IndexColumnMeta builder
     * @return Builder
     */
    public static Builder newBuilder() {
      return new Builder();
    }

    /** Builder for IndexColumnMeta */
    public static class Builder {
      // required parameters
      private String columnName;
      private DataType dataType;
      private int dimension;

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
       * Set column name
       * @param columnName index column name
       * @return Builder
       */
      public Builder withColumnName(String columnName) {
        this.columnName = columnName;
        return this;
      }

      /**
       * Set data type
       * @param dataType index data type
       * @return Builder
       */
      public Builder withDataType(DataType dataType) {
        this.dataType = dataType;
        return this;
      }

      /**
       * Set dimension
       * @param dimension index dimension
       * @return Builder
       */
      public Builder withDimension(int dimension) {
        this.dimension = dimension;
        return this;
      }

      /**
       * Build IndexColumnMeta object
       * @return IndexColumnMeta
       */
      public IndexColumnMeta build() {
        return new IndexColumnMeta(this);
      }
    }
  }

  /**
   * Contains index and forward metas
   */
  public static class RowMeta {
    private final List<IndexColumnMeta> indexColumnMetas;
    private final List<String> forwardColumnNames;

    private RowMeta(Builder builder) {
      this.indexColumnMetas = builder.indexColumnMetas;
      this.forwardColumnNames = builder.forwardColumnNames;
    }

    public List<IndexColumnMeta> getIndexColumnMetas() {
      return indexColumnMetas;
    }

    public List<String> getForwardColumnNames() {
      return forwardColumnNames;
    }

    /**
     * New RowMeta builder
     * @return Builder
     */
    public static Builder newBuilder() {
      return new Builder();
    }

    /** Builder for RowMeta */
    public static class Builder {
      // required parameters
      private List<IndexColumnMeta> indexColumnMetas = new ArrayList<>();

      // optional parameters
      private List<String> forwardColumnNames = new ArrayList<>();

      /**
       * Empty constructor
       */
      public Builder() {
      }

      /**
       * Set index column meta list
       * @param indexColumnMetas index column meta list
       * @return Builder
       */
      public Builder withIndexColumnMetas(List<IndexColumnMeta> indexColumnMetas) {
        this.indexColumnMetas = indexColumnMetas;
        return this;
      }

      /**
       * Set forward column name list
       * @param forwardColumnNames forward column name list
       * @return Builder
       */
      public Builder withForwardColumnNames(List<String> forwardColumnNames) {
        this.forwardColumnNames = forwardColumnNames;
        return this;
      }

      /**
       * Add single index column meta
       * @param indexColumnMeta index column meta
       * @return Builder
       */
      public Builder addIndexColumnMeta(IndexColumnMeta indexColumnMeta) {
        this.indexColumnMetas.add(indexColumnMeta);
        return this;
      }

      /**
       * Add single forward column name
       * @param forwardColumnName forward column name
       * @return Builder
       */
      public Builder addForwardColumnName(String forwardColumnName) {
        this.forwardColumnNames.add(forwardColumnName);
        return this;
      }

      /**
       * Build RowMeta object
       * @return RowMeta
       */
      public RowMeta build() {
        return new RowMeta(this);
      }

    }
  }

}
