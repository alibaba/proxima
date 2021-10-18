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
 *   \date     Mar 2018
 *   \brief    Interface of AiTheta Index Meta
 */

#ifndef __AITHETA2_INDEX_META_H__
#define __AITHETA2_INDEX_META_H__

#include "index_params.h"

namespace aitheta2 {

/*! Index Meta
 */
class IndexMeta {
 public:
  /*! Feature Types
   */
  enum FeatureTypes {
    FT_UNDEFINED = 0,
    FT_BINARY32 = 1,
    FT_BINARY64 = 2,
    FT_FP16 = 3,
    FT_FP32 = 4,
    FT_FP64 = 5,
    FT_INT8 = 6,
    FT_INT16 = 7,
    FT_INT4 = 8,
  };

  /*! Major Orders
   */
  enum MajorOrders {
    MO_UNDEFINED = 0,
    MO_ROW = 1,
    MO_COLUMN = 2,
  };

  //! Constructor
  IndexMeta(void) {
    this->set_meta(FeatureTypes::FT_FP32, 128u);
    this->set_measure("SquaredEuclidean", 0, IndexParams());
  }

  //! Constructor
  IndexMeta(FeatureTypes tp, uint32_t dim) {
    this->set_meta(tp, dim);
    this->set_measure("SquaredEuclidean", 0, IndexParams());
  }

  //! Constructor
  IndexMeta(const IndexMeta &rhs)
      : major_order_(rhs.major_order_),
        type_(rhs.type_),
        dimension_(rhs.dimension_),
        unit_size_(rhs.unit_size_),
        element_size_(rhs.element_size_),
        space_id_(rhs.space_id_),
        measure_revision_(rhs.measure_revision_),
        converter_revision_(rhs.converter_revision_),
        reformer_revision_(rhs.reformer_revision_),
        trainer_revision_(rhs.trainer_revision_),
        builder_revision_(rhs.builder_revision_),
        reducer_revision_(rhs.reducer_revision_),
        searcher_revision_(rhs.searcher_revision_),
        streamer_revision_(rhs.streamer_revision_),
        measure_name_(rhs.measure_name_),
        converter_name_(rhs.converter_name_),
        reformer_name_(rhs.reformer_name_),
        trainer_name_(rhs.trainer_name_),
        builder_name_(rhs.builder_name_),
        reducer_name_(rhs.reducer_name_),
        searcher_name_(rhs.searcher_name_),
        streamer_name_(rhs.streamer_name_),
        measure_params_(rhs.measure_params_),
        converter_params_(rhs.converter_params_),
        reformer_params_(rhs.reformer_params_),
        trainer_params_(rhs.trainer_params_),
        builder_params_(rhs.builder_params_),
        reducer_params_(rhs.reducer_params_),
        searcher_params_(rhs.searcher_params_),
        streamer_params_(rhs.streamer_params_),
        attributes_(rhs.attributes_) {}

  //! Constructor
  IndexMeta(IndexMeta &&rhs)
      : major_order_(rhs.major_order_),
        type_(rhs.type_),
        dimension_(rhs.dimension_),
        unit_size_(rhs.unit_size_),
        element_size_(rhs.element_size_),
        space_id_(rhs.space_id_),
        measure_revision_(rhs.measure_revision_),
        converter_revision_(rhs.converter_revision_),
        reformer_revision_(rhs.reformer_revision_),
        trainer_revision_(rhs.trainer_revision_),
        builder_revision_(rhs.builder_revision_),
        reducer_revision_(rhs.reducer_revision_),
        searcher_revision_(rhs.searcher_revision_),
        streamer_revision_(rhs.streamer_revision_),
        measure_name_(std::move(rhs.measure_name_)),
        converter_name_(std::move(rhs.converter_name_)),
        reformer_name_(std::move(rhs.reformer_name_)),
        trainer_name_(std::move(rhs.trainer_name_)),
        builder_name_(std::move(rhs.builder_name_)),
        reducer_name_(std::move(rhs.reducer_name_)),
        searcher_name_(std::move(rhs.searcher_name_)),
        streamer_name_(std::move(rhs.streamer_name_)),
        measure_params_(std::move(rhs.measure_params_)),
        converter_params_(std::move(rhs.converter_params_)),
        reformer_params_(std::move(rhs.reformer_params_)),
        trainer_params_(std::move(rhs.trainer_params_)),
        builder_params_(std::move(rhs.builder_params_)),
        reducer_params_(std::move(rhs.reducer_params_)),
        searcher_params_(std::move(rhs.searcher_params_)),
        streamer_params_(std::move(rhs.streamer_params_)),
        attributes_(std::move(rhs.attributes_)) {}

