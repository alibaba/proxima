#!/usr/bin/env python
# -*- coding: utf-8 -*-
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
# Proxima SE benchmark toolkits
#

import logging
import random
import threading
import time
import logging
from concurrent.futures import as_completed
from concurrent.futures.thread import ThreadPoolExecutor
from optparse import OptionParser
import asyncio

from pyproximase import *
from common.proxima_se_query import *



class Counter(object):
    def __init__(self):
        self._counts = []

    def counts(self):
        return len(self._counts)

    def _sum(self):
        return sum(self._counts)

    def avg(self):
        c = self.counts()
        return int(self._sum() / c) if c != 0 else 0

    def min(self):
        return min(self._counts) if self.counts() != 0 else 0

    def max(self):
        return max(self._counts) if self.counts() != 0 else 0

    def clear(self):
        self._counts = []

    def append(self, c):
        self._counts.append(c)



class PerfRecord(object):
    def __init__(self):
        self.succeed = 0
        self.failed = 0
        self.qps = 0
        self.latency_avg = 0
        self.latency_min = 0
        self.latency_max = 0



class BenchContext(object):
    def __init__(self, opts):
        self._options = opts
        self._collection = None
        self._queries = None

        self._lock = threading.Lock()
        self._counter = Counter()
        self._failed = 0
        self._start = time.monotonic()
        self.stopped = False

    def _proto(self):
        return 'http' if self._options.http else 'grpc'

    def init(self):
        logging.info("Init BenchContext.")
        if self.is_valid():
            logging.info("BenchContext have been initialized.")
            return True
        client = self.create_client()

        # Init collection
        status, self._collection = client.describe_collection(self._options.collection)
        if not status.ok():
            return False

        # Init query set
        self._queries = MysqlRawQuerySetCache(self._options.query, self._options.table,
                                              self._collection)
        self._queries.init()
        return self._queries.counts() != 0

    def cleanup(self):
        self._options = None
        self._collection = None
        self._queries = None
        logging.info("Cleanup BenchContext.")
        pass

    def is_valid(self):
        return self._options and self._collection and self._queries

    def query(self):
        return random.choice(self._queries)

    def create_client(self):
        return Client(*self._options.host.split(":"), self._proto())

    def create_async_client(self):
        if self._options.http:
            logging.warning('Asyncclient only support grpc')
        return AsyncClient(*self._options.host.split(":"))

    def collection(self):
        return self._collection.collection_config.collection_name

    def column(self, idx=0):
        return self._collection.collection_config.index_column_params[idx].name

    def topk(self):
        return int(self._options.topk)

    def success(self, latency):
        with self._lock:
            self._counter.append(latency)


    def failure(self, _):
        with self._lock:
            self._failed += 1

    def get_report_and_cleanup(self):
        record = PerfRecord()
        with self._lock:
            seconds = time.monotonic() - self._start
            record.succeed = self._counter.counts()
            record.failed = self._failed
            record.latency_avg = self._counter.avg()
            record.latency_min = self._counter.min()
            record.latency_max = self._counter.max()
            record.qps = int(record.succeed / seconds)
            self._counter.clear()
            self._failed = 0
            self._start = time.monotonic()
        return record


def opt_parser():
    arg_parser = OptionParser()
    arg_parser.add_option("--query", type=str, dest='query', default="",
                          help="The source of query, which should be DB connection")
    arg_parser.add_option("--table", type=str, dest="table", default="",
                          help="Table name")
    arg_parser.add_option("--collection", type=str, dest="collection", default="",
                          help="Proxima SE collection")
    arg_parser.add_option("--host", type=str, dest="host", default="",
                          help="Proxima SE grpc service")
    arg_parser.add_option("--topk", type=str, dest="topk", default='200',
                          help="Proxima SE topk, separated by ','")
    arg_parser.add_option("--threads", type=str, dest="threads", default='1',
                          help="Number of threads to run test")
    arg_parser.add_option('--http', dest='http', action='store_true', default=False,
                          help='Using http proto to test, default is grpc')
    arg_parser.add_option('--timeout', type=int, dest='timeout', default=60,
                          help='Total seconds for test')
    arg_parser.add_option('--interval', type=int, dest='interval', default=2,
                          help='Seconds for summary duration')
    arg_parser.add_option('--async_batch', type=int, dest='async_batch', default=0,
                          help='If greater than 0, AsyncClient will be used. '
                          'It should not be set too big(e.g. > 20), '
                          'as python sdk is dominated by protobuf (de)serializing.')
    return arg_parser


