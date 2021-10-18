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
 * \brief    CollectionStats contains running statistic information for collection
 */

package com.alibaba.proxima.be.client;

import java.util.ArrayList;
import java.util.List;

/**
 * Contains statistic information for collection
 */
public class CollectionStats {
  private final String collectionName;
  private final String collectionPath;
  private final long totalDocCount;
  private final long totalSegmentCount;
  private final long totalIndexFileCount;
  private final long totalIndexFileSize;
  private final List<SegmentStats> segmentStats;

  public CollectionStats(Builder builder) {
    this.collectionName = builder.collectionName;
    this.collectionPath = builder.collectionPath;
    this.totalDocCount = builder.totalDocCount;
    this.totalSegmentCount = builder.totalSegmentCount;
    this.totalIndexFileCount = builder.totalIndexFileCount;
    this.totalIndexFileSize = builder.totalIndexFileSize;
    this.segmentStats = builder.segmentStats;
  }

  public String getCollectionName() {
    return collectionName;
  }

  public String getCollectionPath() {
    return collectionPath;
  }

  public long getTotalDocCount() {
    return totalDocCount;
  }

  public long getTotalSegmentCount() {
    return totalSegmentCount;
  }

  public long getTotalIndexFileCount() {
    return totalIndexFileCount;
  }

  public long getTotalIndexFileSize() {
    return totalIndexFileSize;
  }

  public List<SegmentStats> getSegmentStats() {
    return segmentStats;
  }

  /**
   * Get total segments count
   * @return total segment stats count
   */
  public int getSegmentStatsCount() {
    if (this.segmentStats != null) {
      return this.segmentStats.size();
    }
    return 0;
  }

  /**
   * Get one segments stats
   * @param index segment index
   * @return SegmentStats
   */
  public SegmentStats getSegmentStats(int index) {
    return this.segmentStats.get(index);
  }

  /**
   * New Collection stats builder
   * @return Builder
   */
  public static Builder newBuilder() {
    return new Builder();
  }

  /**
   * Builder for CollectionStats
   */
  public static class Builder {
    // required parameters
    private String collectionName;
    private String collectionPath;
    private long totalDocCount;
    private long totalSegmentCount;
    private long totalIndexFileCount;
    private long totalIndexFileSize;
    private List<SegmentStats> segmentStats = new ArrayList<>();

    /**
     * Constructor without parameters
     */
    public Builder() {
    }

    /**
     * Set collection name
     * @param collectionName collection name
     * @return Builder
     */
    public Builder withCollectionName(String collectionName) {
      this.collectionName = collectionName;
      return this;
    }

    /**
     * Set collection path
     * @param collectionPath collection index path
     * @return Builder
     */
    public Builder withCollectionPath(String collectionPath) {
      this.collectionPath = collectionPath;
      return this;
    }

    /**
     * Set total document count
     * @param totalDocCount total document count
     * @return Builder
     */
    public Builder withTotalDocCount(long totalDocCount) {
      this.totalDocCount = totalDocCount;
      return this;
    }

    /**
     * Set total segment count
     * @param totalSegmentCount total segment count
     * @return Builder
     */
    public Builder withTotalSegmentCount(long totalSegmentCount) {
      this.totalSegmentCount = totalSegmentCount;
      return this;
    }

    /**
     * Set total index file count
     * @param totalIndexFileCount total index file count
     * @return Builder
     */
    public Builder withTotalIndexFileCount(long totalIndexFileCount) {
      this.totalIndexFileCount = totalIndexFileCount;
      return this;
    }

    /**
     * Set total index file size
     * @param totalIndexFileSize total index file size
     * @return Builder
     */
    public Builder withTotalIndexFileSize(long totalIndexFileSize) {
      this.totalIndexFileSize = totalIndexFileSize;
      return this;
    }

    /**
     * Set segment stats list
     * @param segmentStatsList segment stats list
     * @return Builder
     */
    public Builder withSegmentStats(List<SegmentStats> segmentStatsList) {
      this.segmentStats = segmentStatsList;
      return this;
    }

    /**
     * Add segment stats
     * @param segmentStats segment stats
     * @return Builder
     */
    public Builder addSegmentStats(SegmentStats segmentStats) {
      this.segmentStats.add(segmentStats);
      return this;
    }

    /**
     * Build collection stats object
     * @return CollectionStats
     */
    public CollectionStats build() {
      return new CollectionStats(this);
    }
  }

  /**
   * Segment running state
   */
  public enum SegmentState {
    /**
     * Segment created
     */
    CREATED(0),
    /**
     * Segment writing
     */
    WRITING(1),
    /**
     * Segment dumping
     */
    DUMPING(2),
    /**
     * Segment compacting
     */
    COMPACTING(3),
    /**
     * Segment persist
     */
    PERSIST(4),
    /**
     * Unknown state
     */
    UNKNOWN(-1);

    private final int value;

    SegmentState(int value) {
      this.value = value;
    }

