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

#include <gtest/gtest.h>
#include "meta/sqlite/sqlite_statement.h"
#include "temp_file_inl.h"

using namespace proxima::be::meta::sqlite;

TEST(StatementTest, TestCreateTableCollection) {
  ScopeFile database(TempFile());

  Statement statement(database.file_,
                      "CREATE TABLE IF NOT EXISTS columns ( \n"
                      "    id INTEGER PRIMARY KEY AUTOINCREMENT, \n"
                      "    coll_uid TEXT NOT NULL, \n"
                      "    coll_uuid TEXT NOT NULL UNIQUE, \n"
                      "    name TEXT NOT NULL, \n"
                      "    alias TEXT NOT NULL, \n"
                      "    index_type INTEGER, \n"
                      "    data_type INTEGER, \n"
                      "    parameters TEXT DEFAULT '' \n"
                      ");");

  ASSERT_EQ(statement.initialize(), 0);
  // ASSERT_EQ(statement.exec(), 0);
  ASSERT_EQ(statement.cleanup(), 0);
}

TEST(StatementTest, TestCreateTable) {
  ScopeFile database(TempFile());
  Statement statement(database.file_,
                      "CREATE TABLE IF NOT EXISTS columns ( \n"
                      "    id INTEGER PRIMARY KEY AUTOINCREMENT, \n"
                      "    coll_uid TEXT NOT NULL, \n"
                      "    coll_uuid TEXT NOT NULL UNIQUE, \n"
                      "    name TEXT NOT NULL, \n"
                      "    alias TEXT NOT NULL, \n"
                      "    index_type INTEGER, \n"
                      "    data_type INTEGER, \n"
                      "    parameters TEXT DEFAULT '' \n"
                      ");");

  ASSERT_EQ(statement.initialize(), 0);
  ASSERT_EQ(statement.exec(), 0);

  {
    // Can't compile .tables;
    ASSERT_TRUE(statement.prepare_sql(".tables") != 0);
    // Wrong table name
    ASSERT_TRUE(statement.prepare_sql("select * from columns1;") != 0);
    // Can't exec sql
    ASSERT_TRUE(statement.exec() != 0);
  }

  {
    ASSERT_EQ(statement.prepare_sql("select * from columns;"), 0);
    ASSERT_EQ(statement.exec(), 0);
  }

  {
    std::string insert(
        "INSERT INTO columns(coll_uid, coll_uuid, name, alias, index_type, "
        "data_type, "
        "parameters) VALUES('uid', ?1, 'name', 'alias', 1, 2, 'params');");
    ASSERT_EQ(statement.prepare_sql(insert), 0);
    ASSERT_EQ(statement.exec([](sqlite3_stmt *stmt) -> int {
      sqlite3_bind_text(stmt, 1, "uuid", 4, nullptr);
      return 0;
    }),
              0);
  }

  {
    std::string insert("update columns set coll_uid = ?1 where name=?2;");
    ASSERT_EQ(statement.prepare_sql(insert), 0);
    ASSERT_EQ(statement.exec([](sqlite3_stmt *stmt) -> int {
      sqlite3_bind_text(stmt, 1, "uuid1", 5, nullptr);
      sqlite3_bind_text(stmt, 2, "name", 4, nullptr);
      return 0;
    }),
              0);
  }

  {
    ASSERT_EQ(statement.prepare_sql("select * from columns;"), 0);

    int row_count = 0, column_count = 0;
    int64_t id = 0, data_type = 0;
    std::string uuid;
    ASSERT_EQ(statement.exec(nullptr,
                             [&row_count, &column_count, &id, &data_type,
                              &uuid](sqlite3_stmt *stmt) -> int {
                               row_count++;
                               column_count = sqlite3_column_count(stmt);
                               id = sqlite3_column_int64(stmt, 0);
                               data_type = sqlite3_column_int(stmt, 6);
                               uuid.assign(reinterpret_cast<const char *>(
                                   sqlite3_column_text(stmt, 2)));
                               return 0;
                             }),
              0);

    ASSERT_EQ(row_count, 1);
    ASSERT_EQ(column_count, 8);
    ASSERT_EQ(id, 1);
    ASSERT_EQ(data_type, 2);
    ASSERT_EQ(uuid, "uuid");
  }

  ASSERT_EQ(statement.cleanup(), 0);

  // Double cleanup
  ASSERT_EQ(statement.cleanup(), 0);
}
