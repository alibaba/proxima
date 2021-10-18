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
	"fmt"

	pb "github.com/alibaba/proximabilin/sdk/go/proto"
)

// ListCollectionFilter for ListCollections method
type ListCollectionFilter interface {
	apply(*pb.ListCondition)
}

type listCollectionFilter struct {
	filter func(*pb.ListCondition)
}

func (opt *listCollectionFilter) apply(opts *pb.ListCondition) {
	opt.filter(opts)
}

func newlistCollectionFilter(filter func(*pb.ListCondition)) *listCollectionFilter {
	return &listCollectionFilter{
		filter: filter,
	}
}

// ByRepo list collections by repo
func ByRepo(repo string) ListCollectionFilter {
	return newlistCollectionFilter(func(opts *pb.ListCondition) {
		opts.RepositoryName = repo
	})
}

// queryOptions advance configuration for customize query
type queryOptions struct {
	// topk, default is 100
	topk uint32
	// Search radius
	radius float32
	// Linear search
	linear bool
	// Debug mode
	debug bool
	// Advanced params
	params map[string]string
}

// QueryOption for customize query params
type QueryOption interface {
	apply(*queryOptions) bool
}

type funcOption struct {
	f func(*queryOptions) bool
}

func (opt *funcOption) apply(opts *queryOptions) bool {
	return opt.f(opts)
}

func defaultOptions() *queryOptions {
	return &queryOptions{
		topk:   100,
		radius: 0.5,
		linear: false,
		debug:  false,
		params: map[string]string{},
	}
}

func newQueryOption(f func(opts *queryOptions) bool) *funcOption {
	return &funcOption{
		f: f,
	}
}

// WithTopK customize topk param, default is 100
func WithTopK(topk uint32) QueryOption {
	return newQueryOption(func(opts *queryOptions) bool {
		if topk < 10000 {
			opts.topk = topk
			return true
		}
		return false
	})
}

// WithRadius customize radius param, default is 0.5
func WithRadius(radius float32) QueryOption {
	return newQueryOption(func(opts *queryOptions) bool {
		if radius >= 0.0 {
			opts.radius = radius
			return true
		}
		return false
	})
}

// WithLinearSearch enable linear search mode
func WithLinearSearch() QueryOption {
	return newQueryOption(func(opts *queryOptions) bool {
		opts.linear = true
		return true
	})
}

// WithDebugMode enable debug search mode, consequence bigger latench,
// does not recommend in product environments
func WithDebugMode() QueryOption {
	return newQueryOption(func(opts *queryOptions) bool {
		opts.debug = true
		return true
	})
}

// WithParam customize param,
// Acceptable value type listed below
// types of value: [[]byte, string, bool, int32, int64, uint32, uint64, float32, float64]
// Examples:
//     int32opt := WithParam("key", int32(10))
//     int64opt := WithParam("key", 10)
func WithParam(key string, value interface{}) QueryOption {
	return newQueryOption(func(opts *queryOptions) bool {
		if acceptableValueTypes(value) {
			opts.params[key] = fmt.Sprint(value)
			return true
		}
		return false
	})
}
