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
	"encoding/json"
	"errors"
	"fmt"
	"reflect"
	"strings"
)

// Version object
type Version struct {
	// Version of client
	Client string `json:"client:omitempty"`
	// Version of ProximaSE
	Server string `json:"server:omitempty"`
}

// Compatible check compatibility between client and server
func (ver *Version) Compatible() bool {
	return strings.HasPrefix(ver.Server, ver.Client)
}

// String return json formatted string of Version object
func (version *Version) String() string {
	if bytes, err := json.Marshal(version); err == nil {
		return string(bytes)
	}
	return "{}"
}

// Address reference to entrypoint of ProximaSE
type Address struct {
	// Host IP address or host name
	Host string `json:"host:omitempty"`
	// Port
	Port uint32 `json:"port"`
}

// String return json formatted string of Address object
func (address *Address) String() string {
	if bytes, err := json.Marshal(address); err == nil {
		return string(bytes)
	}
	return "{}"
}

func (addr *Address) address() string {
	return fmt.Sprintf("%s:%d", addr.Host, addr.Port)
}

var (
	// InvalidIndex used for GenericValueList
	InvalidIndex int = -1
)

// ColumnIndex Description of column
type ColumnIndex struct {
	// Name of column, empty Name is illegal argument
	Name string `json:"name,omitempty"`
	// Type of column, Default is IT_PROXIMA_HNSW_INDEX
	IndexType `json:"index_type"`
	// Datatype of column
	DataType `json:"data_type"`
	// Dimension of column, Default is 0
	Dimension uint32 `json:"dimension"`
	// Customized parameters for column, Optional
	ExtraParams map[string]string `json:"extra_params,omitempty"`
}

// String return json format string of ColumnIndex
func (column *ColumnIndex) String() string {
	if bytes, err := json.Marshal(column); err == nil {
		return string(bytes)
	}
	return "{}"
}

// Repository Base Description of Repository, drove class specific one type of repository
type Repository struct {
	// Name of repository, empty is illegal argument
	Name string `json:"name,omitempty"`
	// Type of repository, Optional field, Default is RT_DATABASE
	Type RepositoryType `json:"type"`
}

// DatabaseRepository database repository
// Indicate where the data of collection coming from, Proxima BE synchronize Collection
// with Database automatically if create collection with Repository.
type DatabaseRepository struct {
	// Derived from Repository class
	Repository
	// URI of database, invalid uri will be deferred by Proxima BE,
	// example: mysql://localhost:3033/example_database
	// Details:
	//   Schema: indicate drivers, mysql is only supported
	//   Path: indicate databases
	//
	Connection string `json:"connection,omitempty"`
	// Table name
	TableName string `json:"table_name,omitempty"`
	// User name used to login database
	User string `json:"user,omitempty"`
	// User password
	Password string `json:"password,omitempty"`
}

// String return json format string of Repository
func (repo *DatabaseRepository) String() string {
	if bytes, err := json.Marshal(repo); err == nil {
		return string(bytes)
	}
	return "{}"
}

// CollectionConfig configuration of collection
type CollectionConfig struct {
	// Name of collection
	CollectionName string `json:"collection_name,omitempty"`
	// Max documents per segment, Optional field, default is max value of system(^uint64(0) = 18446744073709551615）
	MaxDocsPerSegment uint64 `json:"max_docs_per_segment"`
	// Entity of document, optional field, default is empty
	ForwardColumns []string `json:"forward_columns,omitempty"`
	// Columns of document, empty array is illegal argument
	Columns []ColumnIndex `json:"columns"`
	// Repository of collection, Optional field, nil for collection which import from client only
	Repository *DatabaseRepository `json:"repository,omitempty"`
}

// String method return json formatted string of CollectionConfig
func (config *CollectionConfig) String() string {
	if bytes, err := json.Marshal(config); err == nil {
		return string(bytes)
	}
	return "{}"
}

// LsnContext Log Sequence Number Context indicate specific point of collection
type LsnContext struct {
	// Sequence number of collection
	LSN uint64 `json:"lsn"`
	// Context of collection
	Context string `json:"context,omitempty"`
}

// String return json format of LsnContext
func (context *LsnContext) String() string {
	if bytes, err := json.Marshal(context); err == nil {
		return string(bytes)
	}
	return "{}"
}

