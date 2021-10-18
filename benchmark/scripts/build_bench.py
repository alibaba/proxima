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
from datetime import datetime
import json
import shutil
import sys
import logging
from concurrent.futures import as_completed
from concurrent.futures.thread import ThreadPoolExecutor
from optparse import OptionParser
from urllib.parse import urlparse

from google.protobuf.json_format import MessageToDict

from pyproximase import Client as SEClient
from pyproximase import *

from common.proxima_se_repo import MysqlRepoSource
from common.proxima_se_service import *

from urllib import parse

"""
Requirements:
    1. Above Python3.6
Test Scenario:
------
Step 1: Prepared Repository for ProximaSE
        Requirements:
          i: table should have first column named by id with property auto_increment
         ii: vector column should be prefixed by vector
        iii: all columns treated as forward attributes except vector and id
Step 2: Clone source code of ProximaSE
        cd (source of ProximaSE)
Step 3: Build ProximaSE
        mkdir build; cd build; cmake ../; make -j
Step 4: Run Bench tools
        cd benchmark; pip install -r requirements.txt; PYTHONPATH=$(pwd) python scripts/build_bench.py
------
"""


class BenchContext(object):
    def __init__(self, output_dir, kwargs):
        self._output = os.path.realpath(output_dir)
        self._kwargs = kwargs

        self._log_dir = kwargs.log_dir if kwargs.log_dir else os.path.join(self._output, 'logs')
        self._indices_dir = kwargs.index_directory if kwargs.index_directory else os.path.join(
            self._output, 'indices')

        self._meta_uri = kwargs.meta_uri
        self._meta_dir = None
        if self._meta_uri:
            uri = urlparse(self._meta_uri)
            if uri and uri.scheme.lower() == 'sqlite' and uri.path:
                self._meta_dir = os.path.dirname(uri.path)
                if self._meta_dir == "/":
                    logging.error("Can't create meta to root directory '/'")
                    self._meta_dir = None
        else:
            self._meta_dir = os.path.join(self._output, 'meta')
            self._meta_uri = f'sqlite://{self._meta_dir}/meta.sqlite'

        self._repository = None
        self._table = kwargs.table

        self._dimension = int(kwargs.dimension)

        if kwargs.jdbc:
            path = urlparse(kwargs.jdbc).path
            if path and len(path) != 0:
                self._repository = path.strip("/").rstrip("/")

        self._proxima_se_conf = None
        self._repository_conf = None
        self._report = None

    def output(self):
        return self._output

    def options(self):
        return self._kwargs

    def repository(self):
        return self._repository

    def table(self):
        return self._table

    def counts(self):
        return int(self._kwargs.counts) if self._kwargs.counts else sys.maxsize

    def meta_dir(self):
        return self._meta_dir

    def indices_dir(self):
        return self._indices_dir

    def log_dir(self):
        return self._log_dir

    def proxima_se_log_dir(self):
        return os.path.join(self.log_dir(), "be")

    def repository_log_dir(self):
        return os.path.join(self.log_dir(), "repo")

    def timeout_in_seconds(self):
        return int(self._kwargs.timeout)

    def output_flush_interval(self):
        return float(self._kwargs.interval)

    def config_dir(self):
        return os.path.join(self._output, "conf")

    def proxima_se_config(self):
        if self._proxima_se_conf:
            return MessageToDict(self._proxima_se_conf)
        return {}

    def repository_config(self):
        return self._repository

    def proxima_se_config_file(self):
        return os.path.join(self.config_dir(), 'proxima_se.conf')

    def proxima_se_repo_config_file(self):
        return os.path.join(self.config_dir(), 'mysql_repo.conf')

    def proxima_se_admin_address(self):
        return ['127.0.0.1', self._proxima_se_conf.common_config.grpc_listen_port]

    def max_docs_per_segment(self):
        return int(self._kwargs.max_docs_per_segment)

    def dimension(self):
        return self._dimension

    def report_file(self):
        return self._report

    def _init_bench_config(self):
        self._proxima_se_conf = ProximaSE.build_config(log_directory=self.proxima_se_log_dir(),
                                                       grpc_port=int(self._kwargs.grpc_port),
                                                       http_port=int(self._kwargs.http_port),
                                                       index_build_threads=int(self._kwargs.index_build_threads),
                                                       index_build_qps=int(self._kwargs.index_build_qps),
                                                       index_directory=self.indices_dir(),
                                                       meta_uri=self._meta_uri)

        self._repository_conf = ProximaSEMysqlRepo.build_config(log_directory=self.repository_log_dir(),
                                                                index_port=int(self._kwargs.grpc_port),
                                                                repository_name=self._repository)
        if self._proxima_se_conf and self._repository_conf:
            ProximaSEService.write_config_file(self.proxima_se_config_file(), self._proxima_se_conf)
            ProximaSEService.write_file(self.proxima_se_repo_config_file(), self._repository_conf)
            return True
        return False

    @staticmethod
    def _create_directory(directory):
        if not os.path.exists(directory):
            os.makedirs(directory)

    def _init_output_dir(self):
        self._create_directory(self.output())
        if not self.meta_dir():
            return False
        logging.info(f'Create meta directory {self.meta_dir()}')
        self._create_directory(self.meta_dir())
        logging.info(f'Create indices directory {self.indices_dir()}')
        self._create_directory(self.indices_dir())
        logging.info(f'Create log directory {self.log_dir()}')
        self._create_directory(self.log_dir())
        self._create_directory(self.proxima_se_log_dir())
        self._create_directory(self.repository_log_dir())
        logging.info(f'Create config directory {self.config_dir()}')
        self._create_directory(self.config_dir())
        return True

    def _init_report(self):
        if self._kwargs.report:
            if os.path.exists(os.path.dirname(os.path.realpath(self._kwargs.report))):
                self._report = self._kwargs.report
            else:
                logging.error(f'Can not write report file: {self._kwargs.report}')
                return False
        else:
            self._report = os.path.join(self.output(), 'report.json')
        return True

    def init(self):
        if not self._init_report():
            return False

        if not self._init_output_dir():
            logging.error(f'Initialize output directory({self.output()}) failed')
            return False

        if not self._init_bench_config():
            logging.error(f'Can not create bench configs')
            return False
        return True

    @staticmethod
    def _cleanup_dir(directory):
        if os.path.isdir(directory):
            shutil.rmtree(directory)

    def cleanup(self):
        if self._kwargs.cleanup:
            self._cleanup_dir(self.output())
            self._cleanup_dir(self.meta_dir())
            self._cleanup_dir(self.indices_dir())
            self._cleanup_dir(self.log_dir())
            self._cleanup_dir(self.config_dir())

        return True


