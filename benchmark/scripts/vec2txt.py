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
# Proxima SE bench toolkits
#

import sys
from optparse import OptionParser
from common.proxima2_dataset import *


def opt_parser():
    arg_parser = OptionParser()
    arg_parser.add_option('--vec', dest='vec', default=None, help="Proxima2 vectors file")
    arg_parser.add_option('--header', dest='header', action="store_true", default=False, help='Dump header')
    arg_parser.add_option('-o', '--output', dest='output', default='data', help='output directory')
    arg_parser.add_option('-s', '--segment_size', dest='segment', default=1000000, help='segment size')
    arg_parser.add_option('--begin', dest='begin', default=0, help='The beginner of vector')
    arg_parser.add_option('--end', dest='end', default=-1, help='The end of vector')
    return arg_parser


def handle_help_and_exit(arg_options, arg_parser, nargs):
    try:
        arg_parser.print_help() if nargs == 1 or arg_options.help else None
        quit()
    except Exception as e:
        # ignore help exception
        pass


class VectorWriter(object):
    def __init__(self):
        pass

    def write(self, key, vector):
        pass


class SQLWriter(VectorWriter):
    def __init__(self):
        VectorWriter.__init__(self)

    def write(self, key, vector):
        pass


class RawWriter(VectorWriter):
    def __init__(self, output, segment):
        VectorWriter.__init__(self)
        self._output = output
        self._segment_size = segment
        self._count = 0
        self._segment_id = 0
        self._segment = None

    def __del__(self):
        self._close_segment()

    def _close_segment(self):
        if self._segment:
            self._segment.close()
        self._segment = None

    def _segment_name(self):
        return os.path.join(self._output, "segment.%03d" % self._segment_id)

    def _open_segment(self):
        self._close_segment()
        self._segment = open(self._segment_name(), 'w+')
        self._segment_id += 1

    def write(self, key, vector):
        if self._count % self._segment_size == 0:
            self._open_segment()
        fields = [','.join([str(v) for v in vector]), str(key), "%f" % vector[0], str(vector[0])]
        self._segment.write("%s\n" % "|".join(fields))
        self._count += 1


if __name__ == '__main__':
    parser = opt_parser()
    (options, args) = parser.parse_args()
    handle_help_and_exit(options, parser, len(sys.argv))

    if options.vec and os.path.isfile(options.vec) and options.vec.endswith(".vecs2"):
        output = RawWriter(options.output, int(options.segment))
        vectors = Proxima2VectorsReader(RandomMMapReader(options.vec))
        vectors.load()
        if vectors.is_valid():
            if options.header:
                print(vectors.header().json())
                print(vectors.meta().json())

            begin = int(options.begin) if int(options.begin) >= 0 else 0
            end = int(options.end) if int(options.end) >= 0 else vectors.num_vectors()
            while begin < end:
                output.write(vectors.key(begin), vectors.vector(begin))
                begin += 1
                if begin % 100000 == 0:
                    print(begin)
        else:
            print("Failed to open vectors")
            exit(1)
        vectors.unload()
    else:
        print("Lost argument vec, which should be Dataset of Proxima2")
        parser.print_help()
    exit(0)
