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
 *
 *   \author   Dianzhang.Chen
 *   \date     Oct 2020
 *   \brief    Interface of collection
 */

#pragma once

#include "common/macro_define.h"
#include "common_types.h"

namespace proxima {
namespace be {
namespace repository {

class Collection;
using CollectionPtr = std::shared_ptr<Collection>;

/*! Collection Interface
 */
class Collection {
 public:
  virtual ~Collection() = default;

 public:
  //! Initialize Collection
  virtual int init() = 0;

  //! Start collection
  virtual void run() = 0;

  //！ Stop collection
  virtual void stop() = 0;

  //! Update Collection
  virtual void update() = 0;

  //! Drop Collection
  virtual void drop() = 0;

  //! If collection is finished
  virtual bool finished() const = 0;

  //! Get collection state
  virtual CollectionStatus state() const = 0;

  //! Get collection schema revision
  virtual uint32_t schema_revision() const = 0;
};

}  // end namespace repository
}  // namespace be
}  // end namespace proxima
