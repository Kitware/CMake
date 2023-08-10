// Copyright 2019 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "json_serializer.h"

#include "dap/typeinfo.h"
#include "dap/typeof.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace dap {

struct JSONInnerTestObject {
  integer i;
};

DAP_STRUCT_TYPEINFO(JSONInnerTestObject,
                    "json-inner-test-object",
                    DAP_FIELD(i, "i"));

struct JSONTestObject {
  boolean b;
  integer i;
  number n;
  array<integer> a;
  object o;
  string s;
  optional<integer> o1;
  optional<integer> o2;
  JSONInnerTestObject inner;
};

DAP_STRUCT_TYPEINFO(JSONTestObject,
                    "json-test-object",
                    DAP_FIELD(b, "b"),
                    DAP_FIELD(i, "i"),
                    DAP_FIELD(n, "n"),
                    DAP_FIELD(a, "a"),
                    DAP_FIELD(o, "o"),
                    DAP_FIELD(s, "s"),
                    DAP_FIELD(o1, "o1"),
                    DAP_FIELD(o2, "o2"),
                    DAP_FIELD(inner, "inner"));

struct JSONObjectNoFields {};

DAP_STRUCT_TYPEINFO(JSONObjectNoFields, "json-object-no-fields");

struct SimpleJSONTestObject {
  boolean b;
  integer i;
};
DAP_STRUCT_TYPEINFO(SimpleJSONTestObject,
                    "simple-json-test-object",
                    DAP_FIELD(b, "b"),
                    DAP_FIELD(i, "i"));

}  // namespace dap

class JSONSerializer : public testing::Test {
 protected:
  static dap::object GetSimpleObject() {
    return dap::object({{"one", dap::integer(1)},
                        {"two", dap::number(2)},
                        {"three", dap::string("three")},
                        {"four", dap::boolean(true)}});
  }
  void TEST_SIMPLE_OBJECT(const dap::object& obj) {
    NESTED_TEST_FAILED = true;
    auto ref_obj = GetSimpleObject();
    ASSERT_EQ(obj.size(), ref_obj.size());
    ASSERT_TRUE(obj.at("one").is<dap::integer>());
    ASSERT_TRUE(obj.at("two").is<dap::number>());
    ASSERT_TRUE(obj.at("three").is<dap::string>());
    ASSERT_TRUE(obj.at("four").is<dap::boolean>());

    ASSERT_EQ(ref_obj.at("one").get<dap::integer>(),
              obj.at("one").get<dap::integer>());
    ASSERT_EQ(ref_obj.at("two").get<dap::number>(),
              obj.at("two").get<dap::number>());
    ASSERT_EQ(ref_obj.at("three").get<dap::string>(),
              obj.at("three").get<dap::string>());
    ASSERT_EQ(ref_obj.at("four").get<dap::boolean>(),
              obj.at("four").get<dap::boolean>());
    NESTED_TEST_FAILED = false;
  }
  template <typename T>
  void TEST_SERIALIZING_DESERIALIZING(const T& encoded, T& decoded) {
    NESTED_TEST_FAILED = true;
    dap::json::Serializer s;
    ASSERT_TRUE(s.serialize(encoded));
    dap::json::Deserializer d(s.dump());
    ASSERT_TRUE(d.deserialize(&decoded));
    NESTED_TEST_FAILED = false;
  }
  bool NESTED_TEST_FAILED = false;
#define _ASSERT_PASS(NESTED_TEST) \
  NESTED_TEST;                    \
  ASSERT_FALSE(NESTED_TEST_FAILED);
};

TEST_F(JSONSerializer, SerializeDeserialize) {
  dap::JSONTestObject encoded;
  encoded.b = true;
  encoded.i = 32;
  encoded.n = 123.456;
  encoded.a = {2, 4, 6, 8, 0x100000000, -2, -4, -6, -8, -0x100000000};
  encoded.o["one"] = dap::integer(1);
  encoded.o["two"] = dap::number(2);
  encoded.s = "hello world";
  encoded.o2 = 42;
  encoded.inner.i = 70;

  dap::json::Serializer s;
  ASSERT_TRUE(s.serialize(encoded));

  dap::JSONTestObject decoded;
  dap::json::Deserializer d(s.dump());
  ASSERT_TRUE(d.deserialize(&decoded));

  ASSERT_EQ(encoded.b, decoded.b);
  ASSERT_EQ(encoded.i, decoded.i);
  ASSERT_EQ(encoded.n, decoded.n);
  ASSERT_EQ(encoded.a, decoded.a);
  ASSERT_EQ(encoded.o["one"].get<dap::integer>(),
            decoded.o["one"].get<dap::integer>());
  ASSERT_EQ(encoded.o["two"].get<dap::number>(),
            decoded.o["two"].get<dap::number>());
  ASSERT_EQ(encoded.s, decoded.s);
  ASSERT_EQ(encoded.o2, decoded.o2);
  ASSERT_EQ(encoded.inner.i, decoded.inner.i);
}

TEST_F(JSONSerializer, SerializeObjectNoFields) {
  dap::JSONObjectNoFields obj;
  dap::json::Serializer s;
  ASSERT_TRUE(s.serialize(obj));
  ASSERT_EQ(s.dump(), "{}");
}

TEST_F(JSONSerializer, SerializeDeserializeObject) {
  dap::object encoded = GetSimpleObject();
  dap::object decoded;
  _ASSERT_PASS(TEST_SERIALIZING_DESERIALIZING(encoded, decoded));
  _ASSERT_PASS(TEST_SIMPLE_OBJECT(decoded));
}

