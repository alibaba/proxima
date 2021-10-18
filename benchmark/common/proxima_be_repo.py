#! /usr/bin/env python
# -*- coding: utf8 -*-
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#     http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# Proxima SE Repository source
#
from common.mysql_utils import MySQL


class ProximaSERepoSource(object):
    def __init__(self, jdbc):
        self._jdbc = jdbc

    def __del__(self):
        pass

    def jdbc_str(self):
        return self._jdbc

    def is_valid(self) -> bool:
        pass

    def tables(self):
        pass

    def schema(self, table):
        pass

    def counts(self, table):
        pass


class MysqlRepoSource(ProximaSERepoSource):
    def __init__(self, jdbc, table):
        ProximaSERepoSource.__init__(self, jdbc)
        self._table = table
        self._with_conn = None
        self._conn = MySQL.connect(self._jdbc)

    def __del__(self):
        pass

    def __enter__(self):
        if self._with_conn:
            self._with_conn = MySQL.connect(self._jdbc)
        return self._with_conn

    def __exit__(self, exc_type, exc_val, exc_tb):
        if self._with_conn:
            self._with_conn.close()
        self._with_conn = None

    def is_valid(self) -> bool:
        return True if self._conn and len(self.schema(self._table)) != 0 else False

    def tables(self):
        return MySQL.tables(self._conn)

    def schema(self, table=None):
        table = self._table if not table else table
        return MySQL.schema(self._conn, table)

    def counts(self, table=None):
        table = self._table if not table else table
        return MySQL.counts(self._conn, table)
