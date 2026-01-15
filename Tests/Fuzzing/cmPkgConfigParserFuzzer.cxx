/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

/*
 * Fuzzer for CMake's pkg-config file parser
 *
 * CMake parses .pc files (pkg-config) when using PkgConfig find module.
 * Malformed .pc files from untrusted sources could trigger vulnerabilities.
 *
 * Coverage targets:
 * - Variable definitions (key=value)
 * - Keyword definitions (key: value)
 * - Variable references (${var})
 * - Multi-line handling
 * - Comment handling
 */

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "cmPkgConfigParser.h"

// Limit input size
static constexpr size_t kMaxInputSize = 64 * 1024; // 64KB

extern "C" int LLVMFuzzerTestOneInput(uint8_t const* data, size_t size)
{
  if (size == 0 || size > kMaxInputSize) {
    return 0;
  }

  // cmPkgConfigParser::Parse takes non-const buffer (may modify in place)
  std::vector<char> buffer(data, data + size);

  cmPkgConfigParser parser;

  // Parse the input
  auto result = parser.Parse(buffer.data(), buffer.size());
  (void)result;

  // Finish parsing
  result = parser.Finish();
  (void)result;

  // Access parsed data to exercise accessors
  auto& entries = parser.Data();
  for (auto const& entry : entries) {
    (void)entry.IsVariable;
    (void)entry.Key;
    for (auto const& elem : entry.Val) {
      (void)elem.IsVariable;
      (void)elem.Data;
    }
  }

  return 0;
}
