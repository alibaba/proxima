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
import logging
from urllib.parse import urlparse

import pymysql


class MySQL(object):
    @staticmethod
    def connect_db(uri):
        if not uri or not uri.scheme or uri.scheme.lower() != 'mysql':
            return None

        database = uri.path.strip('/').rstrip('/') if uri.path else None
        if not database or database.find('/') != -1:
            return None

        connection = None
        try:
            connection = pymysql.connect(
                host=uri.hostname,
                user=uri.username,
                passwd=uri.password,
                db=uri.path.strip('/').rstrip('/'),
                # charset='utf8mb4',
                port=uri.port
            )
        except Exception as e:
            logging.error(f'Can not connect to mysql {uri.netloc}')
        return connection

    @staticmethod
    def connect(jdbc_str):
        uri = urlparse(jdbc_str)
        return MySQL.connect_db(uri)

    @staticmethod
    def close(connection):
        connection.close()

    @staticmethod
    def execute(cursor, sql):
        cursor.execute(sql)
        return cursor.fetchall()

    @staticmethod
    def counts(conn, table):
        with conn.cursor() as cursor:
            result = MySQL.execute(cursor, f'select count(*) from {table}')
            return result[0][0] if result else 0

    @staticmethod
    def schema(conn, table):
        with conn.cursor() as cursor:
            result = MySQL.execute(cursor, f'show full columns from {table}')
            return [c[:1] for c in result]

    @staticmethod
    def tables(conn):
        with conn.cursor() as cursor:
            result = MySQL.execute(cursor, "show tables")
            data = [c for i in result for c in i]
            return data
