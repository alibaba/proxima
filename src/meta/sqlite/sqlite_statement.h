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

#pragma once

#include <sqlite3.h>
#include <functional>
#include <memory>
#include <string>

namespace proxima {
namespace be {
namespace meta {
namespace sqlite {

//! Predefine class
class Statement;
//! Alias for StatementPtr
using StatementPtr = std::shared_ptr<Statement>;


/*!
 * Statement object
 */
class Statement {
 public:
  //! Constructors
  Statement();
  Statement(std::string database, const char *sql);
  Statement(std::string database, std::string sql);
  Statement(Statement &&stmt) noexcept;
  Statement(const Statement &stmt) = delete;

  //! Destructor
  ~Statement();

 public:
  //! Initialize Statement, 0 for success, otherwise failed
  int initialize();

  //! Cleanup statement, 0 for success, otherwise failed
  int cleanup();

  //! Execute statement, 0 for success, otherwise failed
  //! @param binder: bind the params to statements
  //! @param fetcher: fetch the records returned from sqlite_step
  //! @param retry: the times retry exec sql
  int exec(const std::function<int(sqlite3_stmt *)> &binder = nullptr,
           const std::function<int(sqlite3_stmt *)> &fetcher = nullptr,
           uint32_t retry = 1);

  //! Prepare sql
  int prepare_sql(const std::string &sql);

 private:
  //! Reset the values banded to statement, 0 for success, otherwise failed
  int reset();

  //! cleanup
  int do_cleanup();

  //! compile sql
  int compile_sql();

 private:
  //! the path of databases
  std::string database_{};

  //! database connection
  sqlite3 *connection_{nullptr};

  //! sql
  std::string sql_{};

  //! statement of sqlite
  sqlite3_stmt *statement_{nullptr};
};


}  // namespace sqlite
}  // namespace meta
}  // namespace be
}  // namespace proxima
