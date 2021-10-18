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

import logging

root_logger= logging.getLogger()
root_logger.setLevel(logging.INFO)
handler = logging.FileHandler('it.log', 'a', 'utf-8')
formatter = logging.Formatter(
    '%(levelname)s %(asctime)s %(filename)s:%(lineno)d:%(funcName)s \t%(message)s')
handler.setFormatter(formatter)
root_logger.addHandler(handler)

# logging.basicConfig(level=logging.INFO, filename="it.log", format="%(levelname)s %(asctime)s %(filename)s:%(lineno)d:%(funcName)s \t%(message)s")
