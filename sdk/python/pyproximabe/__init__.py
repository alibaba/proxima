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

from .version import __version__

# import grpc before importing protobuf generated modules, to fix arm64 crashing problem
# see https://github.com/grpc/grpc/issues/26279
import grpc

# hack to import proto
import sys
import os.path
sys.path.insert(0, os.path.dirname(__file__))

from .core.client import Client
from .core.client import AsyncClient

from .core.types import IndexColumnParam
from .core.types import CollectionConfig
from .core.types import CollectionInfo
from .core.types import DataType
from .core.types import IndexType
from .core.types import DatabaseRepository
from .core.types import WriteRequest
from .core.types import QueryResponse
from .core.types import Document
from .core.types import LsnContext
from .core.types import ProximaBeStatus
from .core.types import CollectionStats

sys.path.pop(0)
del sys
del os
