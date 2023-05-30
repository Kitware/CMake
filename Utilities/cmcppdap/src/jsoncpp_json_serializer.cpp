// Copyright 2023 Google LLC
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

#include "jsoncpp_json_serializer.h"

#include "null_json_serializer.h"

#include <cm3p/json/json.h>
#include <cstdlib>
#include <memory>

namespace dap {
namespace json {

JsonCppDeserializer::JsonCppDeserializer(const std::string& str)
    : json(new Json::Value(JsonCppDeserializer::parse(str))), ownsJson(true) {}

JsonCppDeserializer::JsonCppDeserializer(const Json::Value* json)
    : json(json), ownsJson(false) {}

JsonCppDeserializer::~JsonCppDeserializer() {
  if (ownsJson) {
    delete json;
  }
}

bool JsonCppDeserializer::deserialize(dap::boolean* v) const {
  if (!json->isBool()) {
    return false;
  }
  *v = json->asBool();
  return true;
}

bool JsonCppDeserializer::deserialize(dap::integer* v) const {
  if (!json->isInt64()) {
    return false;
  }
  *v = json->asInt64();
  return true;
}

bool JsonCppDeserializer::deserialize(dap::number* v) const {
  if (!json->isNumeric()) {
    return false;
  }
  *v = json->asDouble();
  return true;
}

bool JsonCppDeserializer::deserialize(dap::string* v) const {
  if (!json->isString()) {
    return false;
  }
  *v = json->asString();
  return true;
}

bool JsonCppDeserializer::deserialize(dap::object* v) const {
  v->reserve(json->size());
  for (auto i = json->begin(); i != json->end(); i++) {
    JsonCppDeserializer d(&*i);
    dap::any val;
    if (!d.deserialize(&val)) {
      return false;
    }
    (*v)[i.name()] = val;
  }
  return true;
}

bool JsonCppDeserializer::deserialize(dap::any* v) const {
  if (json->isBool()) {
    *v = dap::boolean(json->asBool());
  } else if (json->type() == Json::ValueType::realValue) {
    // json->isDouble() returns true for integers as well, so we need to
    // explicitly look for the realValue type.
    *v = dap::number(json->asDouble());
  } else if (json->isInt64()) {
    *v = dap::integer(json->asInt64());
  } else if (json->isString()) {
    *v = json->asString();
  } else if (json->isObject()) {
    dap::object obj;
    if (!deserialize(&obj)) {
      return false;
    }
    *v = obj;
  } else if (json->isArray()) {
    dap::array<any> arr;
    if (!deserialize(&arr)) {
      return false;
    }
    *v = arr;
  } else if (json->isNull()) {
    *v = null();
  } else {
    return false;
  }
  return true;
}

size_t JsonCppDeserializer::count() const {
  return json->size();
}

bool JsonCppDeserializer::array(
    const std::function<bool(dap::Deserializer*)>& cb) const {
  if (!json->isArray()) {
    return false;
  }
  for (const auto& value : *json) {
    JsonCppDeserializer d(&value);
    if (!cb(&d)) {
      return false;
    }
  }
  return true;
}

bool JsonCppDeserializer::field(
    const std::string& name,
    const std::function<bool(dap::Deserializer*)>& cb) const {
  if (!json->isObject()) {
    return false;
  }
  auto value = json->find(name.data(), name.data() + name.size());
  if (value == nullptr) {
    return cb(&NullDeserializer::instance);
  }
  JsonCppDeserializer d(value);
  return cb(&d);
}

Json::Value JsonCppDeserializer::parse(const std::string& text) {
  Json::CharReaderBuilder builder;
  auto jsonReader = std::unique_ptr<Json::CharReader>(builder.newCharReader());
  Json::Value json;
  std::string error;
  if (!jsonReader->parse(text.data(), text.data() + text.size(), &json,
                         &error)) {
    // cppdap expects that the JSON layer does not throw exceptions.
    std::abort();
  }
  return json;
}

JsonCppSerializer::JsonCppSerializer()
    : json(new Json::Value()), ownsJson(true) {}

JsonCppSerializer::JsonCppSerializer(Json::Value* json)
    : json(json), ownsJson(false) {}

JsonCppSerializer::~JsonCppSerializer() {
  if (ownsJson) {
    delete json;
  }
}

std::string JsonCppSerializer::dump() const {
  Json::StreamWriterBuilder writer;
  return Json::writeString(writer, *json);
}

bool JsonCppSerializer::serialize(dap::boolean v) {
  *json = (bool)v;
  return true;
}

bool JsonCppSerializer::serialize(dap::integer v) {
  *json = (Json::LargestInt)v;
  return true;
}

bool JsonCppSerializer::serialize(dap::number v) {
  *json = (double)v;
  return true;
}

bool JsonCppSerializer::serialize(const dap::string& v) {
  *json = v;
  return true;
}

bool JsonCppSerializer::serialize(const dap::object& v) {
  if (!json->isObject()) {
    *json = Json::Value(Json::objectValue);
  }
  for (auto& it : v) {
    JsonCppSerializer s(&(*json)[it.first]);
    if (!s.serialize(it.second)) {
      return false;
    }
  }
  return true;
}

bool JsonCppSerializer::serialize(const dap::any& v) {
  if (v.is<dap::boolean>()) {
    *json = (bool)v.get<dap::boolean>();
  } else if (v.is<dap::integer>()) {
    *json = (Json::LargestInt)v.get<dap::integer>();
  } else if (v.is<dap::number>()) {
    *json = (double)v.get<dap::number>();
  } else if (v.is<dap::string>()) {
    *json = v.get<dap::string>();
  } else if (v.is<dap::object>()) {
    // reachable if dap::object nested is inside other dap::object
    return serialize(v.get<dap::object>());
  } else if (v.is<dap::null>()) {
  } else {
    // reachable if array or custom serialized type is nested inside other
    auto type = get_any_type(v);
    auto value = get_any_val(v);
    if (type && value) {
      return type->serialize(this, value);
    }
    return false;
  }
  return true;
}

bool JsonCppSerializer::array(size_t count,
                              const std::function<bool(dap::Serializer*)>& cb) {
  *json = Json::Value(Json::arrayValue);
  for (size_t i = 0; i < count; i++) {
    JsonCppSerializer s(&(*json)[Json::Value::ArrayIndex(i)]);
    if (!cb(&s)) {
      return false;
    }
  }
  return true;
}

bool JsonCppSerializer::object(
    const std::function<bool(dap::FieldSerializer*)>& cb) {
  struct FS : public FieldSerializer {
    Json::Value* const json;

    FS(Json::Value* json) : json(json) {}
    bool field(const std::string& name, const SerializeFunc& cb) override {
      JsonCppSerializer s(&(*json)[name]);
      auto res = cb(&s);
      if (s.removed) {
        json->removeMember(name);
      }
      return res;
    }
  };

  *json = Json::Value(Json::objectValue);
  FS fs{json};
  return cb(&fs);
}

void JsonCppSerializer::remove() {
  removed = true;
}

}  // namespace json
}  // namespace dap
