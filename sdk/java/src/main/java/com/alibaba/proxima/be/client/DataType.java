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
 * \brief    DataType contains all data types for proxima search engine
 */

package com.alibaba.proxima.be.client;

/**
 * DataType contains all supported data types
 */
public enum DataType {
  /**
   * Undefined data type
   */
  UNDEFINED(0),
  /**
   * Binary data type
   */
  BINARY(1),
  /**
   * String data type
   */
  STRING(2),
  /**
   * Bool data type
   */
  BOOL(3),
  /**
   * Int32 data type
   */
  INT32(4),
  /**
   * Int64 data type
   */
  INT64(5),
  /**
   * Uint32 data type
   */
  UINT32(6),
  /**
   * Uint64 data type
   */
  UINT64(7),
  /**
   * Float data type
   */
  FLOAT(8),
  /**
   * Double data type
   */
  DOUBLE(9),

  /**
   * Vector binary32 data type
   */
  VECTOR_BINARY32(20),
  /**
   * Vector binary64 data type
   */
  VECTOR_BINARY64(21),
  /**
   * Vector fp16 data type
   */
  VECTOR_FP16(22),
  /**
   * Vector fp32 data type
   */
  VECTOR_FP32(23),
  /**
   * Vector double data type
   */
  VECTOR_FP64(24),
  /**
   * Vector int4 data type
   */
  VECTOR_INT4(25),
  /**
   * Vector int8 data type
   */
  VECTOR_INT8(26),
  /**
   * Vector int16 data type
   */
  VECTOR_INT16(27);

  private int value;

  DataType(int value) {
    this.value = value;
  }

  public int getValue() {
    return this.value;
  }

  public static DataType valueOf(int value) {
    switch (value) {
      case 0:
        return UNDEFINED;
      case 1:
        return BINARY;
      case 2:
        return STRING;
      case 3:
        return BOOL;
      case 4:
        return INT32;
      case 5:
        return INT64;
      case 6:
        return UINT32;
      case 7:
        return UINT64;
      case 8:
        return FLOAT;
      case 9:
        return DOUBLE;
      case 20:
        return VECTOR_BINARY32;
      case 21:
        return VECTOR_BINARY64;
      case 22:
        return VECTOR_FP16;
      case 23:
        return VECTOR_FP32;
      case 24:
        return VECTOR_FP64;
      case 25:
        return VECTOR_INT4;
      case 26:
        return VECTOR_INT8;
      case 27:
        return VECTOR_INT16;
      default:
        return UNDEFINED;
    }
  }
}
