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

#include "nlohmann_json_serializer.h"

#include "null_json_serializer.h"

// Disable JSON exceptions. We should be guarding against any exceptions being
// fired in this file.
#define JSON_NOEXCEPTION 1
#include <nlohmann/json.hpp>

namespace dap {
namespace json {

NlohmannDeserializer::NlohmannDeserializer(const std::string& str)
    : json(new nlohmann::json(nlohmann::json::parse(str, nullptr, false))),
      ownsJson(true) {}

NlohmannDeserializer::NlohmannDeserializer(const nlohmann::json* json)
    : json(json), ownsJson(false) {}

NlohmannDeserializer::~NlohmannDeserializer() {
  if (ownsJson) {
    delete json;
  }
}

bool NlohmannDeserializer::deserialize(dap::boolean* v) const {
  if (!json->is_boolean()) {
    return false;
  }
  *v = json->get<bool>();
  return true;
}

bool NlohmannDeserializer::deserialize(dap::integer* v) const {
  if (!json->is_number_integer()) {
    return false;
  }
  *v = json->get<int64_t>();
  return true;
}

bool NlohmannDeserializer::deserialize(dap::number* v) const {
  if (!json->is_number()) {
    return false;
  }
  *v = json->get<double>();
  return true;
}

bool NlohmannDeserializer::deserialize(dap::string* v) const {
  if (!json->is_string()) {
    return false;
  }
  *v = json->get<std::string>();
  return true;
}

bool NlohmannDeserializer::deserialize(dap::object* v) const {
  v->reserve(json->size());
  for (auto& el : json->items()) {
    NlohmannDeserializer d(&el.value());
    dap::any val;
    if (!d.deserialize(&val)) {
      return false;
    }
    (*v)[el.key()] = val;
  }
  return true;
}

bool NlohmannDeserializer::deserialize(dap::any* v) const {
  if (json->is_boolean()) {
    *v = dap::boolean(json->get<bool>());
  } else if (json->is_number_float()) {
    *v = dap::number(json->get<double>());
  } else if (json->is_number_integer()) {
    *v = dap::integer(json->get<int64_t>());
  } else if (json->is_string()) {
    *v = json->get<std::string>();
  } else if (json->is_object()) {
    dap::object obj;
    if (!deserialize(&obj)) {
      return false;
    }
    *v = obj;
  } else if (json->is_array()) {
    dap::array<any> arr;
    if (!deserialize(&arr)) {
      return false;
    }
    *v = arr;
  } else if (json->is_null()) {
    *v = null();
  } else {
    return false;
  }
  return true;
}

size_t NlohmannDeserializer::count() const {
  return json->size();
}

bool NlohmannDeserializer::array(
    const std::function<bool(dap::Deserializer*)>& cb) const {
  if (!json->is_array()) {
    return false;
  }
  for (size_t i = 0; i < json->size(); i++) {
    NlohmannDeserializer d(&(*json)[i]);
    if (!cb(&d)) {
      return false;
    }
  }
  return true;
}

bool NlohmannDeserializer::field(
    const std::string& name,
    const std::function<bool(dap::Deserializer*)>& cb) const {
  if (!json->is_structured()) {
    return false;
  }
  auto it = json->find(name);
  if (it == json->end()) {
    return cb(&NullDeserializer::instance);
  }
  auto obj = *it;
  NlohmannDeserializer d(&obj);
  return cb(&d);
}

NlohmannSerializer::NlohmannSerializer()
    : json(new nlohmann::json()), ownsJson(true) {}

NlohmannSerializer::NlohmannSerializer(nlohmann::json* json)
    : json(json), ownsJson(false) {}

NlohmannSerializer::~NlohmannSerializer() {
  if (ownsJson) {
    delete json;
  }
}

std::string NlohmannSerializer::dump() const {
  return json->dump();
}

bool NlohmannSerializer::serialize(dap::boolean v) {
  *json = (bool)v;
  return true;
}

bool NlohmannSerializer::serialize(dap::integer v) {
  *json = (int64_t)v;
  return true;
}

bool NlohmannSerializer::serialize(dap::number v) {
  *json = (double)v;
  return true;
}

bool NlohmannSerializer::serialize(const dap::string& v) {
  *json = v;
  return true;
}

bool NlohmannSerializer::serialize(const dap::object& v) {
  if (!json->is_object()) {
    *json = nlohmann::json::object();
  }
  for (auto& it : v) {
    NlohmannSerializer s(&(*json)[it.first]);
    if (!s.serialize(it.second)) {
      return false;
    }
  }
  return true;
}

bool NlohmannSerializer::serialize(const dap::any& v) {
  if (v.is<dap::boolean>()) {
    *json = (bool)v.get<dap::boolean>();
  } else if (v.is<dap::integer>()) {
    *json = (int64_t)v.get<dap::integer>();
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

bool NlohmannSerializer::array(
    size_t count,
    const std::function<bool(dap::Serializer*)>& cb) {
  *json = std::vector<int>();
  for (size_t i = 0; i < count; i++) {
    NlohmannSerializer s(&(*json)[i]);
    if (!cb(&s)) {
      return false;
    }
  }
  return true;
}

bool NlohmannSerializer::object(
    const std::function<bool(dap::FieldSerializer*)>& cb) {
  struct FS : public FieldSerializer {
    nlohmann::json* const json;

    FS(nlohmann::json* json) : json(json) {}
    bool field(const std::string& name, const SerializeFunc& cb) override {
      NlohmannSerializer s(&(*json)[name]);
      auto res = cb(&s);
      if (s.removed) {
        json->erase(name);
      }
      return res;
    }
  };

  *json = nlohmann::json({}, false, nlohmann::json::value_t::object);
  FS fs{json};
  return cb(&fs);
}

void NlohmannSerializer::remove() {
  removed = true;
}

}  // namespace json
}  // namespace dap