class ProximaSEBuilds(object):
    def __init__(self, roots):
        if not roots:
            roots = os.getenv("PROXIMA_SE_BUILD_ROOT")
            if not roots:
                roots = os.path.realpath(os.path.join(os.getcwd(), "..", "build"))
                if not os.path.isdir(roots):
                    roots = os.path.realpath(os.path.join(os.getcwd(), "..", "cmake-build-debug"))
        self._build_roots = roots
        logging.info(f'Locate Build Root of ProximaSE: {roots}')

    def proxima_se_binary(self):
        return os.path.join(self._build_roots, 'bin', 'proxima_se')

    def proxima_repo_binary(self):
        return os.path.join(self._build_roots, 'bin', 'mysql_repository')

    def is_valid(self):
        return os.path.isdir(self._build_roots) and os.path.isfile(self.proxima_se_binary()) and \
               os.path.isfile(self.proxima_repo_binary())


def _timeout_monitor(stopped, timeout, interval):
    logging.info(f'Timeout monitor started, sleep for {timeout} seconds')
    slept = 0
    while not stopped() and slept < timeout:
        time.sleep(interval)
        slept += interval

    logging.info(f'Timeout monitor stopped')
    if slept < timeout:
        return {}
    else:
        return {"timeout": True}


def _service_stopped_monitor(stopped, interval, *args):
    logging.info(f'Service monitor started, interval({interval})')
    while not stopped():
        services = map(lambda svc: svc.service_name(), filter(lambda svc: not svc.running(), args))
        for service in services:
            logging.info(f'{service} is not running')
            return {"stopped": service}
        time.sleep(interval)
    logging.info(f'Service monitor stopped')
    return {}


