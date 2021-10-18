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
 * \brief    Bench client for java sdk
 */

package com.alibaba.example;

import com.alibaba.proxima.be.client.*;
import io.grpc.internal.JsonParser;
import io.grpc.internal.JsonUtil;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.*;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.atomic.AtomicInteger;

public class BenchClient {
    private static Logger logger = LoggerFactory.getLogger(BenchClient.class);

    private Map<String, String> kv;
    private String collection;
    private List<ProximaSearchClient> clientList;
    private long[] keys;
    private byte[][] features;
    private String columnName;
    private int dimension = 512;
    private int topk = 10;

    public static class ThreadWorker implements Runnable {
        private WorkerType workerType;
        private ProximaSearchClient client;
        private String collection;
        private String columnName;
        private int dimension;
        private int topk;
        private long[] keys;
        private byte[][] features;
        private int start;
        private int end;
        private AtomicInteger index;

        public enum WorkerType {
            WT_INSERT,
            WT_SEARCH
        }

        public ThreadWorker(WorkerType workerType, ProximaSearchClient client, String collection,
                            String columnName, int dimension, long[] keys, byte[][] features, int start, int end,
                            AtomicInteger index) {
            this.workerType = workerType;
            this.client = client;
            this.collection = collection;
            this.columnName = columnName;
            this.dimension = dimension;
            this.keys = keys;
            this.features = features;
            this.start = start;
            this.end = end;
            this.index = index;
        }

        public ThreadWorker(WorkerType workerType, ProximaSearchClient client, String collection,
                            String columnName, int dimension, int topk, long[] keys, byte[][] features, int start, int end,
                            AtomicInteger index) {
            this.workerType = workerType;
            this.client = client;
            this.collection = collection;
            this.columnName = columnName;
            this.topk = topk;
            this.dimension = dimension;
            this.keys = keys;
            this.features = features;
            this.start = start;
            this.end = end;
            this.index = index;
        }

        private void insert() {
            int i = index.getAndIncrement();
            int total = features.length;
            while (i < total) {
            // for (int i = start; i < end; ++i) {
                WriteRequest request = WriteRequest.newBuilder()
                        .withCollectionName(collection)
                        .addIndexColumnMeta(columnName, DataType.VECTOR_FP32, dimension)
                        .addRow(WriteRequest.Row.newBuilder()
                                .withPrimaryKey(keys[i])
                                .withOperationType(WriteRequest.OperationType.INSERT)
                                .addIndexValue(features[i])
                                .build())
                        .build();
                try {
                    Status status = client.write(request);
                    if (!status.ok()) {
                        logger.error("Write failed " + status.toString());
                    }
                } catch (ProximaSEException e) {
                    e.printStackTrace();
                }
                if (i % 10000 == 0) {
                    logger.info("processed " + i);
                }
                i = index.getAndIncrement();
            }
        }

        private void search() {
            int i = index.getAndIncrement();
            int total = features.length;
            while (i < total) {
//            for (int i = start; i < end; ++i) {
                QueryRequest request = QueryRequest.newBuilder()
                        .withCollectionName(collection)
                        .withKnnQueryParam(QueryRequest.KnnQueryParam.newBuilder()
                                .withColumnName(columnName)
                                .withTopk(topk)
                                .withFeatures(features[i], DataType.VECTOR_FP32, dimension, 1)
                                .build())
                        .build();
                try {
                    QueryResponse response = client.query(request);
                    if (!response.ok()) {
                        logger.error("Query failed " + response.getStatus().toString());
                    }
                } catch (ProximaSEException e) {
                    e.printStackTrace();
                }
                i = index.getAndIncrement();
            }
        }

        public void run() {
            if (workerType == WorkerType.WT_INSERT) {
                insert();
            } else if (workerType == WorkerType.WT_SEARCH) {
                search();
            }
        }
    }
    public BenchClient(Map<String, String> kv) {
        this.kv = kv;
        clientList = new ArrayList<ProximaSearchClient>();
    }

