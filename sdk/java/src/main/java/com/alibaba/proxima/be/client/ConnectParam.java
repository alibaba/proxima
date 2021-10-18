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
 * \brief    ConnectParam contains the grpc connecting parameters
 */

package com.alibaba.proxima.be.client;

import java.util.concurrent.TimeUnit;

/**
 * ConnectParam contains the grpc connecting param
 */
public class ConnectParam {
  // required parameters
  private final String host;
  private final int port;
  // optional parameters
  private final long timeoutNanos;
  private final long idleTimeoutNanos;
  private final long keepAliveTimeNanos;
  private final long keepAliveTimeoutNanos;

  //

  /**
   * Constructor with builder
   * @param builder builder
   */
  private ConnectParam(Builder builder) {
    this.port = builder.port;
    this.host = builder.host;
    this.timeoutNanos = builder.timeoutNanos;
    this.idleTimeoutNanos = builder.idleTimeoutNanos;
    this.keepAliveTimeNanos = builder.keepAliveTimeNanos;
    this.keepAliveTimeoutNanos = builder.keepAliveTimeoutNanos;
  }

  /**
   * Get host
   * @return String
   */
  public String getHost() {
    return this.host;
  }

  /**
   * Get port
   * @return int
   */
  public int getPort() {
    return this.port;
  }

  /**
   * Get timeout with unit
   * @param timeUnit time unit
   * @return request timeout
   */
  public long getTimeout(TimeUnit timeUnit) {
    return timeUnit.convert(this.timeoutNanos, TimeUnit.NANOSECONDS);
  }

  /**
   * Get idle timeout with unit
   * @param timeUnit time unit
   * @return idle timeout by time unit
   */
  public long getIdleTimeout(TimeUnit timeUnit) {
    return timeUnit.convert(this.idleTimeoutNanos, TimeUnit.NANOSECONDS);
  }

  /**
   * Get keep alive time with unit
   * @param timeUnit time unit
   * @return keep alive time by time unit
   */
  public long getKeepAliveTime(TimeUnit timeUnit) {
    return timeUnit.convert(this.keepAliveTimeNanos, TimeUnit.NANOSECONDS);
  }

  /**
   * Get keep alive timeout with unit
   * @param timeUnit time unit
   * @return keep alive timeout by time unit
   */
  public long getKeepAliveTimeout(TimeUnit timeUnit) {
    return timeUnit.convert(this.keepAliveTimeoutNanos, TimeUnit.NANOSECONDS);
  }

  /**
   * New ConnectParam builder
   * @return Builder
   */
  public static Builder newBuilder() {
    return new Builder();
  }

  /**
   * Builder for ConnectParam
   */
  public static class Builder {
    private String host = "localhost";
    private int port = 16000;
    private long timeoutNanos = TimeUnit.NANOSECONDS.convert(1, TimeUnit.SECONDS);
    private long idleTimeoutNanos = TimeUnit.NANOSECONDS.convert(12, TimeUnit.HOURS);
    private long keepAliveTimeNanos = Long.MAX_VALUE;
    private long keepAliveTimeoutNanos = TimeUnit.NANOSECONDS.convert(30, TimeUnit.SECONDS);

    /**
     * Build ConnectParam object
     * @return ConnectParam
     */
    public ConnectParam build() {
      return new ConnectParam(this);
    }

    /**
     * Set host
     * @param host grpc server host
     * @return Builder
     */
    public Builder withHost(String host) {
      this.host = host;
      return this;
    }

    /**
     * Set grpc port
     * @param port grpc server port
     * @return Builder
     */
    public Builder withPort(int port) {
      this.port = port;
      return this;
    }

    /**
     * Set request timeout
     * @param timeout request timeout value
     * @param timeUnit time unit
     * @return Builder
     */
    public Builder withTimeout(long timeout, TimeUnit timeUnit) {
      this.timeoutNanos = timeUnit.toNanos(timeout);
      return this;
    }

    /**
     * Set idle timeout
     * @param idleTimeout idle timeout by time unit
     * @param timeUnit time unit
     * @return Builder
     */
    public Builder withIdleTimeout(long idleTimeout, TimeUnit timeUnit) {
      this.idleTimeoutNanos = timeUnit.toNanos(idleTimeout);
      return this;
    }

    /**
     * Set keep alive time
     * @param keepAliveTime keep alive time by time unit
     * @param timeUnit time unit
     * @return Builder
     */
    public Builder withKeepAliveTimeNanos(long keepAliveTime, TimeUnit timeUnit) {
      this.keepAliveTimeNanos = timeUnit.toNanos(keepAliveTime);
      return this;
    }

    /**
     * Set keep alive timeout
     * @param keepAliveTimeout keep alive timeout by time unit
     * @param timeUnit time unit
     * @return Builder
     */
    public Builder withKeepAliveTimeoutNanos(long keepAliveTimeout, TimeUnit timeUnit) {
      this.keepAliveTimeoutNanos = timeUnit.toNanos(keepAliveTimeout);
      return this;
    }
  }
}
