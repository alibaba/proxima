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
# Proxima2 Vectors reader
#
import logging
import math
import os
import json
import struct
import sys
from enum import IntEnum
from mmap import mmap
from typing import Optional, Any, Tuple


class StorageObject(object):
    def __init__(self, fmt: str = ''):
        self._fmt = fmt

    def calcsize(self):
        """The size of storage, counted by bytes, override if size of derived class is inconstant"""
        return struct.calcsize(self._fmt)

    def fmt(self):
        return self._fmt

    def pack(self):
        pass

    def unpack(self, buffer, offset=0):
        pass

    def json(self):
        return json.dumps(self, indent=4, default=lambda o: o.__dict__)


class DatasetHeader(StorageObject):
    def __init__(self):
        StorageObject.__init__(self, 'QI')
        self._num_vectors = 0
        self._meta_size = 0

    def __del__(self):
        pass

    def pack(self):
        return struct.pack(self.fmt(), self._num_vectors, self._meta_size)

    def unpack(self, buffer, offset=0):
        self._num_vectors, self._meta_size = struct.unpack_from(self.fmt(), buffer, offset)

    def num_vectors(self):
        return self._num_vectors

    def meta_size(self):
        return self._meta_size

    def meta_base(self):
        return self.calcsize()

    def vectors_base(self):
        return self.calcsize() + self._meta_size


class MetaHeader(StorageObject):
    def __init__(self):
        StorageObject.__init__(self, 'IHHIIQIIIIIIIIII')
        self._fmts = ['IHHIIQII', 'IIIIIIII']
        self._header_size = 0  # 4 bytes
        self._major_order = 0  # 2 bytes
        self._type = 0  # 2 bytes
        self._dimension = 0  # 4 bytes
        self._unit_size = 0  # 4 bytes
        self._space_id = 0  # 8 bytes
        self._attachment_offset = 0  # 4 bytes
        self._attachment_size = 0  # 4 bytes
        self._reserve = (0, 0, 0, 0, 0, 0, 0, 0)  # 32 bytes
        self._attachment = {}

    def __del__(self):
        pass

    def pack(self):
        return b''.join([struct.pack(self._fmts[0], self._header_size, self._major_order, self._type, self._dimension,
                                     self._unit_size, self._space_id, self._space_id, self._attachment_offset,
                                     self._attachment_size),
                         struct.pack(self._fmts[1], self._reserve[0], self._reserve[1], self._reserve[2],
                                     self._reserve[3], self._reserve[4], self._reserve[5], self._reserve[6],
                                     self._reserve[7]),
                         struct.pack("s", json.dumps(self._attachment))])

    def unpack(self, buffer, offset=0):
        (self._header_size, self._major_order, self._type, self._dimension, self._unit_size, self._space_id,
         self._attachment_offset, self._attachment_size) = struct.unpack_from(self._fmts[0], buffer, offset)
        self._reserve = struct.unpack_from(self._fmts[1], buffer, offset + struct.calcsize(self._fmts[0]))
        if self._attachment_size:
            attachment = struct.unpack_from("%ds" % self._attachment_size, buffer, offset + self._attachment_offset)
            self._attachment = json.loads(attachment[0])

    def header_size(self):
        return self._header_size

    def major_order(self):
        return self._major_order

    def type(self):
        return self._type

    def dimension(self):
        return self._dimension

    def unit_size(self):
        return self._unit_size

    def space_id(self):
        return self._space_id

    def attachment_offset(self):
        return self._attachment_offset

    def attachment_size(self):
        return self._attachment_size

    def reserve(self):
        return self._reserve

    def vector_size(self):
        return self._unit_size * self._dimension


class FeatureTypes(IntEnum):
    # FT_UNDEFINED = 0
    # FT_INT64 = 0  # hack types, which used to load unsigned long long
    # FT_BINARY32 = 1
    # FT_BINARY64 = 2
    FT_FP16 = 3
    FT_FP32 = 4
    FT_FP64 = 5
    FT_INT8 = 6
    FT_INT16 = 7
    # FT_INT4 = 8


