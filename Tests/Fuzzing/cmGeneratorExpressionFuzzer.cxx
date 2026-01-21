/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

/*
 * Fuzzer for CMake's Generator Expression parser
 *
 * Generator expressions ($<...>) are evaluated at build-system generation
 * time. This fuzzer targets the lexer and static parsing utilities that don't
 * require full cmake context.
 *
 * Coverage targets:
 * - Generator expression lexer (cmGeneratorExpressionLexer)
 * - Static parsing/preprocessing functions
 * - Nested expression handling
 * - Expression validation
 */

#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "cmGeneratorExpression.h"
#include "cmGeneratorExpressionLexer.h"

// Limit input size - genex can be exponential in nested cases
static constexpr size_t kMaxInputSize = 16 * 1024; // 16KB

extern "C" int LLVMFuzzerTestOneInput(uint8_t const* data, size_t size)
{
  if (size == 0 || size > kMaxInputSize) {
    return 0;
  }

  std::string input(reinterpret_cast<char const*>(data), size);

  // Test the lexer directly
  {
    cmGeneratorExpressionLexer lexer;
    auto tokens = lexer.Tokenize(input);
    (void)tokens;
  }

  // Test static utility functions that don't need cmake context
  {
    // Find generator expressions
    auto pos = cmGeneratorExpression::Find(input);
    (void)pos;

    // Check if starts with genex
    bool starts = cmGeneratorExpression::StartsWithGeneratorExpression(input);
    (void)starts;

    // Validate as target name
    bool valid = cmGeneratorExpression::IsValidTargetName(input);
    (void)valid;

    // Strip empty list elements
    std::string stripped =
      cmGeneratorExpression::StripEmptyListElements(input);
    (void)stripped;

    // Split expressions
    std::vector<std::string> output;
    cmGeneratorExpression::Split(input, output);

    // Preprocess with different contexts
    std::string preprocessed1 = cmGeneratorExpression::Preprocess(
      input, cmGeneratorExpression::StripAllGeneratorExpressions);
    (void)preprocessed1;

    std::string preprocessed2 = cmGeneratorExpression::Preprocess(
      input, cmGeneratorExpression::BuildInterface);
    (void)preprocessed2;

    std::string preprocessed3 = cmGeneratorExpression::Preprocess(
      input, cmGeneratorExpression::InstallInterface);
    (void)preprocessed3;

    // Collect expressions
    std::map<std::string, std::vector<std::string>> collected;
    std::string collResult = cmGeneratorExpression::Collect(input, collected);
    (void)collResult;
  }

  return 0;
}
