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

import requests
import grpc
import orjson as json

from google.protobuf.json_format import MessageToJson
from google.protobuf.json_format import Parse
from google.protobuf.message import Message

from .types import ProximaBeException
from .types import ProximaBeStatus
from .types import CollectionInfo
from .types import CollectionStats
from .types import QueryResponse
from .types import Document
from .types import _TimerStage
from .types import _Timer

from .types import common_pb2
from .types import proxima_be_pb2
from .types import proxima_be_pb2_grpc
from .types import _build_collection_name


class BaseHandler:
    @staticmethod
    def _parse_response(pb_or_txt,
                        response_type=None,
                        attr_name=None,
                        message=None,
                        **kwargs):
        timer = kwargs.get('timer')
        pb = pb_or_txt
        if message is not None:
            _Timer.begin_stage_helper(timer, _TimerStage.deserialization.name)
            pb = Parse(pb, message)
            _Timer.end_stage_helper(timer, _TimerStage.deserialization.name)
        if isinstance(pb, common_pb2.Status):
            _Timer.end_stage_helper(timer, _TimerStage.total.name)
            return ProximaBeStatus(pb.code, pb.reason)
        status = ProximaBeStatus(pb.status.code, pb.status.reason)
        status.update_timer_report(timer)
        if not status.ok():
            return status, None
        rsp = pb
        if response_type is not None:
            _Timer.begin_stage_helper(timer, _TimerStage.build_py_obj.name)
            obj = pb
            if attr_name is not None:
                if not pb.HasField(attr_name):
                    return status, None
                obj = getattr(pb, attr_name)
            rsp = response_type.from_pb(obj)
            _Timer.end_stage_helper(timer, _TimerStage.build_py_obj.name)
        _Timer.end_stage_helper(timer, _TimerStage.total.name)
        BaseHandler._merge_client_debug_info(rsp, timer)
        return status, rsp

    @staticmethod
    def _merge_client_debug_info(rsp, timer):
        if timer is not None and hasattr(rsp, 'debug_info'):
            debug_info = getattr(rsp, 'debug_info')
            try:
                debug_root = json.loads(debug_info)
                debug_root['client'] = timer.timer_report
                new_debug_info = json.dumps(debug_root).decode()
                setattr(rsp, 'debug_info', new_debug_info)
            except Exception:
                pass

    @staticmethod
    def _parse_collection_lists(status, list_response):
        if not status.ok():
            return status, []
        return status, [
            CollectionInfo.from_pb(c) for c in list_response.collections
        ]

    def _parse_status(self, response):
        return self._parse_response(response)