class ValueTypeHelper(object):
    """
        Reference to deps/proxima/include/aitheta2/index_meta.h
          enum FeatureTypes {
            FT_UNDEFINED = 0,
            FT_BINARY32 = 1,
            FT_BINARY64 = 2,
            FT_FP16 = 3,
            FT_FP32 = 4,
            FT_FP64 = 5,
            FT_INT8 = 6,
            FT_INT16 = 7,
            FT_INT4 = 8,
          };
        """
    fmt_table = '***efdbh*'
    default_value_table = [0, 0, 0, 0.0, 0.0, 0.0, 0, 0, 0]
    precision_table = [0, 0, 0, 3, 6, 10, 0, 0, 0]

    @classmethod
    def fmt(cls, value_type: FeatureTypes, counts: int = 1):
        return counts * cls.fmt_table[value_type]

    @classmethod
    def pack(cls, value_type: FeatureTypes, value) -> bytes:
        return struct.pack(cls.fmt(value_type), value)

    @classmethod
    def pack_values(cls, value_type: FeatureTypes, values):
        for value in values:
            yield cls.pack(value_type, value)

    @classmethod
    def pack_array(cls, value_type: FeatureTypes, array) -> bytes:
        return b''.join(cls.pack_values(value_type, array))

    @classmethod
    def default(cls, value_type):
        return cls.default_value_table[value_type]

    @classmethod
    def default_values(cls, value_type, counts):
        for i in range(counts):
            yield cls.default(value_type)

    @classmethod
    def default_array(cls, value_type, counts):
        return list(cls.default_values(value_type, counts))

    @classmethod
    def precision(cls, value_type):
        return cls.precision_table[value_type]


class VectorObject(StorageObject):
    def __init__(self, value_type: FeatureTypes, counts):
        StorageObject.__init__(self, ValueTypeHelper.fmt(value_type, counts))
        self._value_type = value_type
        self._counts = counts
        self._vector = ValueTypeHelper.default_array(value_type, counts)

    def pack(self):
        return ValueTypeHelper.pack_array(self._value_type, self._vector)

    def unpack(self, buffer, offset=0):
        self._vector = struct.unpack_from(self.fmt(), buffer, offset)
        precision = ValueTypeHelper.precision(self._value_type)
        if precision != 0:
            self._vector = [round(v, precision) for v in self._vector]

    def vector(self):
        return self._vector


class KeyObject(StorageObject):
    def __init__(self):
        StorageObject.__init__(self, 'Q')
        self._key = 0

    def pack(self):
        return struct.pack(self.fmt(), self._key)

    def unpack(self, buffer, offset=0):
        self._key = struct.unpack_from(self.fmt(), buffer, offset)

    def key(self):
        return self._key


class RandomBufferReader(object):
    def __init__(self):
        pass

    def size(self):
        return 0

    def read(self, offset, size):
        pass


class RandomMMapReader(RandomBufferReader):
    def __init__(self, vector_file):
        RandomBufferReader.__init__(self)
        self._fp = open(vector_file, 'r+b')
        self._mm = mmap(self._fp.fileno(), 0)

    def __del__(self):
        self._mm.close()
        self._fp.close()

    def size(self):
        return self._mm.size()

    def read(self, offset, size):
        if offset < 0 or size < 0 or offset + size > self._mm.size():
            raise IOError
        return self._mm[offset:offset + size]


class RandomFileReader(RandomBufferReader):
    def __init__(self, vector_file):
        RandomBufferReader.__init__(self)
        self._file = open(vector_file, 'rb')
        self._size = self.size()

    def __del__(self):
        self._file.close()

    def size(self):
        current = self._file.tell()
        self._file.seek(0, os.SEEK_END)
        count = self._file.tell()
        self._file.seek(current, os.SEEK_SET)
        return count

    def read(self, offset, size):
        if offset < 0 or size < 0 or offset + size > self._size:
            raise IOError
        self._file.seek(offset)
        return self._file.read(size)


