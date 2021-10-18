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
# Proxima Service management module
#
import http
import os
import re
import logging
import signal
import tempfile
import threading
import time
from enum import IntEnum

from google.protobuf.text_format import MessageToString, PrintMessage, Parse

from .runner import ShellRunner

from pyproximase.proto import config_pb2

GRPC_PORT = 16000
HTTP_PORT = 16100


class ServiceListener(object):
    def on_failed(self, name) -> bool:
        logging.info(f'Service({name}) failed!!')
        return True

    def on_finished(self, name) -> bool:
        logging.info(f'Service({name}) finished!!')
        return True


class ServiceStatus(IntEnum):
    INITIALIZED = 0
    RUNNING = 1
    FAILED = 2
    FINISHED = 3
    UNINITIALIZED = 4


class ProximaSEService(object):
    def __init__(self, service_name, working_dir, listener):
        self._service_name = service_name
        self._working_dir = working_dir
        self._listener = listener

    def __del__(self):
        pass

    @staticmethod
    def write_config_file(config_file, config):
        with open(config_file, 'w+') as out:
            PrintMessage(config, out)
            logging.info("Write config(%s): \n%s" % (config_file, MessageToString(config)))

    @staticmethod
    def write_file(config_file, config):
        with open(config_file, 'w+') as out:
            out.write(config)

    class Stats(object):
        def __init__(self, cpu=0, memory=0):
            self._cpu = cpu
            self._memory = memory

        def cpu(self):
            return round(self._cpu, 2)

        def memory_in_bytes(self):
            return self._memory

        def memory_in_kb(self):
            return round(float(self._memory) / 1024, 2)

        def memory_in_mb(self):
            return round(self.memory_in_kb() / 1024, 2)

        def memory_in_gb(self):
            return round(self.memory_in_mb() / 1024, 2)

    def service_name(self):
        return self._service_name

    def working_dir(self):
        return self._working_dir

    def listener(self):
        return self._listener

    def init(self) -> bool:
        pass

    def cleanup(self) -> bool:
        pass

    def start(self) -> bool:
        pass

    def stop(self) -> bool:
        pass

    def stdout_log(self) -> str:
        pass

    def stderr_log(self) -> str:
        pass

    def status(self) -> ServiceStatus:
        pass

    def service_addr(self) -> str:
        pass

    def stats(self) -> Stats:
        pass


class LogHandler(object):
    def on_stdin(self, msg):
        pass

    def on_stderr(self, msg):
        pass


class LoggingMonitor(threading.Thread):
    def __init__(self, process, handler, interval=0.01):
        """
        @param interval: sleep seconds between dump log
        """
        threading.Thread.__init__(self)
        self._process = process
        self._handler = handler
        self._interval = interval
        self._stopped = True

    def run(self):
        self._stopped = False
        while not self._stopped:
            if self._process.poll():  # sub process finished
                break

            if ShellRunner.readable(self._process.stdout):
                self._handler.on_stdin(self._process.stdout.read())
            elif ShellRunner.readable(self._process.stderr):
                self._handler.on_stderr(self._process.stderr.read())
            else:
                time.sleep(self._interval)

    def stop(self):
        self._stopped = True


