// Copyright 2020 Google LLC
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

#ifndef dap_null_json_serializer_h
#define dap_null_json_serializer_h

#include "dap/protocol.h"
#include "dap/serialization.h"
#include "dap/types.h"

namespace dap {
namespace json {

struct NullDeserializer : public dap::Deserializer {
  static NullDeserializer instance;

  bool deserialize(dap::boolean*) const override { return false; }
  bool deserialize(dap::integer*) const override { return false; }
  bool deserialize(dap::number*) const override { return false; }
  bool deserialize(dap::string*) const override { return false; }
  bool deserialize(dap::object*) const override { return false; }
  bool deserialize(dap::any*) const override { return false; }
  size_t count() const override { return 0; }
  bool array(const std::function<bool(dap::Deserializer*)>&) const override {
    return false;
  }
  bool field(const std::string&,
             const std::function<bool(dap::Deserializer*)>&) const override {
    return false;
  }
};

}  // namespace json
}  // namespace dap

#endif  // dap_null_json_serializer_h