class Proxima2VectorsReader(object):
    def __init__(self, buffer: RandomBufferReader):
        self._buffer = buffer
        self._header = None
        self._meta = None
        self._vectors_base = 0
        self._keys_base = 0
        self._keys_cache = ()
        self._cached_begin = 0

    def __del__(self):
        pass

    def header(self):
        return self._header

    def meta(self):
        return self._meta

    def num_vectors(self):
        return self._header.num_vectors()

    def _load_header(self) -> Optional[DatasetHeader]:
        header = DatasetHeader()
        if self._buffer.size() < header.calcsize():
            return None

        buf = self._buffer.read(0, header.calcsize())
        header.unpack(buf)
        return header

    def _load_meta(self) -> Optional[MetaHeader]:
        meta = MetaHeader()
        if self._header is None or self._buffer.size() < self._header.vectors_base():
            return None

        buf = self._buffer.read(self._header.calcsize(), self._header.meta_size())
        meta.unpack(buf)
        return meta

    def _vector_base(self, index):
        return self._vectors_base + index * self._meta.vector_size()

    def _key_base(self, index):
        return self._keys_base + index * 8

    def load(self):
        if not self._buffer:
            return False
        self._header = self._load_header()
        self._meta = self._load_meta()
        self._vectors_base = self._header.vectors_base()
        self._keys_base = self._vector_base(self._header.num_vectors())

    def unload(self):
        return True

    def is_valid(self):
        return self._header and self._meta and \
               self._buffer.size() == self._key_base(self._header.num_vectors())

    def vector(self, index):
        if index >= self._header.num_vectors():
            return None
        vec = VectorObject(self._meta.type(), self._meta.dimension())
        buf = self._buffer.read(self._vector_base(index), vec.calcsize())
        vec.unpack(buf)
        return vec.vector()

    def _load_keys(self, begin, counts):
        if begin + counts > self._header.num_vectors():
            return []

        key = KeyObject()
        buf = self._buffer.read(self._key_base(begin), counts * key.calcsize())
        return struct.unpack(counts * key.fmt(), buf)

    def _cache_range(self):
        return [self._cached_begin, self._cached_begin + len(self._keys_cache) - 1]

    def _in_cache(self, index):
        cached = self._cache_range()
        return cached[0] <= index <= cached[1]

    def _load_batch_keys(self, index):
        begin = int((index / 1024) * 1024)
        counts = 1024 if (begin + 1024) < self._header.num_vectors() else self._header.num_vectors() - begin
        self._keys_cache = self._load_keys(begin, counts)
        self._cached_begin = begin

    def key(self, index):
        if index >= self._header.num_vectors():
            return None

        if not self._in_cache(index):
            self._load_batch_keys(index)

        return self._keys_cache[index - self._cached_begin]


class Proxima2GTRecall(object):
    """
    Format: pk, score
    """
    pack_fmt = 'Lf'

    def __init__(self, pk=0, score=0.0):
        self.pk = pk
        self.score = score

    def __eq__(self, other):
        return self.equals(other)

    @classmethod
    def calcsize(cls):
        return struct.calcsize(Proxima2GTRecall.pack_fmt)

    def pack(self):
        return struct.pack(Proxima2GTRecall.pack_fmt, self.pk, self.score)

    def unpack(self, buf, offset=0):
        self.pk, self.score = struct.unpack_from(Proxima2GTRecall.pack_fmt, buf, offset)
        # self.score = round(self.score, 6)

    def is_same_pk(self, pk):
        return self.pk == pk

    def is_same_score(self, score):
        return math.isclose(self.score, score, 1e-06)

    def equals(self, recall):
        return self.is_same_score(recall.pk) and self.is_same_score(recall.score)


class CustomEncoder(json.JSONEncoder):
    def default(self, x):
        if isinstance(x, Proxima2GTRecall):
            return {'pk': x.pk, 'score': x.score}
        return super().default(self, x)


def _default_receiver(_):
    pass


