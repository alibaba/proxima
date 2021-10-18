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

 *   \author   Hechong.xyf
 *   \date     Oct 2019
 *   \brief    Interface of AiTheta Index Holder
 */

#ifndef __AITHETA2_INDEX_HOLDER_H__
#define __AITHETA2_INDEX_HOLDER_H__

#include <list>
#include <memory>
#include <utility>
#include <vector>
#include <ailego/container/vector.h>
#include <ailego/utility/float_helper.h>
#include "index_meta.h"
#include "index_params.h"

namespace aitheta2 {

/*! Index Holder
 */
struct IndexHolder {
  //! Index Holder Pointer
  typedef std::shared_ptr<IndexHolder> Pointer;

  /*! Index Holder Iterator
   */
  struct Iterator {
    //! Index Holder Iterator Pointer
    typedef std::unique_ptr<Iterator> Pointer;

    //! Destructor
    virtual ~Iterator(void) {}

    //! Retrieve pointer of data
    virtual const void *data(void) const = 0;

    //! Test if the iterator is valid
    virtual bool is_valid(void) const = 0;

    //! Retrieve primary key
    virtual uint64_t key(void) const = 0;

    //! Next iterator
    virtual void next(void) = 0;
  };

  //! Destructor
  virtual ~IndexHolder(void) {}

  //! Retrieve count of elements in holder (-1 indicates unknown)
  virtual size_t count(void) const = 0;

  //! Retrieve dimension
  virtual size_t dimension(void) const = 0;

  //! Retrieve type information
  virtual IndexMeta::FeatureTypes type(void) const = 0;

  //! Retrieve element size in bytes
  virtual size_t element_size(void) const = 0;

  //! Retrieve if it can multi-pass
  virtual bool multipass(void) const = 0;

  //! Create a new iterator
  virtual Iterator::Pointer create_iterator(void) = 0;

  //! Test if matchs the meta
  bool is_matched(const IndexMeta &meta) const {
    return (this->type() == meta.type() &&
            this->dimension() == meta.dimension() &&
            this->element_size() == meta.element_size());
  }
};

/*! One-Pass Numerical Index Holder
 */
template <typename T>
class OnePassNumericalIndexHolder : public IndexHolder {
 public:
  /*! One-Pass Index Holder Iterator
   */
  class Iterator : public IndexHolder::Iterator {
   public:
    //! Index Holder Iterator Pointer
    typedef std::unique_ptr<Iterator> Pointer;

    //! Constructor
    Iterator(OnePassNumericalIndexHolder *owner) : holder_(owner) {
      features_iter_ = holder_->features_.begin();
    }

    //! Destructor
    virtual ~Iterator(void) {}

    //! Retrieve pointer of data
    const void *data(void) const override {
      return features_iter_->second.data();
    }

    //! Test if the iterator is valid
    bool is_valid(void) const override {
      return (features_iter_ != holder_->features_.end());
    }

    //! Retrieve primary key
    uint64_t key(void) const override {
      return features_iter_->first;
    }

    //! Next iterator
    void next(void) override {
      holder_->features_.erase(features_iter_++);
    }

   private:
    OnePassNumericalIndexHolder *holder_{nullptr};
    typename std::list<std::pair<uint64_t, ailego::NumericalVector<T>>>::
        iterator features_iter_{};
  };

  //! Constructor
  OnePassNumericalIndexHolder(size_t dim) : dimension_(dim) {}

  //! Retrieve count of elements in holder (-1 indicates unknown)
  size_t count(void) const override {
    return features_.size();
  }

  //! Retrieve dimension
  size_t dimension(void) const override {
    return dimension_;
  }

  //! Retrieve type information
  IndexMeta::FeatureTypes type(void) const override {
    return IndexMeta::FT_UNDEFINED;
  }

  //! Retrieve element size in bytes
  size_t element_size(void) const override {
    return dimension_ * sizeof(T);
  }

  //! Retrieve if it can multi-pass
  bool multipass(void) const override {
    return false;
  }

