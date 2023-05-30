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

#ifndef dap_json_serializer_h
#define dap_json_serializer_h

#if defined(CPPDAP_JSON_NLOHMANN)
#include "nlohmann_json_serializer.h"
#elif defined(CPPDAP_JSON_RAPID)
#include "rapid_json_serializer.h"
#elif defined(CPPDAP_JSON_JSONCPP)
#include "jsoncpp_json_serializer.h"
#else
#error "Unrecognised cppdap JSON library"
#endif

namespace dap {
namespace json {

#if defined(CPPDAP_JSON_NLOHMANN)
using Deserializer = NlohmannDeserializer;
using Serializer = NlohmannSerializer;
#elif defined(CPPDAP_JSON_RAPID)
using Deserializer = RapidDeserializer;
using Serializer = RapidSerializer;
#elif defined(CPPDAP_JSON_JSONCPP)
using Deserializer = JsonCppDeserializer;
using Serializer = JsonCppSerializer;
#else
#error "Unrecognised cppdap JSON library"
#endif

}  // namespace json
}  // namespace dap

#endif  // dap_json_serializer_h
