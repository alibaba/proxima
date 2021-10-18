/**
 *   Copyright 2021 Alibaba, Inc. and its affiliates. All Rights Reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.

 *   \author   Dianzhang.Chen
 *   \date     Mar 2021
 *   \brief    Implementation of error code
 */

#include "error_code.h"
#include <aitheta2/index_error.h>

namespace proxima {
namespace be {
namespace repository {

// 0~999  [Builtin]
PROXIMA_BE_REPOSITORY_ERROR_CODE_DEFINE(Success, 0, "Success");

// 1000~1999 [Common Error]
PROXIMA_BE_REPOSITORY_ERROR_CODE_DEFINE(RuntimeError, 1000, "Runtime Error");
PROXIMA_BE_REPOSITORY_ERROR_CODE_DEFINE(LogicError, 1001, "Logic Error");
PROXIMA_BE_REPOSITORY_ERROR_CODE_DEFINE(LoadConfig, 1003, "Load Config Error");
PROXIMA_BE_REPOSITORY_ERROR_CODE_DEFINE(ConfigError, 1004, "Config Error");
PROXIMA_BE_REPOSITORY_ERROR_CODE_DEFINE(InvalidArgument, 1005,
                                        "Invalid Arugment");
PROXIMA_BE_REPOSITORY_ERROR_CODE_DEFINE(NotInitialized, 1006,
                                        "Not Initialized");
PROXIMA_BE_REPOSITORY_ERROR_CODE_DEFINE(OpenFile, 1007, "Open File Error");
PROXIMA_BE_REPOSITORY_ERROR_CODE_DEFINE(ExceedLimit, 1010, "Exceed Limit");

// 2000~2999 [Format Check]
PROXIMA_BE_REPOSITORY_ERROR_CODE_DEFINE(MismatchedSchema, 2020,
                                        "Mismatched Schema");
PROXIMA_BE_REPOSITORY_ERROR_CODE_DEFINE(MismatchedMagicNumber, 2021,
                                        "Mismatched Magic Number");

// 4000~4999 [Index]
PROXIMA_BE_REPOSITORY_ERROR_CODE_DEFINE(DuplicateCollection, 4000,
                                        "Duplicate Collection");
PROXIMA_BE_REPOSITORY_ERROR_CODE_DEFINE(ExceedRateLimit, 4008,
                                        "Exceed Rate Limit");

// 20000~29999 [Repository]

// 20000~21000 [Mysql repository]
PROXIMA_BE_REPOSITORY_ERROR_CODE_DEFINE(ConnectMysql, 20000,
                                        "Connect mysql error");
PROXIMA_BE_REPOSITORY_ERROR_CODE_DEFINE(InvalidMysqlTable, 20001,
                                        "Invalid mysql table");
PROXIMA_BE_REPOSITORY_ERROR_CODE_DEFINE(ExecuteMysql, 20002,
                                        "Execute mysql error");
PROXIMA_BE_REPOSITORY_ERROR_CODE_DEFINE(TableNoMoreData, 20003,
                                        "Table no more data");
PROXIMA_BE_REPOSITORY_ERROR_CODE_DEFINE(InvalidRowData, 20004,
                                        "Invalid row data");
PROXIMA_BE_REPOSITORY_ERROR_CODE_DEFINE(UnsupportedMysqlVersion, 20005,
                                        "Unsupported mysql version");
PROXIMA_BE_REPOSITORY_ERROR_CODE_DEFINE(ExecuteSimpleCommand, 20006,
                                        "Execute simple command error");
PROXIMA_BE_REPOSITORY_ERROR_CODE_DEFINE(BinlogDump, 20007, "Binlog dump error");
PROXIMA_BE_REPOSITORY_ERROR_CODE_DEFINE(BinlogNoMoreData, 20008,
                                        "Binlog no more data.");
PROXIMA_BE_REPOSITORY_ERROR_CODE_DEFINE(InvalidMysqlResult, 20009,
                                        "Invalid mysql result.");
PROXIMA_BE_REPOSITORY_ERROR_CODE_DEFINE(UnsupportedBinlogFormat, 20010,
                                        "Unsupported bin log format.");
PROXIMA_BE_REPOSITORY_ERROR_CODE_DEFINE(FetchMysqlResult, 20011,
                                        "Fetch mysql result error.");
PROXIMA_BE_REPOSITORY_ERROR_CODE_DEFINE(Suspended, 20012,
                                        "Bin log suspended status.");
PROXIMA_BE_REPOSITORY_ERROR_CODE_DEFINE(NoInitialized, 20013, "No initialized");
PROXIMA_BE_REPOSITORY_ERROR_CODE_DEFINE(RepeatedInitialized, 20014,
                                        "Repeated initialized");
PROXIMA_BE_REPOSITORY_ERROR_CODE_DEFINE(InvalidCollectionConfig, 20015,
                                        "Invalid collection config");

//! Error for collection
PROXIMA_BE_REPOSITORY_ERROR_CODE_DEFINE(CollectionNotExist, 20016,
                                        "Collection not exist");
PROXIMA_BE_REPOSITORY_ERROR_CODE_DEFINE(RPCFailed, 20017, "RPC Failed");
PROXIMA_BE_REPOSITORY_ERROR_CODE_DEFINE(Terminate, 20018,
                                        "Collection should terminate");
PROXIMA_BE_REPOSITORY_ERROR_CODE_DEFINE(InvalidUri, 20019, "Invalid uri");
PROXIMA_BE_REPOSITORY_ERROR_CODE_DEFINE(InitChannel, 20020,
                                        "Init brpc channel failed");
PROXIMA_BE_REPOSITORY_ERROR_CODE_DEFINE(InvalidMysqlHandler, 20021,
                                        "Invalid mysql handler");
PROXIMA_BE_REPOSITORY_ERROR_CODE_DEFINE(InvalidLSNContext, 20022,
                                        "LSN context is invalid");
PROXIMA_BE_REPOSITORY_ERROR_CODE_DEFINE(NoMoreData, 20023,
                                        "There is no more row data");
PROXIMA_BE_REPOSITORY_ERROR_CODE_DEFINE(SchemaChanged, 20024, "Schema changed");

PROXIMA_BE_REPOSITORY_ERROR_CODE_DEFINE(
    MismatchedVersion, 20025,
    "Server version mismatch with repository version");

const char *ErrorCode::What(int val) {
  if (val >= -1000) {
    return aitheta2::IndexError::What(val);
  } else {
    return ErrorCode::Instance()->what(val);
  }
}

}  // namespace repository
}  // namespace be
}  // end namespace proxima