class HttpHandler(BaseHandler):
    _MANAGE_COLLECTION_URL = "/v1/collection/"
    _STATS_COLLECTION_URL = "/v1/collection/%s/stats"
    _LIST_COLLECTIONS_URL = "/v1/collections"
    _WRITE_URL = "/v1/collection/%s/index"
    _QUERY_URL = "/v1/collection/%s/query"
    _GET_DOCUMENT_BY_KEY_URL = "/v1/collection/%s/doc?key=%d"
    _VERSION_URL = "/service_version"

    def __init__(self, host, port, timeout):
        # hostname
        self._host = host
        # port
        self._port = port
        # base url
        self._base_url = f'http://{self._host}:{self._port}'
        # to use connection pool
        self._session = requests.Session()
        # timeout
        self._timeout = timeout

    def create_collection(self, config):
        rsp = self._send_request(self._MANAGE_COLLECTION_URL +
                                 config.collection_name,
                                 config)
        return self._parse_status(rsp)

    def drop_collection(self, collection_name):
        rsp = self._send_request(self._MANAGE_COLLECTION_URL +
                                 collection_name,
                                 method='DELETE')
        return self._parse_status(rsp)

    def describe_collection(self, collection_name):
        rsp = self._send_request(self._MANAGE_COLLECTION_URL +
                                 collection_name,
                                 method='GET')
        return self._parse_response(
            rsp, CollectionInfo, 'collection',
            proxima_be_pb2.DescribeCollectionResponse())

    def list_collections(self, list_condition):
        query = ""
        if list_condition.repository_name is not None:
            query = f'?repository={list_condition.repository_name}'

        rsp = self._send_request(self._LIST_COLLECTIONS_URL + query,
                                 list_condition,
                                 method='GET')
        pb_rsp = self._parse_response(
            rsp, message=proxima_be_pb2.ListCollectionsResponse())
        return self._parse_collection_lists(*pb_rsp)

    def stats_collection(self, collection):
        rsp = self._send_request(self._STATS_COLLECTION_URL % collection,
                                 method='GET')
        return self._parse_response(rsp, CollectionStats, 'collection_stats',
                                    proxima_be_pb2.StatsCollectionResponse())

    def write(self, req):
        rsp = self._send_request(self._WRITE_URL % req.collection_name,
                                 req, method='POST')
        return self._parse_status(rsp)

    def query(self, req, **kwargs):
        timer = kwargs.get('timer')
        rsp = self._send_request(self._QUERY_URL % req.collection_name,
                                 req, method='POST', **kwargs)
        _Timer.begin_stage_helper(timer, _TimerStage.deserialization.name)
        root = json.loads(rsp)
        _Timer.end_stage_helper(timer, _TimerStage.deserialization.name, start_stage_name=_TimerStage.build_py_obj.name)
        status = root['status']
        se_status = ProximaBeStatus(status['code'], status['reason'])
        se_status.update_timer_report(timer)
        if not se_status.ok():
            return se_status, None
        rsp = QueryResponse.from_json(root)
        _Timer.end_stage_helper(timer, _TimerStage.build_py_obj.name, _TimerStage.total.name)
        self._merge_client_debug_info(rsp, timer)
        return se_status, rsp

    def get_document_by_key(self, req):
        rsp = self._send_request(self._GET_DOCUMENT_BY_KEY_URL % (
            req.collection_name, req.primary_key),
                                 method='GET')
        return self._parse_response(rsp, Document, 'document',
                                    proxima_be_pb2.GetDocumentResponse())

    def get_version(self):
        rsp = self._send_request(self._VERSION_URL, method='GET')
        status, pb = self._parse_response(rsp, message=proxima_be_pb2.GetVersionResponse())
        return status, pb.version

    def close(self):
        self._session.close()

    def _parse_status(self, response):
        return self._parse_response(response, message=common_pb2.Status())

    def _send_request(self, url, body=None, method='POST', **kwargs):
        """
        Send http request.

        Args:
            url (str): url
            body (Optional[str,google.protobuf.Message]: http request body.
                  Converted to json if it's google.protobuf.Message.
            method (str): http request method.

        Returns:
            response body.

        Raises:
            ProximaBeException on error.
        """
        timer = kwargs.get('timer')
        try:
            full_url = f'{self._base_url}{url}'
            if isinstance(body, Message):
                _Timer.begin_stage_helper(timer, _TimerStage.serialization.name)
                body = MessageToJson(body)
                _Timer.end_stage_helper(timer, _TimerStage.serialization.name)
            _Timer.begin_stage_helper(timer, _TimerStage.rpc.name)
            r = self._session.request(method=method, url=full_url, data=body, timeout=self._timeout)
            _Timer.end_stage_helper(timer, _TimerStage.rpc.name)
            if r.status_code != 200:
                raise ProximaBeException(
                    f'Unexpected return code[{r.status_code}] and text[{r.text}]'
                )
            return r.text
        except ProximaBeException:
            raise
        except requests.RequestException as e:
            raise ProximaBeException(
                f'Unexpected network error, url=[{full_url}]') from e
        except Exception as e:
            raise ProximaBeException('Unexpected exception') from e