  //! Create a new iterator
  IndexHolder::Iterator::Pointer create_iterator(void) override {
    return IndexHolder::Iterator::Pointer(
        new OnePassNumericalIndexHolder::Iterator(this));
  }

  //! Append an element into holder
  bool emplace(uint64_t key, const ailego::NumericalVector<T> &vec) {
    if (vec.size() != dimension_) {
      return false;
    }
    features_.emplace_back(key, vec);
    return true;
  }

  //! Append an element into holder
  bool emplace(uint64_t key, ailego::NumericalVector<T> &&vec) {
    if (vec.size() != dimension_) {
      return false;
    }
    features_.emplace_back(key, std::move(vec));
    return true;
  }

 private:
  //! Disable them
  OnePassNumericalIndexHolder(void) = delete;

  //! Members
  size_t dimension_{0};
  std::list<std::pair<uint64_t, ailego::NumericalVector<T>>> features_;
};

/*! Multi-Pass Numerical Index Holder
 */
template <typename T>
class MultiPassNumericalIndexHolder : public IndexHolder {
 public:
  /*! Multi-Pass Index Holder Iterator
   */
  class Iterator : public IndexHolder::Iterator {
   public:
    //! Index Holder Iterator Pointer
    typedef std::unique_ptr<Iterator> Pointer;

    //! Constructor
    Iterator(MultiPassNumericalIndexHolder *owner) : holder_(owner) {
      features_iter_ = holder_->features_.begin();
    }

    //! Destructor
    virtual ~Iterator(void) {}

    //! Retrieve pointer of data
    const void *data(void) const override {
      return features_iter_->second.data();
    }

    //! Test if the iterator is valid
    bool is_valid(void) const override {
      return (features_iter_ != holder_->features_.end());
    }

    //! Retrieve primary key
    uint64_t key(void) const override {
      return features_iter_->first;
    }

    //! Next iterator
    void next(void) override {
      ++features_iter_;
    }

   private:
    MultiPassNumericalIndexHolder *holder_{nullptr};
    typename std::vector<std::pair<uint64_t, ailego::NumericalVector<T>>>::
        iterator features_iter_{};
  };

  //! Constructor
  MultiPassNumericalIndexHolder(size_t dim) : dimension_(dim) {}

  //! Retrieve count of elements in holder (-1 indicates unknown)
  size_t count(void) const override {
    return features_.size();
  }

  //! Retrieve dimension
  size_t dimension(void) const override {
    return dimension_;
  }

  //! Retrieve type information
  IndexMeta::FeatureTypes type(void) const override {
    return IndexMeta::FT_UNDEFINED;
  }

  //! Retrieve element size in bytes
  size_t element_size(void) const override {
    return dimension_ * sizeof(T);
  }

  //! Retrieve if it can multi-pass
  bool multipass(void) const override {
    return true;
  }

  //! Create a new iterator
  IndexHolder::Iterator::Pointer create_iterator(void) override {
    return IndexHolder::Iterator::Pointer(
        new MultiPassNumericalIndexHolder::Iterator(this));
  }

  //! Append an element into holder
  bool emplace(uint64_t key, const ailego::NumericalVector<T> &vec) {
    if (vec.size() != dimension_) {
      return false;
    }
    features_.emplace_back(key, vec);
    return true;
  }

  //! Append an element into holder
  bool emplace(uint64_t key, ailego::NumericalVector<T> &&vec) {
    if (vec.size() != dimension_) {
      return false;
    }
    features_.emplace_back(key, std::move(vec));
    return true;
  }

  //! Request a change in capacity
  void reserve(size_t size) {
    features_.reserve(size);
  }

 private:
  //! Disable them
  MultiPassNumericalIndexHolder(void) = delete;

  //! Members
  size_t dimension_{0};
  std::vector<std::pair<uint64_t, ailego::NumericalVector<T>>> features_;
};

/*! One-Pass Binary Index Holder
 */
template <typename T>
class OnePassBinaryIndexHolder : public IndexHolder {
 public:
  /*! One-Pass Index Holder Iterator
   */
  class Iterator : public IndexHolder::Iterator {
   public:
    //! Index Holder Iterator Pointer
    typedef std::unique_ptr<Iterator> Pointer;

