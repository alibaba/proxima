#! /usr/bin/env python
# -*- coding: utf8 -*-
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
# Service test script
#

from common.proxima_se_service import *


def test_proxima_se_service(service: ProximaSEService):
    if service.init():
        logging.info(f'Initialize {service.service_name()} succeed')
    else:
        logging.error(f'Failed initialize {service.service_name()}')
        return

    if service.start():
        logging.info(f'Start {service.service_name()} success')
    else:
        service.cleanup()
        logging.error(f'Start {service.service_name()} failed')
        return

    logging.info(f'Status: {service.status().name}')
    time.sleep(10)

    if service.stop():
        logging.info(f'Stop {service.service_name()} success')
    else:
        logging.error(f'Stop {service.service_name()} failed')

    logging.info(f'Status: {service.status().name}')
    if service.cleanup():
        logging.info(f'Cleanup {service.service_name()} succeed')


def test_mysql_repo():
    test_proxima_se_service(
        ProximaSEMysqlRepo("../../cmake-build-debug/bin/mysql_repository", "conf/mysql_repo.conf", "bench"))


def test_proxima_se():
    test_proxima_se_service(
        ProximaSE("../../cmake-build-debug/bin/proxima_se", "conf/proxima_se.conf", "bench"))


if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO)
    test_mysql_repo()
    test_proxima_se()
    exit(0)
