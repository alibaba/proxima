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
 * \brief    Document contains the pk, score, and forward values
 */

package com.alibaba.proxima.be.client;

import com.alibaba.proxima.be.grpc.GenericValue;

import java.util.HashMap;
import java.util.Map;
import java.util.Set;

/**
 * Document information
 */
public class Document {
  private final long primaryKey;
  private final float score;
  private final Map<String, ForwardValue> forwardColumnMap;

  private Document(Builder builder) {
    this.primaryKey = builder.primaryKey;
    this.score = builder.score;
    this.forwardColumnMap = builder.forwardColumnMap;
  }

  public long getPrimaryKey() {
    return primaryKey;
  }

  public float getScore() {
    return score;
  }

  public Map<String, ForwardValue> getForwardColumnMap() {
    return forwardColumnMap;
  }

  /**
   * Get forward column count
   * @return Forward column count
   */
  public int getForwardColumnCount() {
    if (forwardColumnMap != null) {
      return forwardColumnMap.size();
    }
    return 0;
  }

  /**
   * Get forward key set
   * @return forward key set
   */
  public Set<String> getForwardKeySet() {
    return this.forwardColumnMap.keySet();
  }

  /**
   * Get specified forward value
   * @param key the forward key
   * @return ForwardValue
   */
  public ForwardValue getForwardValue(String key) {
    return this.forwardColumnMap.get(key);
  }

  /**
   * New document builder
   * @return Builder
   */
  public static Builder newBuilder() {
    return new Builder();
  }

  /**
   * Represents all types forward value
   */
  public static class ForwardValue {
    private GenericValue value;

    private ForwardValue(Builder builder) {
      this.value = builder.value;
    }

    // Get float value
    public float getFloatValue() {
      return this.value.getFloatValue();
    }

    // Get double value
    public double getDoubleValue() {
      return this.value.getDoubleValue();
    }

    // Get int64 value
    public long getInt64Value() {
      return this.value.getInt64Value();
    }

    // Get int32 value
    public int getInt32Value() {
      return this.value.getInt32Value();
    }

    // Get uint64 value
    public long getUint64Value() {
      return this.value.getUint64Value();
    }

    // Get uint32 value
    public int getUint32Value() {
      return this.value.getUint32Value();
    }

    // Get boolean value
    public boolean getBooleanValue() {
      return this.value.getBoolValue();
    }

    /**
     * Get string value, if forward's type not string, will also convert to string
     * @return String
     */
    public String getStringValue() {
      switch (value.getValueOneofCase()) {
        case VALUEONEOF_NOT_SET:
          return "";
        case BYTES_VALUE:
          return value.getBytesValue().toString();
        case STRING_VALUE:
          return value.getStringValue();
        case BOOL_VALUE:
          return String.valueOf(value.getBoolValue());
        case INT32_VALUE:
          return String.valueOf(value.getInt32Value());
        case INT64_VALUE:
          return String.valueOf(value.getInt64Value());
        case UINT32_VALUE:
          return String.valueOf(value.getUint32Value());
        case UINT64_VALUE:
          return String.valueOf(value.getUint64Value());
        case FLOAT_VALUE:
          return String.valueOf(value.getFloatValue());
        case DOUBLE_VALUE:
          return String.valueOf(value.getDoubleValue());
        default:
          return "";
      }
    }

    // Get bytes value
    public byte[] getBytesValue() {
      return this.value.getBytesValue().toByteArray();
    }

    /** Builder for ForwardValue */
    public static class Builder {
      private GenericValue value;

      public Builder(GenericValue genericValue) {
        this.value = genericValue;
      }

      public ForwardValue build() {
        return new ForwardValue(this);
      }
    }
  }

  /**
   * Builder for Document
   */
  public static class Builder {
    // required parameters
    private long primaryKey;
    private float score;

    // optional parameters
    private Map<String, ForwardValue> forwardColumnMap = new HashMap<>();

    // Empty constructor
    public Builder() {
    }

    // Constructor
    public Builder withPrimaryKey(long primaryKey) {
      this.primaryKey = primaryKey;
      return this;
    }

    // Set score
    public Builder withScore(float score) {
      this.score = score;
      return this;
    }

    // Set forward column map
    public Builder withForwardColumnMap(Map<String, ForwardValue> forwardColumnMap) {
      this.forwardColumnMap = forwardColumnMap;
      return this;
    }

    // Build Document object
    public Document build() {
      return new Document(this);
    }
  }
}
