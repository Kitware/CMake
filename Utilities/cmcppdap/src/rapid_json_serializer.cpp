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

#include "rapid_json_serializer.h"

#include "null_json_serializer.h"

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>

namespace dap {
namespace json {

RapidDeserializer::RapidDeserializer(const std::string& str)
    : doc(new rapidjson::Document()) {
  doc->Parse(str.c_str());
}

RapidDeserializer::RapidDeserializer(rapidjson::Value* json) : val(json) {}

RapidDeserializer::~RapidDeserializer() {
  delete doc;
}

bool RapidDeserializer::deserialize(dap::boolean* v) const {
  if (!json()->IsBool()) {
    return false;
  }
  *v = json()->GetBool();
  return true;
}

bool RapidDeserializer::deserialize(dap::integer* v) const {
  if (json()->IsInt()) {
    *v = json()->GetInt();
    return true;
  } else if (json()->IsUint()) {
    *v = static_cast<int64_t>(json()->GetUint());
    return true;
  } else if (json()->IsInt64()) {
    *v = json()->GetInt64();
    return true;
  } else if (json()->IsUint64()) {
    *v = static_cast<int64_t>(json()->GetUint64());
    return true;
  }
  return false;
}

bool RapidDeserializer::deserialize(dap::number* v) const {
  if (!json()->IsNumber()) {
    return false;
  }
  *v = json()->GetDouble();
  return true;
}

bool RapidDeserializer::deserialize(dap::string* v) const {
  if (!json()->IsString()) {
    return false;
  }
  *v = json()->GetString();
  return true;
}

bool RapidDeserializer::deserialize(dap::object* v) const {
  v->reserve(json()->MemberCount());
  for (auto el = json()->MemberBegin(); el != json()->MemberEnd(); el++) {
    dap::any el_val;
    RapidDeserializer d(&(el->value));
    if (!d.deserialize(&el_val)) {
      return false;
    }
    (*v)[el->name.GetString()] = el_val;
  }
  return true;
}

bool RapidDeserializer::deserialize(dap::any* v) const {
  if (json()->IsBool()) {
    *v = dap::boolean(json()->GetBool());
  } else if (json()->IsDouble()) {
    *v = dap::number(json()->GetDouble());
  } else if (json()->IsInt()) {
    *v = dap::integer(json()->GetInt());
  } else if (json()->IsString()) {
    *v = dap::string(json()->GetString());
  } else if (json()->IsNull()) {
    *v = null();
  } else if (json()->IsObject()) {
    dap::object obj;
    if (!deserialize(&obj)) {
      return false;
    }
    *v = obj;
  } else if (json()->IsArray()){
    dap::array<any> arr;
    if (!deserialize(&arr)){
      return false;
    }
    *v = arr;
  } else {
    return false;
  }
  return true;
}

size_t RapidDeserializer::count() const {
  return json()->Size();
}

bool RapidDeserializer::array(
    const std::function<bool(dap::Deserializer*)>& cb) const {
  if (!json()->IsArray()) {
    return false;
  }
  for (uint32_t i = 0; i < json()->Size(); i++) {
    RapidDeserializer d(&(*json())[i]);
    if (!cb(&d)) {
      return false;
    }
  }
  return true;
}

bool RapidDeserializer::field(
    const std::string& name,
    const std::function<bool(dap::Deserializer*)>& cb) const {
  if (!json()->IsObject()) {
    return false;
  }
  auto it = json()->FindMember(name.c_str());
  if (it == json()->MemberEnd()) {
    return cb(&NullDeserializer::instance);
  }
  RapidDeserializer d(&(it->value));
  return cb(&d);
}

RapidSerializer::RapidSerializer()
    : doc(new rapidjson::Document(rapidjson::kObjectType)),
      allocator(doc->GetAllocator()) {}

RapidSerializer::RapidSerializer(rapidjson::Value* json,
                                 rapidjson::Document::AllocatorType& allocator)
    : val(json), allocator(allocator) {}

RapidSerializer::~RapidSerializer() {
  delete doc;
}

std::string RapidSerializer::dump() const {
  rapidjson::StringBuffer sb;
  rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(sb);
  json()->Accept(writer);
  return sb.GetString();
}

bool RapidSerializer::serialize(dap::boolean v) {
  json()->SetBool(v);
  return true;
}

bool RapidSerializer::serialize(dap::integer v) {
  json()->SetInt64(v);
  return true;
}

bool RapidSerializer::serialize(dap::number v) {
  json()->SetDouble(v);
  return true;
}

bool RapidSerializer::serialize(const dap::string& v) {
  json()->SetString(v.data(), static_cast<uint32_t>(v.length()), allocator);
  return true;
}

bool RapidSerializer::serialize(const dap::object& v) {
  if (!json()->IsObject()) {
    json()->SetObject();
  }
  for (auto& it : v) {
    if (!json()->HasMember(it.first.c_str())) {
      rapidjson::Value name_value{it.first.c_str(), allocator};
      json()->AddMember(name_value, rapidjson::Value(), allocator);
    }
    rapidjson::Value& member = (*json())[it.first.c_str()];
    RapidSerializer s(&member, allocator);
    if (!s.serialize(it.second)) {
      return false;
    }
  }
  return true;
}

bool RapidSerializer::serialize(const dap::any& v) {
  if (v.is<dap::boolean>()) {
    json()->SetBool((bool)v.get<dap::boolean>());
  } else if (v.is<dap::integer>()) {
    json()->SetInt64(v.get<dap::integer>());
  } else if (v.is<dap::number>()) {
    json()->SetDouble((double)v.get<dap::number>());
  } else if (v.is<dap::string>()) {
    auto s = v.get<dap::string>();
    json()->SetString(s.data(), static_cast<uint32_t>(s.length()), allocator);
  } else if (v.is<dap::object>()) {
    // reachable if dap::object nested is inside other dap::object
    return serialize(v.get<dap::object>());
  } else if (v.is<dap::null>()) {
  } else {
    // reachable if array or custom serialized type is nested inside other dap::object
    auto type = get_any_type(v);
    auto value = get_any_val(v);
    if (type && value) {
      return type->serialize(this, value);
    }
    return false;
  }

  return true;
}

bool RapidSerializer::array(size_t count,
                            const std::function<bool(dap::Serializer*)>& cb) {
  if (!json()->IsArray()) {
    json()->SetArray();
  }

  while (count > json()->Size()) {
    json()->PushBack(rapidjson::Value(), allocator);
  }

  for (uint32_t i = 0; i < count; i++) {
    RapidSerializer s(&(*json())[i], allocator);
    if (!cb(&s)) {
      return false;
    }
  }
  return true;
}

bool RapidSerializer::object(
    const std::function<bool(dap::FieldSerializer*)>& cb) {
  struct FS : public FieldSerializer {
    rapidjson::Value* const json;
    rapidjson::Document::AllocatorType& allocator;

    FS(rapidjson::Value* json, rapidjson::Document::AllocatorType& allocator)
        : json(json), allocator(allocator) {}
    bool field(const std::string& name, const SerializeFunc& cb) override {
      if (!json->HasMember(name.c_str())) {
        rapidjson::Value name_value{name.c_str(), allocator};
        json->AddMember(name_value, rapidjson::Value(), allocator);
      }
      rapidjson::Value& member = (*json)[name.c_str()];
      RapidSerializer s(&member, allocator);
      auto res = cb(&s);
      if (s.removed) {
        json->RemoveMember(name.c_str());
      }
      return res;
    }
  };

  if (!json()->IsObject()) {
    json()->SetObject();
  }
  FS fs{json(), allocator};
  return cb(&fs);
}

void RapidSerializer::remove() {
  removed = true;
}

}  // namespace json
}  // namespace dap
