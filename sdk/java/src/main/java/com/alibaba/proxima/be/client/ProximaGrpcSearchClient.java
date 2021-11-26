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
 * \brief    Implements all interfaces with ProximaSearchClient
 */

package com.alibaba.proxima.be.client;

import com.alibaba.proxima.be.grpc.CollectionName;
import com.alibaba.proxima.be.grpc.GetVersionRequest;
import com.alibaba.proxima.be.grpc.ProximaServiceGrpc;
import io.grpc.ConnectivityState;
import io.grpc.ManagedChannel;
import io.grpc.ManagedChannelBuilder;
import io.grpc.StatusRuntimeException;

import java.util.List;
import java.util.concurrent.TimeUnit;

/**
 * Proxima Grpc Search Client
 */
public class ProximaGrpcSearchClient implements ProximaSearchClient {
  private ManagedChannel channel;
  private ProximaServiceGrpc.ProximaServiceBlockingStub blockingStub;
  private ConnectParam connectParam;

  /**
   * Constructor ProximaGrpcSearchClient with ConnectParam
   * @param connectParam grpc connecting parameters
   */
  public ProximaGrpcSearchClient(ConnectParam connectParam) {
    this.connectParam = connectParam;
    this.channel = ManagedChannelBuilder.forAddress(connectParam.getHost(), connectParam.getPort())
            .usePlaintext()
            .idleTimeout(connectParam.getIdleTimeout(TimeUnit.NANOSECONDS), TimeUnit.NANOSECONDS)
            .keepAliveTime(connectParam.getKeepAliveTime(TimeUnit.NANOSECONDS), TimeUnit.NANOSECONDS)
            .keepAliveTimeout(connectParam.getKeepAliveTimeout(TimeUnit.NANOSECONDS), TimeUnit.NANOSECONDS)
            .maxInboundMessageSize(Integer.MAX_VALUE)
            .build();
    this.blockingStub = ProximaServiceGrpc.newBlockingStub(this.channel);
    this.checkServerVersion();
  }

  /**
   * Close client
   */
  @Override
  public void close() {
    this.close(10);
  }

  /**
   * Close client
   * @param maxWaitSeconds max wait seconds when close client
   */
  @Override
  public void close(int maxWaitSeconds) {
    this.channel.shutdown();
    long now = System.nanoTime();
    long deadline = now + TimeUnit.SECONDS.toNanos(maxWaitSeconds);
    boolean interrupted = false;
    try {
      while (now < deadline && !channel.isTerminated()) {
        try {
          channel.awaitTermination(deadline - now, TimeUnit.NANOSECONDS);
        } catch (InterruptedException ex) {
          interrupted = true;
        }
      }
      if (!channel.isTerminated()) {
        channel.shutdownNow();
      }
    } finally {
      if (interrupted) {
        Thread.currentThread().interrupt();
      }
    }
  }

  /**
   * Get proxima server version
   * @return GetVersionResponse
   */
  @Override
  public GetVersionResponse getVersion() {
    if (!checkAvailable()) {
      return new GetVersionResponse(Status.ErrorCode.CLIENT_NOT_CONNECTED);
    }
    GetVersionRequest request = GetVersionRequest.newBuilder().build();
    com.alibaba.proxima.be.grpc.GetVersionResponse pbResponse;
    try {
      pbResponse = blockingStub().getVersion(request);
    } catch (StatusRuntimeException e) {
      return new GetVersionResponse(errorCode(e), e.getStatus().toString());
    }

    return ProtoConverter.fromPb(pbResponse);
  }

  /**
   * Create collection with collection config
   * @param collectionConfig the CollectionConfig object
   * @return Status
   */
  @Override
  public Status createCollection(CollectionConfig collectionConfig) {
    if (!checkAvailable()) {
      return new Status(Status.ErrorCode.CLIENT_NOT_CONNECTED);
    }

    this.validate(collectionConfig);

    com.alibaba.proxima.be.grpc.Status status;
    com.alibaba.proxima.be.grpc.CollectionConfig request = ProtoConverter.toPb(collectionConfig);
    try {
      status = blockingStub().createCollection(request);
    } catch (StatusRuntimeException e) {
      return new Status(errorCode(e), e.getStatus().toString());
    }
    return ProtoConverter.fromPb(status);
  }

