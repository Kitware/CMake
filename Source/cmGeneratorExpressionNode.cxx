/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmGeneratorExpressionNode.h"

#include "cmAlgorithms.h"
#include "cmGeneratorExpression.h"
#include "cmGeneratorExpressionContext.h"
#include "cmGeneratorExpressionDAGChecker.h"
#include "cmGeneratorExpressionEvaluator.h"
#include "cmGeneratorTarget.h"
#include "cmGlobalGenerator.h"
#include "cmLinkItem.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmOutputConverter.h"
#include "cmPolicies.h"
#include "cmState.h"
#include "cmStateSnapshot.h"
#include "cmStateTypes.h"
#include "cmSystemTools.h"
#include "cmTarget.h"
#include "cmake.h"

#include "cmsys/RegularExpression.hxx"
#include "cmsys/String.h"
#include <algorithm>
#include <assert.h>
#include <errno.h>
#include <map>
#include <memory> // IWYU pragma: keep
#include <set>
#include <sstream>
#include <stdlib.h>
#include <string.h>
#include <utility>

std::string cmGeneratorExpressionNode::EvaluateDependentExpression(
  std::string const& prop, cmLocalGenerator* lg,
  cmGeneratorExpressionContext* context, cmGeneratorTarget const* headTarget,
  cmGeneratorTarget const* currentTarget,
  cmGeneratorExpressionDAGChecker* dagChecker)
{
  cmGeneratorExpression ge(context->Backtrace);
  std::unique_ptr<cmCompiledGeneratorExpression> cge = ge.Parse(prop);
  cge->SetEvaluateForBuildsystem(context->EvaluateForBuildsystem);
  std::string result =
    cge->Evaluate(lg, context->Config, context->Quiet, headTarget,
                  currentTarget, dagChecker, context->Language);
  if (cge->GetHadContextSensitiveCondition()) {
    context->HadContextSensitiveCondition = true;
  }
  if (cge->GetHadHeadSensitiveCondition()) {
    context->HadHeadSensitiveCondition = true;
  }
  return result;
}

static const struct ZeroNode : public cmGeneratorExpressionNode
{
  ZeroNode() {} // NOLINT(modernize-use-equals-default)

  bool GeneratesContent() const override { return false; }

  bool AcceptsArbitraryContentParameter() const override { return true; }

  std::string Evaluate(
    const std::vector<std::string>& /*parameters*/,
    cmGeneratorExpressionContext* /*context*/,
    const GeneratorExpressionContent* /*content*/,
    cmGeneratorExpressionDAGChecker* /*dagChecker*/) const override
  {
    return std::string();
  }
} zeroNode;

static const struct OneNode : public cmGeneratorExpressionNode
{
  OneNode() {} // NOLINT(modernize-use-equals-default)

  bool AcceptsArbitraryContentParameter() const override { return true; }

  std::string Evaluate(
    const std::vector<std::string>& parameters,
    cmGeneratorExpressionContext* /*context*/,
    const GeneratorExpressionContent* /*content*/,
    cmGeneratorExpressionDAGChecker* /*dagChecker*/) const override
  {
    return parameters.front();
  }
} oneNode;

static const struct OneNode buildInterfaceNode;

static const struct ZeroNode installInterfaceNode;

#define BOOLEAN_OP_NODE(OPNAME, OP, SUCCESS_VALUE, FAILURE_VALUE)             \
  static const struct OP##Node : public cmGeneratorExpressionNode             \
  {                                                                           \
    OP##Node() {} /* NOLINT(modernize-use-equals-default) */                  \
    virtual int NumExpectedParameters() const { return OneOrMoreParameters; } \
                                                                              \
    std::string Evaluate(const std::vector<std::string>& parameters,          \
                         cmGeneratorExpressionContext* context,               \
                         const GeneratorExpressionContent* content,           \
                         cmGeneratorExpressionDAGChecker*) const              \
    {                                                                         \
      for (std::string const& param : parameters) {                           \
        if (param == #FAILURE_VALUE) {                                        \
          return #FAILURE_VALUE;                                              \
        }                                                                     \
        if (param != #SUCCESS_VALUE) {                                        \
          reportError(context, content->GetOriginalExpression(),              \
                      "Parameters to $<" #OP                                  \
                      "> must resolve to either '0' or '1'.");                \
          return std::string();                                               \
        }                                                                     \
      }                                                                       \
      return #SUCCESS_VALUE;                                                  \
    }                                                                         \
  } OPNAME;

BOOLEAN_OP_NODE(andNode, AND, 1, 0)
BOOLEAN_OP_NODE(orNode, OR, 0, 1)

#undef BOOLEAN_OP_NODE

static const struct NotNode : public cmGeneratorExpressionNode
{
  NotNode() {} // NOLINT(modernize-use-equals-default)

  std::string Evaluate(
    const std::vector<std::string>& parameters,
    cmGeneratorExpressionContext* context,
    const GeneratorExpressionContent* content,
    cmGeneratorExpressionDAGChecker* /*dagChecker*/) const override
  {
    if (parameters.front() != "0" && parameters.front() != "1") {
      reportError(
        context, content->GetOriginalExpression(),
        "$<NOT> parameter must resolve to exactly one '0' or '1' value.");
      return std::string();
    }
    return parameters.front() == "0" ? "1" : "0";
  }
} notNode;

static const struct BoolNode : public cmGeneratorExpressionNode
{
  BoolNode() {} // NOLINT(modernize-use-equals-default)

  int NumExpectedParameters() const override { return 1; }

  std::string Evaluate(
    const std::vector<std::string>& parameters,
    cmGeneratorExpressionContext* /*context*/,
    const GeneratorExpressionContent* /*content*/,
    cmGeneratorExpressionDAGChecker* /*dagChecker*/) const override
  {
    return !cmSystemTools::IsOff(parameters.front()) ? "1" : "0";
  }
} boolNode;

static const struct IfNode : public cmGeneratorExpressionNode
{
  IfNode() {} // NOLINT(modernize-use-equals-default)

  int NumExpectedParameters() const override { return 3; }

  std::string Evaluate(const std::vector<std::string>& parameters,
                       cmGeneratorExpressionContext* context,
                       const GeneratorExpressionContent* content,
                       cmGeneratorExpressionDAGChecker*) const override
  {
    if (parameters[0] != "1" && parameters[0] != "0") {
      reportError(context, content->GetOriginalExpression(),
                  "First parameter to $<IF> must resolve to exactly one '0' "
                  "or '1' value.");
      return std::string();
    }
    return parameters[0] == "1" ? parameters[1] : parameters[2];
  }
} ifNode;

static const struct StrEqualNode : public cmGeneratorExpressionNode
{
  StrEqualNode() {} // NOLINT(modernize-use-equals-default)

  int NumExpectedParameters() const override { return 2; }

  std::string Evaluate(
    const std::vector<std::string>& parameters,
    cmGeneratorExpressionContext* /*context*/,
    const GeneratorExpressionContent* /*content*/,
    cmGeneratorExpressionDAGChecker* /*dagChecker*/) const override
  {
    return parameters.front() == parameters[1] ? "1" : "0";
  }
} strEqualNode;

static const struct EqualNode : public cmGeneratorExpressionNode
{
  EqualNode() {} // NOLINT(modernize-use-equals-default)

  int NumExpectedParameters() const override { return 2; }

  std::string Evaluate(
    const std::vector<std::string>& parameters,
    cmGeneratorExpressionContext* context,
    const GeneratorExpressionContent* content,
    cmGeneratorExpressionDAGChecker* /*dagChecker*/) const override
  {
    char* pEnd;

    int base = 0;
    bool flipSign = false;

    const char* lhs = parameters[0].c_str();
    if (cmHasLiteralPrefix(lhs, "0b") || cmHasLiteralPrefix(lhs, "0B")) {
      base = 2;
      lhs += 2;
    }
    if (cmHasLiteralPrefix(lhs, "-0b") || cmHasLiteralPrefix(lhs, "-0B")) {
      base = 2;
      lhs += 3;
      flipSign = true;
    }
    if (cmHasLiteralPrefix(lhs, "+0b") || cmHasLiteralPrefix(lhs, "+0B")) {
      base = 2;
      lhs += 3;
    }

    long lnum = strtol(lhs, &pEnd, base);
    if (pEnd == lhs || *pEnd != '\0' || errno == ERANGE) {
      reportError(context, content->GetOriginalExpression(),
                  "$<EQUAL> parameter " + parameters[0] +
                    " is not a valid integer.");
      return std::string();
    }

    if (flipSign) {
      lnum = -lnum;
    }

    base = 0;
    flipSign = false;

    const char* rhs = parameters[1].c_str();
    if (cmHasLiteralPrefix(rhs, "0b") || cmHasLiteralPrefix(rhs, "0B")) {
      base = 2;
      rhs += 2;
    }
    if (cmHasLiteralPrefix(rhs, "-0b") || cmHasLiteralPrefix(rhs, "-0B")) {
      base = 2;
      rhs += 3;
      flipSign = true;
    }
    if (cmHasLiteralPrefix(rhs, "+0b") || cmHasLiteralPrefix(rhs, "+0B")) {
      base = 2;
      rhs += 3;
    }

    long rnum = strtol(rhs, &pEnd, base);
    if (pEnd == rhs || *pEnd != '\0' || errno == ERANGE) {
      reportError(context, content->GetOriginalExpression(),
                  "$<EQUAL> parameter " + parameters[1] +
                    " is not a valid integer.");
      return std::string();
    }

    if (flipSign) {
      rnum = -rnum;
    }

    return lnum == rnum ? "1" : "0";
  }
} equalNode;

