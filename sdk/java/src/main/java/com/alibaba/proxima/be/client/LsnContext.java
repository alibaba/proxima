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
 * \brief    Contains lsn and context infor
 */

package com.alibaba.proxima.be.client;

/**
 * Lsn context information
 */
public class LsnContext {
  private long lsn;
  private String context;

  private LsnContext(Builder builder) {
    this.lsn = builder.lsn;
    this.context = builder.context;
  }

  public long getLsn() {
    return lsn;
  }

  public String getContext() {
    return context;
  }

  // New LsnContext builder
  public static Builder newBuilder() {
    return new Builder();
  }

  /**
   * Builder for LsnContext
   */
  public static class Builder {
    // required parameters
    private long lsn;
    private String context;

    public Builder() {
    }

    public Builder(long lsn, String context) {
      this.lsn = lsn;
      this.context = context;
    }

    /**
     * Set lsn number
     * @param lsn log sequence number
     * @return Builder
     */
    public Builder withLsn(long lsn) {
      this.lsn = lsn;
      return this;
    }

    /**
     * Set context information
     * @param context lsn context
     * @return Builder
     */
    public Builder withContext(String context) {
      this.context = context;
      return this;
    }

    /**
     * Build LsnContext object
     * @return LsnContext
     */
    public LsnContext build() {
      return new LsnContext(this);
    }
  }

}
