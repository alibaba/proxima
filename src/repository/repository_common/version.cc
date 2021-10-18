/**
 *   Copyright 2021 Alibaba, Inc. and its affiliates. All Rights Reserved.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.

 *   \author   Dianzhang.Chen
 *   \date     Apr 2021
 *   \brief    Mysql repository version information
 */

#include "version.h"
#include <ailego/version.i>

#ifdef mysql_repository_common_VERSION
#define MYSQL_REPOSITORY_VERSION_STRING mysql_repository_common_VERSION
#else
#define MYSQL_REPOSITORY_VERSION_STRING "unknown"
#endif

namespace proxima {
namespace be {
namespace repository {

static const char MYSQL_REPOSITORY_VERSION_DETAILS[] =
    AILEGO_VERSION_COMPILE_DETAILS(
        "Mysql Repository Version " MYSQL_REPOSITORY_VERSION_STRING
        ".\nCopyright (C) Alibaba, Inc. and its affiliates. All rights "
        "reserved.\n");

const char *Version::String(void) {
  return MYSQL_REPOSITORY_VERSION_STRING;
}

const char *Version::Details(void) {
  return MYSQL_REPOSITORY_VERSION_DETAILS;
}

}  // namespace repository
}  // namespace be
}  // namespace proxima