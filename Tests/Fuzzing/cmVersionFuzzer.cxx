/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

/*
 * Fuzzer for CMake's version comparison
 *
 * Tests version string parsing and comparison operations.
 */

#include <cstddef>
#include <cstdint>
#include <string>

#include "cmSystemTools.h"
#include "cmVersion.h"

static constexpr size_t kMaxInputSize = 1024;

extern "C" int LLVMFuzzerTestOneInput(uint8_t const* data, size_t size)
{
  if (size == 0 || size > kMaxInputSize) {
    return 0;
  }

  std::string input(reinterpret_cast<char const*>(data), size);

  // Test version comparison with current CMake version
  (void)cmSystemTools::VersionCompare(cmSystemTools::OP_LESS, input, "3.28.0");
  (void)cmSystemTools::VersionCompare(cmSystemTools::OP_GREATER, input,
                                      "3.28.0");
  (void)cmSystemTools::VersionCompare(cmSystemTools::OP_EQUAL, input,
                                      "3.28.0");
  (void)cmSystemTools::VersionCompare(cmSystemTools::OP_LESS_EQUAL, input,
                                      "3.28.0");
  (void)cmSystemTools::VersionCompare(cmSystemTools::OP_GREATER_EQUAL, input,
                                      "3.28.0");

  // Compare against itself
  (void)cmSystemTools::VersionCompare(cmSystemTools::OP_EQUAL, input, input);

  // If input is large enough, compare two different versions from input
  if (size >= 4) {
    std::string v1(reinterpret_cast<char const*>(data), size / 2);
    std::string v2(reinterpret_cast<char const*>(data + size / 2),
                   size - size / 2);
    (void)cmSystemTools::VersionCompare(cmSystemTools::OP_LESS, v1, v2);
    (void)cmSystemTools::VersionCompare(cmSystemTools::OP_GREATER, v1, v2);
    (void)cmSystemTools::VersionCompare(cmSystemTools::OP_EQUAL, v1, v2);
  }

  // Version compare with greater/less string format
  (void)cmSystemTools::VersionCompareGreater(input, "1.0.0");
  (void)cmSystemTools::VersionCompareGreaterEq(input, "1.0.0");

  return 0;
}
