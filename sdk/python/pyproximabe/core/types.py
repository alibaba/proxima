# Copyright 2021 Alibaba, Inc. and its affiliates. All Rights Reserved.
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

import struct
from enum import Enum
from enum import IntEnum
import base64

import time
from proto import common_pb2
from proto import proxima_be_pb2
from proto import proxima_be_pb2_grpc  # NOQA

from ..version import __version__


class IndexType(IntEnum):
    UNDEFINED = 0
    PROXIMA_GRAPH_INDEX = 1


class DataType(IntEnum):
    UNDEFINED = 0
    BINARY = 1
    STRING = 2
    BOOL = 3
    INT32 = 4
    INT64 = 5
    UINT32 = 6
    UINT64 = 7
    FLOAT = 8
    DOUBLE = 9
    VECTOR_BINARY32 = 20
    VECTOR_BINARY64 = 21
    VECTOR_FP16 = 22
    VECTOR_FP32 = 23
    VECTOR_FP64 = 24
    VECTOR_INT4 = 25
    VECTOR_INT8 = 26
    VECTOR_INT16 = 27


class ProximaBeException(Exception):
    pass


class ProximaBeStatus:
    """
    ProximaBe Status

    Attributes:
        code (int): Status code, 0 for success, otherwise for failure.
        reason (str): Error details.
    """

    def __init__(self, code, reason):
        """
        Args:
            code (int): status code
            reason (str): error details
        """
        self.code = code
        self.reason = reason
        self.timer_report = None

    def ok(self):
        """
        Returns:
            bool: if ok.
        """
        return self.code == 0

    def update_timer_report(self, timer):
        if timer is not None:
            self.timer_report = timer.timer_report

    def __str__(self):
        if self.code == 0:
            return 'success'
        return f'{self.reason}({self.code})'


class _Printable:
    def __str__(self):
        return _stringify(self)

    def __repr__(self):
        return _stringify(self)


class IndexColumnParam(_Printable):
    """
    Column index params.

    Attributes:
        name (str): Column name.
        dimension (int): Vector dimension.
        index_type (IndexType): IndexType enum value or string.
        data_type (DataType): DataType enum value or string.
        extra_params (dict): Extended parameters.

    """

    # pylint: disable=too-many-arguments
    def __init__(self,
                 name,
                 dimension,
                 index_type=IndexType.PROXIMA_GRAPH_INDEX,
                 data_type=DataType.VECTOR_FP32,
                 extra_params=None):
        """
        Constructor

        Args:
            name (str): Column name.
            dimension (int): Vector dimension.
            index_type (int or str): IndexType enum value or string.
            data_type (int or str): DataType enum value or string.
            extra_params (Optional[dict]): Extended parameters.
        """
        # column name
        self.name = name
        # vector dimension
        self.dimension = dimension
        # IndexType enum
        self.index_type = _parse_enum_value_or_string(IndexType,
                                                      common_pb2.IndexType,
                                                      index_type, 'IT_')
        # DataType enum
        self.data_type = _parse_enum_value_or_string(DataType,
                                                     common_pb2.DataType,
                                                     data_type, 'DT_')
        # Extra parameters.
        self.extra_params = _default_if_none(extra_params, {})

    def to_pb(self):
        """Return corresponding protobuf message."""
        param = proxima_be_pb2.CollectionConfig.IndexColumnParam()
        param.column_name = self.name
        param.index_type = self.index_type.value
        param.data_type = self.data_type.value
        param.dimension = self.dimension
        param.extra_params.extend(
            _key_value_pair(key, value)
            for key, value in self.extra_params.items())
        return param

    @staticmethod
    def from_pb(pb_column):
        """Parse from corresponding protobuf type."""
        extra_params = {}
        for extra_param in pb_column.extra_params:
            extra_params[extra_param.key] = extra_param.value
        return IndexColumnParam(pb_column.column_name, pb_column.dimension,
                                pb_column.index_type, pb_column.data_type,
                                extra_params)


