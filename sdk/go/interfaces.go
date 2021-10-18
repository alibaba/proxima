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

// ProximaSearchClient is the client API for Proxima BE service.
//
// For semantics around ctx use and closing/ending streaming RPCs, please refer to https://godoc.org/google.golang.org/grpc#ClientConn.NewStream.
type ProximaSearchClient interface {
	// CreateCollection method to create one collection
	//
	// Return error not nil means error, otherwise succeed
	// Example:
	//
	CreateCollection(config *CollectionConfig) error

	// DropCollection for delete collection
	//
	// return error is nil for success, otherwise failed
	DropCollection(name string) error

	// DescribeCollection method used to retrieve details of collection
	// Argument @name indicate interested collection
	// Return pointer of CollectionInfo, error not nil for failed
	DescribeCollection(name string) (*CollectionInfo, error)

	// ListCollections method, used to retrieve collections from ProximaSE
	// Available filters listed as following:
	//   ByRepo(repo string): only list collection which name of repository equals repo.
	//                        last one task effect, if multiple instance passed
	// Return pointer of CollectionInfo array, error not nil for failed
	ListCollections(filters ...ListCollectionFilter) ([]*CollectionInfo, error)

	// StatCollection used for retrieve stat of collection
	// Return pointer of CollectionStat, error not nil for failed
	StatCollection(name string) (*CollectionStat, error)

	// Write Collection
	// Return Status object, error not nil for failed
	Write(in *WriteRequest) error

	// Perform Query Request to Proxima BE
	// Return pointer of QueryResponse, error not nil for failed
	// Note: acceptable features listed below
	//     // slice/array/matrix of [int8, uint32, uint64, float32]
	//     features := []int8{1,2,3,4}
	//     features := [5]int8{1,2,3,4}
	//     features := [][]int8{{1,2,3,4},{1,2,3,4}}
	//     features := [5][5]int8{{1,2,3,4},{1,2,3,4}}
	//     features := []uint32{1,2,3,4}
	//     features := [5]uint32{1,2,3,4}
	//     features := [][]uint32{{1,2,3,4},{1,2,3,4}}
	//     features := [5][5]uint32{{1,2,3,4},{1,2,3,4}}
	//     features := []float32{1,2,3,4}
	//     features := [5]float32{1,2,3,4}
	//     features := [][]float32{{1,2,3,4},{1,2,3,4}}
	//     features := [5][5]float32{{1,2,3,4},{1,2,3,4}}
	// opts parameter is interface of QueryOptions, possible value listed below:
	//   WithTopK(int): customize topk
	//   WithRadius(float32): customize search radius
	//   WithLinearSearch(): enable linear search
	//   WithDebugMode(): enable debug mode
	//   WithParam(string, interface{}): for advance configuration
	// example:
	//     client.Query("collection", "column", []int{1,2}, WithTopK(10), WithRadius(0.6), WithLinearSearch(), WithDebugMode(), WithParam("key", 1))
	Query(collection string, column string, features interface{}, opts ...QueryOption) (*QueryResponse, error)

	// GetDocumentByKey retrieve document which primary key was indicated by param req
	// Return pointer of Document, error not nil for failed
	GetDocumentByKey(collection string, primaryKey uint64) (*Document, error)
}
