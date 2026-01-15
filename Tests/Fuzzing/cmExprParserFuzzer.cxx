/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

/*
 * Fuzzer for CMake's math expression parser
 *
 * The math() command uses cmExprParserHelper to evaluate mathematical
 * expressions. This fuzzer tests the expression parser for crashes,
 * hangs, and undefined behavior.
 *
 * Coverage targets:
 * - Integer arithmetic parsing
 * - Operator precedence handling
 * - Parentheses nesting
 * - Error handling for invalid expressions
 */

#include <cstddef>
#include <cstdint>
#include <string>

#include "cmExprParserHelper.h"

// Limit input size to prevent DoS via deeply nested expressions
static constexpr size_t kMaxInputSize = 4096;

extern "C" int LLVMFuzzerTestOneInput(uint8_t const* data, size_t size)
{
  if (size == 0 || size > kMaxInputSize) {
    return 0;
  }

  // Create null-terminated string
  std::string input(reinterpret_cast<char const*>(data), size);

  cmExprParserHelper helper;

  // Parse with different verbosity levels
  int result = helper.ParseString(input.c_str(), 0);
  (void)result;

  // Always check result and error accessors
  (void)helper.GetResult();
  (void)helper.GetError();
  (void)helper.GetWarning();

  return 0;
}
