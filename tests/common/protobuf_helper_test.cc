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
 *   \author   jiliang.ljl
 *   \date     Feb 2021
 *   \brief
 */

#include "common/protobuf_helper.h"
#include <ailego/utility/time_helper.h>
#include <gmock/gmock.h>
#include <google/protobuf/util/json_util.h>
#include <gtest/gtest.h>
#include <json2pb/json_to_pb.h>
#include <json2pb/pb_to_json.h>
#include "proto/proxima_be.pb.h"
#include "test.pb.h"

namespace proxima {
namespace be {

using ::proxima::be::proto::test::ProtoTest;

proto::CollectionConfig GetMeta(const std::string &name,
                                uint64_t doc_per_segment) {
  proto::CollectionConfig meta;
  meta.set_collection_name(name);
  meta.add_forward_column_names("f1");
  meta.add_forward_column_names("f2");
  meta.set_max_docs_per_segment(doc_per_segment);
  auto cm1 = meta.add_index_column_params();
  cm1->set_column_name("column1");
  auto kv = cm1->add_extra_params();
  kv->set_key("k1");
  kv->set_value("v1");
  kv = cm1->add_extra_params();
  kv->set_key("k2");
  kv->set_value("v2");
  auto cm2 = meta.add_index_column_params();
  for (size_t i = 0; i < 10; i++) {
    kv = cm2->add_extra_params();
    kv->set_key("k" + std::to_string(i));
    kv->set_value("v" + std::to_string(i));
  }
  return meta;
}

static int ToJson(const google::protobuf::Message &response,
                  std::string *json) {
  google::protobuf::util::JsonOptions options;
  options.add_whitespace = false;
  options.always_print_primitive_fields = true;
  options.preserve_proto_field_names = true;

  google::protobuf::util::Status status =
      google::protobuf::util::MessageToJsonString(response, json, options);
  return status.error_code();
}

static std::string ToJson(const google::protobuf::Message &response) {
  std::string json;
  google::protobuf::util::JsonOptions options;
  options.add_whitespace = false;
  options.preserve_proto_field_names = true;

  google::protobuf::util::MessageToJsonString(response, &json, options);
  return json;
}

static bool ToJsonBrpc(const google::protobuf::Message &response,
                       std::string *json) {
  json2pb::Pb2JsonOptions options;
  options.enable_protobuf_map = false;
  options.bytes_to_base64 = true;
  options.always_print_primitive_fields = true;
  return json2pb::ProtoMessageToJson(response, json);
}

static void TestGoogleJsonDeserializeOk(
    const std::string &json, const google::protobuf::Message &original,
    google::protobuf::Message *back) {
  back->Clear();
  google::protobuf::util::JsonParseOptions options;
  // ignore params which can't be automatic parse from json
  options.ignore_unknown_fields = true;
  google::protobuf::util::Status status =
      google::protobuf::util::JsonStringToMessage(json, back, options);
  ASSERT_TRUE(status.ok());
  EXPECT_EQ(original.SerializeAsString(), back->SerializeAsString());
}

TEST(ProtobufHelper, Test) {
  auto meta = GetMeta("test", 100);
  std::string out;
  bool ret = ProtobufHelper::MessageToJson(meta, &out);
  printf("reflection:\n %s\n", out.c_str());
  EXPECT_TRUE(ret);
  std::string out2;
  ToJson(meta, &out2);
  printf("google:\n %s\n", out2.c_str());
  std::string out3;
  ToJsonBrpc(meta, &out3);
  printf("brpc:\n %s\n", out3.c_str());

  proto::CollectionConfig meta_back;
  TestGoogleJsonDeserializeOk(out, meta, &meta_back);
}

TEST(ProtobufHelper, TestOneof) {
  auto assert_expect = [](const proto::QueryRequest &req) {
    proto::QueryRequest back;
    std::string out;
    bool ret = ProtobufHelper::MessageToJson(req, &out);
    EXPECT_TRUE(ret);
    printf("reflection\n%s\n", out.c_str());
    std::string out2;
    ToJson(req, &out2);
    printf("google:\n%s\n", out2.c_str());
    std::string out3;
    ToJsonBrpc(req, &out3);
    printf("brpc:\n%s\n", out2.c_str());
    TestGoogleJsonDeserializeOk(out, req, &back);
  };
  // set first one of
  {
    proto::QueryRequest req;
    req.mutable_knn_param()->set_batch_count(10);
    assert_expect(req);
  }

  // set none of one of
  {
    proto::QueryRequest req;
    assert_expect(req);
  }
}

TEST(ProtobufHelper, TestOptions) {
  auto meta = GetMeta("test", 100);
  ProtobufHelper::PrintOptions options;
  options.always_print_primitive_fields = false;
  std::string out;
  bool ret = ProtobufHelper::MessageToJson(meta, options, &out);
  printf("reflection:\n %s\n", out.c_str());
  EXPECT_TRUE(ret);
  proto::CollectionConfig meta_back;
  TestGoogleJsonDeserializeOk(out, meta, &meta_back);
}

TEST(ProtobufHelper, Benchmark) {
  std::vector<proto::CollectionConfig> metas;
  const int COUNT = 10000;
  std::vector<std::string> outs1(COUNT);
  std::vector<std::string> outs2(COUNT);
  std::vector<std::string> outs3(COUNT);
  for (size_t i = 0; i < COUNT; i++) {
    metas.push_back(GetMeta("test" + std::to_string(i), i));
  }
  uint64_t time1 = 0;
  uint64_t time2 = 0;
  uint64_t time3 = 0;
  {
    ailego::ElapsedTime timer;
    for (size_t i = 0; i < COUNT; i++) {
      ProtobufHelper::MessageToJson(metas[i], &outs1[i]);
    }
    time1 = timer.micro_seconds();
  }
  {
    ailego::ElapsedTime timer;
    for (size_t i = 0; i < COUNT; i++) {
      ToJson(metas[i], &outs2[i]);
    }
    time2 = timer.micro_seconds();
  }
  {
    ailego::ElapsedTime timer;
    for (size_t i = 0; i < COUNT; i++) {
      ToJsonBrpc(metas[i], &outs3[i]);
    }
    time3 = timer.micro_seconds();
  }
  printf("reflection: %zu\n", (size_t)time1);
  printf("pb MessageToJsonString: %zu\n", (size_t)time2);
  printf("brpc: %zu\n", (size_t)time3);
}

static bool ToMessagePb(const std::string &json,
                        google::protobuf::Message *msg) {
  google::protobuf::util::JsonParseOptions options;
  // ignore params which can't be automatic parse from json
  options.ignore_unknown_fields = true;

  google::protobuf::util::Status status =
      google::protobuf::util::JsonStringToMessage(json, msg, options);
  if (!status.ok()) {
    fprintf(stderr, "ParseRequestFromJson failed. status[%s] json[%s]\n",
            status.ToString().c_str(), json.c_str());
    return false;
  }
  return true;
}

static bool ToMessageBrpc(const std::string &json,
                          google::protobuf::Message *msg,
                          std::string *error = nullptr) {
  return json2pb::JsonToProtoMessage(json, msg, error);
}

const std::string kTestJson =
    R"foo({"collection_name": "test_collection", "debug_mode": false, "knn_param": {"column_name": "test_column", "topk": 20, "batch_count": 1, "dimension": 480, "data_type": "DT_VECTOR_FP32", "is_linear": false, "matrix": "[b'[0.133300781,0.00359916687,0.0234069824,0.0513000488,0.0621032715,-0.0505065918,-0.0451049805,0,-0.00680160522,-0.0156021118,0.00349998474,0.0130996704,0.029296875,0.049987793,0.0753173828,-0.00400161743,0.00540161133,0.0786132812,0.00680160522,0.0455932617,-0.0396118164,-0.0433044434,0.0462036133,0.0588989258,-0.059387207,-0.0390014648,-0.0272979736,-0.0301055908,-0.047088623,-0.0339050293,0.00230026245,0.0523986816,-0.0598144531,-0.0748901367,-0.0380859375,-0.0624084473,0.0437927246,-0.00949859619,-0.00239944458,0.0272064209,0.0645751953,0.0313110352,0.0160980225,0.0207977295,-0.0252075195,0.0817871094,0.0969238281,0.0265045166,0.0859985352,0.000599861145,0.0362854004,-0.0130004883,-0.0443115234,0.0141983032,0.0555114746,-0.0555114746,0.0339050293,-0.0409851074,0.0327148438,-0.0440979004,0.00540161133,0.0200958252,0.103210449,-0.020401001,-0.00510025024,-0.061706543,0.0307006836,-0.0349121094,-0.000599861145,0.0853881836,-0.0477905273,0.073425293,-0.0137023926,0.0314941406,-0.0231018066,-0.00900268555,0.0349121094,-0.0225067139,0.0314941406,0.0527038574,-0.0141983032,-0.028793335,-0.0278015137,-0.069519043,0.0407104492,-0.000899791718,-0.0437927246,0.0883178711,0.0484008789,-0.0260009766,-0.054107666,-0.00709915161,-0.0142974854,-0.0654296875,0.00869750977,-0.0517883301,-0.0397949219,-0.0310058594,-0.032409668,0.0567016602,-0.0066986084,-0.0494995117,0.0204925537,-0.0290985107,0.0883789062,-0.0252075195,-0.0729980469,-0.0510864258,0.0254058838,-0.0158996582,0.000599861145,0.0349121094,-0.00759887695,-0.07421875,0.0567932129,-0.0294036865,-0.0645751953,-0.0559997559,0.0783081055,0.0150985718,0.00239944458,-0.103271484,0.00859832764,-0.0283050537,-0.00270080566,-0.0481872559,-0.0307006836,0.0314941406,-0.0234985352,-0.00340080261,0.0390014648,-0.0328063965,0.00949859619,-0.0648193359,-0.0361938477,-0.00450134277,0.0342102051,-0.0433044434,-0.0433044434,-0.0066986084,0.0513000488,-0.00479888916,0.0505981445,0.0124969482,0.0114974976,0.0408935547,0.0338134766,0.0060005188,-0.0149002075,-0.0425109863,-0.0577087402,-0.014503479,-0.00789642334,-0.00699996948,0.00450134277,0.0867919922,0.00410079956,0.0016002655,-0.00879669189,0.00680160522,-0.0544128418,-0.0581970215,0.0276031494,0.0227966309,0.065612793,-0.0187072754,0.0914916992,-0.0269927979,0.00469970703,-0.0586853027,0.0902709961,0.0441894531,-0.0775146484,-0.000400066376,-0.0563049316,0.028793335,-0.071472168,-0.0142974854,-0.000500202179,0.025604248,-0.0433044434,0.00340080261,0.0591125488,-0.025894165,0.00550079346,0.0614013672,-0.0830078125,-0.0247955322,-0.0265960693,-0.057800293,-0.0706787109,0.057800293,0,0.00239944458,-0.0440979004,-0.0955200195,0.00910186768,-0.0092010498,-0.0202026367,-0.0521850586,0.0314025879,-0.0827026367,0.0111999512,0.036895752,-0.0178985596,0.0100021362,0.0281066895,-0.000899791718,0.0979003906,0.0249023438,-0.0469970703,0.0270996094,0.0240936279,0.0787963867,-0.0407104492,0.102416992,0.0103988647,0.0853271484,0.0811767578,-0.0189971924,0.00849914551,0.00270080566,-0.0254974365,-0.0772094727,-0.032989502,-0.0157928467,-0.0932006836,-0.0225067139,0.0592041016,0.0186004639,0.0933837891,0.0247955322,-0.00609970093,0.0127029419,0.00390052795,-0.0281066895,-0.0109024048,0.0328063965,0.0599060059,0.0659790039,-0.00569915771,0.0635986328,-0.0444946289,0.0184936523,0.0405883789,0.0136032104,0.00469970703,0.0390014648,-0.033203125,-0.0592956543,-0.0285949707,0,-0.0278930664,0.0396118164,0.0880126953,-0.0234985352,0.0060005188,-0.0104980469,0.0258026123,0.0109024048,-0.0133972168,-0.0243988037,0.00490188599,-0.013999939,0.0393981934,-0.00340080261,-0.0468139648,-0.016998291,0.037902832,0.00370025635,0.0422973633,-0.0339050293,0.0180969238,-0.0104980469,0.0187072754,-0.00770187378,0.0180053711,-0.0374145508,-0.00469970703,0.0759887695,-0.0480041504,0.0218963623,-0.025100708,0.0728149414,0.114196777,-0.0169067383,-0.0859985352,-0.0249023438,-0.021697998,0.0225982666,0.0761108398,-0.0454101562,0.00930023193,0.0272064209,0.0548095703,0.0574035645,-0.0260925293,0.000899791718,-0.00250053406,-0.00820159912,-0.0200958252,0.0361022949,-0.000500202179,-0.0897827148,-0.0133972168,0.0589904785,-0.00340080261,0.0320129395,0.00419998169,0.0651245117,-0.0419006348,-0.0367126465,-0.00410079956,-0.0634155273,-0.0616149902,-0.0132980347,0.00939941406,0.053314209,-0.10748291,0.0354003906,-0.0207061768,0.00680160522,0.0411071777,0.114990234,-0.0321044922,0.00109958649,0.037902832,-0.00429916382,-0.016494751,-0.0108032227,0.00559997559,0.0827026367,0.0563049316,-0.0254974365,-0.0582885742,0.0534057617,-0.013999939,-0.0797729492,-0.00619888306,-0.0375976562,0.0585021973,-0.106506348,0.0274047852,-0.0364990234,0.0473937988,0.0892944336,0.00500106812,-0.0178070068,0.0320129395,-0.0477905273,0.0679931641,-0.0544128418,0.0502929688,0.0175018311,-0.029800415,0.0220947266,0.00239944458,0.00289916992,0.0737915039,0.0991210938,-0.0318908691,-0.0182952881,0.0184936523,0.0252075195,-0.00309944153,-0.0157012939,0.0321960449,-0.0234069824,-0.0281066895,0.0502929688,-0.00289916992,0.0476989746,0.0404968262,-0.102905273,-0.0100021362,0.0318908691,0.0715942383,-0.0822753906,-0.0108032227,-0.0659790039,0.0074005127,-0.0245056152,0.0640258789,-0.0114974976,-0.0634155273,-0.0412902832,-0.0328979492,0.0102996826,-0.100585938,-0.00939941406,-0.0294952393,-0.00890350342,-0.00419998169,-0.0301055908,-0.0204925537,0.00200080872,-0.0581970215,0.000800132751,-0.0213928223,-0.0606994629,-0.0736083984,-0.0296936035,-0.0488891602,-0.0382995605,0.044708252,0.0184936523,0.0013999939,0.0173950195,-0.0173034668,0.102478027,0.0614929199,-0.0231018066,-0.00439834595,-0.0139007568,-0.0466918945,0.0315856934,0.0805053711,-0.0222015381,-0.0132980347,0.100524902,-0.0131988525,-0.0988769531,-0.0541992188,0.016204834,0.0130996704,0.0234985352,0.0361022949,-0.033996582,-0.0130004883,-0.0498046875,-0.00550079346,0.0676879883,0.0392150879,0.0117034912,-0.0112991333,-0.0610961914,0.0325927734,-0.00510025024,-0.0770263672,0.0419006348,-0.00579833984,-0.0437927246,-0.0254058838,-0.0585021973,-0.00579833984,0.0662231445,-0.0444030762,0.0236968994,-0.0618896484,-0.0260009766,-0.00250053406,-0.0736083984,-0.0111999512,0.0491943359,0.0656738281,0.078918457,0.094909668,0.0600891113,0.0430908203,0.0155029297,0.0548095703,0.0726928711,0.00129985809,-0.0480957031,-0.0167999268,0.0313110352,0.0452880859,0.0169067383,-0.0115966797,-0.00390052795,0.0930175781,-0.00949859619,0.0357971191,0.0354919434,-0.00289916992,0.0645141602,0.119995117,0.0452880859,0.0265960693,-0.0698242188]']"}})foo";

const std::string kCamelcaseJson =
    R"foo({"collectionName": "test_collection", "debugMode": false, "knn_param": {"column_name": "test_column", "topk": 20, "batch_count": 1, "dimension": 480, "data_type": "DT_VECTOR_FP32", "is_linear": false, "matrix": "[b'[0.133300781,0.00359916687,0.0234069824,0.0513000488,0.0621032715,-0.0505065918,-0.0451049805,0,-0.00680160522,-0.0156021118,0.00349998474,0.0130996704,0.029296875,0.049987793,0.0753173828,-0.00400161743,0.00540161133,0.0786132812,0.00680160522,0.0455932617,-0.0396118164,-0.0433044434,0.0462036133,0.0588989258,-0.059387207,-0.0390014648,-0.0272979736,-0.0301055908,-0.047088623,-0.0339050293,0.00230026245,0.0523986816,-0.0598144531,-0.0748901367,-0.0380859375,-0.0624084473,0.0437927246,-0.00949859619,-0.00239944458,0.0272064209,0.0645751953,0.0313110352,0.0160980225,0.0207977295,-0.0252075195,0.0817871094,0.0969238281,0.0265045166,0.0859985352,0.000599861145,0.0362854004,-0.0130004883,-0.0443115234,0.0141983032,0.0555114746,-0.0555114746,0.0339050293,-0.0409851074,0.0327148438,-0.0440979004,0.00540161133,0.0200958252,0.103210449,-0.020401001,-0.00510025024,-0.061706543,0.0307006836,-0.0349121094,-0.000599861145,0.0853881836,-0.0477905273,0.073425293,-0.0137023926,0.0314941406,-0.0231018066,-0.00900268555,0.0349121094,-0.0225067139,0.0314941406,0.0527038574,-0.0141983032,-0.028793335,-0.0278015137,-0.069519043,0.0407104492,-0.000899791718,-0.0437927246,0.0883178711,0.0484008789,-0.0260009766,-0.054107666,-0.00709915161,-0.0142974854,-0.0654296875,0.00869750977,-0.0517883301,-0.0397949219,-0.0310058594,-0.032409668,0.0567016602,-0.0066986084,-0.0494995117,0.0204925537,-0.0290985107,0.0883789062,-0.0252075195,-0.0729980469,-0.0510864258,0.0254058838,-0.0158996582,0.000599861145,0.0349121094,-0.00759887695,-0.07421875,0.0567932129,-0.0294036865,-0.0645751953,-0.0559997559,0.0783081055,0.0150985718,0.00239944458,-0.103271484,0.00859832764,-0.0283050537,-0.00270080566,-0.0481872559,-0.0307006836,0.0314941406,-0.0234985352,-0.00340080261,0.0390014648,-0.0328063965,0.00949859619,-0.0648193359,-0.0361938477,-0.00450134277,0.0342102051,-0.0433044434,-0.0433044434,-0.0066986084,0.0513000488,-0.00479888916,0.0505981445,0.0124969482,0.0114974976,0.0408935547,0.0338134766,0.0060005188,-0.0149002075,-0.0425109863,-0.0577087402,-0.014503479,-0.00789642334,-0.00699996948,0.00450134277,0.0867919922,0.00410079956,0.0016002655,-0.00879669189,0.00680160522,-0.0544128418,-0.0581970215,0.0276031494,0.0227966309,0.065612793,-0.0187072754,0.0914916992,-0.0269927979,0.00469970703,-0.0586853027,0.0902709961,0.0441894531,-0.0775146484,-0.000400066376,-0.0563049316,0.028793335,-0.071472168,-0.0142974854,-0.000500202179,0.025604248,-0.0433044434,0.00340080261,0.0591125488,-0.025894165,0.00550079346,0.0614013672,-0.0830078125,-0.0247955322,-0.0265960693,-0.057800293,-0.0706787109,0.057800293,0,0.00239944458,-0.0440979004,-0.0955200195,0.00910186768,-0.0092010498,-0.0202026367,-0.0521850586,0.0314025879,-0.0827026367,0.0111999512,0.036895752,-0.0178985596,0.0100021362,0.0281066895,-0.000899791718,0.0979003906,0.0249023438,-0.0469970703,0.0270996094,0.0240936279,0.0787963867,-0.0407104492,0.102416992,0.0103988647,0.0853271484,0.0811767578,-0.0189971924,0.00849914551,0.00270080566,-0.0254974365,-0.0772094727,-0.032989502,-0.0157928467,-0.0932006836,-0.0225067139,0.0592041016,0.0186004639,0.0933837891,0.0247955322,-0.00609970093,0.0127029419,0.00390052795,-0.0281066895,-0.0109024048,0.0328063965,0.0599060059,0.0659790039,-0.00569915771,0.0635986328,-0.0444946289,0.0184936523,0.0405883789,0.0136032104,0.00469970703,0.0390014648,-0.033203125,-0.0592956543,-0.0285949707,0,-0.0278930664,0.0396118164,0.0880126953,-0.0234985352,0.0060005188,-0.0104980469,0.0258026123,0.0109024048,-0.0133972168,-0.0243988037,0.00490188599,-0.013999939,0.0393981934,-0.00340080261,-0.0468139648,-0.016998291,0.037902832,0.00370025635,0.0422973633,-0.0339050293,0.0180969238,-0.0104980469,0.0187072754,-0.00770187378,0.0180053711,-0.0374145508,-0.00469970703,0.0759887695,-0.0480041504,0.0218963623,-0.025100708,0.0728149414,0.114196777,-0.0169067383,-0.0859985352,-0.0249023438,-0.021697998,0.0225982666,0.0761108398,-0.0454101562,0.00930023193,0.0272064209,0.0548095703,0.0574035645,-0.0260925293,0.000899791718,-0.00250053406,-0.00820159912,-0.0200958252,0.0361022949,-0.000500202179,-0.0897827148,-0.0133972168,0.0589904785,-0.00340080261,0.0320129395,0.00419998169,0.0651245117,-0.0419006348,-0.0367126465,-0.00410079956,-0.0634155273,-0.0616149902,-0.0132980347,0.00939941406,0.053314209,-0.10748291,0.0354003906,-0.0207061768,0.00680160522,0.0411071777,0.114990234,-0.0321044922,0.00109958649,0.037902832,-0.00429916382,-0.016494751,-0.0108032227,0.00559997559,0.0827026367,0.0563049316,-0.0254974365,-0.0582885742,0.0534057617,-0.013999939,-0.0797729492,-0.00619888306,-0.0375976562,0.0585021973,-0.106506348,0.0274047852,-0.0364990234,0.0473937988,0.0892944336,0.00500106812,-0.0178070068,0.0320129395,-0.0477905273,0.0679931641,-0.0544128418,0.0502929688,0.0175018311,-0.029800415,0.0220947266,0.00239944458,0.00289916992,0.0737915039,0.0991210938,-0.0318908691,-0.0182952881,0.0184936523,0.0252075195,-0.00309944153,-0.0157012939,0.0321960449,-0.0234069824,-0.0281066895,0.0502929688,-0.00289916992,0.0476989746,0.0404968262,-0.102905273,-0.0100021362,0.0318908691,0.0715942383,-0.0822753906,-0.0108032227,-0.0659790039,0.0074005127,-0.0245056152,0.0640258789,-0.0114974976,-0.0634155273,-0.0412902832,-0.0328979492,0.0102996826,-0.100585938,-0.00939941406,-0.0294952393,-0.00890350342,-0.00419998169,-0.0301055908,-0.0204925537,0.00200080872,-0.0581970215,0.000800132751,-0.0213928223,-0.0606994629,-0.0736083984,-0.0296936035,-0.0488891602,-0.0382995605,0.044708252,0.0184936523,0.0013999939,0.0173950195,-0.0173034668,0.102478027,0.0614929199,-0.0231018066,-0.00439834595,-0.0139007568,-0.0466918945,0.0315856934,0.0805053711,-0.0222015381,-0.0132980347,0.100524902,-0.0131988525,-0.0988769531,-0.0541992188,0.016204834,0.0130996704,0.0234985352,0.0361022949,-0.033996582,-0.0130004883,-0.0498046875,-0.00550079346,0.0676879883,0.0392150879,0.0117034912,-0.0112991333,-0.0610961914,0.0325927734,-0.00510025024,-0.0770263672,0.0419006348,-0.00579833984,-0.0437927246,-0.0254058838,-0.0585021973,-0.00579833984,0.0662231445,-0.0444030762,0.0236968994,-0.0618896484,-0.0260009766,-0.00250053406,-0.0736083984,-0.0111999512,0.0491943359,0.0656738281,0.078918457,0.094909668,0.0600891113,0.0430908203,0.0155029297,0.0548095703,0.0726928711,0.00129985809,-0.0480957031,-0.0167999268,0.0313110352,0.0452880859,0.0169067383,-0.0115966797,-0.00390052795,0.0930175781,-0.00949859619,0.0357971191,0.0354919434,-0.00289916992,0.0645141602,0.119995117,0.0452880859,0.0265960693,-0.0698242188]']"}})foo";

TEST(ProtobufHelper, DesJson) {
  for (auto json : {kTestJson, kCamelcaseJson}) {
    proxima::be::proto::QueryRequest pb_req;
    EXPECT_TRUE(ToMessagePb(kTestJson, &pb_req));

    proxima::be::proto::QueryRequest brpc_req;
    std::string error;
    EXPECT_TRUE(ToMessageBrpc(kTestJson, &brpc_req, &error));
    EXPECT_EQ(error, "");

    proxima::be::proto::QueryRequest se_req;
    EXPECT_TRUE(ProtobufHelper::JsonToMessage(kTestJson, &se_req));

    EXPECT_EQ(pb_req.SerializeAsString(), se_req.SerializeAsString());
    EXPECT_EQ(pb_req.DebugString(), se_req.DebugString());
    // brpc does not support camel case
    //    EXPECT_EQ(pb_req.DebugString(), brpc_req.DebugString());
    //    EXPECT_EQ(pb_req.SerializeAsString(), brpc_req.SerializeAsString());
  }
}

TEST(ProtobufHelper, DesJsonBench) {
  const int COUNT = 10000;
  std::vector<proto::QueryRequest> pb_reqs(COUNT);
  std::vector<proto::QueryRequest> brpc_reqs(COUNT);
  std::vector<proto::QueryRequest> se_reqs(COUNT);
  uint64_t time_pb = 0;
  uint64_t time_brpc = 0;
  uint64_t time_se = 0;
  {
    ailego::ElapsedTime timer;
    for (size_t i = 0; i < COUNT; i++) {
      ToMessagePb(kTestJson, &pb_reqs[i]);
    }
    time_pb = timer.micro_seconds();
  }
  {
    ailego::ElapsedTime timer;
    for (size_t i = 0; i < COUNT; i++) {
      ToMessageBrpc(kTestJson, &brpc_reqs[i]);
    }
    time_brpc = timer.micro_seconds();
  }
  {
    ailego::ElapsedTime timer;
    for (size_t i = 0; i < COUNT; i++) {
      ProtobufHelper::JsonToMessage(kTestJson, &se_reqs[i]);
    }
    time_se = timer.micro_seconds();
  }
  printf("be: %zu\n", (size_t)time_se);
  printf("pb: %zu\n", (size_t)time_pb);
  printf("brpc: %zu\n", (size_t)time_brpc);
}

TEST(ProtobufHelperDesJson, int32) {
  ProtoTest t;
  EXPECT_TRUE(ProtobufHelper::JsonToMessage(R"({"i32":1})", &t));
  EXPECT_EQ(t.i32(), 1);
  EXPECT_TRUE(ProtobufHelper::JsonToMessage(
      R"({"i32":)" + std::to_string(std::numeric_limits<int32_t>::max()) + "}",
      &t));
  EXPECT_EQ(t.i32(), std::numeric_limits<int32_t>::max());
  EXPECT_TRUE(ProtobufHelper::JsonToMessage(
      R"({"i32":)" + std::to_string(std::numeric_limits<int32_t>::min()) + "}",
      &t));
  EXPECT_EQ(t.i32(), std::numeric_limits<int32_t>::min());
  EXPECT_TRUE(ProtobufHelper::JsonToMessage(R"({"i32":null})", &t));
  EXPECT_EQ(t.i32(), 0);

  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"i32":1.0})", &t));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"i32":"1"})", &t));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"i32":[1]})", &t));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"i32":{"k":"v"}})", &t));
}

