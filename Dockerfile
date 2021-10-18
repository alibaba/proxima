##  Copyright 2021 Alibaba, Inc. and its affiliates. All Rights Reserved.
##
##  Licensed under the Apache License, Version 2.0 (the "License");
##  you may not use this file except in compliance with the License.
##  You may obtain a copy of the License at
##
##      http://www.apache.org/licenses/LICENSE-2.0
##
##  Unless required by applicable law or agreed to in writing, software
##  distributed under the License is distributed on an "AS IS" BASIS,
##  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
##  See the License for the specific language governing permissions and
##  limitations under the License.

FROM debian:buster-20210721 AS builder

ARG BUILD_DIR=/src/build.docker
ARG SRC_DIR=/src
ARG BE_DIR=/var/lib/proxima-be
ARG CMAKE_ARG="-DCMAKE_BUILD_TYPE=Release -DENABLE_HASWELL=ON"

WORKDIR $SRC_DIR

RUN apt update && \
    apt install -y --no-install-recommends \
    git \
    ca-certificates \
    cmake \
    build-essential \
    zlib1g-dev \
    pkg-config

COPY . $SRC_DIR
RUN cd $SRC_DIR && git submodule update --init
RUN mkdir -p $BUILD_DIR && cmake -B $BUILD_DIR -DCMAKE_INSTALL_PREFIX:PATH=${BE_DIR} $CMAKE_ARG $SRC_DIR && cmake --build $BUILD_DIR --target install/strip -- -j


FROM debian:buster-20210721
WORKDIR /var/lib/proxima-be

COPY --from=builder /var/lib/proxima-be /var/lib/proxima-be
CMD [ "/var/lib/proxima-be/bin/proxima_be", "--config", "/var/lib/proxima-be/conf/proxima_be.conf" ]
