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
 * \brief    Represents the list condition.
 */

package com.alibaba.proxima.be.client;

/**
 * List collections condition
 */
public class ListCondition {
  private final String repositoryName;

  private ListCondition(Builder builder) {
    this.repositoryName = builder.repositoryName;
  }

  public String getRepositoryName() {
    return repositoryName;
  }

  /**
   * New ListCondition builder
   * @return Builder
   */
  public static Builder newBuilder() {
    return new Builder();
  }

  /**
   * Builder for ListCondition
   */
  public static class Builder {
    private String repositoryName;

    /**
     * Empty constructor
     */
    public Builder() {
    }

    /**
     * Set repository name
     * @param repositoryName repository name
     * @return Builder
     */
    public Builder withRepositoryName(String repositoryName) {
      this.repositoryName = repositoryName;
      return this;
    }

    /**
     * Build list condition object
     * @return ListCondition
     */
    public ListCondition build() {
      return new ListCondition(this);
    }
  }
}