    //! Constructor
    Iterator(OnePassBinaryIndexHolder *owner) : holder_(owner) {
      features_iter_ = holder_->features_.begin();
    }

    //! Destructor
    virtual ~Iterator(void) {}

    //! Retrieve pointer of data
    const void *data(void) const override {
      return features_iter_->second.data();
    }

    //! Test if the iterator is valid
    bool is_valid(void) const override {
      return (features_iter_ != holder_->features_.end());
    }

    //! Retrieve primary key
    uint64_t key(void) const override {
      return features_iter_->first;
    }

    //! Next iterator
    void next(void) override {
      holder_->features_.erase(features_iter_++);
    }

   private:
    OnePassBinaryIndexHolder *holder_{nullptr};
    typename std::list<std::pair<uint64_t, ailego::BinaryVector<T>>>::iterator
        features_iter_{};
  };

  //! Constructor
  OnePassBinaryIndexHolder(size_t dim) : dimension_(dim) {}

  //! Retrieve count of elements in holder (-1 indicates unknown)
  size_t count(void) const override {
    return features_.size();
  }

  //! Retrieve dimension
  size_t dimension(void) const override {
    return dimension_;
  }

  //! Retrieve type information
  IndexMeta::FeatureTypes type(void) const override {
    return IndexMeta::FT_UNDEFINED;
  }

  //! Retrieve element size in bytes
  size_t element_size(void) const override {
    return (dimension_ + (sizeof(T) << 3) - 1) / (sizeof(T) << 3) * sizeof(T);
  }

  //! Retrieve if it can multi-pass
  bool multipass(void) const override {
    return false;
  }

  //! Create a new iterator
  IndexHolder::Iterator::Pointer create_iterator(void) override {
    return IndexHolder::Iterator::Pointer(
        new OnePassBinaryIndexHolder::Iterator(this));
  }

  //! Append an element into holder
  bool emplace(uint64_t key, const ailego::BinaryVector<T> &vec) {
    if (vec.size() != dimension_) {
      return false;
    }
    features_.emplace_back(key, vec);
    return true;
  }

  //! Append an element into holder
  bool emplace(uint64_t key, ailego::BinaryVector<T> &&vec) {
    if (vec.size() != dimension_) {
      return false;
    }
    features_.emplace_back(key, std::move(vec));
    return true;
  }

 private:
  //! Disable them
  OnePassBinaryIndexHolder(void) = delete;

  //! Members
  size_t dimension_{0};
  std::list<std::pair<uint64_t, ailego::BinaryVector<T>>> features_;
};

/*! Multi-Pass Binary Index Holder
 */
template <typename T>
class MultiPassBinaryIndexHolder : public IndexHolder {
 public:
  /*! Multi-Pass Index Holder Iterator
   */
  class Iterator : public IndexHolder::Iterator {
   public:
    //! Index Holder Iterator Pointer
    typedef std::unique_ptr<Iterator> Pointer;

    //! Constructor
    Iterator(MultiPassBinaryIndexHolder *owner) : holder_(owner) {
      features_iter_ = holder_->features_.begin();
    }

    //! Destructor
    virtual ~Iterator(void) {}

    //! Retrieve pointer of data
    const void *data(void) const override {
      return features_iter_->second.data();
    }

    //! Test if the iterator is valid
    bool is_valid(void) const override {
      return (features_iter_ != holder_->features_.end());
    }

    //! Retrieve primary key
    uint64_t key(void) const override {
      return features_iter_->first;
    }

    //! Next iterator
    void next(void) override {
      ++features_iter_;
    }

   private:
    MultiPassBinaryIndexHolder *holder_{nullptr};
    typename std::vector<std::pair<uint64_t, ailego::BinaryVector<T>>>::iterator
        features_iter_{};
  };

  //! Constructor
  MultiPassBinaryIndexHolder(size_t dim) : dimension_(dim) {}

  //! Retrieve count of elements in holder (-1 indicates unknown)
  size_t count(void) const override {
    return features_.size();
  }

  //! Retrieve dimension
  size_t dimension(void) const override {
    return dimension_;
  }