class Proxima2GTRecord(object):
    """
    GTRecord: gt_count, [Proxima2GTRecall, ...]
    """
    pack_fmt = 'i'

    def __init__(self, count=0):
        self._count = count
        self._recalls = []

    def dumps(self):
        return {"count": self._count, "recalls": self._recalls}

    def calcsize(self):
        return struct.calcsize(Proxima2GTRecord.pack_fmt) + self._count * Proxima2GTRecall.calcsize()

    def pack(self):
        return struct.pack(Proxima2GTRecord.pack_fmt, self._count) + \
               b''.join([recall.pack() for recall in self.recalls])

    def unpack(self, buf, offset=0):
        self._count, = struct.unpack_from(Proxima2GTRecord.pack_fmt, buf, offset)
        offset += struct.calcsize(Proxima2GTRecord.pack_fmt)
        self._recalls = []
        for i in range(self._count):
            recall = Proxima2GTRecall()
            recall.unpack(buf, offset)
            offset += recall.calcsize()
            self._recalls.append(recall)

    def count(self):
        return self._count

    def record(self, pk):
        return self.find(pk)

    def recalls(self, end=sys.maxsize):
        end = min(end, self._count)
        for r in self._recalls[:end]:
            yield r

    def find(self, pk):
        try:
            f = filter(lambda record: record.pk == pk, self._recalls)
            return next(f)
        except StopIteration as e:
            return None

    def exists(self, pk):
        return self.find(pk) is not None

    def pk_set(self, end=sys.maxsize):
        end = min(end, len(self._recalls))
        return set([r.pk for r in self._recalls[:end]])

    def score_set(self, end=sys.maxsize):
        return set(self.score_list(end))

    def score_list(self, end=sys.maxsize):
        end = min(end, len(self._recalls))
        return [r.score for r in self._recalls[:end]]

    def append(self, record):
        self._recalls.append(record)
        self._count = len(self._recalls)

    def filter_pk(self, record, nums, receiver=_default_receiver):
        pks = record.pk_set(nums) & self.pk_set(nums)
        results = []
        for r in self.recalls(nums):
            if r.pk in pks:
                results.append(r)
            else:
                receiver(r)
        return results

    def filter_pk_not_in(self, record, nums, receiver=_default_receiver):
        pks = record.pk_set(nums) & self.pk_set(nums)
        results = []
        for r in self.recalls(nums):
            if r.pk not in pks:
                results.append(r)
            else:
                receiver(r)
        return results

    def _score_in_list(self, score, array):
        try:
            f = filter(lambda s: math.isclose(s, score, rel_tol=1e-05), array)
            next(f)
            return True
        except StopIteration as _:
            return False

    def _score_list_and(self, left, right):
        results = []
        for s in left:
            if self._score_in_list(s, right):
                results.append(s)
        return results

    def filter_score(self, record, nums, receiver=_default_receiver):
        score_set = record.score_set(nums) & self.score_set(nums)
        # score_set = set(self._score_list_and(self.score_list(), record.score_list()))
        results = []
        for r in self.recalls(nums):
            if r.score in score_set:
                results.append(r)
            else:
                receiver(r)
        return results

    def filter_score_not_in(self, record, nums, receiver=_default_receiver):
        score_set = record.score_set(nums) & self.score_set(nums)
        # score_set = set(self._score_list_and(self.score_list(), record.score_list()))
        results = []
        for r in self.recalls(nums):
            if r.score not in score_set:
                results.append(r)
            else:
                receiver(r)
        return results

    def __and__(self, other):
        return other.pk_set() and self.pk_set()

    def __or__(self, other):
        return other.pk_set() or self.pk_set()


class Proxima2GT(object):
    """
    Ground Truth file reader
    Format: [Proxima2GTRecord, ....]
    """

    def __init__(self, gt_counts, gt_file, skip_magic=True):
        self._skip_magic = skip_magic
        self._counts = gt_counts
        self._gt_file = gt_file
        self._records = []

    def __del__(self):
        self._records = []

    def load(self):
        record = Proxima2GTRecord(self._counts)
        stat = os.stat(self._gt_file)
        if stat.st_size == 0 or stat.st_size % record.calcsize() != 0:
            logging.error(f'Failed to check size of gt file, which should be multiple times of {record.calcsize()}')
            return False
        with open(self._gt_file, 'rb') as fin:
            while True:
                record = Proxima2GTRecord(self._counts)
                buf = fin.read(record.calcsize())
                if not buf:
                    break
                record.unpack(buf)
                self._records.append(record)
        return True

    def unload(self):
        self._records = []

    def records(self):
        for r in self._records:
            yield r

    def record(self, idx):
        if abs(idx) < len(self._records):
            return self._records[idx]
        return None

    def counts(self):
        return len(self._records)

    def append(self, record):
        self._records.append(record)

    def serialize(self):
        with open(self._gt_file, 'w+b') as out:
            for r in self.records():
                out.write(r.pack())
