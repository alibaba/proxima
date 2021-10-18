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
 * \brief    Represents the result for one query
 */

package com.alibaba.proxima.be.client;

import java.util.ArrayList;
import java.util.List;

/**
 * Contains query result documents
 */
public class QueryResult {
  private final List<Document> documents;

  private QueryResult(Builder builder) {
    this.documents = builder.documents;
  }

  public List<Document> getDocuments() {
    return documents;
  }

  /**
   * Get query document count
   * @return int
   */
  public int getDocumentCount() {
    if (this.documents != null) {
      return this.documents.size();
    }
    return 0;
  }

  /**
   * Get specified document
   * @param index docucment index
   * @return Document
   */
  public Document getDocument(int index) {
    if (index < documents.size()) {
      return this.documents.get(index);
    }
    return null;
  }

  /**
   * New QueryResult builder
   * @return Builder
   */
  public static Builder newBuilder() {
    return new Builder();
  }

  /**
   * Builder for QueryResult
   */
  public static class Builder {
    private List<Document> documents = new ArrayList<>();

    /**
     * Empty constructor
     */
    public Builder() {
    }

    /**
     * Set document list
     * @param documents document list
     * @return Builder
     */
    public Builder withDocuments(List<Document> documents) {
      this.documents = documents;
      return this;
    }

    /**
     * Add one document
     * @param document added document
     * @return Builder
     */
    public Builder addDocument(Document document) {
      this.documents.add(document);
      return this;
    }

    /**
     * Build QueryResult object
     * @return QueryResult
     */
    public QueryResult build() {
      return new QueryResult(this);
    }
  }
}
