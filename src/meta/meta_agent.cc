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
 *   \date     Dec 2020
 *   \brief
 */

#include "meta_agent.h"
#include "common/config.h"
#include "common/error_code.h"
#include "common/logger.h"
#include "meta_service_builder.h"

namespace proxima {
namespace be {
namespace meta {

/*! MetaAgent implementation
 */
class MetaAgentImpl : public MetaAgent {
 public:
  //! Constructor
  explicit MetaAgentImpl(MetaServicePtr meta_service)
      : meta_service_(std::move(meta_service)) {}

  //! Destructor
  ~MetaAgentImpl() override = default;

 public:
  //! Get Meta Service Instance
  MetaServicePtr get_service() const override {
    return meta_service_;
  }

 public:
  //! Init Meta Agent
  int init() override {
    int error_code = meta_service_->init();
    if (error_code == 0) {
      LOG_INFO("MetaAgent initialize complete.");
    } else {
      LOG_ERROR("MetaAgent initialize failed. code[%d] what[%s]", error_code,
                ErrorCode::What(error_code));
    }
    return error_code;
  }

  //! Clean up object
  int cleanup() override {
    int error_code = meta_service_->cleanup();
    if (error_code == 0) {
      LOG_INFO("MetaAgent cleanup complete. ");
    } else {
      LOG_ERROR("MetaAgent cleanup failed. code[%d] what[%s]", error_code,
                ErrorCode::What(error_code));
    }
    return error_code;
  }

  //! Start background service
  int start() override {
    int error_code = meta_service_->start();
    if (error_code == 0) {
      LOG_INFO("MetaAgent start complete.");
    } else {
      LOG_ERROR("MetaAgent start failed. code[%d] what[%s]", error_code,
                ErrorCode::What(error_code));
    }
    return error_code;
  }

  //! Stop background service
  int stop() override {
    int error_code = meta_service_->stop();
    if (error_code == 0) {
      LOG_INFO("MetaAgent stopped.");
    } else {
      LOG_ERROR("MetaAgent stop failed. code[%d] what[%s]", error_code,
                ErrorCode::What(error_code));
    }
    return error_code;
  }

 public:
  //! Reload meta service
  int reload() override {
    int error_code = meta_service_->reload();
    if (error_code == 0) {
      LOG_INFO("MetaAgent reloaded.");
    } else {
      LOG_ERROR("MetaAgent reload failed. code[%d] what[%s]", error_code,
                ErrorCode::What(error_code));
    }
    return error_code;
  }

  //! Create collection
  int create_collection(const CollectionBase &param,
                        CollectionMetaPtr *meta) override {
    int code = meta_service_->create_collection(param, meta);
    if (code != 0) {
      LOG_ERROR("Failed to create collection: code[%d], what[%s],", code,
                ErrorCode::What(code));
    }
    return code;
  }

  //! Update collection
  int update_collection(const CollectionBase &param,
                        CollectionMetaPtr *meta) override {
    int code = meta_service_->update_collection(param, meta);
    if (code != 0) {
      LOG_ERROR("Failed to update collection: code[%d], what[%s],", code,
                ErrorCode::What(code));
    }
    return code;
  }

  //! Update the status of current used collection
  int update_status(const std::string &collection_name,
                    CollectionStatus status) override {
    int code = meta_service_->update_status(collection_name, status);
    if (code != 0) {
      LOG_ERROR("Failed to update collection: code[%d], what[%s],", code,
                ErrorCode::What(code));
    }
    return code;
  }

  //! Enable collection
  int enable_collection(const std::string &collection,
                        uint32_t revision) override {
    int code = meta_service_->enable_collection(collection, revision, true);
    if (code != 0) {
      LOG_ERROR("Failed to enable collection. code[%d] what[%s]", code,
                ErrorCode::What(code));
    }
    return code;
  }

  //! Suspend reading requests of collection
  int suspend_collection_read(const std::string &collection_name) override {
    return meta_service_->suspend_collection_read(collection_name);
  }

  //! Resume reading requests of collection
  int resume_collection_read(const std::string &collection_name) override {
    return meta_service_->resume_collection_read(collection_name);
  }

  //! Suspend writing requests of collection
  int suspend_collection_write(const std::string &collection_name) override {
    return meta_service_->suspend_collection_write(collection_name);
  }

  //! Resume writing requests of collection
  int resume_collection_write(const std::string &collection_name) override {
    return meta_service_->resume_collection_write(collection_name);
  }

  //! Delete collection
  int delete_collection(const std::string &collection) override {
    if (collection.empty()) {
      LOG_ERROR("Collection name can't be empty");
      return PROXIMA_BE_ERROR_CODE(InvalidArgument);
    }
    int code = meta_service_->drop_collection(collection);
    if (code != 0) {
      LOG_ERROR("Drop collection failed: code[%d], what[%s],", code,
                ErrorCode::What(code));
    }
    return code;
  }

  //! Retrieve collections
  int list_collections(CollectionMetaPtrList *collections) const override {
    if (!collections) {
      return PROXIMA_BE_ERROR_CODE(InvalidArgument);
    }
    int code = meta_service_->get_latest_collections(collections);
    if (code != 0) {
      LOG_ERROR("Failed to list collections, code[%d], what[%s].", code,
                ErrorCode::What(code));
    }
    return code;
  }

  //! Retrieve collections
  int get_collection_history(
      const std::string &name,
      CollectionMetaPtrList *collections) const override {
    if (name.empty()) {
      LOG_ERROR("Collection name can't be empty");
      return PROXIMA_BE_ERROR_CODE(InvalidArgument);
    }
    int code = meta_service_->get_collections(name, collections);
    if (code != 0) {
      LOG_ERROR("Failed to list collections, code[%d], what[%s].", code,
                ErrorCode::What(code));
    }
    return code;
  }

  //! Check collection exists
  CollectionMetaPtr get_collection(const std::string &name) const override {
    if (name.empty()) {
      LOG_ERROR("Collection name can't be empty");
      return nullptr;
    }
    return meta_service_->get_current_collection(name);
  }

  //! Check collection exists
  bool exist_collection(const std::string &name) const override {
    if (name.empty()) {
      LOG_ERROR("Collection name can't be empty");
      return false;
    }
    return meta_service_->exist_collection(name);
  }

 private:
  MetaServicePtr meta_service_{nullptr};
};

//! Create one MetaAgent instance
MetaAgentPtr MetaAgent::Create(const std::string &uri) {
  return MetaAgent::Create(MetaServiceBuilder::Create(uri));
}

MetaAgentPtr MetaAgent::Create(const MetaServicePtr &meta_service) {
  if (meta_service) {
    return std::make_shared<MetaAgentImpl>(meta_service);
  }
  LOG_ERROR("Failed to create MetaService, invalid arguments of meta_service");
  return nullptr;
}

}  // namespace meta
}  // namespace be
}  // namespace proxima
