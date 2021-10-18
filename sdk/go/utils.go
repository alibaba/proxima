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
	"bytes"
	"encoding/binary"
	"errors"
	"log"
	"math"
	"reflect"

	pb "github.com/alibaba/proxima-be/sdk/go/proto"

	"github.com/boljen/go-bitmap"
)

func createAcceptableTypes() bitmap.Bitmap {
	var bitmaps = bitmap.New(int(reflect.UnsafePointer))
	bitmaps.Set(int(reflect.Slice), true)
	bitmaps.Set(int(reflect.String), true)
	bitmaps.Set(int(reflect.Bool), true)
	bitmaps.Set(int(reflect.Int), true)
	bitmaps.Set(int(reflect.Int32), true)
	bitmaps.Set(int(reflect.Int64), true)
	bitmaps.Set(int(reflect.Uint), true)
	bitmaps.Set(int(reflect.Uint32), true)
	bitmaps.Set(int(reflect.Uint64), true)
	bitmaps.Set(int(reflect.Float32), true)
	bitmaps.Set(int(reflect.Float64), true)
	return bitmaps
}

func createAcceptableFeatureTypes() bitmap.Bitmap {
	var bitmaps = bitmap.New(int(reflect.UnsafePointer))
	bitmaps.Set(int(reflect.Int8), true)
	bitmaps.Set(int(reflect.Uint32), true)
	bitmaps.Set(int(reflect.Uint64), true)
	bitmaps.Set(int(reflect.Float32), true)
	return bitmaps
}

func indexTypeApplier(column *pb.CollectionConfig_IndexColumnParam) {
	if column.IndexType == pb.IndexType_IT_UNDEFINED {
		column.IndexType = pb.IndexType_IT_PROXIMA_GRAPH_INDEX
	}
}

func dataTypeApplier(column *pb.CollectionConfig_IndexColumnParam) {
	if column.DataType == pb.DataType_DT_UNDEFINED {
		column.DataType = pb.DataType_DT_VECTOR_FP32
	}
}

var (
	genericAcceptableTypes     = createAcceptableTypes()
	queryFeatureTypes          = createAcceptableFeatureTypes()
	columnDefaultValueAppliers = []func(*pb.CollectionConfig_IndexColumnParam){
		indexTypeApplier,
		dataTypeApplier,
	}
	sizeOfDataType = map[pb.DataType]int{
		pb.DataType_DT_VECTOR_BINARY32: 4,
		pb.DataType_DT_VECTOR_BINARY64: 8,
		pb.DataType_DT_VECTOR_FP32:     4,
		pb.DataType_DT_VECTOR_INT8:     1,
	}
)

func acceptableValueTypes(value interface{}) bool {
	kind := reflect.TypeOf(value).Kind()
	return genericAcceptableTypes.Get(int(kind))
}

func acceptableFeatureTypes(t reflect.Type) bool {
	return queryFeatureTypes.Get(int(t.Kind()))
}

func isSlice(t reflect.Type) bool {
	return t.Kind() == reflect.Slice
}

func isArray(t reflect.Type) bool {
	return t.Kind() == reflect.Array
}

func isSliceOrArray(t reflect.Type) bool {
	return isSlice(t) || isArray(t)
}

func inferVectorFeatureType(features interface{}) (reflect.Kind, bool) {
	ft := reflect.ValueOf(features)
	if isSliceOrArray(ft.Type()) {
		if ft.Len() > 0 && acceptableFeatureTypes(ft.Index(0).Type()) {
			return ft.Index(0).Kind(), true
		}
	}
	return reflect.Invalid, false
}

func inferMatrixFeatureType(features interface{}) (reflect.Kind, bool) {
	ft := reflect.ValueOf(features)
	if isSliceOrArray(ft.Type()) && ft.Len() > 0 &&
		isSliceOrArray(ft.Index(0).Type()) && ft.Index(0).Len() > 0 &&
		acceptableFeatureTypes(ft.Index(0).Index(0).Type()) {
		return ft.Index(0).Index(0).Kind(), true
	}
	return reflect.Invalid, false
}