class DatabaseRepository(_Printable):
    """
    Database Repository.

    Attributes:
        repository_name (str): repository name
        connection_uri (str): database connection uri
        table_name (str): table name
        user (str): database user name
        password (str): database password
    """

    # pylint: disable=too-many-arguments
    def __init__(self, repository_name, connection_uri, table_name, user,
                 password):
        """
        Constructor.

        Args:
            repository_name (str): repository name
            connection_uri (str): database connection uri, e.g. mysql://localhost/database
            table_name (str): table name
            user (str): database user name
            password (str): database password
        """
        self.repository_name = repository_name
        self.connection_uri = connection_uri
        self.table_name = table_name
        self.user = user
        self.password = password

    def to_pb(self):
        """Return corresponding protobuf message."""
        repo = proxima_be_pb2.CollectionConfig.RepositoryConfig()
        repo.repository_type = proxima_be_pb2.CollectionConfig.RepositoryConfig.RepositoryType.RT_DATABASE
        repo.repository_name = self.repository_name
        db = repo.database
        db.connection_uri = self.connection_uri
        db.table_name = self.table_name
        db.user = self.user
        db.password = self.password
        return repo

    @staticmethod
    def from_pb(pb_repo):
        """Parse from corresponding protobuf type."""
        assert pb_repo.repository_type == proxima_be_pb2.CollectionConfig.RepositoryConfig.RepositoryType.RT_DATABASE
        db = pb_repo.database
        return DatabaseRepository(pb_repo.repository_name, db.connection_uri,
                                  db.table_name, db.user, db.password)


class CollectionConfig(_Printable):
    """
    Collection configuration.

    Attributes:
        collection_name (str): collection name.
        index_column_params (List[IndexColumnParam]): index column params.
        forward_column_names (List[str]): forward columns.
        repository_config (Optional[DatabaseRepository]): repository config.
        max_docs_per_segment (long): max document number per segment.
    """

    # pylint: disable=too-many-arguments
    def __init__(self,
                 collection_name,
                 index_column_params,
                 forward_column_names=None,
                 repository_config=None,
                 max_docs_per_segment=0):
        """
        Constructor.

        Args:
            collection_name (str): collection name.
            index_column_params (List[IndexColumnParam]): index column params.
            forward_column_names (Optional[List[str]]): forward columns.
            repository_config (Optional[DatabaseRepository]): repository config.
            max_docs_per_segment (Optional[long]): max document number per segment. 0 means infinity.
        """
        # collection name.
        self.collection_name = collection_name
        # index column list.
        self.index_column_params = index_column_params
        # max document number per segment.
        self.max_docs_per_segment = max_docs_per_segment
        # forward column list
        self.forward_column_names = _default_if_none(forward_column_names, [])
        # repository_config
        self.repository_config = repository_config

    def to_pb(self):
        """Return corresponding protobuf message."""
        collection = proxima_be_pb2.CollectionConfig()
        collection.collection_name = self.collection_name
        collection.max_docs_per_segment = self.max_docs_per_segment
        collection.forward_column_names.extend(self.forward_column_names)
        collection.index_column_params.extend(
            column.to_pb() for column in self.index_column_params)
        if self.repository_config is not None:
            collection.repository_config.CopyFrom(self.repository_config.to_pb())
        return collection

    @staticmethod
    def from_pb(pb_collection_config):
        """Parse from corresponding protobuf type."""
        repository_config = _parse_repository_from_pb(pb_collection_config)
        index_column_params = [
            IndexColumnParam.from_pb(column)
            for column in pb_collection_config.index_column_params
        ]
        # pylint: disable=unnecessary-comprehension
        forward_column_names = [
            f for f in pb_collection_config.forward_column_names
        ]
        return CollectionConfig(collection_name=pb_collection_config.collection_name,
                                index_column_params=index_column_params,
                                max_docs_per_segment=pb_collection_config.max_docs_per_segment,
                                repository_config=repository_config, forward_column_names=forward_column_names)