TEST(ProtobufHelperDesJson, uint32) {
  ProtoTest t;
  EXPECT_TRUE(ProtobufHelper::JsonToMessage(R"({"u32":1})", &t));
  EXPECT_EQ(t.u32(), 1);
  EXPECT_TRUE(ProtobufHelper::JsonToMessage(
      R"({"u32":)" + std::to_string(std::numeric_limits<uint32_t>::max()) + "}",
      &t));
  EXPECT_EQ(t.u32(), std::numeric_limits<uint32_t>::max());
  EXPECT_TRUE(ProtobufHelper::JsonToMessage(
      R"({"u32":)" + std::to_string(std::numeric_limits<uint32_t>::min()) + "}",
      &t));
  EXPECT_EQ(t.u32(), std::numeric_limits<uint32_t>::min());
  EXPECT_TRUE(ProtobufHelper::JsonToMessage(R"({"u32":null})", &t));
  EXPECT_EQ(t.u32(), 0);

  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"u32":1.0})", &t));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"u32":"1"})", &t));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"u32":[1]})", &t));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"u32":{"k":"v"}})", &t));
}

TEST(ProtobufHelperDesJson, int64) {
  ProtoTest t;
  EXPECT_TRUE(ProtobufHelper::JsonToMessage(R"({"i64":1})", &t));
  EXPECT_EQ(t.i64(), 1);
  EXPECT_TRUE(ProtobufHelper::JsonToMessage(
      R"({"i64":)" + std::to_string(std::numeric_limits<int64_t>::max()) + "}",
      &t));
  EXPECT_EQ(t.i64(), std::numeric_limits<int64_t>::max());
  EXPECT_TRUE(ProtobufHelper::JsonToMessage(
      R"({"i64":)" + std::to_string(std::numeric_limits<int64_t>::min()) + "}",
      &t));
  EXPECT_EQ(t.i64(), std::numeric_limits<int64_t>::min());
  EXPECT_TRUE(ProtobufHelper::JsonToMessage(R"({"i64":null})", &t));
  EXPECT_EQ(t.i64(), 0);

  // test string
  EXPECT_TRUE(ProtobufHelper::JsonToMessage(R"({"i64":"1"})", &t));
  EXPECT_EQ(t.i64(), 1);
  EXPECT_TRUE(ProtobufHelper::JsonToMessage(
      R"({"i64":")" + std::to_string(std::numeric_limits<int64_t>::max()) +
          "\"}",
      &t));
  EXPECT_EQ(t.i64(), std::numeric_limits<int64_t>::max());
  EXPECT_TRUE(ProtobufHelper::JsonToMessage(
      R"({"i64":")" + std::to_string(std::numeric_limits<int64_t>::min()) +
          "\"}",
      &t));
  EXPECT_EQ(t.i64(), std::numeric_limits<int64_t>::min());

  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"i64":1.0})", &t));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"i64":[1]})", &t));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"i64":{"k":"v"}})", &t));
}