def _progress_monitor(stopped, callback, total, interval, notifier):
    logging.info(f'Start progress monitor, total({total})')
    start = time.monotonic()
    progress = 0.0
    last = [0, 0.0]
    processed = 0
    while not stopped():
        processed = callback()
        if processed < 0:
            processed = 0
            logging.info("Can't get processed from callback")
            break
        progress = round(float(processed) / total * 100, 4)
        seconds = time.monotonic() - start
        incremental = [processed - last[0], seconds - last[1]]
        last = [processed, seconds]
        if progress > 100.00:
            progress = 100.00
        print("Processed: %02.2f%%, QPS %d/S, RTQPS %d/S" % (
            progress, int(processed / seconds), int(incremental[0] / incremental[1])))
        notifier(progress, int(processed / seconds), round(seconds, 2), int(incremental[0] / incremental[1]))
        if processed >= total:
            print("Processed: 100%%, QPS %d/S, RTQPS %d/S" % (
                int(processed / seconds), int(incremental[0] / incremental[1])))
            notifier(100.00, int(processed / seconds), round(seconds, 2), int(incremental[0] / incremental[1]))
            progress = 100.00
            break
        else:
            time.sleep(interval)
    seconds = time.monotonic() - start
    logging.info("Progress monitor finished")
    return {"total": processed, "progress": progress, "seconds": int(seconds), 'qps': int(processed / seconds)}