class ProximaSEBaseService(ProximaSEService, LogHandler):
    def __init__(self, service_name, binary, config, working_dir, listener, cleanup_logs):
        ProximaSEService.__init__(self, service_name, working_dir, listener)
        self._binary = binary
        self._config = config
        self._log = self.stdout_log()
        self._stdout_fd = None
        self._process = None
        self._log_monitor = None
        self._cleanup_logs = cleanup_logs

    def __del__(self):
        self.cleanup()
        self._process = None

    def init(self) -> bool:
        if self._process:
            logging.info(f'Service({self.service_name()}) have been initialized')
            return True

        if not os.path.isdir(self.working_dir()) or os.path.isfile(self._log):
            logging.error(f'Working directory({self.working_dir()}) does not exist or log file({self._log}) exists')
            return False

        if not os.path.isfile(self._binary) or not os.path.isfile(self._config):
            logging.error(
                f'Service({self.service_name()}) binary({self._binary}) or config({self._config}) do not exist')
            return False

        return True

    @staticmethod
    def _remove_log(log):
        if log and os.path.isfile(log):
            os.remove(log)

    def cleanup(self) -> bool:
        if self._process:
            self.stop()
        if self._cleanup_logs:
            self._remove_log(self.stdout_log())
            self._remove_log(self.stderr_log())
        return True

    def start(self) -> bool:
        return False

    def stop(self) -> bool:
        logging.info(f'Stop {self.service_name()} process')
        # Stop sub process
        if self._process and not self._process.poll():
            self._process.terminate()
            self._process.wait()
            # Replaced by signal user2
            # self._process.send_signal(signal.SIGUSR2)
            # code = self._process.poll()
            # if not code:
            #     self._process.terminate()
            #     self._process.wait()

        # Stop logging thread
        if self._log_monitor:
            self._log_monitor.stop()
            self._log_monitor.join()
            self._log_monitor = None

        # CLose log file
        if self._stdout_fd:
            self._stdout_fd.close()
            self._stdout_fd = None
        return True

    def running(self):
        return self.status() == ServiceStatus.RUNNING

    def status(self) -> ServiceStatus:
        if self._process:
            code = self._process.poll()
            if not code:
                return ServiceStatus.RUNNING
            elif code in [0, 1, signal.SIGTERM, signal.SIGUSR2]:
                return ServiceStatus.FINISHED
            logging.info(f'{self.service_name()} returned with code: {code}')
            return ServiceStatus.FAILED

        return ServiceStatus.UNINITIALIZED

    def service_addr(self) -> str:
        return ''

    def _write_log(self, msg):
        if self._stdout_fd and msg:
            self._stdout_fd.write(msg)

    def on_stdin(self, msg):
        self._write_log(msg)

    def on_stderr(self, msg):
        self._write_log(msg)

    def stats(self) -> ProximaSEService.Stats:
        return ProximaSEService.Stats()


def _read_int_value(key, default, **kwargs):
    return int(kwargs[key]) if key in kwargs else default


def _read_str_value(key, default, **kwargs):
    return kwargs[key] if key in kwargs else default


class ProximaSEMysqlRepo(ProximaSEBaseService):
    def __init__(self, binary, config, working_dir, listener=ServiceListener(), cleanup_logs=False):
        ProximaSEBaseService.__init__(self, "ProximaSE MYSQL Repository", binary, config, working_dir, listener,
                                      cleanup_logs)

    @staticmethod
    def build_config(**kwargs):
        log_directory = _read_str_value("log_directory", "log", **kwargs)
        index_agent_addr = _read_str_value("index_agent_addr", None, **kwargs)
        if not index_agent_addr:
            listen_port = _read_int_value("index_port", GRPC_PORT, **kwargs)
            index_agent_addr = f'127.0.0.1:{listen_port}'

        repository_name = _read_str_value("repository_name", None, **kwargs)
        return """common_config {
            log_directory: "%s"
            log_file: "mysql_repo.log"
        }
        repository_config {
            index_agent_addr: "%s"
            repository_name: "%s"
        }""" % (log_directory, index_agent_addr, repository_name)

    @staticmethod
    def build_config_file(config_file, **kwargs):
        config = ProximaSEMysqlRepo.build_config(**kwargs)
        ProximaSEService.write_file(config_file, config)

    def start(self) -> bool:
        if self.running():
            logging.warning(f'{self.service_name()} have been started, please call stop first')
            return False

        logging.info(f'Start {self.service_name()} process')
        # Start Service as sub process
        self._process = ShellRunner.aync_execute([self._binary, '--config', self._config])
        self._stdout_fd = open(self._log, 'w+b')
        # Redirect log of Service
        self._log_monitor = LoggingMonitor(self._process, self)
        self._log_monitor.start()
        return True

    def stdout_log(self) -> str:
        return os.path.join(self.working_dir(), "mysql.repo.log") if self.working_dir() else None

    def stderr_log(self) -> str:
        return ''

    def on_stdin(self, msg):
        ProximaSEBaseService.on_stdin(self, msg)

    def on_stderr(self, msg):
        ProximaSEBaseService.on_stderr(self, msg)


