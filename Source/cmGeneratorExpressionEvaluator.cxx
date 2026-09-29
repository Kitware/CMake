/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmGeneratorExpressionEvaluator.h"

#include <iterator>

#include <cm/string_view>
#include <cmext/string_view>

#ifndef CMAKE_BOOTSTRAP
#  include <cm3p/json/value.h>
#endif
#include "cmGenExContext.h"
#include "cmGenExEvaluation.h"
#include "cmGeneratorExpressionNode.h"
#include "cmLocalGenerator.h"
#include "cmStringAlgorithms.h"
#include "cmake.h"

GeneratorExpressionContent::GeneratorExpressionContent(
  char const* startContent, size_t length)
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
  cmGeneratorExpressionNode const* node, std::string const& identifier,
  cm::GenEx::Evaluation* eval, cmGeneratorExpressionDAGChecker* dagChecker,
  std::vector<cmGeneratorExpressionEvaluatorVector>::const_iterator pit) const
{
  std::string result;

  auto const pend = this->ParamChildren.end();
  for (; pit != pend; ++pit) {
    for (auto const& pExprEval : *pit) {
      if (node->RequiresLiteralInput()) {
        if (pExprEval->GetType() != cmGeneratorExpressionEvaluator::Text) {
          reportError(eval, this->GetOriginalExpression(),
                      cmStrCat("$<", identifier,
                               "> expression requires literal input."));
          return std::string();
        }
      }
      result += pExprEval->Evaluate(eval, dagChecker);
      if (eval->HadError) {
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
    return node->Evaluate(parameters, eval, this, dagChecker);
  }
  return result;
}

std::string GeneratorExpressionContent::Evaluate(
  cm::GenEx::Evaluation* eval,
  cmGeneratorExpressionDAGChecker* dagChecker) const
{
#ifndef CMAKE_BOOTSTRAP
  auto evalProfilingRAII =
    eval->Context.LG->GetCMakeInstance()->CreateProfilingEntry(
      "genex_eval", this->GetOriginalExpression());
#endif

  std::string identifier;
  {
    for (auto const& pExprEval : this->IdentifierChildren) {
      identifier += pExprEval->Evaluate(eval, dagChecker);
      if (eval->HadError) {
        return std::string();
      }
    }
  }

  cmGeneratorExpressionNode const* node =
    cmGeneratorExpressionNode::GetNode(identifier);

  if (!node) {
    reportError(eval, this->GetOriginalExpression(),
                "Expression did not evaluate to a known generator expression");
    return std::string();
  }

  if (!node->GeneratesContent()) {
    if (node->NumExpectedParameters() == 1 &&
        node->AcceptsArbitraryContentParameter()) {
      if (this->ParamChildren.empty()) {
        reportError(
          eval, this->GetOriginalExpression(),
          cmStrCat("$<", identifier, "> expression requires a parameter."));
      }
    } else {
      std::vector<std::string> parameters;
      this->EvaluateParameters(node, identifier, eval, dagChecker, parameters);
    }
    return std::string();
  }

  std::vector<std::string> parameters;
  this->EvaluateParameters(node, identifier, eval, dagChecker, parameters);
  if (eval->HadError) {
    return std::string();
  }

  if (!parameters.empty()) {
    cm::string_view ARGS_BEGIN = "<ARGS>"_s;
    cm::string_view ARGS_END = "</ARGS>"_s;

    std::vector<std::string> args;
    // split any parameter with the prefix <ARGS>:
    for (auto const& param : parameters) {
      if (cmHasPrefix(param, ARGS_BEGIN) && cmHasSuffix(param, ARGS_END)) {
        auto values = param.substr(ARGS_BEGIN.length(),
                                   param.length() - ARGS_BEGIN.length() -
                                     ARGS_END.length());
        if (values.empty()) {
          args.push_back(std::move(values));
          continue;
        }

        std::vector<std::string> tokens =
          cmTokenize(values, ',', cmTokenizerMode::New);
        args.insert(args.end(), std::make_move_iterator(tokens.begin()),
                    std::make_move_iterator(tokens.end()));
      } else {
        args.push_back(param);
      }
    }
    parameters = std::move(args);
  }

  {
#ifndef CMAKE_BOOTSTRAP
    auto execProfilingRAII =
      eval->Context.LG->GetCMakeInstance()->CreateProfilingEntry(
        "genex_exec", identifier, [&parameters]() -> Json::Value {
          Json::Value args = Json::objectValue;
          if (!parameters.empty()) {
            args["genexArgs"] = Json::arrayValue;
            for (auto const& parameter : parameters) {
              args["genexArgs"].append(parameter);
            }
          }
          return args;
        });
#endif

    return node->Evaluate(parameters, eval, this, dagChecker);
  }
}

std::string GeneratorExpressionContent::EvaluateParameters(
  cmGeneratorExpressionNode const* node, std::string const& identifier,
  cm::GenEx::Evaluation* eval, cmGeneratorExpressionDAGChecker* dagChecker,
  std::vector<std::string>& parameters) const
{
  int const numExpected = node->NumExpectedParameters();
  {
    auto pit = this->ParamChildren.begin();
    auto const pend = this->ParamChildren.end();
    bool const acceptsArbitraryContent =
      node->AcceptsArbitraryContentParameter();
    int counter = 1;
    for (; pit != pend; ++pit, ++counter) {
      if (acceptsArbitraryContent && counter == numExpected) {
        parameters.push_back(this->ProcessArbitraryContent(
          node, identifier, eval, dagChecker, pit));
        return std::string();
      }
      std::string parameter;
      if (node->ShouldEvaluateNextParameter(parameters, parameter)) {
        for (auto const& pExprEval : *pit) {
          parameter += pExprEval->Evaluate(eval, dagChecker);
          if (eval->HadError) {
            return std::string();
          }
        }
      }
      parameters.push_back(std::move(parameter));
    }
  }

  if ((numExpected > cmGeneratorExpressionNode::DynamicParameters &&
       static_cast<unsigned int>(numExpected) != parameters.size())) {
    if (numExpected == 0) {
      reportError(
        eval, this->GetOriginalExpression(),
        cmStrCat("$<", identifier, "> expression requires no parameters."));
    } else if (numExpected == 1) {
      reportError(eval, this->GetOriginalExpression(),
                  cmStrCat("$<", identifier,
                           "> expression requires "
                           "exactly one parameter."));
    } else {
      std::string e =
        cmStrCat("$<", identifier, "> expression requires ", numExpected,
                 " comma separated parameters, but got ", parameters.size(),
                 " instead.");
      reportError(eval, this->GetOriginalExpression(), e);
    }
    return std::string();
  }

  if (numExpected == cmGeneratorExpressionNode::OneOrMoreParameters &&
      parameters.empty()) {
    reportError(eval, this->GetOriginalExpression(),
                cmStrCat("$<", identifier,
                         "> expression requires at least one parameter."));
  } else if (numExpected == cmGeneratorExpressionNode::TwoOrMoreParameters &&
             parameters.size() < 2) {
    reportError(eval, this->GetOriginalExpression(),
                cmStrCat("$<", identifier,
                         "> expression requires at least two parameters."));
  } else if (numExpected == cmGeneratorExpressionNode::OneOrZeroParameters &&
             parameters.size() > 1) {
    reportError(eval, this->GetOriginalExpression(),
                cmStrCat("$<", identifier,
                         "> expression requires one or zero parameters."));
  }
  return std::string();
}