class BenchWorker(object):
    def __init__(self, bench_context):
        self.context = bench_context
        self.async_batch = self.context._options.async_batch
        if self.async_batch:
            self.client = context.create_async_client()
            self.loop = asyncio.get_event_loop()
        else:
            self.client = context.create_client()
        self.collection = context.collection()
        self.column = context.column()
        self.topk = context.topk()

    def __call__(self, *_, **__):
        def sync_query():
            while not self.context.stopped:
                start = time.time()
                status, resp = self.client.query(self.collection, 
                                                 self.column, 
                                                 self.context.query(),
                                                 topk=self.topk,
                                                 data_type=DataType.VECTOR_FP32,
                                                 batch_count=1,
                                                 dimension=self.context._queries.dimension())
                elapse = int((time.time() - start) * 1000)
                if status.ok():
                    self.context.success(elapse)
                else:
                    print(status, resp)
                    self.context.failure(elapse)
        async def async_query():
            while not self.context.stopped:
                start = time.time()
                count = self.async_batch
                queries = []
                for i in range(count):
                    queries.append(self.client.query(self.collection, 
                                                     self.column, 
                                                     self.context.query(),
                                                     topk=self.topk,
                                                     data_type=DataType.VECTOR_FP32,
                                                     batch_count=1,
                                                     dimension=self.context._queries.dimension()))
                responses = await asyncio.gather(*queries)
                elapse = int((time.time() - start) * 1000/count)
                for status, resp in responses:
                    if status.ok():
                        self.context.success(elapse)
                    else:
                        print(status, resp)
                        self.context.failure(elapse)
        if self.async_batch:
            self.loop.run_until_complete(async_query())
        else:
            sync_query()
        return True


class ProgressMonitor(object):
    def __init__(self, bench_context, interval):
        self.context = bench_context
        self.interval = interval
        print("   Total   Succeed   Failed     QPS  Latency(AVG)  Latency(Min)  Latency(Max)")

    def __call__(self, *_, **__):
        while not self.context.stopped:
            r = self.context.get_report_and_cleanup()
            print("%8d   %7d  %7d  %6d  %12d  %12d  %12d" %(r.succeed + r.failed, r.succeed, r.failed,
                                                       r.qps, r.latency_avg, r.latency_min, r.latency_max))
            time.sleep(self.interval)
        return True


def handle_help_and_exit(arg_options, arg_parser, nargs):
    try:
        arg_parser.print_help() if nargs == 1 or arg_options.help else None
        quit()
    except AttributeError:
        pass


if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO, 
                        format="%(levelname)s %(asctime)s %(filename)s:%(lineno)d:%(funcName)s \t%(message)s")
    parser = opt_parser()
    (options, args) = parser.parse_args()
    handle_help_and_exit(options, parser, len(sys.argv))

    logging.info(f'Arguments: {options}')

    context = BenchContext(options)
    if not context.init():
        logging.error("Failed to init bench context")
        exit(1)

    threads = int(options.threads)
    pool = ThreadPoolExecutor(max_workers=threads+1, thread_name_prefix='BenchWorker')
    futures = list(map(lambda _: pool.submit(BenchWorker(context)), range(threads)))
    futures.append(pool.submit(ProgressMonitor(context, options.interval)))

    time.sleep(options.timeout)
    context.stopped = True

    for _ in as_completed(futures):
        pass
    pool.shutdown()

    exit(0)