  //! Assignment
  IndexMeta &operator=(const IndexMeta &rhs) {
    major_order_ = rhs.major_order_;
    type_ = rhs.type_;
    dimension_ = rhs.dimension_;
    unit_size_ = rhs.unit_size_;
    element_size_ = rhs.element_size_;
    measure_revision_ = rhs.measure_revision_;
    converter_revision_ = rhs.converter_revision_;
    reformer_revision_ = rhs.reformer_revision_;
    trainer_revision_ = rhs.trainer_revision_;
    builder_revision_ = rhs.builder_revision_;
    reducer_revision_ = rhs.reducer_revision_;
    searcher_revision_ = rhs.searcher_revision_;
    streamer_revision_ = rhs.streamer_revision_;
    measure_name_ = rhs.measure_name_;
    converter_name_ = rhs.converter_name_;
    reformer_name_ = rhs.reformer_name_;
    trainer_name_ = rhs.trainer_name_;
    builder_name_ = rhs.builder_name_;
    reducer_name_ = rhs.reducer_name_;
    searcher_name_ = rhs.searcher_name_;
    streamer_name_ = rhs.streamer_name_;
    measure_params_ = rhs.measure_params_;
    converter_params_ = rhs.converter_params_;
    reformer_params_ = rhs.reformer_params_;
    trainer_params_ = rhs.trainer_params_;
    builder_params_ = rhs.builder_params_;
    reducer_params_ = rhs.reducer_params_;
    searcher_params_ = rhs.searcher_params_;
    streamer_params_ = rhs.streamer_params_;
    attributes_ = rhs.attributes_;
    return *this;
  }

  //! Assignment
  IndexMeta &operator=(IndexMeta &&rhs) {
    major_order_ = rhs.major_order_;
    type_ = rhs.type_;
    dimension_ = rhs.dimension_;
    unit_size_ = rhs.unit_size_;
    element_size_ = rhs.element_size_;
    space_id_ = rhs.space_id_;
    measure_revision_ = rhs.measure_revision_;
    converter_revision_ = rhs.converter_revision_;
    reformer_revision_ = rhs.reformer_revision_;
    trainer_revision_ = rhs.trainer_revision_;
    builder_revision_ = rhs.builder_revision_;
    reducer_revision_ = rhs.reducer_revision_;
    searcher_revision_ = rhs.searcher_revision_;
    streamer_revision_ = rhs.streamer_revision_;
    measure_name_ = std::move(rhs.measure_name_);
    converter_name_ = std::move(rhs.converter_name_);
    reformer_name_ = std::move(rhs.reformer_name_);
    trainer_name_ = std::move(rhs.trainer_name_);
    builder_name_ = std::move(rhs.builder_name_);
    reducer_name_ = std::move(rhs.reducer_name_);
    searcher_name_ = std::move(rhs.searcher_name_);
    streamer_name_ = std::move(rhs.streamer_name_);
    measure_params_ = std::move(rhs.measure_params_);
    converter_params_ = std::move(rhs.converter_params_);
    reformer_params_ = std::move(rhs.reformer_params_);
    trainer_params_ = std::move(rhs.trainer_params_);
    builder_params_ = std::move(rhs.builder_params_);
    reducer_params_ = std::move(rhs.reducer_params_);
    searcher_params_ = std::move(rhs.searcher_params_);
    streamer_params_ = std::move(rhs.streamer_params_);
    attributes_ = std::move(rhs.attributes_);
    return *this;
  }

  //! Reset the meta
  void clear(void) {
    major_order_ = MajorOrders::MO_UNDEFINED;
    type_ = FeatureTypes::FT_UNDEFINED;
    dimension_ = 0;
    unit_size_ = 0;
    element_size_ = 0;
    space_id_ = 0;
    measure_revision_ = 0;
    converter_revision_ = 0;
    reformer_revision_ = 0;
    trainer_revision_ = 0;
    builder_revision_ = 0;
    reducer_revision_ = 0;
    searcher_revision_ = 0;
    streamer_revision_ = 0;
    measure_name_.clear();
    converter_name_.clear();
    reformer_name_.clear();
    trainer_name_.clear();
    builder_name_.clear();
    reducer_name_.clear();
    searcher_name_.clear();
    streamer_name_.clear();
    measure_params_.clear();
    converter_params_.clear();
    reformer_params_.clear();
    trainer_params_.clear();
    builder_params_.clear();
    reducer_params_.clear();
    searcher_params_.clear();
    streamer_params_.clear();
    attributes_.clear();
  }

