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
 *   \author   guonix
 *   \date     Nov 2020
 *   \brief
 */

#include <gtest/gtest.h>
#include "meta/meta_impl.h"

using namespace proxima::be::meta;

TEST(CollectionImpl, TestConstructor) {
  CollectionMeta meta;
  meta.set_name("name");
  meta.set_uid("uid");
  meta.mutable_forward_columns()->assign({"forward1", "forward2"});
  meta.set_max_docs_per_segment(10);
  meta.set_revision(10);
  meta.set_status(CollectionStatus::INITIALIZED);
  meta.set_current(false);

  CollectionImplPtr collection = std::make_shared<CollectionImpl>(meta);
  ASSERT_EQ(meta.name(), collection->name());
  ASSERT_EQ(meta.uid(), collection->uid());

  ASSERT_EQ("forward1,forward2", collection->forward_columns());
  ASSERT_EQ(meta.max_docs_per_segment(), collection->max_docs_per_segment());
  ASSERT_EQ(meta.revision(), collection->revision());
  ASSERT_EQ(meta.status(), static_cast<CollectionStatus>(collection->status()));
  ASSERT_EQ(meta.is_current(), collection->current() != 0);
}
