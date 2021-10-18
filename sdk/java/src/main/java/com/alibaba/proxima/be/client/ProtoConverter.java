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
 * \brief    Convert between proto and inner struct
 */

package com.alibaba.proxima.be.client;


import com.google.protobuf.ByteString;
import com.alibaba.proxima.be.grpc.KeyValuePair;
import com.alibaba.proxima.be.grpc.CollectionName;
import com.alibaba.proxima.be.grpc.OperationType;
import com.alibaba.proxima.be.grpc.GenericKeyValue;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * Convert between proto object and inner object
 */
public class ProtoConverter {
  public static com.alibaba.proxima.be.grpc.DataType toPb(DataType dataType) {
    return com.alibaba.proxima.be.grpc.DataType.forNumber(dataType.getValue());
  }

  public static com.alibaba.proxima.be.grpc.IndexType toPb(IndexType indexType) {
    return com.alibaba.proxima.be.grpc.IndexType.forNumber(indexType.getValue());
  }

  public static com.alibaba.proxima.be.grpc.CollectionConfig toPb(CollectionConfig config) {
    List<IndexColumnParam> indexParams = config.getIndexColumnParams();
    List<com.alibaba.proxima.be.grpc.CollectionConfig.IndexColumnParam> indexParamList = new ArrayList<>();
    for (IndexColumnParam indexParam : indexParams) {
      Map<String, String> extraParams = indexParam.getExtraParams();
      List<KeyValuePair> extraParamList = new ArrayList<>();
      for (Map.Entry<String, String> entry : extraParams.entrySet()) {
        extraParamList.add(
                KeyValuePair.newBuilder().setKey(entry.getKey()).setValue(entry.getValue()).build());
      }
      indexParamList.add(
              com.alibaba.proxima.be.grpc.CollectionConfig.IndexColumnParam.newBuilder()
                      .setColumnName(indexParam.getColumnName())
                      .setDataType(toPb(indexParam.getDataType()))
                      .setIndexType(toPb(indexParam.getIndexType()))
                      .setDimension(indexParam.getDimension())
                      .addAllExtraParams(extraParamList)
                      .build()
      );
    }
    com.alibaba.proxima.be.grpc.CollectionConfig.Builder builder =
            com.alibaba.proxima.be.grpc.CollectionConfig.newBuilder()
                    .setCollectionName(config.getCollectionName())
                    .setMaxDocsPerSegment(config.getMaxDocsPerSegment())
                    .addAllForwardColumnNames(config.getForwardColumnNames())
                    .addAllIndexColumnParams(indexParamList);

    DatabaseRepository databaseRepository = config.getDatabaseRepository();
    if (databaseRepository != null) {
      com.alibaba.proxima.be.grpc.CollectionConfig.RepositoryConfig.Database database =
              com.alibaba.proxima.be.grpc.CollectionConfig.RepositoryConfig.Database.newBuilder()
                      .setConnectionUri(databaseRepository.getConnectionUri())
                      .setTableName(databaseRepository.getTableName())
                      .setUser(databaseRepository.getUser())
                      .setPassword(databaseRepository.getPassword())
                      .build();
      com.alibaba.proxima.be.grpc.CollectionConfig.RepositoryConfig repositoryConfig =
              com.alibaba.proxima.be.grpc.CollectionConfig.RepositoryConfig.newBuilder()
                      .setRepositoryType(com.alibaba.proxima.be.grpc.CollectionConfig
                              .RepositoryConfig.RepositoryType.RT_DATABASE)
                      .setRepositoryName(databaseRepository.getRepositoryName())
                      .setDatabase(database)
                      .build();
      builder.setRepositoryConfig(repositoryConfig);
    }

    return builder.build();
  }

  public static CollectionName toPb(String collectionName) {
    return CollectionName.newBuilder()
            .setCollectionName(collectionName)
            .build();
  }

  public static com.alibaba.proxima.be.grpc.ListCondition toPb(ListCondition listCondition) {
    if (listCondition == null || listCondition.getRepositoryName() == null) {
      return com.alibaba.proxima.be.grpc.ListCondition.newBuilder().build();
    }
    return com.alibaba.proxima.be.grpc.ListCondition.newBuilder()
            .setRepositoryName(listCondition.getRepositoryName())
            .build();
  }

  public static com.alibaba.proxima.be.grpc.WriteRequest toPb(WriteRequest request) {
    return com.alibaba.proxima.be.grpc.WriteRequest.newBuilder()
            .setCollectionName(request.getCollectionName())
            .setRowMeta(toPb(request.getRowMeta()))
            .addAllRows(toPb(request.getRows()))
            .setRequestId(request.getRequestId())
            .setMagicNumber(request.getMagicNumber())
            .build();
  }

