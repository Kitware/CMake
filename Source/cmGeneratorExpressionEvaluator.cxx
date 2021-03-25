/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmGeneratorExpressionEvaluator.h"

#include <sstream>

#include "cmGeneratorExpressionContext.h"
#include "cmGeneratorExpressionNode.h"

GeneratorExpressionContent::GeneratorExpressionContent(
  const char* startContent, size_t length)
  : StartContent(startContent)
  , ContentLength(length)
{
}

GeneratorExpressionContent::~GeneratorExpressionContent() = default;

std::string GeneratorExpressionContent::GetOriginalExpression() const
{
  return std::string(this->StartContent, this->ContentLength);
}

std::string GeneratorExpressionContent::ProcessArbitraryContent(
  const cmGeneratorExpressionNode* node, const std::string& identifier,
  cmGeneratorExpressionContext* context,
  cmGeneratorExpressionDAGChecker* dagChecker,
  std::vector<cmGeneratorExpressionEvaluatorVector>::const_iterator pit) const
{
  std::string result;

  const auto pend = this->ParamChildren.end();
  for (; pit != pend; ++pit) {
    for (const auto& pExprEval : *pit) {
      if (node->RequiresLiteralInput()) {
        if (pExprEval->GetType() != cmGeneratorExpressionEvaluator::Text) {
          reportError(context, this->GetOriginalExpression(),
                      "$<" + identifier +
                        "> expression requires literal input.");
          return std::string();
        }
      }
      result += pExprEval->Evaluate(context, dagChecker);
      if (context->HadError) {
        return std::string();
      }
    }
    if ((pit + 1) != pend) {
      result += ",";
    }
  }
  if (node->RequiresLiteralInput()) {
    std::vector<std::string> parameters;
    parameters.push_back(result);
    return node->Evaluate(parameters, context, this, dagChecker);
  }
  return result;
}

std::string GeneratorExpressionContent::Evaluate(
  cmGeneratorExpressionContext* context,
  cmGeneratorExpressionDAGChecker* dagChecker) const
{
  std::string identifier;
  {
    for (const auto& pExprEval : this->IdentifierChildren) {
      identifier += pExprEval->Evaluate(context, dagChecker);
      if (context->HadError) {
        return std::string();
      }
    }
  }

  const cmGeneratorExpressionNode* node =
    cmGeneratorExpressionNode::GetNode(identifier);

  if (!node) {
    reportError(context, this->GetOriginalExpression(),
                "Expression did not evaluate to a known generator expression");
    return std::string();
  }

  if (!node->GeneratesContent()) {
    if (node->NumExpectedParameters() == 1 &&
        node->AcceptsArbitraryContentParameter()) {
      if (this->ParamChildren.empty()) {
        reportError(context, this->GetOriginalExpression(),
                    "$<" + identifier + "> expression requires a parameter.");
      }
    } else {
      std::vector<std::string> parameters;
      this->EvaluateParameters(node, identifier, context, dagChecker,
                               parameters);
    }
    return std::string();
  }

  std::vector<std::string> parameters;
  this->EvaluateParameters(node, identifier, context, dagChecker, parameters);
  if (context->HadError) {
    return std::string();
  }

  return node->Evaluate(parameters, context, this, dagChecker);
}

std::string GeneratorExpressionContent::EvaluateParameters(
  const cmGeneratorExpressionNode* node, const std::string& identifier,
  cmGeneratorExpressionContext* context,
  cmGeneratorExpressionDAGChecker* dagChecker,
  std::vector<std::string>& parameters) const
{
  const int numExpected = node->NumExpectedParameters();
  {
    auto pit = this->ParamChildren.begin();
    const auto pend = this->ParamChildren.end();
    const bool acceptsArbitraryContent =
      node->AcceptsArbitraryContentParameter();
    int counter = 1;
    for (; pit != pend; ++pit, ++counter) {
      if (acceptsArbitraryContent && counter == numExpected) {
        parameters.push_back(this->ProcessArbitraryContent(
          node, identifier, context, dagChecker, pit));
        return std::string();
      }
      std::string parameter;
      for (const auto& pExprEval : *pit) {
        parameter += pExprEval->Evaluate(context, dagChecker);
        if (context->HadError) {
          return std::string();
        }
      }
      parameters.push_back(std::move(parameter));
    }
  }

  if ((numExpected > cmGeneratorExpressionNode::DynamicParameters &&
       static_cast<unsigned int>(numExpected) != parameters.size())) {
    if (numExpected == 0) {
      reportError(context, this->GetOriginalExpression(),
                  "$<" + identifier + "> expression requires no parameters.");
    } else if (numExpected == 1) {
      reportError(context, this->GetOriginalExpression(),
                  "$<" + identifier +
                    "> expression requires "
                    "exactly one parameter.");
    } else {
      std::ostringstream e;
      e << "$<" + identifier + "> expression requires " << numExpected
        << " comma separated parameters, but got " << parameters.size()
        << " instead.";
      reportError(context, this->GetOriginalExpression(), e.str());
    }
    return std::string();
  }

  if (numExpected == cmGeneratorExpressionNode::OneOrMoreParameters &&
      parameters.empty()) {
    reportError(context, this->GetOriginalExpression(),
                "$<" + identifier +
                  "> expression requires at least one parameter.");
  } else if (numExpected == cmGeneratorExpressionNode::TwoOrMoreParameters &&
             parameters.size() < 2) {
    reportError(context, this->GetOriginalExpression(),
                "$<" + identifier +
                  "> expression requires at least two parameters.");
  } else if (numExpected == cmGeneratorExpressionNode::OneOrZeroParameters &&
             parameters.size() > 1) {
    reportError(context, this->GetOriginalExpression(),
                "$<" + identifier +
                  "> expression requires one or zero parameters.");
  }
  return std::string();
}