// CollectionInfo Details of collection
type CollectionInfo struct {
	// Collection config
	CollectionConfig
	// Status of collection
	Status CollectionStatus `json:"status"`
	// Unique id of collection, allocated by ProximaSE
	UUID string `json:"uuid,omitempty"`
	// Latest LSN Context
	Context *LsnContext `json:"context"`
	// Magic number of collection
	MagicNumber uint64 `json:"magic_number"`
}

// String method return json formatted string of CollectionInfo
func (info *CollectionInfo) String() string {
	if bytes, err := json.Marshal(info); err == nil {
		return string(bytes)
	}
	return "{}"
}

// Range for define limitation of properties
type Range struct {
	// Beginning of range
	Min uint64
	// Ending of range
	Max uint64
}

// SegmentStat stats of Segment
type SegmentStat struct {
	// ID of segment
	SegmentID uint32
	// State of segment
	State SegmentState
	// Count of documents in segment
	DocCount uint64
	// Index files inside segment
	IndexFileCount uint64
	// Size of segment
	IndexFileSize uint64
	// Range of documents inside segment
	DocsRange Range
	// Primary key range of segment
	PrimaryKeyRange Range
	// Range of document timestamp inside segment
	TimestampRange Range
	// Range of LSN
	LSNRange Range
	// Storage path of segment
	SegmentPath string
}

// String method return json formatted string of SegmentStat
func (stat *SegmentStat) String() string {
	if bytes, err := json.Marshal(stat); err == nil {
		return string(bytes)
	}
	return "{}"
}

// CollectionStat stat of Collection
type CollectionStat struct {
	// Name of collection
	CollectionName string
	// Storage path of collection
	CollectionPath string
	// Total documents inside collection
	TotalDocCount uint64
	// The number of segments in collection
	TotalSegmentCount uint64
	// The number of files in collection
	TotalIndexFileCount uint64
	// The total size of storage, count by bytes
	TotalIndexFileSize uint64
	// Segment stats
	SegmentStats []SegmentStat
}

// String method return json formatted string of CollectionStat
func (stat *CollectionStat) String() string {
	if bytes, err := json.Marshal(stat); err == nil {
		return string(bytes)
	}
	return "{}"
}

// Row record
type Row struct {
	// Primary key of row record
	PrimaryKey uint64
	// Operation type
	OperationType
	// Index column value list
	IndexColumnValues []interface{}
	// Forward column value list
	ForwardColumnValues []interface{}
	// Log Sequence Number context
	*LsnContext
}

// RowMeta meta for row records
type RowMeta struct {
	// Index column name list
	IndexColumnNames []string
	// Index column name list
	ForwardColumnNames []string
}

// WriteRequest object, the parameter of ProximaSEClient.Write method
type WriteRequest struct {
	// Name of collection
	CollectionName string
	// Meta header
	Meta RowMeta
	// Row record list
	Rows []Row
	// Request ID, Optional
	RequestID string
	// Magic number, Optional
	MagicNumber uint64
}

// Document for search result
type Document struct {
	// Primary key of document
	PrimaryKey uint64 `json:"primary_key,omitempty"`
	// Similarity of document to query vector, bigger is more similar
	Score float32 `json:"score"`
	// Properties of document
	ForwardColumns map[string]interface{} `json:"forwards,omitempty"`
}

// String return json format of document
func (doc *Document) String() string {
	if bytes, err := json.Marshal(doc); err == nil {
		return string(bytes)
	}
	return "{}"
}

func (doc *Document) GetForward(name string) (reflect.Value, error) {
	if value, ok := doc.ForwardColumns[name]; ok {
		return reflect.ValueOf(value), nil
	}
	return reflect.ValueOf(nil), errors.New("Can't get forward value")
}

// QueryResponse Response of QueryRequest
type QueryResponse struct {
	// Debug Information, json format string
	DebugInfo string
	// Latency of request, count by ms
	Latency uint64
	// Documents of query
	Documents [][]*Document
}

// String return json format of QueryResponse
func (resp *QueryResponse) String() string {
	if bytes, err := json.Marshal(resp); err == nil {
		return string(bytes)
	}
	return "{}"
}