static const struct InListNode : public cmGeneratorExpressionNode
{
  InListNode() {} // NOLINT(modernize-use-equals-default)

  int NumExpectedParameters() const override { return 2; }

  std::string Evaluate(
    const std::vector<std::string>& parameters,
    cmGeneratorExpressionContext* context,
    const GeneratorExpressionContent* /*content*/,
    cmGeneratorExpressionDAGChecker* /*dagChecker*/) const override
  {
    std::vector<std::string> values, checkValues;
    bool check = false;
    switch (context->LG->GetPolicyStatus(cmPolicies::CMP0085)) {
      case cmPolicies::WARN:
        if (parameters.front().empty()) {
          check = true;
          cmSystemTools::ExpandListArgument(parameters[1], checkValues, true);
        }
        CM_FALLTHROUGH;
      case cmPolicies::OLD:
        cmSystemTools::ExpandListArgument(parameters[1], values);
        if (check && values != checkValues) {
          std::ostringstream e;
          e << cmPolicies::GetPolicyWarning(cmPolicies::CMP0085)
            << "\nSearch Item:\n  \"" << parameters.front()
            << "\"\nList:\n  \"" << parameters[1] << "\"\n";
          context->LG->GetCMakeInstance()->IssueMessage(
            MessageType ::AUTHOR_WARNING, e.str(), context->Backtrace);
          return "0";
        }
        if (values.empty()) {
          return "0";
        }
        break;
      case cmPolicies::REQUIRED_IF_USED:
      case cmPolicies::REQUIRED_ALWAYS:
      case cmPolicies::NEW:
        cmSystemTools::ExpandListArgument(parameters[1], values, true);
        break;
    }

    return std::find(values.cbegin(), values.cend(), parameters.front()) ==
        values.cend()
      ? "0"
      : "1";
  }
} inListNode;

static const struct TargetExistsNode : public cmGeneratorExpressionNode
{
  TargetExistsNode() {} // NOLINT(modernize-use-equals-default)

  int NumExpectedParameters() const override { return 1; }

  std::string Evaluate(
    const std::vector<std::string>& parameters,
    cmGeneratorExpressionContext* context,
    const GeneratorExpressionContent* content,
    cmGeneratorExpressionDAGChecker* /*dagChecker*/) const override
  {
    if (parameters.size() != 1) {
      reportError(context, content->GetOriginalExpression(),
                  "$<TARGET_EXISTS:...> expression requires one parameter");
      return std::string();
    }

    std::string targetName = parameters.front();
    if (targetName.empty() ||
        !cmGeneratorExpression::IsValidTargetName(targetName)) {
      reportError(context, content->GetOriginalExpression(),
                  "$<TARGET_EXISTS:tgt> expression requires a non-empty "
                  "valid target name.");
      return std::string();
    }

    return context->LG->GetMakefile()->FindTargetToUse(targetName) ? "1" : "0";
  }
} targetExistsNode;

static const struct TargetNameIfExistsNode : public cmGeneratorExpressionNode
{
  TargetNameIfExistsNode() {} // NOLINT(modernize-use-equals-default)

  int NumExpectedParameters() const override { return 1; }

  std::string Evaluate(
    const std::vector<std::string>& parameters,
    cmGeneratorExpressionContext* context,
    const GeneratorExpressionContent* content,
    cmGeneratorExpressionDAGChecker* /*dagChecker*/) const override
  {
    if (parameters.size() != 1) {
      reportError(context, content->GetOriginalExpression(),
                  "$<TARGET_NAME_IF_EXISTS:...> expression requires one "
                  "parameter");
      return std::string();
    }

    std::string targetName = parameters.front();
    if (targetName.empty() ||
        !cmGeneratorExpression::IsValidTargetName(targetName)) {
      reportError(context, content->GetOriginalExpression(),
                  "$<TARGET_NAME_IF_EXISTS:tgt> expression requires a "
                  "non-empty valid target name.");
      return std::string();
    }

    return context->LG->GetMakefile()->FindTargetToUse(targetName)
      ? targetName
      : std::string();
  }
} targetNameIfExistsNode;

struct GenexEvaluator : public cmGeneratorExpressionNode
{
  GenexEvaluator() {} // NOLINT(modernize-use-equals-default)

protected:
  std::string EvaluateExpression(
    const std::string& genexOperator, const std::string& expression,
    cmGeneratorExpressionContext* context,
    const GeneratorExpressionContent* content,
    cmGeneratorExpressionDAGChecker* dagCheckerParent) const
  {
    if (context->HeadTarget) {
      cmGeneratorExpressionDAGChecker dagChecker(
        context->Backtrace, context->HeadTarget,
        genexOperator + ":" + expression, content, dagCheckerParent);
      switch (dagChecker.Check()) {
        case cmGeneratorExpressionDAGChecker::SELF_REFERENCE:
        case cmGeneratorExpressionDAGChecker::CYCLIC_REFERENCE: {
          dagChecker.ReportError(context, content->GetOriginalExpression());
          return std::string();
        }
        case cmGeneratorExpressionDAGChecker::ALREADY_SEEN:
        case cmGeneratorExpressionDAGChecker::DAG:
          break;
      }

      return this->EvaluateDependentExpression(
        expression, context->LG, context, context->HeadTarget,
        context->CurrentTarget, &dagChecker);
    }

    return this->EvaluateDependentExpression(
      expression, context->LG, context, context->HeadTarget,
      context->CurrentTarget, dagCheckerParent);
  }
};

static const struct TargetGenexEvalNode : public GenexEvaluator
{
  TargetGenexEvalNode() {} // NOLINT(modernize-use-equals-default)

  int NumExpectedParameters() const override { return 2; }

  bool AcceptsArbitraryContentParameter() const override { return true; }

  std::string Evaluate(
    const std::vector<std::string>& parameters,
    cmGeneratorExpressionContext* context,
    const GeneratorExpressionContent* content,
    cmGeneratorExpressionDAGChecker* dagCheckerParent) const override
  {
    const std::string& targetName = parameters.front();
    if (targetName.empty() ||
        !cmGeneratorExpression::IsValidTargetName(targetName)) {
      reportError(context, content->GetOriginalExpression(),
                  "$<TARGET_GENEX_EVAL:tgt, ...> expression requires a "
                  "non-empty valid target name.");
      return std::string();
    }

    const auto* target = context->LG->FindGeneratorTargetToUse(targetName);
    if (!target) {
      std::ostringstream e;
      e << "$<TARGET_GENEX_EVAL:tgt, ...> target \"" << targetName
        << "\" not found.";
      reportError(context, content->GetOriginalExpression(), e.str());
      return std::string();
    }

    const std::string& expression = parameters[1];
    if (expression.empty()) {
      return expression;
    }

    cmGeneratorExpressionContext targetContext(
      context->LG, context->Config, context->Quiet, target, target,
      context->EvaluateForBuildsystem, context->Backtrace, context->Language);

    return this->EvaluateExpression("TARGET_GENEX_EVAL", expression,
                                    &targetContext, content, dagCheckerParent);
  }
} targetGenexEvalNode;

static const struct GenexEvalNode : public GenexEvaluator
{
  GenexEvalNode() {} // NOLINT(modernize-use-equals-default)

  int NumExpectedParameters() const override { return 1; }

  bool AcceptsArbitraryContentParameter() const override { return true; }

  std::string Evaluate(
    const std::vector<std::string>& parameters,
    cmGeneratorExpressionContext* context,
    const GeneratorExpressionContent* content,
    cmGeneratorExpressionDAGChecker* dagCheckerParent) const override
  {
    const std::string& expression = parameters[0];
    if (expression.empty()) {
      return expression;
    }

    return this->EvaluateExpression("GENEX_EVAL", expression, context, content,
                                    dagCheckerParent);
  }
} genexEvalNode;

static const struct LowerCaseNode : public cmGeneratorExpressionNode
{
  LowerCaseNode() {} // NOLINT(modernize-use-equals-default)

  bool AcceptsArbitraryContentParameter() const override { return true; }

  std::string Evaluate(
    const std::vector<std::string>& parameters,
    cmGeneratorExpressionContext* /*context*/,
    const GeneratorExpressionContent* /*content*/,
    cmGeneratorExpressionDAGChecker* /*dagChecker*/) const override
  {
    return cmSystemTools::LowerCase(parameters.front());
  }
} lowerCaseNode;

static const struct UpperCaseNode : public cmGeneratorExpressionNode
{
  UpperCaseNode() {} // NOLINT(modernize-use-equals-default)

  bool AcceptsArbitraryContentParameter() const override { return true; }

  std::string Evaluate(
    const std::vector<std::string>& parameters,
    cmGeneratorExpressionContext* /*context*/,
    const GeneratorExpressionContent* /*content*/,
    cmGeneratorExpressionDAGChecker* /*dagChecker*/) const override
  {
    return cmSystemTools::UpperCase(parameters.front());
  }
} upperCaseNode;

static const struct MakeCIdentifierNode : public cmGeneratorExpressionNode
{
  MakeCIdentifierNode() {} // NOLINT(modernize-use-equals-default)

  bool AcceptsArbitraryContentParameter() const override { return true; }

  std::string Evaluate(
    const std::vector<std::string>& parameters,
    cmGeneratorExpressionContext* /*context*/,
    const GeneratorExpressionContent* /*content*/,
    cmGeneratorExpressionDAGChecker* /*dagChecker*/) const override
  {
    return cmSystemTools::MakeCidentifier(parameters.front());
  }
} makeCIdentifierNode;

static const struct Angle_RNode : public cmGeneratorExpressionNode
{
  Angle_RNode() {} // NOLINT(modernize-use-equals-default)

  int NumExpectedParameters() const override { return 0; }

  std::string Evaluate(
    const std::vector<std::string>& /*parameters*/,
    cmGeneratorExpressionContext* /*context*/,
    const GeneratorExpressionContent* /*content*/,
    cmGeneratorExpressionDAGChecker* /*dagChecker*/) const override
  {
    return ">";
  }
} angle_rNode;

