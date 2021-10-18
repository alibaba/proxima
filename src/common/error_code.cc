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

 *   \author   Hongqing.hu
 *   \date     Nov 2020
 *   \brief    Implementation of Bilin Error
 */

#include "error_code.h"
#include <aitheta2/index_error.h>

namespace proxima {
namespace be {

// 0~999  [Builtin]
PROXIMA_BE_ERROR_CODE_DEFINE(Success, 0, "Success");

// 1000~1999 [Common Error]
PROXIMA_BE_ERROR_CODE_DEFINE(RuntimeError, 1000, "Runtime Error");
PROXIMA_BE_ERROR_CODE_DEFINE(LogicError, 1001, "Logic Error");
PROXIMA_BE_ERROR_CODE_DEFINE(StatusError, 1002, "Status Error");
PROXIMA_BE_ERROR_CODE_DEFINE(LoadConfig, 1003, "Load Config Error");
PROXIMA_BE_ERROR_CODE_DEFINE(ConfigError, 1004, "Config Error");
PROXIMA_BE_ERROR_CODE_DEFINE(InvalidArgument, 1005, "Invalid Arugment");
PROXIMA_BE_ERROR_CODE_DEFINE(NotInitialized, 1006, "Not Initialized");
PROXIMA_BE_ERROR_CODE_DEFINE(OpenFile, 1007, "Open File Error");
PROXIMA_BE_ERROR_CODE_DEFINE(ReadData, 1008, "Read Data Error");
PROXIMA_BE_ERROR_CODE_DEFINE(WriteData, 1009, "Write Data Error");
PROXIMA_BE_ERROR_CODE_DEFINE(ExceedLimit, 1010, "Exceed Limit");
PROXIMA_BE_ERROR_CODE_DEFINE(SerializeError, 1011, "Serialize Error");
PROXIMA_BE_ERROR_CODE_DEFINE(DeserializeError, 1012, "Deserialize Error");
PROXIMA_BE_ERROR_CODE_DEFINE(StartServer, 1013, "Start Server Error");
PROXIMA_BE_ERROR_CODE_DEFINE(StoppedService, 1014, "Visit Stopped Service");

// 2000~2999 [Format Check]
PROXIMA_BE_ERROR_CODE_DEFINE(EmptyCollectionName, 2000,
                             "Empty Collection Name");
PROXIMA_BE_ERROR_CODE_DEFINE(EmptyColumnName, 2001, "Empty Column Name");
PROXIMA_BE_ERROR_CODE_DEFINE(EmptyColumns, 2002, "Empty Columns");
PROXIMA_BE_ERROR_CODE_DEFINE(EmptyRepositoryTable, 2003,
                             "Empty Repository Table");
PROXIMA_BE_ERROR_CODE_DEFINE(EmptyRepositoryName, 2004,
                             "Empty Repository Name");
PROXIMA_BE_ERROR_CODE_DEFINE(EmptyUserName, 2005, "Empty User Name");
PROXIMA_BE_ERROR_CODE_DEFINE(EmptyPassword, 2006, "Empty Password");
PROXIMA_BE_ERROR_CODE_DEFINE(InvalidURI, 2007, "Invalid URI");
PROXIMA_BE_ERROR_CODE_DEFINE(InvalidCollectionStatus, 2008,
                             "Invalid Collection Status");
PROXIMA_BE_ERROR_CODE_DEFINE(InvalidRecord, 2009, "Invalid Record");
PROXIMA_BE_ERROR_CODE_DEFINE(InvalidQuery, 2010, "Invalid Query");
PROXIMA_BE_ERROR_CODE_DEFINE(InvalidIndexDataFormat, 2011,
                             "Invalid Index Data Format");
PROXIMA_BE_ERROR_CODE_DEFINE(InvalidWriteRequest, 2012,
                             "Invalid Write Request");
PROXIMA_BE_ERROR_CODE_DEFINE(InvalidVectorFormat, 2013,
                             "Invalid Vector Format");
PROXIMA_BE_ERROR_CODE_DEFINE(InvalidRepositoryType, 2014,
                             "Invalid Repository Type");
PROXIMA_BE_ERROR_CODE_DEFINE(InvalidDataType, 2015, "Invalid Data Type");
PROXIMA_BE_ERROR_CODE_DEFINE(InvalidIndexType, 2016, "Invalid Index Type");
PROXIMA_BE_ERROR_CODE_DEFINE(InvalidSegment, 2017, "Invalid Segment");
PROXIMA_BE_ERROR_CODE_DEFINE(InvalidRevision, 2018, "Invalid Revision");
PROXIMA_BE_ERROR_CODE_DEFINE(InvalidFeature, 2019, "Invalid Feature");
PROXIMA_BE_ERROR_CODE_DEFINE(MismatchedSchema, 2020, "Mismatched schema");
PROXIMA_BE_ERROR_CODE_DEFINE(MismatchedMagicNumber, 2021,
                             "Mismatched Magic Number");
PROXIMA_BE_ERROR_CODE_DEFINE(MismatchedIndexColumn, 2022,
                             "Mismatched Index Column");
PROXIMA_BE_ERROR_CODE_DEFINE(MismatchedDimension, 2023, "Mismatched Dimension");
PROXIMA_BE_ERROR_CODE_DEFINE(MismatchedDataType, 2024, "Mismatched Data Type");

// 3000~3999 [Meta]
PROXIMA_BE_ERROR_CODE_DEFINE(UpdateStatusField, 3000,
                             "Status Field Is Readonly");
PROXIMA_BE_ERROR_CODE_DEFINE(UpdateRevisionField, 3001,
                             "Revision Field Is Readonly");
PROXIMA_BE_ERROR_CODE_DEFINE(UpdateCollectionUIDField, 3002,
                             "CollectionUID Field Is Readonly");
PROXIMA_BE_ERROR_CODE_DEFINE(UpdateIndexTypeField, 3003,
                             "IndexType Field Is Readonly");
PROXIMA_BE_ERROR_CODE_DEFINE(UpdateDataTypeField, 3004,
                             "DataType Field Is Readonly");
PROXIMA_BE_ERROR_CODE_DEFINE(UpdateParametersField, 3005,
                             "Parameters Filed Is Readonly");
PROXIMA_BE_ERROR_CODE_DEFINE(UpdateRepositoryTypeField, 3006,
                             "RepositoryType Field Is Readonly");
PROXIMA_BE_ERROR_CODE_DEFINE(UpdateColumnNameField, 3007,
                             "Update ColumnName Field Is Readonly");
PROXIMA_BE_ERROR_CODE_DEFINE(ZeroDocsPerSegment, 3008, "Zero Docs Per Segment");
PROXIMA_BE_ERROR_CODE_DEFINE(UnsupportedConnection, 3009,
                             "Unsupported Connection");

// 4000~4999 [Index]
PROXIMA_BE_ERROR_CODE_DEFINE(DuplicateCollection, 4000, "Duplicate Collection");
PROXIMA_BE_ERROR_CODE_DEFINE(DuplicateKey, 4001, "Duplicate Key");
PROXIMA_BE_ERROR_CODE_DEFINE(InexistentCollection, 4002,
                             "Collection Not Exist");
PROXIMA_BE_ERROR_CODE_DEFINE(InexistentColumn, 4003, "Column Not Exist");
PROXIMA_BE_ERROR_CODE_DEFINE(InexistentKey, 4004, "Key Not Exist");
PROXIMA_BE_ERROR_CODE_DEFINE(SuspendedCollection, 4005,
                             "Collection Is Suspended");
PROXIMA_BE_ERROR_CODE_DEFINE(LostSegment, 4006, "Lost Segment");
PROXIMA_BE_ERROR_CODE_DEFINE(EmptyLsnContext, 4007, "Empty Lsn Context");
PROXIMA_BE_ERROR_CODE_DEFINE(ExceedRateLimit, 4008, "Exceed Rate Limit");

// 5000~5999 [Query]
PROXIMA_BE_ERROR_CODE_DEFINE(UnavailableSegment, 5000,
                             "Segment Is "
                             "unavailable");
PROXIMA_BE_ERROR_CODE_DEFINE(MismatchedForward, 5001, "Mismatched Forward");
PROXIMA_BE_ERROR_CODE_DEFINE(OutOfBoundsResult, 5002, "Results Out Of Bounds");
PROXIMA_BE_ERROR_CODE_DEFINE(UnreadyQueue, 5003,
                             "Compute Queue Is Unready Yet");
PROXIMA_BE_ERROR_CODE_DEFINE(ScheduleError, 5004, "Schedule Task Error");
PROXIMA_BE_ERROR_CODE_DEFINE(UnreadableCollection, 5005,
                             "Collection Is Unreadable");
PROXIMA_BE_ERROR_CODE_DEFINE(TaskIsRunning, 5006,
                             "Task is running in other coroutine");

// NOTICE
// 10000~19999 [SDK]
// 20000~29999 [Repository]


const char *ErrorCode::What(int val) {
  if (val >= -1000) {
    return aitheta2::IndexError::What(val);
  } else {
    return ErrorCode::Instance()->what(val);
  }
}

}  // namespace be
}  // end namespace proxima
