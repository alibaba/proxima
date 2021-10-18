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
 * \brief    Contains document information
 */

package com.alibaba.proxima.be.client;

/**
 * GetDocumentResponse contains document information
 */
public class GetDocumentResponse {
  private Status status;
  private String debugInfo;
  private Document document;

  public GetDocumentResponse(Status.ErrorCode code) {
    this.status = new Status(code);
    this.debugInfo = null;
    this.document = null;
  }

  public GetDocumentResponse(Status.ErrorCode code, String reason) {
    this.status = new Status(code, reason);
    this.debugInfo = "";
    this.document = null;
  }

  public GetDocumentResponse(Status status, String debugInfo, Document document) {
    this.status = status;
    this.debugInfo = debugInfo;
    this.document = document;
  }

  public Status getStatus() {
    return status;
  }

  public String getDebugInfo() {
    return debugInfo;
  }

  public Document getDocument() {
    return document;
  }

  /**
   * Is response success, true means success, false means failed
   * @return boolean
   */
  public boolean ok() {
    return this.status.getCode() == 0;
  }
}