TEST(ProtobufHelperDesJson, uint64) {
  ProtoTest t;
  EXPECT_TRUE(ProtobufHelper::JsonToMessage(R"({"u64":1})", &t));
  EXPECT_EQ(t.u64(), 1);
  EXPECT_TRUE(ProtobufHelper::JsonToMessage(
      R"({"u64":)" + std::to_string(std::numeric_limits<uint64_t>::max()) + "}",
      &t));
  EXPECT_EQ(t.u64(), std::numeric_limits<uint64_t>::max());
  EXPECT_TRUE(ProtobufHelper::JsonToMessage(
      R"({"u64":)" + std::to_string(std::numeric_limits<uint64_t>::min()) + "}",
      &t));
  EXPECT_EQ(t.u64(), std::numeric_limits<uint64_t>::min());
  EXPECT_TRUE(ProtobufHelper::JsonToMessage(R"({"u64":null})", &t));
  EXPECT_EQ(t.u64(), 0);

  // test string
  EXPECT_TRUE(ProtobufHelper::JsonToMessage(R"({"u64":"1"})", &t));
  EXPECT_EQ(t.u64(), 1);
  EXPECT_TRUE(ProtobufHelper::JsonToMessage(
      R"({"u64":")" + std::to_string(std::numeric_limits<uint64_t>::max()) +
          "\"}",
      &t));
  EXPECT_EQ(t.u64(), std::numeric_limits<uint64_t>::max());
  EXPECT_TRUE(ProtobufHelper::JsonToMessage(
      R"({"u64":")" + std::to_string(std::numeric_limits<uint64_t>::min()) +
          "\"}",
      &t));
  EXPECT_EQ(t.u64(), std::numeric_limits<uint64_t>::min());

  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"u64":1.0})", &t));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"u64":[1]})", &t));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"u64":{"k":"v"}})", &t));
}