  public static com.alibaba.proxima.be.grpc.QueryRequest toPb(QueryRequest request) {
    return com.alibaba.proxima.be.grpc.QueryRequest.newBuilder()
            .setQueryType(
                    com.alibaba.proxima.be.grpc.QueryRequest.QueryType.forNumber(request.getQueryType().getValue()))
            .setCollectionName(request.getCollectionName())
            .setDebugMode(request.isDebugMode())
            .setKnnParam(toPb(request.getKnnQueryParam()))
            .build();
  }

  public static com.alibaba.proxima.be.grpc.GetDocumentRequest toPb(GetDocumentRequest request) {
    return com.alibaba.proxima.be.grpc.GetDocumentRequest.newBuilder()
            .setCollectionName(request.getCollectionName())
            .setPrimaryKey(request.getPrimaryKey())
            .setDebugMode(request.isDebugMode())
            .build();
  }

  public static Status fromPb(com.alibaba.proxima.be.grpc.Status status) {
    return new Status(status.getCode(), status.getReason());
  }

  public static ListCollectionsResponse fromPb(com.alibaba.proxima.be.grpc.ListCollectionsResponse pbResponse) {
    Status status = new Status(pbResponse.getStatus().getCode(), pbResponse.getStatus().getReason());
    List<CollectionInfo> collectionInfoList = new ArrayList<>();
    for (int i = 0; i < pbResponse.getCollectionsCount(); ++i) {
      collectionInfoList.add(fromPb(pbResponse.getCollections(i)));
    }
    return new ListCollectionsResponse(status, collectionInfoList);
  }

  public static DescribeCollectionResponse fromPb(com.alibaba.proxima.be.grpc.DescribeCollectionResponse pbResponse) {
    Status status = new Status(pbResponse.getStatus().getCode(), pbResponse.getStatus().getReason());
    return new DescribeCollectionResponse(status, fromPb(pbResponse.getCollection()));
  }

  public static StatsCollectionResponse fromPb(com.alibaba.proxima.be.grpc.StatsCollectionResponse pbResponse) {
    Status status = new Status(pbResponse.getStatus().getCode(), pbResponse.getStatus().getReason());
    return new StatsCollectionResponse(status, fromPb(pbResponse.getCollectionStats()));
  }

  public static QueryResponse fromPb(com.alibaba.proxima.be.grpc.QueryResponse pbResponse) {
    Status status = new Status(pbResponse.getStatus().getCode(), pbResponse.getStatus().getReason());
    return new QueryResponse(status, pbResponse.getLatencyUs(),
            pbResponse.getDebugInfo(), fromPb(pbResponse.getResultsList()));
  }

  public static GetDocumentResponse fromPb(com.alibaba.proxima.be.grpc.GetDocumentResponse pbResponse) {
    Status status = new Status(pbResponse.getStatus().getCode(), pbResponse.getStatus().getReason());
    Document document = fromPb(pbResponse.getDocument());
    return new GetDocumentResponse(status, pbResponse.getDebugInfo(), document);
  }

  public static GetVersionResponse fromPb(com.alibaba.proxima.be.grpc.GetVersionResponse pbResponse) {
    return new GetVersionResponse(fromPb(pbResponse.getStatus()), pbResponse.getVersion());
  }

  private static com.alibaba.proxima.be.grpc.QueryRequest.KnnQueryParam toPb(QueryRequest.KnnQueryParam queryParam) {

    com.alibaba.proxima.be.grpc.QueryRequest.KnnQueryParam.Builder builder =
            com.alibaba.proxima.be.grpc.QueryRequest.KnnQueryParam.newBuilder()
                    .setColumnName(queryParam.getColumnName())
                    .setTopk(queryParam.getTopk())
                    .setBatchCount(queryParam.getBatchCount())
                    .setDimension(queryParam.getDimension())
                    .setDataType(com.alibaba.proxima.be.grpc.DataType.forNumber(queryParam.getDataType().getValue()))
                    .setRadius(queryParam.getRadius())
                    .setIsLinear(queryParam.isLinear());
    if (queryParam.getFeatures() != null) {
      builder.setFeatures(ByteString.copyFrom(queryParam.getFeatures()));
    } else {
      builder.setMatrix(queryParam.getMatrix());
    }
    List<KeyValuePair> extraParamList = new ArrayList<>();
    Map<String, String> extraParams = queryParam.getExtraParams();
    for (Map.Entry<String, String> entry : extraParams.entrySet()) {
      extraParamList.add(
              KeyValuePair.newBuilder()
                      .setKey(entry.getKey())
                      .setValue(entry.getValue())
                      .build()
      );
    }
    builder.addAllExtraParams(extraParamList);

    return builder.build();
  }

