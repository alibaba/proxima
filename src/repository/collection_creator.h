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
 *   \date     Oct 2020
 *   \brief    Interface of collection creator
 */

#pragma once

#include <memory>
#include "common/macro_define.h"
#include "collection.h"

namespace proxima {
namespace be {
namespace repository {

class CollectionCreator;
using CollectionCreatorPtr = std::shared_ptr<CollectionCreator>;

/*! Collection Creator
 */
class CollectionCreator {
 public:
  //! Destructor
  virtual ~CollectionCreator() = default;

  //! Create collection
  virtual CollectionPtr create(const CollectionInfo &info) const = 0;
};

/*! Collection Creator Implication
 */
class CollectionCreatorImpl : public CollectionCreator {
 public:
  //! Constructor
  CollectionCreatorImpl() = default;

  //! Destructor
  ~CollectionCreatorImpl() = default;

  //! Create collection
  CollectionPtr create(const CollectionInfo &info) const override;
};

}  // end namespace repository
}  // namespace be
}  // end namespace proxima
