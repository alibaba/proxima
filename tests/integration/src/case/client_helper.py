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

import random

from pyproximabe import Client
import logging


def get_client(global_conf):
    handler = random.choice(['grpc', 'http'])
    logging.info('use %s', handler)
    host = '127.0.0.1'
    timeout = None
    if handler == 'http':
        client = Client(host, global_conf.http_port(), handler, timeout)
    else:
        client = Client(host, global_conf.grpc_port(), handler, timeout)
    return client