    public boolean init() {
        if (!kv.containsKey("address")) {
            logger.error("address no specified.");
            return false;
        }
        String address = kv.get("address");
        String[] array = address.split(":");
        if (array.length != 2) {
            logger.error("invalid address.");
            return false;
        }
        String host = array[0];
        int port = Integer.valueOf(array[1]);
        ConnectParam connectParam = ConnectParam.newBuilder()
                .withHost(host)
                .withPort(port)
                .build();
        int concurrency = 1;
        if (kv.containsKey("concurrency")) {
            concurrency = Integer.valueOf(kv.get("concurrency"));
        }
        for (int i = 0; i < concurrency; ++i) {
            clientList.add(new ProximaGrpcSearchClient(connectParam));
        }

        if (!kv.containsKey("collection")) {
            logger.error("collection no specified.");
            return false;
        }
        collection = kv.get("collection");

        if (kv.containsKey("column")) {
            columnName = kv.get("column");
        }
        if (!kv.containsKey("command")) {
            logger.error("command no specified.");
            return false;
        }

        return true;
    }

    public boolean execute() {
        String command = kv.get("command");
        if (command.equals("create")) {
            return createCollection();
        } else if (command.equals("drop")) {
            dropCollection();
        } else if (command.equals("describe")) {
            describeCollection();
        } else if (command.equals("stats")) {
            statsCollection();
        } else if (command.equals("insert")) {
            insertCollection();
        } else if (command.equals("search")) {
            searchCollection();
        } else {
            logger.error("invalid command " + command);
            return false;
        }

        return true;
    }

    public boolean createCollection() {
        String schema = kv.get("schema");
        Object object;
        try {
            object = JsonParser.parse(schema);
        } catch (IOException e) {
            e.printStackTrace();
            return false;
        }

        if (!(object instanceof Map)) {
            throw new RuntimeException("invalid schema");
        }
        Map<String, ?> kvMap = (Map<String, ?>)object;
        List<Object> indexColumnParams = (List<Object>)kvMap.get("index_column_params");
        List<IndexColumnParam> indexList = new ArrayList<>();
        for (int i = 0; i < indexColumnParams.size(); ++i) {
            Map<String, ?> paramObj = (Map<String, ?>)indexColumnParams.get(i);
            indexList.add(IndexColumnParam.newBuilder()
                    .withColumnName((String)paramObj.get("column_name"))
                    .withDataType(DataType.valueOf((JsonUtil.getNumberAsInteger(paramObj,"data_type")).intValue()))
                    .withDimension((JsonUtil.getNumberAsInteger(paramObj,"dimension")).intValue())
                    .build());
        }

        CollectionConfig config = CollectionConfig.newBuilder()
                .withCollectionName(collection)
                .withIndexColumnParams(indexList)
                .withMaxDocsPerSegment(0)
                .build();

        try {
            Status status = clientList.get(0).createCollection(config);
            if (status.ok()) {
                logger.info("Create collection success.");
                return true;
            } else {
                logger.info("Create collection failed " + status.toString());
                return false;
            }
        } catch (ProximaSEException e) {
            e.printStackTrace();
            return false;
        }
    }

    public void dropCollection() {
        Status status = clientList.get(0).dropCollection(collection);
        if (status.ok()) {
            logger.info("Drop collection success.");
        } else {
            logger.info("Drop collection failed " + status.toString());
        }
    }

    public void describeCollection() {
        DescribeCollectionResponse response = clientList.get(0).describeCollection(collection);
        if (response.ok()) {
            logger.info("Describe collection success.");
            CollectionConfig config = response.getCollectionInfo().getCollectionConfig();
            List<IndexColumnParam> indexColumnParams = config.getIndexColumnParams();
            for (int i = 0; i < indexColumnParams.size(); ++i) {
                logger.info("IndexColumnName: " + indexColumnParams.get(i).getColumnName());
                logger.info("DataType: " + indexColumnParams.get(i).getDataType().name());
                logger.info("Dimension: " + indexColumnParams.get(i).getDimension());
            }
            logger.info("ForwardColumnNames: " + config.getForwardColumnNames());
        } else {
            logger.error("Describe collection failed " + response.getStatus().toString());
        }
    }

