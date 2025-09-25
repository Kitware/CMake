/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <map>
#include <string>
#include <vector>

#include "cmJSONState.h"

namespace Json {
class Value;
}

class cmCTestResourceSpec
{
public:
  class Resource
  {
  public:
    std::string Id;
    unsigned int Capacity;

    bool operator==(Resource const& other) const;
    bool operator!=(Resource const& other) const;
  };

  class Socket
  {
  public:
    std::map<std::string, std::vector<Resource>> Resources;

    bool operator==(Socket const& other) const;
    bool operator!=(Socket const& other) const;
  };

  Socket LocalSocket;
  cmJSONState parseState;

  bool ReadFromJSONFile(std::string const& filename);

  bool operator==(cmCTestResourceSpec const& other) const;
  bool operator!=(cmCTestResourceSpec const& other) const;
};

namespace cmCTestResourceSpecErrors {
auto const FILE_NOT_FOUND = [](Json::Value const*, cmJSONState* state) {
  state->AddError("File not found");
};
auto const JSON_PARSE_ERROR = [](Json::Value const* value,
                                 cmJSONState* state) {
  state->AddErrorAtValue("JSON parse error", value);
};
auto const INVALID_ROOT = [](Json::Value const* value, cmJSONState* state) {
  state->AddErrorAtValue("Invalid root object", value);
};
auto const NO_VERSION = [](Json::Value const* value, cmJSONState* state) {
  state->AddErrorAtValue("No version specified", value);
};
auto const INVALID_VERSION = [](Json::Value const* value, cmJSONState* state) {
  state->AddErrorAtValue("Invalid version object", value);
};
auto const UNSUPPORTED_VERSION = [](Json::Value const* value,
                                    cmJSONState* state) {
  state->AddErrorAtValue("Unsupported version", value);
};
auto const INVALID_SOCKET_SPEC = [](Json::Value const* value,
                                    cmJSONState* state) {
  state->AddErrorAtValue("Invalid socket object", value);
};
auto const INVALID_RESOURCE_TYPE = [](Json::Value const* value,
                                      cmJSONState* state) {
  state->AddErrorAtValue("Invalid resource type object", value);
};
auto const INVALID_RESOURCE = [](Json::Value const* value,
                                 cmJSONState* state) {
  state->AddErrorAtValue("Invalid resource object", value);
};
}