class ProximaSEBVarService(object):
    @staticmethod
    def process_stat(address) -> ProximaSEService.Stats:
        try:
            conn = http.client.HTTPConnection(address)
            conn.request("GET", "/vars/process_cpu_usage,system_core_count,process_memory_resident")
            resp = conn.getresponse().read().decode('UTF-8')
            conn.close()
            match = re.match(
                r"process_cpu_usage : (.*)\r\nprocess_memory_resident : (.*)\r\nsystem_core_count : (.*)\r\n", resp,
                re.M | re.I)
            return ProximaSEService.Stats(float(match.group(1)) * int(match.group(3)), int(match.group(2)))
        except Exception as e:
            logging.error(e)
        return ProximaSEService.Stats()

    @staticmethod
    def api_status(address):
        try:
            conn = http.client.HTTPConnection(address)
            conn.request("GET", "/status")
            resp = conn.getresponse().read().decode('UTF-8')
            conn.close()
            return resp
        except Exception as e:
            logging.error(e)
        return ""


class ProximaSE(ProximaSEBaseService):
    def __init__(self, binary, config, working_dir, listener=ServiceListener(), cleanup_logs=False):
        ProximaSEBaseService.__init__(self, "ProximaSE", binary, config, working_dir, listener, cleanup_logs)
        self._stderr_fd = None
        pass

    @staticmethod
    def build_meta_file(meta_file=None):
        temp_file = tempfile.mktemp(suffix=".sqlite") if not meta_file else meta_file
        return f'sqlite://{temp_file}'

    @staticmethod
    def build_config(**kwargs):
        config = config_pb2.ProximaSEConfig()
        # Common Config
        config.common_config.log_directory = _read_str_value("log_directory", "log", **kwargs)
        config.common_config.grpc_listen_port = _read_int_value("grpc_port", GRPC_PORT, **kwargs)
        config.common_config.http_listen_port = _read_int_value("http_port", HTTP_PORT, **kwargs)
        # Index Config
        config.index_config.build_thread_count = _read_int_value("index_build_threads", 10, **kwargs)
        config.index_config.dump_thread_count = _read_int_value("index_dump_threads", 5, **kwargs)
        config.index_config.max_build_qps = _read_int_value("index_build_qps", 1000000, **kwargs)
        config.index_config.index_directory = _read_str_value("index_directory", "indices", **kwargs)
        # Meta Config
        config.meta_config.meta_uri = _read_str_value("meta_uri", ProximaSE.build_meta_file(), **kwargs)
        return config

    @staticmethod
    def build_config_file(config_file, **kwargs):
        config = ProximaSE.build_config(**kwargs)
        ProximaSEService.write_config_file(config_file, config)

    def stdout_log(self) -> str:
        return os.path.join(self.working_dir(), "proxima_se_stdout.log") if self.working_dir() else None

    def stderr_log(self) -> str:
        return os.path.join(self.working_dir(), "proxima_se_stderr.log") if self.working_dir() else None

    def start(self) -> bool:
        if self.running():
            logging.warning(f'{self.service_name()} have been started, please call stop first')
            return False

        logging.info(f'Start {self.service_name()} process')
        # Start ProximaSE as sub process
        self._process = ShellRunner.aync_execute([self._binary, '--config', self._config])
        self._stdout_fd = open(self.stdout_log(), 'w+b')
        self._stderr_fd = open(self.stderr_log(), 'w+b')

        # Redirect log of Mysql Repo
        self._log_monitor = LoggingMonitor(self._process, self)
        self._log_monitor.start()
        return True

    def stop(self) -> bool:
        code = ProximaSEBaseService.stop(self)
        if code and self._stderr_fd:
            self._stderr_fd.close()
            self._stderr_fd = None
        return code

    def on_stderr(self, msg):
        if self._stderr_fd and msg:
            self._stderr_fd.write(msg)

    def service_addr(self) -> str:
        config = config_pb2.ProximaSEConfig()
        with open(self._config, 'r') as config_fd:
            text = config_fd.read()
            Parse(text, config, allow_unknown_field=True)
        return f'127.0.0.1:{config.common_config.grpc_listen_port}'

    def stats(self) -> ProximaSEService.Stats:
        return ProximaSEBVarService.process_stat(self.service_addr())
