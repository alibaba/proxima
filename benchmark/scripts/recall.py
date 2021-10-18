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
import concurrent
from concurrent.futures.thread import ThreadPoolExecutor
from optparse import OptionParser

from pyproximase import *

from common.proxima_se_query import *
from common.proxima2_dataset import *


class RecallContext(object):
    def __init__(self, recall_options):
        self._options = recall_options
        self._queries = None
        self._client = None
        self._collection = None
        self._collection_stat = None
        self._topk = []

    def init(self):
        logging.info("Init BenchContext.")
        if self.is_valid():
            logging.info("BenchContext have been initialized.")
            return True
        # Init admin
        self._client = Client(*self._options.host.split(":"))
        # Init collection
        code, self._collection = self._client.describe_collection(self._options.collection)
        assert code.ok()
        # Init collection stat
        code, self._collection_stat = self._client.stats_collection(self._options.collection)
        assert code.ok()
        # Init query producer
        self._queries = MysqlRawQuerySetCache(self._options.query, self._options.table, self._collection)
        self._queries.init()
        # Init Topk
        self._topk = [int(k) for k in self._options.topk.split(",")]
        self._topk.sort()
        logging.info(f'Parsing topk {self._topk}')
        if len(self._topk) == 0:
            logging.error("Parsing topk failed")
            return False

    def is_valid(self):
        return self._options and self._queries and \
               self._client and self._collection and \
               self._collection_stat and len(self._topk) != 0

    def client(self):
        return self._client

    def topk(self):
        return self._topk

    def max_topk(self):
        return self._topk[-1]

    def gt_count(self):
        return self.max_topk()

    def host(self):
        return self._options.host

    def sample_counts(self):
        return int(self._options.counts)

    def load_gt(self):
        gt = Proxima2GT(self.gt_count(), self._options.gt)
        gt.load()
        return gt

    def query(self, idx):
        features = self._queries.query(idx)
        return list(map(float, features[0].split(",")))

    def collection(self):
        return self._collection.collection_config.collection_name

    def column(self):
        return self._collection.collection_config.index_column_params[0].name

    def ef(self):
        if 'ef_search' in self._collection.collection_config.index_column_params[0].extra_params:
            return int(self._collection.collection_config.index_column_params[0].extra_params['ef_search'])
        return 500


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
    arg_parser.add_option("--gt", type=str, dest="gt", default=None,
                          help="Proxima SE GT file")
    arg_parser.add_option("--topk", type=str, dest="topk", default='200',
                          help="Proxima SE topk, separated by ','")
    arg_parser.add_option("--counts", type=str, dest="counts", default=1,
                          help="Proxima SE query counts")
    arg_parser.add_option("--dump_mismatch", dest='dump_mismatch', action="store_true", default=False,
                          help='Dump mismatched result')
    return arg_parser


def handle_help_and_exit(arg_options, arg_parser, nargs):
    try:
        arg_parser.print_help() if nargs == 1 or arg_options.help else None
        quit()
    except AttributeError:
        pass


class ProximaSERecall(Proxima2GTRecall):
    def __init__(self, doc):
        super().__init__(doc.forward_column_values['p_key'], doc.score)
        self.doc_id = doc.primary_key


class ProximaSERecallEncoder(json.JSONEncoder):
    def default(self, x):
        if isinstance(x, ProximaSERecall):
            return {'doc_id': x.doc_id, 'pk': x.pk, 'score': x.score}
        elif isinstance(x, Proxima2GTRecord):
            return x.dumps()
        return super().default(self, x)


def _se_recall_record(r_context, feature, linear=False):
    status, response = r_context.client().query(r_context.collection(),
                                                r_context.column(),
                                                feature,
                                                topk=r_context.max_topk(),
                                                is_linear=linear, 
                                                data_type=DataType.VECTOR_FP32,
                                                batch_count=1)
    if status.ok():
        record = Proxima2GTRecord()
        for doc in response.results[0]:
            record.append(ProximaSERecall(doc))
        return True, record
    return False, None


