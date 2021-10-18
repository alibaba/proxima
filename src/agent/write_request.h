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

 *   \author   Hongqing.hu
 *   \date     Feb 2021
 *   \brief    Write request interface definition for proxima search engine
 */

#pragma once

#include "index/collection_dataset.h"

namespace proxima {
namespace be {
namespace agent {

class WriteRequest;
using WriteRequestPtr = std::shared_ptr<WriteRequest>;

/*! Write Request
 */
class WriteRequest {
 public:
  //! Request type
  enum class RequestType : uint32_t { PROXY = 0, DIRECT = 1 };

  //! Constructor
  WriteRequest() = default;

  //! Destructor
  ~WriteRequest() = default;

 public:
  //! Set request type
  void set_request_type(RequestType type) {
    request_type_ = type;
  }

  //! Get request type
  RequestType request_type() const {
    return request_type_;
  }

  // Is proxy request
  bool is_proxy_request() const {
    return request_type_ == RequestType::PROXY;
  }

  //! Set magic number
  void set_magic_number(uint64_t number) {
    magic_number_ = number;
  }

  //! Get magic number
  uint64_t magic_number() const {
    return magic_number_;
  }

  //! Set collection name
  void set_collection_name(const std::string &collection) {
    collection_name_ = collection;
  }

  //! Get collection name
  const std::string &collection_name() const {
    return collection_name_;
  }

  //! Add collection dataset
  void add_collection_dataset(index::CollectionDatasetPtr dataset) {
    row_count_ += dataset->size();
    records_.emplace_back(std::move(dataset));
  }

  //! Get collection dataset
  const index::CollectionDatasetPtr &get_collection_dataset(
      size_t index) const {
    return records_[index];
  }

  //! Get all collection dataset
  const std::vector<index::CollectionDatasetPtr> &get_collection_dataset()
      const {
    return records_;
  }

  //! Get collection dataset count
  size_t collection_dataset_count() const {
    return records_.size();
  }

  //! Get rows count
  size_t row_count() const {
    return row_count_;
  }

 private:
  //! Members
  RequestType request_type_{RequestType::PROXY};
  size_t row_count_{0};
  uint64_t magic_number_{0};
  std::string collection_name_{};
  std::vector<index::CollectionDatasetPtr> records_{};
};

}  // end namespace agent
}  // namespace be
}  // end namespace proxima