TEST_F(JSONSerializer, SerializeDeserializeEmbeddedObject) {
  dap::object encoded;
  dap::object decoded;
  // object nested inside object
  dap::object encoded_embed_obj = GetSimpleObject();
  dap::object decoded_embed_obj;
  encoded["embed_obj"] = encoded_embed_obj;
  _ASSERT_PASS(TEST_SERIALIZING_DESERIALIZING(encoded, decoded));
  ASSERT_TRUE(decoded["embed_obj"].is<dap::object>());
  decoded_embed_obj = decoded["embed_obj"].get<dap::object>();
  _ASSERT_PASS(TEST_SIMPLE_OBJECT(decoded_embed_obj));
}

TEST_F(JSONSerializer, SerializeDeserializeEmbeddedStruct) {
  dap::object encoded;
  dap::object decoded;
  // object nested inside object
  dap::SimpleJSONTestObject encoded_embed_struct;
  encoded_embed_struct.b = true;
  encoded_embed_struct.i = 50;
  encoded["embed_struct"] = encoded_embed_struct;

  dap::object decoded_embed_obj;
  _ASSERT_PASS(TEST_SERIALIZING_DESERIALIZING(encoded, decoded));
  ASSERT_TRUE(decoded["embed_struct"].is<dap::object>());
  decoded_embed_obj = decoded["embed_struct"].get<dap::object>();
  ASSERT_TRUE(decoded_embed_obj.at("b").is<dap::boolean>());
  ASSERT_TRUE(decoded_embed_obj.at("i").is<dap::integer>());

  ASSERT_EQ(encoded_embed_struct.b, decoded_embed_obj["b"].get<dap::boolean>());
  ASSERT_EQ(encoded_embed_struct.i, decoded_embed_obj["i"].get<dap::integer>());
}

TEST_F(JSONSerializer, SerializeDeserializeEmbeddedIntArray) {
  dap::object encoded;
  dap::object decoded;
  // array nested inside object
  dap::array<dap::integer> encoded_embed_arr = {1, 2, 3, 4};
  dap::array<dap::any> decoded_embed_arr;

  encoded["embed_arr"] = encoded_embed_arr;

  _ASSERT_PASS(TEST_SERIALIZING_DESERIALIZING(encoded, decoded));
  // TODO: Deserializing array should infer basic member types
  ASSERT_TRUE(decoded["embed_arr"].is<dap::array<dap::any>>());
  decoded_embed_arr = decoded["embed_arr"].get<dap::array<dap::any>>();
  ASSERT_EQ(encoded_embed_arr.size(), decoded_embed_arr.size());
  for (std::size_t i = 0; i < decoded_embed_arr.size(); i++) {
    ASSERT_TRUE(decoded_embed_arr[i].is<dap::integer>());
    ASSERT_EQ(encoded_embed_arr[i], decoded_embed_arr[i].get<dap::integer>());
  }
}

TEST_F(JSONSerializer, SerializeDeserializeEmbeddedObjectArray) {
  dap::object encoded;
  dap::object decoded;

  dap::array<dap::object> encoded_embed_arr = {GetSimpleObject(),
                                               GetSimpleObject()};
  dap::array<dap::any> decoded_embed_arr;

  encoded["embed_arr"] = encoded_embed_arr;

  _ASSERT_PASS(TEST_SERIALIZING_DESERIALIZING(encoded, decoded));
  // TODO: Deserializing array should infer basic member types
  ASSERT_TRUE(decoded["embed_arr"].is<dap::array<dap::any>>());
  decoded_embed_arr = decoded["embed_arr"].get<dap::array<dap::any>>();
  ASSERT_EQ(encoded_embed_arr.size(), decoded_embed_arr.size());
  for (std::size_t i = 0; i < decoded_embed_arr.size(); i++) {
    ASSERT_TRUE(decoded_embed_arr[i].is<dap::object>());
    _ASSERT_PASS(TEST_SIMPLE_OBJECT(decoded_embed_arr[i].get<dap::object>()));
  }
}

TEST_F(JSONSerializer, DeserializeSerializeEmptyObject) {
  auto empty_obj = "{}";
  dap::object decoded;
  dap::json::Deserializer d(empty_obj);
  ASSERT_TRUE(d.deserialize(&decoded));
  dap::json::Serializer s;
  ASSERT_TRUE(s.serialize(decoded));
  ASSERT_EQ(s.dump(), empty_obj);
}

TEST_F(JSONSerializer, SerializeDeserializeEmbeddedEmptyObject) {
  dap::object encoded_empty_obj;
  dap::object encoded = {{"empty_obj", encoded_empty_obj}};
  dap::object decoded;

  _ASSERT_PASS(TEST_SERIALIZING_DESERIALIZING(encoded, decoded));
  ASSERT_TRUE(decoded["empty_obj"].is<dap::object>());
  dap::object decoded_empty_obj = decoded["empty_obj"].get<dap::object>();
  ASSERT_EQ(encoded_empty_obj.size(), decoded_empty_obj.size());
}

TEST_F(JSONSerializer, SerializeDeserializeObjectWithNulledField) {
  auto thing = dap::any(dap::null());
  dap::object encoded;
  encoded["nulled_field"] = dap::null();
  dap::json::Serializer s;
  ASSERT_TRUE(s.serialize(encoded));
  dap::object decoded;
  auto dump = s.dump();
  dap::json::Deserializer d(dump);
  ASSERT_TRUE(d.deserialize(&decoded));
  ASSERT_TRUE(encoded["nulled_field"].is<dap::null>());
}
