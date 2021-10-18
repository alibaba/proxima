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
	"bytes"
	"context"
	"fmt"
	"io"
	"io/ioutil"
	"math"
	"net/http"
	"time"

	pb "github.com/alibaba/proximabilin/sdk/go/proto"

	"google.golang.org/grpc"
	"google.golang.org/protobuf/encoding/protojson"
	"google.golang.org/protobuf/proto"
)

// proximaSEPBClient implements ProximaSEClient
type proximaSEPBClient interface {
	// get version from Proxima BE, format: Major.Minor.Patch-xxx
	GetVersion() (string, error)
	// create collectionp
	CreateCollection(in *pb.CollectionConfig) (*pb.Status, error)
	// drop collection
	DropCollection(in *pb.CollectionName) (*pb.Status, error)
	// retrieve collection
	DescribeCollection(in *pb.CollectionName) (*pb.DescribeCollectionResponse, error)
	// retrieve collection list
	ListCollections(in *pb.ListCondition) (*pb.ListCollectionsResponse, error)
	// stats collection
	StatsCollection(in *pb.CollectionName) (*pb.StatsCollectionResponse, error)
	// write records
	Write(in *pb.WriteRequest) (*pb.Status, error)
	// query records
	Query(in *pb.QueryRequest) (*pb.QueryResponse, error)
	// get document by key
	GetDocumentByKey(in *pb.GetDocumentRequest) (*pb.GetDocumentResponse, error)
}

func buildContext() (context.Context, context.CancelFunc) {
	return context.WithTimeout(context.Background(), 10*time.Second)
}

// Grpc client
type grpcClient struct {
	// Grpc connection
	client pb.ProximaServiceClient
}

func (c *grpcClient) GetVersion() (string, error) {
	context, _ := buildContext()
	version, err := c.client.GetVersion(context, &pb.GetVersionRequest{})
	if err != nil {
		return "", nil
	}
	return version.Version, nil
}

func (c *grpcClient) CreateCollection(in *pb.CollectionConfig) (*pb.Status, error) {
	context, _ := buildContext()
	return c.client.CreateCollection(context, in)
}

func (c *grpcClient) DropCollection(in *pb.CollectionName) (*pb.Status, error) {
	context, _ := buildContext()
	return c.client.DropCollection(context, in)
}

func (c *grpcClient) DescribeCollection(in *pb.CollectionName) (*pb.DescribeCollectionResponse, error) {
	context, _ := buildContext()
	return c.client.DescribeCollection(context, in)
}

func (c *grpcClient) ListCollections(in *pb.ListCondition) (*pb.ListCollectionsResponse, error) {
	context, _ := buildContext()
	return c.client.ListCollections(context, in)
}

func (c *grpcClient) StatsCollection(in *pb.CollectionName) (*pb.StatsCollectionResponse, error) {
	context, _ := buildContext()
	return c.client.StatsCollection(context, in)
}

func (c *grpcClient) Write(in *pb.WriteRequest) (*pb.Status, error) {
	context, _ := buildContext()
	return c.client.Write(context, in)
}

func (c *grpcClient) Query(in *pb.QueryRequest) (*pb.QueryResponse, error) {
	context, _ := buildContext()
	return c.client.Query(context, in)
}

func (c *grpcClient) GetDocumentByKey(in *pb.GetDocumentRequest) (*pb.GetDocumentResponse, error) {
	context, _ := buildContext()
	return c.client.GetDocumentByKey(context, in)
}

var (
	versionPath          = "/service_version"
	collectionManagePath = "/v1/collection/%s"
	statCollectionPath   = "/v1/collection/%s/stats"
	writePath            = "/v1/collection/%s/index"
	getDocumentByKeyPath = "/v1/collection/%s/doc?key=%d"
	queryPath            = "/v1/collection/%s/query"
	listCollectionsPath  = "/v1/collections%s"
)

type httpClient struct {
	host string
}

func (c *httpClient) makeURI(format string, args ...interface{}) string {
	path := fmt.Sprintf(format, args...)
	return fmt.Sprintf("http://%s%s", c.host, path)
}

func (c *httpClient) getMessageFromIO(r io.Reader, m proto.Message) error {
	body, err := ioutil.ReadAll(r)
	if err != nil {
		return err
	}
	return protojson.Unmarshal(body, m)
}

func (c *httpClient) getStatusFromIO(r io.Reader) (*pb.Status, error) {
	status := new(pb.Status)
	err := c.getMessageFromIO(r, status)
	if err != nil {
		return nil, err
	}
	return status, nil
}

func httpGetWithBody(url string, contentType string, body io.Reader) (resp *http.Response, err error) {
	req, err := http.NewRequest("GET", url, body)
	if err != nil {
		return nil, err
	}
	req.Header.Set("Content-Type", contentType)
	return http.DefaultClient.Do(req)
}

func httpDel(url string) (resp *http.Response, err error) {
	req, err := http.NewRequest("DELETE", url, nil)
	if err != nil {
		return nil, err
	}
	return http.DefaultClient.Do(req)
}