class ProximaSEBuildBench(object):
    _filter_columns = ('id', 'vector')

    def __init__(self, output_dir, arg_options):
        self._builds = ProximaSEBuilds(arg_options.build_root)
        self._context = BenchContext(output_dir, arg_options)

        self._source = None
        self._repository = None
        self._proxima_se = None
        self._client = None
        self._last_progress = -1.0
        self._summary = {"report": self._context.output(),
                         "progress_header": ['Progress', 'QPS(AVG)', 'Seconds', 'QPS(RT)', 'IndexSize', 'CPU',
                                             'MEM(GB)', 'Time'],
                         "progress_table": []}
        self._pool = ThreadPoolExecutor(max_workers=5, thread_name_prefix='BenchMonitors')
        self._futures = []

    @staticmethod
    def human_number(num):
        if num > 1000000000:
            return f'{round(float(num) / 1000000000, 2)}B'
        elif num > 1000000:
            return f'{round(float(num) / 1000000, 2)}M'
        elif num > 1000:
            return f'{round(float(num) / 1000, 2)}T'
        return str(num)

    @staticmethod
    def summary_report(progress, report_file):
        try:
            with open(report_file, 'r') as report_fd:
                report = json.load(report_fd)
                threads = report['config']['proxima_se']['indexConfig']['buildThreadCount']
                print("Threads", "Total", *report['progress_header'])
                for idx in progress:
                    if 0 <= idx < len(report['progress_table']):
                        print(threads,
                              ProximaSEBuildBench.human_number(
                                  int(report['total'] / 100 * report['progress_table'][idx][0])),
                              *report['progress_table'][idx])
                    else:
                        break
        except Exception as e:
            logging.error(e)

    @staticmethod
    def summary_reports(progress, interval, *reports):
        if interval != 0:
            items = [int(i) for i in range(0, 10000, interval)]
        elif progress:
            items = [int(p) for p in progress.split(',')]
        else:
            logging.error("Failed to get interested progress")
            return

        for report in reports:
            ProximaSEBuildBench.summary_report(items, report)

    def init(self):
        if not self._builds.is_valid():
            logging.error("Proxima SE build is invalid, lost binary of proxima_se or mysql_repository")
            return False
        if not self._context.init():
            logging.error("Init bench context failed")
            return False

        self._source = MysqlRepoSource(self._context.options().jdbc, self._context.options().table)
        if not self._source.is_valid():
            logging.error(
                f'Can not init repository with jdbc: {self._context.options().jdbc}, '
                f'table: {self._context.options().table}')
            return False

        self._repository = ProximaSEMysqlRepo(self._builds.proxima_repo_binary(),
                                              self._context.proxima_se_repo_config_file(),
                                              self._context.log_dir())

        self._proxima_se = ProximaSE(self._builds.proxima_se_binary(),
                                     self._context.proxima_se_config_file(),
                                     self._context.log_dir())

        self._host, self._port = self._context.proxima_se_admin_address()

        return self._repository.init() and self._proxima_se.init()

    def cleanup(self):
        if self._proxima_se:
            self._proxima_se.cleanup()
        if self._repository:
            self._repository.cleanup()
        if self._context:
            self._context.cleanup()
        return True

    def _max_lsn(self):
        try:
            status, collection = self._client.describe_collection(self._context.table())
            if not status.ok():
                print(status)
                return -1
            logging.debug(collection)
            return collection.latest_lsn_context.lsn
        except Exception as e:
            logging.error("BRPC Exception")
            logging.error(e)
        return -1

    def _collection_docs(self):
        status, stats = self._client.stats_collection(self._context.table())
        if status.ok():
            return stats.total_doc_count
        else:
            logging.debug(status)
        return -1

    def _collection_index_size(self):
        status, stats = self._client.stats_collection(self._context.table())
        if status.ok():
            return stats.total_index_file_size
        else:
            logging.debug(status)
        return -1

    def _report_progress(self, progress, qps, seconds, rtqps):
        if progress >= 100 or progress - self._last_progress > 0.1:
            stats = self._proxima_se.stats()
            # noinspection PyTypeChecker
            self._summary['progress_table'].append(
                [progress, qps, seconds, rtqps, self._collection_index_size(), stats.cpu(), stats.memory_in_gb(),
                 datetime.now().strftime("%Y%m%d_%H:%M:%S")])
            self._last_progress = progress

    def _start_monitors(self):
        total = min(self._source.counts(), self._context.counts())
        return [self._pool.submit(_timeout_monitor, lambda: self.has_monitor_finished(),
                                  self._context.timeout_in_seconds(), 5),
                self._pool.submit(_service_stopped_monitor, lambda: self.has_monitor_finished(), 1,
                                  self._repository, self._proxima_se),
                self._pool.submit(_progress_monitor, lambda: self.has_monitor_finished(),
                                  lambda: self._collection_docs(), total, self._context.output_flush_interval(),
                                  lambda progress, qps, seconds, rtqps: self._report_progress(progress, qps, seconds,
                                                                                              rtqps))]

    def start(self):
        if self._proxima_se.start():
            time.sleep(10)
            self._client = SEClient(host=self._host, port=self._port)
            self.sync_schema()
            time.sleep(1)
            if self._repository.start():
                self._futures = self._start_monitors()
                return True
        return False

    def stop(self):
        for done in as_completed(self._futures):
            self._summary.update(done.result())
        self._pool.shutdown()
        return self._proxima_se.stop() and self._repository.stop()

    def _valid_schema(self):
        schema = self._source.schema()
        columns = [field[0] for field in schema]
        try:
            columns.index("id")
            columns.index("vector")
        except ValueError:
            return False
        return True

    def _build_repository_config(self):
        o = parse.urlparse(self._source.jdbc_str())
        return DatabaseRepository(self._context.repository(), self._source.jdbc_str(),
                                  self._context.table(), o.username, o.password)

    def _build_forward_metas(self, schema):
        forwards = filter(lambda field: not field[0] in self._filter_columns, schema)
        return [field[0] for field in forwards]

    def _build_column_metas(self):
        # Init column meta
        return [IndexColumnParam(self._filter_columns[1], self._context.dimension())]

    def _build_collection_meta(self, schema):
        # Init collection meta
        return CollectionConfig(self._context.table(),
                                self._build_column_metas(),
                                self._build_forward_metas(schema),
                                self._build_repository_config(),
                                self._context.max_docs_per_segment())

    def sync_schema(self):
        if self._proxima_se.running() and self._valid_schema():
            meta = self._build_collection_meta(self._source.schema())
            status = self._client.create_collection(meta)
            assert status.ok()
            _, collection = self._client.describe_collection(meta.collection_name)
            self._summary['collection'] = str(collection)
            return True
        else:
            logging.error("Can't sync table to ProximaSE")
        return False

    def has_monitor_finished(self):
        logging.debug("Enter")
        for _ in filter(lambda future: future.done() or future.cancelled(), self._futures):
            return True
        logging.debug("Exist")
        return False

    def wait_finish(self):
        logging.debug("Enter")
        for _ in as_completed(self._futures):
            # self._summary.update(done.result())
            break
        logging.debug("Exist")
        return True

    def summary(self):
        temp = self._summary
        temp['config'] = {'proxima_se': self._context.proxima_se_config(),
                          'repository': self._context.repository_config()}
        return temp

    def output_report(self):
        with open(self._context.report_file(), 'w+') as out:
            json.dump(self.summary(), out, indent=4)


