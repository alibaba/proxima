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
 * \brief    IndexType contains all supported index types
 */

package com.alibaba.proxima.be.client;

/**
 * Contains all index types
 */
public enum IndexType {
  /**
   * Undefined index type
   */
  UNDEFINED(0),
  /**
   * Poxima graph index type
   */
  PROXIMA_GRAPH_INDEX(1);

  private int value;

  IndexType(int value) {
    this.value = value;
  }

  public int getValue() {
    return value;
  }

  public static IndexType valueOf(int value) {
    switch (value) {
      case 0:
        return UNDEFINED;
      case 1:
        return PROXIMA_GRAPH_INDEX;
      default:
        return UNDEFINED;
    }
  }
}
