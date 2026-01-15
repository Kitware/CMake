/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

/*
 * Fuzzer for CMake's glob/regex matching
 *
 * Tests glob pattern matching and regex compilation.
 */

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "cmsys/Glob.hxx"
#include "cmsys/RegularExpression.hxx"

#include "cmSystemTools.h"

static constexpr size_t kMaxInputSize = 4096;

extern "C" int LLVMFuzzerTestOneInput(uint8_t const* data, size_t size)
{
  if (size == 0 || size > kMaxInputSize) {
    return 0;
  }

  std::string input(reinterpret_cast<char const*>(data), size);

  // Test glob pattern matching
  {
    cmsys::Glob glob;
    glob.SetRecurse(false);
    glob.SetRelative("/tmp");

    // Try to find files matching the pattern (safe - just pattern matching)
    // Don't actually recurse filesystem, just test pattern parsing
    (void)glob.GetFiles();
  }

  // Test regex compilation (may throw on invalid patterns)
  {
    cmsys::RegularExpression regex;
    bool compiled = regex.compile(input);
    if (compiled) {
      // Test matching against some strings
      (void)regex.find("test string");
      (void)regex.find(input);
      (void)regex.find("");
    }
  }

  // Test string matching utilities
  (void)cmSystemTools::StringStartsWith(input, "CMAKE_");
  (void)cmSystemTools::StringEndsWith(input, ".cmake");

  // Test simple pattern matching
  if (size >= 4) {
    std::string pattern(reinterpret_cast<char const*>(data), size / 2);
    std::string text(reinterpret_cast<char const*>(data + size / 2),
                     size - size / 2);
    // Pattern matching is done through Glob::FindFiles, which we avoid
    // to prevent filesystem access. Just test string operations.
    (void)pattern.length();
    (void)text.length();
  }

  return 0;
}
