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

// ConnectionProtocol types
type ConnectionProtocol uint32

// Available types of connection
const (
	// Using GRPC to communicate with ProximaSE, which is default
	GrpcProtocol ConnectionProtocol = iota
	// Using HTTP to communicate with ProximaSE
	HttpProtocol
)

// MetricType indicates the method of distance between vector
type MetricType uint32

// Available metric types
const (
	// Undefined method, this is not illegal argument for column configuration
	MetricTypeUndefined MetricType = iota
	// Squared Euclidean
	SquaredEuclidean
	// Euclidean
	Euclidean
	// Manhattan
	Manhattan
	// Inner Product
	InnerProduct
	// Hamming
	Hamming
)

// IndexType is types of index
type IndexType uint32

// Available index types
const (
	// Undefined, which is not illegal argument for column configuration
	IndexTypeUndefined IndexType = iota
	// Proxima HNSW Index, Default value
	ProximaGraphIndex
)

// DataType is types of data
type DataType uint32

// Available data types
const (
	// Undefined type
	DataTypeUndefined DataType = iota
	// Binary type
	Binary DataType = 1
	// String type
	String DataType = 2
	// Boolean type
	Bool DataType = 3
	// Signed integer with 4 bytes
	Int32 DataType = 4
	// Signed integer with 8 bytes
	Int64 DataType = 5
	// Unsigned integer with 4 bytes
	Uint32 DataType = 6
	// Unsigned integer with 8 bytes
	Uint64 DataType = 7
	// Single precision float
	Float DataType = 8
	// Double precision float
	Double DataType = 9
	// Binary with 4 bytes array
	// Memory layout:  [item1 item2 item3]
	// Offset          0 ... 4 ... 8 .. 12
	VectorBinary32 DataType = 20
	// Binary with 8 bytes array
	// Memory layout:  [item1 item2 item3]
	// Offset          0 ... 8 .. 16 .. 24
	VectorBinary64 DataType = 21
	// Float with 2 bytes array,
	// Tips: Create index with fp16, the query should be float32 if no float16 type in language built-in types
	VectorFP16 DataType = 22
	// Single precision float with 4 bytes array
	VectorFP32 DataType = 23
	// Double precision float with 8 bytes array
	VectorFP64 DataType = 24
	// Signed integer with 4 bits array
	VectorInt4 DataType = 25
	// Signed integer with 8 bits array
	VectorInt8 DataType = 26
	// Signed integer with 16 bits array
	VectorInt16 DataType = 27
)

// OperationType operation type
type OperationType uint32

// Available operations
const (
	// Insert Operation
	Insert OperationType = iota
	// Update Operation
	Update
	// Delete Operation
	Delete
)

// ErrorCode type of Code
type ErrorCode int32

// Code list of client only
const (
	Success ErrorCode = 0
	// ProximaSE has been overload, please retry later
	RetryLater ErrorCode = -4009
	// Unknown error, which equals to MaxProximaSEErrorCode(refers to ProximaSE manual book)
	Unknown ErrorCode = -1000000
	// Incompatible error, current client can't work with ProximaSE
	// (please check web side of ProximaSE for release notes)
	Incompatible ErrorCode = -1000001
)

// RepositoryType type of repository
type RepositoryType uint32

// Repository list
const (
	// Database repository
	Database RepositoryType = iota
)

// CollectionStatus collection Status type
type CollectionStatus uint32

// Available status of collection
const (
	// Collection has been initialized, is ready for serving
	Initialized CollectionStatus = iota
	// Collection is serving
	Serving
	// Collection has been dropped permanently
	Dropped
)

// SegmentState state of segment
type SegmentState uint32

// Available states of segment
const (
	// Segment has been created
	Created SegmentState = iota
	// Writing operation on Segment
	Writing
	// Segment is dumping
	Dumping
	// Compacting operation on segment
	Compacting
	// Persistent Segment
	Persist
)
