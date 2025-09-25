/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmGeneratorExpressionLexer.h"

cmGeneratorExpressionLexer::cmGeneratorExpressionLexer() = default;

static void InsertText(char const* upto, char const* c,
                       std::vector<cmGeneratorExpressionToken>& result)
{
  if (upto != c) {
    result.emplace_back(cmGeneratorExpressionToken::Text, upto, c - upto);
  }
}

std::vector<cmGeneratorExpressionToken> cmGeneratorExpressionLexer::Tokenize(
  std::string const& input)
{
  std::vector<cmGeneratorExpressionToken> result;

  if (input.find('$') == std::string::npos) {
    result.emplace_back(cmGeneratorExpressionToken::Text, input.c_str(),
                        input.size());
    return result;
  }

  char const* c = input.c_str();
  char const* upto = c;

  for (; *c; ++c) {
    switch (*c) {
      case '$':
        if (c[1] == '<') {
          InsertText(upto, c, result);
          result.emplace_back(cmGeneratorExpressionToken::BeginExpression, c,
                              2);
          upto = c + 2;
          ++c;
          this->SawBeginExpression = true;
        }
        break;
      case '>':
        InsertText(upto, c, result);
        result.emplace_back(cmGeneratorExpressionToken::EndExpression, c, 1);
        upto = c + 1;
        this->SawGeneratorExpression = this->SawBeginExpression;
        break;
      case ':':
        InsertText(upto, c, result);
        result.emplace_back(cmGeneratorExpressionToken::ColonSeparator, c, 1);
        upto = c + 1;
        break;
      case ',':
        InsertText(upto, c, result);
        result.emplace_back(cmGeneratorExpressionToken::CommaSeparator, c, 1);
        upto = c + 1;
        break;
      default:
        break;
    }
  }
  InsertText(upto, c, result);

  return result;
}
