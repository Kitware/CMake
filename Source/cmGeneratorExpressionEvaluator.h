/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmGeneratorExpressionEvaluator_h
#define cmGeneratorExpressionEvaluator_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <stddef.h>
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

  size_t GetLength() { return this->Length; }

private:
  const char* Content;
  size_t Length;
};

struct GeneratorExpressionContent : public cmGeneratorExpressionEvaluator
{
  GeneratorExpressionContent(const char* startContent, size_t length);

  void SetIdentifier(std::vector<cmGeneratorExpressionEvaluator*> identifier)
  {
    this->IdentifierChildren = std::move(identifier);
  }

  void SetParameters(
    std::vector<std::vector<cmGeneratorExpressionEvaluator*>> parameters)
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
    std::vector<std::vector<cmGeneratorExpressionEvaluator*>>::const_iterator
      pit) const;

private:
  std::vector<cmGeneratorExpressionEvaluator*> IdentifierChildren;
  std::vector<std::vector<cmGeneratorExpressionEvaluator*>> ParamChildren;
  const char* StartContent;
  size_t ContentLength;
};

#endif