static void FloatMatch(const ProtoTest &s) {
  ProtoTest d;
  std::cout << ToJson(s) << std::endl;
  EXPECT_TRUE(ProtobufHelper::JsonToMessage(ToJson(s), &d));
  EXPECT_FLOAT_EQ(d.f32(), s.f32());
}

static void FloatNan(const ProtoTest &s) {
  ProtoTest d;
  std::cout << ToJson(s) << std::endl;
  EXPECT_TRUE(ProtobufHelper::JsonToMessage(ToJson(s), &d));
  EXPECT_TRUE(std::isnan(d.f32()));
}

TEST(ProtobufHelperDesJson, float) {
  ProtoTest s;
  FloatMatch(s);

  s.set_f32(1.0);
  FloatMatch(s);

  s.set_f32(std::numeric_limits<float>::max());
  FloatMatch(s);
  s.set_f32(-std::numeric_limits<float>::max());
  FloatMatch(s);

  s.set_f32(std::numeric_limits<float>::min());
  FloatMatch(s);
  s.set_f32(-std::numeric_limits<float>::min());
  FloatMatch(s);

  s.set_f32(std::numeric_limits<float>::infinity());
  FloatMatch(s);
  s.set_f32(-std::numeric_limits<float>::infinity());
  FloatMatch(s);

  s.set_f32(std::numeric_limits<float>::quiet_NaN());
  FloatNan(s);
  s.set_f32(-std::numeric_limits<float>::quiet_NaN());
  FloatNan(s);

  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"f32":"1"})", &s));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"f32":[1]})", &s));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"f32":{"k":"v"}})", &s));
}

