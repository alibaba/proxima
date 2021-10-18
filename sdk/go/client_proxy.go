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
 *   \brief Implementation of ProximaBEClient interface
 */

package be

import (
	"errors"

	pb "github.com/alibaba/proximabilin/sdk/go/proto"
)

func wrapError(in *pb.Status) error {
	err := Status{
		Code:   in.Code,
		Reason: in.Reason,
	}
	if err.OK() {
		return nil
	}
	return &err
}

type proximaSEClientProxy struct {
	client proximaSEPBClient
}

func (proxy *proximaSEClientProxy) CreateCollection(config *CollectionConfig) error {
	in, err := buildPBCollectionConfig(config)
	if err != nil {
		return err
	}
	status, err := proxy.client.CreateCollection(in)
	if err != nil {
		return err
	}
	return wrapError(status)
}

func (proxy *proximaSEClientProxy) DropCollection(collection string) error {
	status, err := proxy.client.DropCollection(&pb.CollectionName{
		CollectionName: collection,
	})
	if err != nil {
		return err
	}
	return wrapError(status)
}

func (proxy *proximaSEClientProxy) DescribeCollection(collection string) (*CollectionInfo, error) {
	resp, err := proxy.client.DescribeCollection(&pb.CollectionName{
		CollectionName: collection,
	})
	// Error of rpc client
	if err != nil {
		return nil, err
	}
	// Error from BE
	err = wrapError(resp.Status)
	if err != nil {
		return nil, err
	}
	return buildCollectionInfoFromPB(resp.Collection)
}

func (proxy *proximaSEClientProxy) ListCollections(filters ...ListCollectionFilter) ([]*CollectionInfo, error) {
	cond := pb.ListCondition{}
	for _, filter := range filters {
		filter.apply(&cond)
	}
	resp, err := proxy.client.ListCollections(&cond)
	if err != nil {
		return nil, err
	}
	err = wrapError(resp.Status)
	if err != nil {
		return nil, err
	}
	collections := make([]*CollectionInfo, len(resp.Collections))
	for i, param := range resp.Collections {
		collection, err := buildCollectionInfoFromPB(param)
		if err != nil {
			return nil, err
		}
		collections[i] = collection
	}
	return collections, nil
}

func (proxy *proximaSEClientProxy) StatCollection(collection string) (*CollectionStat, error) {
	in := pb.CollectionName{
		CollectionName: collection,
	}
	resp, err := proxy.client.StatsCollection(&in)
	if err != nil {
		return nil, err
	}
	err = wrapError(resp.Status)
	if err != nil {
		return nil, err
	}
	return buildCollectionStatFromPB(resp.CollectionStats)
}

func (proxy *proximaSEClientProxy) Write(req *WriteRequest) error {
	in, err := buildPBWriteRequest(req)
	if err != nil {
		return &Status{
			Code:   int32(Unknown),
			Reason: err.Error(),
		}
	}
	status, err := proxy.client.Write(in)
	if err != nil {
		return &Status{
			Code:   int32(Unknown),
			Reason: err.Error(),
		}
	}
	if err = wrapError(status); err != nil {
		if ErrorCode(status.Code) == RetryLater {
			return &Status{
				Code:   int32(RetryLater),
				Reason: err.Error(),
			}
		}
		return &Status{
			Code:   int32(Unknown),
			Reason: err.Error(),
		}
	}
	return nil
}

func (proxy *proximaSEClientProxy) Query(collection string, column string, features interface{}, opts ...QueryOption) (*QueryResponse, error) {
	req, err := buildPBQueryRequest(collection, column, features, opts...)
	if err != nil {
		return nil, err
	}
	resp, err := proxy.client.Query(req)
	if err != nil {
		return nil, err
	}
	if err = wrapError(resp.Status); err != nil {
		return nil, err
	}
	return buildQueryResponseFromPB(resp)
}

func (proxy *proximaSEClientProxy) GetDocumentByKey(collection string, primaryKey uint64) (*Document, error) {
	in := &pb.GetDocumentRequest{
		CollectionName: collection,
		PrimaryKey:     primaryKey,
		DebugMode:      false,
	}
	resp, err := proxy.client.GetDocumentByKey(in)
	if err != nil {
		return nil, err
	}
	if err = wrapError(resp.Status); err != nil {
		return nil, err
	}
        if resp.Document == nil {
                return nil, errors.New("Null Document")
        }
	return buildDocumentFromPB(resp.Document), nil
}

// DefaultAddress create one address for Proxima Search Engine
func DefaultAddress() *Address {
	return &Address{
		Host: "127.0.0.1",
		Port: 16000,
	}
}

// NewProximaSearchClient create new ProximaSearchClient object
func NewProximaSearchClient(conn ConnectionProtocol, address *Address) (ProximaSearchClient, error) {
	client, err := newProximaSEPBClient(conn, *address)
	if err != nil {
		// log.Printf("Failed to create PB client. err: %+v", err)
		return nil, err
	}

	ver, err := client.GetVersion()
	if err != nil {
		return nil, err
	}

	v := Version{Client: VERSION, Server: ver}
	if v.Compatible() {
		return &proximaSEClientProxy{
			client: client,
		}, nil
	}
	return nil, errors.New("Uncompatible SDK and Proxima BE. " + v.String())
}
