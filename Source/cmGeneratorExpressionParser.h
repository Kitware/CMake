/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <memory>
#include <vector>

#include "cmGeneratorExpressionEvaluator.h"
#include "cmGeneratorExpressionLexer.h"

struct cmGeneratorExpressionParser
{
  cmGeneratorExpressionParser(std::vector<cmGeneratorExpressionToken> tokens);

  using cmGeneratorExpressionEvaluatorVector =
    std::vector<std::unique_ptr<cmGeneratorExpressionEvaluator>>;

  void Parse(cmGeneratorExpressionEvaluatorVector& result);

private:
  void ParseContent(cmGeneratorExpressionEvaluatorVector&);
  void ParseGeneratorExpression(cmGeneratorExpressionEvaluatorVector&);

  std::vector<cmGeneratorExpressionToken>::const_iterator it;
  std::vector<cmGeneratorExpressionToken> const Tokens;
  unsigned int NestingLevel = 0;
};