class CollectionInfo(_Printable):
    """
    Collection information.

    Attributes:
        collection_config (CollectionConfig): collection config.
        status (CollectionInfo.Status): Status enum
        uuid (Optional[str]): collection uuid
        latest_lsn_context (Optional[LsnContext]): lsn context
        magic_number (Optional[long]): magic number
    """

    class Status(IntEnum):
        """Collection Status"""
        INITIALIZED = 0
        SERVING = 1
        DROPPED = 2

    # pylint: disable=too-many-arguments
    def __init__(self,
                 collection_config,
                 status,
                 uuid=None,
                 latest_lsn_context=None,
                 magic_number=None):
        """
        Constructor.

        Args:
            collection_config (CollectionConfig): collection config.
            status (Optional[CollectionInfo.Status, int, str]): Status enum int or str
            uuid (Optional[str]): collection uuid
            latest_lsn_context (Optional[LsnContext]): lsn context
            magic_number (Optional[long]): magic number
        """
        self.collection_config = collection_config
        self.status = _parse_enum_value_or_string(
            CollectionInfo.Status,
            proxima_be_pb2.CollectionInfo.CollectionStatus, status, 'CS_')
        self.uuid = uuid
        self.latest_lsn_context = latest_lsn_context
        self.magic_number = magic_number

    @staticmethod
    def from_pb(pb_collection_info):
        """Parse from corresponding protobuf type."""
        collection_config = CollectionConfig.from_pb(pb_collection_info.config)
        latest_lsn_context = LsnContext(
            pb_collection_info.latest_lsn_context.lsn,
            pb_collection_info.latest_lsn_context.context)
        return CollectionInfo(collection_config, pb_collection_info.status,
                              pb_collection_info.uuid, latest_lsn_context,
                              pb_collection_info.magic_number)