  /**
   * Drop collection with colleciton name
   * @param collectionName the collection name
   * @return Status
   */
  @Override
  public Status dropCollection(String collectionName) {
    if (!checkAvailable()) {
      return new Status(Status.ErrorCode.CLIENT_NOT_CONNECTED);
    }

    CollectionName request = ProtoConverter.toPb(collectionName);
    com.alibaba.proxima.be.grpc.Status status;
    try {
      status = blockingStub().dropCollection(request);
    } catch (StatusRuntimeException e) {
      return new Status(errorCode(e), e.getStatus().toString());
    }
    return ProtoConverter.fromPb(status);
  }

  /**
   * Describe collection with colleciton name
   * @param collectionName the collection name
   * @return describe collection response
   */
  @Override
  public DescribeCollectionResponse describeCollection(String collectionName) {
    if (!checkAvailable()) {
      return new DescribeCollectionResponse(Status.ErrorCode.CLIENT_NOT_CONNECTED);
    }

    CollectionName request = ProtoConverter.toPb(collectionName);
    com.alibaba.proxima.be.grpc.DescribeCollectionResponse pbResponse;
    try {
      pbResponse = blockingStub().withDeadlineAfter(connectParam.getTimeout(TimeUnit.NANOSECONDS), TimeUnit.NANOSECONDS)
              .describeCollection(request);
    } catch (StatusRuntimeException e) {
      return new DescribeCollectionResponse(errorCode(e), e.getStatus().toString());
    }

    return ProtoConverter.fromPb(pbResponse);
  }

  /**
   * List collections with list condition
   * @param listCondition list collections condition
   * @return list collection response
   */
  @Override
  public ListCollectionsResponse listCollections(ListCondition listCondition) {
    if (!checkAvailable()) {
      return new ListCollectionsResponse(Status.ErrorCode.CLIENT_NOT_CONNECTED);
    }

    com.alibaba.proxima.be.grpc.ListCondition request = ProtoConverter.toPb(listCondition);
    com.alibaba.proxima.be.grpc.ListCollectionsResponse pbResponse;
    try {
      pbResponse = blockingStub().listCollections(request);
    } catch (StatusRuntimeException e) {
      return new ListCollectionsResponse(errorCode(e), e.getStatus().toString());
    }

    return ProtoConverter.fromPb(pbResponse);
  }

  /**
   * Stats collection with collection name
   * @param collectionName the collection name
   * @return stats collection response
   */
  @Override
  public StatsCollectionResponse statsCollection(String collectionName) {
    if (!checkAvailable()) {
      return new StatsCollectionResponse(Status.ErrorCode.CLIENT_NOT_CONNECTED);
    }

    com.alibaba.proxima.be.grpc.CollectionName request = ProtoConverter.toPb(collectionName);
    com.alibaba.proxima.be.grpc.StatsCollectionResponse pbResponse;
    try {
      pbResponse = blockingStub().statsCollection(request);
    } catch (StatusRuntimeException e) {
      return new StatsCollectionResponse(errorCode(e), e.getStatus().toString());
    }

    return ProtoConverter.fromPb(pbResponse);
  }

  /**
   * Write records for collection
   * @param request write request included insert/update/delete operations
   * @return Status
   */
  @Override
  public Status write(WriteRequest request) {
    if (!checkAvailable()) {
      return new Status(Status.ErrorCode.CLIENT_NOT_CONNECTED);
    }

    this.validate(request);

    com.alibaba.proxima.be.grpc.WriteRequest pbRequest = ProtoConverter.toPb(request);
    com.alibaba.proxima.be.grpc.Status pbResponse;
    try {
      pbResponse = blockingStub().write(pbRequest);
    } catch (StatusRuntimeException e) {
      return new Status(errorCode(e), e.getStatus().toString());
    }

    return ProtoConverter.fromPb(pbResponse);
  }