    public int getValue() {
      return this.value;
    }

    public static SegmentState valueOf(int value) {
      switch (value) {
        case 0:
          return CREATED;
        case 1:
          return WRITING;
        case 2:
          return DUMPING;
        case 3:
          return COMPACTING;
        case 4:
          return PERSIST;
        default:
          return UNKNOWN;
      }
    }
  }

  /**
   * Statistic for one segment
   */
  public static class SegmentStats {
    private final int segmentId;
    private final SegmentState segmentState;
    private final long docDount;
    private final long indexFileCount;
    private final long indexFileSize;
    private final long minDocId;
    private final long maxDocId;
    private final long minPrimaryKey;
    private final long maxPrimaryKey;
    private final long minTimestamp;
    private final long maxTimestamp;
    private final long minLsn;
    private final long maxLsn;
    private final String segmentPath;

    private SegmentStats(Builder builder) {
      this.segmentId = builder.segmentId;
      this.segmentState = builder.segmentState;
      this.docDount = builder.docDount;
      this.indexFileCount = builder.indexFileCount;
      this.indexFileSize = builder.indexFileSize;
      this.minDocId = builder.minDocId;
      this.maxDocId = builder.maxDocId;
      this.minPrimaryKey = builder.minPrimaryKey;
      this.maxPrimaryKey = builder.maxPrimaryKey;
      this.minTimestamp = builder.minTimestamp;
      this.maxTimestamp = builder.maxTimestamp;
      this.minLsn = builder.minLsn;
      this.maxLsn = builder.maxLsn;
      this.segmentPath = builder.segmentPath;
    }

    public int getSegmentId() {
      return segmentId;
    }

    public SegmentState getSegmentState() {
      return segmentState;
    }

    public long getDocDount() {
      return docDount;
    }

    public long getIndexFileCount() {
      return indexFileCount;
    }

    public long getIndexFileSize() {
      return indexFileSize;
    }

    public long getMinDocId() {
      return minDocId;
    }

    public long getMaxDocId() {
      return maxDocId;
    }

    public long getMinPrimaryKey() {
      return minPrimaryKey;
    }

    public long getMaxPrimaryKey() {
      return maxPrimaryKey;
    }

    public long getMinTimestamp() {
      return minTimestamp;
    }

    public long getMaxTimestamp() {
      return maxTimestamp;
    }

    public long getMinLsn() {
      return minLsn;
    }

    public long getMaxLsn() {
      return maxLsn;
    }

    public String getSegmentPath() {
      return segmentPath;
    }

    public static Builder newBuilder() {
      return new Builder();
    }

    /**
     * Segment stats builder
     */
    public static class Builder {
      private int segmentId;
      private SegmentState segmentState;
      private long docDount;
      private long indexFileCount;
      private long indexFileSize;
      private long minDocId;
      private long maxDocId;
      private long minPrimaryKey;
      private long maxPrimaryKey;
      private long minTimestamp;
      private long maxTimestamp;
      private long minLsn;
      private long maxLsn;
      private String segmentPath;

      public Builder() {
      }

      public Builder withSegmentId(int segmentId) {
        this.segmentId = segmentId;
        return this;
      }

      public Builder withSegmentState(SegmentState segmentState) {
        this.segmentState = segmentState;
        return this;
      }

      public Builder withDocDount(long docDount) {
        this.docDount = docDount;
        return this;
      }

      public Builder withIndexFileCount(long indexFileCount) {
        this.indexFileCount = indexFileCount;
        return this;
      }

      public Builder withIndexFileSize(long indexFileSize) {
        this.indexFileSize = indexFileSize;
        return this;
      }

      public Builder withMinDocId(long minDocId) {
        this.minDocId = minDocId;
        return this;
      }

      public Builder withMaxDocId(long maxDocId) {
        this.maxDocId = maxDocId;
        return this;
      }

      public Builder withMinPrimaryKey(long minPrimaryKey) {
        this.minPrimaryKey = minPrimaryKey;
        return this;
      }

      public Builder withMaxPrimaryKey(long maxPrimaryKey) {
        this.maxPrimaryKey = maxPrimaryKey;
        return this;
      }

      public Builder withMinTimestamp(long minTimestamp) {
        this.minTimestamp = minTimestamp;
        return this;
      }

      public Builder withMaxTimestamp(long maxTimestamp) {
        this.maxTimestamp = maxTimestamp;
        return this;
      }

      public Builder withMinLsn(long minLsn) {
        this.minLsn = minLsn;
        return this;
      }

      public Builder withMaxLsn(long maxLsn) {
        this.maxLsn = maxLsn;
        return this;
      }

      public Builder withSegmentPath(String segmentPath) {
        this.segmentPath = segmentPath;
        return this;
      }

      /**
       * Build segment stats object
       * @return SegmentStats
       */
      public SegmentStats build() {
        return new SegmentStats(this);
      }
    }
  }


}