  //! Retrieve major order information
  MajorOrders major_order(void) const {
    return major_order_;
  }

  //! Retrieve type information
  FeatureTypes type(void) const {
    return type_;
  }

  //! Retrieve dimension
  uint32_t dimension(void) const {
    return dimension_;
  }

  //! Retrieve unit size in bytes
  uint32_t unit_size(void) const {
    return unit_size_;
  }

  //! Retrieve element size in bytes
  uint32_t element_size(void) const {
    return element_size_;
  }

  //! Retrieve space id
  uint64_t space_id(void) const {
    return space_id_;
  }

  //! Retrieve revision of measure
  uint32_t measure_revision(void) const {
    return measure_revision_;
  }

  //! Retrieve revision of converter
  uint32_t converter_revision(void) const {
    return converter_revision_;
  }

  //! Retrieve revision of reformer
  uint32_t reformer_revision(void) const {
    return reformer_revision_;
  }

  //! Retrieve revision of trainer
  uint32_t trainer_revision(void) const {
    return trainer_revision_;
  }

  //! Retrieve revision of builder
  uint32_t builder_revision(void) const {
    return builder_revision_;
  }

  //! Retrieve revision of searcher
  uint32_t searcher_revision(void) const {
    return searcher_revision_;
  }

  //! Retrieve revision of reducer
  uint32_t reducer_revision(void) const {
    return reducer_revision_;
  }

  //! Retrieve revision of streamer
  uint32_t streamer_revision(void) const {
    return streamer_revision_;
  }

  //! Retrieve name of measure
  const std::string &measure_name(void) const {
    return measure_name_;
  }

  //! Retrieve name of converter
  const std::string &converter_name(void) const {
    return converter_name_;
  }

  //! Retrieve name of reformer
  const std::string &reformer_name(void) const {
    return reformer_name_;
  }

  //! Retrieve name of trainer
  const std::string &trainer_name(void) const {
    return trainer_name_;
  }

  //! Retrieve name of builder
  const std::string &builder_name(void) const {
    return builder_name_;
  }

  //! Retrieve name of reducer
  const std::string &reducer_name(void) const {
    return reducer_name_;
  }

  //! Retrieve name of searcher
  const std::string &searcher_name(void) const {
    return searcher_name_;
  }

  //! Retrieve name of streamer
  const std::string &streamer_name(void) const {
    return streamer_name_;
  }

  //! Retrieve measure params
  const IndexParams &measure_params(void) const {
    return measure_params_;
  }

  //! Retrieve converter params
  const IndexParams &converter_params(void) const {
    return converter_params_;
  }

  //! Retrieve reformer params
  const IndexParams &reformer_params(void) const {
    return reformer_params_;
  }

  //! Retrieve trainer params
  const IndexParams &trainer_params(void) const {
    return trainer_params_;
  }

  //! Retrieve builder params
  const IndexParams &builder_params(void) const {
    return builder_params_;
  }

  //! Retrieve reducer params
  const IndexParams &reducer_params(void) const {
    return reducer_params_;
  }

  //! Retrieve searcher params
  const IndexParams &searcher_params(void) const {
    return searcher_params_;
  }

  //! Retrieve streamer params
  const IndexParams &streamer_params(void) const {
    return streamer_params_;
  }

  //! Retrieve attributes
  const IndexParams &attributes(void) const {
    return attributes_;
  }

  //! Retrieve mutable attributes
  IndexParams *mutable_attributes(void) {
    return &attributes_;
  }

  //! Set major order of features
  void set_major_order(MajorOrders order) {
    major_order_ = order;
  }

  //! Set dimension of feature
  void set_dimension(uint32_t dim) {
    dimension_ = dim;
    element_size_ = IndexMeta::ElementSizeof(type_, unit_size_, dim);
  }