    public void statsCollection() {
        StatsCollectionResponse response = clientList.get(0).statsCollection(collection);
        if (response.ok()) {
            logger.info("Stats collection success.");
            CollectionStats stats = response.getCollectionStats();
            logger.info("TotalDocCount: " + stats.getTotalDocCount());
            logger.info("SegmentCount: " + stats.getSegmentStatsCount());
        } else {
            logger.error("Describe collection failed " + response.getStatus().toString());
        }
    }

    private boolean loadTxtFile(String fileName) {
        File file = new File(fileName);
        BufferedReader reader = null;
        List<List<Float>> vectorArray = new ArrayList<List<Float>>();
        List<Long> vectorKeys = new ArrayList<Long>();
        try {
            reader = new BufferedReader(new FileReader(fileName));
            String tempStr;
            while ((tempStr = reader.readLine()) != null) {
                String[] array = tempStr.split(";");
                String[] vector;
                if (array.length == 2) {
                    vector = array[1].split(" ");
                    vectorKeys.add(Long.valueOf(array[0]));
                } else {
                    continue;
                }
                List<Float> floatVector = new ArrayList<>();
                for (int i = 0; i < vector.length; ++i) {
                    floatVector.add(Float.valueOf(vector[i]));
                }
                vectorArray.add(floatVector);
            }
            reader.close();
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            if (reader != null) {
                try {
                    reader.close();
                } catch (IOException e1) {
                    e1.printStackTrace();
                }
            }
        }

        this.features = new byte[vectorArray.size()][];
        this.keys = new long[vectorArray.size()];
        for (int i = 0; i < vectorArray.size(); ++i) {
            List<Float> tmpArray = vectorArray.get(i);
            ByteBuffer bf = ByteBuffer.allocate(dimension * 4).order(ByteOrder.LITTLE_ENDIAN);
            FloatBuffer floatBuffer = bf.asFloatBuffer();
            for (int j = 0; j < dimension; ++j) {
                floatBuffer.put(tmpArray.get(j));
            }
            this.features[i] = bf.array();
            this.keys[i] = vectorKeys.get(i).longValue();
        }

        return true;
    }

    private boolean loadVecs2File(String fileName) {
        File file = new File(fileName);
        FileInputStream stream;
        try {
            stream = new FileInputStream(file);
            byte[] vecsHeaderBytes = new byte[12];
            int bytes = stream.read(vecsHeaderBytes);
            if (bytes != 12) {
                logger.error("invalid vecs file.");
                return false;
            }
            long totalCount = 0;
            for (int i = 0; i < 8; ++i) {
                totalCount += ((long)(vecsHeaderBytes[i] & 0xFF) << (i * 8));
            }
            int metaSize = 0;
            for (int i = 8; i < 12; ++i) {
                metaSize += ((vecsHeaderBytes[i] & 0xFF) << ((i - 8) * 8));
            }
            logger.info(String.valueOf(metaSize));
            stream.skip(metaSize);

            this.features = new byte[(int)totalCount][dimension * 4];
            this.keys = new long[(int)totalCount];
            for (int i = 0; i < totalCount; ++i) {
                stream.read(this.features[i]);
            }
            byte[] keysBuf = new byte[(int)totalCount * 8];
            stream.read(keysBuf);
            for (int i = 0; i < totalCount; ++i) {
                int start = i * 8;
                long value = 0;
                for (int j = 0; j < 8; ++j) {
                    value += ((long)(keysBuf[start + j] & 0xFF) << (j * 8));
                }
                this.keys[i] = value;
            }
            return true;
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }

        return true;
    }

    private boolean loadFileData() {
        if (!kv.containsKey("file")) {
            logger.error("file no specified.");
            return false;
        }
        String file = kv.get("file");
        if (file.endsWith(".txt")) {
            return loadTxtFile(file);
        } else if (file.endsWith(".vecs2")) {
            return loadVecs2File(file);
        } else {
            logger.error("unsupported file " + file);
            return false;
        }
    }