  /**
   * Query records with specified features
   * @param request query request included querying featuress
   * @return QueryResponse included query result
   */
  @Override
  public QueryResponse query(QueryRequest request) {
    if (!checkAvailable()) {
      return new QueryResponse(Status.ErrorCode.CLIENT_NOT_CONNECTED);
    }

    this.validate(request);

    com.alibaba.proxima.be.grpc.QueryRequest pbRequest = ProtoConverter.toPb(request);
    com.alibaba.proxima.be.grpc.QueryResponse pbResponse;
    try {
      pbResponse = blockingStub().query(pbRequest);
    } catch (StatusRuntimeException e) {
      return new QueryResponse(errorCode(e), e.getStatus().toString());
    }

    return ProtoConverter.fromPb(pbResponse);
  }

  /**
   * Get document by primary key
   * @param request the GetDocumentRequest object included collection name and primary key
   * @return GetDocumentResponse
   */
  @Override
  public GetDocumentResponse getDocumentByKey(GetDocumentRequest request) {
    if (!checkAvailable()) {
      return new GetDocumentResponse(Status.ErrorCode.CLIENT_NOT_CONNECTED);
    }

    this.validate(request);

    com.alibaba.proxima.be.grpc.GetDocumentRequest pbRequest = ProtoConverter.toPb(request);
    com.alibaba.proxima.be.grpc.GetDocumentResponse pbResponse;
    try {
      pbResponse = blockingStub().getDocumentByKey(pbRequest);
    } catch (StatusRuntimeException e) {
      return new GetDocumentResponse(errorCode(e), e.getStatus().toString());
    }

    return ProtoConverter.fromPb(pbResponse);
  }

  private boolean checkAvailable() {
    ConnectivityState state = this.channel.getState(false);
    switch (state) {
      case IDLE:
      case READY:
      case CONNECTING:
        return true;
      default:
        return false;
    }
  }

  private void validate(CollectionConfig config) throws ProximaSEException {
    if (config.getCollectionName() == null || config.getCollectionName().isEmpty()) {
      throw new ProximaSEException("Collection name is empty.");
    }

    List<IndexColumnParam> indexParamList = config.getIndexColumnParams();
    if (indexParamList == null || indexParamList.isEmpty()) {
      throw new ProximaSEException("Index Column params is empty.");
    }
    for (IndexColumnParam indexParam : indexParamList) {
      if (indexParam.getColumnName() == null || indexParam.getColumnName().isEmpty()) {
        throw new ProximaSEException("Index column name is empty.");
      }
      if (indexParam.getIndexType() != IndexType.PROXIMA_GRAPH_INDEX) {
        throw new ProximaSEException("Index type is invalid.");
      }
      if (indexParam.getDataType() == DataType.UNDEFINED) {
        throw new ProximaSEException("Index data type is undefined.");
      }
      if (indexParam.getDimension() <= 0) {
        throw new ProximaSEException("Index dimension should > 0");
      }
    }

    DatabaseRepository dbRepo = config.getDatabaseRepository();
    if (dbRepo != null) {
      if (dbRepo.getRepositoryName() == null || dbRepo.getRepositoryName().isEmpty()) {
        throw new ProximaSEException("Repository name is empty.");
      }
      if (dbRepo.getConnectionUri() == null || dbRepo.getConnectionUri().isEmpty()) {
        throw new ProximaSEException("Connection uri is empty.");
      }
      if (dbRepo.getTableName() == null || dbRepo.getTableName().isEmpty()) {
        throw new ProximaSEException("Table name is empty.");
      }
      if (dbRepo.getUser() == null || dbRepo.getUser().isEmpty()) {
        throw new ProximaSEException("User name is empty.");
      }
      if (dbRepo.getPassword() == null || dbRepo.getPassword().isEmpty()) {
        throw new ProximaSEException("Password is empty.");
      }
    }
  }

