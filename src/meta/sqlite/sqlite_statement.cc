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
 *
 *   \author   guonix
 *   \date     Nov 2020
 *   \brief
 */

#include "sqlite_statement.h"
#include "common/error_code.h"
#include "common/logger.h"

namespace proxima {
namespace be {
namespace meta {
namespace sqlite {

Statement::Statement() = default;

Statement::Statement(std::string database, const char *sql)
    : database_(std::move(database)), sql_(sql) {}

Statement::Statement(std::string database, std::string sql)
    : database_(std::move(database)), sql_(std::move(sql)) {}

Statement::Statement(Statement &&stmt) noexcept
    : database_(std::move(stmt.database_)),
      connection_(stmt.connection_),
      sql_(std::move(stmt.sql_)),
      statement_(stmt.statement_) {
  stmt.connection_ = nullptr;
  stmt.statement_ = nullptr;
}

Statement::~Statement() {
  cleanup();
}

int Statement::initialize() {
  if (connection_ != nullptr && statement_ != nullptr) {
    return 0;
  }

  if (connection_ != nullptr || statement_ != nullptr) {
    LOG_ERROR(
        "Statement have been initialized partially, invoke cleanup, before "
        "initialize");
    return PROXIMA_BE_ERROR_CODE(RuntimeError);
  }

  int code = sqlite3_open_v2(
      database_.c_str(), &connection_,
      SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX,
      nullptr);
  if (code == SQLITE_OK) {
    code = compile_sql();
  } else {
    LOG_ERROR("Failed to open sqlite db, msg[%s]", sqlite3_errstr(code));
  }

  if (code != SQLITE_OK) {
    do_cleanup();
  }
  return code;
}

int Statement::cleanup() {
  return do_cleanup();
}

int Statement::exec(const std::function<int(sqlite3_stmt *)> &binder,
                    const std::function<int(sqlite3_stmt *)> &fetcher,
                    uint32_t retry) {
  if (!statement_ || reset() != SQLITE_OK) {
    return PROXIMA_BE_ERROR_CODE(RuntimeError);
  }

  // Bind values
  int code = binder ? binder(statement_) : 0;
  if (code != 0) {
    LOG_ERROR("Failed to bind values to statement, code[%d]", code);
  } else {
    do {
      code = sqlite3_step(statement_);
      switch (code) {
        case SQLITE_ROW:  // have result
          if (fetcher && fetcher(statement_) != 0) {
            // Can't fetch the result, break loop
            code = PROXIMA_BE_ERROR_CODE(RuntimeError);
          }
          break;
        case SQLITE_BUSY:  // try again
          if (retry-- == 0) {
            code = PROXIMA_BE_ERROR_CODE(RuntimeError);
          }
          break;
        case SQLITE_DONE:  // finished
        case SQLITE_ERROR:
        case SQLITE_MISUSE:
        default:
          break;
      }
    } while (code == SQLITE_ROW || code == SQLITE_BUSY);
    if (code == SQLITE_DONE) {  // return 0 if and only if code == SQLITE_DONE
      code = 0;
    }
  }

  if (reset() != SQLITE_OK) {
    // Do not return error code if reset failed, block following write requests
    // to sqlite, keep proxima be alive for QueryService and IndexAgent.
    LOG_ERROR(
        "Unexpected reset statements failed, contact administrator for "
        "help.");
  }
  return code;
}

int Statement::prepare_sql(const std::string &sql) {
  if (statement_ != nullptr) {
    int code = sqlite3_finalize(statement_);
    if (code != SQLITE_OK) {
      LOG_ERROR("Failed to finalize statement. msg[%s]", sqlite3_errstr(code));
    }
    // Ignore return code of sqlite3_finalize, just reset statement_ to nullptr
    statement_ = nullptr;
  }
  sql_ = sql;
  return compile_sql();
}

int Statement::reset() {
  int code = sqlite3_reset(statement_);
  // There are nothing we can do when reset failed. all write requests to
  // sqlite will be failed
  if (code != SQLITE_OK) {
    code = sqlite3_clear_bindings(statement_);
    if (code != SQLITE_OK) {
      const char *sql = sqlite3_expanded_sql(statement_);
      LOG_ERROR("Can't reset statement. sql[%s] code[%d] what[%s]", sql, code,
                sqlite3_errstr(code));
      sqlite3_free(const_cast<char *>(sql));
    }
  }
  return code;
}

int Statement::do_cleanup() {
  int code = 0;
  if (statement_ != nullptr) {
    code = sqlite3_finalize(statement_);
    if (code != SQLITE_OK) {
      LOG_ERROR("Failed to finalize statement. msg[%s]", sqlite3_errstr(code));
    }
    // Ignore error code, just reset statement_ to nullptr (Memory Leak?)
    statement_ = nullptr;
  }

  if (connection_ != nullptr) {
    code = sqlite3_close_v2(connection_);
    if (code != SQLITE_OK) {  // Fatal Error, can't ignore this error,
      // once we set connection to nullptr,
      // cause to another unknown issue.
      LOG_ERROR(
          "Failed to close connection with sqlite database. code[%d], what[%s]",
          code, sqlite3_errstr(code));
    } else {
      connection_ = nullptr;
    }
  }
  return code;
}

int Statement::compile_sql() {
  int code = sqlite3_prepare_v2(connection_, sql_.c_str(), sql_.length(),
                                &statement_, nullptr);
  if (code == SQLITE_OK) {
    LOG_DEBUG("Prepare statement succeeded");
  } else {
    LOG_ERROR("Failed to prepare statement. sql[%s], msg[%s]", sql_.c_str(),
              sqlite3_errstr(code));
    code = PROXIMA_BE_ERROR_CODE(RuntimeError);
  }
  return code;
}

}  // namespace sqlite
}  // namespace meta
}  // namespace be
}  // namespace proxima