static const struct CommaNode : public cmGeneratorExpressionNode
{
  CommaNode() {} // NOLINT(modernize-use-equals-default)

  int NumExpectedParameters() const override { return 0; }

  std::string Evaluate(
    const std::vector<std::string>& /*parameters*/,
    cmGeneratorExpressionContext* /*context*/,
    const GeneratorExpressionContent* /*content*/,
    cmGeneratorExpressionDAGChecker* /*dagChecker*/) const override
  {
    return ",";
  }
} commaNode;

static const struct SemicolonNode : public cmGeneratorExpressionNode
{
  SemicolonNode() {} // NOLINT(modernize-use-equals-default)

  int NumExpectedParameters() const override { return 0; }

  std::string Evaluate(
    const std::vector<std::string>& /*parameters*/,
    cmGeneratorExpressionContext* /*context*/,
    const GeneratorExpressionContent* /*content*/,
    cmGeneratorExpressionDAGChecker* /*dagChecker*/) const override
  {
    return ";";
  }
} semicolonNode;

struct CompilerIdNode : public cmGeneratorExpressionNode
{
  CompilerIdNode() {} // NOLINT(modernize-use-equals-default)

  int NumExpectedParameters() const override { return OneOrZeroParameters; }

  std::string EvaluateWithLanguage(const std::vector<std::string>& parameters,
                                   cmGeneratorExpressionContext* context,
                                   const GeneratorExpressionContent* content,
                                   cmGeneratorExpressionDAGChecker* /*unused*/,
                                   const std::string& lang) const
  {
    std::string const& compilerId =
      context->LG->GetMakefile()->GetSafeDefinition("CMAKE_" + lang +
                                                    "_COMPILER_ID");
    if (parameters.empty()) {
      return compilerId;
    }
    static cmsys::RegularExpression compilerIdValidator("^[A-Za-z0-9_]*$");
    if (!compilerIdValidator.find(parameters.front())) {
      reportError(context, content->GetOriginalExpression(),
                  "Expression syntax not recognized.");
      return std::string();
    }
    if (compilerId.empty()) {
      return parameters.front().empty() ? "1" : "0";
    }

    if (strcmp(parameters.front().c_str(), compilerId.c_str()) == 0) {
      return "1";
    }

    if (cmsysString_strcasecmp(parameters.front().c_str(),
                               compilerId.c_str()) == 0) {
      switch (context->LG->GetPolicyStatus(cmPolicies::CMP0044)) {
        case cmPolicies::WARN: {
          std::ostringstream e;
          e << cmPolicies::GetPolicyWarning(cmPolicies::CMP0044);
          context->LG->GetCMakeInstance()->IssueMessage(
            MessageType::AUTHOR_WARNING, e.str(), context->Backtrace);
          CM_FALLTHROUGH;
        }
        case cmPolicies::OLD:
          return "1";
        case cmPolicies::NEW:
        case cmPolicies::REQUIRED_ALWAYS:
        case cmPolicies::REQUIRED_IF_USED:
          break;
      }
    }
    return "0";
  }
};

static const struct CCompilerIdNode : public CompilerIdNode
{
  CCompilerIdNode() {} // NOLINT(modernize-use-equals-default)

  std::string Evaluate(
    const std::vector<std::string>& parameters,
    cmGeneratorExpressionContext* context,
    const GeneratorExpressionContent* content,
    cmGeneratorExpressionDAGChecker* dagChecker) const override
  {
    if (!context->HeadTarget) {
      reportError(
        context, content->GetOriginalExpression(),
        "$<C_COMPILER_ID> may only be used with binary targets.  It may "
        "not be used with add_custom_command or add_custom_target.");
      return std::string();
    }
    return this->EvaluateWithLanguage(parameters, context, content, dagChecker,
                                      "C");
  }
} cCompilerIdNode;

static const struct CXXCompilerIdNode : public CompilerIdNode
{
  CXXCompilerIdNode() {} // NOLINT(modernize-use-equals-default)

  std::string Evaluate(
    const std::vector<std::string>& parameters,
    cmGeneratorExpressionContext* context,
    const GeneratorExpressionContent* content,
    cmGeneratorExpressionDAGChecker* dagChecker) const override
  {
    if (!context->HeadTarget) {
      reportError(
        context, content->GetOriginalExpression(),
        "$<CXX_COMPILER_ID> may only be used with binary targets.  It may "
        "not be used with add_custom_command or add_custom_target.");
      return std::string();
    }
    return this->EvaluateWithLanguage(parameters, context, content, dagChecker,
                                      "CXX");
  }
} cxxCompilerIdNode;

static const struct CUDACompilerIdNode : public CompilerIdNode
{
  CUDACompilerIdNode() {} // NOLINT(modernize-use-equals-default)

  std::string Evaluate(
    const std::vector<std::string>& parameters,
    cmGeneratorExpressionContext* context,
    const GeneratorExpressionContent* content,
    cmGeneratorExpressionDAGChecker* dagChecker) const override
  {
    if (!context->HeadTarget) {
      reportError(
        context, content->GetOriginalExpression(),
        "$<CUDA_COMPILER_ID> may only be used with binary targets.  It may "
        "not be used with add_custom_command or add_custom_target.");
      return std::string();
    }
    return this->EvaluateWithLanguage(parameters, context, content, dagChecker,
                                      "CUDA");
  }
} cudaCompilerIdNode;

static const struct FortranCompilerIdNode : public CompilerIdNode
{
  FortranCompilerIdNode() {} // NOLINT(modernize-use-equals-default)

  std::string Evaluate(
    const std::vector<std::string>& parameters,
    cmGeneratorExpressionContext* context,
    const GeneratorExpressionContent* content,
    cmGeneratorExpressionDAGChecker* dagChecker) const override
  {
    if (!context->HeadTarget) {
      reportError(
        context, content->GetOriginalExpression(),
        "$<Fortran_COMPILER_ID> may only be used with binary targets.  It may "
        "not be used with add_custom_command or add_custom_target.");
      return std::string();
    }
    return this->EvaluateWithLanguage(parameters, context, content, dagChecker,
                                      "Fortran");
  }
} fortranCompilerIdNode;

struct CompilerVersionNode : public cmGeneratorExpressionNode
{
  CompilerVersionNode() {} // NOLINT(modernize-use-equals-default)

  int NumExpectedParameters() const override { return OneOrZeroParameters; }

  std::string EvaluateWithLanguage(const std::vector<std::string>& parameters,
                                   cmGeneratorExpressionContext* context,
                                   const GeneratorExpressionContent* content,
                                   cmGeneratorExpressionDAGChecker* /*unused*/,
                                   const std::string& lang) const
  {
    std::string const& compilerVersion =
      context->LG->GetMakefile()->GetSafeDefinition("CMAKE_" + lang +
                                                    "_COMPILER_VERSION");
    if (parameters.empty()) {
      return compilerVersion;
    }

    static cmsys::RegularExpression compilerIdValidator("^[0-9\\.]*$");
    if (!compilerIdValidator.find(parameters.front())) {
      reportError(context, content->GetOriginalExpression(),
                  "Expression syntax not recognized.");
      return std::string();
    }
    if (compilerVersion.empty()) {
      return parameters.front().empty() ? "1" : "0";
    }

    return cmSystemTools::VersionCompare(cmSystemTools::OP_EQUAL,
                                         parameters.front().c_str(),
                                         compilerVersion.c_str())
      ? "1"
      : "0";
  }
};

static const struct CCompilerVersionNode : public CompilerVersionNode
{
  CCompilerVersionNode() {} // NOLINT(modernize-use-equals-default)

  std::string Evaluate(
    const std::vector<std::string>& parameters,
    cmGeneratorExpressionContext* context,
    const GeneratorExpressionContent* content,
    cmGeneratorExpressionDAGChecker* dagChecker) const override
  {
    if (!context->HeadTarget) {
      reportError(
        context, content->GetOriginalExpression(),
        "$<C_COMPILER_VERSION> may only be used with binary targets.  It "
        "may not be used with add_custom_command or add_custom_target.");
      return std::string();
    }
    return this->EvaluateWithLanguage(parameters, context, content, dagChecker,
                                      "C");
  }
} cCompilerVersionNode;

static const struct CXXCompilerVersionNode : public CompilerVersionNode
{
  CXXCompilerVersionNode() {} // NOLINT(modernize-use-equals-default)

  std::string Evaluate(
    const std::vector<std::string>& parameters,
    cmGeneratorExpressionContext* context,
    const GeneratorExpressionContent* content,
    cmGeneratorExpressionDAGChecker* dagChecker) const override
  {
    if (!context->HeadTarget) {
      reportError(
        context, content->GetOriginalExpression(),
        "$<CXX_COMPILER_VERSION> may only be used with binary targets.  It "
        "may not be used with add_custom_command or add_custom_target.");
      return std::string();
    }
    return this->EvaluateWithLanguage(parameters, context, content, dagChecker,
                                      "CXX");
  }
} cxxCompilerVersionNode;

static const struct CUDACompilerVersionNode : public CompilerVersionNode
{
  CUDACompilerVersionNode() {} // NOLINT(modernize-use-equals-default)

  std::string Evaluate(
    const std::vector<std::string>& parameters,
    cmGeneratorExpressionContext* context,
    const GeneratorExpressionContent* content,
    cmGeneratorExpressionDAGChecker* dagChecker) const override
  {
    if (!context->HeadTarget) {
      reportError(
        context, content->GetOriginalExpression(),
        "$<CUDA_COMPILER_VERSION> may only be used with binary targets.  It "
        "may not be used with add_custom_command or add_custom_target.");
      return std::string();
    }
    return this->EvaluateWithLanguage(parameters, context, content, dagChecker,
                                      "CUDA");
  }
} cudaCompilerVersionNode;