func inferMatrixDimension(matrix interface{}) (int, int, error) {
	ft := reflect.ValueOf(matrix)
	min, max := math.MaxInt32, math.MinInt32
	for i := 0; i < ft.Len(); i++ {
		counts := ft.Index(i).Len()
		if counts < min {
			min = counts
		}
		if counts > max {
			max = counts
		}
	}
	if min == max || min > 0 {
		return ft.Len(), max, nil
	}
	return 0, 0, errors.New("Dimension of vectors not equal")
}

func inferFeatures(features interface{}) (pb.DataType, int, int, error) {
	if kind, ok := inferMatrixFeatureType(features); ok {
		if batch, dimension, err := inferMatrixDimension(features); err == nil {
			switch kind {
			case reflect.Int8:
				return pb.DataType_DT_VECTOR_INT8, dimension, batch, nil
			case reflect.Uint32:
				return pb.DataType_DT_VECTOR_BINARY32, dimension, batch, nil
			case reflect.Uint64:
				return pb.DataType_DT_VECTOR_BINARY64, dimension, batch, nil
			case reflect.Float32:
				return pb.DataType_DT_VECTOR_FP32, dimension, batch, nil
			}
		}
	} else if kind, ok := inferVectorFeatureType(features); ok {
		value := reflect.ValueOf(features)
		switch kind {
		case reflect.Int8:
			return pb.DataType_DT_VECTOR_INT8, value.Len(), 1, nil
		case reflect.Uint32:
			return pb.DataType_DT_VECTOR_BINARY32, value.Len(), 1, nil
		case reflect.Uint64:
			return pb.DataType_DT_VECTOR_BINARY64, value.Len(), 1, nil
		case reflect.Float32:
			return pb.DataType_DT_VECTOR_FP32, value.Len(), 1, nil
		}
	}
	return pb.DataType_DT_UNDEFINED, 0, 0, errors.New("The type of features can't acceptable, please refers to help of Query method")
}

func applyDefaultValue(column *pb.CollectionConfig_IndexColumnParam) *pb.CollectionConfig_IndexColumnParam {
	for _, applier := range columnDefaultValueAppliers {
		applier(column)
	}
	return column
}

func buildPBCollectionConfig(config *CollectionConfig) (*pb.CollectionConfig, error) {
	in := pb.CollectionConfig{
		CollectionName:     config.CollectionName,
		MaxDocsPerSegment:  config.MaxDocsPerSegment,
		ForwardColumnNames: config.ForwardColumns,
		IndexColumnParams:  make([]*pb.CollectionConfig_IndexColumnParam, len(config.Columns)),
		RepositoryConfig:   nil,
	}
	for i, cc := range config.Columns {
		column := pb.CollectionConfig_IndexColumnParam{
			ColumnName:  cc.Name,
			IndexType:   pb.IndexType(cc.IndexType),
			DataType:    pb.DataType(cc.DataType),
			Dimension:   cc.Dimension,
			ExtraParams: []*pb.KeyValuePair{},
		}
		for k, v := range cc.ExtraParams {
			column.ExtraParams = append(column.ExtraParams, &pb.KeyValuePair{Key: k, Value: v})
		}
		in.IndexColumnParams[i] = applyDefaultValue(&column)
	}

	if config.Repository != nil {
		if config.Repository.Type == Database {
			in.RepositoryConfig = &pb.CollectionConfig_RepositoryConfig{
				RepositoryType: pb.CollectionConfig_RepositoryConfig_RepositoryType(Database),
				RepositoryName: config.Repository.Name,
				Entity: &pb.CollectionConfig_RepositoryConfig_Database_{
					Database: &pb.CollectionConfig_RepositoryConfig_Database{
						ConnectionUri: config.Repository.Connection,
						TableName:     config.Repository.TableName,
						User:          config.Repository.User,
						Password:      config.Repository.Password,
					},
				},
			}
		} else {
			// Error, Unknown respository type
			return nil, errors.New("Error, Unknown respository type")
		}
	}
	return &in, nil
}

