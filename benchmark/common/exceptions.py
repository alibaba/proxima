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
# base library for proxima se bench toolkits
#

__all__ = ['ProximaSEException', 'FileNotExits']


class ProximaSEException(Exception):
    """Base Error Unknown type."""

    def __init__(self, message, response_header, response_data):
        ProximaSEException.__init__(
            message, response_header, response_data)


class FileNotExits(ProximaSEException):
    def __init__(self, file):
        ProximaSEException.__init__("%s does not exists" % file,
                                    None, None)


class PropKeyExceptions(ProximaSEException):
    def __init__(self, key):
        ProximaSEException.__init__("%s does not exists" % key,
                                    None, None)
