/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmGeneratorExpressionParser.h"

#include <cassert>
#include <cstddef>
#include <utility>

#include <cm/memory>
#include <cmext/algorithm>
#include <cmext/memory>

#include "cmGeneratorExpressionEvaluator.h"

cmGeneratorExpressionParser::cmGeneratorExpressionParser(
  std::vector<cmGeneratorExpressionToken> tokens)
  : Tokens(std::move(tokens))
{
}

void cmGeneratorExpressionParser::Parse(
  cmGeneratorExpressionEvaluatorVector& result)
{
  this->it = this->Tokens.begin();

  while (this->it != this->Tokens.end()) {
    this->ParseContent(result);
  }
}

static void extendText(
  cmGeneratorExpressionEvaluatorVector& result,
  std::vector<cmGeneratorExpressionToken>::const_iterator it)
{
  if (!result.empty() &&
      (*(result.end() - 1))->GetType() ==
        cmGeneratorExpressionEvaluator::Text) {
    cm::static_reference_cast<TextContent>(*(result.end() - 1))
      .Extend(it->Length);
  } else {
    auto textContent = cm::make_unique<TextContent>(it->Content, it->Length);
    result.push_back(std::move(textContent));
  }
}

static void extendResult(
  cmGeneratorExpressionParser::cmGeneratorExpressionEvaluatorVector& result,
  cmGeneratorExpressionParser::cmGeneratorExpressionEvaluatorVector&& contents)
{
  if (!result.empty() &&
      (*(result.end() - 1))->GetType() ==
        cmGeneratorExpressionEvaluator::Text &&
      contents.front()->GetType() == cmGeneratorExpressionEvaluator::Text) {
    cm::static_reference_cast<TextContent>(*(result.end() - 1))
      .Extend(
        cm::static_reference_cast<TextContent>(contents.front()).GetLength());
    contents.erase(contents.begin());
  }
  cm::append(result, std::move(contents));
}

