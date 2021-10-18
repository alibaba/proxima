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
 *   \date     Oct 2020
 *   \brief    Vecs file reader
 */

#pragma once

#include <iostream>
#include <ailego/io/mmap_file.h>
#include <aitheta2/index_holder.h>
#include <aitheta2/index_meta.h>

namespace proxima {
namespace be {
namespace tools {

#pragma pack(4)
struct VecsHeader {
  uint64_t num_vecs;
  uint32_t meta_size;
  uint8_t meta_buf[0];
};
#pragma pack()

class VecsReader {
 public:
  VecsReader()
      : mmap_file_(),
        index_meta_(),
        num_vecs_(0),
        vector_base_(nullptr),
        key_base_(nullptr) {}

  void set_measure(const std::string &name,
                   const aitheta2::IndexParams &params) {
    index_meta_.set_measure(name, 0, params);
  }

  bool load(const std::string &fname) {
    return load(fname.c_str());
  }

  bool load(const char *fname) {
    if (!fname) {
      std::cerr << "Load fname is nullptr" << std::endl;
      return false;
    }
    if (!mmap_file_.open(fname, true)) {
      std::cerr << "Open file error: " << fname << std::endl;
      return false;
    }
    if (mmap_file_.size() < sizeof(VecsHeader)) {
      std::cerr << "File size is too small: " << mmap_file_.size() << std::endl;
      return false;
    }

    const VecsHeader *header =
        reinterpret_cast<const VecsHeader *>(mmap_file_.region());
    // check
    num_vecs_ = header->num_vecs;
    if ((mmap_file_.size() - sizeof(*header) - header->meta_size) % num_vecs_ !=
        0) {
      std::cerr << "input file foramt check error." << std::endl;
      return false;
    }
    // deserialize
    bool bret = index_meta_.deserialize(&header->meta_buf, header->meta_size);
    if (!bret) {
      std::cerr << "deserialize index meta error." << std::endl;
      return false;
    }
    vector_base_ =
        reinterpret_cast<const char *>(header + 1) + header->meta_size;
    key_base_ = reinterpret_cast<const uint64_t *>(
        vector_base_ + num_vecs_ * index_meta_.element_size());
    return true;
  }

  template <typename T>
  bool write_vecs_output(const aitheta2::IndexMeta &meta,
                         const std::vector<uint64_t> &keys,
                         const std::vector<std::vector<T>> &features) {
    if (keys.empty()) {
      std::cerr << "keys is empty." << std::endl;
      return false;
    }

    if (keys.size() != features.size()) {
      std::cerr << "keys's size(" << keys.size()
                << ") is not equal to features's size(" << features.size()
                << ")." << std::endl;
      return false;
    }

    FILE *wfp = fopen("my.vecs", "wb");
    if (!wfp) {
      std::cerr << "Open file my.vecs error. " << std::endl;
      return false;
    }

    // write header
    VecsHeader header;
    header.num_vecs = keys.size();
    std::string meta_buf;
    meta.serialize(&meta_buf);
    header.meta_size = meta_buf.size();
    size_t wret = fwrite(&header, sizeof(header), 1, wfp);
    if (wret != 1) {
      std::cerr << "Write header error" << std::endl;
      fclose(wfp);
      return false;
    }

    wret = fwrite(meta_buf.c_str(), meta_buf.size(), 1, wfp);
    if (wret != 1) {
      std::cerr << "Write header meta_buf error" << std::endl;
      fclose(wfp);
      return false;
    }
    size_t nx = keys.size();
    for (size_t i = 0; i < nx; ++i) {
      auto &feature = features[i];
      wret = fwrite(&feature[0], sizeof(T), feature.size(), wfp);
      if (wret != feature.size()) {
        std::cerr << "Write feature error. " << std::endl;
        fclose(wfp);
        return false;
      }
    }

    for (size_t i = 0; i < nx; ++i) {
      uint64_t key = keys[i];
      wret = fwrite(&key, sizeof(key), 1, wfp);
      if (wret != 1) {
        std::cerr << "Write key error. key:" << key << std::endl;
        fclose(wfp);
        return false;
      }
    }

    fclose(wfp);
    return true;
  }

  size_t num_vecs() const {
    return num_vecs_;
  }

  const void *vector_base() const {
    return vector_base_;
  }

  const uint64_t *key_base() const {
    return key_base_;
  }

  const aitheta2::IndexMeta &index_meta() const {
    return index_meta_;
  }

  uint64_t get_key(size_t index) const {
    return key_base_[index];
  }

  const void *get_vector(size_t index) const {
    return vector_base_ + index * index_meta_.element_size();
  }

 private:
  ailego::MMapFile mmap_file_;
  aitheta2::IndexMeta index_meta_;
  size_t num_vecs_{0U};
  const char *vector_base_{nullptr};
  const uint64_t *key_base_{nullptr};
};


}  // end namespace tools
}  // end namespace be
}  // end namespace proxima
