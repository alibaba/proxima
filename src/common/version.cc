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

 *   \author   Haichao.chc
 *   \date     Apr 2021
 *   \brief    Proxima BE version information
 */

#include "version.h"
#include <ailego/version.i>

#ifdef proxima_be_common_VERSION
#define PROXIMA_BE_VERSION_STRING proxima_be_common_VERSION
#else
#define PROXIMA_BE_VERSION_STRING "unknown"
#endif

namespace proxima {
namespace be {

static const char PROXIMA_BE_VERSION_DETAILS[] = AILEGO_VERSION_COMPILE_DETAILS(
    "Proxima BE Version " PROXIMA_BE_VERSION_STRING
    ".\nCopyright (C) Alibaba, Inc. and its affiliate. All rights reserved.\n");

const char *Version::String() {
  return PROXIMA_BE_VERSION_STRING;
}

const char *Version::Details(void) {
  return PROXIMA_BE_VERSION_DETAILS;
}

}  // namespace be
}  // end namespace proxima
