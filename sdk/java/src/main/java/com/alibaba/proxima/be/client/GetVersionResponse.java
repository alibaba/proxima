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
 * \brief    Proxima Search Engine version
 */

package com.alibaba.proxima.be.client;

/**
 * Contains the server version
 */
public class GetVersionResponse {
  Status status;
  String version;

  public GetVersionResponse(Status.ErrorCode errorCode) {
    this.status = new Status(errorCode);
    this.version = "";
  }

  public GetVersionResponse(Status.ErrorCode errorCode, String errorMsg) {
    this.status = new Status(errorCode, errorMsg);
    this.version = "";
  }

  public GetVersionResponse(Status status, String version) {
    this.status = status;
    this.version = version;
  }

  public Status getStatus() {
    return status;
  }

  public String getVersion() {
    return version;
  }

  /**
   * Is the response success
   * @return boolean
   */
  public boolean ok() {
    return this.status.ok();
  }
}