  //! Set meta information of feature
  void set_meta(FeatureTypes tp, uint32_t unit, uint32_t dim) {
    type_ = tp;
    dimension_ = dim;
    unit_size_ = unit;
    element_size_ = ElementSizeof(tp, unit, dim);
  }

  //! Set meta information of feature
  void set_meta(FeatureTypes tp, uint32_t dim) {
    this->set_meta(tp, UnitSizeof(tp), dim);
  }

  //! Set space id of index
  void set_space_id(uint64_t val) {
    space_id_ = val;
  }

  //! Set information of measure
  template <typename TName, typename TParams>
  void set_measure(TName &&name, uint32_t rev, TParams &&params) {
    measure_name_ = std::forward<TName>(name);
    measure_revision_ = rev;
    measure_params_ = std::forward<TParams>(params);
  }

  //! Set information of converter
  template <typename TName, typename TParams>
  void set_converter(TName &&name, uint32_t rev, TParams &&params) {
    converter_name_ = std::forward<TName>(name);
    converter_revision_ = rev;
    converter_params_ = std::forward<TParams>(params);
  }

  //! Set information of reformer
  template <typename TName, typename TParams>
  void set_reformer(TName &&name, uint32_t rev, TParams &&params) {
    reformer_name_ = std::forward<TName>(name);
    reformer_revision_ = rev;
    reformer_params_ = std::forward<TParams>(params);
  }

  //! Set information of trainer
  template <typename TName, typename TParams>
  void set_trainer(TName &&name, uint32_t rev, TParams &&params) {
    trainer_name_ = std::forward<TName>(name);
    trainer_revision_ = rev;
    trainer_params_ = std::forward<TParams>(params);
  }

  //! Set information of builder
  template <typename TName, typename TParams>
  void set_builder(TName &&name, uint32_t rev, TParams &&params) {
    builder_name_ = std::forward<TName>(name);
    builder_revision_ = rev;
    builder_params_ = std::forward<TParams>(params);
  }

  //! Set information of reducer
  template <typename TName, typename TParams>
  void set_reducer(TName &&name, uint32_t rev, TParams &&params) {
    reducer_name_ = std::forward<TName>(name);
    reducer_revision_ = rev;
    reducer_params_ = std::forward<TParams>(params);
  }

  //! Set information of searcher
  template <typename TName, typename TParams>
  void set_searcher(TName &&name, uint32_t rev, TParams &&params) {
    searcher_name_ = std::forward<TName>(name);
    searcher_revision_ = rev;
    searcher_params_ = std::forward<TParams>(params);
  }

  //! Set information of streamer
  template <typename TName, typename TParams>
  void set_streamer(TName &&name, uint32_t rev, TParams &&params) {
    streamer_name_ = std::forward<TName>(name);
    streamer_revision_ = rev;
    streamer_params_ = std::forward<TParams>(params);
  }

  //! Serialize meta information into buffer
  void serialize(std::string *out) const;

  //! Derialize meta information from buffer
  bool deserialize(const void *data, size_t len);

  //! Retrieve debug information
  std::string debug_string(void) const;

  //! Calculate unit size of feature
  static uint32_t UnitSizeof(FeatureTypes ft) {
    static const uint32_t unit_size_table[] = {
        0u,                // FT_UNDEFINED
        sizeof(uint32_t),  // FT_BINARY32
        sizeof(uint64_t),  // FT_BINARY64
        sizeof(uint16_t),  // FT_FP16
        sizeof(float),     // FT_FP32
        sizeof(double),    // FT_FP64
        sizeof(int8_t),    // FT_INT8
        sizeof(int16_t),   // FT_INT16
        sizeof(uint8_t)    // FT_INT4
    };
    return unit_size_table[ft];
  }

  //! Calculate align size of feature
  static uint32_t AlignSizeof(FeatureTypes ft) {
    static const uint32_t align_size_table[] = {
        0u,                  // FT_UNDEFINED
        sizeof(uint32_t),    // FT_BINARY32
        sizeof(uint64_t),    // FT_BINARY64
        sizeof(uint16_t),    // FT_FP16
        sizeof(float),       // FT_FP32
        sizeof(double),      // FT_FP64
        sizeof(int8_t) * 4,  // FT_INT8
        sizeof(int16_t),     // FT_INT16
        sizeof(uint8_t) * 4  // FT_INT4
    };
    return align_size_table[ft];
  }