func buildRepositoryConfigFromPB(in *pb.CollectionConfig_RepositoryConfig) *DatabaseRepository {
	if in == nil {
		return nil
	}
	// Deserialize DatabaseRepository from PB message
	repo := DatabaseRepository{
		Repository: Repository{
			Name: in.RepositoryName,
			Type: RepositoryType(in.RepositoryType),
		},
		Connection: in.GetDatabase().ConnectionUri,
		TableName:  in.GetDatabase().TableName,
		User:       in.GetDatabase().User,
		Password:   in.GetDatabase().Password,
	}
	// Error
	if repo.Repository.Type != Database {
		log.Print("Error: can't deserialize DatabaseRepository from PB")
		return nil
	}

	return &repo
}

func buildCollectionInfoFromPB(collection *pb.CollectionInfo) (*CollectionInfo, error) {
	info := CollectionInfo{
		CollectionConfig: CollectionConfig{
			CollectionName:    collection.Config.CollectionName,
			MaxDocsPerSegment: collection.Config.MaxDocsPerSegment,
			ForwardColumns:    collection.Config.ForwardColumnNames,
			Columns:           make([]ColumnIndex, len(collection.Config.IndexColumnParams)),
			Repository:        nil,
		},
		Status:      CollectionStatus(collection.Status),
		UUID:        collection.Uuid,
		Context:     nil,
		MagicNumber: collection.MagicNumber,
	}

	if collection.LatestLsnContext != nil {
		info.Context = &LsnContext{
			LSN:     collection.LatestLsnContext.Lsn,
			Context: collection.LatestLsnContext.Context,
		}
	}

	for i, param := range collection.Config.IndexColumnParams {
		column := ColumnIndex{
			Name:        param.ColumnName,
			IndexType:   IndexType(param.IndexType),
			DataType:    DataType(param.DataType),
			Dimension:   param.Dimension,
			ExtraParams: map[string]string{},
		}
		for _, kv := range param.ExtraParams {
			column.ExtraParams[kv.Key] = kv.Value
		}
		info.CollectionConfig.Columns[i] = column
	}

	info.CollectionConfig.Repository = buildRepositoryConfigFromPB(collection.Config.RepositoryConfig)
	return &info, nil
}

func buildCollectionStatFromPB(in *pb.CollectionStats) (*CollectionStat, error) {
	stat := CollectionStat{
		CollectionName:      in.CollectionName,
		CollectionPath:      in.CollectionPath,
		TotalDocCount:       in.TotalDocCount,
		TotalSegmentCount:   in.TotalSegmentCount,
		TotalIndexFileCount: in.TotalIndexFileCount,
		TotalIndexFileSize:  in.TotalIndexFileSize,
		SegmentStats:        make([]SegmentStat, len(in.SegmentStats)),
	}
	for i, segment := range in.SegmentStats {
		ss := SegmentStat{
			SegmentID:      segment.SegmentId,
			State:          SegmentState(segment.State),
			DocCount:       segment.DocCount,
			IndexFileCount: segment.IndexFileCount,
			IndexFileSize:  segment.IndexFileSize,
			DocsRange: Range{
				Min: segment.MinDocId,
				Max: segment.MaxDocId,
			},
			PrimaryKeyRange: Range{
				Min: segment.MinPrimaryKey,
				Max: segment.MaxPrimaryKey,
			},
			TimestampRange: Range{
				Min: segment.MinTimestamp,
				Max: segment.MaxTimestamp,
			},
			LSNRange: Range{
				Min: segment.MinLsn,
				Max: segment.MaxLsn,
			},
			SegmentPath: segment.SegmentPath,
		}
		stat.SegmentStats[i] = ss
	}
	return &stat, nil
}

func serializeSlice(in reflect.Value) ([]byte, error) {
	if in.Kind() != reflect.Array && in.Kind() != reflect.Slice {
		return nil, errors.New("Input value is not array or slice")
	}
	bytes := bytes.NewBuffer(make([]byte, 0, 1024))
	if err := binary.Write(bytes, binary.LittleEndian, in.Interface()); err != nil {
		return nil, err
	}
	return bytes.Bytes(), nil
}

