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
 *   \brief    Interface of Bilin Error
 */

#pragma once

#include <map>

namespace proxima {
namespace be {

/*! Error
 */
class ErrorCode {
 public:
  /*! Error Code
   */
  class Code {
   public:
    //! Constructor
    Code(int val, const char *str) : value_(-val), desc_(str) {
      ErrorCode::Instance()->emplace(this);
    }

    //! Retrieve the value of code
    operator int() const {
      return (this->value_);
    }

    //! Retrieve the value of code
    int value() const {
      return (this->value_);
    }

    //! Retrieve the description of code
    const char *desc() const {
      return (this->desc_);
    }

   private:
    int value_;
    const char *desc_;
  };

  //! Retrieve the description of code
  static const char *What(int val);

 protected:
  //! Constructor
  ErrorCode(void) : map_() {}

  //! Inserts a new code into map
  void emplace(const ErrorCode::Code *code) {
    map_.emplace(code->value(), code);
  }

  //! Retrieve the description of code
  const char *what(int val) const {
    auto iter = map_.find(val);
    if (iter != map_.end()) {
      return iter->second->desc();
    }
    return "";
  }

  //! Retrieve the singleton
  static ErrorCode *Instance(void) {
    static ErrorCode error;
    return (&error);
  }

 private:
  //! Disable them
  ErrorCode(const ErrorCode &) = delete;
  ErrorCode(ErrorCode &&) = delete;
  ErrorCode &operator=(const ErrorCode &) = delete;