  //! Calculate element size of feature
  static uint32_t ElementSizeof(FeatureTypes ft, uint32_t unit, uint32_t dim) {
    switch (ft) {
      case FeatureTypes::FT_UNDEFINED:
        return 0;
      case FeatureTypes::FT_BINARY32:
      case FeatureTypes::FT_BINARY64:
        return (dim + unit * 8 - 1) / (unit * 8) * unit;
      case FeatureTypes::FT_FP16:
      case FeatureTypes::FT_FP32:
      case FeatureTypes::FT_FP64:
      case FeatureTypes::FT_INT8:
      case FeatureTypes::FT_INT16:
        return (dim * unit);
      case FeatureTypes::FT_INT4:
        return (dim + unit * 2 - 1) / (unit * 2) * unit;
    }
    return 0;
  }

  //! Calculate element size of feature
  static uint32_t ElementSizeof(FeatureTypes ft, uint32_t dim) {
    return ElementSizeof(ft, UnitSizeof(ft), dim);
  }

 private:
  MajorOrders major_order_{MajorOrders::MO_UNDEFINED};
  FeatureTypes type_{FeatureTypes::FT_UNDEFINED};
  uint32_t dimension_{0};
  uint32_t unit_size_{0};
  uint32_t element_size_{0};
  uint64_t space_id_{0};
  uint32_t measure_revision_{0};
  uint32_t converter_revision_{0};
  uint32_t reformer_revision_{0};
  uint32_t trainer_revision_{0};
  uint32_t builder_revision_{0};
  uint32_t reducer_revision_{0};
  uint32_t searcher_revision_{0};
  uint32_t streamer_revision_{0};
  std::string measure_name_{};
  std::string converter_name_{};
  std::string reformer_name_{};
  std::string trainer_name_{};
  std::string builder_name_{};
  std::string reducer_name_{};
  std::string searcher_name_{};
  std::string streamer_name_{};
  IndexParams measure_params_{};
  IndexParams converter_params_{};
  IndexParams reformer_params_{};
  IndexParams trainer_params_{};
  IndexParams builder_params_{};
  IndexParams reducer_params_{};
  IndexParams searcher_params_{};
  IndexParams streamer_params_{};
  IndexParams attributes_{};
};

/*! Index Query Meta
 */
class IndexQueryMeta {
 public:
  //! Constructor
  IndexQueryMeta(void) {}

  //! Constructor
  IndexQueryMeta(IndexMeta::FeatureTypes ft, uint32_t unit, uint32_t dim)
      : type_(ft),
        dimension_(dim),
        unit_size_(unit),
        element_size_(IndexMeta::ElementSizeof(ft, unit, dim)) {}

  //! Constructor
  IndexQueryMeta(IndexMeta::FeatureTypes ft, uint32_t dim)
      : IndexQueryMeta{ft, IndexMeta::UnitSizeof(ft), dim} {}

  //! Retrieve type of features
  IndexMeta::FeatureTypes type(void) const {
    return type_;
  }

  //! Retrieve dimension of features
  uint32_t dimension(void) const {
    return dimension_;
  }

  //! Retrieve unit size of feature
  uint32_t unit_size(void) const {
    return unit_size_;
  }

  //! Retrieve element size of feature
  uint32_t element_size(void) const {
    return element_size_;
  }

  //! Set dimension of feature
  void set_dimension(uint32_t dim) {
    dimension_ = dim;
    element_size_ = IndexMeta::ElementSizeof(type_, unit_size_, dim);
  }

  //! Set meta information of feature
  void set_meta(IndexMeta::FeatureTypes tp, uint32_t unit, uint32_t dim) {
    type_ = tp;
    dimension_ = dim;
    unit_size_ = unit;
    element_size_ = IndexMeta::ElementSizeof(tp, unit, dim);
  }

  //! Set meta information of feature
  void set_meta(IndexMeta::FeatureTypes tp, uint32_t dim) {
    this->set_meta(tp, IndexMeta::UnitSizeof(tp), dim);
  }

 private:
  IndexMeta::FeatureTypes type_{IndexMeta::FT_UNDEFINED};
  uint32_t dimension_{0};
  uint32_t unit_size_{0};
  uint32_t element_size_{0};
};

}  // namespace aitheta2

#endif  // __AITHETA2_INDEX_META_H__
