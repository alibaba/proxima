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
 *   \brief    Interface of error code
 */

#pragma once

#include <map>

namespace proxima {
namespace be {
namespace repository {

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
#define PROXIMA_BE_REPOSITORY_ERROR_CODE_DEFINE(__NAME__, __VAL__, __DESC__) \
  const proxima::be::repository::ErrorCode::Code ErrorCode_##__NAME__(       \
      (__VAL__), (__DESC__));                                                \
  const proxima::be::repository::ErrorCode::Code                             \
      &_ErrorCode_##__VAL__##_Register(ErrorCode_##__NAME__)

//! Proxima BE Error Code Declare
#define PROXIMA_BE_REPOSITORY_ERROR_CODE_DECLARE(__NAME__) \
  extern const proxima::be::repository::ErrorCode::Code ErrorCode_##__NAME__

//! Error code helper
#define PROXIMA_BE_REPOSITORY_ERROR_CODE(__NAME__) \
  proxima::be::repository::ErrorCode_##__NAME__

//! Build-in error code
PROXIMA_BE_REPOSITORY_ERROR_CODE_DECLARE(Success);  // Success

// 1000~1999 [Common Error]
PROXIMA_BE_REPOSITORY_ERROR_CODE_DECLARE(RuntimeError);
PROXIMA_BE_REPOSITORY_ERROR_CODE_DECLARE(LogicError);
PROXIMA_BE_REPOSITORY_ERROR_CODE_DECLARE(LoadConfig);
PROXIMA_BE_REPOSITORY_ERROR_CODE_DECLARE(ConfigError);
PROXIMA_BE_REPOSITORY_ERROR_CODE_DECLARE(InvalidArgument);
PROXIMA_BE_REPOSITORY_ERROR_CODE_DECLARE(NotInitialized);
PROXIMA_BE_REPOSITORY_ERROR_CODE_DECLARE(OpenFile);
PROXIMA_BE_REPOSITORY_ERROR_CODE_DECLARE(ReadData);
PROXIMA_BE_REPOSITORY_ERROR_CODE_DECLARE(ExceedLimit);

// //! Common Error for all module

//! Error for index module
PROXIMA_BE_REPOSITORY_ERROR_CODE_DECLARE(DuplicateCollection);

//! Error for agent module
PROXIMA_BE_REPOSITORY_ERROR_CODE_DECLARE(ExceedRateLimit);

//! Error for binlog
PROXIMA_BE_REPOSITORY_ERROR_CODE_DECLARE(ConnectMysql);
PROXIMA_BE_REPOSITORY_ERROR_CODE_DECLARE(InvalidMysqlTable);
PROXIMA_BE_REPOSITORY_ERROR_CODE_DECLARE(ExecuteMysql);
PROXIMA_BE_REPOSITORY_ERROR_CODE_DECLARE(TableNoMoreData);
PROXIMA_BE_REPOSITORY_ERROR_CODE_DECLARE(InvalidRowData);
PROXIMA_BE_REPOSITORY_ERROR_CODE_DECLARE(UnsupportedMysqlVersion);
PROXIMA_BE_REPOSITORY_ERROR_CODE_DECLARE(ExecuteSimpleCommand);
PROXIMA_BE_REPOSITORY_ERROR_CODE_DECLARE(BinlogDump);
PROXIMA_BE_REPOSITORY_ERROR_CODE_DECLARE(BinlogNoMoreData);
PROXIMA_BE_REPOSITORY_ERROR_CODE_DECLARE(InvalidMysqlResult);
PROXIMA_BE_REPOSITORY_ERROR_CODE_DECLARE(UnsupportedBinlogFormat);
PROXIMA_BE_REPOSITORY_ERROR_CODE_DECLARE(FetchMysqlResult);
PROXIMA_BE_REPOSITORY_ERROR_CODE_DECLARE(Suspended);
PROXIMA_BE_REPOSITORY_ERROR_CODE_DECLARE(InvalidCollectionConfig);
PROXIMA_BE_REPOSITORY_ERROR_CODE_DECLARE(RepeatedInitialized);
PROXIMA_BE_REPOSITORY_ERROR_CODE_DECLARE(NoInitialized);
PROXIMA_BE_REPOSITORY_ERROR_CODE_DECLARE(MismatchedVersion);

//! Error for collection
PROXIMA_BE_REPOSITORY_ERROR_CODE_DECLARE(CollectionNotExist);
PROXIMA_BE_REPOSITORY_ERROR_CODE_DECLARE(RPCFailed);
PROXIMA_BE_REPOSITORY_ERROR_CODE_DECLARE(Terminate);
PROXIMA_BE_REPOSITORY_ERROR_CODE_DECLARE(InvalidUri);
PROXIMA_BE_REPOSITORY_ERROR_CODE_DECLARE(InitChannel);
PROXIMA_BE_REPOSITORY_ERROR_CODE_DECLARE(InvalidMysqlHandler);
PROXIMA_BE_REPOSITORY_ERROR_CODE_DECLARE(InvalidLSNContext);
PROXIMA_BE_REPOSITORY_ERROR_CODE_DECLARE(NoMoreData);
PROXIMA_BE_REPOSITORY_ERROR_CODE_DECLARE(SchemaChanged);

PROXIMA_BE_REPOSITORY_ERROR_CODE_DECLARE(MismatchedSchema);
PROXIMA_BE_REPOSITORY_ERROR_CODE_DECLARE(MismatchedMagicNumber);


}  // end namespace repository
}  // namespace be
}  // end namespace proxima