  //! Error code map
  std::map<int, const ErrorCode::Code *> map_;
};

//! Error Code Define
#define PROXIMA_BE_ERROR_CODE_DEFINE(__NAME__, __VAL__, __DESC__)      \
  const proxima::be::ErrorCode::Code ErrorCode_##__NAME__((__VAL__),   \
                                                          (__DESC__)); \
  const proxima::be::ErrorCode::Code &_ErrorCode_##__VAL__##_Register( \
      ErrorCode_##__NAME__)

//! Proxima BE Error Code Declare
#define PROXIMA_BE_ERROR_CODE_DECLARE(__NAME__) \
  extern const proxima::be::ErrorCode::Code ErrorCode_##__NAME__

//! Error code helper
#define PROXIMA_BE_ERROR_CODE(__NAME__) proxima::be::ErrorCode_##__NAME__


// 0~999  [Builtin]
PROXIMA_BE_ERROR_CODE_DECLARE(Success);


// 1000~1999 [Common Error]
PROXIMA_BE_ERROR_CODE_DECLARE(RuntimeError);
PROXIMA_BE_ERROR_CODE_DECLARE(LogicError);
PROXIMA_BE_ERROR_CODE_DECLARE(StatusError);
PROXIMA_BE_ERROR_CODE_DECLARE(LoadConfig);
PROXIMA_BE_ERROR_CODE_DECLARE(ConfigError);
PROXIMA_BE_ERROR_CODE_DECLARE(InvalidArgument);
PROXIMA_BE_ERROR_CODE_DECLARE(NotInitialized);
PROXIMA_BE_ERROR_CODE_DECLARE(OpenFile);
PROXIMA_BE_ERROR_CODE_DECLARE(ReadData);
PROXIMA_BE_ERROR_CODE_DECLARE(WriteData);
PROXIMA_BE_ERROR_CODE_DECLARE(ExceedLimit);
PROXIMA_BE_ERROR_CODE_DECLARE(SerializeError);
PROXIMA_BE_ERROR_CODE_DECLARE(DerializeError);
PROXIMA_BE_ERROR_CODE_DECLARE(StartServer);
PROXIMA_BE_ERROR_CODE_DECLARE(StoppedService);

// 2000~2999 [Format Check]
PROXIMA_BE_ERROR_CODE_DECLARE(EmptyCollectionName);
PROXIMA_BE_ERROR_CODE_DECLARE(EmptyColumnName);
PROXIMA_BE_ERROR_CODE_DECLARE(EmptyColumns);
PROXIMA_BE_ERROR_CODE_DECLARE(EmptyRepositoryTable);
PROXIMA_BE_ERROR_CODE_DECLARE(EmptyRepositoryName);
PROXIMA_BE_ERROR_CODE_DECLARE(EmptyUserName);
PROXIMA_BE_ERROR_CODE_DECLARE(EmptyPassword);
PROXIMA_BE_ERROR_CODE_DECLARE(InvalidURI);
PROXIMA_BE_ERROR_CODE_DECLARE(InvalidCollectionStatus);
PROXIMA_BE_ERROR_CODE_DECLARE(InvalidRecord);
PROXIMA_BE_ERROR_CODE_DECLARE(InvalidQuery);
PROXIMA_BE_ERROR_CODE_DECLARE(InvalidIndexDataFormat);
PROXIMA_BE_ERROR_CODE_DECLARE(InvalidWriteRequest);
PROXIMA_BE_ERROR_CODE_DECLARE(InvalidVectorFormat);
PROXIMA_BE_ERROR_CODE_DECLARE(InvalidRepositoryType);
PROXIMA_BE_ERROR_CODE_DECLARE(InvalidDataType);
PROXIMA_BE_ERROR_CODE_DECLARE(InvalidIndexType);
PROXIMA_BE_ERROR_CODE_DECLARE(InvalidSegment);
PROXIMA_BE_ERROR_CODE_DECLARE(InvalidRevision);
PROXIMA_BE_ERROR_CODE_DECLARE(InvalidFeature);
PROXIMA_BE_ERROR_CODE_DECLARE(MismatchedSchema);
PROXIMA_BE_ERROR_CODE_DECLARE(MismatchedMagicNumber);
PROXIMA_BE_ERROR_CODE_DECLARE(MismatchedIndexColumn);
PROXIMA_BE_ERROR_CODE_DECLARE(MismatchedForward);
PROXIMA_BE_ERROR_CODE_DECLARE(MismatchedDimension);
PROXIMA_BE_ERROR_CODE_DECLARE(MismatchedDataType);

// 3000~3999 [Meta]
PROXIMA_BE_ERROR_CODE_DECLARE(UpdateStatusField);
PROXIMA_BE_ERROR_CODE_DECLARE(UpdateRevisionField);
PROXIMA_BE_ERROR_CODE_DECLARE(UpdateCollectionUIDField);
PROXIMA_BE_ERROR_CODE_DECLARE(UpdateIndexTypeField);
PROXIMA_BE_ERROR_CODE_DECLARE(UpdateDataTypeField);
PROXIMA_BE_ERROR_CODE_DECLARE(UpdateParametersField);
PROXIMA_BE_ERROR_CODE_DECLARE(UpdateRepositoryTypeField);
PROXIMA_BE_ERROR_CODE_DECLARE(UpdateColumnNameField);
PROXIMA_BE_ERROR_CODE_DECLARE(ZeroDocsPerSegment);
PROXIMA_BE_ERROR_CODE_DECLARE(UnsupportedConnection);

// 4000~4999 [Index]
PROXIMA_BE_ERROR_CODE_DECLARE(DuplicateCollection);
PROXIMA_BE_ERROR_CODE_DECLARE(DuplicateKey);
PROXIMA_BE_ERROR_CODE_DECLARE(InexistentCollection);
PROXIMA_BE_ERROR_CODE_DECLARE(InexistentColumn);
PROXIMA_BE_ERROR_CODE_DECLARE(InexistentKey);
PROXIMA_BE_ERROR_CODE_DECLARE(SuspendedCollection);
PROXIMA_BE_ERROR_CODE_DECLARE(LostSegment);
PROXIMA_BE_ERROR_CODE_DECLARE(EmptyLsnContext);
PROXIMA_BE_ERROR_CODE_DECLARE(ExceedRateLimit);

// 5000~5999 [Query]
PROXIMA_BE_ERROR_CODE_DECLARE(UnavailableSegment);
PROXIMA_BE_ERROR_CODE_DECLARE(OutOfBoundsResult);
PROXIMA_BE_ERROR_CODE_DECLARE(UnreadyQueue);
PROXIMA_BE_ERROR_CODE_DECLARE(ScheduleError);
PROXIMA_BE_ERROR_CODE_DECLARE(UnreadableCollection);
PROXIMA_BE_ERROR_CODE_DECLARE(TaskIsRunning);

// NOTICE
// 10000~19999 [SDK]
// 20000~29999 [Repository]

}  // end namespace be
}  // end namespace proxima