func buildPBGenericValue(value interface{}) *pb.GenericValue {
	gv := &pb.GenericValue{
		ValueOneof: nil,
	}
	v := reflect.ValueOf(value)
	switch v.Kind() {
	case reflect.String:
		gv.ValueOneof = &pb.GenericValue_StringValue{
			StringValue: v.String(),
		}
	case reflect.Bool:
		gv.ValueOneof = &pb.GenericValue_BoolValue{
			BoolValue: v.Bool(),
		}
	case reflect.Int32:
		gv.ValueOneof = &pb.GenericValue_Int32Value{
			Int32Value: int32(v.Int()),
		}
	case reflect.Int64, reflect.Int:
		gv.ValueOneof = &pb.GenericValue_Int64Value{
			Int64Value: v.Int(),
		}
	case reflect.Uint32:
		gv.ValueOneof = &pb.GenericValue_Uint32Value{
			Uint32Value: uint32(v.Uint()),
		}
	case reflect.Uint64, reflect.Uint:
		gv.ValueOneof = &pb.GenericValue_Uint64Value{
			Uint64Value: v.Uint(),
		}
	case reflect.Float32:
		gv.ValueOneof = &pb.GenericValue_FloatValue{
			FloatValue: float32(v.Float()),
		}
	case reflect.Float64:
		gv.ValueOneof = &pb.GenericValue_DoubleValue{
			DoubleValue: v.Float(),
		}
	case reflect.Slice, reflect.Array:
		bytes, err := serializeSlice(v)
		if err == nil {
			gv.ValueOneof = &pb.GenericValue_BytesValue{
				BytesValue: bytes,
			}
		} else {
			log.Print(err)
		}
	default:
		log.Print("Unknown value type skip it")
	}

	if gv.ValueOneof == nil {
		return nil
	}
	return gv
}

func buildGenericValueList(in []interface{}, out *pb.GenericValueList) error {
	for i, v := range in {
		gv := buildPBGenericValue(v)
		if gv == nil {
			return errors.New("Serialize value to pb failed")
		}
		out.Values[i] = gv
	}
	return nil
}

func buildPBWriteRequest(req *WriteRequest) (*pb.WriteRequest, error) {
	in := pb.WriteRequest{
		CollectionName: req.CollectionName,
		RowMeta: &pb.WriteRequest_RowMeta{
			IndexColumnMetas:   make([]*pb.WriteRequest_IndexColumnMeta, len(req.Meta.IndexColumnNames)),
			ForwardColumnNames: req.Meta.ForwardColumnNames,
		},
		Rows:        make([]*pb.WriteRequest_Row, len(req.Rows)),
		RequestId:   req.RequestID,
		MagicNumber: req.MagicNumber,
	}
	for i, row := range req.Rows {
		pbRow := pb.WriteRequest_Row{
			PrimaryKey:    row.PrimaryKey,
			OperationType: pb.OperationType(row.OperationType),
			IndexColumnValues: &pb.GenericValueList{
				Values: make([]*pb.GenericValue, len(row.IndexColumnValues)),
			},
			ForwardColumnValues: &pb.GenericValueList{
				Values: make([]*pb.GenericValue, len(row.ForwardColumnValues)),
			},
			LsnContext: nil,
		}
		err := buildGenericValueList(row.IndexColumnValues, pbRow.IndexColumnValues)
		if err != nil {
			log.Print("Can't serialize value to PB")
			return nil, err
		}
		err = buildGenericValueList(row.ForwardColumnValues, pbRow.ForwardColumnValues)
		if err != nil {
			log.Print("Can't serialize value to PB")
			return nil, err
		}
		if row.LsnContext != nil {
			pbRow.LsnContext = &pb.LsnContext{
				Lsn:     row.LsnContext.LSN,
				Context: row.LsnContext.Context,
			}
		}
		in.Rows[i] = &pbRow
	}
	if len(req.Rows) == 0 {
		return nil, errors.New("No attached rows in request")
	}
	row := req.Rows[0]
	for i, name := range req.Meta.IndexColumnNames {
		if len(row.IndexColumnValues) <= i {
			return nil, errors.New("Mismatched values in write request")
		}
		dataType, dimension, _, err := inferFeatures(row.IndexColumnValues[i])
		if err != nil {
			return nil, errors.New("Can't infer the data type of vector")
		}
		in.RowMeta.IndexColumnMetas[i] = &pb.WriteRequest_IndexColumnMeta{
			ColumnName: name,
			DataType:   dataType,
			Dimension:  uint32(dimension),
		}
	}
	return &in, nil
}

