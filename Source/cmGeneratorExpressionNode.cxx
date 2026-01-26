/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmGeneratorExpressionNode.h"

#include <algorithm>
#include <cassert>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <functional>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <stdexcept>
#include <unordered_map>
#include <utility>

#include <cm/iterator>
#include <cm/optional>
#include <cm/string_view>
#include <cmext/algorithm>
#include <cmext/string_view>

#include "cmsys/RegularExpression.hxx"
#include "cmsys/String.h"

#include "cmCMakePath.h"
#include "cmCMakeString.hxx"
#include "cmComputeLinkInformation.h"
#include "cmFileSet.h"
#include "cmGenExContext.h"
#include "cmGenExEvaluation.h"
#include "cmGeneratorExpression.h"
#include "cmGeneratorExpressionDAGChecker.h"
#include "cmGeneratorExpressionEvaluator.h"
#include "cmGeneratorTarget.h"
#include "cmGlobalGenerator.h"
#include "cmLinkItem.h"
#include "cmList.h"
#include "cmListFileCache.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmOutputConverter.h"
#include "cmPolicies.h"
#include "cmRange.h"
#include "cmSourceFile.h"
#include "cmStandardLevelResolver.h"
#include "cmState.h"
#include "cmStateSnapshot.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmTarget.h"
#include "cmValue.h"
#include "cmake.h"

namespace {

bool HasKnownObjectFileLocation(cm::GenEx::Evaluation* eval,
                                GeneratorExpressionContent const* content,
                                std::string const& genex,
                                cmGeneratorTarget const* target)
{
  std::string reason;
  if (!eval->EvaluateForBuildsystem &&
      !target->Target->HasKnownObjectFileLocation(&reason)) {
    std::ostringstream e;
    e << "The evaluation of the " << genex
      << " generator expression "
         "is only suitable for consumption by CMake (limited"
      << reason
      << ").  "
         "It is not suitable for writing out elsewhere.";
    reportError(eval, content->GetOriginalExpression(), e.str());
    return false;
  }
  return true;
}

} // namespace

std::string cmGeneratorExpressionNode::EvaluateDependentExpression(
  std::string const& prop, cm::GenEx::Evaluation* eval,
  cmGeneratorTarget const* headTarget,
  cmGeneratorExpressionDAGChecker* dagChecker,
  cmGeneratorTarget const* currentTarget)
{
  cmGeneratorExpression ge(*eval->Context.LG->GetCMakeInstance(),
                           eval->Backtrace);
  std::unique_ptr<cmCompiledGeneratorExpression> cge = ge.Parse(prop);
  cge->SetEvaluateForBuildsystem(eval->EvaluateForBuildsystem);
  cge->SetQuiet(eval->Quiet);
  std::string result =
    cge->Evaluate(eval->Context, dagChecker, headTarget, currentTarget);
  if (cge->GetHadContextSensitiveCondition()) {
    eval->HadContextSensitiveCondition = true;
  }
  if (cge->GetHadHeadSensitiveCondition()) {
    eval->HadHeadSensitiveCondition = true;
  }
  if (cge->GetHadLinkLanguageSensitiveCondition()) {
    eval->HadLinkLanguageSensitiveCondition = true;
  }
  return result;
}

static const struct ZeroNode : public cmGeneratorExpressionNode
{
  ZeroNode() {} // NOLINT(modernize-use-equals-default)

  bool GeneratesContent() const override { return false; }

  bool AcceptsArbitraryContentParameter() const override { return true; }

  std::string Evaluate(
    std::vector<std::string> const& /*parameters*/,
    cm::GenEx::Evaluation* /*eval*/,
    GeneratorExpressionContent const* /*content*/,
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
    std::vector<std::string> const& parameters,
    cm::GenEx::Evaluation* /*eval*/,
    GeneratorExpressionContent const* /*content*/,
    cmGeneratorExpressionDAGChecker* /*dagChecker*/) const override
  {
    return parameters.front();
  }
} oneNode;

static const struct OneNode buildInterfaceNode;

static const struct ZeroNode installInterfaceNode;

static const struct OneNode buildLocalInterfaceNode;

struct BooleanOpNode : public cmGeneratorExpressionNode
{
  BooleanOpNode(char const* op_, char const* successVal_,
                char const* failureVal_)
    : op(op_)
    , successVal(successVal_)
    , failureVal(failureVal_)
  {
  }

  int NumExpectedParameters() const override { return OneOrMoreParameters; }

  bool ShouldEvaluateNextParameter(std::vector<std::string> const& parameters,
                                   std::string& def_value) const override
  {
    if (!parameters.empty() && parameters.back() == failureVal) {
      def_value = failureVal;
      return false;
    }
    return true;
  }

  std::string Evaluate(std::vector<std::string> const& parameters,
                       cm::GenEx::Evaluation* eval,
                       GeneratorExpressionContent const* content,
                       cmGeneratorExpressionDAGChecker*) const override
  {
    for (std::string const& param : parameters) {
      if (param == this->failureVal) {
        return this->failureVal;
      }
      if (param != this->successVal) {
        std::ostringstream e;
        e << "Parameters to $<" << this->op;
        e << "> must resolve to either '0' or '1'.";
        reportError(eval, content->GetOriginalExpression(), e.str());
        return std::string();
      }
    }
    return this->successVal;
  }

  char const *const op, *const successVal, *const failureVal;
};

static BooleanOpNode const andNode("AND", "1", "0"), orNode("OR", "0", "1");

