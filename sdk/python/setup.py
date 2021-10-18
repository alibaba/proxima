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

import pathlib
import setuptools
import subprocess

SE_DIR = pathlib.Path(__file__).parent.absolute().parent.parent
SDK_PATH = f'{SE_DIR}/sdk/python/pyproximabe'

# Generate protocol libs for python
generate_cmd = f'python3 -m grpc_tools.protoc --proto_path={SE_DIR}/src/ --grpc_python_out={SDK_PATH}' \
               f' --python_out={SDK_PATH} {SE_DIR}/src/proto/*.proto'

subprocess.check_output(generate_cmd, shell=True)

setuptools.setup(
    name="pyproximabe",
    description="Python Sdk for Proxima BE",
    setup_requires=['setuptools_scm'],
    use_scm_version={
        "root": "../..",
        "relative_to": __file__,
        'write_to': 'sdk/python/pyproximabe/version.py',
        'write_to_template': '__version__ = "{version}"',
        'version_scheme' : 'post-release',
    },
    url='https://github.com/alibaba/proximabilin/',
    license="inner",
    packages=setuptools.find_packages(),
    include_package_data=True,
    install_requires=["grpcio>=1.22.0", "grpcio-tools>=1.22.0", "requests>=2.20.0", "orjson>=3.4.0"],
    classifiers=[
        "Programming Language :: Python :: 3.6",
        "Programming Language :: Python :: 3.7",
        "Programming Language :: Python :: 3.8",
    ],

    python_requires='>=3.6'
)
