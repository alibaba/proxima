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
# Proxima SE query
#
import sys
import struct

from common.mysql_utils import MySQL


def _parse_int(string):
    return int(string)


def _parse_float(string):
    return float(string)


def _parse_none(_):
    return None


class QuerySet(object):
    def __init__(self):
        pass

    def init(self) -> bool:
        pass

    def cleanup(self) -> bool:
        pass

    def counts(self) -> int:
        pass

    def queries(self, start=0, end=sys.maxsize):
        pass

    def query(self, start=0, counts=1):
        pass


class MysqlRawQuerySet(QuerySet):
    def __init__(self, uri, table, collection):
        super().__init__()
        self._uri = uri
        self._conn = MySQL.connect(uri)
        self._table = table
        self._collection = collection

    def __del__(self):
        MySQL.close(self._conn)

    def _columns(self):
        return [c.name for c in self._collection.collection_config.index_column_params]

    def counts(self) -> int:
        return MySQL.counts(self._conn, self._table)

    def queries(self, start=0, end=sys.maxsize):
        end = min(end, self.counts())
        columns = self._columns()
        if len(columns) != 1 or end <= 0:
            return []
        with self._conn.cursor() as cursor:
            result = MySQL.execute(
                cursor,
                f'select {",".join(columns)} from {self._table} where id >= {start} and id < {end + 1}')
            for query in result:
                yield query

    def query(self, start=0, counts=1):
        columns = self._columns()
        with self._conn.cursor() as cursor:
            return MySQL.execute(cursor,
                                 f'select {",".join(columns)} from {self._table} where id >= {start} limit {counts}')


class MysqlRawQuerySetCache(MysqlRawQuerySet):
    def __init__(self, uri, table, collection):
        super().__init__(uri, table, collection)
        self._queries = []
        self._use_binary = True
        self._dimension = None

    def __len__(self):
        return len(self._queries)

    def __getitem__(self, item):
        assert isinstance(item, int) and item < len(self._queries)
        return self._queries[item]

    def counts(self) -> int:
        if len(self._queries) == 0:
            return super().counts()
        return len(self._queries)

    def init(self) -> bool:
        for query in super().queries():
            query = list(map(float, query[0].split(",")))
            self._dimension = len(query)
            if self._use_binary:
                qlen = len(query)
                query = struct.pack(f'<{qlen}f', *query)
            self._queries.append(query)
        return True

    def cleanup(self) -> bool:
        self._queries = []

    def queries(self, start=0, end=sys.maxsize):
        for idx in range(start, end):
            yield self._queries[idx]

    def query(self, start=0, counts=1):
        if counts == 1 and start < len(self._queries):
            return self._queries[start]
        return self._queries[start: start+counts]

    def dimension(self):
        return self._dimension
