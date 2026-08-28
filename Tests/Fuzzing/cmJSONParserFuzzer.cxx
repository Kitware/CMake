/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

/*
 * Fuzzer for CMake's JSON parsing (via jsoncpp)
 *
 * CMake parses JSON files for CMakePresets.json, compile_commands.json,
 * and various other configuration files. This fuzzer tests the JSON
 * parser for crashes and undefined behavior.
 *
 * Coverage targets:
 * - JSON value parsing (strings, numbers, booleans, null)
 * - Array and object parsing
 * - Nested structures
 * - Unicode handling
 * - Error recovery
 */

#include <cstddef>
#include <cstdint>
#include <sstream>
#include <string>

#include <cm3p/json/value.h>

#include "cmJSONState.h"

// Limit input size
static constexpr size_t kMaxInputSize = 256 * 1024; // 256KB

// Recursive helper to access all values (exercises accessor code)
static void TraverseValue(Json::Value const& value, int depth = 0)
{
  // Prevent stack overflow on deeply nested structures
  if (depth > 100) {
    return;
  }

  switch (value.type()) {
    case Json::nullValue:
      (void)value.isNull();
      break;
    case Json::intValue:
      (void)value.asInt64();
      break;
    case Json::uintValue:
      (void)value.asUInt64();
      break;
    case Json::realValue:
      (void)value.asDouble();
      break;
    case Json::stringValue:
      (void)value.asString();
      break;
    case Json::booleanValue:
      (void)value.asBool();
      break;
    case Json::arrayValue:
      for (Json::ArrayIndex i = 0; i < value.size() && i < 1000; ++i) {
        TraverseValue(value[i], depth + 1);
      }
      break;
    case Json::objectValue:
      for (auto const& name : value.getMemberNames()) {
        (void)name;
        TraverseValue(value[name], depth + 1);
      }
      break;
  }
}

extern "C" int LLVMFuzzerTestOneInput(uint8_t const* data, size_t size)
{
  if (size == 0 || size > kMaxInputSize) {
    return 0;
  }

  std::string input(reinterpret_cast<char const*>(data), size);

  {
    Json::Value root;
    std::istringstream stream(input);

    // Try parsing with default settings
    cmJSONState parseState(stream, &root, cmJSONState::StrictMode::Relaxed);
    if (!parseState.errors.empty()) {
      // Traverse the parsed structure
      TraverseValue(root);
    }
  }

  {
    Json::Value root;
    std::istringstream stream(input);

    // Also try with strict mode
    cmJSONState parseState(stream, &root, cmJSONState::StrictMode::Strict);
    if (!parseState.errors.empty()) {
      // Traverse the parsed structure
      TraverseValue(root);
    }
  }

  return 0;
}
