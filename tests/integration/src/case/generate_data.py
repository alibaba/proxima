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

import struct

def generate_features_str(start, dimension, count):
    i = 0
    features = []
    while(i < count):
        current_feature = "["
        j = 0
        while(j < dimension - 1):
            current_feature = current_feature + str(start) + ","
            j = j+1
        current_feature = current_feature + str(start) + "]"
        features.append(current_feature)
        start += 1
        i = i+1
    return features

def generate_features_bytes(start, dimension, count):
    i = 0
    features = []
    while(i < count):
        j = 0
        current_feature = []
        while(j < dimension - 1):
            current_feature.append(start)
            j = j+1
        current_feature.append(start)
        fea_bytes = []
        for f in current_feature:
            fea_bytes += struct.pack('f', f)
        features_bytes = bytes(fea_bytes)
        features.append(features_bytes)
        start += 1
        i = i+1
    return features

def generate_delete_primary_keys(start, count):
    i = 0
    keys = []
    while(i < count):
        keys.append(start)
        start += 1
        i = i+1
    return keys


if __name__ == "__main__":
    pass