class WriteRequest(_Printable):
    class IndexColumnMeta(_Printable):
        """
        Index column meta.

        Attributes:
            name (str): column name
            data_type (DataType): DataType enum
            dimension (int): vector dimension
        """
        def __init__(self, name, data_type, dimension):
            """
            Constructor.

            Args:
                name (str): column name
                data_type (Union[DataType, int, str]): DataType enum, enum value or str
                dimension (int): vector dimension
            """
            self.name = name
            self.data_type = _parse_enum_value_or_string(DataType, common_pb2.DataType, data_type, 'DT_')
            self.dimension = dimension

    class RowMeta(_Printable):
        """
        Row meta data.

        Attributes:
            index_column_metas (List[IndexColumnMeta]): index column metas
            forward_column_names (List[str]): forward column names
            forward_column_types (List[DataType]): forward column types
        """

        def __init__(self,
                     index_column_metas,
                     forward_column_names=None,
                     forward_column_types=None):
            """
            Constructor.

            Args:
                index_column_metas (List[IndexColumnMeta]): index column metas
                forward_column_names (Optional[List[str]]): forward column names
                forward_column_types (Optional[List[DataType]]): forward column types
            """
            self.index_column_metas = index_column_metas
            self.forward_column_names = _default_if_none(forward_column_names, [])
            self.forward_column_types = [
                _parse_enum_value_or_string(DataType, common_pb2.DataType, column_type, 'DT_')
                for column_type in _default_if_none(forward_column_types, [])
            ]
            self._valid_check()

        def to_pb(self):
            """Return corresponding protobuf message."""
            row_meta = proxima_be_pb2.WriteRequest.RowMeta()
            row_meta.forward_column_names.extend(self.forward_column_names)
            for m in self.index_column_metas:
                index_meta = row_meta.index_column_metas.add()
                index_meta.data_type = m.data_type.value
                index_meta.dimension = m.dimension
                index_meta.column_name = m.name
            return row_meta

        def _valid_check(self):
            forward_names_len = len(self.forward_column_names)
            forward_types_len = len(self.forward_column_types)

            if forward_names_len != forward_types_len:
                raise ProximaBeException(
                    f"Mismatched forward names with types, forward_names_len={forward_names_len}, "
                    f"forward_types_len={forward_types_len}")

            if len(self.index_column_metas) < 1:
                raise ProximaBeException("Expect at least one index column, got zero")

    class OperationType(IntEnum):
        INSERT = 0
        UPDATE = 1
        DELETE = 2


    class Row(_Printable):
        """
        Row

        Attributes:
            primary_key (long): primary key
            operation_type (OperationType): OperationType enum
            index_column_values (List[Union[str, bytes, List[Union[float, int, ...]]]]): index column values. Should contain the same number as RowMeta.
            forward_column_values (Optional[List[Any]]): forward column values. Should contain the same number and type as RowMeta.
            lsn_context (Optional[LsnContext]): lsn context

        """

        # pylint: disable=too-many-arguments
        def __init__(self,
                     primary_key,
                     operation_type,
                     index_column_values=None,
                     forward_column_values=None,
                     lsn_context=None):
            """
            Constructor.

            Args:
                primary_key (long): primary key
                operation_type (Union[OperationType, int, str]): OperationType enum, enum value or str
                index_column_values (Optional[List[Union[str, bytes, List[Union[float, int, ...]]]]]): index column values. Should contain the same number as RowMeta.
                    Not required if operation_type is DELETE.
                forward_column_values (Optional[List[Any]]): forward column values. Should contain the same number and type as RowMeta.
                lsn_context (Optional[LsnContext]): lsn context

            Examples:
                >>> # only primary_key is required for delete request
                >>> row = WriteRequest.Row(primary_key=1, operation_type=WriteRequest.OperationType.DELETE)

                >>> # index column value can take bytes, str(in the format of json array) or vector list
                >>> # 1. vector list
                >>> row = WriteRequest.Row(primary_key=1, operation_type=WriteRequest.OperationType.INSERT,
                ...     index_column_values=[[1.0, 2.0, 3.0]])
                >>> # 2. bytes in the format of binary representation in little endian
                >>> row = WriteRequest.Row(primary_key=1, operation_type=WriteRequest.OperationType.INSERT,
                ...     index_column_values=[struct.pack('<3f', 1.0, 2.0, 3.0)])
                >>> # 3. str in the format of json array
                >>> row = WriteRequest.Row(primary_key=1, operation_type=WriteRequest.OperationType.INSERT,
                ...     index_column_values=['[1.0, 2.0, 3.0]'])
            """
            self.primary_key = primary_key
            self.operation_type = _parse_enum_value_or_string(
                WriteRequest.OperationType, common_pb2.OperationType, operation_type, 'OP_')
            self.index_column_values = _default_if_none(index_column_values, [])
            self.forward_column_values = _default_if_none(forward_column_values, [])
            self.lsn_context = lsn_context
            self._valid_check()

        def _valid_check(self):
            if self.operation_type != WriteRequest.OperationType.DELETE and not self.index_column_values:
                raise ProximaBeException("Insert/Update request require index_column_values set")

    # pylint: disable=too-many-arguments
    def __init__(self,
                 collection_name,
                 rows,
                 row_meta=None,
                 request_id=None,
                 magic_number=None):
        """
        Constructor

        Args:
            collection_name (str): collection name
            row_meta (Optional[RowMeta]): row meta
                Not required if all operation_type of rows is DELETE
            rows (List[Rows]): rows
            request_id (Optional[str]): request id
            magic_number (Optional[long]): magic number
        """
        self._collection_name = collection_name
        self._request_id = request_id
        self._magic_number = magic_number
        self._row_meta = row_meta
        self._rows = rows
        self._valid_check()
        self._convert_vector_feature_to_bytes()

    def _valid_check(self):
        if not self._rows:
            raise ProximaBeException("Cannot write empty rows")
        for row in self._rows:
            row._valid_check()  # NOQA
        # only allow empty row meta on delete requests
        if self._row_meta is None:
            all_delete = all(row.operation_type == WriteRequest.OperationType.DELETE for row in self._rows)
            if not all_delete:
                raise ProximaBeException("Insert/Update request require row_meta set")
        else:
            forward_names_len = len(self._row_meta.forward_column_names)
            forward_values_len = len(self._rows[0].forward_column_values)

            if forward_names_len != forward_values_len:
                raise ProximaBeException(
                    f"Mismatched forward meta with value, forward_names_len={forward_names_len}, "
                    f"forward_values_len={forward_values_len}"
                )

            index_names_len = len(self._row_meta.index_column_metas)
            index_values_len = len(self._rows[0].index_column_values)
            if index_values_len != index_names_len:
                raise ProximaBeException(
                    f"Mismatched index meta with value, index_names_len={index_names_len}, "
                    f"index_values_len={index_values_len}")

    def _convert_vector_feature_to_bytes(self):
        for row in self._rows:
            for i, features in enumerate(row.index_column_values):
                if isinstance(features, list):
                    index_meta = self._row_meta.index_column_metas[i]
                    row.index_column_values[i] = _pack_feature(features, index_meta.data_type, index_meta.dimension)

    def to_pb(self):
        """Return corresponding protobuf message."""
        pb = proxima_be_pb2.WriteRequest()
        pb.collection_name = self._collection_name
        if self._row_meta is not None:
            pb.row_meta.CopyFrom(self._row_meta.to_pb())
        if self._request_id is not None:
            pb.request_id = self._request_id
        if self._magic_number is not None:
            pb.magic_number = self._magic_number
        for row in self._rows:
            pb_row = pb.rows.add()
            pb_row.primary_key = row.primary_key
            pb_row.operation_type = row.operation_type.value
            if self._row_meta is not None:
                pb_row.forward_column_values.values.extend(
                    _generic_value(value, column_type) for value, column_type in zip(
                        row.forward_column_values, self._row_meta.forward_column_types))
            pb_row.index_column_values.values.extend(
                _index_value_to_generic_value(value)
                for value in row.index_column_values)
            if row.lsn_context is not None:
                pb_row.lsn_context.lsn = row.lsn_context.lsn
                pb_row.lsn_context.context = row.lsn_context.context
        return pb


