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

package main

import (
	"flag"
	"fmt"
	"log"
	"os"

	be "github.com/alibaba/proximabilin/sdk/go"
)

var (
	Host = flag.String("host", "localhost", "Host of ProximaBE")
	Port = flag.Uint("port", 16000, "Port of ProximaBE")
	Http = flag.Bool("http", false, "Using http protocol to communicate with ProximaBE, default is false")
)

func initAndParseArgs() {
	flag.Usage = func() {
		_, _ = fmt.Fprint(os.Stderr, "cli : example -host=127.0.0.1 -port=16000\n")
		_, _ = fmt.Fprint(os.Stderr, "cli : example -host=127.0.0.1 -port=16001 -http\n")
		flag.PrintDefaults()
	}
	flag.Parse()
}

func main() {
	initAndParseArgs()
	address := be.Address{
		Host: *Host,
		Port: uint32(*Port),
	}
	protoc := be.GrpcProtocol
	if *Http {
		protoc = be.HttpProtocol
	}
	client, err := be.NewProximaSearchClient(protoc, &address)
	if err != nil {
		log.Fatal("Can't create ProximaClient instance.", err)
	}

	// List all collections
	collections, err := client.ListCollections()
	if err != nil {
		log.Fatal("Can't retrieve collections from Proxima Server.", err)
	}
	log.Printf("Collections (%d): \n", len(collections))
	for _, collection := range collections {
		log.Printf("%+v\n", collection)
	}

	// List all collections by Repo
	collections, err = client.ListCollections(be.ByRepo("repo"))
	if err != nil {
		log.Fatal("Can't retrieve collections from Proxima Server.", err)
	}
	log.Printf("Collections (%d): \n", len(collections))
	for _, collection := range collections {
		log.Printf("%+v\n", collection)
	}

	client.DropCollection("example")
	// Create collection with no attached repository
	config := &be.CollectionConfig{
		CollectionName:    "example",
		MaxDocsPerSegment: 0, // 0 means unlimited, which is equal max value of system
		ForwardColumns:    []string{"forward", "forward1", "forward2"},
		Columns: []be.ColumnIndex{{
			Name:        "column",
			IndexType:   0,                                     // 0 means default index, which is be.ProximaGraphIndex
			DataType:    0,                                     // 0 means default, which is be.VectorFP32
			Dimension:   8,                                     // Required field, no default value, 0 is not legal argument
			ExtraParams: map[string]string{"ef_search": "200"}, // Advanced params
		}, {
			Name:        "column1",
			IndexType:   be.ProximaGraphIndex,
			DataType:    be.VectorFP16, // Index type is fp16, query could be fp32 for lack of language types
			Dimension:   128,
			ExtraParams: map[string]string{},
		},
		},
		Repository: nil, // No repository attached
	}
	if err = client.CreateCollection(config); err != nil {
		log.Fatal("Can't create collection.", err)
	}
	log.Print("Create collection succeed.")

	// Retrieve collection named by 'example'
	info, err := client.DescribeCollection("example")
	if err != nil {
		log.Fatal("Lost collection named by 'example', which created before.", err)
	}
	log.Printf("Collection: %+v\n", info)

	// Create collection with no attached repository
	config = &be.CollectionConfig{
		CollectionName:    "example_with_repo",
		MaxDocsPerSegment: 0, // 0 means unlimited, which is equal max value of system
		ForwardColumns:    []string{"forward", "forward1"},
		Columns: []be.ColumnIndex{{
			Name:        "column1",
			IndexType:   0,                                     // 0 means default index, which is be.ProximaGraphIndex
			DataType:    0,                                     // 0 means default, which is be.VectorFP32
			Dimension:   512,                                   // Required field, no default value, 0 is not legal argument
			ExtraParams: map[string]string{"ef_search": "200"}, // Advanced params
		},
		},
		Repository: &be.DatabaseRepository{
			Repository: be.Repository{
				Name: "mysql_repo",
				Type: be.Database,
			},
			Connection: "mysql://host.com:8080/mysql_database", // JDBC connection uri
			TableName:  "table_name",                           // Table name
			User:       "root",                                 // User name
			Password:   "root",                                 // Password
		},
	}
	client.DropCollection("example_with_repo")
	if err = client.CreateCollection(config); err != nil {
		log.Fatal("Can't create collection.", err)
	}
	log.Print("Create collection with attached mysql repository succeed.")

	// Retrieve collection named by 'example_with_repo'
	info, err = client.DescribeCollection("example_with_repo")
	if err != nil {
		log.Fatal("Lost collection named by 'example_with_repo', which created before.", err)
	}
	log.Printf("Collection(With Repository): %+v\n", info)

	// Delete collection
	if err = client.DropCollection("example_with_repo"); err != nil {
		log.Fatal("Failed to drop collection,", err)
	}
	log.Print("Drop collection succeed.")

	rows := &be.WriteRequest{
		CollectionName: "example",
		Meta: be.RowMeta{
			IndexColumnNames:   []string{"column"},
			ForwardColumnNames: []string{"forward", "forward1", "forward2"},
		},
		Rows: []be.Row{
			{
				PrimaryKey:          0,
				OperationType:       be.Insert,
				ForwardColumnValues: []interface{}{1, float32(1.1), true},
				IndexColumnValues: []interface{}{ // Data type should same with index created before
					[]float32{1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0},
				},
				LsnContext: nil, // With empty context
			}, {
				PrimaryKey:          2,
				OperationType:       be.Insert,
				ForwardColumnValues: []interface{}{2, float32(2.2), false},
				IndexColumnValues: []interface{}{ // Data type should same with index created before
					[]float32{21.0, 22.0, 23.0, 24.0, 25.0, 26.0, 27.0, 28.0},
				},
				LsnContext: &be.LsnContext{ // With valid context
					LSN:     0,
					Context: "write context hear",
				},
			}},
		RequestID:   "", // Optional field
		MagicNumber: 0,  // Optional field
	}
	// Write rows to collection
	err = client.Write(rows)
	if err != nil {
		log.Fatal("Insert data to collection failed.", err)
	}

	stat, err := client.StatCollection("example")
	if err != nil {
		log.Fatal("Stat collection failed.", err)
	}
	log.Printf("Collection Stat: %+v", stat)

	// Query one vector
	resp, err := client.Query("example", "column", []float32{1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0}, be.WithTopK(10))
	if err != nil {
		log.Fatal("Query failed.", err)
	}
	log.Printf("Response: %+v\n", resp)

	// Query with matrix
	resp, err = client.Query("example", "column",
		[][]float32{{1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0},
			{1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0},
			{1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0},
			{1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0},
		},
		be.WithTopK(10),
		be.WithDebugMode(),    // Enable debug mode, do not recommend on product environments
		be.WithRadius(1.5),    // Search radius, no effect if be.WithLinearSearch enabled
		be.WithLinearSearch(), // Enable linear search
		be.WithParam("customize_param", 10),
		be.WithParam("customize_param2", 1.0),
		be.WithParam("customize_param3", "str"))
	if err != nil {
		log.Fatal("Query failed.", err)
	}
	log.Printf("Response: %+v\n", resp)

	// Retrieve document by primary key
	doc, err := client.GetDocumentByKey("example", 0)
	if err != nil {
		log.Fatal("Failed to retrieve document from server.", err)
	}
	log.Printf("Document: %+v\n", doc)

	// Retrieve stat of collection
	stat, err = client.StatCollection("example")
	if err != nil {
		log.Fatal("Failed to retrieve stat of collection.", err)
	}
	log.Printf("Stat: %+v\n", stat)
}