  //! Retrieve type information
  IndexMeta::FeatureTypes type(void) const override {
    return IndexMeta::FT_UNDEFINED;
  }

  //! Retrieve element size in bytes
  size_t element_size(void) const override {
    return (dimension_ + (sizeof(T) << 3) - 1) / (sizeof(T) << 3) * sizeof(T);
  }

  //! Retrieve if it can multi-pass
  bool multipass(void) const override {
    return true;
  }

  //! Create a new iterator
  IndexHolder::Iterator::Pointer create_iterator(void) override {
    return IndexHolder::Iterator::Pointer(
        new MultiPassBinaryIndexHolder::Iterator(this));
  }

  //! Append an element into holder
  bool emplace(uint64_t key, const ailego::BinaryVector<T> &vec) {
    if (vec.size() != dimension_) {
      return false;
    }
    features_.emplace_back(key, vec);
    return true;
  }

  //! Append an element into holder
  bool emplace(uint64_t key, ailego::BinaryVector<T> &&vec) {
    if (vec.size() != dimension_) {
      return false;
    }
    features_.emplace_back(key, std::move(vec));
    return true;
  }

  //! Request a change in capacity
  void reserve(size_t size) {
    features_.reserve(size);
  }

 private:
  //! Disable them
  MultiPassBinaryIndexHolder(void) = delete;