class Document(_Printable):
    """
    Document

    Attributes:
        primary_key (long): primary key
        score (float): score, i.e. distance from query vector
        forward_column_values (dict): dict of forward column name and value
    """

    def __init__(self, pb_doc=None, json_doc=None):
        """
        Constructor.

        Args:
            pb_doc (proximase_pb2.Document): protobuf message
            json_doc (json): json message
        """
        if pb_doc is not None:
            self.primary_key = pb_doc.primary_key
            self.score = pb_doc.score
            self.forward_column_values = {
                value.key: _parse_generic_value_from_pb(value.value)
                for value in pb_doc.forward_column_values
            }
            return
        self.primary_key = int(json_doc['primary_key'])
        self.score = json_doc['score']
        self.forward_column_values = {}
        for forward in json_doc['forward_column_values']:
            key = forward['key']
            pb_value = forward['value']
            value = None
            if len(pb_value) == 1:
                type_key, value = next(iter(pb_value.items()))
                if type_key in ['int64_value', 'uint64_value']:
                    value = int(value)
                elif type_key == 'bytes_value':
                    value = base64.b64decode(value)
            self.forward_column_values[key] = value

    @staticmethod
    def from_pb(pb_doc):
        """Parse from corresponding protobuf type."""
        return Document(pb_doc)

    @staticmethod
    def from_json(json_doc):
        """Parse from json."""
        return Document(json_doc=json_doc)


class QueryResponse(_Printable):
    """

    Attributes:
        results (List[List[Document]]): query results.
        debug_info (str): debug information.
        latency_us (long): query latency.
    """

    def __init__(self, pb_rsp=None, json_rsp=None):
        if pb_rsp is not None:
            self.results = [[Document(d) for d in r.documents] for r in pb_rsp.results]
            self.debug_info = pb_rsp.debug_info
            self.latency_us = pb_rsp.latency_us
            return
        self.debug_info = json_rsp['debug_info']
        self.latency_us = int(json_rsp['latency_us'])
        self.results = [[Document.from_json(d) for d in r['documents']] for r in json_rsp['results']]

    @staticmethod
    def from_pb(pb_rsp):
        """Parse from corresponding protobuf type."""
        return QueryResponse(pb_rsp)

    @staticmethod
    def from_json(json_rsp):
        """Parse from json."""
        return QueryResponse(json_rsp=json_rsp)