static void DoubleMatch(const ProtoTest &s) {
  ProtoTest d;
  std::cout << ToJson(s) << std::endl;
  EXPECT_TRUE(ProtobufHelper::JsonToMessage(ToJson(s), &d));
  EXPECT_FLOAT_EQ(d.f64(), s.f64());
}

static void DoubleNan(const ProtoTest &s) {
  ProtoTest d;
  std::cout << ToJson(s) << std::endl;
  EXPECT_TRUE(ProtobufHelper::JsonToMessage(ToJson(s), &d));
  EXPECT_TRUE(std::isnan(d.f64()));
}

TEST(ProtobufHelperDesJson, double) {
  ProtoTest s;
  DoubleMatch(s);

  s.set_f64(1.0);
  DoubleMatch(s);

  s.set_f64(std::numeric_limits<double>::max());
  DoubleMatch(s);
  s.set_f64(-std::numeric_limits<double>::max());
  DoubleMatch(s);

  s.set_f64(std::numeric_limits<double>::min());
  DoubleMatch(s);
  s.set_f64(-std::numeric_limits<double>::min());
  DoubleMatch(s);

  s.set_f64(std::numeric_limits<double>::infinity());
  DoubleMatch(s);
  s.set_f64(-std::numeric_limits<double>::infinity());
  DoubleMatch(s);

  s.set_f64(std::numeric_limits<double>::quiet_NaN());
  DoubleNan(s);
  s.set_f64(-std::numeric_limits<double>::quiet_NaN());
  DoubleNan(s);

  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"f64":"1"})", &s));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"f64":[1]})", &s));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"f64":{"k":"v"}})", &s));
}

