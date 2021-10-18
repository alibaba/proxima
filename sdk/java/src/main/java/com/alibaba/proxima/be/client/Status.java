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
 * \brief    Contains the code and reason
 */

package com.alibaba.proxima.be.client;

/**
 * Status for response
 */
public class Status {
  private final int code;
  private final String reason;

  /**
   * Constructor with int code and reason
   * @param code code value from server
   * @param reason error message
   */
  public Status(int code, String reason) {
    this.code = code;
    this.reason = reason;
  }

  /**
   * Constructor with ErrorCode and reason
   * @param code error code
   * @param reason error message
   */
  public Status(ErrorCode code, String reason) {
    this.code = code.getCode();
    this.reason = reason;
  }

  /**
   * Constructor with error code
   * @param code error code
   */
  public Status(ErrorCode code) {
    this.code = code.getCode();
    if (code == ErrorCode.CLIENT_NOT_CONNECTED) {
      this.reason = "Client not connected to proxima search engine";
    } else if (code == ErrorCode.RPC_TIMEOUT) {
      this.reason = "Rpc request timeout";
    } else if (code == ErrorCode.RPC_ERROR) {
      this.reason = "Rpc error occurred";
    } else if (code == ErrorCode.UNKNOWN_ERROR) {
      this.reason = "Unknown error occurred";
    } else {
      this.reason = "Success";
    }
  }

  public int getCode() {
    return code;
  }

  public String getReason() {
    return reason;
  }

  /**
   * Is status normal
   * @return boolean
   */
  public boolean ok() {
    return this.code == 0;
  }

  /**
   * Convert the Status to json string
   * @return String
   */
  public String toString() {
    return "{ \"code\": " + String.valueOf(code) + ", \"reason\": \"" + reason + "\"}";
  }

  /**
   * Java client side error
   */
  public enum ErrorCode {
    /**
     * Success status
     */
    SUCCESSS(0),
    /**
     * Rpc timeout
     */
    RPC_TIMEOUT(10000),
    /**
     * Rpc call error
     */
    RPC_ERROR(10001),
    /**
     * Client not connected
     */
    CLIENT_NOT_CONNECTED(10002),
    /**
     * Unknown error
     */
    UNKNOWN_ERROR(10003);

    private int code;

    ErrorCode(int code) {
      this.code = code;
    }

    public int getCode() {
      return this.code;
    }
  }
}
