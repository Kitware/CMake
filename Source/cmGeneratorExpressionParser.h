/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmGeneratorExpressionParser_h
#define cmGeneratorExpressionParser_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <memory>
#include <vector>

#include "cmGeneratorExpressionLexer.h"

struct cmGeneratorExpressionEvaluator;

struct cmGeneratorExpressionParser
{
  cmGeneratorExpressionParser(std::vector<cmGeneratorExpressionToken> tokens);

  using cmGeneratorExpressionEvaluatorVector =
    std::vector<std::unique_ptr<cmGeneratorExpressionEvaluator>>;

  void Parse(cmGeneratorExpressionEvaluatorVector& result);

private:
  void ParseContent(cmGeneratorExpressionEvaluatorVector&);
  void ParseGeneratorExpression(cmGeneratorExpressionEvaluatorVector&);

private:
  std::vector<cmGeneratorExpressionToken>::const_iterator it;
  const std::vector<cmGeneratorExpressionToken> Tokens;
  unsigned int NestingLevel;
};

#endif