static void BoolMatch(const ProtoTest &s) {
  ProtoTest d;
  EXPECT_TRUE(ProtobufHelper::JsonToMessage(ToJson(s), &d));
  EXPECT_EQ(d.b(), s.b());
}

TEST(ProtobufHelperDesJson, bool) {
  ProtoTest s;
  BoolMatch(s);

  s.set_b(true);
  BoolMatch(s);

  s.set_b(false);
  BoolMatch(s);

  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"b":1})", &s));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"b":1.0})", &s));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"b":"1"})", &s));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"b":[1]})", &s));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"b":{"k":"v"}})", &s));
}

static void EnumMatch(const ProtoTest &s) {
  ProtoTest d;
  EXPECT_TRUE(ProtobufHelper::JsonToMessage(ToJson(s), &d));
  EXPECT_EQ(d.e(), s.e());
}

TEST(ProtobufHelperDesJson, enum) {
  ProtoTest s;
  EnumMatch(s);

  s.set_e(proto::test::ProtoTest_Enum_MON);
  EnumMatch(s);

  EXPECT_TRUE(ProtobufHelper::JsonToMessage(R"({"e":1})", &s));
  EXPECT_EQ(s.e(), proto::test::ProtoTest_Enum_TUE);

  EXPECT_TRUE(ProtobufHelper::JsonToMessage(R"({"e":"TUE"})", &s));
  EXPECT_EQ(s.e(), proto::test::ProtoTest_Enum_TUE);

  // invalid value
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"e":10000})", &s));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"e":"Invalid"})", &s));

  // invalid type
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"e":1.0})", &s));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"e":"1"})", &s));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"e":[1]})", &s));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"e":{"k":"v"}})", &s));
}

static void MessageMatch(const ProtoTest &s) {
  ProtoTest d;
  EXPECT_TRUE(ProtobufHelper::JsonToMessage(ToJson(s), &d));
  EXPECT_EQ(d.nest().i32(), s.nest().i32());
  EXPECT_EQ(d.nest().u32(), s.nest().u32());
}

TEST(ProtobufHelperDesJson, Message) {
  ProtoTest s;
  MessageMatch(s);

  s.mutable_nest()->set_i32(42);
  MessageMatch(s);

  s.mutable_nest()->set_i32(42);
  s.mutable_nest()->set_u32(2);
  MessageMatch(s);

  s.mutable_nest()->set_i32(std::numeric_limits<int32_t>::max());
  s.mutable_nest()->set_u32(std::numeric_limits<uint32_t>::min());
  MessageMatch(s);

  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"nest":1})", &s));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"nest":1.0})", &s));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"nest":"1"})", &s));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"nest":[1]})", &s));
}

static void StringMatch(const ProtoTest &s) {
  ProtoTest d;
  EXPECT_TRUE(ProtobufHelper::JsonToMessage(ToJson(s), &d));
  EXPECT_EQ(d.str(), s.str());
}

TEST(ProtobufHelperDesJson, string) {
  ProtoTest s;
  StringMatch(s);

  s.set_str("42");
  StringMatch(s);

  s.set_str("long long long long long long long long long long long long ago");
  StringMatch(s);

  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"str":1})", &s));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"str":1.0})", &s));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"str":[1]})", &s));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"str":{"k":"v"}})", &s));
}

static void BinaryMatch(const ProtoTest &s) {
  ProtoTest d;
  std::cout << ToJson(s) << std::endl;
  EXPECT_TRUE(ProtobufHelper::JsonToMessage(ToJson(s), &d));
  EXPECT_EQ(d.binary(), s.binary());
}

TEST(ProtobufHelperDesJson, binary) {
  ProtoTest s;
  BinaryMatch(s);

  s.set_binary("42");
  BinaryMatch(s);

  s.set_binary(
      "long long long long long long long long long long long long ago");
  BinaryMatch(s);

  float v = 42.0;
  s.set_binary(reinterpret_cast<char *>(&v), sizeof(v));
  BinaryMatch(s);

  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"binary":1})", &s));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"binary":1.0})", &s));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"binary":[1]})", &s));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"binary":{"k":"v"}})", &s));
}

static void Int32ArrayMatch(const ProtoTest &s) {
  ProtoTest d;
  std::cout << ToJson(s) << std::endl;
  EXPECT_TRUE(ProtobufHelper::JsonToMessage(ToJson(s), &d));
  EXPECT_EQ(d.i32s_size(), s.i32s_size());
  for (int i = 0; i < d.i32s_size(); i++) {
    EXPECT_EQ(d.i32s(i), s.i32s(i));
  }
}

TEST(ProtobufHelperDesJson, Int32Array) {
  ProtoTest s;
  Int32ArrayMatch(s);

  s.add_i32s(42);
  Int32ArrayMatch(s);

  s.add_i32s(std::numeric_limits<int32_t>::max());
  Int32ArrayMatch(s);

  s.add_i32s(std::numeric_limits<int32_t>::min());
  Int32ArrayMatch(s);

  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"i32s":1})", &s));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"i32s":1.0})", &s));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"i32s":"string"})", &s));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"i32s":["string"}])", &s));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"i32s":[1,2,3.0])", &s));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"i32s":{"k":"v"}})", &s));
}

static void Uint32ArrayMatch(const ProtoTest &s) {
  ProtoTest d;
  std::cout << ToJson(s) << std::endl;
  EXPECT_TRUE(ProtobufHelper::JsonToMessage(ToJson(s), &d));
  EXPECT_EQ(d.u32s_size(), s.u32s_size());
  for (int i = 0; i < d.u32s_size(); i++) {
    EXPECT_EQ(d.u32s(i), s.u32s(i));
  }
}

TEST(ProtobufHelperDesJson, Uint32Array) {
  ProtoTest s;
  Uint32ArrayMatch(s);

  s.add_u32s(42);
  Uint32ArrayMatch(s);

  s.add_u32s(std::numeric_limits<uint32_t>::max());
  Uint32ArrayMatch(s);

  s.add_u32s(std::numeric_limits<uint32_t>::min());
  Uint32ArrayMatch(s);

  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"u32s":1})", &s));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"u32s":1.0})", &s));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"u32s":"string"})", &s));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"u32s":["string"}])", &s));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"u32s":[1,2,3.0])", &s));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"u32s":{"k":"v"}})", &s));
}

static void Int64ArrayMatch(const ProtoTest &s) {
  ProtoTest d;
  std::cout << ToJson(s) << std::endl;
  EXPECT_TRUE(ProtobufHelper::JsonToMessage(ToJson(s), &d));
  EXPECT_EQ(d.i64s_size(), s.i64s_size());
  for (int i = 0; i < d.i64s_size(); i++) {
    EXPECT_EQ(d.i64s(i), s.i64s(i));
  }
}

TEST(ProtobufHelperDesJson, Int64Array) {
  ProtoTest s;
  Int64ArrayMatch(s);

  s.add_i64s(42);
  Int64ArrayMatch(s);

  s.add_i64s(std::numeric_limits<int64_t>::max());
  Int64ArrayMatch(s);

  s.add_i64s(std::numeric_limits<int64_t>::min());
  Int64ArrayMatch(s);

  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"i64s":1})", &s));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"i64s":1.0})", &s));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"i64s":"string"})", &s));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"i64s":["string"}])", &s));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"i64s":[1,2,3.0])", &s));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"i64s":{"k":"v"}})", &s));
}