  //! Members
  size_t dimension_{0};
  std::vector<std::pair<uint64_t, ailego::BinaryVector<T>>> features_;
};

/*! One-Pass Index Holder
 */
template <IndexMeta::FeatureTypes FT>
struct OnePassIndexHolder;

/*! One-Pass Index Holder (BINARY32)
 */
template <>
struct OnePassIndexHolder<IndexMeta::FT_BINARY32>
    : public OnePassBinaryIndexHolder<uint32_t> {
  //! Constructor
  using OnePassBinaryIndexHolder::OnePassBinaryIndexHolder;

  //! Retrieve type information
  IndexMeta::FeatureTypes type(void) const override {
    return IndexMeta::FT_BINARY32;
  }
};

/*! One-Pass Index Holder (BINARY64)
 */
template <>
struct OnePassIndexHolder<IndexMeta::FT_BINARY64>
    : public OnePassBinaryIndexHolder<uint64_t> {
  //! Constructor
  using OnePassBinaryIndexHolder::OnePassBinaryIndexHolder;

  //! Retrieve type information
  IndexMeta::FeatureTypes type(void) const override {
    return IndexMeta::FT_BINARY64;
  }
};

/*! One-Pass Index Holder (FP16)
 */
template <>
struct OnePassIndexHolder<IndexMeta::FT_FP16>
    : public OnePassNumericalIndexHolder<ailego::Float16> {
  //! Constructor
  using OnePassNumericalIndexHolder::OnePassNumericalIndexHolder;

  //! Retrieve type information
  IndexMeta::FeatureTypes type(void) const override {
    return IndexMeta::FT_FP16;
  }
};

/*! One-Pass Index Holder (FP32)
 */
template <>
struct OnePassIndexHolder<IndexMeta::FT_FP32>
    : public OnePassNumericalIndexHolder<float> {
  //! Constructor
  using OnePassNumericalIndexHolder::OnePassNumericalIndexHolder;

  //! Retrieve type information
  IndexMeta::FeatureTypes type(void) const override {
    return IndexMeta::FT_FP32;
  }
};

/*! One-Pass Index Holder (FP64)
 */
template <>
struct OnePassIndexHolder<IndexMeta::FT_FP64>
    : public OnePassNumericalIndexHolder<double> {
  //! Constructor
  using OnePassNumericalIndexHolder::OnePassNumericalIndexHolder;

  //! Retrieve type information
  IndexMeta::FeatureTypes type(void) const override {
    return IndexMeta::FT_FP64;
  }
};

/*! One-Pass Index Holder (INT8)
 */
template <>
struct OnePassIndexHolder<IndexMeta::FT_INT8>
    : public OnePassNumericalIndexHolder<int8_t> {
  //! Constructor
  using OnePassNumericalIndexHolder::OnePassNumericalIndexHolder;

  //! Retrieve type information
  IndexMeta::FeatureTypes type(void) const override {
    return IndexMeta::FT_INT8;
  }
};

/*! One-Pass Index Holder (INT16)
 */
template <>
struct OnePassIndexHolder<IndexMeta::FT_INT16>
    : public OnePassNumericalIndexHolder<int16_t> {
  //! Constructor
  using OnePassNumericalIndexHolder::OnePassNumericalIndexHolder;

  //! Retrieve type information
  IndexMeta::FeatureTypes type(void) const override {
    return IndexMeta::FT_INT16;
  }
};

/*! Multi-Pass Index Holder
 */
template <IndexMeta::FeatureTypes FT>
struct MultiPassIndexHolder;

/*! Multi-Pass Index Holder (BINARY32)
 */
template <>
struct MultiPassIndexHolder<IndexMeta::FT_BINARY32>
    : public MultiPassBinaryIndexHolder<uint32_t> {
  //! Constructor
  using MultiPassBinaryIndexHolder::MultiPassBinaryIndexHolder;

  //! Retrieve type information
  IndexMeta::FeatureTypes type(void) const override {
    return IndexMeta::FT_BINARY32;
  }
};

/*! Multi-Pass Index Holder (BINARY64)
 */
template <>
struct MultiPassIndexHolder<IndexMeta::FT_BINARY64>
    : public MultiPassBinaryIndexHolder<uint64_t> {
  //! Constructor
  using MultiPassBinaryIndexHolder::MultiPassBinaryIndexHolder;

  //! Retrieve type information
  IndexMeta::FeatureTypes type(void) const override {
    return IndexMeta::FT_BINARY64;
  }
};

/*! Multi-Pass Index Holder (FP16)
 */
template <>
struct MultiPassIndexHolder<IndexMeta::FT_FP16>
    : public MultiPassNumericalIndexHolder<ailego::Float16> {
  //! Constructor
  using MultiPassNumericalIndexHolder::MultiPassNumericalIndexHolder;

  //! Retrieve type information
  IndexMeta::FeatureTypes type(void) const override {
    return IndexMeta::FT_FP16;
  }
};

/*! Multi-Pass Index Holder (FP32)
 */
template <>
struct MultiPassIndexHolder<IndexMeta::FT_FP32>
    : public MultiPassNumericalIndexHolder<float> {
  //! Constructor
  using MultiPassNumericalIndexHolder::MultiPassNumericalIndexHolder;

  //! Retrieve type information
  IndexMeta::FeatureTypes type(void) const override {
    return IndexMeta::FT_FP32;
  }
};

/*! Multi-Pass Index Holder (FP64)
 */
template <>
struct MultiPassIndexHolder<IndexMeta::FT_FP64>
    : public MultiPassNumericalIndexHolder<double> {
  //! Constructor
  using MultiPassNumericalIndexHolder::MultiPassNumericalIndexHolder;

  //! Retrieve type information
  IndexMeta::FeatureTypes type(void) const override {
    return IndexMeta::FT_FP64;
  }
};

/*! Multi-Pass Index Holder (INT8)
 */
template <>
struct MultiPassIndexHolder<IndexMeta::FT_INT8>
    : public MultiPassNumericalIndexHolder<int8_t> {
  //! Constructor
  using MultiPassNumericalIndexHolder::MultiPassNumericalIndexHolder;

  //! Retrieve type information
  IndexMeta::FeatureTypes type(void) const override {
    return IndexMeta::FT_INT8;
  }
};

/*! Multi-Pass Index Holder (INT16)
 */
template <>
struct MultiPassIndexHolder<IndexMeta::FT_INT16>
    : public MultiPassNumericalIndexHolder<int16_t> {
  //! Constructor
  using MultiPassNumericalIndexHolder::MultiPassNumericalIndexHolder;

  //! Retrieve type information
  IndexMeta::FeatureTypes type(void) const override {
    return IndexMeta::FT_INT16;
  }
};

}  // namespace aitheta2

#endif  // __AITHETA2_INDEX_HOLDER_H__
