/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

/*
 * Fuzzer for CMake's string algorithms
 *
 * Tests string manipulation, escaping, and processing functions.
 */

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

static constexpr size_t kMaxInputSize = 16 * 1024;

extern "C" int LLVMFuzzerTestOneInput(uint8_t const* data, size_t size)
{
  if (size == 0 || size > kMaxInputSize) {
    return 0;
  }

  std::string input(reinterpret_cast<char const*>(data), size);

  // Test string manipulation functions
  (void)cmTrimWhitespace(input);
  (void)cmRemoveQuotes(input);
  (void)cmEscapeQuotes(input);

  // Test case conversion
  (void)cmSystemTools::UpperCase(input);
  (void)cmSystemTools::LowerCase(input);

  // Test tokenization
  std::vector<std::string> tokens = cmTokenize(input, " \t\n\r");
  (void)tokens.size();

  // Test with different separators if input is large enough
  if (size > 4) {
    std::string sep(reinterpret_cast<char const*>(data), 2);
    std::string str(reinterpret_cast<char const*>(data + 2), size - 2);
    std::vector<std::string> parts = cmTokenize(str, sep);
    (void)parts.size();
  }

  // Test join operations
  if (!tokens.empty()) {
    (void)cmJoin(tokens, ";");
    (void)cmJoin(tokens, ",");
  }

  // Test string contains
  if (size > 2) {
    std::string haystack(reinterpret_cast<char const*>(data), size / 2);
    std::string needle(reinterpret_cast<char const*>(data + size / 2),
                       size - size / 2);
    (void)cmHasPrefix(haystack, needle);
    (void)cmHasSuffix(haystack, needle);
  }

  // Test path operations
  (void)cmSystemTools::GetFilenameWithoutExtension(input);
  (void)cmSystemTools::GetFilenameExtension(input);
  (void)cmSystemTools::GetFilenameName(input);
  (void)cmSystemTools::GetFilenameLastExtension(input);
  (void)cmSystemTools::GetFilenamePath(input);

  // Test path normalization
  (void)cmSystemTools::CollapseFullPath(input);

  return 0;
}
