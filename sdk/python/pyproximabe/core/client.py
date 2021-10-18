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

import abc

from .types import ProximaBeException
from .types import _build_get_document_request
from .types import _build_knn_query
from .types import _build_list_condition
from .types import WriteRequest
from .types import _check_version
from .types import _Timer
from .types import _TimerStage

from .handlers import HttpHandler
from .handlers import GrpcHandler
from .handlers import AsyncGrpcHandler


class BaseClient(abc.ABC):
    """
    BaseClient.
    """
    @abc.abstractmethod
    def __init__(self, handler):
        self._handler = handler
        self._check_version()

    def create_collection(self, collection_config):
        """
        Create collection.

        Args:
            collection_config (CollectionConfig): collection config.

        Returns:
            ProximaBeStatus: status
        """
        return self._handler.create_collection(collection_config.to_pb())

    def describe_collection(self, collection_name):
        """
        Describe collection.

        Args:
            collection_name (str):  collection name

        Returns:
            tuple: 2-element tuple containing
              * :class:`ProximaBeStatus`: status
              * :class:`CollectionInfo`: collection information
        """
        return self._handler.describe_collection(collection_name)

    def drop_collection(self, collection_name):
        """
        Drop collection.

        Args:
            collection_name (str): collection name

        Returns:
            ProximaBeStatus: status
        """
        return self._handler.drop_collection(collection_name)

    def stats_collection(self, collection_name):
        """
        Stats collection.

        Args:
            collection_name (str): collection name

        Returns:
            tuple: 2-element tuple containing
              * :class:`ProximaBeStatus`: status
              * :class:`CollectionStats`: collection statistics
        """
        return self._handler.stats_collection(collection_name)

    def list_collections(self, repository_name=None):
        """
        List all collections.

        Args:
            repository_name (Optional[str]): repository name

        Returns:
            tuple: 2-element tuple containing
              * :class:`ProximaBeStatus`: status
              * List[:class:`CollectionInfo`]: list of collection info
        """
        return self._handler.list_collections(
            _build_list_condition(repository_name))

    # pylint: disable=too-many-arguments
    def query(self,
              collection_name,
              column_name,
              features,
              data_type=None,
              dimension=None,
              batch_count=None,
              topk=100,
              is_linear=False,
              radius=None,
              extra_params=None,
              **kwargs):
        """
        Query documents using vector search.

        Args:
            collection_name (str): collection name
            column_name (str): index column name
            features (Union[bytes, str, List[List[Union[float, int, ...]]]]):
              vector features in the following formats:
               * bytes of little endian order.
               * json string in the following format
                   * flatten json array, e.g. '[1.0,2.0,3.0,4.0]' with 2 batch of 2 dimensional vectors
                   * json array of json array, e.g. '[[1.0,2.0],[3.0,4.0]]' with above case
               * list of vectors, e.g. [[1.0,2.0,3.0], [4.0, 5.0, 6.0]]
            data_type (Optional[Union[DataType, str, int]]):
              vector data type.
               * optional for bytes features.
               * required for list of vectors features.
            dimension (Optional[int]):
              vector dimension.
               * required for bytes features.
               * auto computed for list of vectors features.
            batch_count (Optional[int]):
              query batch.
               * required for bytes features.
               * auto computed for list of vectors features.
            topk (int): result number to be returned.
            is_linear (bool): whether to linear search.
            radius (Optional[float]): return only documents within `radius` distance from query.
            extra_params (Optional[dict]): extra parameters.

        Returns:
            tuple: 2-element tuple containing
              * :class:`ProximaBeStatus`: status
              * :class:`QueryResponse`: query response, with ``batch_count`` number of :class:`QueryResponse.Result` s,
                each with at most ``topk`` number of :class:`Document` s.

        Examples:
            >>> # query with binary feature，batch_count and dimension must be set, data_type is optional
            >>> client.query(collection_name='collection_xxx', column_name='index_xxx', features=b'xxx',
            ...        dimension = 256, batch_count=10)

            >>> # query with lists of vectors feature, data_type must be set,
            >>> # batch_count and dimension can be auto inferred
            >>> client.query(collection_name='collection_xxx', column_name='index_xxx', features=[1.0, 2.0, ...],
            ...        data_type = Datatype.VECTOR_FP32)

        """
        debug = kwargs.get('debug', False)
        timer = None
        if debug:
            timer = _Timer()
        req = _build_knn_query(collection_name, column_name, features,
                               data_type, batch_count, dimension, topk,
                               is_linear, radius, extra_params, debug)
        _Timer.end_stage_helper(timer, _TimerStage.build_pb_obj.name)
        return self._handler.query(req, timer=timer, **kwargs)

    def get_document_by_key(self, collection_name, primary_key):
        """
        Query document by primary key.

        Args:
            collection_name (str): collection name
            primary_key (long): primary key

        Returns:
            tuple: 2-element tuple containing
              * :class:`ProximaBeStatus`: status. status.ok() if server succeeds(including key not exists).
              * :class:`Document`: document. None if key not exists.

        """
        req = _build_get_document_request(collection_name, primary_key)
        return self._handler.get_document_by_key(req)

    def write(self, write_request):
        """
        Write batch documents to proxima be.

        Args:
            write_request (WriteRequest): write request.

        Returns:
            ProximaBeStatus: status

        """
        return self._handler.write(write_request.to_pb())

    def delete_document_by_keys(self, collection_name, primary_keys):
        """
        Delete documents by key.

        Args:
            collection_name (str): collection name
            primary_keys (List[long]): primary keys

        Returns:
            ProximaBeStatus: status

        """
        if not isinstance(primary_keys, list):
            primary_keys = [primary_keys]
        rows = []
        for pk in primary_keys:
            row = WriteRequest.Row(
                primary_key=pk,
                operation_type=WriteRequest.OperationType.DELETE,
            )
            rows.append(row)
        delete_request = WriteRequest(
            collection_name=collection_name,
            rows=rows
        )
        return self.write(delete_request)

    def close(self):
        """
        Close connection.

        Returns: None
        """
        return self._handler.close()

    def _check_version(self):
        status, version = self._handler.get_version()
        _check_version(status, version)


class Client(BaseClient):
    def __init__(self, host, port=16000, handler='grpc', timeout=10):
        """
        Constructor.

        Args:
            host (str): hostname
            port (int): port
            handler (str): use grpc or http, defaults to grpc.
            timeout (Optional[float]): timeout in seconds, default to 10. Passing None means no timeout.
        """
        if handler.lower() == 'http':
            handler = HttpHandler(host, port, timeout)
        elif handler.lower() == 'grpc':
            handler = GrpcHandler(host, port, timeout)
        else:
            raise ProximaBeException(f"Invalid handler type[{handler}]")
        super().__init__(handler)


class AsyncClient(BaseClient):
    def __init__(self, host, port=16000, handler='grpc', timeout=10):
        """
        Constructor.

        Args:
            host (str): hostname
            port (int): port
            handler (str): only grpc is supported for now.
            timeout (Optional[float]): timeout in seconds, default to 10. Passing None means no timeout.
        """
        if handler.lower() != 'grpc':
            raise ProximaBeException("AsyncClient only support grpc")
        super().__init__(AsyncGrpcHandler(host, port, timeout))