    public void insertCollection() {
        if (columnName == null || columnName.isEmpty()) {
            logger.error("column no specified.");
            return;
        }

        if (!loadFileData()) {
            logger.error("Load file data failed.");
            return;
        }

        long startTime = System.currentTimeMillis();
        int totalCount = this.features.length;
        int concurrency = this.clientList.size();
        int step = (totalCount + concurrency - 1) / concurrency;
        List<Thread> threads = new ArrayList<Thread>();
        int start = 0;
        AtomicInteger idx = new AtomicInteger(0);
        for (int i = 0; i < concurrency; ++i) {
            threads.add(new Thread(new ThreadWorker(ThreadWorker.WorkerType.WT_INSERT,
                    clientList.get(i), collection, columnName, dimension,
                    keys, features, start, start + step, idx)));
            start += step;
            threads.get(i).start();
            logger.info("Thread " + i + " start working.");
        }

        for (int i = 0; i < concurrency; ++i) {
            try {
                threads.get(i).join();
                logger.info("Thread " + i + " stopped.");
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }

        long endTime = System.currentTimeMillis();
        logger.info("Build Qps: " + totalCount * 1000 / (endTime - startTime));

    }

    public void searchCollection() {
        if (columnName == null || columnName.isEmpty()) {
            logger.error("column no specified.");
            return;
        }
        if (kv.containsKey("topk")) {
            topk = Integer.valueOf(kv.get("topk")).intValue();
        }
        if (!loadFileData()) {
            logger.error("Load file data failed.");
            return;
        }

//        long startTime = System.currentTimeMillis();
        int totalCount = this.features.length;
//    if (totalCount > 10000) {
//      totalCount = 10000;
//    }
        int concurrency = this.clientList.size();
        int step = (totalCount + concurrency - 1) / concurrency;
        List<Thread> threads = new ArrayList<Thread>();
        int start = 0;
        AtomicInteger idx = new AtomicInteger(0);
        long startTime = System.currentTimeMillis();
        for (int i = 0; i < concurrency; ++i) {
            threads.add(new Thread(new ThreadWorker(ThreadWorker.WorkerType.WT_SEARCH,
                    clientList.get(i), collection, columnName, dimension, topk,
                    keys, features, start, start + step, idx)));
            start += step;
            threads.get(i).start();
//            logger.info("Thread " + i + " start working.");
        }

        for (int i = 0; i < concurrency; ++i) {
            try {
                threads.get(i).join();
//                logger.info("Thread " + i + " stopped.");
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
        long endTime = System.currentTimeMillis();
        logger.info("Search Qps: " + totalCount * 1000.0 / (endTime - startTime));
    }

    /**
     * Example:
     * --command create --address 127.0.0.1:16000 --collection test1 --schema {\"index_column_params\":[{\"column_name\":\"index1\",\"data_type\":23,\"dimension\":4}]}
     * --command describe --address 127.0.0.1:16000 --collection test1
     * --command drop --address 127.0.0.1:16000 --collection test1
     * --command insert --address 127.0.0.1:16000 --collection test1 --column index1 --file insert_vectors.txt --concurrency 8
     * --command search --address 127.0.0.1:16000 --collection test1 --column index1 --file query_vectors.txt --concurrency 8
     *
     * Each line format(key;dim1 dim2 dim3 dim4) in file insert_vectors.txt or query_vectors.txt as follows:
     *           3;0.13 0.03 0.31 0.24
     *           10;0.25 0.65 0.23 0.14
     * @param args
     */
    public static void main(String[] args) {
        logger.info(args.length + " " + args);
        if (args.length <= 1) {
            logger.info("Usage: BenchClient --command [create/drop/insert/search] ...");
            return;
        }
        Map<String, String> kv = new HashMap<String, String>();
        for (int i = 0; i < args.length; i += 2) {
            kv.put(args[i].substring(2), args[i + 1]);
            logger.info(args[i].substring(2) + " -> " + args[i + 1]);
        }
        if (!kv.containsKey("command")) {
            logger.info("Usage: BenchClient --command [create/drop/insert/search] ...");
            return;
        }
        BenchClient client = new BenchClient(kv);
        if (!client.init()) {
            logger.error("BenchClient init failed.");
            return;
        }
        if (!client.execute()) {
            logger.error("BenchClient execute failed.");
            return;
        } else {
            logger.info("BenchClient execute success.");
        }
    }
}