class CollectionStats(_Printable):
    """
    Collection statistics.

    Attributes:
        collection_name(str) : collection name
        collection_path(str) : collection path
        total_doc_count(long) : total document count
        total_segment_count(long) : total segment count
        total_index_file_count(long) : total index file count
        total_index_file_size(long) : total index file size
        segment_stats(List[SegmentStats]) : segments statistics
    """
    class SegmentState(IntEnum):
        CREATED = 0
        WRITING = 1
        DUMPING = 2
        COMPACTING = 3
        PERSIST = 4

    class SegmentStats(_Printable):
        """
        Segment statistics.

        Attributes:
            segment_id(int) : segment id
            state(SegmentState) : segment state
            doc_count(long) : document count
            index_file_count(long) : index file count
            index_file_size(long) : index file size
            min_doc_id(long) : minimum document id in current segment
            max_doc_id(long) : maximum document id in current segment
            min_primary_key(long) : minimum primary key in current segment
            max_primary_key(long) : maximum primary key in current segment
            min_timestamp(long) : minimum document timestamp in current segment
            max_timestamp(long) : maximum document timestamp in current segment
            min_lsn(long) : minimum log sequence number(lsn) in current segment
            max_lsn(long) : maximum log sequence number(lsn) in current segment
            segment_path(str) : segment path

        """
        def __init__(self, pb):
            self.segment_id = pb.segment_id
            self.state = _parse_enum_value_or_string(
                CollectionStats.SegmentState,
                proxima_be_pb2.CollectionStats.SegmentStats.SegmentState, pb.state, 'SS_')
            self.doc_count = pb.doc_count
            self.index_file_count = pb.index_file_count
            self.index_file_size = pb.index_file_size
            self.min_doc_id = pb.min_doc_id
            self.max_doc_id = pb.max_doc_id
            self.min_primary_key = pb.min_primary_key
            self.max_primary_key = pb.max_primary_key
            self.min_timestamp = pb.min_timestamp
            self.max_timestamp = pb.max_timestamp
            self.min_lsn = pb.min_lsn
            self.max_lsn = pb.max_lsn
            self.segment_path = pb.segment_path

        @staticmethod
        def from_pb(pb):
            """Parse from corresponding protobuf type."""
            return CollectionStats.SegmentStats(pb)

    def __init__(self, pb):
        self.collection_name = pb.collection_name
        self.collection_path = pb.collection_path
        self.total_doc_count = pb.total_doc_count
        self.total_segment_count = pb.total_segment_count
        self.total_index_file_count = pb.total_index_file_count
        self.total_index_file_size = pb.total_index_file_size
        self.segment_stats = [
            CollectionStats.SegmentStats.from_pb(s) for s in pb.segment_stats
        ]

    @staticmethod
    def from_pb(pb):
        """Parse from corresponding protobuf type."""
        return CollectionStats(pb)


class LsnContext(_Printable):
    """
    Log sequence number context.

    Usually optional.
    Currently used by database repository.

    Attributes:
        lsn (long): log sequence number
        context (str): context str.
    """

    def __init__(self, lsn, context):
        """
        Constructor

        Args:
            lsn (long): log sequence number
            context (str): context str.
        """
        self.lsn = lsn
        self.context = context