static const struct NotNode : public cmGeneratorExpressionNode
{
  NotNode() {} // NOLINT(modernize-use-equals-default)

  std::string Evaluate(
    std::vector<std::string> const& parameters, cm::GenEx::Evaluation* eval,
    GeneratorExpressionContent const* content,
    cmGeneratorExpressionDAGChecker* /*dagChecker*/) const override
  {
    if (parameters.front() != "0" && parameters.front() != "1") {
      reportError(
        eval, content->GetOriginalExpression(),
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
    std::vector<std::string> const& parameters,
    cm::GenEx::Evaluation* /*eval*/,
    GeneratorExpressionContent const* /*content*/,
    cmGeneratorExpressionDAGChecker* /*dagChecker*/) const override
  {
    return !cmIsOff(parameters.front()) ? "1" : "0";
  }
} boolNode;

static const struct IfNode : public cmGeneratorExpressionNode
{
  IfNode() {} // NOLINT(modernize-use-equals-default)

  int NumExpectedParameters() const override { return 3; }

  bool ShouldEvaluateNextParameter(std::vector<std::string> const& parameters,
                                   std::string&) const override
  {
    return (parameters.empty() ||
            parameters[0] != cmStrCat(parameters.size() - 1, ""));
  }

  std::string Evaluate(std::vector<std::string> const& parameters,
                       cm::GenEx::Evaluation* eval,
                       GeneratorExpressionContent const* content,
                       cmGeneratorExpressionDAGChecker*) const override
  {
    if (parameters[0] != "1" && parameters[0] != "0") {
      reportError(eval, content->GetOriginalExpression(),
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
    std::vector<std::string> const& parameters,
    cm::GenEx::Evaluation* /*eval*/,
    GeneratorExpressionContent const* /*content*/,
    cmGeneratorExpressionDAGChecker* /*dagChecker*/) const override
  {
    return cm::CMakeString{ parameters.front() }.Compare(
             cm::CMakeString::CompOperator::EQUAL, parameters[1])
      ? "1"
      : "0";
  }
} strEqualNode;
static const struct StrLessNode : public cmGeneratorExpressionNode
{
  StrLessNode() {} // NOLINT(modernize-use-equals-default)

  int NumExpectedParameters() const override { return 2; }

  std::string Evaluate(
    std::vector<std::string> const& parameters,
    cm::GenEx::Evaluation* /*eval*/,
    GeneratorExpressionContent const* /*content*/,
    cmGeneratorExpressionDAGChecker* /*dagChecker*/) const override
  {
    return cm::CMakeString{ parameters.front() }.Compare(
             cm::CMakeString::CompOperator::LESS, parameters[1])
      ? "1"
      : "0";
  }
} strLessNode;
static const struct StrLessEqualNode : public cmGeneratorExpressionNode
{
  StrLessEqualNode() {} // NOLINT(modernize-use-equals-default)

  int NumExpectedParameters() const override { return 2; }

  std::string Evaluate(
    std::vector<std::string> const& parameters,
    cm::GenEx::Evaluation* /*eval*/,
    GeneratorExpressionContent const* /*content*/,
    cmGeneratorExpressionDAGChecker* /*dagChecker*/) const override
  {
    return cm::CMakeString{ parameters.front() }.Compare(
             cm::CMakeString::CompOperator::LESS_EQUAL, parameters[1])
      ? "1"
      : "0";
  }
} strLessEqualNode;
static const struct StrGreaterNode : public cmGeneratorExpressionNode
{
  StrGreaterNode() {} // NOLINT(modernize-use-equals-default)

  int NumExpectedParameters() const override { return 2; }

  std::string Evaluate(
    std::vector<std::string> const& parameters,
    cm::GenEx::Evaluation* /*eval*/,
    GeneratorExpressionContent const* /*content*/,
    cmGeneratorExpressionDAGChecker* /*dagChecker*/) const override
  {
    return cm::CMakeString{ parameters.front() }.Compare(
             cm::CMakeString::CompOperator::GREATER, parameters[1])
      ? "1"
      : "0";
  }
} strGreaterNode;
static const struct StrGreaterEqualNode : public cmGeneratorExpressionNode
{
  StrGreaterEqualNode() {} // NOLINT(modernize-use-equals-default)

  int NumExpectedParameters() const override { return 2; }

  std::string Evaluate(
    std::vector<std::string> const& parameters,
    cm::GenEx::Evaluation* /*eval*/,
    GeneratorExpressionContent const* /*content*/,
    cmGeneratorExpressionDAGChecker* /*dagChecker*/) const override
  {
    return cm::CMakeString{ parameters.front() }.Compare(
             cm::CMakeString::CompOperator::GREATER_EQUAL, parameters[1])
      ? "1"
      : "0";
  }
} strGreaterEqualNode;

static const struct EqualNode : public cmGeneratorExpressionNode
{
  EqualNode() {} // NOLINT(modernize-use-equals-default)

  int NumExpectedParameters() const override { return 2; }

  std::string Evaluate(
    std::vector<std::string> const& parameters, cm::GenEx::Evaluation* eval,
    GeneratorExpressionContent const* content,
    cmGeneratorExpressionDAGChecker* /*dagChecker*/) const override
  {
    long numbers[2];
    for (int i = 0; i < 2; ++i) {
      if (!ParameterToLong(parameters[i].c_str(), &numbers[i])) {
        reportError(eval, content->GetOriginalExpression(),
                    "$<EQUAL> parameter " + parameters[i] +
                      " is not a valid integer.");
        return {};
      }
    }
    return numbers[0] == numbers[1] ? "1" : "0";
  }

  static bool ParameterToLong(char const* param, long* outResult)
  {
    char const isNegative = param[0] == '-';

    int base = 0;
    if (cmHasLiteralPrefix(param, "0b") || cmHasLiteralPrefix(param, "0B")) {
      base = 2;
      param += 2;
    } else if (cmHasLiteralPrefix(param, "-0b") ||
               cmHasLiteralPrefix(param, "-0B") ||
               cmHasLiteralPrefix(param, "+0b") ||
               cmHasLiteralPrefix(param, "+0B")) {
      base = 2;
      param += 3;
    }

    char* pEnd;
    long result = strtol(param, &pEnd, base);
    if (pEnd == param || *pEnd != '\0' || errno == ERANGE) {
      return false;
    }
    if (isNegative && result > 0) {
      result *= -1;
    }
    *outResult = result;
    return true;
  }
} equalNode;

static const struct InListNode : public cmGeneratorExpressionNode
{
  InListNode() {} // NOLINT(modernize-use-equals-default)

  int NumExpectedParameters() const override { return 2; }

  std::string Evaluate(
    std::vector<std::string> const& parameters, cm::GenEx::Evaluation* eval,
    GeneratorExpressionContent const* /*content*/,
    cmGeneratorExpressionDAGChecker* /*dagChecker*/) const override
  {
    cmList values;
    cmList checkValues;
    bool check = false;
    switch (eval->Context.LG->GetPolicyStatus(cmPolicies::CMP0085)) {
      case cmPolicies::WARN:
        if (parameters.front().empty()) {
          check = true;
          checkValues.assign(parameters[1], cmList::EmptyElements::Yes);
        }
        CM_FALLTHROUGH;
      case cmPolicies::OLD:
        values.assign(parameters[1]);
        if (check && values != checkValues) {
          std::ostringstream e;
          e << cmPolicies::GetPolicyWarning(cmPolicies::CMP0085)
            << "\nSearch Item:\n  \"" << parameters.front()
            << "\"\nList:\n  \"" << parameters[1] << "\"\n";
          eval->Context.LG->GetCMakeInstance()->IssueMessage(
            MessageType ::AUTHOR_WARNING, e.str(), eval->Backtrace);
          return "0";
        }
        if (values.empty()) {
          return "0";
        }
        break;
      case cmPolicies::NEW:
        values.assign(parameters[1], cmList::EmptyElements::Yes);
        break;
    }

    return values.find(parameters.front()) != cmList::npos ? "1" : "0";
  }
} inListNode;

static const struct FilterNode : public cmGeneratorExpressionNode
{
  FilterNode() {} // NOLINT(modernize-use-equals-default)

  int NumExpectedParameters() const override { return 3; }

  std::string Evaluate(
    std::vector<std::string> const& parameters, cm::GenEx::Evaluation* eval,
    GeneratorExpressionContent const* content,
    cmGeneratorExpressionDAGChecker* /*dagChecker*/) const override
  {
    if (parameters.size() != 3) {
      reportError(eval, content->GetOriginalExpression(),
                  "$<FILTER:...> expression requires three parameters");
      return {};
    }

    if (parameters[1] != "INCLUDE" && parameters[1] != "EXCLUDE") {
      reportError(
        eval, content->GetOriginalExpression(),
        "$<FILTER:...> second parameter must be either INCLUDE or EXCLUDE");
      return {};
    }

    try {
      return cmList{ parameters.front(), cmList::EmptyElements::Yes }
        .filter(parameters[2],
                parameters[1] == "EXCLUDE" ? cmList::FilterMode::EXCLUDE
                                           : cmList::FilterMode::INCLUDE)
        .to_string();
    } catch (std::invalid_argument&) {
      reportError(eval, content->GetOriginalExpression(),
                  "$<FILTER:...> failed to compile regex");
      return {};
    }
  }
} filterNode;

static const struct RemoveDuplicatesNode : public cmGeneratorExpressionNode
{
  RemoveDuplicatesNode() {} // NOLINT(modernize-use-equals-default)

  int NumExpectedParameters() const override { return 1; }

  std::string Evaluate(
    std::vector<std::string> const& parameters, cm::GenEx::Evaluation* eval,
    GeneratorExpressionContent const* content,
    cmGeneratorExpressionDAGChecker* /*dagChecker*/) const override
  {
    if (parameters.size() != 1) {
      reportError(
        eval, content->GetOriginalExpression(),
        "$<REMOVE_DUPLICATES:...> expression requires one parameter");
    }

    return cmList{ parameters.front(), cmList::EmptyElements::Yes }
      .remove_duplicates()
      .to_string();
  }

} removeDuplicatesNode;

static const struct TargetExistsNode : public cmGeneratorExpressionNode
{
  TargetExistsNode() {} // NOLINT(modernize-use-equals-default)

  int NumExpectedParameters() const override { return 1; }

  std::string Evaluate(
    std::vector<std::string> const& parameters, cm::GenEx::Evaluation* eval,
    GeneratorExpressionContent const* content,
    cmGeneratorExpressionDAGChecker* /*dagChecker*/) const override
  {
    if (parameters.size() != 1) {
      reportError(eval, content->GetOriginalExpression(),
                  "$<TARGET_EXISTS:...> expression requires one parameter");
      return std::string();
    }

    std::string const& targetName = parameters.front();
    if (targetName.empty() ||
        !cmGeneratorExpression::IsValidTargetName(targetName)) {
      reportError(eval, content->GetOriginalExpression(),
                  "$<TARGET_EXISTS:tgt> expression requires a non-empty "
                  "valid target name.");
      return std::string();
    }

    return eval->Context.LG->GetMakefile()->FindTargetToUse(targetName) ? "1"
                                                                        : "0";
  }
} targetExistsNode;

static const struct TargetNameIfExistsNode : public cmGeneratorExpressionNode
{
  TargetNameIfExistsNode() {} // NOLINT(modernize-use-equals-default)

  int NumExpectedParameters() const override { return 1; }

  std::string Evaluate(
    std::vector<std::string> const& parameters, cm::GenEx::Evaluation* eval,
    GeneratorExpressionContent const* content,
    cmGeneratorExpressionDAGChecker* /*dagChecker*/) const override
  {
    if (parameters.size() != 1) {
      reportError(eval, content->GetOriginalExpression(),
                  "$<TARGET_NAME_IF_EXISTS:...> expression requires one "
                  "parameter");
      return std::string();
    }

    std::string const& targetName = parameters.front();
    if (targetName.empty() ||
        !cmGeneratorExpression::IsValidTargetName(targetName)) {
      reportError(eval, content->GetOriginalExpression(),
                  "$<TARGET_NAME_IF_EXISTS:tgt> expression requires a "
                  "non-empty valid target name.");
      return std::string();
    }

    return eval->Context.LG->GetMakefile()->FindTargetToUse(targetName)
      ? targetName
      : std::string();
  }
} targetNameIfExistsNode;

struct GenexEvaluator : public cmGeneratorExpressionNode
{
  GenexEvaluator() {} // NOLINT(modernize-use-equals-default)

protected:
  std::string EvaluateExpression(
    std::string const& genexOperator, std::string const& expression,
    cm::GenEx::Evaluation* eval, GeneratorExpressionContent const* content,
    cmGeneratorExpressionDAGChecker* dagCheckerParent) const
  {
    if (eval->HeadTarget) {
      cmGeneratorExpressionDAGChecker dagChecker{
        eval->HeadTarget, cmStrCat(genexOperator, ':', expression),
        content,          dagCheckerParent,
        eval->Context,    eval->Backtrace,
      };
      switch (dagChecker.Check()) {
        case cmGeneratorExpressionDAGChecker::SELF_REFERENCE:
        case cmGeneratorExpressionDAGChecker::CYCLIC_REFERENCE: {
          dagChecker.ReportError(eval, content->GetOriginalExpression());
          return std::string();
        }
        case cmGeneratorExpressionDAGChecker::ALREADY_SEEN:
        case cmGeneratorExpressionDAGChecker::DAG:
          break;
      }

      return this->EvaluateDependentExpression(
        expression, eval, eval->HeadTarget, &dagChecker, eval->CurrentTarget);
    }

    return this->EvaluateDependentExpression(
      expression, eval, eval->HeadTarget, dagCheckerParent,
      eval->CurrentTarget);
  }
};

static const struct TargetGenexEvalNode : public GenexEvaluator
{
  TargetGenexEvalNode() {} // NOLINT(modernize-use-equals-default)

  int NumExpectedParameters() const override { return 2; }

  bool AcceptsArbitraryContentParameter() const override { return true; }

  std::string Evaluate(
    std::vector<std::string> const& parameters, cm::GenEx::Evaluation* eval,
    GeneratorExpressionContent const* content,
    cmGeneratorExpressionDAGChecker* dagCheckerParent) const override
  {
    std::string const& targetName = parameters.front();
    if (targetName.empty() ||
        !cmGeneratorExpression::IsValidTargetName(targetName)) {
      reportError(eval, content->GetOriginalExpression(),
                  "$<TARGET_GENEX_EVAL:tgt, ...> expression requires a "
                  "non-empty valid target name.");
      return std::string();
    }

    auto const* target =
      eval->Context.LG->FindGeneratorTargetToUse(targetName);
    if (!target) {
      std::ostringstream e;
      e << "$<TARGET_GENEX_EVAL:tgt, ...> target \"" << targetName
        << "\" not found.";
      reportError(eval, content->GetOriginalExpression(), e.str());
      return std::string();
    }

    std::string const& expression = parameters[1];
    if (expression.empty()) {
      return expression;
    }

    // Replace the surrounding context with the named target.
    cm::GenEx::Evaluation targetEval(eval->Context, eval->Quiet, target,
                                     target, eval->EvaluateForBuildsystem,
                                     eval->Backtrace);

    return this->EvaluateExpression("TARGET_GENEX_EVAL", expression,
                                    &targetEval, content, dagCheckerParent);
  }
} targetGenexEvalNode;

static const struct GenexEvalNode : public GenexEvaluator
{
  GenexEvalNode() {} // NOLINT(modernize-use-equals-default)

  int NumExpectedParameters() const override { return 1; }

  bool AcceptsArbitraryContentParameter() const override { return true; }

  std::string Evaluate(
    std::vector<std::string> const& parameters, cm::GenEx::Evaluation* eval,
    GeneratorExpressionContent const* content,
    cmGeneratorExpressionDAGChecker* dagCheckerParent) const override
  {
    std::string const& expression = parameters[0];
    if (expression.empty()) {
      return expression;
    }

    return this->EvaluateExpression("GENEX_EVAL", expression, eval, content,
                                    dagCheckerParent);
  }
} genexEvalNode;

static const struct LowerCaseNode : public cmGeneratorExpressionNode
{
  LowerCaseNode() {} // NOLINT(modernize-use-equals-default)

  bool AcceptsArbitraryContentParameter() const override { return true; }

  std::string Evaluate(
    std::vector<std::string> const& parameters,
    cm::GenEx::Evaluation* /*eval*/,
    GeneratorExpressionContent const* /*content*/,
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
    std::vector<std::string> const& parameters,
    cm::GenEx::Evaluation* /*eval*/,
    GeneratorExpressionContent const* /*content*/,
    cmGeneratorExpressionDAGChecker* /*dagChecker*/) const override
  {
    return cmSystemTools::UpperCase(parameters.front());
  }
} upperCaseNode;

namespace {
template <typename Container>
class Range : public cmRange<typename Container::const_iterator>
{
private:
  using Base = cmRange<typename Container::const_iterator>;

public:
  using const_iterator = typename Container::const_iterator;
  using value_type = typename Container::value_type;
  using size_type = typename Container::size_type;
  using difference_type = typename Container::difference_type;
  using const_reference = typename Container::const_reference;

  Range(Container const& container)
    : Base(container.begin(), container.end())
  {
  }

  const_reference operator[](size_type pos) const
  {
    return *(this->begin() + pos);
  }

  const_reference front() const { return *this->begin(); }
  const_reference back() const { return *std::prev(this->end()); }

  Range& advance(difference_type amount) &
  {
    Base::advance(amount);
    return *this;
  }
  Range advance(difference_type amount) &&
  {
    Base::advance(amount);
    return std::move(*this);
  }
};

using Arguments = Range<std::vector<std::string>>;

bool CheckGenExParameters(cm::GenEx::Evaluation* eval,
                          GeneratorExpressionContent const* cnt,
                          cm::string_view genex, cm::string_view option,
                          std::size_t count, int required = 1,
                          bool exactly = true)
{
  if (static_cast<int>(count) < required ||
      (exactly && static_cast<int>(count) > required)) {
    std::string nbParameters;
    switch (required) {
      case 1:
        nbParameters = "one parameter";
        break;
      case 2:
        nbParameters = "two parameters";
        break;
      case 3:
        nbParameters = "three parameters";
        break;
      case 4:
        nbParameters = "four parameters";
        break;
      default:
        nbParameters = cmStrCat(required, " parameters");
    }
    reportError(eval, cnt->GetOriginalExpression(),
                cmStrCat("$<", genex, ':', option, "> expression requires ",
                         (exactly ? "exactly" : "at least"), ' ', nbParameters,
                         '.'));
    return false;
  }
  return true;
};

template <typename IndexType>
bool GetNumericArgument(std::string const& arg, IndexType& value)
{
  try {
    std::size_t pos;

    if (sizeof(IndexType) == sizeof(long)) {
      value = std::stol(arg, &pos);
    } else {
      value = std::stoll(arg, &pos);
    }

    if (pos != arg.length()) {
      // this is not a number
      return false;
    }
  } catch (std::invalid_argument const&) {
    return false;
  }

  return true;
}

template <typename IndexType>
bool GetNumericArguments(
  cm::GenEx::Evaluation* eval, GeneratorExpressionContent const* cnt,
  Arguments args, std::vector<IndexType>& indexes,
  cmList::ExpandElements expandElements = cmList::ExpandElements::No)
{
  using IndexRange = cmRange<Arguments::const_iterator>;
  IndexRange arguments(args.begin(), args.end());
  cmList list;
  if (expandElements == cmList::ExpandElements::Yes) {
    list = cmList{ args.begin(), args.end(), expandElements };
    arguments = IndexRange{ list.begin(), list.end() };
  }

  for (auto const& value : arguments) {
    IndexType index;
    if (!GetNumericArgument(value, index)) {
      reportError(eval, cnt->GetOriginalExpression(),
                  cmStrCat("index: \"", value, "\" is not a valid index"));
      return false;
    }
    indexes.push_back(index);
  }
  return true;
}

bool CheckPathParametersEx(cm::GenEx::Evaluation* eval,
                           GeneratorExpressionContent const* cnt,
                           cm::string_view option, std::size_t count,
                           int required = 1, bool exactly = true)
{
  return CheckGenExParameters(eval, cnt, "PATH"_s, option, count, required,
                              exactly);
}
bool CheckPathParameters(cm::GenEx::Evaluation* eval,
                         GeneratorExpressionContent const* cnt,
                         cm::string_view option, Arguments args,
                         int required = 1)
{
  return CheckPathParametersEx(eval, cnt, option, args.size(), required);
};

std::string ToString(bool isTrue)
{
  return isTrue ? "1" : "0";
};
}

static const struct PathNode : public cmGeneratorExpressionNode
{
  PathNode() {} // NOLINT(modernize-use-equals-default)

  int NumExpectedParameters() const override { return TwoOrMoreParameters; }

  bool AcceptsArbitraryContentParameter() const override { return true; }

  std::string Evaluate(
    std::vector<std::string> const& parameters, cm::GenEx::Evaluation* eval,
    GeneratorExpressionContent const* content,
    cmGeneratorExpressionDAGChecker* /*dagChecker*/) const override
  {
    static auto processList =
      [](std::string const& arg,
         std::function<void(std::string&)> transform) -> std::string {
      cmList list{ arg };
      std::for_each(list.begin(), list.end(), std::move(transform));
      return list.to_string();
    };

    static std::unordered_map<
      cm::string_view,
      std::function<std::string(cm::GenEx::Evaluation*,
                                GeneratorExpressionContent const*,
                                Arguments&)>>
      pathCommands{
        { "GET_ROOT_NAME"_s,
          [](cm::GenEx::Evaluation* ev, GeneratorExpressionContent const* cnt,
             Arguments& args) -> std::string {
            if (CheckPathParameters(ev, cnt, "GET_ROOT_NAME"_s, args) &&
                !args.front().empty()) {
              return processList(args.front(), [](std::string& value) {
                value = cmCMakePath{ value }.GetRootName().String();
              });
            }
            return std::string{};
          } },
        { "GET_ROOT_DIRECTORY"_s,
          [](cm::GenEx::Evaluation* ev, GeneratorExpressionContent const* cnt,
             Arguments& args) -> std::string {
            if (CheckPathParameters(ev, cnt, "GET_ROOT_DIRECTORY"_s, args) &&
                !args.front().empty()) {
              return processList(args.front(), [](std::string& value) {
                value = cmCMakePath{ value }.GetRootDirectory().String();
              });
            }
            return std::string{};
          } },
        { "GET_ROOT_PATH"_s,
          [](cm::GenEx::Evaluation* ev, GeneratorExpressionContent const* cnt,
             Arguments& args) -> std::string {
            if (CheckPathParameters(ev, cnt, "GET_ROOT_PATH"_s, args) &&
                !args.front().empty()) {
              return processList(args.front(), [](std::string& value) {
                value = cmCMakePath{ value }.GetRootPath().String();
              });
            }
            return std::string{};
          } },
        { "GET_FILENAME"_s,
          [](cm::GenEx::Evaluation* ev, GeneratorExpressionContent const* cnt,
             Arguments& args) -> std::string {
            if (CheckPathParameters(ev, cnt, "GET_FILENAME"_s, args) &&
                !args.front().empty()) {
              return processList(args.front(), [](std::string& value) {
                value = cmCMakePath{ value }.GetFileName().String();
              });
            }
            return std::string{};
          } },
        { "GET_EXTENSION"_s,
          [](cm::GenEx::Evaluation* ev, GeneratorExpressionContent const* cnt,
             Arguments& args) -> std::string {
            bool lastOnly = args.front() == "LAST_ONLY"_s;
            if (lastOnly) {
              args.advance(1);
            }
            if (CheckPathParametersEx(ev, cnt,
                                      lastOnly ? "GET_EXTENSION,LAST_ONLY"_s
                                               : "GET_EXTENSION"_s,
                                      args.size())) {
              if (args.front().empty()) {
                return std::string{};
              }
              if (lastOnly) {
                return processList(args.front(), [](std::string& value) {
                  value = cmCMakePath{ value }.GetExtension().String();
                });
              }
              return processList(args.front(), [](std::string& value) {
                value = cmCMakePath{ value }.GetWideExtension().String();
              });
            }
            return std::string{};
          } },
        { "GET_STEM"_s,
          [](cm::GenEx::Evaluation* ev, GeneratorExpressionContent const* cnt,
             Arguments& args) -> std::string {
            bool lastOnly = args.front() == "LAST_ONLY"_s;
            if (lastOnly) {
              args.advance(1);
            }
            if (CheckPathParametersEx(
                  ev, cnt, lastOnly ? "GET_STEM,LAST_ONLY"_s : "GET_STEM"_s,
                  args.size())) {
              if (args.front().empty()) {
                return std::string{};
              }
              if (lastOnly) {
                return processList(args.front(), [](std::string& value) {
                  value = cmCMakePath{ value }.GetStem().String();
                });
              }
              return processList(args.front(), [](std::string& value) {
                value = cmCMakePath{ value }.GetNarrowStem().String();
              });
            }
            return std::string{};
          } },
        { "GET_RELATIVE_PART"_s,
          [](cm::GenEx::Evaluation* ev, GeneratorExpressionContent const* cnt,
             Arguments& args) -> std::string {
            if (CheckPathParameters(ev, cnt, "GET_RELATIVE_PART"_s, args) &&
                !args.front().empty()) {
              return processList(args.front(), [](std::string& value) {
                value = cmCMakePath{ value }.GetRelativePath().String();
              });
            }
            return std::string{};
          } },
        { "GET_PARENT_PATH"_s,
          [](cm::GenEx::Evaluation* ev, GeneratorExpressionContent const* cnt,
             Arguments& args) -> std::string {
            if (CheckPathParameters(ev, cnt, "GET_PARENT_PATH"_s, args)) {
              return processList(args.front(), [](std::string& value) {
                value = cmCMakePath{ value }.GetParentPath().String();
              });
            }
            return std::string{};
          } },
        { "HAS_ROOT_NAME"_s,
          [](cm::GenEx::Evaluation* ev, GeneratorExpressionContent const* cnt,
             Arguments& args) -> std::string {
            return CheckPathParameters(ev, cnt, "HAS_ROOT_NAME"_s, args)
              ? ToString(cmCMakePath{ args.front() }.HasRootName())
              : std::string{ "0" };
          } },
        { "HAS_ROOT_DIRECTORY"_s,
          [](cm::GenEx::Evaluation* ev, GeneratorExpressionContent const* cnt,
             Arguments& args) -> std::string {
            return CheckPathParameters(ev, cnt, "HAS_ROOT_DIRECTORY"_s, args)
              ? ToString(cmCMakePath{ args.front() }.HasRootDirectory())
              : std::string{ "0" };
          } },
        { "HAS_ROOT_PATH"_s,
          [](cm::GenEx::Evaluation* ev, GeneratorExpressionContent const* cnt,
             Arguments& args) -> std::string {
            return CheckPathParameters(ev, cnt, "HAS_ROOT_PATH"_s, args)
              ? ToString(cmCMakePath{ args.front() }.HasRootPath())
              : std::string{ "0" };
          } },
        { "HAS_FILENAME"_s,
          [](cm::GenEx::Evaluation* ev, GeneratorExpressionContent const* cnt,
             Arguments& args) -> std::string {
            return CheckPathParameters(ev, cnt, "HAS_FILENAME"_s, args)
              ? ToString(cmCMakePath{ args.front() }.HasFileName())
              : std::string{ "0" };
          } },
        { "HAS_EXTENSION"_s,
          [](cm::GenEx::Evaluation* ev, GeneratorExpressionContent const* cnt,
             Arguments& args) -> std::string {
            return CheckPathParameters(ev, cnt, "HAS_EXTENSION"_s, args) &&
                !args.front().empty()
              ? ToString(cmCMakePath{ args.front() }.HasExtension())
              : std::string{ "0" };
          } },
        { "HAS_STEM"_s,
          [](cm::GenEx::Evaluation* ev, GeneratorExpressionContent const* cnt,
             Arguments& args) -> std::string {
            return CheckPathParameters(ev, cnt, "HAS_STEM"_s, args)
              ? ToString(cmCMakePath{ args.front() }.HasStem())
              : std::string{ "0" };
          } },
        { "HAS_RELATIVE_PART"_s,
          [](cm::GenEx::Evaluation* ev, GeneratorExpressionContent const* cnt,
             Arguments& args) -> std::string {
            return CheckPathParameters(ev, cnt, "HAS_RELATIVE_PART"_s, args)
              ? ToString(cmCMakePath{ args.front() }.HasRelativePath())
              : std::string{ "0" };
          } },
        { "HAS_PARENT_PATH"_s,
          [](cm::GenEx::Evaluation* ev, GeneratorExpressionContent const* cnt,
             Arguments& args) -> std::string {
            return CheckPathParameters(ev, cnt, "HAS_PARENT_PATH"_s, args)
              ? ToString(cmCMakePath{ args.front() }.HasParentPath())
              : std::string{ "0" };
          } },
        { "IS_ABSOLUTE"_s,
          [](cm::GenEx::Evaluation* ev, GeneratorExpressionContent const* cnt,
             Arguments& args) -> std::string {
            return CheckPathParameters(ev, cnt, "IS_ABSOLUTE"_s, args)
              ? ToString(cmCMakePath{ args.front() }.IsAbsolute())
              : std::string{ "0" };
          } },
        { "IS_RELATIVE"_s,
          [](cm::GenEx::Evaluation* ev, GeneratorExpressionContent const* cnt,
             Arguments& args) -> std::string {
            return CheckPathParameters(ev, cnt, "IS_RELATIVE"_s, args)
              ? ToString(cmCMakePath{ args.front() }.IsRelative())
              : std::string{ "0" };
          } },
        { "IS_PREFIX"_s,
          [](cm::GenEx::Evaluation* ev, GeneratorExpressionContent const* cnt,
             Arguments& args) -> std::string {
            bool normalize = args.front() == "NORMALIZE"_s;
            if (normalize) {
              args.advance(1);
            }
            if (CheckPathParametersEx(
                  ev, cnt, normalize ? "IS_PREFIX,NORMALIZE"_s : "IS_PREFIX"_s,
                  args.size(), 2)) {
              if (normalize) {
                return ToString(cmCMakePath{ args[0] }.Normal().IsPrefix(
                  cmCMakePath{ args[1] }.Normal()));
              }
              return ToString(
                cmCMakePath{ args[0] }.IsPrefix(cmCMakePath{ args[1] }));
            }
            return std::string{};
          } },
        { "CMAKE_PATH"_s,
          [](cm::GenEx::Evaluation* ev, GeneratorExpressionContent const* cnt,
             Arguments& args) -> std::string {
            bool normalize = args.front() == "NORMALIZE"_s;
            if (normalize) {
              args.advance(1);
            }
            if (CheckPathParametersEx(ev, cnt,
                                      normalize ? "CMAKE_PATH,NORMALIZE"_s
                                                : "CMAKE_PATH"_s,
                                      args.size(), 1)) {
              return processList(
                args.front(), [normalize](std::string& value) {
                  auto path = cmCMakePath{ value, cmCMakePath::auto_format };
                  value = normalize ? path.Normal().GenericString()
                                    : path.GenericString();
                });
            }
            return std::string{};
          } },
        { "NATIVE_PATH"_s,
          [](cm::GenEx::Evaluation* ev, GeneratorExpressionContent const* cnt,
             Arguments& args) -> std::string {
            bool normalize = args.front() == "NORMALIZE"_s;
            if (normalize) {
              args.advance(1);
            }
            if (CheckPathParametersEx(ev, cnt,
                                      normalize ? "NATIVE_PATH,NORMALIZE"_s
                                                : "NATIVE_PATH"_s,
                                      args.size(), 1)) {
              return processList(
                args.front(), [normalize](std::string& value) {
                  auto path = cmCMakePath{ value };
                  value = normalize ? path.Normal().NativeString()
                                    : path.NativeString();
                });
            }
            return std::string{};
          } },
        { "APPEND"_s,
          [](cm::GenEx::Evaluation* ev, GeneratorExpressionContent const* cnt,
             Arguments& args) -> std::string {
            if (CheckPathParametersEx(ev, cnt, "APPEND"_s, args.size(), 1,
                                      false)) {
              auto const& list = args.front();
              args.advance(1);

              return processList(list, [&args](std::string& value) {
                cmCMakePath path{ value };
                for (auto const& p : args) {
                  path /= p;
                }
                value = path.String();
              });
            }
            return std::string{};
          } },
        { "REMOVE_FILENAME"_s,
          [](cm::GenEx::Evaluation* ev, GeneratorExpressionContent const* cnt,
             Arguments& args) -> std::string {
            if (CheckPathParameters(ev, cnt, "REMOVE_FILENAME"_s, args) &&
                !args.front().empty()) {
              return processList(args.front(), [](std::string& value) {
                value = cmCMakePath{ value }.RemoveFileName().String();
              });
            }
            return std::string{};
          } },
        { "REPLACE_FILENAME"_s,
          [](cm::GenEx::Evaluation* ev, GeneratorExpressionContent const* cnt,
             Arguments& args) -> std::string {
            if (CheckPathParameters(ev, cnt, "REPLACE_FILENAME"_s, args, 2)) {
              return processList(args.front(), [&args](std::string& value) {
                value = cmCMakePath{ value }
                          .ReplaceFileName(cmCMakePath{ args[1] })
                          .String();
              });
            }
            return std::string{};
          } },
        { "REMOVE_EXTENSION"_s,
          [](cm::GenEx::Evaluation* ev, GeneratorExpressionContent const* cnt,
             Arguments& args) -> std::string {
            bool lastOnly = args.front() == "LAST_ONLY"_s;
            if (lastOnly) {
              args.advance(1);
            }
            if (CheckPathParametersEx(ev, cnt,
                                      lastOnly ? "REMOVE_EXTENSION,LAST_ONLY"_s
                                               : "REMOVE_EXTENSION"_s,
                                      args.size())) {
              if (args.front().empty()) {
                return std::string{};
              }
              if (lastOnly) {
                return processList(args.front(), [](std::string& value) {
                  value = cmCMakePath{ value }.RemoveExtension().String();
                });
              }
              return processList(args.front(), [](std::string& value) {
                value = cmCMakePath{ value }.RemoveWideExtension().String();
              });
            }
            return std::string{};
          } },
        { "REPLACE_EXTENSION"_s,
          [](cm::GenEx::Evaluation* ev, GeneratorExpressionContent const* cnt,
             Arguments& args) -> std::string {
            bool lastOnly = args.front() == "LAST_ONLY"_s;
            if (lastOnly) {
              args.advance(1);
            }
            if (CheckPathParametersEx(ev, cnt,
                                      lastOnly
                                        ? "REPLACE_EXTENSION,LAST_ONLY"_s
                                        : "REPLACE_EXTENSION"_s,
                                      args.size(), 2)) {
              if (lastOnly) {
                return processList(args.front(), [&args](std::string& value) {
                  value = cmCMakePath{ value }
                            .ReplaceExtension(cmCMakePath{ args[1] })
                            .String();
                });
              }
              return processList(args.front(), [&args](std::string& value) {
                value = cmCMakePath{ value }
                          .ReplaceWideExtension(cmCMakePath{ args[1] })
                          .String();
              });
            }
            return std::string{};
          } },
        { "NORMAL_PATH"_s,
          [](cm::GenEx::Evaluation* ev, GeneratorExpressionContent const* cnt,
             Arguments& args) -> std::string {
            if (CheckPathParameters(ev, cnt, "NORMAL_PATH"_s, args) &&
                !args.front().empty()) {
              return processList(args.front(), [](std::string& value) {
                value = cmCMakePath{ value }.Normal().String();
              });
            }
            return std::string{};
          } },
        { "RELATIVE_PATH"_s,
          [](cm::GenEx::Evaluation* ev, GeneratorExpressionContent const* cnt,
             Arguments& args) -> std::string {
            if (CheckPathParameters(ev, cnt, "RELATIVE_PATH"_s, args, 2)) {
              return processList(args.front(), [&args](std::string& value) {
                value = cmCMakePath{ value }.Relative(args[1]).String();
              });
            }
            return std::string{};
          } },
        { "ABSOLUTE_PATH"_s,
          [](cm::GenEx::Evaluation* ev, GeneratorExpressionContent const* cnt,
             Arguments& args) -> std::string {
            bool normalize = args.front() == "NORMALIZE"_s;
            if (normalize) {
              args.advance(1);
            }
            if (CheckPathParametersEx(ev, cnt,
                                      normalize ? "ABSOLUTE_PATH,NORMALIZE"_s
                                                : "ABSOLUTE_PATH"_s,
                                      args.size(), 2)) {
              return processList(
                args.front(), [&args, normalize](std::string& value) {
                  auto path = cmCMakePath{ value }.Absolute(args[1]);
                  value = normalize ? path.Normal().String() : path.String();
                });
            }
            return std::string{};
          } }
      };

    if (cm::contains(pathCommands, parameters.front())) {
      auto args = Arguments{ parameters }.advance(1);
      return pathCommands[parameters.front()](eval, content, args);
    }

    reportError(eval, content->GetOriginalExpression(),
                cmStrCat(parameters.front(), ": invalid option."));
    return std::string{};
  }
} pathNode;

static const struct PathEqualNode : public cmGeneratorExpressionNode
{
  PathEqualNode() {} // NOLINT(modernize-use-equals-default)

  int NumExpectedParameters() const override { return 2; }

  std::string Evaluate(
    std::vector<std::string> const& parameters,
    cm::GenEx::Evaluation* /*eval*/,
    GeneratorExpressionContent const* /*content*/,
    cmGeneratorExpressionDAGChecker* /*dagChecker*/) const override
  {
    return cmCMakePath{ parameters[0] } == cmCMakePath{ parameters[1] } ? "1"
                                                                        : "0";
  }
} pathEqualNode;

namespace {
inline bool CheckStringParametersEx(cm::GenEx::Evaluation* eval,
                                    GeneratorExpressionContent const* cnt,
                                    cm::string_view option, std::size_t count,
                                    int required = 1, bool exactly = true)
{
  return CheckGenExParameters(eval, cnt, "STRING"_s, option, count, required,
                              exactly);
}
inline bool CheckStringParameters(cm::GenEx::Evaluation* eval,
                                  GeneratorExpressionContent const* cnt,
                                  cm::string_view option, Arguments args,
                                  int required = 1)
{
  return CheckStringParametersEx(eval, cnt, option, args.size(), required);
};
}

static const struct StringNode : public cmGeneratorExpressionNode
{
  StringNode() {} // NOLINT(modernize-use-equals-default)

  int NumExpectedParameters() const override { return OneOrMoreParameters; }

  bool AcceptsArbitraryContentParameter() const override { return true; }

  std::string Evaluate(
    std::vector<std::string> const& parameters, cm::GenEx::Evaluation* eval,
    GeneratorExpressionContent const* content,
    cmGeneratorExpressionDAGChecker* /*dagChecker*/) const override
  {
    static std::unordered_map<
      cm::string_view,
      std::function<std::string(cm::GenEx::Evaluation*,
                                GeneratorExpressionContent const*,
                                Arguments&)>>
      stringCommands{
        { "LENGTH"_s,
          [](cm::GenEx::Evaluation* ev, GeneratorExpressionContent const* cnt,
             Arguments& args) -> std::string {
            if (CheckStringParameters(ev, cnt, "LENGTH"_s, args)) {
              return std::to_string(cm::CMakeString{ args.front() }.Length());
            }
            return std::string{};
          } },
        { "SUBSTRING"_s,
          [](cm::GenEx::Evaluation* ev, GeneratorExpressionContent const* cnt,
             Arguments& args) -> std::string {
            if (CheckStringParameters(ev, cnt, "SUBSTRING"_s, args, 3)) {
              cm::CMakeString str{ args.front() };
              std::vector<long> indexes;
              if (GetNumericArguments(ev, cnt, args.advance(1), indexes)) {
                try {
                  return str.Substring(indexes.front(), indexes.back());
                } catch (std::out_of_range const& e) {
                  reportError(ev, cnt->GetOriginalExpression(), e.what());
                  return std::string{};
                }
              }
            }
            return std::string{};
          } },
        { "FIND"_s,
          [](cm::GenEx::Evaluation* ev, GeneratorExpressionContent const* cnt,
             Arguments& args) -> std::string {
            if (CheckStringParametersEx(ev, cnt, "FIND"_s, args.size(), 2,
                                        false)) {
              if (args.size() > 3) {
                reportError(ev, cnt->GetOriginalExpression(),
                            "$<STRING:FIND> expression expects at "
                            "most three parameters.");
                return std::string{};
              }

              auto const FROM = "FROM:"_s;

              cm::CMakeString str{ args.front() };
              cm::CMakeString::FindFrom from =
                cm::CMakeString::FindFrom::Begin;
              cm::string_view substring;

              args.advance(1);
              if (args.size() == 2) {
                cm::CMakeString::FindFrom opt =
                  static_cast<cm::CMakeString::FindFrom>(-1);

                for (auto const& arg : args) {
                  if (cmHasPrefix(arg, FROM)) {
                    if (arg != "FROM:BEGIN"_s && arg != "FROM:END"_s) {
                      reportError(
                        ev, cnt->GetOriginalExpression(),
                        cmStrCat("Invalid value for '", FROM,
                                 "' option. 'BEGIN' or 'END' expected."));
                      return std::string{};
                    }
                    opt = arg == "FROM:BEGIN"_s
                      ? cm::CMakeString::FindFrom::Begin
                      : cm::CMakeString::FindFrom::End;
                  } else {
                    substring = arg;
                  }
                }
                if (opt == static_cast<cm::CMakeString::FindFrom>(-1)) {
                  reportError(
                    ev, cnt->GetOriginalExpression(),
                    cmStrCat("Expected option '", FROM, "' is missing."));
                  return std::string{};
                }
                from = opt;
              } else {
                substring = args.front();
              }
              auto pos = str.Find(substring, from);
              return pos == cm::CMakeString::npos ? "-1" : std::to_string(pos);
            }
            return std::string{};
          } },
        { "MATCH"_s,
          [](cm::GenEx::Evaluation* ev, GeneratorExpressionContent const* cnt,
             Arguments& args) -> std::string {
            if (CheckStringParametersEx(ev, cnt, "MATCH"_s, args.size(), 2,
                                        false)) {
              if (args.size() > 3) {
                reportError(ev, cnt->GetOriginalExpression(),
                            "$<STRING:MATCH> expression expects at "
                            "most three parameters.");
                return std::string{};
              }

              auto const SEEK = "SEEK:"_s;

              cm::CMakeString str{ args.front() };
              cm::CMakeString::MatchItems seek =
                cm::CMakeString::MatchItems::Once;
              auto const* regex = &args[1];

              args.advance(1);
              if (args.size() == 2) {
                cm::CMakeString::MatchItems opt =
                  static_cast<cm::CMakeString::MatchItems>(-1);

                for (auto const& arg : args) {
                  if (cmHasPrefix(arg, SEEK)) {
                    if (arg != "SEEK:ONCE"_s && arg != "SEEK:ALL"_s) {
                      reportError(
                        ev, cnt->GetOriginalExpression(),
                        cmStrCat("Invalid value for '", SEEK,
                                 "' option. 'ONCE' or 'ALL' expected."));
                      return std::string{};
                    }
                    opt = arg == "SEEK:ONCE"_s
                      ? cm::CMakeString::MatchItems::Once
                      : cm::CMakeString::MatchItems::All;
                  } else {
                    regex = &arg;
                  }
                }
                if (opt == static_cast<cm::CMakeString::MatchItems>(-1)) {
                  reportError(
                    ev, cnt->GetOriginalExpression(),
                    cmStrCat("Expected option '", SEEK, "' is missing."));
                  return std::string{};
                }
                seek = opt;
              }

              try {
                return str.Match(*regex, seek).to_string();
              } catch (std::invalid_argument const& e) {
                reportError(ev, cnt->GetOriginalExpression(), e.what());
                return std::string{};
              }
            }
            return std::string{};
          } },
        { "JOIN"_s,
          [](cm::GenEx::Evaluation* ev, GeneratorExpressionContent const* cnt,
             Arguments& args) -> std::string {
            if (CheckStringParametersEx(ev, cnt, "JOIN"_s, args.size(), 2,
                                        false)) {
              auto const& glue = args.front();
              return cm::CMakeString{ args.advance(1), glue };
            }
            return std::string{};
          } },
        { "ASCII"_s,
          [](cm::GenEx::Evaluation* ev, GeneratorExpressionContent const* cnt,
             Arguments& args) -> std::string {
            if (CheckStringParametersEx(ev, cnt, "ASCII"_s, args.size(), 1,
                                        false)) {
              try {
                return cm::CMakeString{}.FromASCII(args);
              } catch (std::invalid_argument const& e) {
                reportError(ev, cnt->GetOriginalExpression(), e.what());
                return std::string{};
              }
            }
            return std::string{};
          } },
        { "TIMESTAMP"_s,
          [](cm::GenEx::Evaluation* ev, GeneratorExpressionContent const* cnt,
             Arguments& args) -> std::string {
            cm::string_view format;
            cm::CMakeString::UTC utc = cm::CMakeString::UTC::No;

            if (args.size() == 2 && args.front() != "UTC"_s &&
                args.back() != "UTC"_s) {
              reportError(ev, cnt->GetOriginalExpression(),
                          "'UTC' option is expected.");
              return std::string{};
            }
            if (args.size() > 2) {
              reportError(ev, cnt->GetOriginalExpression(),
                          "$<STRING:TIMESTAMP> expression expects at most two "
                          "parameters.");
              return std::string{};
            }

            for (auto const& arg : args) {
              if (arg == "UTC"_s) {
                utc = cm::CMakeString::UTC::Yes;
              } else {
                format = arg;
              }
            }
            return cm::CMakeString{}.Timestamp(format, utc);
          } },
        { "RANDOM"_s,
          [](cm::GenEx::Evaluation* ev, GeneratorExpressionContent const* cnt,
             Arguments& args) -> std::string {
            auto const ALPHABET = "ALPHABET:"_s;
            auto const LENGTH = "LENGTH:"_s;
            auto const RANDOM_SEED = "RANDOM_SEED:"_s;

            if (args.size() > 3) {
              reportError(ev, cnt->GetOriginalExpression(),
                          "$<STRING:RANDOM> expression expects at most three "
                          "parameters.");
              return std::string{};
            }

            cm::string_view alphabet;
            std::size_t length = 5;
            bool seed_specified = false;
            unsigned int seed = 0;
            for (auto const& arg : args) {
              if (cmHasPrefix(arg, ALPHABET)) {
                alphabet = cm::string_view{ arg.c_str() + ALPHABET.length() };
                continue;
              }
              if (cmHasPrefix(arg, LENGTH)) {
                try {
                  length = std::stoul(arg.substr(LENGTH.size()));
                } catch (std::exception const&) {
                  reportError(ev, cnt->GetOriginalExpression(),
                              cmStrCat(arg, ": invalid numeric value for '",
                                       LENGTH, "' option."));
                  return std::string{};
                }
                continue;
              }
              if (cmHasPrefix(arg, RANDOM_SEED)) {
                try {
                  seed_specified = true;
                  seed = static_cast<unsigned int>(
                    std::stoul(arg.substr(RANDOM_SEED.size())));
                } catch (std::exception const&) {
                  reportError(ev, cnt->GetOriginalExpression(),
                              cmStrCat(arg, ": invalid numeric value for '",
                                       RANDOM_SEED, "' option."));
                  return std::string{};
                }
                continue;
              }
              reportError(ev, cnt->GetOriginalExpression(),
                          cmStrCat(arg, ": invalid parameter."));
              return std::string{};
            }

            try {
              if (seed_specified) {
                return cm::CMakeString{}.Random(seed, length, alphabet);
              }
              return cm::CMakeString{}.Random(length, alphabet);
            } catch (std::exception const& e) {
              reportError(ev, cnt->GetOriginalExpression(), e.what());
              return std::string{};
            }
          } },
        { "UUID"_s,
          [](cm::GenEx::Evaluation* ev, GeneratorExpressionContent const* cnt,
             Arguments& args) -> std::string {
            if (CheckStringParametersEx(ev, cnt, "UUID"_s, args.size(), 2,
                                        false)) {
              auto const NAMESPACE = "NAMESPACE:"_s;
              auto const NAME = "NAME:"_s;
              auto const TYPE = "TYPE:"_s;
              auto const CASE = "CASE:"_s;

              if (args.size() > 4) {
                reportError(ev, cnt->GetOriginalExpression(),
                            "$<STRING:UUID> expression expects at most four "
                            "parameters.");
                return std::string{};
              }

              cm::string_view nameSpace;
              cm::string_view name;
              cm::CMakeString::UUIDType type =
                static_cast<cm::CMakeString::UUIDType>(-1);
              cm::CMakeString::Case uuidCase = cm::CMakeString::Case::Lower;
              for (auto const& arg : args) {
                if (cmHasPrefix(arg, NAMESPACE)) {
                  nameSpace =
                    cm::string_view{ arg.c_str() + NAMESPACE.length() };
                  if (nameSpace.empty()) {
                    reportError(
                      ev, cnt->GetOriginalExpression(),
                      cmStrCat("Invalid value for '", NAMESPACE, "' option."));
                    return std::string{};
                  }
                  continue;
                }
                if (cmHasPrefix(arg, NAME)) {
                  name = cm::string_view{ arg.c_str() + NAME.length() };
                  continue;
                }
                if (cmHasPrefix(arg, TYPE)) {
                  auto value = cm::string_view{ arg.c_str() + TYPE.length() };
                  if (value != "MD5"_s && value != "SHA1"_s) {
                    reportError(
                      ev, cnt->GetOriginalExpression(),
                      cmStrCat("Invalid value for '", TYPE,
                               "' option. 'MD5' or 'SHA1' expected."));
                    return std::string{};
                  }
                  type = value == "MD5"_s ? cm::CMakeString::UUIDType::MD5
                                          : cm::CMakeString::UUIDType::SHA1;
                  continue;
                }
                if (cmHasPrefix(arg, CASE)) {
                  auto value = cm::string_view{ arg.c_str() + CASE.length() };
                  if (value != "UPPER"_s && value != "LOWER"_s) {
                    reportError(
                      ev, cnt->GetOriginalExpression(),
                      cmStrCat("Invalid value for '", CASE,
                               "' option. 'UPPER' or 'LOWER' expected."));
                    return std::string{};
                  }
                  uuidCase = value == "UPPER"_s ? cm::CMakeString::Case::Upper
                                                : cm::CMakeString::Case::Lower;
                  continue;
                }
                reportError(ev, cnt->GetOriginalExpression(),
                            cmStrCat(arg, ": invalid parameter."));
                return std::string{};
              }
              if (nameSpace.empty()) {
                reportError(
                  ev, cnt->GetOriginalExpression(),
                  cmStrCat("Required option '", NAMESPACE, "' is missing."));
                return std::string{};
              }
              if (type == static_cast<cm::CMakeString::UUIDType>(-1)) {
                reportError(
                  ev, cnt->GetOriginalExpression(),
                  cmStrCat("Required option '", TYPE, "' is missing."));
                return std::string{};
              }

              try {
                return cm::CMakeString{}.UUID(nameSpace, name, type, uuidCase);
              } catch (std::exception const& e) {
                reportError(ev, cnt->GetOriginalExpression(), e.what());
                return std::string{};
              }
            }
            return std::string{};
          } },
        { "REPLACE"_s,
          [](cm::GenEx::Evaluation* ev, GeneratorExpressionContent const* cnt,
             Arguments& args) -> std::string {
            if (CheckStringParametersEx(ev, cnt, "REPLACE"_s, args.size(), 3,
                                        false)) {
              if (args.size() > 4) {
                reportError(ev, cnt->GetOriginalExpression(),
                            "$<STRING:REPLACE> expression expects at "
                            "most four parameters.");
                return std::string{};
              }

              cm::CMakeString::Regex isRegex = cm::CMakeString::Regex::No;
              if (args.size() == 4) {
                cm::string_view type = args.front();
                if (type != "STRING"_s && type != "REGEX"_s) {
                  reportError(
                    ev, cnt->GetOriginalExpression(),
                    cmStrCat(
                      '\'', type,
                      "' is unexpected. 'STRING' or 'REGEX' expected."));
                  return std::string{};
                }
                isRegex = type == "STRING"_s ? cm::CMakeString::Regex::No
                                             : cm::CMakeString::Regex::Yes;
                args.advance(1);
              }

              try {
                return cm::CMakeString{ args.front() }.Replace(
                  args[1], args[2], isRegex);
              } catch (std::invalid_argument const& e) {
                reportError(ev, cnt->GetOriginalExpression(), e.what());
                return std::string{};
              }
            }
            return std::string{};
          } },
        { "APPEND"_s,
          [](cm::GenEx::Evaluation* ev, GeneratorExpressionContent const* cnt,
             Arguments& args) -> std::string {
            if (CheckStringParametersEx(ev, cnt, "APPEND"_s, args.size(), 2,
                                        false)) {
              cm::CMakeString data{ args.front() };
              return data.Append(args.advance(1));
            }
            return std::string{};
          } },
        { "PREPEND"_s,
          [](cm::GenEx::Evaluation* ev, GeneratorExpressionContent const* cnt,
             Arguments& args) -> std::string {
            if (CheckStringParametersEx(ev, cnt, "PREPEND "_s, args.size(), 2,
                                        false)) {
              cm::CMakeString data{ args.front() };
              return data.Prepend(args.advance(1));
            }
            return std::string{};
          } },
        { "TOLOWER"_s,
          [](cm::GenEx::Evaluation* ev, GeneratorExpressionContent const* cnt,
             Arguments& args) -> std::string {
            if (CheckStringParameters(ev, cnt, "TOLOWER"_s, args, 1)) {
              return cm::CMakeString{}.ToLower(args.front());
            }
            return std::string{};
          } },
        { "TOUPPER"_s,
          [](cm::GenEx::Evaluation* ev, GeneratorExpressionContent const* cnt,
             Arguments& args) -> std::string {
            if (CheckStringParameters(ev, cnt, "TOUPPER"_s, args, 1)) {
              return cm::CMakeString{}.ToUpper(args.front());
            }
            return std::string{};
          } },
        { "STRIP"_s,
          [](cm::GenEx::Evaluation* ev, GeneratorExpressionContent const* cnt,
             Arguments& args) -> std::string {
            if (CheckStringParameters(ev, cnt, "STRIP"_s, args, 2)) {
              if (args.front() != "SPACES"_s) {
                reportError(ev, cnt->GetOriginalExpression(),
                            cmStrCat('\'', args.front(),
                                     "' is unexpected. 'SPACES' expected."));
                return std::string{};
              }

              return cm::CMakeString{ args[1] }.Strip();
            }
            return std::string{};
          } },
        { "QUOTE"_s,
          [](cm::GenEx::Evaluation* ev, GeneratorExpressionContent const* cnt,
             Arguments& args) -> std::string {
            if (CheckStringParameters(ev, cnt, "QUOTE"_s, args, 2)) {
              if (args.front() != "REGEX"_s) {
                reportError(ev, cnt->GetOriginalExpression(),
                            cmStrCat('\'', args.front(),
                                     "' is unexpected. 'REGEX' expected."));
                return std::string{};
              }

              return cm::CMakeString{ args[1] }.Quote();
            }
            return std::string{};
          } },
        { "HEX"_s,
          [](cm::GenEx::Evaluation* ev, GeneratorExpressionContent const* cnt,
             Arguments& args) -> std::string {
            if (CheckStringParameters(ev, cnt, "HEX"_s, args, 1)) {
              return cm::CMakeString{ args.front() }.ToHexadecimal();
            }
            return std::string{};
          } },
        { "HASH"_s,
          [](cm::GenEx::Evaluation* ev, GeneratorExpressionContent const* cnt,
             Arguments& args) -> std::string {
            if (CheckStringParameters(ev, cnt, "HASH"_s, args, 2)) {
              auto const ALGORITHM = "ALGORITHM:"_s;

              if (cmHasPrefix(args[1], ALGORITHM)) {
                try {
                  auto const algo =
                    cm::string_view{ args[1].c_str() + ALGORITHM.length() };
                  if (algo.empty()) {
                    reportError(
                      ev, cnt->GetOriginalExpression(),
                      cmStrCat("Missing value for '", ALGORITHM, "' option."));
                    return std::string{};
                  }
                  return cm::CMakeString{ args.front() }.Hash(algo);
                } catch (std::exception const& e) {
                  reportError(ev, cnt->GetOriginalExpression(), e.what());
                  return std::string{};
                }
              }
              reportError(ev, cnt->GetOriginalExpression(),
                          cmStrCat(args[1], ": invalid parameter. Option '",
                                   ALGORITHM, "' expected."));
            }
            return std::string{};
          } },
        { "MAKE_C_IDENTIFIER"_s,
          [](cm::GenEx::Evaluation* ev, GeneratorExpressionContent const* cnt,
             Arguments& args) -> std::string {
            if (CheckStringParameters(ev, cnt, "MAKE_C_IDENTIFIER"_s, args,
                                      1)) {
              return cm::CMakeString{ args.front() }.MakeCIdentifier();
            }
            return std::string{};
          } }
      };

    if (parameters.front().empty()) {
      reportError(eval, content->GetOriginalExpression(),
                  "$<STRING> expression requires at least one parameter.");
      return std::string{};
    }

    if (cm::contains(stringCommands, parameters.front())) {
      auto args = Arguments{ parameters }.advance(1);
      return stringCommands[parameters.front()](eval, content, args);
    }

    reportError(eval, content->GetOriginalExpression(),
                cmStrCat(parameters.front(), ": invalid option."));
    return std::string{};
  }
} stringNode;

namespace {
inline bool CheckListParametersEx(cm::GenEx::Evaluation* eval,
                                  GeneratorExpressionContent const* cnt,
                                  cm::string_view option, std::size_t count,
                                  int required = 1, bool exactly = true)
{
  return CheckGenExParameters(eval, cnt, "LIST"_s, option, count, required,
                              exactly);
}
inline bool CheckListParameters(cm::GenEx::Evaluation* eval,
                                GeneratorExpressionContent const* cnt,
                                cm::string_view option, Arguments args,
                                int required = 1)
{
  return CheckListParametersEx(eval, cnt, option, args.size(), required);
};

inline cmList GetList(std::string const& list)
{
  return list.empty() ? cmList{} : cmList{ list, cmList::EmptyElements::Yes };
}
}

static const struct ListNode : public cmGeneratorExpressionNode
{
  ListNode() {} // NOLINT(modernize-use-equals-default)

  int NumExpectedParameters() const override { return TwoOrMoreParameters; }

  bool AcceptsArbitraryContentParameter() const override { return true; }

  std::string Evaluate(
    std::vector<std::string> const& parameters, cm::GenEx::Evaluation* eval,
    GeneratorExpressionContent const* content,
    cmGeneratorExpressionDAGChecker* /*dagChecker*/) const override
  {
    static std::unordered_map<
      cm::string_view,
      std::function<std::string(cm::GenEx::Evaluation*,
                                GeneratorExpressionContent const*,
                                Arguments&)>>
      listCommands{
        { "LENGTH"_s,
          [](cm::GenEx::Evaluation* ev, GeneratorExpressionContent const* cnt,
             Arguments& args) -> std::string {
            if (CheckListParameters(ev, cnt, "LENGTH"_s, args)) {
              return std::to_string(GetList(args.front()).size());
            }
            return std::string{};
          } },
        { "GET"_s,
          [](cm::GenEx::Evaluation* ev, GeneratorExpressionContent const* cnt,
             Arguments& args) -> std::string {
            if (CheckListParametersEx(ev, cnt, "GET"_s, args.size(), 2,
                                      false)) {
              auto list = GetList(args.front());
              if (list.empty()) {
                reportError(ev, cnt->GetOriginalExpression(),
                            "given empty list");
                return std::string{};
              }

              std::vector<cmList::index_type> indexes;
              if (!GetNumericArguments(ev, cnt, args.advance(1), indexes,
                                       cmList::ExpandElements::Yes)) {
                return std::string{};
              }
              try {
                return list.get_items(indexes.begin(), indexes.end())
                  .to_string();
              } catch (std::out_of_range& e) {
                reportError(ev, cnt->GetOriginalExpression(), e.what());
                return std::string{};
              }
            }
            return std::string{};
          } },
        { "JOIN"_s,
          [](cm::GenEx::Evaluation* ev, GeneratorExpressionContent const* cnt,
             Arguments& args) -> std::string {
            if (CheckListParameters(ev, cnt, "JOIN"_s, args, 2)) {
              return GetList(args.front()).join(args[1]);
            }
            return std::string{};
          } },
        { "SUBLIST"_s,
          [](cm::GenEx::Evaluation* ev, GeneratorExpressionContent const* cnt,
             Arguments& args) -> std::string {
            if (CheckListParameters(ev, cnt, "SUBLIST"_s, args, 3)) {
              auto list = GetList(args.front());
              if (!list.empty()) {
                std::vector<cmList::index_type> indexes;
                if (!GetNumericArguments(ev, cnt, args.advance(1), indexes)) {
                  return std::string{};
                }
                if (indexes[0] < 0) {
                  reportError(ev, cnt->GetOriginalExpression(),
                              cmStrCat("begin index: ", indexes[0],
                                       " is out of range 0 - ",
                                       list.size() - 1));
                  return std::string{};
                }
                if (indexes[1] < -1) {
                  reportError(ev, cnt->GetOriginalExpression(),
                              cmStrCat("length: ", indexes[1],
                                       " should be -1 or greater"));
                  return std::string{};
                }
                try {
                  return list
                    .sublist(static_cast<cmList::size_type>(indexes[0]),
                             static_cast<cmList::size_type>(indexes[1]))
                    .to_string();
                } catch (std::out_of_range& e) {
                  reportError(ev, cnt->GetOriginalExpression(), e.what());
                  return std::string{};
                }
              }
            }
            return std::string{};
          } },
        { "FIND"_s,
          [](cm::GenEx::Evaluation* ev, GeneratorExpressionContent const* cnt,
             Arguments& args) -> std::string {
            if (CheckListParameters(ev, cnt, "FIND"_s, args, 2)) {
              auto list = GetList(args.front());
              auto index = list.find(args[1]);
              return index == cmList::npos ? "-1" : std::to_string(index);
            }
            return std::string{};
          } },
        { "APPEND"_s,
          [](cm::GenEx::Evaluation* ev, GeneratorExpressionContent const* cnt,
             Arguments& args) -> std::string {
            if (CheckListParametersEx(ev, cnt, "APPEND"_s, args.size(), 2,
                                      false)) {
              auto list = args.front();
              args.advance(1);
              return cmList::append(list, args.begin(), args.end());
            }
            return std::string{};
          } },
        { "PREPEND"_s,
          [](cm::GenEx::Evaluation* ev, GeneratorExpressionContent const* cnt,
             Arguments& args) -> std::string {
            if (CheckListParametersEx(ev, cnt, "PREPEND"_s, args.size(), 2,
                                      false)) {
              auto list = args.front();
              args.advance(1);
              return cmList::prepend(list, args.begin(), args.end());
            }
            return std::string{};
          } },
        { "INSERT"_s,
          [](cm::GenEx::Evaluation* ev, GeneratorExpressionContent const* cnt,
             Arguments& args) -> std::string {
            if (CheckListParametersEx(ev, cnt, "INSERT"_s, args.size(), 3,
                                      false)) {
              cmList::index_type index;
              if (!GetNumericArgument(args[1], index)) {
                reportError(
                  ev, cnt->GetOriginalExpression(),
                  cmStrCat("index: \"", args[1], "\" is not a valid index"));
                return std::string{};
              }
              try {
                auto list = GetList(args.front());
                args.advance(2);
                list.insert_items(index, args.begin(), args.end(),
                                  cmList::ExpandElements::No,
                                  cmList::EmptyElements::Yes);
                return list.to_string();
              } catch (std::out_of_range& e) {
                reportError(ev, cnt->GetOriginalExpression(), e.what());
                return std::string{};
              }
            }
            return std::string{};
          } },
        { "POP_BACK"_s,
          [](cm::GenEx::Evaluation* ev, GeneratorExpressionContent const* cnt,
             Arguments& args) -> std::string {
            if (CheckListParameters(ev, cnt, "POP_BACK"_s, args)) {
              auto list = GetList(args.front());
              if (!list.empty()) {
                list.pop_back();
                return list.to_string();
              }
            }
            return std::string{};
          } },
        { "POP_FRONT"_s,
          [](cm::GenEx::Evaluation* ev, GeneratorExpressionContent const* cnt,
             Arguments& args) -> std::string {
            if (CheckListParameters(ev, cnt, "POP_FRONT"_s, args)) {
              auto list = GetList(args.front());
              if (!list.empty()) {
                list.pop_front();
                return list.to_string();
              }
            }
            return std::string{};
          } },
        { "REMOVE_DUPLICATES"_s,
          [](cm::GenEx::Evaluation* ev, GeneratorExpressionContent const* cnt,
             Arguments& args) -> std::string {
            if (CheckListParameters(ev, cnt, "REMOVE_DUPLICATES"_s, args)) {
              return GetList(args.front()).remove_duplicates().to_string();
            }
            return std::string{};
          } },
        { "REMOVE_ITEM"_s,
          [](cm::GenEx::Evaluation* ev, GeneratorExpressionContent const* cnt,
             Arguments& args) -> std::string {
            if (CheckListParametersEx(ev, cnt, "REMOVE_ITEM"_s, args.size(), 2,
                                      false)) {
              auto list = GetList(args.front());
              args.advance(1);
              cmList items{ args.begin(), args.end(),
                            cmList::ExpandElements::Yes };
              return list.remove_items(items.begin(), items.end()).to_string();
            }
            return std::string{};
          } },
        { "REMOVE_AT"_s,
          [](cm::GenEx::Evaluation* ev, GeneratorExpressionContent const* cnt,
             Arguments& args) -> std::string {
            if (CheckListParametersEx(ev, cnt, "REMOVE_AT"_s, args.size(), 2,
                                      false)) {
              auto list = GetList(args.front());
              std::vector<cmList::index_type> indexes;
              if (!GetNumericArguments(ev, cnt, args.advance(1), indexes,
                                       cmList::ExpandElements::Yes)) {
                return std::string{};
              }
              try {
                return list.remove_items(indexes.begin(), indexes.end())
                  .to_string();
              } catch (std::out_of_range& e) {
                reportError(ev, cnt->GetOriginalExpression(), e.what());
                return std::string{};
              }
            }
            return std::string{};
          } },
        { "FILTER"_s,
          [](cm::GenEx::Evaluation* ev, GeneratorExpressionContent const* cnt,
             Arguments& args) -> std::string {
            if (CheckListParameters(ev, cnt, "FILTER"_s, args, 3)) {
              auto const& op = args[1];
              if (op != "INCLUDE"_s && op != "EXCLUDE"_s) {
                reportError(
                  ev, cnt->GetOriginalExpression(),
                  cmStrCat("sub-command FILTER does not recognize operator \"",
                           op, "\". It must be either INCLUDE or EXCLUDE."));
                return std::string{};
              }
              try {
                return GetList(args.front())
                  .filter(args[2],
                          op == "INCLUDE"_s ? cmList::FilterMode::INCLUDE
                                            : cmList::FilterMode::EXCLUDE)
                  .to_string();
              } catch (std::invalid_argument&) {
                reportError(
                  ev, cnt->GetOriginalExpression(),
                  cmStrCat("sub-command FILTER, failed to compile regex \"",
                           args[2], "\"."));
                return std::string{};
              }
            }
            return std::string{};
          } },
        { "TRANSFORM"_s,
          [](cm::GenEx::Evaluation* ev, GeneratorExpressionContent const* cnt,
             Arguments& args) -> std::string {
            if (CheckListParametersEx(ev, cnt, "TRANSFORM"_s, args.size(), 2,
                                      false)) {
              auto list = GetList(args.front());
              if (!list.empty()) {
                struct ActionDescriptor
                {
                  ActionDescriptor(std::string name)
                    : Name(std::move(name))
                  {
                  }
                  ActionDescriptor(std::string name,
                                   cmList::TransformAction action, int arity)
                    : Name(std::move(name))
                    , Action(action)
                    , Arity(arity)
                  {
                  }

                  operator std::string const&() const { return this->Name; }

                  std::string Name;
                  cmList::TransformAction Action;
                  int Arity = 0;
                };

                static std::set<
                  ActionDescriptor,
                  std::function<bool(std::string const&, std::string const&)>>
                  descriptors{
                    { { "APPEND", cmList::TransformAction::APPEND, 1 },
                      { "PREPEND", cmList::TransformAction::PREPEND, 1 },
                      { "TOUPPER", cmList::TransformAction::TOUPPER, 0 },
                      { "TOLOWER", cmList::TransformAction::TOLOWER, 0 },
                      { "STRIP", cmList::TransformAction::STRIP, 0 },
                      { "REPLACE", cmList::TransformAction::REPLACE, 2 } },
                    [](std::string const& x, std::string const& y) {
                      return x < y;
                    }
                  };

                auto descriptor = descriptors.find(args.advance(1).front());
                if (descriptor == descriptors.end()) {
                  reportError(ev, cnt->GetOriginalExpression(),
                              cmStrCat(" sub-command TRANSFORM, ",
                                       args.front(), " invalid action."));
                  return std::string{};
                }

                // Action arguments
                args.advance(1);
                if (args.size() < descriptor->Arity) {
                  reportError(ev, cnt->GetOriginalExpression(),
                              cmStrCat("sub-command TRANSFORM, action ",
                                       descriptor->Name, " expects ",
                                       descriptor->Arity, " argument(s)."));
                  return std::string{};
                }
                std::vector<std::string> arguments;
                if (descriptor->Arity > 0) {
                  arguments = std::vector<std::string>(
                    args.begin(), args.begin() + descriptor->Arity);
                  args.advance(descriptor->Arity);
                }

                std::string const REGEX{ "REGEX" };
                std::string const AT{ "AT" };
                std::string const FOR{ "FOR" };
                std::unique_ptr<cmList::TransformSelector> selector;

                try {
                  // handle optional arguments
                  while (!args.empty()) {
                    if ((args.front() == REGEX || args.front() == AT ||
                         args.front() == FOR) &&
                        selector) {
                      reportError(ev, cnt->GetOriginalExpression(),
                                  cmStrCat("sub-command TRANSFORM, selector "
                                           "already specified (",
                                           selector->GetTag(), ")."));

                      return std::string{};
                    }

                    // REGEX selector
                    if (args.front() == REGEX) {
                      if (args.advance(1).empty()) {
                        reportError(
                          ev, cnt->GetOriginalExpression(),
                          "sub-command TRANSFORM, selector REGEX expects "
                          "'regular expression' argument.");
                        return std::string{};
                      }

                      selector = cmList::TransformSelector::New<
                        cmList::TransformSelector::REGEX>(args.front());

                      args.advance(1);
                      continue;
                    }

                    // AT selector
                    if (args.front() == AT) {
                      args.advance(1);
                      // get all specified indexes
                      std::vector<cmList::index_type> indexes;
                      while (!args.empty()) {
                        cmList indexList{ args.front() };
                        for (auto const& index : indexList) {
                          cmList::index_type value;

                          if (!GetNumericArgument(index, value)) {
                            // this is not a number, stop processing
                            reportError(
                              ev, cnt->GetOriginalExpression(),
                              cmStrCat("sub-command TRANSFORM, selector AT: '",
                                       index, "': unexpected argument."));
                            return std::string{};
                          }
                          indexes.push_back(value);
                        }
                        args.advance(1);
                      }

                      if (indexes.empty()) {
                        reportError(ev, cnt->GetOriginalExpression(),
                                    "sub-command TRANSFORM, selector AT "
                                    "expects at least one "
                                    "numeric value.");
                        return std::string{};
                      }

                      selector = cmList::TransformSelector::New<
                        cmList::TransformSelector::AT>(std::move(indexes));

                      continue;
                    }

                    // FOR selector
                    if (args.front() == FOR) {
                      if (args.advance(1).size() < 2) {
                        reportError(ev, cnt->GetOriginalExpression(),
                                    "sub-command TRANSFORM, selector FOR "
                                    "expects, at least,"
                                    " two arguments.");
                        return std::string{};
                      }

                      cmList::index_type start = 0;
                      cmList::index_type stop = 0;
                      cmList::index_type step = 1;
                      bool valid = false;

                      if (GetNumericArgument(args.front(), start) &&
                          GetNumericArgument(args.advance(1).front(), stop)) {
                        valid = true;
                      }

                      if (!valid) {
                        reportError(
                          ev, cnt->GetOriginalExpression(),
                          "sub-command TRANSFORM, selector FOR expects, "
                          "at least, two numeric values.");
                        return std::string{};
                      }
                      // try to read a third numeric value for step
                      if (!args.advance(1).empty()) {
                        if (!GetNumericArgument(args.front(), step)) {
                          // this is not a number
                          step = -1;
                        }
                        args.advance(1);
                      }

                      if (step <= 0) {
                        reportError(
                          ev, cnt->GetOriginalExpression(),
                          "sub-command TRANSFORM, selector FOR expects "
                          "positive numeric value for <step>.");
                        return std::string{};
                      }

                      selector = cmList::TransformSelector::New<
                        cmList::TransformSelector::FOR>({ start, stop, step });
                      continue;
                    }

                    reportError(ev, cnt->GetOriginalExpression(),
                                cmStrCat("sub-command TRANSFORM, '",
                                         cmJoin(args, ", "),
                                         "': unexpected argument(s)."));
                    return std::string{};
                  }

                  if (!selector) {
                    selector = cmList::TransformSelector::New();
                  }
                  selector->Makefile = ev->Context.LG->GetMakefile();

                  return list
                    .transform(descriptor->Action, arguments,
                               std::move(selector))
                    .to_string();
                } catch (cmList::transform_error& e) {
                  reportError(ev, cnt->GetOriginalExpression(), e.what());
                  return std::string{};
                }
              }
            }
            return std::string{};
          } },
        { "REVERSE"_s,
          [](cm::GenEx::Evaluation* ev, GeneratorExpressionContent const* cnt,
             Arguments& args) -> std::string {
            if (CheckListParameters(ev, cnt, "REVERSE"_s, args)) {
              return GetList(args.front()).reverse().to_string();
            }
            return std::string{};
          } },
        { "SORT"_s,
          [](cm::GenEx::Evaluation* ev, GeneratorExpressionContent const* cnt,
             Arguments& args) -> std::string {
            if (CheckListParametersEx(ev, cnt, "SORT"_s, args.size(), 1,
                                      false)) {
              auto list = GetList(args.front());
              args.advance(1);
              auto const COMPARE = "COMPARE:"_s;
              auto const CASE = "CASE:"_s;
              auto const ORDER = "ORDER:"_s;
              using SortConfig = cmList::SortConfiguration;
              SortConfig sortConfig;
              for (auto const& arg : args) {
                if (cmHasPrefix(arg, COMPARE)) {
                  if (sortConfig.Compare !=
                      SortConfig::CompareMethod::DEFAULT) {
                    reportError(ev, cnt->GetOriginalExpression(),
                                "sub-command SORT, COMPARE option has been "
                                "specified multiple times.");
                    return std::string{};
                  }
                  auto option =
                    cm::string_view{ arg.c_str() + COMPARE.length() };
                  if (option == "STRING"_s) {
                    sortConfig.Compare = SortConfig::CompareMethod::STRING;
                    continue;
                  }
                  if (option == "FILE_BASENAME"_s) {
                    sortConfig.Compare =
                      SortConfig::CompareMethod::FILE_BASENAME;
                    continue;
                  }
                  if (option == "NATURAL"_s) {
                    sortConfig.Compare = SortConfig::CompareMethod::NATURAL;
                    continue;
                  }
                  reportError(
                    ev, cnt->GetOriginalExpression(),
                    cmStrCat(
                      "sub-command SORT, an invalid COMPARE option has been "
                      "specified: \"",
                      option, "\"."));
                  return std::string{};
                }
                if (cmHasPrefix(arg, CASE)) {
                  if (sortConfig.Case !=
                      SortConfig::CaseSensitivity::DEFAULT) {
                    reportError(ev, cnt->GetOriginalExpression(),
                                "sub-command SORT, CASE option has been "
                                "specified multiple times.");
                    return std::string{};
                  }
                  auto option = cm::string_view{ arg.c_str() + CASE.length() };
                  if (option == "SENSITIVE"_s) {
                    sortConfig.Case = SortConfig::CaseSensitivity::SENSITIVE;
                    continue;
                  }
                  if (option == "INSENSITIVE"_s) {
                    sortConfig.Case = SortConfig::CaseSensitivity::INSENSITIVE;
                    continue;
                  }
                  reportError(
                    ev, cnt->GetOriginalExpression(),
                    cmStrCat(
                      "sub-command SORT, an invalid CASE option has been "
                      "specified: \"",
                      option, "\"."));
                  return std::string{};
                }
                if (cmHasPrefix(arg, ORDER)) {
                  if (sortConfig.Order != SortConfig::OrderMode::DEFAULT) {
                    reportError(ev, cnt->GetOriginalExpression(),
                                "sub-command SORT, ORDER option has been "
                                "specified multiple times.");
                    return std::string{};
                  }
                  auto option =
                    cm::string_view{ arg.c_str() + ORDER.length() };
                  if (option == "ASCENDING"_s) {
                    sortConfig.Order = SortConfig::OrderMode::ASCENDING;
                    continue;
                  }
                  if (option == "DESCENDING"_s) {
                    sortConfig.Order = SortConfig::OrderMode::DESCENDING;
                    continue;
                  }
                  reportError(
                    ev, cnt->GetOriginalExpression(),
                    cmStrCat(
                      "sub-command SORT, an invalid ORDER option has been "
                      "specified: \"",
                      option, "\"."));
                  return std::string{};
                }
                reportError(ev, cnt->GetOriginalExpression(),
                            cmStrCat("sub-command SORT, option \"", arg,
                                     "\" is invalid."));
                return std::string{};
              }

              return list.sort(sortConfig).to_string();
            }
            return std::string{};
          } }
      };

    if (cm::contains(listCommands, parameters.front())) {
      auto args = Arguments{ parameters }.advance(1);
      return listCommands[parameters.front()](eval, content, args);
    }

    reportError(eval, content->GetOriginalExpression(),
                cmStrCat(parameters.front(), ": invalid option."));
    return std::string{};
  }
} listNode;

static const struct MakeCIdentifierNode : public cmGeneratorExpressionNode
{
  MakeCIdentifierNode() {} // NOLINT(modernize-use-equals-default)

  bool AcceptsArbitraryContentParameter() const override { return true; }

  std::string Evaluate(
    std::vector<std::string> const& parameters,
    cm::GenEx::Evaluation* /*eval*/,
    GeneratorExpressionContent const* /*content*/,
    cmGeneratorExpressionDAGChecker* /*dagChecker*/) const override
  {
    return cmSystemTools::MakeCidentifier(parameters.front());
  }
} makeCIdentifierNode;

template <char C>
struct CharacterNode : public cmGeneratorExpressionNode
{
  CharacterNode() {} // NOLINT(modernize-use-equals-default)

  int NumExpectedParameters() const override { return 0; }

  std::string Evaluate(
    std::vector<std::string> const& /*parameters*/,
    cm::GenEx::Evaluation* /*eval*/,
    GeneratorExpressionContent const* /*content*/,
    cmGeneratorExpressionDAGChecker* /*dagChecker*/) const override
  {
    return { C };
  }
};
static CharacterNode<'>'> const angle_rNode;
static CharacterNode<','> const commaNode;
static CharacterNode<';'> const semicolonNode;
static CharacterNode<'"'> const quoteNode;

struct CompilerIdNode : public cmGeneratorExpressionNode
{
  CompilerIdNode(char const* compilerLang)
    : CompilerLanguage(compilerLang)
  {
  }

  int NumExpectedParameters() const override { return ZeroOrMoreParameters; }

  std::string Evaluate(
    std::vector<std::string> const& parameters, cm::GenEx::Evaluation* eval,
    GeneratorExpressionContent const* content,
    cmGeneratorExpressionDAGChecker* dagChecker) const override
  {
    if (!eval->HeadTarget) {
      std::ostringstream e;
      e << "$<" << this->CompilerLanguage
        << "_COMPILER_ID> may only be used with binary targets.  It may "
           "not be used with add_custom_command or add_custom_target.";
      reportError(eval, content->GetOriginalExpression(), e.str());
      return {};
    }
    return this->EvaluateWithLanguage(parameters, eval, content, dagChecker,
                                      this->CompilerLanguage);
  }

  std::string EvaluateWithLanguage(std::vector<std::string> const& parameters,
                                   cm::GenEx::Evaluation* eval,
                                   GeneratorExpressionContent const* content,
                                   cmGeneratorExpressionDAGChecker* /*unused*/,
                                   std::string const& lang) const
  {
    std::string const& compilerId =
      eval->Context.LG->GetMakefile()->GetSafeDefinition("CMAKE_" + lang +
                                                         "_COMPILER_ID");
    if (parameters.empty()) {
      return compilerId;
    }
    if (compilerId.empty()) {
      return parameters.front().empty() ? "1" : "0";
    }
    static cmsys::RegularExpression compilerIdValidator("^[A-Za-z0-9_]*$");

    for (auto const& param : parameters) {
      if (!compilerIdValidator.find(param)) {
        reportError(eval, content->GetOriginalExpression(),
                    "Expression syntax not recognized.");
        return std::string();
      }

      if (strcmp(param.c_str(), compilerId.c_str()) == 0) {
        return "1";
      }
    }
    return "0";
  }

  char const* const CompilerLanguage;
};

static CompilerIdNode const cCompilerIdNode("C"), cxxCompilerIdNode("CXX"),
  cudaCompilerIdNode("CUDA"), objcCompilerIdNode("OBJC"),
  objcxxCompilerIdNode("OBJCXX"), fortranCompilerIdNode("Fortran"),
  hipCompilerIdNode("HIP"), ispcCompilerIdNode("ISPC");

struct CompilerVersionNode : public cmGeneratorExpressionNode
{
  CompilerVersionNode(char const* compilerLang)
    : CompilerLanguage(compilerLang)
  {
  }

  int NumExpectedParameters() const override { return OneOrZeroParameters; }

  std::string Evaluate(
    std::vector<std::string> const& parameters, cm::GenEx::Evaluation* eval,
    GeneratorExpressionContent const* content,
    cmGeneratorExpressionDAGChecker* dagChecker) const override
  {
    if (!eval->HeadTarget) {
      std::ostringstream e;
      e << "$<" << this->CompilerLanguage
        << "_COMPILER_VERSION> may only be used with binary targets.  It "
           "may not be used with add_custom_command or add_custom_target.";
      reportError(eval, content->GetOriginalExpression(), e.str());
      return {};
    }
    return this->EvaluateWithLanguage(parameters, eval, content, dagChecker,
                                      this->CompilerLanguage);
  }

  std::string EvaluateWithLanguage(std::vector<std::string> const& parameters,
                                   cm::GenEx::Evaluation* eval,
                                   GeneratorExpressionContent const* content,
                                   cmGeneratorExpressionDAGChecker* /*unused*/,
                                   std::string const& lang) const
  {
    std::string const& compilerVersion =
      eval->Context.LG->GetMakefile()->GetSafeDefinition("CMAKE_" + lang +
                                                         "_COMPILER_VERSION");
    if (parameters.empty()) {
      return compilerVersion;
    }

    static cmsys::RegularExpression compilerIdValidator("^[0-9\\.]*$");
    if (!compilerIdValidator.find(parameters.front())) {
      reportError(eval, content->GetOriginalExpression(),
                  "Expression syntax not recognized.");
      return {};
    }
    if (compilerVersion.empty()) {
      return parameters.front().empty() ? "1" : "0";
    }

    return cmSystemTools::VersionCompare(cmSystemTools::OP_EQUAL,
                                         parameters.front(), compilerVersion)
      ? "1"
      : "0";
  }

  char const* const CompilerLanguage;
};

static CompilerVersionNode const cCompilerVersionNode("C"),
  cxxCompilerVersionNode("CXX"), cudaCompilerVersionNode("CUDA"),
  objcCompilerVersionNode("OBJC"), objcxxCompilerVersionNode("OBJCXX"),
  fortranCompilerVersionNode("Fortran"), ispcCompilerVersionNode("ISPC"),
  hipCompilerVersionNode("HIP");

struct CompilerFrontendVariantNode : public cmGeneratorExpressionNode
{
  CompilerFrontendVariantNode(char const* compilerLang)
    : CompilerLanguage(compilerLang)
  {
  }

  int NumExpectedParameters() const override { return ZeroOrMoreParameters; }

  std::string Evaluate(
    std::vector<std::string> const& parameters, cm::GenEx::Evaluation* eval,
    GeneratorExpressionContent const* content,
    cmGeneratorExpressionDAGChecker* dagChecker) const override
  {
    if (!eval->HeadTarget) {
      std::ostringstream e;
      e << "$<" << this->CompilerLanguage
        << "_COMPILER_FRONTEND_VARIANT> may only be used with binary targets. "
           " It may not be used with add_custom_command or add_custom_target.";
      reportError(eval, content->GetOriginalExpression(), e.str());
      return {};
    }
    return this->EvaluateWithLanguage(parameters, eval, content, dagChecker,
                                      this->CompilerLanguage);
  }

  std::string EvaluateWithLanguage(std::vector<std::string> const& parameters,
                                   cm::GenEx::Evaluation* eval,
                                   GeneratorExpressionContent const* content,
                                   cmGeneratorExpressionDAGChecker* /*unused*/,
                                   std::string const& lang) const
  {
    std::string const& compilerFrontendVariant =
      eval->Context.LG->GetMakefile()->GetSafeDefinition(
        cmStrCat("CMAKE_", lang, "_COMPILER_FRONTEND_VARIANT"));
    if (parameters.empty()) {
      return compilerFrontendVariant;
    }
    if (compilerFrontendVariant.empty()) {
      return parameters.front().empty() ? "1" : "0";
    }
    static cmsys::RegularExpression compilerFrontendVariantValidator(
      "^[A-Za-z0-9_]*$");

    for (auto const& param : parameters) {
      if (!compilerFrontendVariantValidator.find(param)) {
        reportError(eval, content->GetOriginalExpression(),
                    "Expression syntax not recognized.");
        return {};
      }
      if (strcmp(param.c_str(), compilerFrontendVariant.c_str()) == 0) {
        return "1";
      }
    }
    return "0";
  }

  char const* const CompilerLanguage;
};

static CompilerFrontendVariantNode const cCompilerFrontendVariantNode("C"),
  cxxCompilerFrontendVariantNode("CXX"),
  cudaCompilerFrontendVariantNode("CUDA"),
  objcCompilerFrontendVariantNode("OBJC"),
  objcxxCompilerFrontendVariantNode("OBJCXX"),
  fortranCompilerFrontendVariantNode("Fortran"),
  hipCompilerFrontendVariantNode("HIP"),
  ispcCompilerFrontendVariantNode("ISPC");

struct PlatformIdNode : public cmGeneratorExpressionNode
{
  PlatformIdNode() {} // NOLINT(modernize-use-equals-default)

  int NumExpectedParameters() const override { return ZeroOrMoreParameters; }

  std::string Evaluate(
    std::vector<std::string> const& parameters, cm::GenEx::Evaluation* eval,
    GeneratorExpressionContent const* /*content*/,
    cmGeneratorExpressionDAGChecker* /*dagChecker*/) const override
  {
    std::string const& platformId =
      eval->Context.LG->GetMakefile()->GetSafeDefinition("CMAKE_SYSTEM_NAME");
    if (parameters.empty()) {
      return platformId;
    }

    if (platformId.empty()) {
      return parameters.front().empty() ? "1" : "0";
    }

    for (auto const& param : parameters) {
      if (param == platformId) {
        return "1";
      }
    }
    return "0";
  }
};
static struct PlatformIdNode platformIdNode;

template <cmSystemTools::CompareOp Op>
struct VersionNode : public cmGeneratorExpressionNode
{
  VersionNode() {} // NOLINT(modernize-use-equals-default)

  int NumExpectedParameters() const override { return 2; }

  std::string Evaluate(
    std::vector<std::string> const& parameters,
    cm::GenEx::Evaluation* /*eval*/,
    GeneratorExpressionContent const* /*content*/,
    cmGeneratorExpressionDAGChecker* /*dagChecker*/) const override
  {
    return cmSystemTools::VersionCompare(Op, parameters.front(), parameters[1])
      ? "1"
      : "0";
  }
};

static VersionNode<cmSystemTools::OP_GREATER> const versionGreaterNode;
static VersionNode<cmSystemTools::OP_GREATER_EQUAL> const versionGreaterEqNode;
static VersionNode<cmSystemTools::OP_LESS> const versionLessNode;
static VersionNode<cmSystemTools::OP_LESS_EQUAL> const versionLessEqNode;
static VersionNode<cmSystemTools::OP_EQUAL> const versionEqualNode;

static const struct CompileOnlyNode : public cmGeneratorExpressionNode
{
  CompileOnlyNode() {} // NOLINT(modernize-use-equals-default)

  std::string Evaluate(
    std::vector<std::string> const& parameters, cm::GenEx::Evaluation* eval,
    GeneratorExpressionContent const* content,
    cmGeneratorExpressionDAGChecker* dagChecker) const override
  {
    if (!dagChecker) {
      reportError(eval, content->GetOriginalExpression(),
                  "$<COMPILE_ONLY:...> may only be used for linking");
      return std::string();
    }
    if (dagChecker->GetTransitivePropertiesOnly()) {
      return parameters.front();
    }
    return std::string{};
  }
} compileOnlyNode;

static const struct LinkOnlyNode : public cmGeneratorExpressionNode
{
  LinkOnlyNode() {} // NOLINT(modernize-use-equals-default)

  std::string Evaluate(
    std::vector<std::string> const& parameters, cm::GenEx::Evaluation* eval,
    GeneratorExpressionContent const* content,
    cmGeneratorExpressionDAGChecker* dagChecker) const override
  {
    if (!dagChecker) {
      reportError(eval, content->GetOriginalExpression(),
                  "$<LINK_ONLY:...> may only be used for linking");
      return std::string();
    }
    if (!dagChecker->GetTransitivePropertiesOnlyCMP0131()) {
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
    std::vector<std::string> const& /*parameters*/,
    cm::GenEx::Evaluation* eval, GeneratorExpressionContent const* /*content*/,
    cmGeneratorExpressionDAGChecker* /*dagChecker*/) const override
  {
    eval->HadContextSensitiveCondition = true;
    return eval->Context.Config;
  }
} configurationNode;

static const struct ConfigurationTestNode : public cmGeneratorExpressionNode
{
  ConfigurationTestNode() {} // NOLINT(modernize-use-equals-default)

  int NumExpectedParameters() const override { return ZeroOrMoreParameters; }

  std::string Evaluate(
    std::vector<std::string> const& parameters, cm::GenEx::Evaluation* eval,
    GeneratorExpressionContent const* content,
    cmGeneratorExpressionDAGChecker* /*dagChecker*/) const override
  {
    if (parameters.empty()) {
      return configurationNode.Evaluate(parameters, eval, content, nullptr);
    }

    eval->HadContextSensitiveCondition = true;

    // First, validate our arguments.
    static cmsys::RegularExpression configValidator("^[A-Za-z0-9_]*$");
    bool firstParam = true;
    for (auto const& param : parameters) {
      if (!configValidator.find(param)) {
        if (firstParam) {
          reportError(eval, content->GetOriginalExpression(),
                      "Expression syntax not recognized.");
          return std::string();
        }
        // for backwards compat invalid config names are only errors as
        // the first parameter
        std::ostringstream e;
        /* clang-format off */
        e << "Warning evaluating generator expression:\n"
          << "  " << content->GetOriginalExpression() << "\n"
          << "The config name of \"" << param << "\" is invalid";
        /* clang-format on */
        eval->Context.LG->GetCMakeInstance()->IssueMessage(
          MessageType::WARNING, e.str(), eval->Backtrace);
      }
      firstParam = false;
    }

    // Partially determine the context(s) in which the expression should be
    // evaluated.
    //
    // If CMPxxxx is NEW, the context is exactly one of the imported target's
    // selected configuration, if applicable and if the target was imported
    // from CPS, or the consuming target's configuration otherwise. Here, we
    // determine if we are in that 'otherwise' branch.
    //
    // Longer term, we need a way for non-CPS users to match the selected
    // configuration of the imported target. At that time, CPS should switch
    // to that mechanism and the CPS-specific logic here should be dropped.
    // (We can do that because CPS doesn't use generator expressions directly;
    // rather, CMake generates them on import.)
    bool const targetIsImported =
      (eval->CurrentTarget && eval->CurrentTarget->IsImported());
    bool const useConsumerConfig =
      (targetIsImported &&
       eval->CurrentTarget->Target->GetOrigin() != cmTarget::Origin::Cps);

    if (!targetIsImported || useConsumerConfig) {
      // Does the consuming target's configuration match any of the arguments?
      for (auto const& param : parameters) {
        if (eval->Context.Config.empty()) {
          if (param.empty()) {
            return "1";
          }
        } else if (cmsysString_strcasecmp(param.c_str(),
                                          eval->Context.Config.c_str()) == 0) {
          return "1";
        }
      }
    }

    if (targetIsImported) {
      cmValue loc = nullptr;
      cmValue imp = nullptr;
      std::string suffix;
      if (eval->CurrentTarget->Target->GetMappedConfig(eval->Context.Config,
                                                       loc, imp, suffix)) {
        // Finish determine the context(s) in which the expression should be
        // evaluated. Note that we use the consumer's policy, so that end users
        // can override the imported target's policy. This may be needed if
        // upstream has changed their policy version without realizing that
        // consumers were depending on the OLD behavior.
        bool const oldPolicy = [&] {
          if (!useConsumerConfig) {
            // Targets imported from CPS shall use only the selected
            // configuration of the imported target.
            return false;
          }
          cmLocalGenerator const* const lg = eval->Context.LG;
          switch (eval->HeadTarget->GetPolicyStatusCMP0199()) {
            case cmPolicies::WARN:
              if (lg->GetMakefile()->PolicyOptionalWarningEnabled(
                    "CMAKE_POLICY_WARNING_CMP0199")) {
                std::string const err =
                  cmStrCat(cmPolicies::GetPolicyWarning(cmPolicies::CMP0199),
                           "\nEvaluation of $<CONFIG> for imported target  \"",
                           eval->CurrentTarget->GetName(), "\", used by \"",
                           eval->HeadTarget->GetName(),
                           "\", may match multiple configurations.\n");
                lg->GetCMakeInstance()->IssueMessage(
                  MessageType ::AUTHOR_WARNING, err, eval->Backtrace);
              }
              CM_FALLTHROUGH;
            case cmPolicies::OLD:
              return true;
            case cmPolicies::NEW:
              return false;
          }

          // Should be unreachable
          assert(false);
          return false;
        }();

        if (oldPolicy) {
          // If CMPxxxx is OLD (and we aren't dealing with a target imported
          // form CPS), we already evaluated in the context of the consuming
          // target. Next, for imported targets, we will evaluate based on the
          // mapped configurations.
          //
          // If the target has a MAP_IMPORTED_CONFIG_<CONFIG> property for the
          // consumer's <CONFIG>, we will match *any* config in that list,
          // regardless of whether it's valid or of what GetMappedConfig
          // actually picked. This will result in $<CONFIG> producing '1' for
          // multiple configs, and is almost certainly wrong, but it's what
          // CMake did for a very long time, and... Hyrum's Law.
          cmList mappedConfigs;
          std::string mapProp =
            cmStrCat("MAP_IMPORTED_CONFIG_",
                     cmSystemTools::UpperCase(eval->Context.Config));
          if (cmValue mapValue = eval->CurrentTarget->GetProperty(mapProp)) {
            mappedConfigs.assign(cmSystemTools::UpperCase(*mapValue));

            for (auto const& param : parameters) {
              if (cm::contains(mappedConfigs,
                               cmSystemTools::UpperCase(param))) {
                return "1";
              }
            }

            return "0";
          }
        }

        // Finally, check if we selected (possibly via mapping) a configuration
        // for this imported target, and if we should evaluate the expression
        // in the context of the same.
        //
        // For targets imported from CPS, this is the only context we evaluate
        // the expression.
        if (!suffix.empty()) {
          for (auto const& param : parameters) {
            if (cmStrCat('_', cmSystemTools::UpperCase(param)) == suffix) {
              return "1";
            }
          }
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
    std::vector<std::string> const& parameters,
    cm::GenEx::Evaluation* /*eval*/,
    GeneratorExpressionContent const* /*content*/,
    cmGeneratorExpressionDAGChecker* /*dagChecker*/) const override
  {
    return cmList{ parameters.front() }.join(parameters[1]);
  }
} joinNode;

static const struct CompileLanguageNode : public cmGeneratorExpressionNode
{
  CompileLanguageNode() {} // NOLINT(modernize-use-equals-default)

  int NumExpectedParameters() const override { return ZeroOrMoreParameters; }

  std::string Evaluate(
    std::vector<std::string> const& parameters, cm::GenEx::Evaluation* eval,
    GeneratorExpressionContent const* content,
    cmGeneratorExpressionDAGChecker* dagChecker) const override
  {
    if (eval->Context.Language.empty() &&
        (!dagChecker || !dagChecker->EvaluatingCompileExpression())) {
      reportError(
        eval, content->GetOriginalExpression(),
        "$<COMPILE_LANGUAGE:...> may only be used to specify include "
        "directories, compile definitions, compile options, and to evaluate "
        "components of the file(GENERATE) command.");
      return std::string();
    }

    cmGlobalGenerator const* gg = eval->Context.LG->GetGlobalGenerator();
    std::string genName = gg->GetName();
    if (genName.find("Makefiles") == std::string::npos &&
        genName.find("Ninja") == std::string::npos &&
        genName.find("Visual Studio") == std::string::npos &&
        genName.find("Xcode") == std::string::npos &&
        genName.find("Watcom WMake") == std::string::npos &&
        genName.find("FASTBuild") == std::string::npos &&
        genName.find("Green Hills MULTI") == std::string::npos) {
      reportError(eval, content->GetOriginalExpression(),
                  "$<COMPILE_LANGUAGE:...> not supported for this generator.");
      return std::string();
    }
    if (parameters.empty()) {
      return eval->Context.Language;
    }

    for (auto const& param : parameters) {
      if (eval->Context.Language == param) {
        return "1";
      }
    }
    return "0";
  }
} languageNode;

static const struct CompileLanguageAndIdNode : public cmGeneratorExpressionNode
{
  CompileLanguageAndIdNode() {} // NOLINT(modernize-use-equals-default)

  int NumExpectedParameters() const override { return TwoOrMoreParameters; }

  std::string Evaluate(
    std::vector<std::string> const& parameters, cm::GenEx::Evaluation* eval,
    GeneratorExpressionContent const* content,
    cmGeneratorExpressionDAGChecker* dagChecker) const override
  {
    if (!eval->HeadTarget ||
        (eval->Context.Language.empty() &&
         (!dagChecker || !dagChecker->EvaluatingCompileExpression()))) {
      // reportError(eval, content->GetOriginalExpression(), "");
      reportError(
        eval, content->GetOriginalExpression(),
        "$<COMPILE_LANG_AND_ID:lang,id> may only be used with binary "
        "targets "
        "to specify include directories, compile definitions, and compile "
        "options.  It may not be used with the add_custom_command, "
        "add_custom_target, or file(GENERATE) commands.");
      return std::string();
    }
    cmGlobalGenerator const* gg = eval->Context.LG->GetGlobalGenerator();
    std::string genName = gg->GetName();
    if (genName.find("Makefiles") == std::string::npos &&
        genName.find("Ninja") == std::string::npos &&
        genName.find("FASTBuild") == std::string::npos &&
        genName.find("Visual Studio") == std::string::npos &&
        genName.find("Xcode") == std::string::npos &&
        genName.find("Watcom WMake") == std::string::npos &&
        genName.find("Green Hills MULTI") == std::string::npos) {
      reportError(
        eval, content->GetOriginalExpression(),
        "$<COMPILE_LANG_AND_ID:lang,id> not supported for this generator.");
      return std::string();
    }

    std::string const& lang = eval->Context.Language;
    if (lang == parameters.front()) {
      std::vector<std::string> idParameter((parameters.cbegin() + 1),
                                           parameters.cend());
      return CompilerIdNode{ lang.c_str() }.EvaluateWithLanguage(
        idParameter, eval, content, dagChecker, lang);
    }
    return "0";
  }
} languageAndIdNode;

static const struct LinkLanguageNode : public cmGeneratorExpressionNode
{
  LinkLanguageNode() {} // NOLINT(modernize-use-equals-default)

  int NumExpectedParameters() const override { return ZeroOrMoreParameters; }

  std::string Evaluate(
    std::vector<std::string> const& parameters, cm::GenEx::Evaluation* eval,
    GeneratorExpressionContent const* content,
    cmGeneratorExpressionDAGChecker* dagChecker) const override
  {
    if (!eval->HeadTarget || !dagChecker ||
        !(dagChecker->EvaluatingLinkExpression() ||
          dagChecker->EvaluatingLinkLibraries() ||
          dagChecker->EvaluatingLinkerLauncher())) {
      reportError(eval, content->GetOriginalExpression(),
                  "$<LINK_LANGUAGE:...> may only be used with binary targets "
                  "to specify link libraries, link directories, link options "
                  "and link depends.");
      return std::string();
    }
    if (dagChecker->EvaluatingLinkLibraries() && parameters.empty()) {
      reportError(
        eval, content->GetOriginalExpression(),
        "$<LINK_LANGUAGE> is not supported in link libraries expression.");
      return std::string();
    }

    cmGlobalGenerator const* gg = eval->Context.LG->GetGlobalGenerator();
    std::string genName = gg->GetName();
    if (genName.find("Makefiles") == std::string::npos &&
        genName.find("Ninja") == std::string::npos &&
        genName.find("FASTBuild") == std::string::npos &&
        genName.find("Visual Studio") == std::string::npos &&
        genName.find("Xcode") == std::string::npos &&
        genName.find("Watcom WMake") == std::string::npos &&
        genName.find("Green Hills MULTI") == std::string::npos) {
      reportError(eval, content->GetOriginalExpression(),
                  "$<LINK_LANGUAGE:...> not supported for this generator.");
      return std::string();
    }

    if (dagChecker->EvaluatingLinkLibraries()) {
      eval->HadHeadSensitiveCondition = true;
      eval->HadLinkLanguageSensitiveCondition = true;
    }

    if (parameters.empty()) {
      return eval->Context.Language;
    }

    for (auto const& param : parameters) {
      if (eval->Context.Language == param) {
        return "1";
      }
    }
    return "0";
  }
} linkLanguageNode;

namespace {
struct LinkerId
{
  static std::string Evaluate(std::vector<std::string> const& parameters,
                              cm::GenEx::Evaluation* eval,
                              GeneratorExpressionContent const* content,
                              std::string const& lang)
  {
    std::string const& linkerId =
      eval->Context.LG->GetMakefile()->GetSafeDefinition("CMAKE_" + lang +
                                                         "_COMPILER_ID");
    if (parameters.empty()) {
      return linkerId;
    }
    if (linkerId.empty()) {
      return parameters.front().empty() ? "1" : "0";
    }
    static cmsys::RegularExpression linkerIdValidator("^[A-Za-z0-9_]*$");

    for (auto const& param : parameters) {
      if (!linkerIdValidator.find(param)) {
        reportError(eval, content->GetOriginalExpression(),
                    "Expression syntax not recognized.");
        return std::string();
      }

      if (param == linkerId) {
        return "1";
      }
    }
    return "0";
  }
};
}

static const struct LinkLanguageAndIdNode : public cmGeneratorExpressionNode
{
  LinkLanguageAndIdNode() {} // NOLINT(modernize-use-equals-default)

  int NumExpectedParameters() const override { return TwoOrMoreParameters; }

  std::string Evaluate(
    std::vector<std::string> const& parameters, cm::GenEx::Evaluation* eval,
    GeneratorExpressionContent const* content,
    cmGeneratorExpressionDAGChecker* dagChecker) const override
  {
    if (!eval->HeadTarget || !dagChecker ||
        !(dagChecker->EvaluatingLinkExpression() ||
          dagChecker->EvaluatingLinkLibraries() ||
          dagChecker->EvaluatingLinkerLauncher())) {
      reportError(
        eval, content->GetOriginalExpression(),
        "$<LINK_LANG_AND_ID:lang,id> may only be used with binary targets "
        "to specify link libraries, link directories, link options, and "
        "link "
        "depends.");
      return std::string();
    }

    cmGlobalGenerator const* gg = eval->Context.LG->GetGlobalGenerator();
    std::string genName = gg->GetName();
    if (genName.find("Makefiles") == std::string::npos &&
        genName.find("Ninja") == std::string::npos &&
        genName.find("FASTBuild") == std::string::npos &&
        genName.find("Visual Studio") == std::string::npos &&
        genName.find("Xcode") == std::string::npos &&
        genName.find("Watcom WMake") == std::string::npos &&
        genName.find("Green Hills MULTI") == std::string::npos) {
      reportError(
        eval, content->GetOriginalExpression(),
        "$<LINK_LANG_AND_ID:lang,id> not supported for this generator.");
      return std::string();
    }

    if (dagChecker->EvaluatingLinkLibraries()) {
      eval->HadHeadSensitiveCondition = true;
      eval->HadLinkLanguageSensitiveCondition = true;
    }

    std::string const& lang = eval->Context.Language;
    if (lang == parameters.front()) {
      std::vector<std::string> idParameter((parameters.cbegin() + 1),
                                           parameters.cend());
      return LinkerId::Evaluate(idParameter, eval, content, lang);
    }
    return "0";
  }
} linkLanguageAndIdNode;

struct CompilerLinkerIdNode : public cmGeneratorExpressionNode
{
  CompilerLinkerIdNode(char const* lang)
    : Language(lang)
  {
  }

  int NumExpectedParameters() const override { return ZeroOrMoreParameters; }

  std::string Evaluate(
    std::vector<std::string> const& parameters, cm::GenEx::Evaluation* eval,
    GeneratorExpressionContent const* content,
    cmGeneratorExpressionDAGChecker* dagChecker) const override
  {
    if (!eval->HeadTarget) {
      reportError(
        eval, content->GetOriginalExpression(),
        cmStrCat(
          "$<", this->Language,
          "_COMPILER_LINKER_ID> may only be used with binary targets. It may "
          "not be used with add_custom_command or add_custom_target."));
      return {};
    }
    return this->EvaluateWithLanguage(parameters, eval, content, dagChecker,
                                      this->Language);
  }

  std::string EvaluateWithLanguage(std::vector<std::string> const& parameters,
                                   cm::GenEx::Evaluation* eval,
                                   GeneratorExpressionContent const* content,
                                   cmGeneratorExpressionDAGChecker* /*unused*/,
                                   std::string const& lang) const
  {
    std::string const& compilerLinkerId =
      eval->Context.LG->GetMakefile()->GetSafeDefinition(
        cmStrCat("CMAKE_", lang, "_COMPILER_LINKER_ID"));
    if (parameters.empty()) {
      return compilerLinkerId;
    }
    if (compilerLinkerId.empty()) {
      return parameters.front().empty() ? "1" : "0";
    }
    static cmsys::RegularExpression compilerLinkerIdValidator(
      "^[A-Za-z0-9_]*$");

    for (auto const& param : parameters) {
      if (!compilerLinkerIdValidator.find(param)) {
        reportError(eval, content->GetOriginalExpression(),
                    "Expression syntax not recognized.");
        return std::string();
      }

      if (param == compilerLinkerId) {
        return "1";
      }
    }
    return "0";
  }

  char const* const Language;
};

static CompilerLinkerIdNode const cCompilerLinkerIdNode("C"),
  cxxCompilerLinkerIdNode("CXX"), cudaCompilerLinkerIdNode("CUDA"),
  objcCompilerLinkerIdNode("OBJC"), objcxxCompilerLinkerIdNode("OBJCXX"),
  fortranCompilerLinkerIdNode("Fortran"), hipCompilerLinkerIdNode("HIP");

struct CompilerLinkerFrontendVariantNode : public cmGeneratorExpressionNode
{
  CompilerLinkerFrontendVariantNode(char const* lang)
    : Language(lang)
  {
  }

  int NumExpectedParameters() const override { return ZeroOrMoreParameters; }

  std::string Evaluate(
    std::vector<std::string> const& parameters, cm::GenEx::Evaluation* eval,
    GeneratorExpressionContent const* content,
    cmGeneratorExpressionDAGChecker* dagChecker) const override
  {
    if (!eval->HeadTarget) {
      reportError(
        eval, content->GetOriginalExpression(),
        cmStrCat(
          "$<", this->Language,
          "_COMPILER_LINKER_FRONTEND_VARIANT> may only be used with binary "
          "targets. It may not be used with add_custom_command or "
          "add_custom_target."));
      return {};
    }
    return this->EvaluateWithLanguage(parameters, eval, content, dagChecker,
                                      this->Language);
  }

  std::string EvaluateWithLanguage(std::vector<std::string> const& parameters,
                                   cm::GenEx::Evaluation* eval,
                                   GeneratorExpressionContent const* content,
                                   cmGeneratorExpressionDAGChecker* /*unused*/,
                                   std::string const& lang) const
  {
    std::string const& compilerLinkerFrontendVariant =
      eval->Context.LG->GetMakefile()->GetSafeDefinition(
        cmStrCat("CMAKE_", lang, "_COMPILER_LINKER_FRONTEND_VARIANT"));
    if (parameters.empty()) {
      return compilerLinkerFrontendVariant;
    }
    if (compilerLinkerFrontendVariant.empty()) {
      return parameters.front().empty() ? "1" : "0";
    }
    static cmsys::RegularExpression compilerLinkerFrontendVariantValidator(
      "^[A-Za-z0-9_]*$");

    for (auto const& param : parameters) {
      if (!compilerLinkerFrontendVariantValidator.find(param)) {
        reportError(eval, content->GetOriginalExpression(),
                    "Expression syntax not recognized.");
        return {};
      }
      if (param == compilerLinkerFrontendVariant) {
        return "1";
      }
    }
    return "0";
  }

  char const* const Language;
};

static CompilerLinkerFrontendVariantNode const
  cCompilerLinkerFrontendVariantNode("C"),
  cxxCompilerLinkerFrontendVariantNode("CXX"),
  cudaCompilerLinkerFrontendVariantNode("CUDA"),
  objcCompilerLinkerFrontendVariantNode("OBJC"),
  objcxxCompilerLinkerFrontendVariantNode("OBJCXX"),
  fortranCompilerLinkerFrontendVariantNode("Fortran"),
  hipCompilerLinkerFrontendVariantNode("HIP");

static const struct LinkLibraryNode : public cmGeneratorExpressionNode
{
  LinkLibraryNode() {} // NOLINT(modernize-use-equals-default)

  int NumExpectedParameters() const override { return OneOrMoreParameters; }

  std::string Evaluate(
    std::vector<std::string> const& parameters, cm::GenEx::Evaluation* eval,
    GeneratorExpressionContent const* content,
    cmGeneratorExpressionDAGChecker* dagChecker) const override
  {
    using ForGenex = cmGeneratorExpressionDAGChecker::ForGenex;

    if (!eval->HeadTarget || !dagChecker ||
        !dagChecker->EvaluatingLinkLibraries(nullptr,
                                             ForGenex::LINK_LIBRARY)) {
      reportError(eval, content->GetOriginalExpression(),
                  "$<LINK_LIBRARY:...> may only be used with binary targets "
                  "to specify link libraries through 'LINK_LIBRARIES', "
                  "'INTERFACE_LINK_LIBRARIES', and "
                  "'INTERFACE_LINK_LIBRARIES_DIRECT' properties.");
      return std::string();
    }

    cmList list{ parameters.begin(), parameters.end() };
    if (list.empty()) {
      reportError(
        eval, content->GetOriginalExpression(),
        "$<LINK_LIBRARY:...> expects a feature name as first argument.");
      return std::string();
    }
    if (list.size() == 1) {
      // no libraries specified, ignore this genex
      return std::string();
    }

    static cmsys::RegularExpression featureNameValidator("^[A-Za-z0-9_]+$");
    auto const& feature = list.front();
    if (!featureNameValidator.find(feature)) {
      reportError(eval, content->GetOriginalExpression(),
                  cmStrCat("The feature name '", feature,
                           "' contains invalid characters."));
      return std::string();
    }

    auto const LL_BEGIN = cmStrCat("<LINK_LIBRARY:", feature, '>');
    auto const LL_END = cmStrCat("</LINK_LIBRARY:", feature, '>');

    // filter out $<LINK_LIBRARY:..> tags with same feature
    // and raise an error for any different feature
    cm::erase_if(list, [&](std::string const& item) -> bool {
      return item == LL_BEGIN || item == LL_END;
    });
    auto it =
      std::find_if(list.cbegin() + 1, list.cend(),
                   [&feature](std::string const& item) -> bool {
                     return cmHasPrefix(item, "<LINK_LIBRARY:"_s) &&
                       item.substr(14, item.find('>', 14) - 14) != feature;
                   });
    if (it != list.cend()) {
      reportError(
        eval, content->GetOriginalExpression(),
        "$<LINK_LIBRARY:...> with different features cannot be nested.");
      return std::string();
    }
    // $<LINK_GROUP:...> must not appear as part of $<LINK_LIBRARY:...>
    it = std::find_if(list.cbegin() + 1, list.cend(),
                      [](std::string const& item) -> bool {
                        return cmHasPrefix(item, "<LINK_GROUP:"_s);
                      });
    if (it != list.cend()) {
      reportError(eval, content->GetOriginalExpression(),
                  "$<LINK_GROUP:...> cannot be nested inside a "
                  "$<LINK_LIBRARY:...> expression.");
      return std::string();
    }

    list.front() = LL_BEGIN;
    list.push_back(LL_END);

    return list.to_string();
  }
} linkLibraryNode;

static const struct LinkGroupNode : public cmGeneratorExpressionNode
{
  LinkGroupNode() {} // NOLINT(modernize-use-equals-default)

  int NumExpectedParameters() const override { return OneOrMoreParameters; }

  std::string Evaluate(
    std::vector<std::string> const& parameters, cm::GenEx::Evaluation* eval,
    GeneratorExpressionContent const* content,
    cmGeneratorExpressionDAGChecker* dagChecker) const override
  {
    using ForGenex = cmGeneratorExpressionDAGChecker::ForGenex;

    if (!eval->HeadTarget || !dagChecker ||
        !dagChecker->EvaluatingLinkLibraries(nullptr, ForGenex::LINK_GROUP)) {
      reportError(
        eval, content->GetOriginalExpression(),
        "$<LINK_GROUP:...> may only be used with binary targets "
        "to specify group of link libraries through 'LINK_LIBRARIES', "
        "'INTERFACE_LINK_LIBRARIES', and "
        "'INTERFACE_LINK_LIBRARIES_DIRECT' properties.");
      return std::string();
    }

    cmList list{ parameters.begin(), parameters.end() };
    if (list.empty()) {
      reportError(
        eval, content->GetOriginalExpression(),
        "$<LINK_GROUP:...> expects a feature name as first argument.");
      return std::string();
    }
    // $<LINK_GROUP:..> cannot be nested
    if (std::find_if(list.cbegin(), list.cend(),
                     [](std::string const& item) -> bool {
                       return cmHasPrefix(item, "<LINK_GROUP"_s);
                     }) != list.cend()) {
      reportError(eval, content->GetOriginalExpression(),
                  "$<LINK_GROUP:...> cannot be nested.");
      return std::string();
    }
    if (list.size() == 1) {
      // no libraries specified, ignore this genex
      return std::string();
    }

    static cmsys::RegularExpression featureNameValidator("^[A-Za-z0-9_]+$");
    auto const& feature = list.front();
    if (!featureNameValidator.find(feature)) {
      reportError(eval, content->GetOriginalExpression(),
                  cmStrCat("The feature name '", feature,
                           "' contains invalid characters."));
      return std::string();
    }

    auto const LG_BEGIN = cmStrCat(
      "<LINK_GROUP:", feature, ':',
      cmJoin(cmRange<decltype(list.cbegin())>(list.cbegin() + 1, list.cend()),
             "|"_s),
      '>');
    auto const LG_END = cmStrCat("</LINK_GROUP:", feature, '>');

    list.front() = LG_BEGIN;
    list.push_back(LG_END);

    return list.to_string();
  }
} linkGroupNode;

static const struct HostLinkNode : public cmGeneratorExpressionNode
{
  HostLinkNode() {} // NOLINT(modernize-use-equals-default)

  int NumExpectedParameters() const override { return ZeroOrMoreParameters; }

  std::string Evaluate(
    std::vector<std::string> const& parameters, cm::GenEx::Evaluation* eval,
    GeneratorExpressionContent const* content,
    cmGeneratorExpressionDAGChecker* dagChecker) const override
  {
    if (!eval->HeadTarget || !dagChecker ||
        !dagChecker->EvaluatingLinkOptionsExpression()) {
      reportError(eval, content->GetOriginalExpression(),
                  "$<HOST_LINK:...> may only be used with binary targets "
                  "to specify link options.");
      return std::string();
    }

    return eval->HeadTarget->IsDeviceLink() ? std::string()
                                            : cmList::to_string(parameters);
  }
} hostLinkNode;

static const struct DeviceLinkNode : public cmGeneratorExpressionNode
{
  DeviceLinkNode() {} // NOLINT(modernize-use-equals-default)

  int NumExpectedParameters() const override { return ZeroOrMoreParameters; }

  std::string Evaluate(
    std::vector<std::string> const& parameters, cm::GenEx::Evaluation* eval,
    GeneratorExpressionContent const* content,
    cmGeneratorExpressionDAGChecker* dagChecker) const override
  {
    if (!eval->HeadTarget || !dagChecker ||
        !dagChecker->EvaluatingLinkOptionsExpression()) {
      reportError(eval, content->GetOriginalExpression(),
                  "$<DEVICE_LINK:...> may only be used with binary targets "
                  "to specify link options.");
      return std::string();
    }

    if (eval->HeadTarget->IsDeviceLink()) {
      cmList list{ parameters.begin(), parameters.end() };
      auto const DL_BEGIN = "<DEVICE_LINK>"_s;
      auto const DL_END = "</DEVICE_LINK>"_s;
      cm::erase_if(list, [&](std::string const& item) {
        return item == DL_BEGIN || item == DL_END;
      });

      list.insert(list.begin(), static_cast<std::string>(DL_BEGIN));
      list.push_back(static_cast<std::string>(DL_END));

      return list.to_string();
    }

    return std::string();
  }
} deviceLinkNode;

namespace {
bool GetFileSet(std::vector<std::string> const& parameters,
                cm::GenEx::Evaluation* eval,
                GeneratorExpressionContent const* content, cmFileSet*& fileSet)
{
  auto const& fileSetName = parameters[0];
  auto targetName = parameters[1];
  auto* makefile = eval->Context.LG->GetMakefile();
  fileSet = nullptr;

  auto const TARGET = "TARGET:"_s;

  if (cmHasPrefix(targetName, TARGET)) {
    targetName = targetName.substr(TARGET.length());
    if (targetName.empty()) {
      reportError(eval, content->GetOriginalExpression(),
                  cmStrCat("No value provided for the ", TARGET, " option."));
      return false;
    }
    auto* target = makefile->FindTargetToUse(targetName);
    if (!target) {
      reportError(eval, content->GetOriginalExpression(),
                  cmStrCat("Non-existent target: ", targetName));
      return false;
    }
    fileSet = target->GetFileSet(fileSetName);
  } else {
    reportError(eval, content->GetOriginalExpression(),
                cmStrCat("Invalid option. ", TARGET, " expected."));
    return false;
  }
  return true;
}
}

static const struct FileSetExistsNode : public cmGeneratorExpressionNode
{
  FileSetExistsNode() {} // NOLINT(modernize-use-equals-default)

  // This node handles errors on parameter count itself.
  int NumExpectedParameters() const override { return 2; }

  std::string Evaluate(
    std::vector<std::string> const& parameters, cm::GenEx::Evaluation* eval,
    GeneratorExpressionContent const* content,
    cmGeneratorExpressionDAGChecker* /*dagCheckerParent*/) const override
  {
    if (parameters[0].empty()) {
      reportError(
        eval, content->GetOriginalExpression(),
        "$<FILE_SET_EXISTS:fileset,TARGET:tgt> expression requires a "
        "non-empty FILE_SET name.");
      return std::string{};
    }

    cmFileSet* fileSet = nullptr;
    if (!GetFileSet(parameters, eval, content, fileSet)) {
      return std::string{};
    }

    return fileSet ? "1" : "0";
  }
} fileSetExistsNode;

static const struct FileSetPropertyNode : public cmGeneratorExpressionNode
{
  FileSetPropertyNode() {} // NOLINT(modernize-use-equals-default)

  // This node handles errors on parameter count itself.
  int NumExpectedParameters() const override { return 3; }

  std::string Evaluate(
    std::vector<std::string> const& parameters, cm::GenEx::Evaluation* eval,
    GeneratorExpressionContent const* content,
    cmGeneratorExpressionDAGChecker* /*dagCheckerParent*/) const override
  {
    static cmsys::RegularExpression propertyNameValidator("^[A-Za-z0-9_]+$");

    std::string const& fileSetName = parameters.front();
    std::string const& propertyName = parameters.back();

    if (fileSetName.empty() && propertyName.empty()) {
      reportError(eval, content->GetOriginalExpression(),
                  "$<FILE_SET_PROPERTY:fileset,TARGET:tgt,prop> expression "
                  "requires a non-empty FILE_SET name and property name.");
      return std::string{};
    }
    if (fileSetName.empty()) {
      reportError(
        eval, content->GetOriginalExpression(),
        "$<FILE_SET_PROPERTY:fileset,TARGET:tgt,prop> expression requires a "
        "non-empty FILE_SET name.");
      return std::string{};
    }
    if (propertyName.empty()) {
      reportError(
        eval, content->GetOriginalExpression(),
        "$<FILE_SET_PROPERTY:fileset,TARGET:tgt,prop> expression requires a "
        "non-empty property name.");
      return std::string{};
    }
    if (!propertyNameValidator.find(propertyName)) {
      reportError(eval, content->GetOriginalExpression(),
                  "Property name not supported.");
      return std::string{};
    }

    cmFileSet* fileSet = nullptr;
    if (!GetFileSet(parameters, eval, content, fileSet)) {
      return std::string{};
    }
    if (!fileSet) {
      reportError(
        eval, content->GetOriginalExpression(),
        cmStrCat("FILE_SET \"", fileSetName, "\" is not known from CMake."));
      return std::string{};
    }

    return fileSet->GetProperty(propertyName);
  }
} fileSetPropertyNode;

namespace {
bool GetSourceFile(
  cmRange<std::vector<std::string>::const_iterator> parameters,
  cm::GenEx::Evaluation* eval, GeneratorExpressionContent const* content,
  cmSourceFile*& sourceFile)
{
  auto sourceName = *parameters.begin();
  auto* makefile = eval->Context.LG->GetMakefile();
  sourceFile = nullptr;

  if (parameters.size() == 2) {
    auto const& option = *parameters.advance(1).begin();
    auto const DIRECTORY = "DIRECTORY:"_s;
    auto const TARGET_DIRECTORY = "TARGET_DIRECTORY:"_s;
    if (cmHasPrefix(option, DIRECTORY)) {
      auto dir = option.substr(DIRECTORY.length());
      if (dir.empty()) {
        reportError(
          eval, content->GetOriginalExpression(),
          cmStrCat("No value provided for the ", DIRECTORY, " option."));
        return false;
      }
      dir = cmSystemTools::CollapseFullPath(
        dir, makefile->GetCurrentSourceDirectory());
      makefile = makefile->GetGlobalGenerator()->FindMakefile(dir);
      if (!makefile) {
        reportError(
          eval, content->GetOriginalExpression(),
          cmStrCat("Directory \"", dir, "\" is not known from CMake."));
        return false;
      }
    } else if (cmHasPrefix(option, TARGET_DIRECTORY)) {
      auto targetName = option.substr(TARGET_DIRECTORY.length());
      if (targetName.empty()) {
        reportError(eval, content->GetOriginalExpression(),
                    cmStrCat("No value provided for the ", TARGET_DIRECTORY,
                             " option."));
        return false;
      }
      auto* target = makefile->FindTargetToUse(targetName);
      if (!target) {
        reportError(eval, content->GetOriginalExpression(),
                    cmStrCat("Non-existent target: ", targetName));
        return false;
      }
      makefile = makefile->GetGlobalGenerator()->FindMakefile(
        target->GetProperty("BINARY_DIR"));
    } else {
      reportError(eval, content->GetOriginalExpression(),
                  cmStrCat("Invalid option. ", DIRECTORY, " or ",
                           TARGET_DIRECTORY, " expected."));
      return false;
    }

    sourceName = cmSystemTools::CollapseFullPath(
      sourceName,
      eval->Context.LG->GetMakefile()->GetCurrentSourceDirectory());
  }

  sourceFile = makefile->GetSource(sourceName);
  return true;
}
}

static const struct SourceExistsNode : public cmGeneratorExpressionNode
{
  SourceExistsNode() {} // NOLINT(modernize-use-equals-default)

  // This node handles errors on parameter count itself.
  int NumExpectedParameters() const override { return OneOrMoreParameters; }

  std::string Evaluate(
    std::vector<std::string> const& parameters, cm::GenEx::Evaluation* eval,
    GeneratorExpressionContent const* content,
    cmGeneratorExpressionDAGChecker* /*dagCheckerParent*/) const override
  {
    if (parameters.size() > 2) {
      reportError(eval, content->GetOriginalExpression(),
                  "$<SOURCE_EXISTS:...> expression requires at most two "
                  "parameters.");
      return std::string{};
    }

    if (parameters[0].empty()) {
      reportError(eval, content->GetOriginalExpression(),
                  "$<SOURCE_EXISTS:src> expression requires a "
                  "non-empty source name.");
      return std::string{};
    }

    cmSourceFile* sourceFile = nullptr;
    if (!GetSourceFile(cmMakeRange(parameters), eval, content, sourceFile)) {
      return std::string{};
    }

    return sourceFile ? "1" : "0";
  }
} sourceExistsNode;

static const struct SourcePropertyNode : public cmGeneratorExpressionNode
{
  SourcePropertyNode() {} // NOLINT(modernize-use-equals-default)

  // This node handles errors on parameter count itself.
  int NumExpectedParameters() const override { return TwoOrMoreParameters; }

  std::string Evaluate(
    std::vector<std::string> const& parameters, cm::GenEx::Evaluation* eval,
    GeneratorExpressionContent const* content,
    cmGeneratorExpressionDAGChecker* /*dagCheckerParent*/) const override
  {
    static cmsys::RegularExpression propertyNameValidator("^[A-Za-z0-9_]+$");

    if (parameters.size() > 3) {
      reportError(eval, content->GetOriginalExpression(),
                  "$<SOURCE_PROPERTY:...> expression requires at most three "
                  "parameters.");
      return std::string{};
    }

    std::string sourceName = parameters.front();
    std::string const& propertyName = parameters.back();

    if (sourceName.empty() && propertyName.empty()) {
      reportError(eval, content->GetOriginalExpression(),
                  "$<SOURCE_PROPERTY:src,prop> expression requires a "
                  "non-empty source name and property name.");
      return std::string{};
    }
    if (sourceName.empty()) {
      reportError(eval, content->GetOriginalExpression(),
                  "$<SOURCE_PROPERTY:src,prop> expression requires a "
                  "non-empty source name.");
      return std::string{};
    }
    if (propertyName.empty()) {
      reportError(eval, content->GetOriginalExpression(),
                  "$<SOURCE_PROPERTY:src,prop> expression requires a "
                  "non-empty property name.");
      return std::string{};
    }
    if (!propertyNameValidator.find(propertyName)) {
      reportError(eval, content->GetOriginalExpression(),
                  "Property name not supported.");
      return std::string{};
    }

    cmSourceFile* sourceFile = nullptr;
    if (!GetSourceFile(cmMakeRange(parameters).retreat(1), eval, content,
                       sourceFile)) {
      return std::string{};
    }
    if (!sourceFile) {
      reportError(
        eval, content->GetOriginalExpression(),
        cmStrCat("Source file \"", sourceName, "\" is not known from CMake."));
      return std::string{};
    }

    return sourceFile->GetPropertyForUser(propertyName);
  }
} sourcePropertyNode;

static std::string getLinkedTargetsContent(
  cmGeneratorTarget const* target, std::string const& prop,
  cm::GenEx::Evaluation* eval, cmGeneratorExpressionDAGChecker* dagChecker,
  cmGeneratorTarget::UseTo usage)
{
  std::string result;
  if (cmLinkImplementationLibraries const* impl =
        target->GetLinkImplementationLibraries(
          eval->Context.Config, cmGeneratorTarget::UseTo::Compile)) {
    for (cmLinkItem const& lib : impl->Libraries) {
      if (lib.Target) {
        // Pretend $<TARGET_PROPERTY:lib.Target,prop> appeared in our
        // caller's property and hand-evaluate it as if it were compiled.
        // Create a context as cmCompiledGeneratorExpression::Evaluate does.
        cm::GenEx::Context libContext(eval->Context);
        // FIXME: Why have we long used the target's local generator
        // instead of that of the evaluation context?
        libContext.LG = target->GetLocalGenerator();
        cm::GenEx::Evaluation libEval(
          std::move(libContext), eval->Quiet, target, target,
          eval->EvaluateForBuildsystem, lib.Backtrace);
        std::string libResult = lib.Target->EvaluateInterfaceProperty(
          prop, &libEval, dagChecker, usage);
        if (!libResult.empty()) {
          if (result.empty()) {
            result = std::move(libResult);
          } else {
            result.reserve(result.size() + 1 + libResult.size());
            result += ";";
            result += libResult;
          }
        }
      }
    }
  }
  return result;
}

static const struct TargetPropertyNode : public cmGeneratorExpressionNode
{
  TargetPropertyNode() {} // NOLINT(modernize-use-equals-default)

  // This node handles errors on parameter count itself.
  int NumExpectedParameters() const override { return OneOrMoreParameters; }

  static char const* GetErrorText(std::string const& targetName,
                                  std::string const& propertyName)
  {
    static cmsys::RegularExpression propertyNameValidator("^[A-Za-z0-9_]+$");
    if (targetName.empty() && propertyName.empty()) {
      return "$<TARGET_PROPERTY:tgt,prop> expression requires a non-empty "
             "target name and property name.";
    }
    if (targetName.empty()) {
      return "$<TARGET_PROPERTY:tgt,prop> expression requires a non-empty "
             "target name.";
    }
    if (!cmGeneratorExpression::IsValidTargetName(targetName)) {
      if (!propertyNameValidator.find(propertyName)) {
        return "Target name and property name not supported.";
      }
      return "Target name not supported.";
    }
    return nullptr;
  }

  std::string Evaluate(
    std::vector<std::string> const& parameters, cm::GenEx::Evaluation* eval,
    GeneratorExpressionContent const* content,
    cmGeneratorExpressionDAGChecker* dagCheckerParent) const override
  {
    static cmsys::RegularExpression propertyNameValidator("^[A-Za-z0-9_]+$");

    cmGeneratorTarget const* target = nullptr;
    std::string targetName;
    std::string propertyName;

    if (parameters.size() == 2) {
      targetName = parameters[0];
      propertyName = parameters[1];

      if (char const* e = GetErrorText(targetName, propertyName)) {
        reportError(eval, content->GetOriginalExpression(), e);
        return std::string();
      }
      if (propertyName == "ALIASED_TARGET"_s) {
        if (eval->Context.LG->GetMakefile()->IsAlias(targetName)) {
          if (cmGeneratorTarget* tgt =
                eval->Context.LG->FindGeneratorTargetToUse(targetName)) {
            return tgt->GetName();
          }
        }
        return std::string();
      }
      if (propertyName == "ALIAS_GLOBAL"_s) {
        if (eval->Context.LG->GetMakefile()->IsAlias(targetName)) {
          return eval->Context.LG->GetGlobalGenerator()->IsAlias(targetName)
            ? "TRUE"
            : "FALSE";
        }
        return std::string();
      }
      cmLocalGenerator const* lg = eval->CurrentTarget
        ? eval->CurrentTarget->GetLocalGenerator()
        : eval->Context.LG;
      target = lg->FindGeneratorTargetToUse(targetName);

      if (!target) {
        std::ostringstream e;
        e << "Target \"" << targetName << "\" not found.";
        reportError(eval, content->GetOriginalExpression(), e.str());
        return std::string();
      }
      eval->AllTargets.insert(target);

    } else if (parameters.size() == 1) {
      target = eval->HeadTarget;
      propertyName = parameters[0];

      // Keep track of the properties seen while processing.
      // The evaluation of the LINK_LIBRARIES generator expressions
      // will check this to ensure that properties have one consistent
      // value for all evaluations.
      eval->SeenTargetProperties.insert(propertyName);

      eval->HadHeadSensitiveCondition = true;
      if (!target) {
        reportError(
          eval, content->GetOriginalExpression(),
          "$<TARGET_PROPERTY:prop>  may only be used with binary targets.  "
          "It may not be used with add_custom_command or add_custom_target. "
          " "
          " "
          "Specify the target to read a property from using the "
          "$<TARGET_PROPERTY:tgt,prop> signature instead.");
        return std::string();
      }
    } else {
      reportError(
        eval, content->GetOriginalExpression(),
        "$<TARGET_PROPERTY:...> expression requires one or two parameters");
      return std::string();
    }

    if (propertyName == "SOURCES") {
      eval->SourceSensitiveTargets.insert(target);
    }

    if (propertyName.empty()) {
      reportError(
        eval, content->GetOriginalExpression(),
        "$<TARGET_PROPERTY:...> expression requires a non-empty property "
        "name.");
      return std::string();
    }

    if (!propertyNameValidator.find(propertyName)) {
      ::reportError(eval, content->GetOriginalExpression(),
                    "Property name not supported.");
      return std::string();
    }

    assert(target);

    if (propertyName == "LINKER_LANGUAGE") {
      if (target->LinkLanguagePropagatesToDependents() && dagCheckerParent &&
          (dagCheckerParent->EvaluatingLinkLibraries() ||
           dagCheckerParent->EvaluatingSources())) {
        reportError(
          eval, content->GetOriginalExpression(),
          "LINKER_LANGUAGE target property can not be used while evaluating "
          "link libraries for a static library");
        return std::string();
      }
      return target->GetLinkerLanguage(eval->Context.Config);
    }

    bool const evaluatingLinkLibraries =
      dagCheckerParent && dagCheckerParent->EvaluatingLinkLibraries();

    std::string interfacePropertyName;
    bool isInterfaceProperty = false;
    cmGeneratorTarget::UseTo usage = cmGeneratorTarget::UseTo::Compile;

    if (cm::optional<cmGeneratorTarget::TransitiveProperty> transitiveProp =
          target->IsTransitiveProperty(propertyName, eval->Context,
                                       dagCheckerParent)) {
      interfacePropertyName = std::string(transitiveProp->InterfaceName);
      isInterfaceProperty = transitiveProp->InterfaceName == propertyName;
      usage = transitiveProp->Usage;
    }

    if (dagCheckerParent) {
      // This $<TARGET_PROPERTY:...> node has been reached while evaluating
      // another target property value.  Check that the outermost evaluation
      // expects such nested evaluations.
      if (dagCheckerParent->EvaluatingGenexExpression() ||
          dagCheckerParent->EvaluatingPICExpression() ||
          dagCheckerParent->EvaluatingLinkerLauncher()) {
        // No check required.
      } else if (evaluatingLinkLibraries) {
        if (!interfacePropertyName.empty() &&
            interfacePropertyName != "INTERFACE_LINK_LIBRARIES"_s) {
          reportError(
            eval, content->GetOriginalExpression(),
            "$<TARGET_PROPERTY:...> expression in link libraries "
            "evaluation depends on target property which is transitive "
            "over the link libraries, creating a recursion.");
          return std::string();
        }
      } else {
        assert(dagCheckerParent->EvaluatingTransitiveProperty());
      }
    }

    if (isInterfaceProperty) {
      return cmGeneratorExpression::StripEmptyListElements(
        target->EvaluateInterfaceProperty(propertyName, eval, dagCheckerParent,
                                          usage));
    }

    cmGeneratorExpressionDAGChecker dagChecker{
      target,           propertyName,  content,
      dagCheckerParent, eval->Context, eval->Backtrace,
    };

    switch (dagChecker.Check()) {
      case cmGeneratorExpressionDAGChecker::SELF_REFERENCE:
        dagChecker.ReportError(eval, content->GetOriginalExpression());
        return std::string();
      case cmGeneratorExpressionDAGChecker::CYCLIC_REFERENCE:
        // No error. We just skip cyclic references.
        return std::string();
      case cmGeneratorExpressionDAGChecker::ALREADY_SEEN:
        // We handle transitive properties above.  For non-transitive
        // properties we accept repeats anyway.
      case cmGeneratorExpressionDAGChecker::DAG:
        break;
    }

    std::string result;
    bool haveProp = false;
    if (cmValue p = target->GetProperty(propertyName)) {
      result = *p;
      haveProp = true;
    } else if (evaluatingLinkLibraries) {
      return std::string();
    }

    // Properties named by COMPATIBLE_INTERFACE_ properties combine over
    // the transitive link closure as a single order-independent value.
    // Imported targets do not themselves have a defined value for these
    // properties, but they can contribute to the value of a non-imported
    // dependent.
    //
    // For COMPATIBLE_INTERFACE_{BOOL,STRING}:
    // * If set on this target, use the value directly.  It is checked
    //   elsewhere for consistency over the transitive link closure.
    // * If not set on this target, compute the value from the closure.
    //
    // For COMPATIBLE_INTERFACE_NUMBER_{MAX,MIN} we always compute the value
    // from this target and the transitive link closure to get the max or min.
    if (!haveProp && !target->IsImported()) {
      if (target->IsLinkInterfaceDependentBoolProperty(propertyName,
                                                       eval->Context.Config)) {
        eval->HadContextSensitiveCondition = true;
        return target->GetLinkInterfaceDependentBoolProperty(
                 propertyName, eval->Context.Config)
          ? "1"
          : "0";
      }
      if (target->IsLinkInterfaceDependentStringProperty(
            propertyName, eval->Context.Config)) {
        eval->HadContextSensitiveCondition = true;
        char const* propContent =
          target->GetLinkInterfaceDependentStringProperty(
            propertyName, eval->Context.Config);
        return propContent ? propContent : "";
      }
    }
    if (!evaluatingLinkLibraries && !target->IsImported()) {
      if (target->IsLinkInterfaceDependentNumberMinProperty(
            propertyName, eval->Context.Config)) {
        eval->HadContextSensitiveCondition = true;
        char const* propContent =
          target->GetLinkInterfaceDependentNumberMinProperty(
            propertyName, eval->Context.Config);
        return propContent ? propContent : "";
      }
      if (target->IsLinkInterfaceDependentNumberMaxProperty(
            propertyName, eval->Context.Config)) {
        eval->HadContextSensitiveCondition = true;
        char const* propContent =
          target->GetLinkInterfaceDependentNumberMaxProperty(
            propertyName, eval->Context.Config);
        return propContent ? propContent : "";
      }
    }

    // Some properties, such as usage requirements, combine over the
    // transitive link closure as an ordered list.
    if (!interfacePropertyName.empty()) {
      result = cmGeneratorExpression::StripEmptyListElements(
        this->EvaluateDependentExpression(result, eval, target, &dagChecker,
                                          target));
      std::string linkedTargetsContent = getLinkedTargetsContent(
        target, interfacePropertyName, eval, &dagChecker, usage);
      if (!linkedTargetsContent.empty()) {
        result += (result.empty() ? "" : ";") + linkedTargetsContent;
      }
    }
    return result;
  }
} targetPropertyNode;

static const struct targetIntermediateDirNode
  : public cmGeneratorExpressionNode
{
  targetIntermediateDirNode() {} // NOLINT(modernize-use-equals-default)

  static char const* GetErrorText(std::string const& targetName)
  {
    static cmsys::RegularExpression propertyNameValidator("^[A-Za-z0-9_]+$");
    if (targetName.empty()) {
      return "$<TARGET_INTERMEDIATE_DIR:tgt> expression requires a non-empty "
             "target name.";
    }
    if (!cmGeneratorExpression::IsValidTargetName(targetName)) {
      return "Target name not supported.";
    }
    return nullptr;
  }

  std::string Evaluate(
    std::vector<std::string> const& parameters, cm::GenEx::Evaluation* eval,
    GeneratorExpressionContent const* content,
    cmGeneratorExpressionDAGChecker* /*dagChecker*/) const override
  {
    cmGeneratorTarget const* target = nullptr;
    std::string targetName;

    if (parameters.size() == 1) {
      targetName = parameters[0];

      if (char const* e = GetErrorText(targetName)) {
        reportError(eval, content->GetOriginalExpression(), e);
        return std::string();
      }
      cmLocalGenerator const* lg = eval->CurrentTarget
        ? eval->CurrentTarget->GetLocalGenerator()
        : eval->Context.LG;
      target = lg->FindGeneratorTargetToUse(targetName);

      if (!target) {
        std::ostringstream e;
        e << "Target \"" << targetName << "\" not found.";
        reportError(eval, content->GetOriginalExpression(), e.str());
        return std::string();
      }
      eval->AllTargets.insert(target);

    } else {
      reportError(
        eval, content->GetOriginalExpression(),
        "$<TARGET_INTERMEDIATE_DIR:...> expression requires one parameter");
      return std::string();
    }

    assert(target);

    if (!HasKnownObjectFileLocation(eval, content, "TARGET_INTERMEDIATE_DIR",
                                    target)) {
      return std::string();
    }

    return cmSystemTools::CollapseFullPath(
      target->GetObjectDirectory(eval->Context.Config));
  }
} targetIntermediateDirNode;

static const struct TargetNameNode : public cmGeneratorExpressionNode
{
  TargetNameNode() {} // NOLINT(modernize-use-equals-default)

  bool GeneratesContent() const override { return true; }

  bool AcceptsArbitraryContentParameter() const override { return true; }
  bool RequiresLiteralInput() const override { return true; }

  std::string Evaluate(
    std::vector<std::string> const& parameters,
    cm::GenEx::Evaluation* /*eval*/,
    GeneratorExpressionContent const* /*content*/,
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
    std::vector<std::string> const& parameters, cm::GenEx::Evaluation* eval,
    GeneratorExpressionContent const* content,
    cmGeneratorExpressionDAGChecker* /*dagChecker*/) const override
  {
    std::string const& tgtName = parameters.front();
    cmGeneratorTarget* gt =
      eval->Context.LG->FindGeneratorTargetToUse(tgtName);
    if (!gt) {
      std::ostringstream e;
      e << "Objects of target \"" << tgtName
        << "\" referenced but no such target exists.";
      reportError(eval, content->GetOriginalExpression(), e.str());
      return std::string();
    }
    cmStateEnums::TargetType type = gt->GetType();
    if (type != cmStateEnums::EXECUTABLE &&
        type != cmStateEnums::STATIC_LIBRARY &&
        type != cmStateEnums::SHARED_LIBRARY &&
        type != cmStateEnums::MODULE_LIBRARY &&
        type != cmStateEnums::OBJECT_LIBRARY) {
      std::ostringstream e;
      e << "Objects of target \"" << tgtName
        << "\" referenced but is not one of the allowed target types "
        << "(EXECUTABLE, STATIC, SHARED, MODULE, OBJECT).";
      reportError(eval, content->GetOriginalExpression(), e.str());
      return std::string();
    }
    cmGlobalGenerator const* gg = eval->Context.LG->GetGlobalGenerator();
    if (!HasKnownObjectFileLocation(eval, content, "TARGET_OBJECTS", gt)) {
      return std::string();
    }

    cmList objects;

    if (gt->IsImported()) {
      cmValue loc = nullptr;
      cmValue imp = nullptr;
      std::string suffix;
      if (gt->Target->GetMappedConfig(eval->Context.Config, loc, imp,
                                      suffix)) {
        objects.assign(*loc);
      }
      eval->HadContextSensitiveCondition = true;
    } else {
      gt->GetTargetObjectNames(eval->Context.Config, objects);

      std::string obj_dir;
      if (eval->EvaluateForBuildsystem && !gg->SupportsCrossConfigs()) {
        // Use object file directory with buildsystem placeholder.
        obj_dir = gt->ObjectDirectory;
        eval->HadContextSensitiveCondition = gt->HasContextDependentSources();
      } else {
        // Use object file directory with per-config location.
        obj_dir = gt->GetObjectDirectory(eval->Context.Config);
        eval->HadContextSensitiveCondition = true;
      }

      for (auto& o : objects) {
        o = cmStrCat(obj_dir, o);
      }
    }

    // Create the cmSourceFile instances in the referencing directory.
    cmMakefile* mf = eval->Context.LG->GetMakefile();
    for (std::string const& o : objects) {
      mf->AddTargetObject(tgtName, o);
    }

    return objects.to_string();
  }
} targetObjectsNode;

struct TargetRuntimeDllsBaseNode : public cmGeneratorExpressionNode
{
  std::vector<std::string> CollectDlls(
    std::vector<std::string> const& parameters, cm::GenEx::Evaluation* eval,
    GeneratorExpressionContent const* content) const
  {
    std::string const& tgtName = parameters.front();
    cmGeneratorTarget* gt =
      eval->Context.LG->FindGeneratorTargetToUse(tgtName);
    if (!gt) {
      std::ostringstream e;
      e << "Objects of target \"" << tgtName
        << "\" referenced but no such target exists.";
      reportError(eval, content->GetOriginalExpression(), e.str());
      return std::vector<std::string>();
    }
    cmStateEnums::TargetType type = gt->GetType();
    if (type != cmStateEnums::EXECUTABLE &&
        type != cmStateEnums::SHARED_LIBRARY &&
        type != cmStateEnums::MODULE_LIBRARY) {
      std::ostringstream e;
      e << "Objects of target \"" << tgtName
        << "\" referenced but is not one of the allowed target types "
        << "(EXECUTABLE, SHARED, MODULE).";
      reportError(eval, content->GetOriginalExpression(), e.str());
      return std::vector<std::string>();
    }

    if (auto* cli = gt->GetLinkInformation(eval->Context.Config)) {
      std::vector<std::string> dllPaths;
      auto const& dlls = cli->GetRuntimeDLLs();

      for (auto const& dll : dlls) {
        if (auto loc = dll->MaybeGetLocation(eval->Context.Config)) {
          dllPaths.emplace_back(*loc);
        }
      }

      return dllPaths;
    }

    return std::vector<std::string>();
  }
};

static const struct TargetRuntimeDllsNode : public TargetRuntimeDllsBaseNode
{
  TargetRuntimeDllsNode() {} // NOLINT(modernize-use-equals-default)

  std::string Evaluate(
    std::vector<std::string> const& parameters, cm::GenEx::Evaluation* eval,
    GeneratorExpressionContent const* content,
    cmGeneratorExpressionDAGChecker* /*dagChecker*/) const override
  {
    std::vector<std::string> dlls = CollectDlls(parameters, eval, content);
    return cmList::to_string(dlls);
  }
} targetRuntimeDllsNode;

static const struct TargetRuntimeDllDirsNode : public TargetRuntimeDllsBaseNode
{
  TargetRuntimeDllDirsNode() {} // NOLINT(modernize-use-equals-default)

  std::string Evaluate(
    std::vector<std::string> const& parameters, cm::GenEx::Evaluation* eval,
    GeneratorExpressionContent const* content,
    cmGeneratorExpressionDAGChecker* /*dagChecker*/) const override
  {
    std::vector<std::string> dlls = CollectDlls(parameters, eval, content);
    std::vector<std::string> dllDirs;
    for (std::string const& dll : dlls) {
      std::string directory = cmSystemTools::GetFilenamePath(dll);
      if (std::find(dllDirs.begin(), dllDirs.end(), directory) ==
          dllDirs.end()) {
        dllDirs.push_back(directory);
      }
    }
    return cmList::to_string(dllDirs);
  }
} targetRuntimeDllDirsNode;

static const struct CompileFeaturesNode : public cmGeneratorExpressionNode
{
  CompileFeaturesNode() {} // NOLINT(modernize-use-equals-default)

  int NumExpectedParameters() const override { return OneOrMoreParameters; }

  std::string Evaluate(
    std::vector<std::string> const& parameters, cm::GenEx::Evaluation* eval,
    GeneratorExpressionContent const* content,
    cmGeneratorExpressionDAGChecker* dagChecker) const override
  {
    cmGeneratorTarget const* target = eval->HeadTarget;
    if (!target) {
      reportError(
        eval, content->GetOriginalExpression(),
        "$<COMPILE_FEATURE> may only be used with binary targets.  It may "
        "not be used with add_custom_command or add_custom_target.");
      return std::string();
    }
    eval->HadHeadSensitiveCondition = true;

    using LangMap = std::map<std::string, cmList>;
    static LangMap availableFeatures;

    LangMap testedFeatures;
    cmStandardLevelResolver standardResolver(eval->Context.LG->GetMakefile());
    for (std::string const& p : parameters) {
      std::string error;
      std::string lang;
      if (!standardResolver.CompileFeatureKnown(
            eval->HeadTarget->Target->GetName(), p, lang, &error)) {
        reportError(eval, content->GetOriginalExpression(), error);
        return std::string();
      }
      testedFeatures[lang].push_back(p);

      if (availableFeatures.find(lang) == availableFeatures.end()) {
        cmValue featuresKnown =
          standardResolver.CompileFeaturesAvailable(lang, &error);
        if (!featuresKnown) {
          reportError(eval, content->GetOriginalExpression(), error);
          return std::string();
        }
        availableFeatures[lang].assign(featuresKnown);
      }
    }

    bool evalLL = dagChecker && dagChecker->EvaluatingLinkLibraries();

    for (auto const& lit : testedFeatures) {
      std::vector<std::string> const& langAvailable =
        availableFeatures[lit.first];
      cmValue standardDefault = eval->Context.LG->GetMakefile()->GetDefinition(
        cmStrCat("CMAKE_", lit.first, "_STANDARD_DEFAULT"));
      for (std::string const& it : lit.second) {
        if (!cm::contains(langAvailable, it)) {
          return "0";
        }
        if (standardDefault && standardDefault->empty()) {
          // This compiler has no notion of language standard levels.
          // All features known for the language are always available.
          continue;
        }
        if (!standardResolver.HaveStandardAvailable(
              target, lit.first, eval->Context.Config, it)) {
          if (evalLL) {
            cmValue l =
              target->GetLanguageStandard(lit.first, eval->Context.Config);
            if (!l) {
              l = standardDefault;
            }
            assert(l);
            eval->MaxLanguageStandard[target][lit.first] = *l;
          } else {
            return "0";
          }
        }
      }
    }
    return "1";
  }
} compileFeaturesNode;

static char const* targetPolicyWhitelist[] = {
  nullptr
#define TARGET_POLICY_STRING(POLICY) , #POLICY

  CM_FOR_EACH_TARGET_POLICY(TARGET_POLICY_STRING)

#undef TARGET_POLICY_STRING
};

static cmPolicies::PolicyStatus statusForTarget(cmGeneratorTarget const* tgt,
                                                char const* policy)
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

static cmPolicies::PolicyID policyForString(char const* policy_id)
{
#define RETURN_POLICY_ID(POLICY_ID)                                           \
  if (strcmp(policy_id, #POLICY_ID) == 0) {                                   \
    return cmPolicies::POLICY_ID;                                             \
  }

  CM_FOR_EACH_TARGET_POLICY(RETURN_POLICY_ID)

#undef RETURN_POLICY_ID

  assert(false && "Unreachable code. Not a valid policy");
  return cmPolicies::CMPCOUNT;
}

static const struct TargetPolicyNode : public cmGeneratorExpressionNode
{
  TargetPolicyNode() {} // NOLINT(modernize-use-equals-default)

  int NumExpectedParameters() const override { return 1; }

  std::string Evaluate(
    std::vector<std::string> const& parameters, cm::GenEx::Evaluation* eval,
    GeneratorExpressionContent const* content,
    cmGeneratorExpressionDAGChecker* /*dagChecker*/) const override
  {
    if (!eval->HeadTarget) {
      reportError(
        eval, content->GetOriginalExpression(),
        "$<TARGET_POLICY:prop> may only be used with binary targets.  It "
        "may not be used with add_custom_command or add_custom_target.");
      return std::string();
    }

    eval->HadContextSensitiveCondition = true;
    eval->HadHeadSensitiveCondition = true;

    for (size_t i = 1; i < cm::size(targetPolicyWhitelist); ++i) {
      char const* policy = targetPolicyWhitelist[i];
      if (parameters.front() == policy) {
        cmLocalGenerator* lg = eval->HeadTarget->GetLocalGenerator();
        switch (statusForTarget(eval->HeadTarget, policy)) {
          case cmPolicies::WARN:
            lg->IssueMessage(
              MessageType::AUTHOR_WARNING,
              cmPolicies::GetPolicyWarning(policyForString(policy)));
            CM_FALLTHROUGH;
          case cmPolicies::OLD:
            return "0";
          case cmPolicies::NEW:
            return "1";
        }
      }
    }
    reportError(
      eval, content->GetOriginalExpression(),
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
    std::vector<std::string> const& /*parameters*/,
    cm::GenEx::Evaluation* eval, GeneratorExpressionContent const* content,
    cmGeneratorExpressionDAGChecker* /*dagChecker*/) const override
  {
    reportError(eval, content->GetOriginalExpression(),
                "INSTALL_PREFIX is a marker for install(EXPORT) only.  It "
                "should never be evaluated.");
    return std::string();
  }

} installPrefixNode;

class ArtifactDirTag;
class ArtifactLinkerTag;
class ArtifactLinkerLibraryTag;
class ArtifactLinkerImportTag;
class ArtifactNameTag;
class ArtifactImportTag;
class ArtifactPathTag;
class ArtifactPdbTag;
class ArtifactSonameTag;
class ArtifactSonameImportTag;
class ArtifactBundleDirTag;
class ArtifactBundleDirNameTag;
class ArtifactBundleContentDirTag;

template <typename ArtifactT, typename ComponentT>
struct TargetFilesystemArtifactDependency
{
  static void AddDependency(cmGeneratorTarget* target,
                            cm::GenEx::Evaluation* eval)
  {
    eval->DependTargets.insert(target);
    eval->AllTargets.insert(target);
  }
};

struct TargetFilesystemArtifactDependencyCMP0112
{
  static void AddDependency(cmGeneratorTarget* target,
                            cm::GenEx::Evaluation* eval)
  {
    eval->AllTargets.insert(target);
    cmLocalGenerator const* lg = eval->Context.LG;
    switch (target->GetPolicyStatusCMP0112()) {
      case cmPolicies::WARN:
        if (lg->GetMakefile()->PolicyOptionalWarningEnabled(
              "CMAKE_POLICY_WARNING_CMP0112")) {
          std::string err =
            cmStrCat(cmPolicies::GetPolicyWarning(cmPolicies::CMP0112),
                     "\nDependency being added to target:\n  \"",
                     target->GetName(), "\"\n");
          lg->GetCMakeInstance()->IssueMessage(MessageType ::AUTHOR_WARNING,
                                               err, eval->Backtrace);
        }
        CM_FALLTHROUGH;
      case cmPolicies::OLD:
        eval->DependTargets.insert(target);
        break;
      case cmPolicies::NEW:
        break;
    }
  }
};

template <typename ArtifactT>
struct TargetFilesystemArtifactDependency<ArtifactT, ArtifactNameTag>
  : TargetFilesystemArtifactDependencyCMP0112
{
};
template <typename ArtifactT>
struct TargetFilesystemArtifactDependency<ArtifactT, ArtifactDirTag>
  : TargetFilesystemArtifactDependencyCMP0112
{
};
template <>
struct TargetFilesystemArtifactDependency<ArtifactBundleDirTag,
                                          ArtifactPathTag>
  : TargetFilesystemArtifactDependencyCMP0112
{
};
template <>
struct TargetFilesystemArtifactDependency<ArtifactBundleDirNameTag,
                                          ArtifactPathTag>
  : TargetFilesystemArtifactDependencyCMP0112
{
};
template <>
struct TargetFilesystemArtifactDependency<ArtifactBundleContentDirTag,
                                          ArtifactPathTag>
  : TargetFilesystemArtifactDependencyCMP0112
{
};

template <typename ArtifactT>
struct TargetFilesystemArtifactResultCreator
{
  static std::string Create(cmGeneratorTarget* target,
                            cm::GenEx::Evaluation* eval,
                            GeneratorExpressionContent const* content);
};

template <>
struct TargetFilesystemArtifactResultCreator<ArtifactSonameTag>
{
  static std::string Create(cmGeneratorTarget* target,
                            cm::GenEx::Evaluation* eval,
                            GeneratorExpressionContent const* content)
  {
    // The target soname file (.so.1).
    if (target->IsDLLPlatform()) {
      ::reportError(eval, content->GetOriginalExpression(),
                    "TARGET_SONAME_FILE is not allowed "
                    "for DLL target platforms.");
      return std::string();
    }
    if (target->GetType() != cmStateEnums::SHARED_LIBRARY) {
      ::reportError(eval, content->GetOriginalExpression(),
                    "TARGET_SONAME_FILE is allowed only for "
                    "SHARED libraries.");
      return std::string();
    }
    if (target->IsArchivedAIXSharedLibrary()) {
      ::reportError(eval, content->GetOriginalExpression(),
                    "TARGET_SONAME_FILE is not allowed for "
                    "AIX_SHARED_LIBRARY_ARCHIVE libraries.");
      return std::string();
    }
    std::string result =
      cmStrCat(target->GetDirectory(eval->Context.Config), '/',
               target->GetSOName(eval->Context.Config));
    return result;
  }
};

template <>
struct TargetFilesystemArtifactResultCreator<ArtifactSonameImportTag>
{
  static std::string Create(cmGeneratorTarget* target,
                            cm::GenEx::Evaluation* eval,
                            GeneratorExpressionContent const* content)
  {
    // The target soname file (.so.1).
    if (target->IsDLLPlatform()) {
      ::reportError(eval, content->GetOriginalExpression(),
                    "TARGET_SONAME_IMPORT_FILE is not allowed "
                    "for DLL target platforms.");
      return std::string();
    }
    if (target->GetType() != cmStateEnums::SHARED_LIBRARY) {
      ::reportError(eval, content->GetOriginalExpression(),
                    "TARGET_SONAME_IMPORT_FILE is allowed only for "
                    "SHARED libraries.");
      return std::string();
    }
    if (target->IsArchivedAIXSharedLibrary()) {
      ::reportError(eval, content->GetOriginalExpression(),
                    "TARGET_SONAME_IMPORT_FILE is not allowed for "
                    "AIX_SHARED_LIBRARY_ARCHIVE libraries.");
      return std::string();
    }

    if (target->HasImportLibrary(eval->Context.Config)) {
      return cmStrCat(
        target->GetDirectory(eval->Context.Config,
                             cmStateEnums::ImportLibraryArtifact),
        '/',
        target->GetSOName(eval->Context.Config,
                          cmStateEnums::ImportLibraryArtifact));
    }
    return std::string{};
  }
};

template <>
struct TargetFilesystemArtifactResultCreator<ArtifactPdbTag>
{
  static std::string Create(cmGeneratorTarget* target,
                            cm::GenEx::Evaluation* eval,
                            GeneratorExpressionContent const* content)
  {
    if (target->IsImported()) {
      ::reportError(eval, content->GetOriginalExpression(),
                    "TARGET_PDB_FILE not allowed for IMPORTED targets.");
      return std::string();
    }

    std::string language = target->GetLinkerLanguage(eval->Context.Config);

    std::string pdbSupportVar =
      cmStrCat("CMAKE_", language, "_LINKER_SUPPORTS_PDB");

    if (!eval->Context.LG->GetMakefile()->IsOn(pdbSupportVar)) {
      ::reportError(eval, content->GetOriginalExpression(),
                    "TARGET_PDB_FILE is not supported by the target linker.");
      return std::string();
    }

    cmStateEnums::TargetType targetType = target->GetType();

    if (targetType != cmStateEnums::SHARED_LIBRARY &&
        targetType != cmStateEnums::MODULE_LIBRARY &&
        targetType != cmStateEnums::EXECUTABLE) {
      ::reportError(eval, content->GetOriginalExpression(),
                    "TARGET_PDB_FILE is allowed only for "
                    "targets with linker created artifacts.");
      return std::string();
    }

    std::string result =
      cmStrCat(target->GetPDBDirectory(eval->Context.Config), '/',
               target->GetPDBName(eval->Context.Config));
    return result;
  }
};

template <>
struct TargetFilesystemArtifactResultCreator<ArtifactLinkerTag>
{
  static std::string Create(cmGeneratorTarget* target,
                            cm::GenEx::Evaluation* eval,
                            GeneratorExpressionContent const* content)
  {
    // The file used to link to the target (.so, .lib, .a) or import file
    // (.lib,  .tbd).
    if (!target->IsLinkable()) {
      ::reportError(eval, content->GetOriginalExpression(),
                    "TARGET_LINKER_FILE is allowed only for libraries and "
                    "executables with ENABLE_EXPORTS.");
      return std::string();
    }
    cmStateEnums::ArtifactType artifact =
      target->HasImportLibrary(eval->Context.Config)
      ? cmStateEnums::ImportLibraryArtifact
      : cmStateEnums::RuntimeBinaryArtifact;
    return target->GetFullPath(eval->Context.Config, artifact);
  }
};

template <>
struct TargetFilesystemArtifactResultCreator<ArtifactLinkerLibraryTag>
{
  static std::string Create(cmGeneratorTarget* target,
                            cm::GenEx::Evaluation* eval,
                            GeneratorExpressionContent const* content)
  {
    // The file used to link to the target (.dylib, .so, .a).
    if (!target->IsLinkable() ||
        target->GetType() == cmStateEnums::EXECUTABLE) {
      ::reportError(eval, content->GetOriginalExpression(),
                    "TARGET_LINKER_LIBRARY_FILE is allowed only for libraries "
                    "with ENABLE_EXPORTS.");
      return std::string();
    }

    if (!target->IsDLLPlatform() ||
        target->GetType() == cmStateEnums::STATIC_LIBRARY) {
      return target->GetFullPath(eval->Context.Config,
                                 cmStateEnums::RuntimeBinaryArtifact);
    }
    return std::string{};
  }
};

template <>
struct TargetFilesystemArtifactResultCreator<ArtifactLinkerImportTag>
{
  static std::string Create(cmGeneratorTarget* target,
                            cm::GenEx::Evaluation* eval,
                            GeneratorExpressionContent const* content)
  {
    // The file used to link to the target (.lib, .tbd).
    if (!target->IsLinkable()) {
      ::reportError(
        eval, content->GetOriginalExpression(),
        "TARGET_LINKER_IMPORT_FILE is allowed only for libraries and "
        "executables with ENABLE_EXPORTS.");
      return std::string();
    }

    if (target->HasImportLibrary(eval->Context.Config)) {
      return target->GetFullPath(eval->Context.Config,
                                 cmStateEnums::ImportLibraryArtifact);
    }
    return std::string{};
  }
};

template <>
struct TargetFilesystemArtifactResultCreator<ArtifactBundleDirTag>
{
  static std::string Create(cmGeneratorTarget* target,
                            cm::GenEx::Evaluation* eval,
                            GeneratorExpressionContent const* content)
  {
    if (target->IsImported()) {
      ::reportError(eval, content->GetOriginalExpression(),
                    "TARGET_BUNDLE_DIR not allowed for IMPORTED targets.");
      return std::string();
    }
    if (!target->IsBundleOnApple()) {
      ::reportError(eval, content->GetOriginalExpression(),
                    "TARGET_BUNDLE_DIR is allowed only for Bundle targets.");
      return std::string();
    }

    std::string outpath = target->GetDirectory(eval->Context.Config) + '/';
    return target->BuildBundleDirectory(outpath, eval->Context.Config,
                                        cmGeneratorTarget::BundleDirLevel);
  }
};

template <>
struct TargetFilesystemArtifactResultCreator<ArtifactBundleDirNameTag>
{
  static std::string Create(cmGeneratorTarget* target,
                            cm::GenEx::Evaluation* eval,
                            GeneratorExpressionContent const* content)
  {
    if (target->IsImported()) {
      ::reportError(
        eval, content->GetOriginalExpression(),
        "TARGET_BUNDLE_DIR_NAME not allowed for IMPORTED targets.");
      return std::string();
    }
    if (!target->IsBundleOnApple()) {
      ::reportError(
        eval, content->GetOriginalExpression(),
        "TARGET_BUNDLE_DIR_NAME is allowed only for Bundle targets.");
      return std::string();
    }

    auto level = cmGeneratorTarget::BundleDirLevel;
    auto config = eval->Context.Config;
    if (target->IsAppBundleOnApple()) {
      return target->GetAppBundleDirectory(config, level);
    }
    if (target->IsFrameworkOnApple()) {
      return target->GetFrameworkDirectory(config, level);
    }
    if (target->IsCFBundleOnApple()) {
      return target->GetCFBundleDirectory(config, level);
    }
    return std::string();
  }
};

template <>
struct TargetFilesystemArtifactResultCreator<ArtifactBundleContentDirTag>
{
  static std::string Create(cmGeneratorTarget* target,
                            cm::GenEx::Evaluation* eval,
                            GeneratorExpressionContent const* content)
  {
    if (target->IsImported()) {
      ::reportError(
        eval, content->GetOriginalExpression(),
        "TARGET_BUNDLE_CONTENT_DIR not allowed for IMPORTED targets.");
      return std::string();
    }
    if (!target->IsBundleOnApple()) {
      ::reportError(
        eval, content->GetOriginalExpression(),
        "TARGET_BUNDLE_CONTENT_DIR is allowed only for Bundle targets.");
      return std::string();
    }

    std::string outpath = target->GetDirectory(eval->Context.Config) + '/';
    return target->BuildBundleDirectory(outpath, eval->Context.Config,
                                        cmGeneratorTarget::ContentLevel);
  }
};

template <>
struct TargetFilesystemArtifactResultCreator<ArtifactNameTag>
{
  static std::string Create(cmGeneratorTarget* target,
                            cm::GenEx::Evaluation* eval,
                            GeneratorExpressionContent const* /*unused*/)
  {
    return target->GetFullPath(eval->Context.Config,
                               cmStateEnums::RuntimeBinaryArtifact, true);
  }
};

template <>
struct TargetFilesystemArtifactResultCreator<ArtifactImportTag>
{
  static std::string Create(cmGeneratorTarget* target,
                            cm::GenEx::Evaluation* eval,
                            GeneratorExpressionContent const* /*unused*/)
  {
    if (target->HasImportLibrary(eval->Context.Config)) {
      return target->GetFullPath(eval->Context.Config,
                                 cmStateEnums::ImportLibraryArtifact, true);
    }
    return std::string{};
  }
};

template <typename ArtifactT>
struct TargetFilesystemArtifactResultGetter
{
  static std::string Get(std::string const& result);
};

template <>
struct TargetFilesystemArtifactResultGetter<ArtifactNameTag>
{
  static std::string Get(std::string const& result)
  {
    return cmSystemTools::GetFilenameName(result);
  }
};

template <>
struct TargetFilesystemArtifactResultGetter<ArtifactDirTag>
{
  static std::string Get(std::string const& result)
  {
    return cmSystemTools::GetFilenamePath(result);
  }
};

template <>
struct TargetFilesystemArtifactResultGetter<ArtifactPathTag>
{
  static std::string Get(std::string const& result) { return result; }
};

struct TargetArtifactBase : public cmGeneratorExpressionNode
{
  TargetArtifactBase() {} // NOLINT(modernize-use-equals-default)

protected:
  cmGeneratorTarget* GetTarget(
    std::vector<std::string> const& parameters, cm::GenEx::Evaluation* eval,
    GeneratorExpressionContent const* content,
    cmGeneratorExpressionDAGChecker* dagChecker) const
  {
    // Lookup the referenced target.
    std::string const& name = parameters.front();

    if (!cmGeneratorExpression::IsValidTargetName(name)) {
      ::reportError(eval, content->GetOriginalExpression(),
                    "Expression syntax not recognized.");
      return nullptr;
    }
    cmGeneratorTarget* target =
      eval->Context.LG->FindGeneratorTargetToUse(name);
    if (!target) {
      ::reportError(eval, content->GetOriginalExpression(),
                    cmStrCat("No target \"", name, '"'));
      return nullptr;
    }
    if (target->GetType() >= cmStateEnums::OBJECT_LIBRARY &&
        target->GetType() != cmStateEnums::UNKNOWN_LIBRARY) {
      ::reportError(
        eval, content->GetOriginalExpression(),
        cmStrCat("Target \"", name, "\" is not an executable or library."));
      return nullptr;
    }
    if (dagChecker &&
        (dagChecker->EvaluatingLinkLibraries(target) ||
         (dagChecker->EvaluatingSources() &&
          target == dagChecker->TopTarget()))) {
      ::reportError(eval, content->GetOriginalExpression(),
                    "Expressions which require the linker language may not "
                    "be used while evaluating link libraries");
      return nullptr;
    }

    return target;
  }
};

template <typename ArtifactT, typename ComponentT>
struct TargetFilesystemArtifact : public TargetArtifactBase
{
  TargetFilesystemArtifact() {} // NOLINT(modernize-use-equals-default)

  int NumExpectedParameters() const override { return 1; }

  std::string Evaluate(
    std::vector<std::string> const& parameters, cm::GenEx::Evaluation* eval,
    GeneratorExpressionContent const* content,
    cmGeneratorExpressionDAGChecker* dagChecker) const override
  {
    cmGeneratorTarget* target =
      this->GetTarget(parameters, eval, content, dagChecker);
    if (!target) {
      return std::string();
    }
    // Not a dependent target if we are querying for ArtifactDirTag,
    // ArtifactNameTag, ArtifactBundleDirTag, ArtifactBundleDirNameTag,
    // and ArtifactBundleContentDirTag
    TargetFilesystemArtifactDependency<ArtifactT, ComponentT>::AddDependency(
      target, eval);

    std::string result =
      TargetFilesystemArtifactResultCreator<ArtifactT>::Create(target, eval,
                                                               content);
    if (eval->HadError) {
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

static TargetFilesystemArtifactNodeGroup<ArtifactNameTag> const
  targetNodeGroup;

static TargetFilesystemArtifactNodeGroup<ArtifactImportTag> const
  targetImportNodeGroup;

static TargetFilesystemArtifactNodeGroup<ArtifactLinkerTag> const
  targetLinkerNodeGroup;

static TargetFilesystemArtifactNodeGroup<ArtifactLinkerLibraryTag> const
  targetLinkerLibraryNodeGroup;

static TargetFilesystemArtifactNodeGroup<ArtifactLinkerImportTag> const
  targetLinkerImportNodeGroup;

static TargetFilesystemArtifactNodeGroup<ArtifactSonameTag> const
  targetSoNameNodeGroup;

static TargetFilesystemArtifactNodeGroup<ArtifactSonameImportTag> const
  targetSoNameImportNodeGroup;

static TargetFilesystemArtifactNodeGroup<ArtifactPdbTag> const
  targetPdbNodeGroup;

static TargetFilesystemArtifact<ArtifactBundleDirTag, ArtifactPathTag> const
  targetBundleDirNode;

static TargetFilesystemArtifact<ArtifactBundleDirNameTag,
                                ArtifactNameTag> const targetBundleDirNameNode;

static TargetFilesystemArtifact<ArtifactBundleContentDirTag,
                                ArtifactPathTag> const
  targetBundleContentDirNode;

//
// To retrieve base name for various artifacts
//
enum class Postfix
{
  Unspecified,
  Exclude,
  Include
};

template <typename ArtifactT>
struct TargetOutputNameArtifactResultGetter
{
  static std::string Get(cmGeneratorTarget* target,
                         cm::GenEx::Evaluation* eval,
                         GeneratorExpressionContent const* content,
                         Postfix postfix);
};

template <>
struct TargetOutputNameArtifactResultGetter<ArtifactNameTag>
{
  static std::string Get(cmGeneratorTarget* target,
                         cm::GenEx::Evaluation* eval,
                         GeneratorExpressionContent const* /*unused*/,
                         Postfix postfix)
  {
    auto output = target->GetOutputName(eval->Context.Config,
                                        cmStateEnums::RuntimeBinaryArtifact);
    return postfix != Postfix::Exclude
      ? cmStrCat(output, target->GetFilePostfix(eval->Context.Config))
      : output;
  }
};

template <>
struct TargetOutputNameArtifactResultGetter<ArtifactImportTag>
{
  static std::string Get(cmGeneratorTarget* target,
                         cm::GenEx::Evaluation* eval,
                         GeneratorExpressionContent const* /*unused*/,
                         Postfix postfix)
  {
    if (target->HasImportLibrary(eval->Context.Config)) {
      auto output = target->GetOutputName(eval->Context.Config,
                                          cmStateEnums::ImportLibraryArtifact);
      return postfix != Postfix::Exclude
        ? cmStrCat(output, target->GetFilePostfix(eval->Context.Config))
        : output;
    }
    return std::string{};
  }
};

template <>
struct TargetOutputNameArtifactResultGetter<ArtifactLinkerTag>
{
  static std::string Get(cmGeneratorTarget* target,
                         cm::GenEx::Evaluation* eval,
                         GeneratorExpressionContent const* content,
                         Postfix postfix)
  {
    // The library file used to link to the target (.so, .lib, .a) or import
    // file (.lin,  .tbd).
    if (!target->IsLinkable()) {
      ::reportError(eval, content->GetOriginalExpression(),
                    "TARGET_LINKER_FILE_BASE_NAME is allowed only for "
                    "libraries and executables with ENABLE_EXPORTS.");
      return std::string();
    }
    cmStateEnums::ArtifactType artifact =
      target->HasImportLibrary(eval->Context.Config)
      ? cmStateEnums::ImportLibraryArtifact
      : cmStateEnums::RuntimeBinaryArtifact;
    auto output = target->GetOutputName(eval->Context.Config, artifact);
    return postfix != Postfix::Exclude
      ? cmStrCat(output, target->GetFilePostfix(eval->Context.Config))
      : output;
  }
};

template <>
struct TargetOutputNameArtifactResultGetter<ArtifactLinkerLibraryTag>
{
  static std::string Get(cmGeneratorTarget* target,
                         cm::GenEx::Evaluation* eval,
                         GeneratorExpressionContent const* content,
                         Postfix postfix)
  {
    // The library file used to link to the target (.so, .lib, .a).
    if (!target->IsLinkable() ||
        target->GetType() == cmStateEnums::EXECUTABLE) {
      ::reportError(eval, content->GetOriginalExpression(),
                    "TARGET_LINKER_LIBRARY_FILE_BASE_NAME is allowed only for "
                    "libraries with ENABLE_EXPORTS.");
      return std::string();
    }

    if (!target->IsDLLPlatform() ||
        target->GetType() == cmStateEnums::STATIC_LIBRARY) {
      auto output = target->GetOutputName(eval->Context.Config,
                                          cmStateEnums::ImportLibraryArtifact);
      return postfix != Postfix::Exclude
        ? cmStrCat(output, target->GetFilePostfix(eval->Context.Config))
        : output;
    }
    return std::string{};
  }
};

template <>
struct TargetOutputNameArtifactResultGetter<ArtifactLinkerImportTag>
{
  static std::string Get(cmGeneratorTarget* target,
                         cm::GenEx::Evaluation* eval,
                         GeneratorExpressionContent const* content,
                         Postfix postfix)
  {
    // The import file used to link to the target (.lib, .tbd).
    if (!target->IsLinkable()) {
      ::reportError(eval, content->GetOriginalExpression(),
                    "TARGET_LINKER_IMPORT_FILE_BASE_NAME is allowed only for "
                    "libraries and executables with ENABLE_EXPORTS.");
      return std::string();
    }

    if (target->HasImportLibrary(eval->Context.Config)) {
      auto output = target->GetOutputName(eval->Context.Config,
                                          cmStateEnums::ImportLibraryArtifact);
      return postfix != Postfix::Exclude
        ? cmStrCat(output, target->GetFilePostfix(eval->Context.Config))
        : output;
    }
    return std::string{};
  }
};

template <>
struct TargetOutputNameArtifactResultGetter<ArtifactPdbTag>
{
  static std::string Get(cmGeneratorTarget* target,
                         cm::GenEx::Evaluation* eval,
                         GeneratorExpressionContent const* content,
                         Postfix postfix)
  {
    if (target->IsImported()) {
      ::reportError(
        eval, content->GetOriginalExpression(),
        "TARGET_PDB_FILE_BASE_NAME not allowed for IMPORTED targets.");
      return std::string();
    }

    std::string language = target->GetLinkerLanguage(eval->Context.Config);

    std::string pdbSupportVar =
      cmStrCat("CMAKE_", language, "_LINKER_SUPPORTS_PDB");

    if (!eval->Context.LG->GetMakefile()->IsOn(pdbSupportVar)) {
      ::reportError(
        eval, content->GetOriginalExpression(),
        "TARGET_PDB_FILE_BASE_NAME is not supported by the target linker.");
      return std::string();
    }

    cmStateEnums::TargetType targetType = target->GetType();

    if (targetType != cmStateEnums::SHARED_LIBRARY &&
        targetType != cmStateEnums::MODULE_LIBRARY &&
        targetType != cmStateEnums::EXECUTABLE) {
      ::reportError(eval, content->GetOriginalExpression(),
                    "TARGET_PDB_FILE_BASE_NAME is allowed only for "
                    "targets with linker created artifacts.");
      return std::string();
    }

    auto output = target->GetPDBOutputName(eval->Context.Config);

    if (target->GetPolicyStatusCMP0202() == cmPolicies::NEW) {
      return postfix != Postfix::Exclude
        ? cmStrCat(output, target->GetFilePostfix(eval->Context.Config))
        : output;
    }

    if (target->GetPolicyStatusCMP0202() == cmPolicies::WARN &&
        postfix != Postfix::Unspecified) {
      eval->Context.LG->GetCMakeInstance()->IssueMessage(
        MessageType::AUTHOR_WARNING,
        cmStrCat(cmPolicies::GetPolicyWarning(cmPolicies::CMP0202),
                 "\n"
                 "\"POSTFIX\" option is recognized only when the policy is "
                 "set to NEW. Since the policy is not set, the OLD behavior "
                 "will be used."),
        eval->Backtrace);
    }

    return output;
  }
};

template <typename ArtifactT>
struct TargetFileBaseNameArtifact : public TargetArtifactBase
{
  TargetFileBaseNameArtifact() {} // NOLINT(modernize-use-equals-default)

  int NumExpectedParameters() const override { return OneOrMoreParameters; }

  std::string Evaluate(
    std::vector<std::string> const& parameters, cm::GenEx::Evaluation* eval,
    GeneratorExpressionContent const* content,
    cmGeneratorExpressionDAGChecker* dagChecker) const override
  {
    if (parameters.size() > 2) {
      ::reportError(eval, content->GetOriginalExpression(),
                    "Unexpected parameters, require one or two parameters.");
      return std::string{};
    }

    cmGeneratorTarget* target =
      this->GetTarget(parameters, eval, content, dagChecker);
    if (!target) {
      return std::string();
    }

    Postfix postfix = Postfix::Unspecified;
    if (parameters.size() == 2) {
      if (parameters[1] == "POSTFIX:INCLUDE") {
        postfix = Postfix::Include;
      } else if (parameters[1] == "POSTFIX:EXCLUDE") {
        postfix = Postfix::Exclude;
      } else {
        ::reportError(eval, content->GetOriginalExpression(),
                      "Wrong second parameter: \"POSTFIX:INCLUDE\" or "
                      "\"POSTFIX:EXCLUDE\" is expected");
      }
    }

    std::string result = TargetOutputNameArtifactResultGetter<ArtifactT>::Get(
      target, eval, content, postfix);
    if (eval->HadError) {
      return std::string();
    }
    return result;
  }
};

static TargetFileBaseNameArtifact<ArtifactNameTag> const
  targetFileBaseNameNode;
static TargetFileBaseNameArtifact<ArtifactImportTag> const
  targetImportFileBaseNameNode;
static TargetFileBaseNameArtifact<ArtifactLinkerTag> const
  targetLinkerFileBaseNameNode;
static TargetFileBaseNameArtifact<ArtifactLinkerLibraryTag> const
  targetLinkerLibraryFileBaseNameNode;
static TargetFileBaseNameArtifact<ArtifactLinkerImportTag> const
  targetLinkerImportFileBaseNameNode;
static TargetFileBaseNameArtifact<ArtifactPdbTag> const
  targetPdbFileBaseNameNode;

class ArtifactFilePrefixTag;
class ArtifactImportFilePrefixTag;
class ArtifactLinkerFilePrefixTag;
class ArtifactLinkerLibraryFilePrefixTag;
class ArtifactLinkerImportFilePrefixTag;
class ArtifactFileSuffixTag;
class ArtifactImportFileSuffixTag;
class ArtifactLinkerFileSuffixTag;
class ArtifactLinkerLibraryFileSuffixTag;
class ArtifactLinkerImportFileSuffixTag;

template <typename ArtifactT>
struct TargetFileArtifactResultGetter
{
  static std::string Get(cmGeneratorTarget* target,
                         cm::GenEx::Evaluation* eval,
                         GeneratorExpressionContent const* content);
};

template <>
struct TargetFileArtifactResultGetter<ArtifactFilePrefixTag>
{
  static std::string Get(cmGeneratorTarget* target,
                         cm::GenEx::Evaluation* eval,
                         GeneratorExpressionContent const*)
  {
    return target->GetFilePrefix(eval->Context.Config);
  }
};
template <>
struct TargetFileArtifactResultGetter<ArtifactImportFilePrefixTag>
{
  static std::string Get(cmGeneratorTarget* target,
                         cm::GenEx::Evaluation* eval,
                         GeneratorExpressionContent const*)
  {
    if (target->HasImportLibrary(eval->Context.Config)) {
      return target->GetFilePrefix(eval->Context.Config,
                                   cmStateEnums::ImportLibraryArtifact);
    }
    return std::string{};
  }
};
template <>
struct TargetFileArtifactResultGetter<ArtifactLinkerFilePrefixTag>
{
  static std::string Get(cmGeneratorTarget* target,
                         cm::GenEx::Evaluation* eval,
                         GeneratorExpressionContent const* content)
  {
    if (!target->IsLinkable()) {
      ::reportError(
        eval, content->GetOriginalExpression(),
        "TARGET_LINKER_FILE_PREFIX is allowed only for libraries and "
        "executables with ENABLE_EXPORTS.");
      return std::string();
    }

    cmStateEnums::ArtifactType artifact =
      target->HasImportLibrary(eval->Context.Config)
      ? cmStateEnums::ImportLibraryArtifact
      : cmStateEnums::RuntimeBinaryArtifact;

    return target->GetFilePrefix(eval->Context.Config, artifact);
  }
};
template <>
struct TargetFileArtifactResultGetter<ArtifactLinkerLibraryFilePrefixTag>
{
  static std::string Get(cmGeneratorTarget* target,
                         cm::GenEx::Evaluation* eval,
                         GeneratorExpressionContent const* content)
  {
    if (!target->IsLinkable() ||
        target->GetType() == cmStateEnums::EXECUTABLE) {
      ::reportError(
        eval, content->GetOriginalExpression(),
        "TARGET_LINKER_LIBRARY_FILE_PREFIX is allowed only for libraries "
        "with ENABLE_EXPORTS.");
      return std::string();
    }

    if (!target->IsDLLPlatform() ||
        target->GetType() == cmStateEnums::STATIC_LIBRARY) {
      return target->GetFilePrefix(eval->Context.Config,
                                   cmStateEnums::RuntimeBinaryArtifact);
    }
    return std::string{};
  }
};
template <>
struct TargetFileArtifactResultGetter<ArtifactLinkerImportFilePrefixTag>
{
  static std::string Get(cmGeneratorTarget* target,
                         cm::GenEx::Evaluation* eval,
                         GeneratorExpressionContent const* content)
  {
    if (!target->IsLinkable()) {
      ::reportError(
        eval, content->GetOriginalExpression(),
        "TARGET_LINKER_IMPORT_FILE_PREFIX is allowed only for libraries and "
        "executables with ENABLE_EXPORTS.");
      return std::string();
    }

    if (target->HasImportLibrary(eval->Context.Config)) {
      return target->GetFilePrefix(eval->Context.Config,
                                   cmStateEnums::ImportLibraryArtifact);
    }
    return std::string{};
  }
};
template <>
struct TargetFileArtifactResultGetter<ArtifactFileSuffixTag>
{
  static std::string Get(cmGeneratorTarget* target,
                         cm::GenEx::Evaluation* eval,
                         GeneratorExpressionContent const*)
  {
    return target->GetFileSuffix(eval->Context.Config);
  }
};
template <>
struct TargetFileArtifactResultGetter<ArtifactImportFileSuffixTag>
{
  static std::string Get(cmGeneratorTarget* target,
                         cm::GenEx::Evaluation* eval,
                         GeneratorExpressionContent const*)
  {
    if (target->HasImportLibrary(eval->Context.Config)) {
      return target->GetFileSuffix(eval->Context.Config,
                                   cmStateEnums::ImportLibraryArtifact);
    }
    return std::string{};
  }
};
template <>
struct TargetFileArtifactResultGetter<ArtifactLinkerFileSuffixTag>
{
  static std::string Get(cmGeneratorTarget* target,
                         cm::GenEx::Evaluation* eval,
                         GeneratorExpressionContent const* content)
  {
    if (!target->IsLinkable()) {
      ::reportError(
        eval, content->GetOriginalExpression(),
        "TARGET_LINKER_FILE_SUFFIX is allowed only for libraries and "
        "executables with ENABLE_EXPORTS.");
      return std::string();
    }

    cmStateEnums::ArtifactType artifact =
      target->HasImportLibrary(eval->Context.Config)
      ? cmStateEnums::ImportLibraryArtifact
      : cmStateEnums::RuntimeBinaryArtifact;

    return target->GetFileSuffix(eval->Context.Config, artifact);
  }
};
template <>
struct TargetFileArtifactResultGetter<ArtifactLinkerLibraryFileSuffixTag>
{
  static std::string Get(cmGeneratorTarget* target,
                         cm::GenEx::Evaluation* eval,
                         GeneratorExpressionContent const* content)
  {
    if (!target->IsLinkable() ||
        target->GetType() == cmStateEnums::STATIC_LIBRARY) {
      ::reportError(eval, content->GetOriginalExpression(),
                    "TARGET_LINKER_LIBRARY_FILE_SUFFIX is allowed only for "
                    "libraries with ENABLE_EXPORTS.");
      return std::string();
    }

    if (!target->IsDLLPlatform() ||
        target->GetType() == cmStateEnums::STATIC_LIBRARY) {
      return target->GetFileSuffix(eval->Context.Config,
                                   cmStateEnums::RuntimeBinaryArtifact);
    }
    return std::string{};
  }
};
template <>
struct TargetFileArtifactResultGetter<ArtifactLinkerImportFileSuffixTag>
{
  static std::string Get(cmGeneratorTarget* target,
                         cm::GenEx::Evaluation* eval,
                         GeneratorExpressionContent const* content)
  {
    if (!target->IsLinkable()) {
      ::reportError(
        eval, content->GetOriginalExpression(),
        "TARGET_LINKER_IMPORT_FILE_SUFFIX is allowed only for libraries and "
        "executables with ENABLE_EXPORTS.");
      return std::string();
    }

    if (target->HasImportLibrary(eval->Context.Config)) {
      return target->GetFileSuffix(eval->Context.Config,
                                   cmStateEnums::ImportLibraryArtifact);
    }
    return std::string{};
  }
};

template <typename ArtifactT>
struct TargetFileArtifact : public TargetArtifactBase
{
  TargetFileArtifact() {} // NOLINT(modernize-use-equals-default)

  int NumExpectedParameters() const override { return 1; }

  std::string Evaluate(
    std::vector<std::string> const& parameters, cm::GenEx::Evaluation* eval,
    GeneratorExpressionContent const* content,
    cmGeneratorExpressionDAGChecker* dagChecker) const override
  {
    cmGeneratorTarget* target =
      this->GetTarget(parameters, eval, content, dagChecker);
    if (!target) {
      return std::string();
    }

    std::string result =
      TargetFileArtifactResultGetter<ArtifactT>::Get(target, eval, content);
    if (eval->HadError) {
      return std::string();
    }
    return result;
  }
};

static TargetFileArtifact<ArtifactFilePrefixTag> const targetFilePrefixNode;
static TargetFileArtifact<ArtifactImportFilePrefixTag> const
  targetImportFilePrefixNode;
static TargetFileArtifact<ArtifactLinkerFilePrefixTag> const
  targetLinkerFilePrefixNode;
static TargetFileArtifact<ArtifactLinkerLibraryFilePrefixTag> const
  targetLinkerLibraryFilePrefixNode;
static TargetFileArtifact<ArtifactLinkerImportFilePrefixTag> const
  targetLinkerImportFilePrefixNode;
static TargetFileArtifact<ArtifactFileSuffixTag> const targetFileSuffixNode;
static TargetFileArtifact<ArtifactImportFileSuffixTag> const
  targetImportFileSuffixNode;
static TargetFileArtifact<ArtifactLinkerFileSuffixTag> const
  targetLinkerFileSuffixNode;
static TargetFileArtifact<ArtifactLinkerLibraryFileSuffixTag> const
  targetLinkerLibraryFileSuffixNode;
static TargetFileArtifact<ArtifactLinkerImportFileSuffixTag> const
  targetLinkerImportFileSuffixNode;

static const struct ShellPathNode : public cmGeneratorExpressionNode
{
  ShellPathNode() {} // NOLINT(modernize-use-equals-default)

  std::string Evaluate(
    std::vector<std::string> const& parameters, cm::GenEx::Evaluation* eval,
    GeneratorExpressionContent const* content,
    cmGeneratorExpressionDAGChecker* /*dagChecker*/) const override
  {
    cmList list_in{ parameters.front() };
    if (list_in.empty()) {
      reportError(eval, content->GetOriginalExpression(),
                  "\"\" is not an absolute path.");
      return std::string();
    }
    cmStateSnapshot snapshot = eval->Context.LG->GetStateSnapshot();
    cmOutputConverter converter(snapshot);
    char const* separator = snapshot.GetState()->UseWindowsShell() ? ";" : ":";
    std::vector<std::string> list_out;
    list_out.reserve(list_in.size());
    for (auto const& in : list_in) {
      if (!cmSystemTools::FileIsFullPath(in)) {
        reportError(eval, content->GetOriginalExpression(),
                    cmStrCat('"', in, "\" is not an absolute path."));
        return std::string();
      }
      list_out.emplace_back(converter.ConvertDirectorySeparatorsForShell(in));
    }
    return cmJoin(list_out, separator);
  }
} shellPathNode;

cmGeneratorExpressionNode const* cmGeneratorExpressionNode::GetNode(
  std::string const& identifier)
{
  static std::map<std::string, cmGeneratorExpressionNode const*> const nodeMap{
    { "0", &zeroNode },
    { "1", &oneNode },
    { "AND", &andNode },
    { "OR", &orNode },
    { "NOT", &notNode },
    { "C_COMPILER_ID", &cCompilerIdNode },
    { "CXX_COMPILER_ID", &cxxCompilerIdNode },
    { "OBJC_COMPILER_ID", &objcCompilerIdNode },
    { "OBJCXX_COMPILER_ID", &objcxxCompilerIdNode },
    { "CUDA_COMPILER_ID", &cudaCompilerIdNode },
    { "Fortran_COMPILER_ID", &fortranCompilerIdNode },
    { "HIP_COMPILER_ID", &hipCompilerIdNode },
    { "VERSION_GREATER", &versionGreaterNode },
    { "VERSION_GREATER_EQUAL", &versionGreaterEqNode },
    { "VERSION_LESS", &versionLessNode },
    { "VERSION_LESS_EQUAL", &versionLessEqNode },
    { "VERSION_EQUAL", &versionEqualNode },
    { "C_COMPILER_VERSION", &cCompilerVersionNode },
    { "CXX_COMPILER_VERSION", &cxxCompilerVersionNode },
    { "CUDA_COMPILER_VERSION", &cudaCompilerVersionNode },
    { "OBJC_COMPILER_VERSION", &objcCompilerVersionNode },
    { "OBJCXX_COMPILER_VERSION", &objcxxCompilerVersionNode },
    { "Fortran_COMPILER_VERSION", &fortranCompilerVersionNode },
    { "HIP_COMPILER_VERSION", &hipCompilerVersionNode },
    { "C_COMPILER_FRONTEND_VARIANT", &cCompilerFrontendVariantNode },
    { "CXX_COMPILER_FRONTEND_VARIANT", &cxxCompilerFrontendVariantNode },
    { "CUDA_COMPILER_FRONTEND_VARIANT", &cudaCompilerFrontendVariantNode },
    { "OBJC_COMPILER_FRONTEND_VARIANT", &objcCompilerFrontendVariantNode },
    { "OBJCXX_COMPILER_FRONTEND_VARIANT", &objcxxCompilerFrontendVariantNode },
    { "Fortran_COMPILER_FRONTEND_VARIANT",
      &fortranCompilerFrontendVariantNode },
    { "HIP_COMPILER_FRONTEND_VARIANT", &hipCompilerFrontendVariantNode },
    { "PLATFORM_ID", &platformIdNode },
    { "COMPILE_FEATURES", &compileFeaturesNode },
    { "CONFIGURATION", &configurationNode },
    { "CONFIG", &configurationTestNode },
    { "TARGET_FILE", &targetNodeGroup.File },
    { "TARGET_IMPORT_FILE", &targetImportNodeGroup.File },
    { "TARGET_LINKER_FILE", &targetLinkerNodeGroup.File },
    { "TARGET_LINKER_LIBRARY_FILE", &targetLinkerLibraryNodeGroup.File },
    { "TARGET_LINKER_IMPORT_FILE", &targetLinkerImportNodeGroup.File },
    { "TARGET_SONAME_FILE", &targetSoNameNodeGroup.File },
    { "TARGET_SONAME_IMPORT_FILE", &targetSoNameImportNodeGroup.File },
    { "TARGET_PDB_FILE", &targetPdbNodeGroup.File },
    { "TARGET_FILE_BASE_NAME", &targetFileBaseNameNode },
    { "TARGET_IMPORT_FILE_BASE_NAME", &targetImportFileBaseNameNode },
    { "TARGET_LINKER_FILE_BASE_NAME", &targetLinkerFileBaseNameNode },
    { "TARGET_LINKER_LIBRARY_FILE_BASE_NAME",
      &targetLinkerLibraryFileBaseNameNode },
    { "TARGET_LINKER_IMPORT_FILE_BASE_NAME",
      &targetLinkerImportFileBaseNameNode },
    { "TARGET_PDB_FILE_BASE_NAME", &targetPdbFileBaseNameNode },
    { "TARGET_FILE_PREFIX", &targetFilePrefixNode },
    { "TARGET_IMPORT_FILE_PREFIX", &targetImportFilePrefixNode },
    { "TARGET_LINKER_FILE_PREFIX", &targetLinkerFilePrefixNode },
    { "TARGET_LINKER_LIBRARY_FILE_PREFIX",
      &targetLinkerLibraryFilePrefixNode },
    { "TARGET_LINKER_IMPORT_FILE_PREFIX", &targetLinkerImportFilePrefixNode },
    { "TARGET_FILE_SUFFIX", &targetFileSuffixNode },
    { "TARGET_IMPORT_FILE_SUFFIX", &targetImportFileSuffixNode },
    { "TARGET_LINKER_FILE_SUFFIX", &targetLinkerFileSuffixNode },
    { "TARGET_LINKER_LIBRARY_FILE_SUFFIX",
      &targetLinkerLibraryFileSuffixNode },
    { "TARGET_LINKER_IMPORT_FILE_SUFFIX", &targetLinkerImportFileSuffixNode },
    { "TARGET_FILE_NAME", &targetNodeGroup.FileName },
    { "TARGET_IMPORT_FILE_NAME", &targetImportNodeGroup.FileName },
    { "TARGET_LINKER_FILE_NAME", &targetLinkerNodeGroup.FileName },
    { "TARGET_LINKER_LIBRARY_FILE_NAME",
      &targetLinkerLibraryNodeGroup.FileName },
    { "TARGET_LINKER_IMPORT_FILE_NAME",
      &targetLinkerImportNodeGroup.FileName },
    { "TARGET_SONAME_FILE_NAME", &targetSoNameNodeGroup.FileName },
    { "TARGET_SONAME_IMPORT_FILE_NAME",
      &targetSoNameImportNodeGroup.FileName },
    { "TARGET_PDB_FILE_NAME", &targetPdbNodeGroup.FileName },
    { "TARGET_FILE_DIR", &targetNodeGroup.FileDir },
    { "TARGET_IMPORT_FILE_DIR", &targetImportNodeGroup.FileDir },
    { "TARGET_LINKER_FILE_DIR", &targetLinkerNodeGroup.FileDir },
    { "TARGET_LINKER_LIBRARY_FILE_DIR",
      &targetLinkerLibraryNodeGroup.FileDir },
    { "TARGET_LINKER_IMPORT_FILE_DIR", &targetLinkerImportNodeGroup.FileDir },
    { "TARGET_SONAME_FILE_DIR", &targetSoNameNodeGroup.FileDir },
    { "TARGET_SONAME_IMPORT_FILE_DIR", &targetSoNameImportNodeGroup.FileDir },
    { "TARGET_PDB_FILE_DIR", &targetPdbNodeGroup.FileDir },
    { "TARGET_BUNDLE_DIR", &targetBundleDirNode },
    { "TARGET_BUNDLE_DIR_NAME", &targetBundleDirNameNode },
    { "TARGET_BUNDLE_CONTENT_DIR", &targetBundleContentDirNode },
    { "STREQUAL", &strEqualNode },
    { "STRLESS", &strLessNode },
    { "STRLESS_EQUAL", &strLessEqualNode },
    { "STRGREATER", &strGreaterNode },
    { "STRGREATER_EQUAL", &strGreaterEqualNode },
    { "STRING", &stringNode },
    { "EQUAL", &equalNode },
    { "IN_LIST", &inListNode },
    { "FILTER", &filterNode },
    { "REMOVE_DUPLICATES", &removeDuplicatesNode },
    { "LIST", &listNode },
    { "LOWER_CASE", &lowerCaseNode },
    { "UPPER_CASE", &upperCaseNode },
    { "PATH", &pathNode },
    { "PATH_EQUAL", &pathEqualNode },
    { "MAKE_C_IDENTIFIER", &makeCIdentifierNode },
    { "BOOL", &boolNode },
    { "IF", &ifNode },
    { "ANGLE-R", &angle_rNode },
    { "COMMA", &commaNode },
    { "SEMICOLON", &semicolonNode },
    { "QUOTE", &quoteNode },
    { "SOURCE_EXISTS", &sourceExistsNode },
    { "SOURCE_PROPERTY", &sourcePropertyNode },
    { "FILE_SET_EXISTS", &fileSetExistsNode },
    { "FILE_SET_PROPERTY", &fileSetPropertyNode },
    { "TARGET_PROPERTY", &targetPropertyNode },
    { "TARGET_INTERMEDIATE_DIR", &targetIntermediateDirNode },
    { "TARGET_NAME", &targetNameNode },
    { "TARGET_OBJECTS", &targetObjectsNode },
    { "TARGET_POLICY", &targetPolicyNode },
    { "TARGET_EXISTS", &targetExistsNode },
    { "TARGET_NAME_IF_EXISTS", &targetNameIfExistsNode },
    { "TARGET_GENEX_EVAL", &targetGenexEvalNode },
    { "TARGET_RUNTIME_DLLS", &targetRuntimeDllsNode },
    { "TARGET_RUNTIME_DLL_DIRS", &targetRuntimeDllDirsNode },
    { "GENEX_EVAL", &genexEvalNode },
    { "BUILD_INTERFACE", &buildInterfaceNode },
    { "INSTALL_INTERFACE", &installInterfaceNode },
    { "BUILD_LOCAL_INTERFACE", &buildLocalInterfaceNode },
    { "INSTALL_PREFIX", &installPrefixNode },
    { "JOIN", &joinNode },
    { "COMPILE_ONLY", &compileOnlyNode },
    { "LINK_ONLY", &linkOnlyNode },
    { "COMPILE_LANG_AND_ID", &languageAndIdNode },
    { "COMPILE_LANGUAGE", &languageNode },
    { "LINK_LANG_AND_ID", &linkLanguageAndIdNode },
    { "LINK_LANGUAGE", &linkLanguageNode },
    { "C_COMPILER_LINKER_ID", &cCompilerLinkerIdNode },
    { "CXX_COMPILER_LINKER_ID", &cxxCompilerLinkerIdNode },
    { "OBJC_COMPILER_LINKER_ID", &objcCompilerLinkerIdNode },
    { "OBJCXX_COMPILER_LINKER_ID", &objcxxCompilerLinkerIdNode },
    { "CUDA_COMPILER_LINKER_ID", &cudaCompilerLinkerIdNode },
    { "Fortran_COMPILER_LINKER_ID", &fortranCompilerLinkerIdNode },
    { "HIP_COMPILER_LINKER_ID", &hipCompilerLinkerIdNode },
    { "C_COMPILER_LINKER_FRONTEND_VARIANT",
      &cCompilerLinkerFrontendVariantNode },
    { "CXX_COMPILER_LINKER_FRONTEND_VARIANT",
      &cxxCompilerLinkerFrontendVariantNode },
    { "CUDA_COMPILER_LINKER_FRONTEND_VARIANT",
      &cudaCompilerLinkerFrontendVariantNode },
    { "OBJC_COMPILER_LINKER_FRONTEND_VARIANT",
      &objcCompilerLinkerFrontendVariantNode },
    { "OBJCXX_COMPILER_LINKER_FRONTEND_VARIANT",
      &objcxxCompilerLinkerFrontendVariantNode },
    { "Fortran_COMPILER_LINKER_FRONTEND_VARIANT",
      &fortranCompilerLinkerFrontendVariantNode },
    { "HIP_COMPILER_LINKER_FRONTEND_VARIANT",
      &hipCompilerLinkerFrontendVariantNode },
    { "LINK_LIBRARY", &linkLibraryNode },
    { "LINK_GROUP", &linkGroupNode },
    { "HOST_LINK", &hostLinkNode },
    { "DEVICE_LINK", &deviceLinkNode },
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

void reportError(cm::GenEx::Evaluation* eval, std::string const& expr,
                 std::string const& result)
{
  eval->HadError = true;
  if (eval->Quiet) {
    return;
  }

  std::ostringstream e;
  /* clang-format off */
  e << "Error evaluating generator expression:\n"
    << "  " << expr << "\n"
    << result;
  /* clang-format on */
  eval->Context.LG->GetCMakeInstance()->IssueMessage(MessageType::FATAL_ERROR,
                                                     e.str(), eval->Backtrace);
}
