/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <cstddef>
#include <memory>
#include <string>
#include <utility>
#include <vector>

struct cmGeneratorExpressionContext;
struct cmGeneratorExpressionDAGChecker;
struct cmGeneratorExpressionNode;

struct cmGeneratorExpressionEvaluator
{
  cmGeneratorExpressionEvaluator() = default;
  virtual ~cmGeneratorExpressionEvaluator() = default;

  cmGeneratorExpressionEvaluator(cmGeneratorExpressionEvaluator const&) =
    delete;
  cmGeneratorExpressionEvaluator& operator=(
    cmGeneratorExpressionEvaluator const&) = delete;

  enum Type
  {
    Text,
    Generator
  };

  virtual Type GetType() const = 0;

  virtual std::string Evaluate(cmGeneratorExpressionContext* context,
                               cmGeneratorExpressionDAGChecker*) const = 0;
};

using cmGeneratorExpressionEvaluatorVector =
  std::vector<std::unique_ptr<cmGeneratorExpressionEvaluator>>;

struct TextContent : public cmGeneratorExpressionEvaluator
{
  TextContent(const char* start, size_t length)
    : Content(start)
    , Length(length)
  {
  }

  std::string Evaluate(cmGeneratorExpressionContext*,
                       cmGeneratorExpressionDAGChecker*) const override
  {
    return std::string(this->Content, this->Length);
  }

  Type GetType() const override
  {
    return cmGeneratorExpressionEvaluator::Text;
  }

  void Extend(size_t length) { this->Length += length; }

  size_t GetLength() const { return this->Length; }

private:
  const char* Content;
  size_t Length;
};

struct GeneratorExpressionContent : public cmGeneratorExpressionEvaluator
{
  GeneratorExpressionContent(const char* startContent, size_t length);

  void SetIdentifier(cmGeneratorExpressionEvaluatorVector&& identifier)
  {
    this->IdentifierChildren = std::move(identifier);
  }

  void SetParameters(
    std::vector<cmGeneratorExpressionEvaluatorVector>&& parameters)
  {
    this->ParamChildren = std::move(parameters);
  }

  Type GetType() const override
  {
    return cmGeneratorExpressionEvaluator::Generator;
  }

  std::string Evaluate(cmGeneratorExpressionContext* context,
                       cmGeneratorExpressionDAGChecker*) const override;

  std::string GetOriginalExpression() const;

  ~GeneratorExpressionContent() override;

private:
  std::string EvaluateParameters(const cmGeneratorExpressionNode* node,
                                 const std::string& identifier,
                                 cmGeneratorExpressionContext* context,
                                 cmGeneratorExpressionDAGChecker* dagChecker,
                                 std::vector<std::string>& parameters) const;

  std::string ProcessArbitraryContent(
    const cmGeneratorExpressionNode* node, const std::string& identifier,
    cmGeneratorExpressionContext* context,
    cmGeneratorExpressionDAGChecker* dagChecker,
    std::vector<cmGeneratorExpressionEvaluatorVector>::const_iterator pit)
    const;

  cmGeneratorExpressionEvaluatorVector IdentifierChildren;
  std::vector<cmGeneratorExpressionEvaluatorVector> ParamChildren;
  const char* StartContent;
  size_t ContentLength;
};