def opt_parser():
    arg_parser = OptionParser()
    arg_parser.add_option('--build_root', dest='build_root', default=None,
                          help="The build directory of ProximaSE, default value: [ENV variable PROXIMA_SE_BUILD_ROOT "
                               "or '$(pwd)/../build']")
    arg_parser.add_option('--repo', dest='jdbc', default=None,
                          help='The source of repository, represented by jdbc string')
    arg_parser.add_option('-t', '--table', dest='table', default=None, help='Target table sync to ProximaSE')
    arg_parser.add_option('--counts', dest='counts', default=None,
                          help='The number of records will be sync to ProximaSE')
    arg_parser.add_option('--log_dir', dest='log_dir', default=None, help='Log directory, default is logs')
    arg_parser.add_option('--grpc_port', dest='grpc_port', default=GRPC_PORT,
                          help=f'Proxima SE GRPC service port, default {GRPC_PORT}')
    arg_parser.add_option('--http_port', dest='http_port', default=HTTP_PORT,
                          help=f'Proxima SE GRPC service port, default {HTTP_PORT}')
    arg_parser.add_option('--index_build_threads', dest='index_build_threads', default=10,
                          help='Index Agent build threads count, default is 10')
    arg_parser.add_option('--index_build_qps', dest='index_build_qps', default=1000000,
                          help='Threshold QPS of incremental records, default 1000000')
    arg_parser.add_option('--index_directory', dest='index_directory', default=None,
                          help="Index directory, where indices located, default is 'indices'")
    arg_parser.add_option('--max_docs_per_segment', dest='max_docs_per_segment', default=1000000,
                          help='Max records per segment, default 1000000')
    arg_parser.add_option('--dimension', dest='dimension', default=512,
                          help='Dimension of vector')
    arg_parser.add_option('--meta_uri', dest='meta_uri', default=None, help='URI of meta store, meta/meta.sqlite')
    arg_parser.add_option('-o', '--output_dir', dest='output', default=None,
                          help='Output directory, default random directory')
    arg_parser.add_option('--cleanup', dest='cleanup', action='store_true', default=False,
                          help='Cleanup all the outputs after finished')
    arg_parser.add_option('--timeout', dest='timeout', default=86400,
                          help='Timeout in seconds, default is 86400')
    arg_parser.add_option('--interval', dest='interval', default=5.0,
                          help='Progress flush interval, default is 5 seconds')
    arg_parser.add_option('--report', dest='report', default=None,
                          help='Report file, default write to [output]/report.json')
    arg_parser.add_option('--summary_progress', dest='summary', default=None,
                          help="Extract interested (approximate Progress, separated by ',') progress records from "
                               "reports")
    arg_parser.add_option('--summary_interval', dest='summary_interval', default=0,
                          help="Extract interested progress records from reports")
    return arg_parser


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
    if options.summary or int(options.summary_interval) != 0:
        ProximaSEBuildBench.summary_reports(options.summary, int(options.summary_interval), *args)
        exit(0)

    logging.info(f'Arguments: {options}')
    output = options.output if options.output else tempfile.mktemp()
    logging.info(f'Run tools with output directory: {output}')

    code = 0
    task = ProximaSEBuildBench(output, options)
    if task.init():
        logging.info("Init bench tools succeed")
        task.start()
        task.wait_finish()
        task.stop()
        task.output_report()
    else:
        logging.error("Failed to init bench tools")
        code = 1

    summary = task.summary()
    del summary['progress_header']
    del summary['progress_table']
    logging.info(task.summary())
    task.cleanup()

    exit(code)