def _diff_ground_truth(gt_context, mismatch_handler):
    gt = gt_context.load_gt()
    consist_count = 0
    for idx in range(gt_context.sample_counts()):
        code, recall = _se_recall_record(gt_context, gt_context.query(idx), True)
        if code:
            mismatched = recall.filter_score_not_in(gt.record(idx), gt_context.gt_count())
            if len(mismatched) != 0:
                mismatch_handler(idx, gt_context.gt_count(), mismatched)
            else:
                consist_count += 1
            print(f'{idx}, {round(float(gt_context.gt_count() - len(mismatched)) / gt_context.gt_count() * 100, 4)}')
    print("consist percentage: %.2f" % round(float(consist_count) / gt_context.sample_counts() * 100, 4))


def _print_header(topk):
    print("Processed", "  ef", " ".join(["%5s(%%)" % f'@{k}' for k in topk]))


def _print_summary(topk, ef, summary):
    samples = "%9d" % len(summary)
    stats = [samples, "%4d" % ef]
    for i in range(len(topk)):
        total = sum([metrics[i] for metrics in summary.values()])
        percent = round(total / len(summary) * 100, 4)
        stats.append("%8s" % str(percent))
    print(" ".join(stats))


class BatchRecall(object):
    def __init__(self, recall_context, start, end, mismatch_handler, reporter):
        self._context = recall_context
        self._start = start
        self._end = end
        self._mismatch_handler = mismatch_handler
        self._reporter = reporter
        self._pool = ThreadPoolExecutor(max_workers=(end - start), thread_name_prefix='BenchRecall')

    def __del__(self):
        self._pool.shutdown()

    @staticmethod
    def run_one(rc_context, idx, mismatch_handler):
        metrics = []
        feature = rc_context.query(idx)
        _, gt = _se_recall_record(rc_context, feature, True)
        code, knn = _se_recall_record(rc_context, feature)
        if code and knn.count() == rc_context.max_topk():
            for topk in rc_context.topk():
                mismatched = knn.filter_score_not_in(gt, topk)
                if len(mismatched) != 0:
                    mismatch_handler(idx, topk, mismatched)
                metrics.append(round(float(topk - len(mismatched)) / topk, 4))
        else:
            logging.error(f'Failed to get result of query: {idx}, topk {rc_context.max_topk()}')
            code = False

        if code:
            return idx, metrics
        return None, None

    def run(self):
        futures = [self._pool.submit(BatchRecall.run_one, self._context, i, self._mismatch_handler) for i in
                   range(self._start, self._end)]
        failed = False
        for future in concurrent.futures.as_completed(futures):
            idx, metrics = future.result()
            if not failed and idx is None:
                failed = True
            else:
                self._reporter(idx, metrics)
        return not failed


def _recall(recall_context, mismatch_handler):
    summary = {}

    def collector(index, stats):
        summary[index] = stats

    total = recall_context.sample_counts()
    interval = int(total / 20) if total > 50 else 1

    start = 0
    while start < total:
        if start == 0:
            _print_header(recall_context.topk())
        end = min(start + interval, total)
        bench = BatchRecall(recall_context, start, end, mismatch_handler, collector)
        if bench.run():
            _print_summary(recall_context.topk(), recall_context.ef(), summary)
        else:
            logging.error(f'Failed to run bench recall test from {start} to {end}')
            break
        start = end

    if len(summary) == recall_context.sample_counts():
        print("--------------------Recall Tests------------------")
        _print_header(recall_context.topk())
        _print_summary(recall_context.topk(), recall_context.ef(), summary)
        

def _skipped_handler(*_):
    pass


def _dump_mismatch(idx, topk, records):
    print('-----Mismatched Result-----')
    print(json.dumps({'query_id': idx, 'topk': topk, 'mismatched': records}, cls=ProximaSERecallEncoder, indent=2))


if __name__ == '__main__':
    parser = opt_parser()
    (options, args) = parser.parse_args()
    handle_help_and_exit(options, parser, len(sys.argv))

    context = RecallContext(options)
    context.init()

    handler = _dump_mismatch if options.dump_mismatch else _skipped_handler
    if options.gt:
        _diff_ground_truth(context, handler)
    else:
        _recall(context, handler)
