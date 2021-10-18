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
# Shell command runner
#

import os
import subprocess
import time
import select
from .exceptions import FileNotExits


class ShellRunner(object):
    @staticmethod
    def read_output(fd):
        out = fd.read()
        if len(out) == 0:
            out = None
        return out

    @staticmethod
    def readable(fd):
        res = select.select([fd], [], [], 1)
        if res == ([fd], [], []):
            return True
        return False

    @staticmethod
    def aync_execute(args):
        return subprocess.Popen(args,
                                stdin=subprocess.PIPE,
                                stdout=subprocess.PIPE,
                                stderr=subprocess.PIPE,
                                shell=False)

    @staticmethod
    def execute2(args):
        p = subprocess.Popen(args,
                             stdin=subprocess.PIPE,
                             stdout=subprocess.PIPE,
                             stderr=subprocess.PIPE,
                             shell=False)
        str = ""
        for line in p.stdout:
            str += line.decode("utf-8")
        p.wait()
        return p.returncode, str

    @staticmethod
    def execute(args, timeout=10):
        st = int(time.time())
        out = ''
        err = ''
        p = subprocess.Popen(args,
                             stdin=subprocess.PIPE,
                             stdout=subprocess.PIPE,
                             stderr=subprocess.PIPE,
                             shell=False)

        code = 0
        ct = int(time.time())
        while ct - st < timeout:
            p.poll()
            if p.returncode:
                code = p.returncode
                break

            if ShellRunner.readable(p.stdout):
                out += p.stdout.read()
            if ShellRunner.readable(p.stderr):
                err += p.stderr.read()

            time.sleep(0.2)
            ct = int(time.time())
        if ct - st >= timeout and not p.returncode:
            code = 0
            p.terminate()

        if ShellRunner.readable(p.stdout):
            out += p.stdout.read()
        if ShellRunner.readable(p.stderr):
            err += p.stderr.read()

        return code, out, err


class ShellCommand(object):
    def __init__(self, cmd):
        if not os.path.isfile(cmd):
            raise FileNotExits(cmd)
        self.cmd = cmd

    def execute(self, *args):
        return ShellRunner.execute([self.cmd] + args)

    def execute2(self, *args):
        return ShellRunner.execute2([self.cmd] + args)