def _parse_enum_value_or_string(enum_type, pb_enum_type, value, pb_enum_prefix=''):
    """
    Convert `value` to corresponding enum value.

    Args:
        enum_type: enum type.
        pb_enum_type: Protobuf enum type.
        value (int, str or enum_type): enum value

    Returns:
        enum type instance.

    Raises:
        ProximaBeException on invalid enum.
    """
    try:
        if isinstance(value, str):
            assert pb_enum_type.Value(pb_enum_prefix + value) == enum_type[value].value
            return enum_type[value]
        if isinstance(value, enum_type):
            assert pb_enum_type.Name(value.value) == pb_enum_prefix + value.name
            return value
        assert pb_enum_type.Name(value) == pb_enum_prefix + enum_type(value).name
        return enum_type(value)
    except ValueError as e:
        raise ProximaBeException(str(e))
    except AssertionError as e:
        raise ProximaBeException(f"Enum definition mismatch:{str(e)}")


def _key_value_pair(key, value):
    """Return KeyValuePair"""
    pair = common_pb2.KeyValuePair()
    pair.key = key
    pair.value = value
    return pair


def _build_get_document_request(collection_name, primary_key):
    req = proxima_be_pb2.GetDocumentRequest()
    req.collection_name = collection_name
    req.primary_key = primary_key
    return req


_data_type_to_dimension = {
    DataType.VECTOR_BINARY32 : 32,
    DataType.VECTOR_BINARY64 : 64,
}

_data_type_to_format = {
    DataType.VECTOR_FP16: 'e',
    DataType.VECTOR_FP32: 'f',
    DataType.VECTOR_FP64: 'd',
    DataType.VECTOR_INT16: 'h',
    DataType.VECTOR_INT8: 'c',
    DataType.VECTOR_BINARY32: 'I',
    DataType.VECTOR_BINARY64: 'Q',
}


def _infer_dimension_and_batch_count(features, dimension, batch_count, data_type):
    inferred_dim = None
    inferred_batch = None
    if dimension is None or batch_count is None:
        if isinstance(features, list):
            inferred_dim = len(features[0]) * _data_type_to_dimension.get(data_type, 1)
            inferred_batch = len(features)
    if dimension is None:
        dimension = inferred_dim
    if batch_count is None:
        batch_count = inferred_batch
    if not dimension or not batch_count:
        raise ProximaBeException(
            f"Empty dimension[{dimension}] or batch_count[{batch_count}]")
    return dimension, batch_count


def _pack_feature(feature, data_type, dimension):
    format_dimension = dimension // _data_type_to_dimension.get(data_type, 1)
    if data_type not in _data_type_to_format:
        raise ProximaBeException(
            f'not support auto pack feature type[{data_type}]')
    return struct.pack(f'<{format_dimension}{_data_type_to_format[data_type]}', *feature)


def _build_features(features, data_type, dimension):
    if isinstance(features, bytes):
        return features
    if not isinstance(features, list):
        raise ProximaBeException(
            f"unsupported features type[{type(features)}]")
    bs = []
    for feature in features:
        bs.append(_pack_feature(feature, data_type, dimension))
    return b''.join(bs)


# pylint: disable=too-many-arguments
def _build_knn_query(collection_name,
                     column_name,
                     features,
                     data_type=None,
                     batch_count=None,
                     dimension=None,
                     topk=100,
                     is_linear=False,
                     radius=None,
                     extra_params=None,
                     debug=False):
    req = proxima_be_pb2.QueryRequest()
    req.query_type = proxima_be_pb2.QueryRequest.QueryType.QT_KNN
    req.collection_name = collection_name
    knn = req.knn_param
    knn.column_name = column_name
    knn.topk = topk
    if not features:
        raise ProximaBeException(f'Empty features:{features}')
    if isinstance(features, list) and not isinstance(features[0], list):
        # get list(list(...))
        features = [features]
    # get data_type enum
    if data_type is None:
        if not isinstance(features, bytes):
            raise ProximaBeException(f'data_type should be set for non bytes features')
    else:
        data_type = _parse_enum_value_or_string(DataType, common_pb2.DataType,
                                                data_type, 'DT_')
        if not data_type.name.startswith('VECTOR'):
            raise ProximaBeException(
                f'Invalid data_type[{knn.data_type}], expect vector type')
        knn.data_type = data_type.value
    knn.dimension, knn.batch_count = _infer_dimension_and_batch_count(
        features, dimension, batch_count, knn.data_type)
    if isinstance(features, str):
        knn.matrix = features
    else:
        knn.features = _build_features(features, knn.data_type, knn.dimension)
    if radius is not None:
        knn.radius = radius
    knn.is_linear = is_linear
    if extra_params is not None:
        knn.extra_params.extend(
            _key_value_pair(key, value) for key, value in extra_params.items())
    req.debug_mode = debug
    return req