void cmGeneratorExpressionParser::ParseGeneratorExpression(
  cmGeneratorExpressionEvaluatorVector& result)
{
  assert(this->it != this->Tokens.end());
  unsigned int nestedLevel = this->NestingLevel;
  ++this->NestingLevel;

  auto startToken = this->it - 1;

  cmGeneratorExpressionEvaluatorVector identifier;
  while (this->it->TokenType != cmGeneratorExpressionToken::EndExpression &&
         this->it->TokenType != cmGeneratorExpressionToken::ColonSeparator) {
    if (this->it->TokenType == cmGeneratorExpressionToken::CommaSeparator) {
      extendText(identifier, this->it);
      ++this->it;
    } else {
      this->ParseContent(identifier);
    }
    if (this->it == this->Tokens.end()) {
      break;
    }
  }
  if (identifier.empty()) {
    // ERROR
  }

  if (this->it != this->Tokens.end() &&
      this->it->TokenType == cmGeneratorExpressionToken::EndExpression) {
    auto content = cm::make_unique<GeneratorExpressionContent>(
      startToken->Content,
      this->it->Content - startToken->Content + this->it->Length);
    assert(this->it != this->Tokens.end());
    ++this->it;
    --this->NestingLevel;
    content->SetIdentifier(std::move(identifier));
    result.push_back(std::move(content));
    return;
  }

  std::vector<cmGeneratorExpressionEvaluatorVector> parameters;
  std::vector<std::vector<cmGeneratorExpressionToken>::const_iterator>
    commaTokens;
  std::vector<cmGeneratorExpressionToken>::const_iterator colonToken;

  bool emptyParamTermination = false;

  if (this->it != this->Tokens.end() &&
      this->it->TokenType == cmGeneratorExpressionToken::ColonSeparator) {
    colonToken = this->it;
    parameters.resize(parameters.size() + 1);
    assert(this->it != this->Tokens.end());
    ++this->it;
    if (this->it == this->Tokens.end()) {
      emptyParamTermination = true;
    }

    while (this->it != this->Tokens.end() &&
           this->it->TokenType == cmGeneratorExpressionToken::CommaSeparator) {
      commaTokens.push_back(this->it);
      parameters.resize(parameters.size() + 1);
      assert(this->it != this->Tokens.end());
      ++this->it;
      if (this->it == this->Tokens.end()) {
        emptyParamTermination = true;
      }
    }
    while (this->it != this->Tokens.end() &&
           this->it->TokenType == cmGeneratorExpressionToken::ColonSeparator) {
      extendText(*(parameters.end() - 1), this->it);
      assert(this->it != this->Tokens.end());
      ++this->it;
    }
    while (this->it != this->Tokens.end() &&
           this->it->TokenType != cmGeneratorExpressionToken::EndExpression) {
      this->ParseContent(*(parameters.end() - 1));
      if (this->it == this->Tokens.end()) {
        break;
      }
      while (this->it != this->Tokens.end() &&
             this->it->TokenType ==
               cmGeneratorExpressionToken::CommaSeparator) {
        commaTokens.push_back(this->it);
        parameters.resize(parameters.size() + 1);
        assert(this->it != this->Tokens.end());
        ++this->it;
        if (this->it == this->Tokens.end()) {
          emptyParamTermination = true;
        }
      }
      while (this->it != this->Tokens.end() &&
             this->it->TokenType ==
               cmGeneratorExpressionToken::ColonSeparator) {
        extendText(*(parameters.end() - 1), this->it);
        assert(this->it != this->Tokens.end());
        ++this->it;
      }
    }
    if (this->it != this->Tokens.end() &&
        this->it->TokenType == cmGeneratorExpressionToken::EndExpression) {
      --this->NestingLevel;
      assert(this->it != this->Tokens.end());
      ++this->it;
    }
  }

  if (nestedLevel != this->NestingLevel) {
    // There was a '$<' in the text, but no corresponding '>'. Rebuild to
    // treat the '$<' as having been plain text, along with the
    // corresponding : and , tokens that might have been found.
    extendText(result, startToken);
    extendResult(result, std::move(identifier));
    if (!parameters.empty()) {
      extendText(result, colonToken);

      auto pit = parameters.begin();
      const auto pend = parameters.end();
      auto commaIt = commaTokens.begin();
      assert(parameters.size() > commaTokens.size());
      for (; pit != pend; ++pit, ++commaIt) {
        if (!pit->empty() && !emptyParamTermination) {
          extendResult(result, std::move(*pit));
        }
        if (commaIt != commaTokens.end()) {
          extendText(result, *commaIt);
        } else {
          break;
        }
      }
    }
    return;
  }

  size_t contentLength =
    ((this->it - 1)->Content - startToken->Content) + (this->it - 1)->Length;
  auto content = cm::make_unique<GeneratorExpressionContent>(
    startToken->Content, contentLength);
  content->SetIdentifier(std::move(identifier));
  content->SetParameters(std::move(parameters));
  result.push_back(std::move(content));
}

void cmGeneratorExpressionParser::ParseContent(
  cmGeneratorExpressionEvaluatorVector& result)
{
  assert(this->it != this->Tokens.end());
  switch (this->it->TokenType) {
    case cmGeneratorExpressionToken::Text: {
      if (this->NestingLevel == 0) {
        if (!result.empty() &&
            (*(result.end() - 1))->GetType() ==
              cmGeneratorExpressionEvaluator::Text) {
          // A comma in 'plain text' could have split text that should
          // otherwise be continuous. Extend the last text content instead of
          // creating a new one.
          cm::static_reference_cast<TextContent>(*(result.end() - 1))
            .Extend(this->it->Length);
          assert(this->it != this->Tokens.end());
          ++this->it;
          return;
        }
      }
      auto n =
        cm::make_unique<TextContent>(this->it->Content, this->it->Length);
      result.push_back(std::move(n));
      assert(this->it != this->Tokens.end());
      ++this->it;
      return;
    }
    case cmGeneratorExpressionToken::BeginExpression:
      assert(this->it != this->Tokens.end());
      ++this->it;
      this->ParseGeneratorExpression(result);
      return;
    case cmGeneratorExpressionToken::EndExpression:
    case cmGeneratorExpressionToken::ColonSeparator:
    case cmGeneratorExpressionToken::CommaSeparator:
      if (this->NestingLevel == 0) {
        extendText(result, this->it);
      } else {
        assert(false && "Got unexpected syntax token.");
      }
      assert(this->it != this->Tokens.end());
      ++this->it;
      return;
  }
  assert(false && "Unhandled token in generator expression.");
}