static void Uint64ArrayMatch(const ProtoTest &s) {
  ProtoTest d;
  std::cout << ToJson(s) << std::endl;
  EXPECT_TRUE(ProtobufHelper::JsonToMessage(ToJson(s), &d));
  EXPECT_EQ(d.u64s_size(), s.u64s_size());
  for (int i = 0; i < d.u64s_size(); i++) {
    EXPECT_EQ(d.u64s(i), s.u64s(i));
  }
}

TEST(ProtobufHelperDesJson, Uint64Array) {
  ProtoTest s;
  Uint64ArrayMatch(s);

  s.add_u64s(42);
  Uint64ArrayMatch(s);

  s.add_u64s(std::numeric_limits<uint64_t>::max());
  Uint64ArrayMatch(s);

  s.add_u64s(std::numeric_limits<uint64_t>::min());
  Uint64ArrayMatch(s);

  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"u64s":1})", &s));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"u64s":1.0})", &s));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"u64s":"string"})", &s));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"u64s":["string"}])", &s));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"u64s":[1,2,3.0])", &s));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"u64s":{"k":"v"}})", &s));
}

static void FloatArrayMatch(const ProtoTest &s) {
  ProtoTest d;
  std::cout << ToJson(s) << std::endl;
  EXPECT_TRUE(ProtobufHelper::JsonToMessage(ToJson(s), &d));
  EXPECT_EQ(d.f32s_size(), s.f32s_size());
  for (int i = 0; i < d.f32s_size(); i++) {
    EXPECT_THAT(d.f32s(i), testing::NanSensitiveFloatEq(s.f32s(i)));
  }
}

TEST(ProtobufHelperDesJson, FloatArray) {
  ProtoTest s;
  FloatArrayMatch(s);

  s.add_f32s(42);
  FloatArrayMatch(s);

  s.add_f32s(std::numeric_limits<float>::max());
  FloatArrayMatch(s);
  s.add_f32s(-std::numeric_limits<float>::max());
  FloatArrayMatch(s);

  s.add_f32s(std::numeric_limits<float>::min());
  FloatArrayMatch(s);
  s.add_f32s(-std::numeric_limits<float>::min());
  FloatArrayMatch(s);

  s.add_f32s(std::numeric_limits<float>::infinity());
  FloatArrayMatch(s);
  s.add_f32s(-std::numeric_limits<float>::infinity());
  FloatArrayMatch(s);

  s.add_f32s(std::numeric_limits<float>::quiet_NaN());
  FloatArrayMatch(s);
  s.add_f32s(-std::numeric_limits<float>::quiet_NaN());
  FloatArrayMatch(s);

  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"f32s":1})", &s));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"f32s":1.0})", &s));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"f32s":"string"})", &s));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"f32s":["string"}])", &s));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"f32s":[1,2,[]])", &s));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"f32s":{"k":"v"}})", &s));
}

static void DoubleArrayMatch(const ProtoTest &s) {
  ProtoTest d;
  std::cout << ToJson(s) << std::endl;
  EXPECT_TRUE(ProtobufHelper::JsonToMessage(ToJson(s), &d));
  EXPECT_EQ(d.f64s_size(), s.f64s_size());
  for (int i = 0; i < d.f64s_size(); i++) {
    EXPECT_THAT(d.f64s(i), testing::NanSensitiveDoubleEq(s.f64s(i)));
  }
}

TEST(ProtobufHelperDesJson, DoubleArray) {
  ProtoTest s;
  DoubleArrayMatch(s);

  s.add_f64s(42);
  DoubleArrayMatch(s);

  s.add_f64s(std::numeric_limits<float>::max());
  DoubleArrayMatch(s);
  s.add_f64s(-std::numeric_limits<float>::max());
  DoubleArrayMatch(s);

  s.add_f64s(std::numeric_limits<float>::min());
  DoubleArrayMatch(s);
  s.add_f64s(-std::numeric_limits<float>::min());
  DoubleArrayMatch(s);

  s.add_f64s(std::numeric_limits<float>::infinity());
  DoubleArrayMatch(s);
  s.add_f64s(-std::numeric_limits<float>::infinity());
  DoubleArrayMatch(s);

  s.add_f64s(std::numeric_limits<float>::quiet_NaN());
  DoubleArrayMatch(s);
  s.add_f64s(-std::numeric_limits<float>::quiet_NaN());
  DoubleArrayMatch(s);

  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"f64s":1})", &s));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"f64s":1.0})", &s));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"f64s":"string"})", &s));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"f64s":["string"}])", &s));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"f64s":[1,2,[]])", &s));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"f64s":{"k":"v"}})", &s));
}

static void BoolArrayMatch(const ProtoTest &s) {
  ProtoTest d;
  std::cout << ToJson(s) << std::endl;
  EXPECT_TRUE(ProtobufHelper::JsonToMessage(ToJson(s), &d));
  EXPECT_EQ(d.bs_size(), s.bs_size());
  for (int i = 0; i < d.bs_size(); i++) {
    EXPECT_EQ(d.bs(i), s.bs(i));
  }
}

TEST(ProtobufHelperDesJson, BoolArray) {
  ProtoTest s;
  BoolArrayMatch(s);

  s.add_bs(42);
  BoolArrayMatch(s);

  s.add_bs(true);
  BoolArrayMatch(s);

  s.add_bs(false);
  BoolArrayMatch(s);

  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"bs":1})", &s));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"bs":1.0})", &s));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"bs":"string"})", &s));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"bs":["string"}])", &s));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"bs":[1,2,3.0])", &s));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"bs":{"k":"v"}})", &s));
}

static void EnumArrayMatch(const ProtoTest &s) {
  ProtoTest d;
  std::cout << ToJson(s) << std::endl;
  EXPECT_TRUE(ProtobufHelper::JsonToMessage(ToJson(s), &d));
  EXPECT_EQ(d.es_size(), s.es_size());
  for (int i = 0; i < d.es_size(); i++) {
    EXPECT_EQ(d.es(i), s.es(i));
  }
}

TEST(ProtobufHelperDesJson, EnumArray) {
  ProtoTest s;
  EnumArrayMatch(s);

  s.add_es(proto::test::ProtoTest_Enum_TUE);
  EnumArrayMatch(s);

  s.add_es(proto::test::ProtoTest_Enum_MON);
  EnumArrayMatch(s);

  s.add_es(proto::test::ProtoTest_Enum_SUN);
  EnumArrayMatch(s);

  EXPECT_TRUE(ProtobufHelper::JsonToMessage(R"({"es":["TUE"]})", &s));
  EXPECT_EQ(s.es_size(), 1);
  EXPECT_EQ(s.es(0), proto::test::ProtoTest_Enum_TUE);

  EXPECT_TRUE(ProtobufHelper::JsonToMessage(R"({"es":["TUE", "MON"]})", &s));
  EXPECT_EQ(s.es_size(), 2);
  EXPECT_EQ(s.es(0), proto::test::ProtoTest_Enum_TUE);
  EXPECT_EQ(s.es(1), proto::test::ProtoTest_Enum_MON);

  EXPECT_TRUE(ProtobufHelper::JsonToMessage(R"({"es":["TUE", 0]})", &s));
  EXPECT_EQ(s.es_size(), 2);
  EXPECT_EQ(s.es(0), proto::test::ProtoTest_Enum_TUE);
  EXPECT_EQ(s.es(1), proto::test::ProtoTest_Enum_MON);

  EXPECT_TRUE(ProtobufHelper::JsonToMessage(R"({"es":[1, 0]})", &s));
  EXPECT_EQ(s.es_size(), 2);
  EXPECT_EQ(s.es(0), proto::test::ProtoTest_Enum_TUE);
  EXPECT_EQ(s.es(1), proto::test::ProtoTest_Enum_MON);

  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"es":1})", &s));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"es":1.0})", &s));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"es":"string"})", &s));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"es":["string"}])", &s));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"es":[1,2,3.0])", &s));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"es":{"k":"v"}})", &s));
}

