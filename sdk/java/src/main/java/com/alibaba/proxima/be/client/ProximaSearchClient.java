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
 * \brief    Interface to describe proxima client interface and action.
 * You can get more details usage from examples
 */

package com.alibaba.proxima.be.client;

import java.io.IOException;
import java.io.InputStream;
import java.util.Properties;
import java.util.function.Supplier;

/**
 * The Proxima Search Client Interface
 */
public interface ProximaSearchClient {
  final String CLIENT_VERSION = new Supplier<String>() {
    @Override
    public String get() {
      Properties properties = new Properties();
      InputStream inputStream = ProximaSearchClient.class.getClassLoader()
              .getResourceAsStream("proxima-be.properties");
      try {
        properties.load(inputStream);
      } catch (IOException e) {
        e.printStackTrace();
      } finally {
        try {
          inputStream.close();
        } catch (IOException e) {
        }
      }
      return properties.getProperty("version");
    }
  }.get();

  /**
   * Get proxima search client version
   * @return String: client version
   */
  default String getClientVersion() {
    return this.CLIENT_VERSION;
  }

  /**
   * Get proxima server version
   * @return GetVersionResponse
   */
  GetVersionResponse getVersion();

  /**
   * Create collection with collection config
   * @param collectionConfig the CollectionConfig object
   * @return Status
   */
  Status createCollection(CollectionConfig collectionConfig);

  /**
   * Drop collection with colleciton name
   * @param collectionName the collection name
   * @return Status
   */
  Status dropCollection(String collectionName);

  /**
   * Describe collection with colleciton name
   * @param collectionName the collection name
   * @return DescribeCollectionResponse: describe collection response
   */
  DescribeCollectionResponse describeCollection(String collectionName);

  /**
   * List collections with list condition
   * @param listCondition list collections condition
   * @return ListCollectionsResponse: list collections response
   */
  ListCollectionsResponse listCollections(ListCondition listCondition);

  // Stats collection with collection name

  /**
   * Stats collection with collection name
   * @param collectionName the collection name
   * @return stats collection response
   */
  StatsCollectionResponse statsCollection(String collectionName);

  /**
   * Write records for collection
   * @param request write request included insert/update/delete operations
   * @return Status
   */
  Status write(WriteRequest request);

  /**
   * Query records with specified features
   * @param request query request included querying featuress
   * @return QueryResponse: included query result
   */
  QueryResponse query(QueryRequest request);

  /**
   * Get document by primary key
   * @param request the GetDocumentRequest object included collection name and primary key
   * @return GetDocumentResponse: document response
   */
  GetDocumentResponse getDocumentByKey(GetDocumentRequest request);

  /**
   * Close client
   */
  void close();

  /**
   * Close client
   * @param maxWaitSeconds max wait seconds when close client
   */
  void close(int maxWaitSeconds);
}