  private static com.alibaba.proxima.be.grpc.WriteRequest.RowMeta toPb(WriteRequest.RowMeta rowMeta) {
    List<com.alibaba.proxima.be.grpc.WriteRequest.IndexColumnMeta> indexColumnMetaList = new ArrayList<>();
    List<WriteRequest.IndexColumnMeta> indexColumnMetas = rowMeta.getIndexColumnMetas();
    for (int i = 0; i < indexColumnMetas.size(); ++i) {
      WriteRequest.IndexColumnMeta columnMeta = indexColumnMetas.get(i);
      indexColumnMetaList.add(com.alibaba.proxima.be.grpc.WriteRequest.IndexColumnMeta.newBuilder()
              .setColumnName(columnMeta.getColumnName())
              .setDataType(com.alibaba.proxima.be.grpc.DataType.forNumber(columnMeta.getDataType().getValue()))
              .setDimension(columnMeta.getDimension())
              .build());
    }
    return com.alibaba.proxima.be.grpc.WriteRequest.RowMeta.newBuilder()
            .addAllForwardColumnNames(rowMeta.getForwardColumnNames())
            .addAllIndexColumnMetas(indexColumnMetaList)
            .build();
  }

  private static List<com.alibaba.proxima.be.grpc.WriteRequest.Row> toPb(List<WriteRequest.Row> rows) {
    List<com.alibaba.proxima.be.grpc.WriteRequest.Row> pbRowList = new ArrayList<>();
    for (int i = 0; i < rows.size(); ++i) {
      WriteRequest.Row row = rows.get(i);
      com.alibaba.proxima.be.grpc.WriteRequest.Row.Builder builder =
              com.alibaba.proxima.be.grpc.WriteRequest.Row.newBuilder()
                      .setPrimaryKey(row.getPrimaryKey())
                      .setOperationType(OperationType.forNumber(row.getOperationType().getValue()))
                      .setIndexColumnValues(row.getIndexValues())
                      .setForwardColumnValues(row.getForwardValues());
      LsnContext lsnContext = row.getLsnContext();
      if (lsnContext != null) {
        builder.setLsnContext(
                com.alibaba.proxima.be.grpc.LsnContext.newBuilder()
                        .setLsn(lsnContext.getLsn())
                        .setContext(lsnContext.getContext())
                        .build());
      }
      pbRowList.add(builder.build());
    }
    return pbRowList;
  }

  private static List<QueryResult> fromPb(List<com.alibaba.proxima.be.grpc.QueryResponse.Result> pbResults) {
    List<QueryResult> queryResultList = new ArrayList<>();
    if (pbResults == null) {
      return queryResultList;
    }
    for (int i = 0; i < pbResults.size(); ++i) {
      queryResultList.add(fromPb(pbResults.get(i)));
    }
    return queryResultList;
  }

  private static QueryResult fromPb(com.alibaba.proxima.be.grpc.QueryResponse.Result pbResult) {
    QueryResult.Builder builder = new QueryResult.Builder();
    for (int i = 0; i < pbResult.getDocumentsCount(); ++i) {
      builder.addDocument(fromPb(pbResult.getDocuments(i)));
    }
    return builder.build();
  }

  private static Document fromPb(com.alibaba.proxima.be.grpc.Document pbDocument) {
    if (pbDocument == null) {
      return null;
    }
    Map<String, Document.ForwardValue> forwardMap = new HashMap<>();
    for (int i = 0; i < pbDocument.getForwardColumnValuesCount(); ++i) {
      GenericKeyValue keyValue = pbDocument.getForwardColumnValues(i);
      forwardMap.put(keyValue.getKey(),
              new Document.ForwardValue.Builder(keyValue.getValue()).build());
    }
    return new Document.Builder()
            .withPrimaryKey(pbDocument.getPrimaryKey())
            .withScore(pbDocument.getScore())
            .withForwardColumnMap(forwardMap)
            .build();
  }

  private static CollectionStats fromPb(com.alibaba.proxima.be.grpc.CollectionStats pbStats) {
    CollectionStats.Builder builder = new CollectionStats.Builder()
            .withCollectionName(pbStats.getCollectionName())
            .withCollectionPath(pbStats.getCollectionPath())
            .withTotalDocCount(pbStats.getTotalDocCount())
            .withTotalIndexFileCount(pbStats.getTotalIndexFileCount())
            .withTotalIndexFileSize(pbStats.getTotalIndexFileSize())
            .withTotalSegmentCount(pbStats.getTotalSegmentCount());
    for (int i = 0; i < pbStats.getSegmentStatsCount(); ++i) {
      builder.addSegmentStats(fromPb(pbStats.getSegmentStats(i)));
    }
    return builder.build();
  }

