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
 * \date     Apr 2021
 */

package com.alibaba.example;

import com.alibaba.proxima.be.client.*;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.util.Arrays;
import java.util.List;
import java.util.Set;

public class TestExample {
  private static final Logger logger = LoggerFactory.getLogger(TestExample.class);

  public static void main(String[] args) {
    // init connect params
    ConnectParam connectParam = ConnectParam.newBuilder()
            .withHost("127.0.0.1")
            .withPort(16000)
            .build();

    // create client
    ProximaSearchClient client = new ProximaGrpcSearchClient(connectParam);

    String collectionName = "collection1";
    int dimension = 8;

    // create collection
    CollectionConfig config = CollectionConfig.newBuilder()
            .withCollectionName(collectionName)
            .withForwardColumnNames(Arrays.asList("forward1", "forward2"))
            .addIndexColumnParam("index1", DataType.VECTOR_FP32, dimension)
            .build();

    try {
      Status status = client.createCollection(config);
      if (status.ok()) {
        logger.info("============== Create collection success. ================");
      } else {
        logger.info("Create collection failed." + status.toString());
      }
    } catch (ProximaSEException e) {
      e.printStackTrace();
    }

    // describe collection
    DescribeCollectionResponse descResponse = client.describeCollection(collectionName);
    if (descResponse.ok()) {
      logger.info("================ Describe collection success. ===================");
      CollectionInfo info = descResponse.getCollectionInfo();

      CollectionConfig collectionConfig = info.getCollectionConfig();
      logger.info("CollectionName: " + collectionConfig.getCollectionName());
      logger.info("CollectionStatus: " + info.getCollectionStatus());
      logger.info("Uuid: " + info.getUuid());
      logger.info("Forward columns: " + collectionConfig.getForwardColumnNames());
      List<IndexColumnParam> indexColumnParamList = collectionConfig.getIndexColumnParams();
      for (int i = 0; i < indexColumnParamList.size(); ++i) {
        IndexColumnParam indexParam = indexColumnParamList.get(i);
        logger.info("IndexColumn: " + indexParam.getColumnName());
        logger.info("IndexType: " + indexParam.getDataType());
        logger.info("DataType: " + indexParam.getDataType());
        logger.info("Dimension: " + indexParam.getDimension());
      }
    } else {
      logger.info("Describe collection failed " + descResponse.toString());
      return;
    }

    // list collections
    ListCondition listCondition = null;
    ListCollectionsResponse listResponse = client.listCollections(listCondition);
    if (listResponse.ok()) {
      logger.info("List collections success.");
      for (int i = 0; i < listResponse.getCollectionCount(); ++i) {
        logger.info("Collection " + i);
        CollectionInfo info = listResponse.getCollection(i);
        CollectionConfig collectionConfig = info.getCollectionConfig();
        logger.info("CollectionName: " + collectionConfig.getCollectionName());
        logger.info("CollectionStatus: " + info.getCollectionStatus());
        logger.info("Uuid: " + info.getUuid());
        logger.info("Forward columns: " + collectionConfig.getForwardColumnNames());
        List<IndexColumnParam> indexColumnParamList = collectionConfig.getIndexColumnParams();
        for (int j = 0; j < indexColumnParamList.size(); ++j) {
          IndexColumnParam indexParam = indexColumnParamList.get(j);
          logger.info("IndexColumn: " + indexParam.getColumnName());
          logger.info("IndexType: " + indexParam.getDataType());
          logger.info("DataType: " + indexParam.getDataType());
          logger.info("Dimension: " + indexParam.getDimension());
        }
      }
    } else {
      logger.info("List collections failed " + listResponse.toString());
      return;
    }

    // write request
    float[] vectors = {1, 2, 3, 4, 5, 6, 7, 8};
    WriteRequest.Row row1 = WriteRequest.Row.newBuilder()
            .withPrimaryKey(123456)
            .withOperationType(WriteRequest.OperationType.INSERT)
            .addIndexValue(vectors)
            .addForwardValue(123)
            .addForwardValue(100.123)
            .build();
    WriteRequest writeRequest;
    try {
      writeRequest = WriteRequest.newBuilder()
              .withCollectionName(collectionName)
              .withForwardColumnList(Arrays.asList("forward1", "forward2"))
              .addIndexColumnMeta("index1", DataType.VECTOR_FP32, dimension)
              .addRow(row1)
              .build();
    } catch (ProximaSEException e) {
      e.printStackTrace();
      return;
    }
    Status status = client.write(writeRequest);
    if (status.ok()) {
      logger.info("============== Write success. ===============");
    } else {
      logger.info("Write failed " + status.toString());
    }

    // search with float array
    float[] features = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
//        float[][] features = {
//                {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f},
//                {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f}
//        };
    QueryRequest queryRequest = QueryRequest.newBuilder()
            .withCollectionName(collectionName)
            .withKnnQueryParam(QueryRequest.KnnQueryParam.newBuilder()
                    .withColumnName("index1")
                    .withTopk(10)
                    .withFeatures(features)
                    .build())
            .build();

    QueryResponse queryResponse = null;
    try {
      queryResponse = client.query(queryRequest);
    } catch (ProximaSEException e) {
      e.printStackTrace();
      return;
    }
    if (queryResponse.ok()) {
      logger.info("================ Query success =====================");
      for (int i = 0; i < queryResponse.getQueryResultCount(); ++i) {
        QueryResult queryResult = queryResponse.getQueryResult(i);
        for (int d = 0; d < queryResult.getDocumentCount(); ++d) {
          Document document = queryResult.getDocument(d);
          logger.info("Doc key: " + document.getPrimaryKey());
          logger.info("Doc score: " + document.getScore());

          Set<String> forwardKeys = document.getForwardKeySet();
          for (String forwardKey : forwardKeys) {
            logger.info(
                    "Doc forward: " + forwardKey + " -> " + document.getForwardValue(forwardKey).getStringValue());
          }
        }
      }
    } else {
      logger.info("Query failed: " + queryResponse.getStatus().toString());
    }

    // search with json matrix
    String matrix = "[[1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0], [1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0]]";
    QueryRequest queryRequest1 = QueryRequest.newBuilder()
            .withCollectionName(collectionName)
            .withKnnQueryParam(QueryRequest.KnnQueryParam.newBuilder()
                    .withColumnName("index1")
                    .withTopk(10)
                    .withFeatures(matrix, DataType.VECTOR_FP32, dimension, 2)
                    .build())
            .build();

    QueryResponse queryResponse1 = null;
    try {
      queryResponse1 = client.query(queryRequest1);
    } catch (ProximaSEException e) {
      e.printStackTrace();
      return;
    }
    if (queryResponse1.ok()) {
      logger.info("================ Query success =====================");
      for (int i = 0; i < queryResponse1.getQueryResultCount(); ++i) {
        QueryResult queryResult = queryResponse1.getQueryResult(i);
        for (int d = 0; d < queryResult.getDocumentCount(); ++d) {
          Document document = queryResult.getDocument(d);
          logger.info("Doc key: " + document.getPrimaryKey());
          logger.info("Doc score: " + document.getScore());

          Set<String> forwardKeys = document.getForwardKeySet();
          for (String forwardKey : forwardKeys) {
            logger.info(
                    "Doc forward: " + forwardKey + " -> " + document.getForwardValue(forwardKey).getStringValue());
          }
        }
      }
    } else {
      logger.info("Query failed: " + queryResponse1.getStatus().toString());
    }

    // Get document by pk
    GetDocumentRequest getRequest = GetDocumentRequest.newBuilder()
            .withCollectionName(collectionName)
            .withPrimaryKey(123456)
            .withDebugMode(true)
            .build();
    GetDocumentResponse getResponse = null;
    try {
      getResponse = client.getDocumentByKey(getRequest);
    } catch (ProximaSEException e) {
      e.printStackTrace();
      return;
    }
    if (getResponse.ok()) {
      logger.info("======================== Get Document success ======================== ");
      Document document = getResponse.getDocument();
      logger.info("PrimaryKey: " + document.getPrimaryKey());
      logger.info("Score: " + document.getScore());
      Set<String> forwardKeys = document.getForwardKeySet();
      for (String forwardKey : forwardKeys) {
        logger.info(
                "Doc forward: " + forwardKey + " -> " + document.getForwardValue(forwardKey).getStringValue());
      }
      logger.info("DebugInfo: " + getResponse.getDebugInfo());
    } else {
      logger.info("Get document failed " + getResponse.getStatus().toString());
    }

    // stats collection
    StatsCollectionResponse statsResponse = client.statsCollection(collectionName);
    if (statsResponse.ok()) {
      logger.info("==================== Stats collection success. ======================");
      CollectionStats stats = statsResponse.getCollectionStats();
      logger.info("CollectionName: " + stats.getCollectionName());
      logger.info("TotalDocCount: " + stats.getTotalDocCount());
      logger.info("TotalSegmentCount: " + stats.getTotalSegmentCount());
    } else {
      logger.info("Stats collection failed " + statsResponse.getStatus().toString());
    }

    // drop collection
    status = client.dropCollection(collectionName);
    if (status.ok()) {
      logger.info("==================== Dorp collection success. ========================");
    } else {
      logger.info("Drop collection failed " + status.toString());
    }

    // close client
    client.close();
  }
}