static const struct FortranCompilerVersionNode : public CompilerVersionNode
{
  FortranCompilerVersionNode() {} // NOLINT(modernize-use-equals-default)

  std::string Evaluate(
    const std::vector<std::string>& parameters,
    cmGeneratorExpressionContext* context,
    const GeneratorExpressionContent* content,
    cmGeneratorExpressionDAGChecker* dagChecker) const override
  {
    if (!context->HeadTarget) {
      reportError(
        context, content->GetOriginalExpression(),
        "$<Fortran_COMPILER_VERSION> may only be used with binary targets.  "
        "It may not be used with add_custom_command or add_custom_target.");
      return std::string();
    }
    return this->EvaluateWithLanguage(parameters, context, content, dagChecker,
                                      "Fortran");
  }
} fortranCompilerVersionNode;

struct PlatformIdNode : public cmGeneratorExpressionNode
{
  PlatformIdNode() {} // NOLINT(modernize-use-equals-default)

  int NumExpectedParameters() const override { return OneOrZeroParameters; }

  std::string Evaluate(
    const std::vector<std::string>& parameters,
    cmGeneratorExpressionContext* context,
    const GeneratorExpressionContent* /*content*/,
    cmGeneratorExpressionDAGChecker* /*dagChecker*/) const override
  {
    std::string const& platformId =
      context->LG->GetMakefile()->GetSafeDefinition("CMAKE_SYSTEM_NAME");
    if (parameters.empty()) {
      return platformId;
    }

    if (platformId.empty()) {
      return parameters.front().empty() ? "1" : "0";
    }

    if (parameters.front() == platformId) {
      return "1";
    }
    return "0";
  }
} platformIdNode;

static const struct VersionGreaterNode : public cmGeneratorExpressionNode
{
  VersionGreaterNode() {} // NOLINT(modernize-use-equals-default)

  int NumExpectedParameters() const override { return 2; }

  std::string Evaluate(
    const std::vector<std::string>& parameters,
    cmGeneratorExpressionContext* /*context*/,
    const GeneratorExpressionContent* /*content*/,
    cmGeneratorExpressionDAGChecker* /*dagChecker*/) const override
  {
    return cmSystemTools::VersionCompare(cmSystemTools::OP_GREATER,
                                         parameters.front().c_str(),
                                         parameters[1].c_str())
      ? "1"
      : "0";
  }
} versionGreaterNode;

static const struct VersionGreaterEqNode : public cmGeneratorExpressionNode
{
  VersionGreaterEqNode() {} // NOLINT(modernize-use-equals-default)

  int NumExpectedParameters() const override { return 2; }

  std::string Evaluate(
    const std::vector<std::string>& parameters,
    cmGeneratorExpressionContext* /*context*/,
    const GeneratorExpressionContent* /*content*/,
    cmGeneratorExpressionDAGChecker* /*dagChecker*/) const override
  {
    return cmSystemTools::VersionCompare(cmSystemTools::OP_GREATER_EQUAL,
                                         parameters.front().c_str(),
                                         parameters[1].c_str())
      ? "1"
      : "0";
  }
} versionGreaterEqNode;

static const struct VersionLessNode : public cmGeneratorExpressionNode
{
  VersionLessNode() {} // NOLINT(modernize-use-equals-default)

  int NumExpectedParameters() const override { return 2; }

  std::string Evaluate(
    const std::vector<std::string>& parameters,
    cmGeneratorExpressionContext* /*context*/,
    const GeneratorExpressionContent* /*content*/,
    cmGeneratorExpressionDAGChecker* /*dagChecker*/) const override
  {
    return cmSystemTools::VersionCompare(cmSystemTools::OP_LESS,
                                         parameters.front().c_str(),
                                         parameters[1].c_str())
      ? "1"
      : "0";
  }
} versionLessNode;

static const struct VersionLessEqNode : public cmGeneratorExpressionNode
{
  VersionLessEqNode() {} // NOLINT(modernize-use-equals-default)

  int NumExpectedParameters() const override { return 2; }

  std::string Evaluate(
    const std::vector<std::string>& parameters,
    cmGeneratorExpressionContext* /*context*/,
    const GeneratorExpressionContent* /*content*/,
    cmGeneratorExpressionDAGChecker* /*dagChecker*/) const override
  {
    return cmSystemTools::VersionCompare(cmSystemTools::OP_LESS_EQUAL,
                                         parameters.front().c_str(),
                                         parameters[1].c_str())
      ? "1"
      : "0";
  }
} versionLessEqNode;

static const struct VersionEqualNode : public cmGeneratorExpressionNode
{
  VersionEqualNode() {} // NOLINT(modernize-use-equals-default)

  int NumExpectedParameters() const override { return 2; }

  std::string Evaluate(
    const std::vector<std::string>& parameters,
    cmGeneratorExpressionContext* /*context*/,
    const GeneratorExpressionContent* /*content*/,
    cmGeneratorExpressionDAGChecker* /*dagChecker*/) const override
  {
    return cmSystemTools::VersionCompare(cmSystemTools::OP_EQUAL,
                                         parameters.front().c_str(),
                                         parameters[1].c_str())
      ? "1"
      : "0";
  }
} versionEqualNode;

static const struct LinkOnlyNode : public cmGeneratorExpressionNode
{
  LinkOnlyNode() {} // NOLINT(modernize-use-equals-default)

  std::string Evaluate(
    const std::vector<std::string>& parameters,
    cmGeneratorExpressionContext* context,
    const GeneratorExpressionContent* content,
    cmGeneratorExpressionDAGChecker* dagChecker) const override
  {
    if (!dagChecker) {
      reportError(context, content->GetOriginalExpression(),
                  "$<LINK_ONLY:...> may only be used for linking");
      return std::string();
    }
    if (!dagChecker->GetTransitivePropertiesOnly()) {
      return parameters.front();
    }
    return std::string();
  }
} linkOnlyNode;

static const struct ConfigurationNode : public cmGeneratorExpressionNode
{
  ConfigurationNode() {} // NOLINT(modernize-use-equals-default)

  int NumExpectedParameters() const override { return 0; }

  std::string Evaluate(
    const std::vector<std::string>& /*parameters*/,
    cmGeneratorExpressionContext* context,
    const GeneratorExpressionContent* /*content*/,
    cmGeneratorExpressionDAGChecker* /*dagChecker*/) const override
  {
    context->HadContextSensitiveCondition = true;
    return context->Config;
  }
} configurationNode;

static const struct ConfigurationTestNode : public cmGeneratorExpressionNode
{
  ConfigurationTestNode() {} // NOLINT(modernize-use-equals-default)

  int NumExpectedParameters() const override { return OneOrZeroParameters; }

  std::string Evaluate(
    const std::vector<std::string>& parameters,
    cmGeneratorExpressionContext* context,
    const GeneratorExpressionContent* content,
    cmGeneratorExpressionDAGChecker* /*dagChecker*/) const override
  {
    if (parameters.empty()) {
      return configurationNode.Evaluate(parameters, context, content, nullptr);
    }
    static cmsys::RegularExpression configValidator("^[A-Za-z0-9_]*$");
    if (!configValidator.find(parameters.front())) {
      reportError(context, content->GetOriginalExpression(),
                  "Expression syntax not recognized.");
      return std::string();
    }
    context->HadContextSensitiveCondition = true;
    if (context->Config.empty()) {
      return parameters.front().empty() ? "1" : "0";
    }

    if (cmsysString_strcasecmp(parameters.front().c_str(),
                               context->Config.c_str()) == 0) {
      return "1";
    }

    if (context->CurrentTarget && context->CurrentTarget->IsImported()) {
      const char* loc = nullptr;
      const char* imp = nullptr;
      std::string suffix;
      if (context->CurrentTarget->Target->GetMappedConfig(
            context->Config, &loc, &imp, suffix)) {
        // This imported target has an appropriate location
        // for this (possibly mapped) config.
        // Check if there is a proper config mapping for the tested config.
        std::vector<std::string> mappedConfigs;
        std::string mapProp = "MAP_IMPORTED_CONFIG_";
        mapProp += cmSystemTools::UpperCase(context->Config);
        if (const char* mapValue =
              context->CurrentTarget->GetProperty(mapProp)) {
          cmSystemTools::ExpandListArgument(cmSystemTools::UpperCase(mapValue),
                                            mappedConfigs);
          return std::find(mappedConfigs.begin(), mappedConfigs.end(),
                           cmSystemTools::UpperCase(parameters.front())) !=
              mappedConfigs.end()
            ? "1"
            : "0";
        }
      }
    }
    return "0";
  }
} configurationTestNode;

static const struct JoinNode : public cmGeneratorExpressionNode
{
  JoinNode() {} // NOLINT(modernize-use-equals-default)

  int NumExpectedParameters() const override { return 2; }

  bool AcceptsArbitraryContentParameter() const override { return true; }

  std::string Evaluate(
    const std::vector<std::string>& parameters,
    cmGeneratorExpressionContext* /*context*/,
    const GeneratorExpressionContent* /*content*/,
    cmGeneratorExpressionDAGChecker* /*dagChecker*/) const override
  {
    std::vector<std::string> list;
    cmSystemTools::ExpandListArgument(parameters.front(), list);
    return cmJoin(list, parameters[1]);
  }
} joinNode;

static const struct CompileLanguageNode : public cmGeneratorExpressionNode
{
  CompileLanguageNode() {} // NOLINT(modernize-use-equals-default)

  int NumExpectedParameters() const override { return OneOrZeroParameters; }

  std::string Evaluate(
    const std::vector<std::string>& parameters,
    cmGeneratorExpressionContext* context,
    const GeneratorExpressionContent* content,
    cmGeneratorExpressionDAGChecker* /*dagChecker*/) const override
  {
    if (context->Language.empty()) {
      reportError(
        context, content->GetOriginalExpression(),
        "$<COMPILE_LANGUAGE:...> may only be used to specify include "
        "directories, compile definitions, compile options, and to evaluate "
        "components of the file(GENERATE) command.");
      return std::string();
    }

    cmGlobalGenerator* gg = context->LG->GetGlobalGenerator();
    std::string genName = gg->GetName();
    if (genName.find("Makefiles") == std::string::npos &&
        genName.find("Ninja") == std::string::npos &&
        genName.find("Visual Studio") == std::string::npos &&
        genName.find("Xcode") == std::string::npos &&
        genName.find("Watcom WMake") == std::string::npos) {
      reportError(context, content->GetOriginalExpression(),
                  "$<COMPILE_LANGUAGE:...> not supported for this generator.");
      return std::string();
    }
    if (parameters.empty()) {
      return context->Language;
    }
    return context->Language == parameters.front() ? "1" : "0";
  }
} languageNode;

