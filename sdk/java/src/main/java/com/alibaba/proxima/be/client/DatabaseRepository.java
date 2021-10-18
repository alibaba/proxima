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
 * \brief    DatabaseRepository contains the database config
 */

package com.alibaba.proxima.be.client;

/**
 * Contains the database config
 */
public class DatabaseRepository {
  private final String repositoryName;
  private final String connectionUri;
  private final String tableName;
  private final String user;
  private final String password;

  private DatabaseRepository(Builder builder) {
    this.repositoryName = builder.repositoryName;
    this.connectionUri = builder.connectionUri;
    this.tableName = builder.tableName;
    this.user = builder.user;
    this.password = builder.password;
  }

  public String getRepositoryName() {
    return repositoryName;
  }

  public String getConnectionUri() {
    return connectionUri;
  }

  public String getTableName() {
    return tableName;
  }

  public String getUser() {
    return user;
  }

  public String getPassword() {
    return password;
  }

  /**
   * New DatabaseRepository builde
   * @return Builder
   */
  public static Builder newBuilder() {
    return new Builder();
  }

  /**
   * Builder for DatabaseRepository
   */
  public static class Builder {
    // required parameters
    private String repositoryName;
    private String connectionUri;
    private String tableName;
    private String user;
    private String password;

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
     * Set connection uri
     * @param connectionUri connection uri
     * @return Builder
     */
    public Builder withConnectionUri(String connectionUri) {
      this.connectionUri = connectionUri;
      return this;
    }

    /**
     * Set table name
     * @param tableName mysql table name
     * @return Builder
     */
    public Builder withTableName(String tableName) {
      this.tableName = tableName;
      return this;
    }

    /**
     * Set database username
     * @param user user name
     * @return Builder
     */
    public Builder withUser(String user) {
      this.user = user;
      return this;
    }

    /**
     * Set database password
     * @param password myssql password
     * @return Builder
     */
    public Builder withPassword(String password) {
      this.password = password;
      return this;
    }

    /**
     * Build the DatabaseRepository object
     * @return DatabaseRepository
     */
    public DatabaseRepository build() {
      return new DatabaseRepository(this);
    }
  }
}