func (c *httpClient) GetVersion() (string, error) {
	res, err := http.Get(c.makeURI(versionPath))
	if err != nil {
		return "", err
	}
	defer res.Body.Close()
	version := new(pb.GetVersionResponse)
	err = c.getMessageFromIO(res.Body, version)
	if err != nil {
		return "", err
	}
	return version.Version, nil
}

func (c *httpClient) CreateCollection(in *pb.CollectionConfig) (*pb.Status, error) {
	body, err := protojson.Marshal(in)
	if err != nil {
		return nil, err
	}
	res, err := http.Post(c.makeURI(collectionManagePath, in.CollectionName), "application/json", bytes.NewReader(body))
	if err != nil {
		return nil, err
	}
	defer res.Body.Close()
	return c.getStatusFromIO(res.Body)
}

func (c *httpClient) DropCollection(in *pb.CollectionName) (*pb.Status, error) {
	res, err := httpDel(c.makeURI(collectionManagePath, in.CollectionName))
	if err != nil {
		return nil, err
	}
	defer res.Body.Close()
	return c.getStatusFromIO(res.Body)
}

func (c *httpClient) DescribeCollection(in *pb.CollectionName) (*pb.DescribeCollectionResponse, error) {
	res, err := http.Get(c.makeURI(collectionManagePath, in.CollectionName))
	if err != nil {
		return nil, err
	}
	defer res.Body.Close()

	resp := new(pb.DescribeCollectionResponse)
	err = c.getMessageFromIO(res.Body, resp)
	if err != nil {
		return nil, err
	}
	return resp, nil
}

func (c *httpClient) ListCollections(in *pb.ListCondition) (*pb.ListCollectionsResponse, error) {
	query := ""
	if len(in.RepositoryName) != 0 {
		query = fmt.Sprintf("?repository=%s", in.RepositoryName)
	}
	res, err := http.Get(c.makeURI(listCollectionsPath, query))
	if err != nil {
		return nil, err
	}
	defer res.Body.Close()

	resp := &pb.ListCollectionsResponse{}
	err = c.getMessageFromIO(res.Body, resp)
	if err != nil {
		return nil, err
	}
	return resp, nil
}

func (c *httpClient) StatsCollection(in *pb.CollectionName) (*pb.StatsCollectionResponse, error) {
	res, err := http.Get(c.makeURI(statCollectionPath, in.CollectionName))
	if err != nil {
		return nil, err
	}
	defer res.Body.Close()

	resp := new(pb.StatsCollectionResponse)
	err = c.getMessageFromIO(res.Body, resp)
	if err != nil {
		return nil, err
	}
	return resp, nil
}

func (c *httpClient) Write(in *pb.WriteRequest) (*pb.Status, error) {
	body, err := protojson.Marshal(in)
	if err != nil {
		return nil, err
	}
	res, err := http.Post(c.makeURI(writePath, in.CollectionName), "application/json", bytes.NewReader(body))
	if err != nil {
		return nil, err
	}
	defer res.Body.Close()
	return c.getStatusFromIO(res.Body)
}

func (c *httpClient) Query(in *pb.QueryRequest) (*pb.QueryResponse, error) {
	body, err := protojson.Marshal(in)
	if err != nil {
		return nil, err
	}
	res, err := http.Post(c.makeURI(queryPath, in.CollectionName), "application/json", bytes.NewReader(body))
	if err != nil {
		return nil, err
	}
	defer res.Body.Close()

	resp := new(pb.QueryResponse)
	err = c.getMessageFromIO(res.Body, resp)
	if err != nil {
		return nil, err
	}
	return resp, nil
}

func (c *httpClient) GetDocumentByKey(in *pb.GetDocumentRequest) (*pb.GetDocumentResponse, error) {
	res, err := http.Get(c.makeURI(getDocumentByKeyPath, in.CollectionName, in.PrimaryKey))
	if err != nil {
		return nil, err
	}
	defer res.Body.Close()

	resp := new(pb.GetDocumentResponse)
	err = c.getMessageFromIO(res.Body, resp)
	if err != nil {
		return nil, err
	}
	return resp, nil
}

// newProximaSEPBClientFromClient create PB Client
func newProximaSEPBClient(connType ConnectionProtocol, address Address) (proximaSEPBClient, error) {
	if connType == GrpcProtocol {
		var opts []grpc.DialOption
		opts = append(opts, grpc.WithInsecure())
		opts = append(opts, grpc.WithBlock())
		opts = append(opts, grpc.WithDefaultCallOptions(grpc.MaxCallSendMsgSize(math.MaxInt64)))
		opts = append(opts, grpc.WithDefaultCallOptions(grpc.MaxCallRecvMsgSize(math.MaxInt64)))

		ctx, cancel := context.WithTimeout(context.Background(), 10*time.Second)
		defer cancel()
		conn, err := grpc.DialContext(ctx, address.address(), opts...)
		if err != nil {
			return nil, err
		}
		return &grpcClient{client: pb.NewProximaServiceClient(conn)}, nil
	}
	return &httpClient{host: address.address()}, nil
}