  private static CollectionStats.SegmentStats fromPb(
          com.alibaba.proxima.be.grpc.CollectionStats.SegmentStats pbStats) {
    if (pbStats == null) {
      return null;
    }
    return new CollectionStats.SegmentStats.Builder()
            .withSegmentId(pbStats.getSegmentId())
            .withSegmentState(CollectionStats.SegmentState.valueOf(pbStats.getState().getNumber()))
            .withDocDount(pbStats.getDocCount())
            .withIndexFileCount(pbStats.getIndexFileCount())
            .withIndexFileSize(pbStats.getIndexFileSize())
            .withMinDocId(pbStats.getMinDocId())
            .withMaxDocId(pbStats.getMaxDocId())
            .withMinDocId(pbStats.getMinDocId())
            .withMinPrimaryKey(pbStats.getMinPrimaryKey())
            .withMaxPrimaryKey(pbStats.getMaxPrimaryKey())
            .withMinTimestamp(pbStats.getMinTimestamp())
            .withMaxTimestamp(pbStats.getMaxTimestamp())
            .withMinLsn(pbStats.getMinLsn())
            .withMaxLsn(pbStats.getMaxLsn())
            .withSegmentPath(pbStats.getSegmentPath())
            .build();
  }

  private static CollectionInfo fromPb(com.alibaba.proxima.be.grpc.CollectionInfo pbInfo) {
    if (pbInfo == null) {
      return null;
    }
    CollectionInfo.Builder builder = new CollectionInfo.Builder()
            .withCollectionConfig(fromPb(pbInfo.getConfig()))
            .withCollectionStatus(fromPb(pbInfo.getStatus()))
            .withUuid(pbInfo.getUuid())
            .withMagicNumber(pbInfo.getMagicNumber());
    if (pbInfo.hasLatestLsnContext()) {
      com.alibaba.proxima.be.grpc.LsnContext lsnContext = pbInfo.getLatestLsnContext();
      builder.withLatestLsnContext(
              LsnContext.newBuilder()
                      .withLsn(lsnContext.getLsn())
                      .withContext(lsnContext.getContext())
                      .build());
    }
    return builder.build();
  }

  private static CollectionConfig fromPb(com.alibaba.proxima.be.grpc.CollectionConfig config) {
    List<com.alibaba.proxima.be.grpc.CollectionConfig.IndexColumnParam> columnParams =
            config.getIndexColumnParamsList();
    List<IndexColumnParam> columnParamList = new ArrayList<>();
    for (int i = 0; i < columnParams.size(); ++i) {
      columnParamList.add(fromPb(columnParams.get(i)));
    }
    CollectionConfig.Builder builder = new CollectionConfig.Builder()
            .withCollectionName(config.getCollectionName())
            .withMaxDocsPerSegment(config.getMaxDocsPerSegment())
            .withForwardColumnNames(config.getForwardColumnNamesList())
            .withIndexColumnParams(columnParamList);
    if (config.hasRepositoryConfig()) {
      builder.withDatabaseRepository(fromPb(config.getRepositoryConfig()));
    }
    return builder.build();
  }

  private static IndexColumnParam fromPb(
          com.alibaba.proxima.be.grpc.CollectionConfig.IndexColumnParam indexParam) {
    if (indexParam == null) {
      return null;
    }
    Map<String, String> extraParamMap = new HashMap<>();
    for (int i = 0; i < indexParam.getExtraParamsCount(); ++i) {
      KeyValuePair keyValuePair = indexParam.getExtraParams(i);
      extraParamMap.put(keyValuePair.getKey(), keyValuePair.getValue());
    }
    return new IndexColumnParam.Builder(
            indexParam.getColumnName(),
            DataType.valueOf(indexParam.getDataType().getNumber()),
            indexParam.getDimension())
            .withIndexType(IndexType.valueOf(indexParam.getIndexType().getNumber()))
            .build();
  }

  private static DatabaseRepository fromPb(
          com.alibaba.proxima.be.grpc.CollectionConfig.RepositoryConfig config) {
    return new DatabaseRepository.Builder()
            .withConnectionUri(config.getDatabase().getConnectionUri())
            .withPassword(config.getDatabase().getPassword())
            .withUser(config.getDatabase().getUser())
            .withTableName(config.getDatabase().getTableName())
            .withRepositoryName(config.getRepositoryName())
            .build();
  }

  private static CollectionInfo.CollectionStatus fromPb(
          com.alibaba.proxima.be.grpc.CollectionInfo.CollectionStatus status) {
    return CollectionInfo.CollectionStatus.valueOf(status.getNumber());
  }
}