#define TRANSITIVE_PROPERTY_NAME(PROPERTY) , "INTERFACE_" #PROPERTY

static const char* targetPropertyTransitiveWhitelist[] = {
  nullptr CM_FOR_EACH_TRANSITIVE_PROPERTY_NAME(TRANSITIVE_PROPERTY_NAME)
};

#undef TRANSITIVE_PROPERTY_NAME

template <typename T>
std::string getLinkedTargetsContent(
  std::vector<T> const& libraries, cmGeneratorTarget const* target,
  cmGeneratorTarget const* headTarget, cmGeneratorExpressionContext* context,
  cmGeneratorExpressionDAGChecker* dagChecker,
  const std::string& interfacePropertyName)
{
  std::string linkedTargetsContent;
  std::string sep;
  std::string depString;
  for (T const& l : libraries) {
    // Broken code can have a target in its own link interface.
    // Don't follow such link interface entries so as not to create a
    // self-referencing loop.
    if (l.Target && l.Target != target) {
      std::string uniqueName =
        target->GetGlobalGenerator()->IndexGeneratorTargetUniquely(l.Target);
      depString += sep + "$<TARGET_PROPERTY:" + std::move(uniqueName) + "," +
        interfacePropertyName + ">";
      sep = ";";
    }
  }
  if (!depString.empty()) {
    linkedTargetsContent =
      cmGeneratorExpressionNode::EvaluateDependentExpression(
        depString, target->GetLocalGenerator(), context, headTarget, target,
        dagChecker);
  }
  linkedTargetsContent =
    cmGeneratorExpression::StripEmptyListElements(linkedTargetsContent);
  return linkedTargetsContent;
}