def _build_collection_name(collection_name):
    pb = proxima_be_pb2.CollectionName()
    pb.collection_name = collection_name
    return pb


def _build_list_condition(repository_name):
    pb = proxima_be_pb2.ListCondition()
    if repository_name is not None:
        pb.repository_name = repository_name
    return pb


def _generic_value(value, data_type):
    generic_value = common_pb2.GenericValue()
    type_to_attr = {
        DataType.BINARY: 'bytes_value',
        DataType.BOOL: 'bool_value',
        DataType.INT32: 'int32_value',
        DataType.INT64: 'int64_value',
        DataType.UINT32: 'uint32_value',
        DataType.UINT64: 'uint64_value',
        DataType.FLOAT: 'float_value',
        DataType.DOUBLE: 'double_value',
        DataType.STRING: 'string_value',
    }
    if data_type not in type_to_attr:
        raise ProximaBeException(
            f"Unsupported type[{type}], supported={type_to_attr.keys()}")
    setattr(generic_value, type_to_attr[data_type], value)
    return generic_value


def _index_value_to_generic_value(value):
    if isinstance(value, str):
        return _generic_value(value, DataType.STRING)
    if isinstance(value, bytes):
        return _generic_value(value, DataType.BINARY)
    raise ProximaBeException(
        f'Index value only support str or bytes, got {type(value)}')


def _parse_repository_from_pb(pb_collection):
    if not pb_collection.HasField('repository_config'):
        return None
    pb_repo = pb_collection.repository_config
    if pb_repo.repository_type == proxima_be_pb2.CollectionConfig.RepositoryConfig.RepositoryType.RT_DATABASE:
        return DatabaseRepository.from_pb(pb_repo)
    raise ProximaBeException(
        f'Unexpected repository type, repo=[{str(pb_repo)}]')


def _parse_generic_value_from_pb(pb_generic_value):
    if not pb_generic_value.HasField('value_oneof'):
        return None
    field_name = pb_generic_value.WhichOneof('value_oneof')
    return getattr(pb_generic_value, field_name)


def _default_if_none(value, default_value):
    return value if value is not None else default_value


def _stringify(self):
    return f'{type(self).__name__}{vars(self)}'


def _check_version(status, version):
    if not status.ok():
        raise ProximaBeException(f'Get server version failed, status={status}')
    client_version = __version__.split('.', maxsplit=2)[:2]
    server_version = version.split('.', maxsplit=2)[:2]
    if client_version != server_version:
        raise ProximaBeException(f'Version mismatch, client_version={__version__}, server_version={version}')

class _TimerStage(Enum):
    serialization = 1
    rpc = 2
    deserialization = 3
    build_pb_obj = 4
    build_py_obj = 5
    total = 6

class _Timer(object):
    def __init__(self):
        self._start_time = time.perf_counter()
        self._starts_map = {}
        self.timer_report = {}

    def begin_stage(self, *names):
        now = time.perf_counter()
        for n in names:
            self._starts_map[n] = now

    def end_stage(self, *names, start_stage_name=None):
        now = time.perf_counter()
        for n in names:
            self.timer_report[n] = now - self._starts_map.get(n, self._start_time)
        if start_stage_name:
            self._starts_map[start_stage_name] = now

    @staticmethod
    def begin_stage_helper(timer, *names):
        if timer is not None:
            timer.begin_stage(*names)

    @staticmethod
    def end_stage_helper(timer, *names, start_stage_name=None):
        if timer is not None:
            timer.end_stage(*names, start_stage_name=start_stage_name)
