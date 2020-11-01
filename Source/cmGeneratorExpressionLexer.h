/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <cstddef>
#include <string>
#include <vector>

struct cmGeneratorExpressionToken
{
  cmGeneratorExpressionToken(unsigned type, const char* c, size_t l)
    : TokenType(type)
    , Content(c)
    , Length(l)
  {
  }
  enum
  {
    Text,
    BeginExpression,
    EndExpression,
    ColonSeparator,
    CommaSeparator
  };
  unsigned TokenType;
  const char* Content;
  size_t Length;
};

/** \class cmGeneratorExpressionLexer
 *
 */
class cmGeneratorExpressionLexer
{
public:
  cmGeneratorExpressionLexer();

  std::vector<cmGeneratorExpressionToken> Tokenize(const std::string& input);

  bool GetSawGeneratorExpression() const
  {
    return this->SawGeneratorExpression;
  }

private:
  bool SawBeginExpression = false;
  bool SawGeneratorExpression = false;
};