static void MessageArrayMatch(const ProtoTest &s) {
  ProtoTest d;
  std::cout << ToJson(s) << std::endl;
  EXPECT_TRUE(ProtobufHelper::JsonToMessage(ToJson(s), &d));
  EXPECT_EQ(d.nests_size(), s.nests_size());
  for (int i = 0; i < d.nests_size(); i++) {
    EXPECT_EQ(d.nests(i).i32(), s.nests(i).i32());
    EXPECT_EQ(d.nests(i).u32(), s.nests(i).u32());
  }
}

TEST(ProtobufHelperDesJson, MessageArray) {
  ProtoTest s;
  MessageArrayMatch(s);

  auto *n = s.add_nests();
  MessageArrayMatch(s);

  n = s.add_nests();
  n->set_i32(432);
  n->set_u32(234);
  MessageArrayMatch(s);

  n = s.add_nests();
  n->set_i32(std::numeric_limits<int32_t>::min());
  n->set_u32(std::numeric_limits<uint32_t>::max());
  MessageArrayMatch(s);

  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"nests":1})", &s));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"nests":1.0})", &s));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"nests":"string"})", &s));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"nests":["string"}])", &s));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"nests":[1,2,3.0])", &s));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"nests":{"k":"v"}})", &s));
  EXPECT_FALSE(
      ProtobufHelper::JsonToMessage(R"({"nests":[1, {"k":"v"}]})", &s));
}

static void StringArrayMatch(const ProtoTest &s) {
  ProtoTest d;
  std::cout << ToJson(s) << std::endl;
  EXPECT_TRUE(ProtobufHelper::JsonToMessage(ToJson(s), &d));
  EXPECT_EQ(d.strs_size(), s.strs_size());
  for (int i = 0; i < d.strs_size(); i++) {
    EXPECT_EQ(d.strs(i), s.strs(i));
  }
}

TEST(ProtobufHelperDesJson, StringArray) {
  ProtoTest s;
  StringArrayMatch(s);

  s.add_strs("42");
  StringArrayMatch(s);

  s.add_strs("long long long long long long long long long long long long ago");
  StringArrayMatch(s);

  s.add_strs("");
  StringArrayMatch(s);

  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"strs":1})", &s));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"strs":1.0})", &s));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"strs":"string"})", &s));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"strs":["string", 1}])", &s));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"strs":[1,2,3.0])", &s));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"strs":{"k":"v"}})", &s));
}

static void BinaryArrayMatch(const ProtoTest &s) {
  ProtoTest d;
  std::cout << ToJson(s) << std::endl;
  EXPECT_TRUE(ProtobufHelper::JsonToMessage(ToJson(s), &d));
  EXPECT_EQ(d.binarys_size(), s.binarys_size());
  for (int i = 0; i < d.binarys_size(); i++) {
    EXPECT_EQ(d.binarys(i), s.binarys(i));
  }
}

TEST(ProtobufHelperDesJson, BinaryArray) {
  ProtoTest s;
  BinaryArrayMatch(s);

  s.add_binarys("42");
  BinaryArrayMatch(s);

  s.add_binarys(
      "long long long long long long long long long long long long ago");
  BinaryArrayMatch(s);

  s.add_binarys("");
  BinaryArrayMatch(s);

  float v = 42.0;
  s.add_binarys(reinterpret_cast<char *>(&v), sizeof(v));

  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"binarys":1})", &s));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"binarys":1.0})", &s));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"binarys":"string"})", &s));
  EXPECT_FALSE(
      ProtobufHelper::JsonToMessage(R"({"binarys":["string", 1}])", &s));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"binarys":[1,2,3.0])", &s));
  EXPECT_FALSE(ProtobufHelper::JsonToMessage(R"({"binarys":{"k":"v"}})", &s));
}

static void AllMatch(const ProtoTest &s) {
  ProtoTest d;
  std::cout << ToJson(s) << std::endl;
  EXPECT_TRUE(ProtobufHelper::JsonToMessage(ToJson(s), &d));
  EXPECT_EQ(d.SerializeAsString(), s.SerializeAsString());
}

TEST(ProtobufHelperDesJson, All) {
  ProtoTest s;
  AllMatch(s);

  s.set_i32(42);
  AllMatch(s);

  s.set_u32(42);
  AllMatch(s);

  s.set_i64(42);
  AllMatch(s);

  s.set_u64(42);
  AllMatch(s);

  s.set_b(true);
  AllMatch(s);

  s.set_f32(42.0);
  AllMatch(s);

  s.set_f64(42.0);
  AllMatch(s);

  s.set_e(proto::test::ProtoTest_Enum_TUE);
  AllMatch(s);

  s.set_str("42");
  AllMatch(s);

  s.set_binary("42");
  AllMatch(s);

  s.mutable_nest()->set_i32(42);
  AllMatch(s);

  s.mutable_nest()->set_u32(42);
  AllMatch(s);

  s.add_i32s(42);
  AllMatch(s);

  s.add_i32s(42);
  AllMatch(s);

  s.add_u32s(42);
  AllMatch(s);

  s.add_u32s(42);
  AllMatch(s);

  s.add_i64s(42);
  AllMatch(s);

  s.add_i64s(42);
  AllMatch(s);

  s.add_u64s(42);
  AllMatch(s);

  s.add_u64s(42);
  AllMatch(s);

  s.add_bs(true);
  AllMatch(s);

  s.add_bs(false);
  AllMatch(s);

  s.add_f32s(42);
  AllMatch(s);

  s.add_f32s(42);
  AllMatch(s);

  s.add_f64s(42);
  AllMatch(s);

  s.add_f64s(42);
  AllMatch(s);

  s.add_es(proto::test::ProtoTest_Enum_TUE);
  AllMatch(s);

  s.add_es(proto::test::ProtoTest_Enum_MON);
  AllMatch(s);

  s.add_nests()->set_i32(42);
  AllMatch(s);

  s.add_nests()->set_u32(42);
  AllMatch(s);

  s.add_strs("42");
  AllMatch(s);

  s.add_strs("42");
  AllMatch(s);

  s.add_binarys("42");
  AllMatch(s);

  s.add_binarys(
      "long long long long long long long long long long long long ago");
  AllMatch(s);
}

TEST(ProtobufHelperDesJson, Options) {
  ProtoTest s;
  EXPECT_FALSE(
      ProtobufHelper::JsonToMessage(R"({"i32":42,"non_exist":[]})", &s));

  ProtobufHelper::JsonParseOptions opt;
  opt.ignore_unknown_fields = true;
  EXPECT_TRUE(
      ProtobufHelper::JsonToMessage(R"({"i32":42,"non_exist":[]})", opt, &s));
  EXPECT_EQ(s.i32(), 42);
}

}  // namespace be
}  // namespace proxima
