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
 *   \date     Mar 2021
 *   \brief
 */

package be

import (
	"reflect"
	"testing"

	pb "proto"
)

func TestAcceptableFeatureType(t *testing.T) {
	if acceptableFeatureTypes(reflect.TypeOf(int(10))) {
		t.Error("int64 is not acceptable as features")
	}
}

func TestIsSlice(t *testing.T) {
	if !isSlice(reflect.ValueOf([]int{1, 2}).Type()) {
		t.Error("Check []int slice failed")
	}
	if !isSlice(reflect.ValueOf([]float32{1.0, 3.0}).Type()) {
		t.Error("Check []float32 slice failed")
	}
	if isSlice(reflect.ValueOf([3]float32{1.0, 3.0}).Type()) {
		t.Error("Check [3]float32 slice failed")
	}
}

func TestIsArray(t *testing.T) {
	if isArray(reflect.ValueOf([]int{1, 2}).Type()) {
		t.Error("Check []int is array failed")
	}
	if isArray(reflect.ValueOf([]float32{1.0, 3.0}).Type()) {
		t.Error("Check []float32 is array failed")
	}
	if !isArray(reflect.ValueOf([3]float32{1.0, 3.0}).Type()) {
		t.Error("Check [3]float32 is array failed")
	}
}

func TestIsSliceOrArray(t *testing.T) {
	if !isSliceOrArray(reflect.ValueOf([]int{1, 2}).Type()) {
		t.Error("Check []int is slice or array failed")
	}
	if !isSliceOrArray(reflect.ValueOf([]float32{1.0, 3.0}).Type()) {
		t.Error("Check []int is slice or array failed")
	}
	if !isSliceOrArray(reflect.ValueOf([3]float32{1.0, 3.0}).Type()) {
		t.Error("Check [3]float32 is slice or array failed")
	}
}

func TestInferVectorFeature(t *testing.T) {
	// supportted list
	kind, ok := inferVectorFeatureType([]int8{1, 2, 3})
	if !ok || kind != reflect.Int8 {
		t.Error("Can't infer feature type of int8 vector")
	}

	kind, ok = inferVectorFeatureType([]uint32{1, 2, 3})
	if !ok || kind != reflect.Uint32 {
		t.Error("Can't infer feature type of uint32 vector")
	}

	kind, ok = inferVectorFeatureType([]uint64{1, 2, 3})
	if !ok || kind != reflect.Uint64 {
		t.Error("Can't infer feature type of uint64 vector")
	}

	kind, ok = inferVectorFeatureType([]float32{1, 2, 3})
	if !ok || kind != reflect.Float32 {
		t.Error("Can't infer feature type of float32 vector")
	}

	// Not supportted
	if _, ok = inferVectorFeatureType([]int32{1, 2, 3}); ok {
		t.Error("infer feature type of int32 vector")
	}
	if _, ok = inferVectorFeatureType([]int64{1, 2, 3}); ok {
		t.Error("infer feature type of int64 vector")
	}
	if _, ok = inferVectorFeatureType([][]int64{{1, 2, 3}}); ok {
		t.Error("infer feature type of int64 metrix")
	}
	if _, ok = inferVectorFeatureType(10); ok {
		t.Error("infer feature type of int64 metrix")
	}
}

func TestInferMatrixFeatureType(t *testing.T) {
	kind, ok := inferMatrixFeatureType([][]int8{{1, 2, 3}})
	if !ok || kind != reflect.Int8 {
		t.Error("Can't infer feature type of int8 vector")
	}
	kind, ok = inferMatrixFeatureType([][]uint32{{1, 2, 3}})
	if !ok || kind != reflect.Uint32 {
		t.Error("Can't infer feature type of uint32 vector")
	}
	kind, ok = inferMatrixFeatureType([][]uint64{{1, 2, 3}})
	if !ok || kind != reflect.Uint64 {
		t.Error("Can't infer feature type of uint64 vector")
	}
	kind, ok = inferMatrixFeatureType([][]float32{{1.0, 2, 3}})
	if !ok || kind != reflect.Float32 {
		t.Error("Can't infer feature type of float32 vector")
	}

	if _, ok = inferMatrixFeatureType([][]int8{{}}); ok {
		t.Error("infer feature type of [][]int8{{}} vector")
	}
	if _, ok = inferMatrixFeatureType([][]int8{}); ok {
		t.Error("infer feature type of [][]int8{ vector")
	}
	if _, ok = inferMatrixFeatureType([][]int32{{1, 2, 3}}); ok {
		t.Error("infer feature type of [][]int32{{1, 2, 3}} vector")
	}
	if _, ok = inferMatrixFeatureType([][]int32{{}}); ok {
		t.Error("infer feature type of [][]int32{{}} vector")
	}
	if _, ok = inferMatrixFeatureType([][]int32{}); ok {
		t.Error("infer feature type of [][]int32{ vector")
	}
}

func TestInferMatrixDimension(t *testing.T) {
	batch, deminsion, err := inferMatrixDimension([][]float32{{1.0, 2, 3}})
	if err != nil || batch != 1 || deminsion != 3 {
		t.Error("Can't refer batch and dimension from [][]float32{{1.0, 2, 3}}")
	}
	batch, deminsion, err = inferMatrixDimension([][]int{{1, 2, 3}, {2, 3, 4}})
	if err != nil || batch != 2 || deminsion != 3 {
		t.Error("Can't refer batch and dimension from [][]float32{{1.0, 2, 3}}")
	}
	batch, deminsion, err = inferMatrixDimension([][]float32{{1.0, 2, 3}, {}})
	if err == nil {
		t.Error("Can't refer batch and dimension from [][]float32{{1.0, 2, 3}}")
	}
}

func TestInferFeatures(t *testing.T) {
	dt, deminsion, batch, err := inferFeatures([][]float32{{1.0, 2, 3}})
	if err != nil || dt != pb.DataType_DT_VECTOR_FP32 || deminsion != 3 || batch != 1 {
		t.Error(err)
		t.Error("can't infer features of [][]float32{{1.0, 2, 3}}")
	}
	dt, deminsion, batch, err = inferFeatures([][]float32{{1.0, 2, 3}, {2, 3, 4}})
	if err != nil || dt != pb.DataType_DT_VECTOR_FP32 || deminsion != 3 || batch != 2 {
		t.Error(err)
		t.Error("can't infer features of [][]float32{{1.0, 2, 3}, {2, 3, 4}}")
	}
	dt, deminsion, batch, err = inferFeatures([]float32{1.0, 2, 3, 2, 3, 4})
	if err != nil || dt != pb.DataType_DT_VECTOR_FP32 || deminsion != 6 || batch != 1 {
		t.Error(err)
		t.Error("can't infer features of []float32{1.0, 2, 3, 2, 3, 4}")
	}
}