static const struct TargetPropertyNode : public cmGeneratorExpressionNode
{
  TargetPropertyNode() {} // NOLINT(modernize-use-equals-default)

  // This node handles errors on parameter count itself.
  int NumExpectedParameters() const override { return OneOrMoreParameters; }

  std::string Evaluate(
    const std::vector<std::string>& parameters,
    cmGeneratorExpressionContext* context,
    const GeneratorExpressionContent* content,
    cmGeneratorExpressionDAGChecker* dagCheckerParent) const override
  {
    if (parameters.size() != 1 && parameters.size() != 2) {
      reportError(
        context, content->GetOriginalExpression(),
        "$<TARGET_PROPERTY:...> expression requires one or two parameters");
      return std::string();
    }
    static cmsys::RegularExpression propertyNameValidator("^[A-Za-z0-9_]+$");

    cmGeneratorTarget const* target = context->HeadTarget;
    std::string propertyName = parameters.front();

    if (parameters.size() == 1) {
      context->HadHeadSensitiveCondition = true;
    }
    if (!target && parameters.size() == 1) {
      reportError(
        context, content->GetOriginalExpression(),
        "$<TARGET_PROPERTY:prop>  may only be used with binary targets.  "
        "It may not be used with add_custom_command or add_custom_target.  "
        "Specify the target to read a property from using the "
        "$<TARGET_PROPERTY:tgt,prop> signature instead.");
      return std::string();
    }

    if (parameters.size() == 2) {
      if (parameters.front().empty() && parameters[1].empty()) {
        reportError(
          context, content->GetOriginalExpression(),
          "$<TARGET_PROPERTY:tgt,prop> expression requires a non-empty "
          "target name and property name.");
        return std::string();
      }
      if (parameters.front().empty()) {
        reportError(
          context, content->GetOriginalExpression(),
          "$<TARGET_PROPERTY:tgt,prop> expression requires a non-empty "
          "target name.");
        return std::string();
      }

      std::string targetName = parameters.front();
      propertyName = parameters[1];
      if (!cmGeneratorExpression::IsValidTargetName(targetName)) {
        if (!propertyNameValidator.find(propertyName)) {
          ::reportError(context, content->GetOriginalExpression(),
                        "Target name and property name not supported.");
          return std::string();
        }
        ::reportError(context, content->GetOriginalExpression(),
                      "Target name not supported.");
        return std::string();
      }
      static const std::string propALIASED_TARGET = "ALIASED_TARGET";
      if (propertyName == propALIASED_TARGET) {
        if (context->LG->GetMakefile()->IsAlias(targetName)) {
          if (cmGeneratorTarget* tgt =
                context->LG->FindGeneratorTargetToUse(targetName)) {
            return tgt->GetName();
          }
        }
        return "";
      }
      target = context->LG->FindGeneratorTargetToUse(targetName);

      if (!target) {
        std::ostringstream e;
        e << "Target \"" << targetName << "\" not found.";
        reportError(context, content->GetOriginalExpression(), e.str());
        return std::string();
      }
      context->AllTargets.insert(target);
    }

    if (target == context->HeadTarget) {
      // Keep track of the properties seen while processing.
      // The evaluation of the LINK_LIBRARIES generator expressions
      // will check this to ensure that properties have one consistent
      // value for all evaluations.
      context->SeenTargetProperties.insert(propertyName);
    }
    if (propertyName == "SOURCES") {
      context->SourceSensitiveTargets.insert(target);
    }

    if (propertyName.empty()) {
      reportError(
        context, content->GetOriginalExpression(),
        "$<TARGET_PROPERTY:...> expression requires a non-empty property "
        "name.");
      return std::string();
    }

    if (!propertyNameValidator.find(propertyName)) {
      ::reportError(context, content->GetOriginalExpression(),
                    "Property name not supported.");
      return std::string();
    }

    assert(target);

    if (propertyName == "LINKER_LANGUAGE") {
      if (target->LinkLanguagePropagatesToDependents() && dagCheckerParent &&
          (dagCheckerParent->EvaluatingLinkLibraries() ||
           dagCheckerParent->EvaluatingSources())) {
        reportError(
          context, content->GetOriginalExpression(),
          "LINKER_LANGUAGE target property can not be used while evaluating "
          "link libraries for a static library");
        return std::string();
      }
      return target->GetLinkerLanguage(context->Config);
    }

    cmGeneratorExpressionDAGChecker dagChecker(
      context->Backtrace, target, propertyName, content, dagCheckerParent);

    switch (dagChecker.Check()) {
      case cmGeneratorExpressionDAGChecker::SELF_REFERENCE:
        dagChecker.ReportError(context, content->GetOriginalExpression());
        return std::string();
      case cmGeneratorExpressionDAGChecker::CYCLIC_REFERENCE:
        // No error. We just skip cyclic references.
        return std::string();
      case cmGeneratorExpressionDAGChecker::ALREADY_SEEN:
        for (size_t i = 1; i < cm::size(targetPropertyTransitiveWhitelist);
             ++i) {
          if (targetPropertyTransitiveWhitelist[i] == propertyName) {
            // No error. We're not going to find anything new here.
            return std::string();
          }
        }
      case cmGeneratorExpressionDAGChecker::DAG:
        break;
    }

    const char* prop = target->GetProperty(propertyName);

    if (dagCheckerParent) {
      if (dagCheckerParent->EvaluatingGenexExpression() ||
          dagCheckerParent->EvaluatingPICExpression()) {
        // No check required.
      } else if (dagCheckerParent->EvaluatingLinkLibraries()) {
#define TRANSITIVE_PROPERTY_COMPARE(PROPERTY)                                 \
  (#PROPERTY == propertyName || "INTERFACE_" #PROPERTY == propertyName) ||
        if (CM_FOR_EACH_TRANSITIVE_PROPERTY_NAME(
              TRANSITIVE_PROPERTY_COMPARE) false) { // NOLINT(*)
          reportError(
            context, content->GetOriginalExpression(),
            "$<TARGET_PROPERTY:...> expression in link libraries "
            "evaluation depends on target property which is transitive "
            "over the link libraries, creating a recursion.");
          return std::string();
        }
#undef TRANSITIVE_PROPERTY_COMPARE

        if (!prop) {
          return std::string();
        }
      } else {
#define ASSERT_TRANSITIVE_PROPERTY_METHOD(METHOD) dagCheckerParent->METHOD() ||

        assert(CM_FOR_EACH_TRANSITIVE_PROPERTY_METHOD(
          ASSERT_TRANSITIVE_PROPERTY_METHOD) false); // NOLINT(clang-tidy)
#undef ASSERT_TRANSITIVE_PROPERTY_METHOD
      }
    }

    std::string linkedTargetsContent;

    std::string interfacePropertyName;
    bool isInterfaceProperty = false;

#define POPULATE_INTERFACE_PROPERTY_NAME(prop)                                \
  if (propertyName == #prop) {                                                \
    interfacePropertyName = "INTERFACE_" #prop;                               \
  } else if (propertyName == "INTERFACE_" #prop) {                            \
    interfacePropertyName = "INTERFACE_" #prop;                               \
    isInterfaceProperty = true;                                               \
  } else

    CM_FOR_EACH_TRANSITIVE_PROPERTY_NAME(POPULATE_INTERFACE_PROPERTY_NAME)
    // Note that the above macro terminates with an else
    /* else */ if (cmHasLiteralPrefix(propertyName, "COMPILE_DEFINITIONS_")) {
      cmPolicies::PolicyStatus polSt =
        context->LG->GetPolicyStatus(cmPolicies::CMP0043);
      if (polSt == cmPolicies::WARN || polSt == cmPolicies::OLD) {
        interfacePropertyName = "INTERFACE_COMPILE_DEFINITIONS";
      }
    }
#undef POPULATE_INTERFACE_PROPERTY_NAME
    cmGeneratorTarget const* headTarget =
      context->HeadTarget && isInterfaceProperty ? context->HeadTarget
                                                 : target;

    if (isInterfaceProperty) {
      if (cmLinkInterfaceLibraries const* iface =
            target->GetLinkInterfaceLibraries(context->Config, headTarget,
                                              true)) {
        linkedTargetsContent =
          getLinkedTargetsContent(iface->Libraries, target, headTarget,
                                  context, &dagChecker, interfacePropertyName);
      }
    } else if (!interfacePropertyName.empty()) {
      if (cmLinkImplementationLibraries const* impl =
            target->GetLinkImplementationLibraries(context->Config)) {
        linkedTargetsContent =
          getLinkedTargetsContent(impl->Libraries, target, target, context,
                                  &dagChecker, interfacePropertyName);
      }
    }

    if (!prop) {
      if (target->IsImported() ||
          target->GetType() == cmStateEnums::INTERFACE_LIBRARY) {
        return linkedTargetsContent;
      }
      if (target->IsLinkInterfaceDependentBoolProperty(propertyName,
                                                       context->Config)) {
        context->HadContextSensitiveCondition = true;
        return target->GetLinkInterfaceDependentBoolProperty(propertyName,
                                                             context->Config)
          ? "1"
          : "0";
      }
      if (target->IsLinkInterfaceDependentStringProperty(propertyName,
                                                         context->Config)) {
        context->HadContextSensitiveCondition = true;
        const char* propContent =
          target->GetLinkInterfaceDependentStringProperty(propertyName,
                                                          context->Config);
        return propContent ? propContent : "";
      }
      if (target->IsLinkInterfaceDependentNumberMinProperty(propertyName,
                                                            context->Config)) {
        context->HadContextSensitiveCondition = true;
        const char* propContent =
          target->GetLinkInterfaceDependentNumberMinProperty(propertyName,
                                                             context->Config);
        return propContent ? propContent : "";
      }
      if (target->IsLinkInterfaceDependentNumberMaxProperty(propertyName,
                                                            context->Config)) {
        context->HadContextSensitiveCondition = true;
        const char* propContent =
          target->GetLinkInterfaceDependentNumberMaxProperty(propertyName,
                                                             context->Config);
        return propContent ? propContent : "";
      }

      return linkedTargetsContent;
    }

    if (!target->IsImported() && dagCheckerParent &&
        !dagCheckerParent->EvaluatingLinkLibraries()) {
      if (target->IsLinkInterfaceDependentNumberMinProperty(propertyName,
                                                            context->Config)) {
        context->HadContextSensitiveCondition = true;
        const char* propContent =
          target->GetLinkInterfaceDependentNumberMinProperty(propertyName,
                                                             context->Config);
        return propContent ? propContent : "";
      }
      if (target->IsLinkInterfaceDependentNumberMaxProperty(propertyName,
                                                            context->Config)) {
        context->HadContextSensitiveCondition = true;
        const char* propContent =
          target->GetLinkInterfaceDependentNumberMaxProperty(propertyName,
                                                             context->Config);
        return propContent ? propContent : "";
      }
    }
    if (!interfacePropertyName.empty()) {
      std::string result = this->EvaluateDependentExpression(
        prop, context->LG, context, headTarget, target, &dagChecker);
      if (!linkedTargetsContent.empty()) {
        result += (result.empty() ? "" : ";") + linkedTargetsContent;
      }
      return result;
    }
    return prop;
  }
} targetPropertyNode;

static const struct TargetNameNode : public cmGeneratorExpressionNode
{
  TargetNameNode() {} // NOLINT(modernize-use-equals-default)

  bool GeneratesContent() const override { return true; }

  bool AcceptsArbitraryContentParameter() const override { return true; }
  bool RequiresLiteralInput() const override { return true; }

  std::string Evaluate(
    const std::vector<std::string>& parameters,
    cmGeneratorExpressionContext* /*context*/,
    const GeneratorExpressionContent* /*content*/,
    cmGeneratorExpressionDAGChecker* /*dagChecker*/) const override
  {
    return parameters.front();
  }

  int NumExpectedParameters() const override { return 1; }

} targetNameNode;

static const struct TargetObjectsNode : public cmGeneratorExpressionNode
{
  TargetObjectsNode() {} // NOLINT(modernize-use-equals-default)

  std::string Evaluate(
    const std::vector<std::string>& parameters,
    cmGeneratorExpressionContext* context,
    const GeneratorExpressionContent* content,
    cmGeneratorExpressionDAGChecker* /*dagChecker*/) const override
  {
    std::string tgtName = parameters.front();
    cmGeneratorTarget* gt = context->LG->FindGeneratorTargetToUse(tgtName);
    if (!gt) {
      std::ostringstream e;
      e << "Objects of target \"" << tgtName
        << "\" referenced but no such target exists.";
      reportError(context, content->GetOriginalExpression(), e.str());
      return std::string();
    }
    if (gt->GetType() != cmStateEnums::OBJECT_LIBRARY) {
      std::ostringstream e;
      e << "Objects of target \"" << tgtName
        << "\" referenced but is not an OBJECT library.";
      reportError(context, content->GetOriginalExpression(), e.str());
      return std::string();
    }
    if (!context->EvaluateForBuildsystem) {
      cmGlobalGenerator* gg = context->LG->GetGlobalGenerator();
      std::string reason;
      if (!gg->HasKnownObjectFileLocation(&reason)) {
        std::ostringstream e;
        e << "The evaluation of the TARGET_OBJECTS generator expression "
             "is only suitable for consumption by CMake (limited"
          << reason
          << ").  "
             "It is not suitable for writing out elsewhere.";
        reportError(context, content->GetOriginalExpression(), e.str());
        return std::string();
      }
    }

    std::vector<std::string> objects;

    if (gt->IsImported()) {
      const char* loc = nullptr;
      const char* imp = nullptr;
      std::string suffix;
      if (gt->Target->GetMappedConfig(context->Config, &loc, &imp, suffix)) {
        cmSystemTools::ExpandListArgument(loc, objects);
      }
      context->HadContextSensitiveCondition = true;
    } else {
      gt->GetTargetObjectNames(context->Config, objects);

      std::string obj_dir;
      if (context->EvaluateForBuildsystem) {
        // Use object file directory with buildsystem placeholder.
        obj_dir = gt->ObjectDirectory;
        // Here we assume that the set of object files produced
        // by an object library does not vary with configuration
        // and do not set HadContextSensitiveCondition to true.
      } else {
        // Use object file directory with per-config location.
        obj_dir = gt->GetObjectDirectory(context->Config);
        context->HadContextSensitiveCondition = true;
      }

      for (std::string& o : objects) {
        o = obj_dir + o;
      }
    }

    // Create the cmSourceFile instances in the referencing directory.
    cmMakefile* mf = context->LG->GetMakefile();
    for (std::string& o : objects) {
      mf->AddTargetObject(tgtName, o);
    }

    return cmJoin(objects, ";");
  }
} targetObjectsNode;

static const struct CompileFeaturesNode : public cmGeneratorExpressionNode
{
  CompileFeaturesNode() {} // NOLINT(modernize-use-equals-default)

  int NumExpectedParameters() const override { return OneOrMoreParameters; }

  std::string Evaluate(
    const std::vector<std::string>& parameters,
    cmGeneratorExpressionContext* context,
    const GeneratorExpressionContent* content,
    cmGeneratorExpressionDAGChecker* dagChecker) const override
  {
    cmGeneratorTarget const* target = context->HeadTarget;
    if (!target) {
      reportError(
        context, content->GetOriginalExpression(),
        "$<COMPILE_FEATURE> may only be used with binary targets.  It may "
        "not be used with add_custom_command or add_custom_target.");
      return std::string();
    }
    context->HadHeadSensitiveCondition = true;

    typedef std::map<std::string, std::vector<std::string>> LangMap;
    static LangMap availableFeatures;

    LangMap testedFeatures;

    for (std::string const& p : parameters) {
      std::string error;
      std::string lang;
      if (!context->LG->GetMakefile()->CompileFeatureKnown(
            context->HeadTarget->Target, p, lang, &error)) {
        reportError(context, content->GetOriginalExpression(), error);
        return std::string();
      }
      testedFeatures[lang].push_back(p);

      if (availableFeatures.find(lang) == availableFeatures.end()) {
        const char* featuresKnown =
          context->LG->GetMakefile()->CompileFeaturesAvailable(lang, &error);
        if (!featuresKnown) {
          reportError(context, content->GetOriginalExpression(), error);
          return std::string();
        }
        cmSystemTools::ExpandListArgument(featuresKnown,
                                          availableFeatures[lang]);
      }
    }

    bool evalLL = dagChecker && dagChecker->EvaluatingLinkLibraries();

    for (auto const& lit : testedFeatures) {
      std::vector<std::string> const& langAvailable =
        availableFeatures[lit.first];
      const char* standardDefault = context->LG->GetMakefile()->GetDefinition(
        "CMAKE_" + lit.first + "_STANDARD_DEFAULT");
      for (std::string const& it : lit.second) {
        if (std::find(langAvailable.begin(), langAvailable.end(), it) ==
            langAvailable.end()) {
          return "0";
        }
        if (standardDefault && !*standardDefault) {
          // This compiler has no notion of language standard levels.
          // All features known for the language are always available.
          continue;
        }
        if (!context->LG->GetMakefile()->HaveStandardAvailable(
              target->Target, lit.first, it)) {
          if (evalLL) {
            const char* l = target->GetProperty(lit.first + "_STANDARD");
            if (!l) {
              l = standardDefault;
            }
            assert(l);
            context->MaxLanguageStandard[target][lit.first] = l;
          } else {
            return "0";
          }
        }
      }
    }
    return "1";
  }
} compileFeaturesNode;

static const char* targetPolicyWhitelist[] = {
  nullptr
#define TARGET_POLICY_STRING(POLICY) , #POLICY

  CM_FOR_EACH_TARGET_POLICY(TARGET_POLICY_STRING)

#undef TARGET_POLICY_STRING
};

cmPolicies::PolicyStatus statusForTarget(cmGeneratorTarget const* tgt,
                                         const char* policy)
{
#define RETURN_POLICY(POLICY)                                                 \
  if (strcmp(policy, #POLICY) == 0) {                                         \
    return tgt->GetPolicyStatus##POLICY();                                    \
  }

  CM_FOR_EACH_TARGET_POLICY(RETURN_POLICY)

#undef RETURN_POLICY

  assert(false && "Unreachable code. Not a valid policy");
  return cmPolicies::WARN;
}

cmPolicies::PolicyID policyForString(const char* policy_id)
{
#define RETURN_POLICY_ID(POLICY_ID)                                           \
  if (strcmp(policy_id, #POLICY_ID) == 0) {                                   \
    return cmPolicies::POLICY_ID;                                             \
  }

  CM_FOR_EACH_TARGET_POLICY(RETURN_POLICY_ID)

#undef RETURN_POLICY_ID

  assert(false && "Unreachable code. Not a valid policy");
  return cmPolicies::CMP0002;
}

static const struct TargetPolicyNode : public cmGeneratorExpressionNode
{
  TargetPolicyNode() {} // NOLINT(modernize-use-equals-default)

  int NumExpectedParameters() const override { return 1; }

  std::string Evaluate(
    const std::vector<std::string>& parameters,
    cmGeneratorExpressionContext* context,
    const GeneratorExpressionContent* content,
    cmGeneratorExpressionDAGChecker* /*dagChecker*/) const override
  {
    if (!context->HeadTarget) {
      reportError(
        context, content->GetOriginalExpression(),
        "$<TARGET_POLICY:prop> may only be used with binary targets.  It "
        "may not be used with add_custom_command or add_custom_target.");
      return std::string();
    }

    context->HadContextSensitiveCondition = true;
    context->HadHeadSensitiveCondition = true;

    for (size_t i = 1; i < cm::size(targetPolicyWhitelist); ++i) {
      const char* policy = targetPolicyWhitelist[i];
      if (parameters.front() == policy) {
        cmLocalGenerator* lg = context->HeadTarget->GetLocalGenerator();
        switch (statusForTarget(context->HeadTarget, policy)) {
          case cmPolicies::WARN:
            lg->IssueMessage(
              MessageType::AUTHOR_WARNING,
              cmPolicies::GetPolicyWarning(policyForString(policy)));
            CM_FALLTHROUGH;
          case cmPolicies::REQUIRED_IF_USED:
          case cmPolicies::REQUIRED_ALWAYS:
          case cmPolicies::OLD:
            return "0";
          case cmPolicies::NEW:
            return "1";
        }
      }
    }
    reportError(
      context, content->GetOriginalExpression(),
      "$<TARGET_POLICY:prop> may only be used with a limited number of "
      "policies.  Currently it may be used with the following policies:\n"

#define STRINGIFY_HELPER(X) #X
#define STRINGIFY(X) STRINGIFY_HELPER(X)

#define TARGET_POLICY_LIST_ITEM(POLICY) " * " STRINGIFY(POLICY) "\n"

      CM_FOR_EACH_TARGET_POLICY(TARGET_POLICY_LIST_ITEM)

#undef TARGET_POLICY_LIST_ITEM
    );
    return std::string();
  }

} targetPolicyNode;

static const struct InstallPrefixNode : public cmGeneratorExpressionNode
{
  InstallPrefixNode() {} // NOLINT(modernize-use-equals-default)

  bool GeneratesContent() const override { return true; }
  int NumExpectedParameters() const override { return 0; }

  std::string Evaluate(
    const std::vector<std::string>& /*parameters*/,
    cmGeneratorExpressionContext* context,
    const GeneratorExpressionContent* content,
    cmGeneratorExpressionDAGChecker* /*dagChecker*/) const override
  {
    reportError(context, content->GetOriginalExpression(),
                "INSTALL_PREFIX is a marker for install(EXPORT) only.  It "
                "should never be evaluated.");
    return std::string();
  }

} installPrefixNode;

class ArtifactDirTag;
class ArtifactLinkerTag;
class ArtifactNameTag;
class ArtifactPathTag;
class ArtifactPdbTag;
class ArtifactSonameTag;
class ArtifactBundleDirTag;
class ArtifactBundleContentDirTag;

template <typename ArtifactT>
struct TargetFilesystemArtifactResultCreator
{
  static std::string Create(cmGeneratorTarget* target,
                            cmGeneratorExpressionContext* context,
                            const GeneratorExpressionContent* content);
};

template <>
struct TargetFilesystemArtifactResultCreator<ArtifactSonameTag>
{
  static std::string Create(cmGeneratorTarget* target,
                            cmGeneratorExpressionContext* context,
                            const GeneratorExpressionContent* content)
  {
    // The target soname file (.so.1).
    if (target->IsDLLPlatform()) {
      ::reportError(context, content->GetOriginalExpression(),
                    "TARGET_SONAME_FILE is not allowed "
                    "for DLL target platforms.");
      return std::string();
    }
    if (target->GetType() != cmStateEnums::SHARED_LIBRARY) {
      ::reportError(context, content->GetOriginalExpression(),
                    "TARGET_SONAME_FILE is allowed only for "
                    "SHARED libraries.");
      return std::string();
    }
    std::string result = target->GetDirectory(context->Config);
    result += "/";
    result += target->GetSOName(context->Config);
    return result;
  }
};

template <>
struct TargetFilesystemArtifactResultCreator<ArtifactPdbTag>
{
  static std::string Create(cmGeneratorTarget* target,
                            cmGeneratorExpressionContext* context,
                            const GeneratorExpressionContent* content)
  {
    if (target->IsImported()) {
      ::reportError(context, content->GetOriginalExpression(),
                    "TARGET_PDB_FILE not allowed for IMPORTED targets.");
      return std::string();
    }

    std::string language = target->GetLinkerLanguage(context->Config);

    std::string pdbSupportVar = "CMAKE_" + language + "_LINKER_SUPPORTS_PDB";

    if (!context->LG->GetMakefile()->IsOn(pdbSupportVar)) {
      ::reportError(context, content->GetOriginalExpression(),
                    "TARGET_PDB_FILE is not supported by the target linker.");
      return std::string();
    }

    cmStateEnums::TargetType targetType = target->GetType();

    if (targetType != cmStateEnums::SHARED_LIBRARY &&
        targetType != cmStateEnums::MODULE_LIBRARY &&
        targetType != cmStateEnums::EXECUTABLE) {
      ::reportError(context, content->GetOriginalExpression(),
                    "TARGET_PDB_FILE is allowed only for "
                    "targets with linker created artifacts.");
      return std::string();
    }

    std::string result = target->GetPDBDirectory(context->Config);
    result += "/";
    result += target->GetPDBName(context->Config);
    return result;
  }
};

template <>
struct TargetFilesystemArtifactResultCreator<ArtifactLinkerTag>
{
  static std::string Create(cmGeneratorTarget* target,
                            cmGeneratorExpressionContext* context,
                            const GeneratorExpressionContent* content)
  {
    // The file used to link to the target (.so, .lib, .a).
    if (!target->IsLinkable()) {
      ::reportError(context, content->GetOriginalExpression(),
                    "TARGET_LINKER_FILE is allowed only for libraries and "
                    "executables with ENABLE_EXPORTS.");
      return std::string();
    }
    cmStateEnums::ArtifactType artifact =
      target->HasImportLibrary(context->Config)
      ? cmStateEnums::ImportLibraryArtifact
      : cmStateEnums::RuntimeBinaryArtifact;
    return target->GetFullPath(context->Config, artifact);
  }
};

template <>
struct TargetFilesystemArtifactResultCreator<ArtifactBundleDirTag>
{
  static std::string Create(cmGeneratorTarget* target,
                            cmGeneratorExpressionContext* context,
                            const GeneratorExpressionContent* content)
  {
    if (target->IsImported()) {
      ::reportError(context, content->GetOriginalExpression(),
                    "TARGET_BUNDLE_DIR not allowed for IMPORTED targets.");
      return std::string();
    }
    if (!target->IsBundleOnApple()) {
      ::reportError(context, content->GetOriginalExpression(),
                    "TARGET_BUNDLE_DIR is allowed only for Bundle targets.");
      return std::string();
    }

    std::string outpath = target->GetDirectory(context->Config) + '/';
    return target->BuildBundleDirectory(outpath, context->Config,
                                        cmGeneratorTarget::BundleDirLevel);
  }
};

template <>
struct TargetFilesystemArtifactResultCreator<ArtifactBundleContentDirTag>
{
  static std::string Create(cmGeneratorTarget* target,
                            cmGeneratorExpressionContext* context,
                            const GeneratorExpressionContent* content)
  {
    if (target->IsImported()) {
      ::reportError(
        context, content->GetOriginalExpression(),
        "TARGET_BUNDLE_CONTENT_DIR not allowed for IMPORTED targets.");
      return std::string();
    }
    if (!target->IsBundleOnApple()) {
      ::reportError(
        context, content->GetOriginalExpression(),
        "TARGET_BUNDLE_CONTENT_DIR is allowed only for Bundle targets.");
      return std::string();
    }

    std::string outpath = target->GetDirectory(context->Config) + '/';
    return target->BuildBundleDirectory(outpath, context->Config,
                                        cmGeneratorTarget::ContentLevel);
  }
};

template <>
struct TargetFilesystemArtifactResultCreator<ArtifactNameTag>
{
  static std::string Create(cmGeneratorTarget* target,
                            cmGeneratorExpressionContext* context,
                            const GeneratorExpressionContent* /*unused*/)
  {
    return target->GetFullPath(context->Config,
                               cmStateEnums::RuntimeBinaryArtifact, true);
  }
};

template <typename ArtifactT>
struct TargetFilesystemArtifactResultGetter
{
  static std::string Get(const std::string& result);
};

template <>
struct TargetFilesystemArtifactResultGetter<ArtifactNameTag>
{
  static std::string Get(const std::string& result)
  {
    return cmSystemTools::GetFilenameName(result);
  }
};

template <>
struct TargetFilesystemArtifactResultGetter<ArtifactDirTag>
{
  static std::string Get(const std::string& result)
  {
    return cmSystemTools::GetFilenamePath(result);
  }
};

template <>
struct TargetFilesystemArtifactResultGetter<ArtifactPathTag>
{
  static std::string Get(const std::string& result) { return result; }
};

template <typename ArtifactT, typename ComponentT>
struct TargetFilesystemArtifact : public cmGeneratorExpressionNode
{
  TargetFilesystemArtifact() {} // NOLINT(modernize-use-equals-default)

  int NumExpectedParameters() const override { return 1; }

  std::string Evaluate(
    const std::vector<std::string>& parameters,
    cmGeneratorExpressionContext* context,
    const GeneratorExpressionContent* content,
    cmGeneratorExpressionDAGChecker* dagChecker) const override
  {
    // Lookup the referenced target.
    std::string name = parameters.front();

    if (!cmGeneratorExpression::IsValidTargetName(name)) {
      ::reportError(context, content->GetOriginalExpression(),
                    "Expression syntax not recognized.");
      return std::string();
    }
    cmGeneratorTarget* target = context->LG->FindGeneratorTargetToUse(name);
    if (!target) {
      ::reportError(context, content->GetOriginalExpression(),
                    "No target \"" + name + "\"");
      return std::string();
    }
    if (target->GetType() >= cmStateEnums::OBJECT_LIBRARY &&
        target->GetType() != cmStateEnums::UNKNOWN_LIBRARY) {
      ::reportError(context, content->GetOriginalExpression(),
                    "Target \"" + name +
                      "\" is not an executable or library.");
      return std::string();
    }
    if (dagChecker &&
        (dagChecker->EvaluatingLinkLibraries(target) ||
         (dagChecker->EvaluatingSources() &&
          target == dagChecker->TopTarget()))) {
      ::reportError(context, content->GetOriginalExpression(),
                    "Expressions which require the linker language may not "
                    "be used while evaluating link libraries");
      return std::string();
    }
    context->DependTargets.insert(target);
    context->AllTargets.insert(target);

    std::string result =
      TargetFilesystemArtifactResultCreator<ArtifactT>::Create(target, context,
                                                               content);
    if (context->HadError) {
      return std::string();
    }
    return TargetFilesystemArtifactResultGetter<ComponentT>::Get(result);
  }
};

template <typename ArtifactT>
struct TargetFilesystemArtifactNodeGroup
{
  TargetFilesystemArtifactNodeGroup() // NOLINT(modernize-use-equals-default)
  {
  }

  TargetFilesystemArtifact<ArtifactT, ArtifactPathTag> File;
  TargetFilesystemArtifact<ArtifactT, ArtifactNameTag> FileName;
  TargetFilesystemArtifact<ArtifactT, ArtifactDirTag> FileDir;
};

static const TargetFilesystemArtifactNodeGroup<ArtifactNameTag>
  targetNodeGroup;

static const TargetFilesystemArtifactNodeGroup<ArtifactLinkerTag>
  targetLinkerNodeGroup;

static const TargetFilesystemArtifactNodeGroup<ArtifactSonameTag>
  targetSoNameNodeGroup;

static const TargetFilesystemArtifactNodeGroup<ArtifactPdbTag>
  targetPdbNodeGroup;

static const TargetFilesystemArtifact<ArtifactBundleDirTag, ArtifactPathTag>
  targetBundleDirNode;

static const TargetFilesystemArtifact<ArtifactBundleContentDirTag,
                                      ArtifactPathTag>
  targetBundleContentDirNode;

static const struct ShellPathNode : public cmGeneratorExpressionNode
{
  ShellPathNode() {} // NOLINT(modernize-use-equals-default)

  std::string Evaluate(
    const std::vector<std::string>& parameters,
    cmGeneratorExpressionContext* context,
    const GeneratorExpressionContent* content,
    cmGeneratorExpressionDAGChecker* /*dagChecker*/) const override
  {
    std::vector<std::string> listIn;
    cmSystemTools::ExpandListArgument(parameters.front(), listIn);
    if (listIn.empty()) {
      reportError(context, content->GetOriginalExpression(),
                  "\"\" is not an absolute path.");
      return std::string();
    }
    cmStateSnapshot snapshot = context->LG->GetStateSnapshot();
    cmOutputConverter converter(snapshot);
    const char* separator = snapshot.GetState()->UseWindowsShell() ? ";" : ":";
    std::vector<std::string> listOut;
    listOut.reserve(listIn.size());
    for (auto const& in : listIn) {
      if (!cmSystemTools::FileIsFullPath(in)) {
        reportError(context, content->GetOriginalExpression(),
                    "\"" + in + "\" is not an absolute path.");
        return std::string();
      }
      listOut.emplace_back(converter.ConvertDirectorySeparatorsForShell(in));
    }
    return cmJoin(listOut, separator);
  }
} shellPathNode;

const cmGeneratorExpressionNode* cmGeneratorExpressionNode::GetNode(
  const std::string& identifier)
{
  static std::map<std::string, cmGeneratorExpressionNode const*> const nodeMap{
    { "0", &zeroNode },
    { "1", &oneNode },
    { "AND", &andNode },
    { "OR", &orNode },
    { "NOT", &notNode },
    { "C_COMPILER_ID", &cCompilerIdNode },
    { "CXX_COMPILER_ID", &cxxCompilerIdNode },
    { "CUDA_COMPILER_ID", &cudaCompilerIdNode },
    { "Fortran_COMPILER_ID", &fortranCompilerIdNode },
    { "VERSION_GREATER", &versionGreaterNode },
    { "VERSION_GREATER_EQUAL", &versionGreaterEqNode },
    { "VERSION_LESS", &versionLessNode },
    { "VERSION_LESS_EQUAL", &versionLessEqNode },
    { "VERSION_EQUAL", &versionEqualNode },
    { "C_COMPILER_VERSION", &cCompilerVersionNode },
    { "CXX_COMPILER_VERSION", &cxxCompilerVersionNode },
    { "CUDA_COMPILER_VERSION", &cudaCompilerVersionNode },
    { "Fortran_COMPILER_VERSION", &fortranCompilerVersionNode },
    { "PLATFORM_ID", &platformIdNode },
    { "COMPILE_FEATURES", &compileFeaturesNode },
    { "CONFIGURATION", &configurationNode },
    { "CONFIG", &configurationTestNode },
    { "TARGET_FILE", &targetNodeGroup.File },
    { "TARGET_LINKER_FILE", &targetLinkerNodeGroup.File },
    { "TARGET_SONAME_FILE", &targetSoNameNodeGroup.File },
    { "TARGET_PDB_FILE", &targetPdbNodeGroup.File },
    { "TARGET_FILE_NAME", &targetNodeGroup.FileName },
    { "TARGET_LINKER_FILE_NAME", &targetLinkerNodeGroup.FileName },
    { "TARGET_SONAME_FILE_NAME", &targetSoNameNodeGroup.FileName },
    { "TARGET_PDB_FILE_NAME", &targetPdbNodeGroup.FileName },
    { "TARGET_FILE_DIR", &targetNodeGroup.FileDir },
    { "TARGET_LINKER_FILE_DIR", &targetLinkerNodeGroup.FileDir },
    { "TARGET_SONAME_FILE_DIR", &targetSoNameNodeGroup.FileDir },
    { "TARGET_PDB_FILE_DIR", &targetPdbNodeGroup.FileDir },
    { "TARGET_BUNDLE_DIR", &targetBundleDirNode },
    { "TARGET_BUNDLE_CONTENT_DIR", &targetBundleContentDirNode },
    { "STREQUAL", &strEqualNode },
    { "EQUAL", &equalNode },
    { "IN_LIST", &inListNode },
    { "LOWER_CASE", &lowerCaseNode },
    { "UPPER_CASE", &upperCaseNode },
    { "MAKE_C_IDENTIFIER", &makeCIdentifierNode },
    { "BOOL", &boolNode },
    { "IF", &ifNode },
    { "ANGLE-R", &angle_rNode },
    { "COMMA", &commaNode },
    { "SEMICOLON", &semicolonNode },
    { "TARGET_PROPERTY", &targetPropertyNode },
    { "TARGET_NAME", &targetNameNode },
    { "TARGET_OBJECTS", &targetObjectsNode },
    { "TARGET_POLICY", &targetPolicyNode },
    { "TARGET_EXISTS", &targetExistsNode },
    { "TARGET_NAME_IF_EXISTS", &targetNameIfExistsNode },
    { "TARGET_GENEX_EVAL", &targetGenexEvalNode },
    { "GENEX_EVAL", &genexEvalNode },
    { "BUILD_INTERFACE", &buildInterfaceNode },
    { "INSTALL_INTERFACE", &installInterfaceNode },
    { "INSTALL_PREFIX", &installPrefixNode },
    { "JOIN", &joinNode },
    { "LINK_ONLY", &linkOnlyNode },
    { "COMPILE_LANGUAGE", &languageNode },
    { "SHELL_PATH", &shellPathNode }
  };

  {
    auto itr = nodeMap.find(identifier);
    if (itr != nodeMap.end()) {
      return itr->second;
    }
  }
  return nullptr;
}

void reportError(cmGeneratorExpressionContext* context,
                 const std::string& expr, const std::string& result)
{
  context->HadError = true;
  if (context->Quiet) {
    return;
  }

  std::ostringstream e;
  /* clang-format off */
  e << "Error evaluating generator expression:\n"
    << "  " << expr << "\n"
    << result;
  /* clang-format on */
  context->LG->GetCMakeInstance()->IssueMessage(MessageType::FATAL_ERROR,
                                                e.str(), context->Backtrace);
}