func serializeFeature(batch int, features interface{}) (*bytes.Buffer, error) {
	bytes := bytes.NewBuffer(make([]byte, 0, 1024))
	if batch > 1 {
		queryFeatures := reflect.ValueOf(features)
		for i := 0; i < batch; i++ {
			err := binary.Write(bytes, binary.LittleEndian, queryFeatures.Index(i).Interface())
			if err != nil {
				return nil, err
			}
		}
	} else {
		if err := binary.Write(bytes, binary.LittleEndian, features); err != nil {
			return nil, err
		}
	}
	return bytes, nil
}

func buildPBQueryRequest(collection string, column string, features interface{}, opts ...QueryOption) (*pb.QueryRequest, error) {
	options := defaultOptions()
	for _, opt := range opts {
		if !opt.apply(options) {
			return nil, errors.New("Can't serialize options to query request")
		}
	}
	dataType, dimension, batch, err := inferFeatures(features)
	if err != nil {
		return nil, err
	}

	pbReq := &pb.QueryRequest{
		QueryType:      pb.QueryRequest_QT_KNN,
		CollectionName: collection,
		DebugMode:      options.debug,
		QueryParam:     nil,
	}
	bytes, err := serializeFeature(batch, features)
	if err != nil {
		return nil, err
	}
	param := &pb.QueryRequest_KnnParam{
		KnnParam: &pb.QueryRequest_KnnQueryParam{
			ColumnName:    column,
			Topk:          options.topk,
			FeaturesValue: &pb.QueryRequest_KnnQueryParam_Features{Features: bytes.Bytes()},
			BatchCount:    uint32(batch),
			Dimension:     uint32(dimension),
			DataType:      dataType,
			Radius:        options.radius,
			IsLinear:      options.linear,
			ExtraParams:   []*pb.KeyValuePair{},
		},
	}

	for k, v := range options.params {
		kv := &pb.KeyValuePair{
			Key:   k,
			Value: v,
		}
		param.KnnParam.ExtraParams = append(param.KnnParam.ExtraParams, kv)
	}
	pbReq.QueryParam = param
	return pbReq, nil
}

func buildDocumentFromPB(doc *pb.Document) *Document {
	document := &Document{
		PrimaryKey:     doc.PrimaryKey,
		Score:          doc.Score,
		ForwardColumns: map[string]interface{}{},
	}

	for _, pair := range doc.ForwardColumnValues {
		switch pair.Value.ValueOneof.(type) {
		case (*pb.GenericValue_BytesValue):
			document.ForwardColumns[pair.Key] = pair.Value.GetBytesValue()
		case (*pb.GenericValue_StringValue):
			document.ForwardColumns[pair.Key] = pair.Value.GetStringValue()
		case (*pb.GenericValue_BoolValue):
			document.ForwardColumns[pair.Key] = pair.Value.GetBoolValue()
		case (*pb.GenericValue_Int32Value):
			document.ForwardColumns[pair.Key] = pair.Value.GetInt32Value()
		case (*pb.GenericValue_Int64Value):
			document.ForwardColumns[pair.Key] = pair.Value.GetInt64Value()
		case (*pb.GenericValue_Uint32Value):
			document.ForwardColumns[pair.Key] = pair.Value.GetUint32Value()
		case (*pb.GenericValue_Uint64Value):
			document.ForwardColumns[pair.Key] = pair.Value.GetUint64Value()
		case (*pb.GenericValue_FloatValue):
			document.ForwardColumns[pair.Key] = pair.Value.GetFloatValue()
		case (*pb.GenericValue_DoubleValue):
			document.ForwardColumns[pair.Key] = pair.Value.GetDoubleValue()
		}
	}
	return document
}

func buildQueryResponseFromPB(resp *pb.QueryResponse) (*QueryResponse, error) {
	response := &QueryResponse{
		DebugInfo: resp.DebugInfo,
		Latency:   resp.LatencyUs,
		Documents: make([][]*Document, len(resp.Results)),
	}
	for nRes, res := range resp.Results {
		documents := make([]*Document, len(res.Documents))
		for i, doc := range res.Documents {
			documents[i] = buildDocumentFromPB(doc)
		}
		response.Documents[nRes] = documents
	}
	return response, nil
}
