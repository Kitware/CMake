/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2012 Stephen Kelly <steveire@gmail.com>

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmGeneratorExpressionLexer.h"


//----------------------------------------------------------------------------
cmGeneratorExpressionLexer::cmGeneratorExpressionLexer()
  : SawBeginExpression(false), SawGeneratorExpression(false)
{

}

//----------------------------------------------------------------------------
static void InsertText(const char *upto, const char *c,
                        std::vector<cmGeneratorExpressionToken> &result)
{
  if (upto != c)
    {
    result.push_back(cmGeneratorExpressionToken(
                          cmGeneratorExpressionToken::Text, upto, c - upto));
    }
}

//----------------------------------------------------------------------------
std::vector<cmGeneratorExpressionToken>
cmGeneratorExpressionLexer::Tokenize(const char *input)
{
  std::vector<cmGeneratorExpressionToken> result;
  if (!input)
    return result;

  const char *c = input;
  const char *upto = c;

  for ( ; *c; ++c)
  {
  if(c[0] == '$' && c[1] == '<')
    {
    InsertText(upto, c, result);
    upto = c;
    result.push_back(cmGeneratorExpressionToken(
                      cmGeneratorExpressionToken::BeginExpression, upto, 2));
    upto = c + 2;
    ++c;
    SawBeginExpression = true;
    }
  else if(c[0] == '>')
    {
    InsertText(upto, c, result);
    upto = c;
    result.push_back(cmGeneratorExpressionToken(
                        cmGeneratorExpressionToken::EndExpression, upto, 1));
    upto = c + 1;
    SawGeneratorExpression = SawBeginExpression;
    }
  else if(c[0] == ':')
    {
    InsertText(upto, c, result);
    upto = c;
    result.push_back(cmGeneratorExpressionToken(
                        cmGeneratorExpressionToken::ColonSeparator, upto, 1));
    upto = c + 1;
    }
  else if(c[0] == ',')
    {
    InsertText(upto, c, result);
    upto = c;
    result.push_back(cmGeneratorExpressionToken(
                        cmGeneratorExpressionToken::CommaSeparator, upto, 1));
    upto = c + 1;
    }
  }
  InsertText(upto, c, result);

  return result;
}