  private void validate(WriteRequest request) throws ProximaSEException {
    if (request.getCollectionName() == null || request.getCollectionName().isEmpty()) {
      throw new ProximaSEException("Collection name is empty.");
    }
    WriteRequest.RowMeta rowMeta = request.getRowMeta();
    if (rowMeta == null) {
      throw new ProximaSEException("RowMeta is empty.");
    }
    if (rowMeta.getIndexColumnMetas() == null || rowMeta.getIndexColumnMetas().isEmpty()) {
      throw new ProximaSEException("Index column metas is empty in RowMeta.");
    }

    List<WriteRequest.Row> rows = request.getRows();
    if (rows == null || rows.isEmpty()) {
      throw new ProximaSEException("Rows is empty.");
    }
    for (WriteRequest.Row row : rows) {
      if(row.getOperationType() != WriteRequest.OperationType.DELETE){
        if (row.getIndexValues() == null || row.getIndexValues().getValuesCount() == 0) {
          throw new ProximaSEException("Index column values is empty in Row.");
        }
      }
    }
  }

  private void validate(QueryRequest request) throws ProximaSEException {
    if (request.getQueryType() != QueryRequest.QueryType.KNN) {
      throw new ProximaSEException("Query type is invalid.");
    }
    if (request.getCollectionName() == null || request.getCollectionName().isEmpty()) {
      throw new ProximaSEException("Collection name is empty.");
    }
    QueryRequest.KnnQueryParam knnQueryParam = request.getKnnQueryParam();
    if (knnQueryParam == null) {
      throw new ProximaSEException("Knn query param is empty.");
    }
    if (knnQueryParam.getColumnName() == null || knnQueryParam.getColumnName().isEmpty()) {
      throw new ProximaSEException("Column name is empty in KnnQueryParam.");
    }
    if (knnQueryParam.getTopk() <= 0) {
      throw new ProximaSEException("Topk should > 0 in KnnQueryParam");
    }
    if ((knnQueryParam.getFeatures() == null || knnQueryParam.getFeatures().length == 0) &&
            (knnQueryParam.getMatrix() == null || knnQueryParam.getMatrix().length() == 0)) {
      throw new ProximaSEException("Features is empty in KnnQueryParam");
    }
    if (knnQueryParam.getBatchCount() <= 0) {
      throw new ProximaSEException("Batch count should > 0 in KnnQueryParam");
    }
    if (knnQueryParam.getDimension() <= 0) {
      throw new ProximaSEException("Dimension should > 0 in KnnQueryParam");
    }
  }

  private void validate(GetDocumentRequest request) throws ProximaSEException {
    if (request.getCollectionName() == null || request.getCollectionName().isEmpty()) {
      throw new ProximaSEException("Collection name is empty.");
    }
  }

  private ProximaServiceGrpc.ProximaServiceBlockingStub blockingStub() {
    return this.blockingStub.withDeadlineAfter(
            this.connectParam.getTimeout(TimeUnit.MILLISECONDS), TimeUnit.MILLISECONDS);
  }

  private void checkServerVersion() throws ProximaSEException {
    String clientVersion = this.getClientVersion();
    if (clientVersion == null || clientVersion.isEmpty()) {
      throw new ProximaSEException("Java SDK client version is empty.");
    }
    GetVersionResponse response = this.getVersion();
    if (!response.ok()) {
      throw new ProximaSEException("Get server version failed: " + response.getStatus().getReason());
    }
    String serverVersion = response.getVersion();

    String[] clientArray = clientVersion.split("\\.");
    String[] serverArray = serverVersion.split("\\.");
    if (clientArray.length != 3) {
      throw new ProximaSEException("Get client version failed.");
    }
    if (serverArray.length < 3) {
      throw new ProximaSEException("Server version is invalid.");
    }
    if (clientArray[0].compareTo(serverArray[0]) != 0 || clientArray[1].compareTo(serverArray[1]) != 0) {
      throw new ProximaSEException("Client and server version mismatched.");
    }
  }

  private Status.ErrorCode errorCode(StatusRuntimeException e) {
    io.grpc.Status status = e.getStatus();
    if (status == null) {
      return Status.ErrorCode.RPC_ERROR;
    }
    if (status.getCode() == io.grpc.Status.Code.DEADLINE_EXCEEDED) {
      return Status.ErrorCode.RPC_TIMEOUT;
    } else {
      return Status.ErrorCode.RPC_ERROR;
    }
  }
}
