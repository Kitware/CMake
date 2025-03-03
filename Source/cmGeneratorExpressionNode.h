/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>
#include <vector>

class cmGeneratorTarget;
class cmLocalGenerator;
struct GeneratorExpressionContent;
struct cmGeneratorExpressionContext;
struct cmGeneratorExpressionDAGChecker;

struct cmGeneratorExpressionNode
{
  enum
  {
    DynamicParameters = 0,
    OneOrMoreParameters = -1,
    TwoOrMoreParameters = -2,
    ZeroOrMoreParameters = -3,
    OneOrZeroParameters = -4
  };
  virtual ~cmGeneratorExpressionNode() = default;

  virtual bool GeneratesContent() const { return true; }

  virtual bool RequiresLiteralInput() const { return false; }

  virtual bool AcceptsArbitraryContentParameter() const { return false; }

  virtual int NumExpectedParameters() const { return 1; }

  virtual bool ShouldEvaluateNextParameter(std::vector<std::string> const&,
                                           std::string&) const
  {
    return true;
  }

  virtual std::string Evaluate(
    std::vector<std::string> const& parameters,
    cmGeneratorExpressionContext* context,
    GeneratorExpressionContent const* content,
    cmGeneratorExpressionDAGChecker* dagChecker) const = 0;

  static std::string EvaluateDependentExpression(
    std::string const& prop, cmLocalGenerator* lg,
    cmGeneratorExpressionContext* context, cmGeneratorTarget const* headTarget,
    cmGeneratorExpressionDAGChecker* dagChecker,
    cmGeneratorTarget const* currentTarget);

  static cmGeneratorExpressionNode const* GetNode(
    std::string const& identifier);
};

void reportError(cmGeneratorExpressionContext* context,
                 std::string const& expr, std::string const& result);