class GrpcHandler(BaseHandler):
    def __init__(self, host, port, timeout):
        # hostname
        self._host = host
        # port
        self._port = port
        spec = f'{self._host}:{self._port}'
        self._channel = grpc.insecure_channel(spec)
        self._stub = proxima_be_pb2_grpc.ProximaServiceStub(self._channel)
        self._timeout = timeout

    def create_collection(self, collection_config):
        rsp = self._stub.create_collection(collection_config, timeout=self._timeout)
        return self._parse_status(rsp)

    def drop_collection(self, collection_name):
        rsp = self._stub.drop_collection(
            _build_collection_name(collection_name), timeout=self._timeout)
        return self._parse_status(rsp)

    def describe_collection(self, collection_name):
        rsp = self._stub.describe_collection(
            _build_collection_name(collection_name), timeout=self._timeout)
        return self._parse_response(rsp, CollectionInfo, 'collection')

    def list_collections(self, list_condition):
        rsp = self._stub.list_collections(list_condition, timeout=self._timeout)
        status, rsp = self._parse_response(rsp)
        return self._parse_collection_lists(status, rsp)

    def stats_collection(self, collection_name):
        rsp = self._stub.stats_collection(
            _build_collection_name(collection_name), timeout=self._timeout)
        return self._parse_response(rsp, CollectionStats, 'collection_stats')

    def write(self, write_request):
        rsp = self._stub.write(write_request, timeout=self._timeout)
        return self._parse_status(rsp)

    def query(self, query_request, **kwargs):
        timer = kwargs.get('timer')
        _Timer.begin_stage_helper(timer, _TimerStage.rpc.name)
        rsp = self._stub.query(query_request, timeout=self._timeout)
        _Timer.end_stage_helper(timer, _TimerStage.rpc.name)
        return self._parse_response(rsp, QueryResponse, **kwargs)

    def get_document_by_key(self, get_document_request):
        rsp = self._stub.get_document_by_key(get_document_request, timeout=self._timeout)
        return self._parse_response(rsp, Document, 'document')

    def get_version(self):
        rsp = self._stub.get_version(proxima_be_pb2.GetVersionRequest(), timeout=self._timeout)
        status, pb = self._parse_response(rsp)
        return status, pb.version

    def close(self):
        self._channel.close()


class AsyncGrpcHandler(BaseHandler):
    def __init__(self, host, port, timeout):
        # hostname
        self._host = host
        # port
        self._port = port
        self._spec = f'{self._host}:{self._port}'
        self._channel = grpc.aio.insecure_channel(self._spec)
        self._stub = proxima_be_pb2_grpc.ProximaServiceStub(self._channel)
        self._timeout = timeout

    async def create_collection(self, collection_config):
        rsp = await self._stub.create_collection(collection_config, timeout=self._timeout)
        return self._parse_status(rsp)

    async def drop_collection(self, collection_name):
        rsp = await self._stub.drop_collection(
            _build_collection_name(collection_name), timeout=self._timeout)
        return self._parse_status(rsp)

    async def describe_collection(self, collection_name):
        rsp = await self._stub.describe_collection(
            _build_collection_name(collection_name), timeout=self._timeout)
        return self._parse_response(rsp, CollectionInfo, 'collection')

    async def list_collections(self, list_condition):
        rsp = await self._stub.list_collections(list_condition, timeout=self._timeout)
        status, rsp = self._parse_response(rsp)
        return self._parse_collection_lists(status, rsp)

    async def stats_collection(self, collection_name):
        rsp = await self._stub.stats_collection(
            _build_collection_name(collection_name), timeout=self._timeout)
        return self._parse_response(rsp, CollectionStats, 'collection_stats')

    async def write(self, write_request):
        rsp = await self._stub.write(write_request, timeout=self._timeout)
        return self._parse_status(rsp)

    async def query(self, query_request, **kwargs):
        timer = kwargs.get('timer')
        _Timer.begin_stage_helper(timer, _TimerStage.rpc.name)
        rsp = await self._stub.query(query_request, timeout=self._timeout)
        _Timer.end_stage_helper(timer, _TimerStage.rpc.name)
        return self._parse_response(rsp, QueryResponse, **kwargs)

    async def get_document_by_key(self, get_document_request):
        rsp = await self._stub.get_document_by_key(get_document_request, timeout=self._timeout)
        return self._parse_response(rsp, Document, 'document')

    def get_version(self):
        # use synchronous channel to get version
        # Another option is to use asynchronous channel like below comment.
        # But asyncio does not support nested event loop by design,
        # nest_asyncio is needed to patch event loop, which may lead to other issues.
        channel = grpc.insecure_channel(self._spec)
        stub = proxima_be_pb2_grpc.ProximaServiceStub(channel)
        rsp = stub.get_version(proxima_be_pb2.GetVersionRequest(), timeout=self._timeout)
        status, pb = self._parse_response(rsp)
        channel.close()
        return status, pb.version

        # loop = asyncio.get_event_loop()
        # nest_asyncio.apply(loop)
        # rsp = loop.run_until_complete(self._stub.get_version(proxima_be_pb2.GetVersionRequest()))
        # status, pb = self._parse_response(rsp)
        # return status, pb.version

    async def close(self):
        await self._channel.close()
