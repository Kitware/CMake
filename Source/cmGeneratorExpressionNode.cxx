/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmGeneratorExpressionNode.h"

#include <algorithm>
#include <cassert>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <unordered_map>
#include <utility>

#include <cm/iterator>
#include <cm/optional>
#include <cm/string_view>
#include <cm/vector>
#include <cmext/algorithm>
#include <cmext/string_view>

#include "cmsys/RegularExpression.hxx"
#include "cmsys/String.h"

#include "cmAlgorithms.h"
#include "cmCMakePath.h"
#include "cmComputeLinkInformation.h"
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
#include "cmRange.h"
#include "cmStandardLevelResolver.h"
#include "cmState.h"
#include "cmStateSnapshot.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmTarget.h"
#include "cmValue.h"
#include "cmake.h"

std::string cmGeneratorExpressionNode::EvaluateDependentExpression(
  std::string const& prop, cmLocalGenerator* lg,
  cmGeneratorExpressionContext* context, cmGeneratorTarget const* headTarget,
  cmGeneratorExpressionDAGChecker* dagChecker,
  cmGeneratorTarget const* currentTarget)
{
  cmGeneratorExpression ge(context->Backtrace);
  std::unique_ptr<cmCompiledGeneratorExpression> cge = ge.Parse(prop);
  cge->SetEvaluateForBuildsystem(context->EvaluateForBuildsystem);
  cge->SetQuiet(context->Quiet);
  std::string result =
    cge->Evaluate(lg, context->Config, headTarget, dagChecker, currentTarget,
                  context->Language);
  if (cge->GetHadContextSensitiveCondition()) {
    context->HadContextSensitiveCondition = true;
  }
  if (cge->GetHadHeadSensitiveCondition()) {
    context->HadHeadSensitiveCondition = true;
  }
  if (cge->GetHadLinkLanguageSensitiveCondition()) {
    context->HadLinkLanguageSensitiveCondition = true;
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

struct BooleanOpNode : public cmGeneratorExpressionNode
{
  BooleanOpNode(const char* op_, const char* successVal_,
                const char* failureVal_)
    : op(op_)
    , successVal(successVal_)
    , failureVal(failureVal_)
  {
  }

  int NumExpectedParameters() const override { return OneOrMoreParameters; }

  std::string Evaluate(const std::vector<std::string>& parameters,
                       cmGeneratorExpressionContext* context,
                       const GeneratorExpressionContent* content,
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
        reportError(context, content->GetOriginalExpression(), e.str());
        return std::string();
      }
    }
    return this->successVal;
  }

  const char *const op, *const successVal, *const failureVal;
};

static const BooleanOpNode andNode("AND", "1", "0"), orNode("OR", "0", "1");

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
    return !cmIsOff(parameters.front()) ? "1" : "0";
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
    long numbers[2];
    for (int i = 0; i < 2; ++i) {
      if (!ParameterToLong(parameters[i].c_str(), &numbers[i])) {
        reportError(context, content->GetOriginalExpression(),
                    "$<EQUAL> parameter " + parameters[i] +
                      " is not a valid integer.");
        return {};
      }
    }
    return numbers[0] == numbers[1] ? "1" : "0";
  }

  static bool ParameterToLong(const char* param, long* outResult)
  {
    const char isNegative = param[0] == '-';

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
    const std::vector<std::string>& parameters,
    cmGeneratorExpressionContext* context,
    const GeneratorExpressionContent* /*content*/,
    cmGeneratorExpressionDAGChecker* /*dagChecker*/) const override
  {
    std::vector<std::string> values;
    std::vector<std::string> checkValues;
    bool check = false;
    switch (context->LG->GetPolicyStatus(cmPolicies::CMP0085)) {
      case cmPolicies::WARN:
        if (parameters.front().empty()) {
          check = true;
          cmExpandList(parameters[1], checkValues, true);
        }
        CM_FALLTHROUGH;
      case cmPolicies::OLD:
        cmExpandList(parameters[1], values);
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
        cmExpandList(parameters[1], values, true);
        break;
    }

    return cm::contains(values, parameters.front()) ? "1" : "0";
  }
} inListNode;

static const struct FilterNode : public cmGeneratorExpressionNode
{
  FilterNode() {} // NOLINT(modernize-use-equals-default)

  int NumExpectedParameters() const override { return 3; }

  std::string Evaluate(
    const std::vector<std::string>& parameters,
    cmGeneratorExpressionContext* context,
    const GeneratorExpressionContent* content,
    cmGeneratorExpressionDAGChecker* /*dagChecker*/) const override
  {
    if (parameters.size() != 3) {
      reportError(context, content->GetOriginalExpression(),
                  "$<FILTER:...> expression requires three parameters");
      return {};
    }

    if (parameters[1] != "INCLUDE" && parameters[1] != "EXCLUDE") {
      reportError(
        context, content->GetOriginalExpression(),
        "$<FILTER:...> second parameter must be either INCLUDE or EXCLUDE");
      return {};
    }

    const bool exclude = parameters[1] == "EXCLUDE";

    cmsys::RegularExpression re;
    if (!re.compile(parameters[2])) {
      reportError(context, content->GetOriginalExpression(),
                  "$<FILTER:...> failed to compile regex");
      return {};
    }

    std::vector<std::string> values;
    std::vector<std::string> result;
    cmExpandList(parameters.front(), values, true);

    std::copy_if(values.cbegin(), values.cend(), std::back_inserter(result),
                 [&re, exclude](std::string const& input) {
                   return exclude ^ re.find(input);
                 });
    return cmJoin(cmMakeRange(result.cbegin(), result.cend()), ";");
  }
} filterNode;

static const struct RemoveDuplicatesNode : public cmGeneratorExpressionNode
{
  RemoveDuplicatesNode() {} // NOLINT(modernize-use-equals-default)

  int NumExpectedParameters() const override { return 1; }

  std::string Evaluate(
    const std::vector<std::string>& parameters,
    cmGeneratorExpressionContext* context,
    const GeneratorExpressionContent* content,
    cmGeneratorExpressionDAGChecker* /*dagChecker*/) const override
  {
    if (parameters.size() != 1) {
      reportError(
        context, content->GetOriginalExpression(),
        "$<REMOVE_DUPLICATES:...> expression requires one parameter");
    }

    std::vector<std::string> values = cmExpandedList(parameters.front(), true);

    auto valuesEnd = cmRemoveDuplicates(values);
    auto valuesBegin = values.cbegin();
    return cmJoin(cmMakeRange(valuesBegin, valuesEnd), ";");
  }

} removeDuplicatesNode;

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

    std::string const& targetName = parameters.front();
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

    std::string const& targetName = parameters.front();
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
        expression, context->LG, context, context->HeadTarget, &dagChecker,
        context->CurrentTarget);
    }

    return this->EvaluateDependentExpression(
      expression, context->LG, context, context->HeadTarget, dagCheckerParent,
      context->CurrentTarget);
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

  Range(const Container& container)
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

bool CheckPathParametersEx(cmGeneratorExpressionContext* ctx,
                           const GeneratorExpressionContent* cnt,
                           cm::string_view option, std::size_t count,
                           int required = 1, bool exactly = true)
{
  if (static_cast<int>(count) < required ||
      (exactly && static_cast<int>(count) > required)) {
    reportError(ctx, cnt->GetOriginalExpression(),
                cmStrCat("$<PATH:", option, "> expression requires ",
                         (exactly ? "exactly" : "at least"), ' ',
                         (required == 1 ? "one parameter" : "two parameters"),
                         '.'));
    return false;
  }
  return true;
};
bool CheckPathParameters(cmGeneratorExpressionContext* ctx,
                         const GeneratorExpressionContent* cnt,
                         cm::string_view option, const Arguments& args,
                         int required = 1)
{
  return CheckPathParametersEx(ctx, cnt, option, args.size(), required);
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
    const std::vector<std::string>& parameters,
    cmGeneratorExpressionContext* context,
    const GeneratorExpressionContent* content,
    cmGeneratorExpressionDAGChecker* /*dagChecker*/) const override
  {
    static std::unordered_map<
      cm::string_view,
      std::function<std::string(cmGeneratorExpressionContext*,
                                const GeneratorExpressionContent*,
                                Arguments&)>>
      pathCommands{
        { "GET_ROOT_NAME"_s,
          [](cmGeneratorExpressionContext* ctx,
             const GeneratorExpressionContent* cnt,
             Arguments& args) -> std::string {
            return CheckPathParameters(ctx, cnt, "GET_ROOT_NAME"_s, args) &&
                !args.front().empty()
              ? cmCMakePath{ args.front() }.GetRootName().String()
              : std::string{};
          } },
        { "GET_ROOT_DIRECTORY"_s,
          [](cmGeneratorExpressionContext* ctx,
             const GeneratorExpressionContent* cnt,
             Arguments& args) -> std::string {
            return CheckPathParameters(ctx, cnt, "GET_ROOT_DIRECTORY"_s,
                                       args) &&
                !args.front().empty()
              ? cmCMakePath{ args.front() }.GetRootDirectory().String()
              : std::string{};
          } },
        { "GET_ROOT_PATH"_s,
          [](cmGeneratorExpressionContext* ctx,
             const GeneratorExpressionContent* cnt,
             Arguments& args) -> std::string {
            return CheckPathParameters(ctx, cnt, "GET_ROOT_PATH"_s, args) &&
                !args.front().empty()
              ? cmCMakePath{ args.front() }.GetRootPath().String()
              : std::string{};
          } },
        { "GET_FILENAME"_s,
          [](cmGeneratorExpressionContext* ctx,
             const GeneratorExpressionContent* cnt,
             Arguments& args) -> std::string {
            return CheckPathParameters(ctx, cnt, "GET_FILENAME"_s, args) &&
                !args.front().empty()
              ? cmCMakePath{ args.front() }.GetFileName().String()
              : std::string{};
          } },
        { "GET_EXTENSION"_s,
          [](cmGeneratorExpressionContext* ctx,
             const GeneratorExpressionContent* cnt,
             Arguments& args) -> std::string {
            bool lastOnly = args.front() == "LAST_ONLY"_s;
            if (lastOnly) {
              args.advance(1);
            }
            if (CheckPathParametersEx(ctx, cnt,
                                      lastOnly ? "GET_EXTENSION,LAST_ONLY"_s
                                               : "GET_EXTENSION"_s,
                                      args.size())) {
              if (args.front().empty()) {
                return std::string{};
              }
              return lastOnly
                ? cmCMakePath{ args.front() }.GetExtension().String()
                : cmCMakePath{ args.front() }.GetWideExtension().String();
            }
            return std::string{};
          } },
        { "GET_STEM"_s,
          [](cmGeneratorExpressionContext* ctx,
             const GeneratorExpressionContent* cnt,
             Arguments& args) -> std::string {
            bool lastOnly = args.front() == "LAST_ONLY"_s;
            if (lastOnly) {
              args.advance(1);
            }
            if (CheckPathParametersEx(
                  ctx, cnt, lastOnly ? "GET_STEM,LAST_ONLY"_s : "GET_STEM"_s,
                  args.size())) {
              if (args.front().empty()) {
                return std::string{};
              }
              return lastOnly
                ? cmCMakePath{ args.front() }.GetStem().String()
                : cmCMakePath{ args.front() }.GetNarrowStem().String();
            }
            return std::string{};
          } },
        { "GET_RELATIVE_PART"_s,
          [](cmGeneratorExpressionContext* ctx,
             const GeneratorExpressionContent* cnt,
             Arguments& args) -> std::string {
            return CheckPathParameters(ctx, cnt, "GET_RELATIVE_PART"_s,
                                       args) &&
                !args.front().empty()
              ? cmCMakePath{ args.front() }.GetRelativePath().String()
              : std::string{};
          } },
        { "GET_PARENT_PATH"_s,
          [](cmGeneratorExpressionContext* ctx,
             const GeneratorExpressionContent* cnt,
             Arguments& args) -> std::string {
            return CheckPathParameters(ctx, cnt, "GET_PARENT_PATH"_s, args)
              ? cmCMakePath{ args.front() }.GetParentPath().String()
              : std::string{};
          } },
        { "HAS_ROOT_NAME"_s,
          [](cmGeneratorExpressionContext* ctx,
             const GeneratorExpressionContent* cnt,
             Arguments& args) -> std::string {
            return CheckPathParameters(ctx, cnt, "HAS_ROOT_NAME"_s, args)
              ? ToString(cmCMakePath{ args.front() }.HasRootName())
              : std::string{ "0" };
          } },
        { "HAS_ROOT_DIRECTORY"_s,
          [](cmGeneratorExpressionContext* ctx,
             const GeneratorExpressionContent* cnt,
             Arguments& args) -> std::string {
            return CheckPathParameters(ctx, cnt, "HAS_ROOT_DIRECTORY"_s, args)
              ? ToString(cmCMakePath{ args.front() }.HasRootDirectory())
              : std::string{ "0" };
          } },
        { "HAS_ROOT_PATH"_s,
          [](cmGeneratorExpressionContext* ctx,
             const GeneratorExpressionContent* cnt,
             Arguments& args) -> std::string {
            return CheckPathParameters(ctx, cnt, "HAS_ROOT_PATH"_s, args)
              ? ToString(cmCMakePath{ args.front() }.HasRootPath())
              : std::string{ "0" };
          } },
        { "HAS_FILENAME"_s,
          [](cmGeneratorExpressionContext* ctx,
             const GeneratorExpressionContent* cnt,
             Arguments& args) -> std::string {
            return CheckPathParameters(ctx, cnt, "HAS_FILENAME"_s, args)
              ? ToString(cmCMakePath{ args.front() }.HasFileName())
              : std::string{ "0" };
          } },
        { "HAS_EXTENSION"_s,
          [](cmGeneratorExpressionContext* ctx,
             const GeneratorExpressionContent* cnt,
             Arguments& args) -> std::string {
            return CheckPathParameters(ctx, cnt, "HAS_EXTENSION"_s, args) &&
                !args.front().empty()
              ? ToString(cmCMakePath{ args.front() }.HasExtension())
              : std::string{ "0" };
          } },
        { "HAS_STEM"_s,
          [](cmGeneratorExpressionContext* ctx,
             const GeneratorExpressionContent* cnt,
             Arguments& args) -> std::string {
            return CheckPathParameters(ctx, cnt, "HAS_STEM"_s, args)
              ? ToString(cmCMakePath{ args.front() }.HasStem())
              : std::string{ "0" };
          } },
        { "HAS_RELATIVE_PART"_s,
          [](cmGeneratorExpressionContext* ctx,
             const GeneratorExpressionContent* cnt,
             Arguments& args) -> std::string {
            return CheckPathParameters(ctx, cnt, "HAS_RELATIVE_PART"_s, args)
              ? ToString(cmCMakePath{ args.front() }.HasRelativePath())
              : std::string{ "0" };
          } },
        { "HAS_PARENT_PATH"_s,
          [](cmGeneratorExpressionContext* ctx,
             const GeneratorExpressionContent* cnt,
             Arguments& args) -> std::string {
            return CheckPathParameters(ctx, cnt, "HAS_PARENT_PATH"_s, args)
              ? ToString(cmCMakePath{ args.front() }.HasParentPath())
              : std::string{ "0" };
          } },
        { "IS_ABSOLUTE"_s,
          [](cmGeneratorExpressionContext* ctx,
             const GeneratorExpressionContent* cnt,
             Arguments& args) -> std::string {
            return CheckPathParameters(ctx, cnt, "IS_ABSOLUTE"_s, args)
              ? ToString(cmCMakePath{ args.front() }.IsAbsolute())
              : std::string{ "0" };
          } },
        { "IS_RELATIVE"_s,
          [](cmGeneratorExpressionContext* ctx,
             const GeneratorExpressionContent* cnt,
             Arguments& args) -> std::string {
            return CheckPathParameters(ctx, cnt, "IS_RELATIVE"_s, args)
              ? ToString(cmCMakePath{ args.front() }.IsRelative())
              : std::string{ "0" };
          } },
        { "IS_PREFIX"_s,
          [](cmGeneratorExpressionContext* ctx,
             const GeneratorExpressionContent* cnt,
             Arguments& args) -> std::string {
            bool normalize = args.front() == "NORMALIZE"_s;
            if (normalize) {
              args.advance(1);
            }
            if (CheckPathParametersEx(ctx, cnt,
                                      normalize ? "IS_PREFIX,NORMALIZE"_s
                                                : "IS_PREFIX"_s,
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
          [](cmGeneratorExpressionContext* ctx,
             const GeneratorExpressionContent* cnt,
             Arguments& args) -> std::string {
            bool normalize = args.front() == "NORMALIZE"_s;
            if (normalize) {
              args.advance(1);
            }
            if (CheckPathParametersEx(ctx, cnt,
                                      normalize ? "CMAKE_PATH,NORMALIZE"_s
                                                : "CMAKE_PATH"_s,
                                      args.size(), 1)) {
              auto path =
                cmCMakePath{ args.front(), cmCMakePath::auto_format };
              return normalize ? path.Normal().GenericString()
                               : path.GenericString();
            }
            return std::string{};
          } },
        { "APPEND"_s,
          [](cmGeneratorExpressionContext* ctx,
             const GeneratorExpressionContent* cnt,
             Arguments& args) -> std::string {
            if (CheckPathParametersEx(ctx, cnt, "APPEND"_s, args.size(), 1,
                                      false)) {
              cmCMakePath path;
              for (const auto& p : args) {
                path /= p;
              }
              return path.String();
            }
            return std::string{};
          } },
        { "REMOVE_FILENAME"_s,
          [](cmGeneratorExpressionContext* ctx,
             const GeneratorExpressionContent* cnt,
             Arguments& args) -> std::string {
            return CheckPathParameters(ctx, cnt, "REMOVE_FILENAME"_s, args) &&
                !args.front().empty()
              ? cmCMakePath{ args.front() }.RemoveFileName().String()
              : std::string{};
          } },
        { "REPLACE_FILENAME"_s,
          [](cmGeneratorExpressionContext* ctx,
             const GeneratorExpressionContent* cnt,
             Arguments& args) -> std::string {
            return CheckPathParameters(ctx, cnt, "REPLACE_FILENAME"_s, args, 2)
              ? cmCMakePath{ args[0] }
                  .ReplaceFileName(cmCMakePath{ args[1] })
                  .String()
              : std::string{};
          } },
        { "REMOVE_EXTENSION"_s,
          [](cmGeneratorExpressionContext* ctx,
             const GeneratorExpressionContent* cnt,
             Arguments& args) -> std::string {
            bool lastOnly = args.front() == "LAST_ONLY"_s;
            if (lastOnly) {
              args.advance(1);
            }
            if (CheckPathParametersEx(ctx, cnt,
                                      lastOnly ? "REMOVE_EXTENSION,LAST_ONLY"_s
                                               : "REMOVE_EXTENSION"_s,
                                      args.size())) {
              if (args.front().empty()) {
                return std::string{};
              }
              return lastOnly
                ? cmCMakePath{ args.front() }.RemoveExtension().String()
                : cmCMakePath{ args.front() }.RemoveWideExtension().String();
            }
            return std::string{};
          } },
        { "REPLACE_EXTENSION"_s,
          [](cmGeneratorExpressionContext* ctx,
             const GeneratorExpressionContent* cnt,
             Arguments& args) -> std::string {
            bool lastOnly = args.front() == "LAST_ONLY"_s;
            if (lastOnly) {
              args.advance(1);
            }
            if (CheckPathParametersEx(ctx, cnt,
                                      lastOnly
                                        ? "REPLACE_EXTENSION,LAST_ONLY"_s
                                        : "REPLACE_EXTENSION"_s,
                                      args.size(), 2)) {
              if (lastOnly) {
                return cmCMakePath{ args[0] }
                  .ReplaceExtension(cmCMakePath{ args[1] })
                  .String();
              }
              return cmCMakePath{ args[0] }
                .ReplaceWideExtension(cmCMakePath{ args[1] })
                .String();
            }
            return std::string{};
          } },
        { "NORMAL_PATH"_s,
          [](cmGeneratorExpressionContext* ctx,
             const GeneratorExpressionContent* cnt,
             Arguments& args) -> std::string {
            return CheckPathParameters(ctx, cnt, "NORMAL_PATH"_s, args) &&
                !args.front().empty()
              ? cmCMakePath{ args.front() }.Normal().String()
              : std::string{};
          } },
        { "RELATIVE_PATH"_s,
          [](cmGeneratorExpressionContext* ctx,
             const GeneratorExpressionContent* cnt,
             Arguments& args) -> std::string {
            return CheckPathParameters(ctx, cnt, "RELATIVE_PATH"_s, args, 2)
              ? cmCMakePath{ args[0] }.Relative(args[1]).String()
              : std::string{};
          } },
        { "ABSOLUTE_PATH"_s,
          [](cmGeneratorExpressionContext* ctx,
             const GeneratorExpressionContent* cnt,
             Arguments& args) -> std::string {
            bool normalize = args.front() == "NORMALIZE"_s;
            if (normalize) {
              args.advance(1);
            }
            if (CheckPathParametersEx(ctx, cnt,
                                      normalize ? "ABSOLUTE_PATH,NORMALIZE"_s
                                                : "ABSOLUTE_PATH"_s,
                                      args.size(), 2)) {
              auto path = cmCMakePath{ args[0] }.Absolute(args[1]);
              return normalize ? path.Normal().String() : path.String();
            }
            return std::string{};
          } }
      };

    if (cm::contains(pathCommands, parameters.front())) {
      auto args = Arguments{ parameters }.advance(1);
      return pathCommands[parameters.front()](context, content, args);
    }

    reportError(context, content->GetOriginalExpression(),
                cmStrCat(parameters.front(), ": invalid option."));
    return std::string{};
  }
} pathNode;

static const struct PathEqualNode : public cmGeneratorExpressionNode
{
  PathEqualNode() {} // NOLINT(modernize-use-equals-default)

  int NumExpectedParameters() const override { return 2; }

  std::string Evaluate(
    const std::vector<std::string>& parameters,
    cmGeneratorExpressionContext* /*context*/,
    const GeneratorExpressionContent* /*content*/,
    cmGeneratorExpressionDAGChecker* /*dagChecker*/) const override
  {
    return cmCMakePath{ parameters[0] } == cmCMakePath{ parameters[1] } ? "1"
                                                                        : "0";
  }
} pathEqualNode;

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

template <char C>
struct CharacterNode : public cmGeneratorExpressionNode
{
  CharacterNode() {} // NOLINT(modernize-use-equals-default)

  int NumExpectedParameters() const override { return 0; }

  std::string Evaluate(
    const std::vector<std::string>& /*parameters*/,
    cmGeneratorExpressionContext* /*context*/,
    const GeneratorExpressionContent* /*content*/,
    cmGeneratorExpressionDAGChecker* /*dagChecker*/) const override
  {
    return { C };
  }
};
static const CharacterNode<'>'> angle_rNode;
static const CharacterNode<','> commaNode;
static const CharacterNode<';'> semicolonNode;

struct CompilerIdNode : public cmGeneratorExpressionNode
{
  CompilerIdNode(const char* compilerLang)
    : CompilerLanguage(compilerLang)
  {
  }

  int NumExpectedParameters() const override { return ZeroOrMoreParameters; }

  std::string Evaluate(
    const std::vector<std::string>& parameters,
    cmGeneratorExpressionContext* context,
    const GeneratorExpressionContent* content,
    cmGeneratorExpressionDAGChecker* dagChecker) const override
  {
    if (!context->HeadTarget) {
      std::ostringstream e;
      e << "$<" << this->CompilerLanguage
        << "_COMPILER_ID> may only be used with binary targets.  It may "
           "not be used with add_custom_command or add_custom_target.";
      reportError(context, content->GetOriginalExpression(), e.str());
      return {};
    }
    return this->EvaluateWithLanguage(parameters, context, content, dagChecker,
                                      this->CompilerLanguage);
  }

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
    if (compilerId.empty()) {
      return parameters.front().empty() ? "1" : "0";
    }
    static cmsys::RegularExpression compilerIdValidator("^[A-Za-z0-9_]*$");

    for (auto const& param : parameters) {

      if (!compilerIdValidator.find(param)) {
        reportError(context, content->GetOriginalExpression(),
                    "Expression syntax not recognized.");
        return std::string();
      }

      if (strcmp(param.c_str(), compilerId.c_str()) == 0) {
        return "1";
      }

      if (cmsysString_strcasecmp(param.c_str(), compilerId.c_str()) == 0) {
        switch (context->LG->GetPolicyStatus(cmPolicies::CMP0044)) {
          case cmPolicies::WARN: {
            context->LG->GetCMakeInstance()->IssueMessage(
              MessageType::AUTHOR_WARNING,
              cmPolicies::GetPolicyWarning(cmPolicies::CMP0044),
              context->Backtrace);
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
    }
    return "0";
  }

  const char* const CompilerLanguage;
};

static const CompilerIdNode cCompilerIdNode("C"), cxxCompilerIdNode("CXX"),
  cudaCompilerIdNode("CUDA"), objcCompilerIdNode("OBJC"),
  objcxxCompilerIdNode("OBJCXX"), fortranCompilerIdNode("Fortran"),
  hipCompilerIdNode("HIP"), ispcCompilerIdNode("ISPC");

struct CompilerVersionNode : public cmGeneratorExpressionNode
{
  CompilerVersionNode(const char* compilerLang)
    : CompilerLanguage(compilerLang)
  {
  }

  int NumExpectedParameters() const override { return OneOrZeroParameters; }

  std::string Evaluate(
    const std::vector<std::string>& parameters,
    cmGeneratorExpressionContext* context,
    const GeneratorExpressionContent* content,
    cmGeneratorExpressionDAGChecker* dagChecker) const override
  {
    if (!context->HeadTarget) {
      std::ostringstream e;
      e << "$<" << this->CompilerLanguage
        << "_COMPILER_VERSION> may only be used with binary targets.  It "
           "may not be used with add_custom_command or add_custom_target.";
      reportError(context, content->GetOriginalExpression(), e.str());
      return {};
    }
    return this->EvaluateWithLanguage(parameters, context, content, dagChecker,
                                      this->CompilerLanguage);
  }

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

  const char* const CompilerLanguage;
};

static const CompilerVersionNode cCompilerVersionNode("C"),
  cxxCompilerVersionNode("CXX"), cudaCompilerVersionNode("CUDA"),
  objcCompilerVersionNode("OBJC"), objcxxCompilerVersionNode("OBJCXX"),
  fortranCompilerVersionNode("Fortran"), ispcCompilerVersionNode("ISPC"),
  hipCompilerVersionNode("HIP");

struct PlatformIdNode : public cmGeneratorExpressionNode
{
  PlatformIdNode() {} // NOLINT(modernize-use-equals-default)

  int NumExpectedParameters() const override { return ZeroOrMoreParameters; }

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
    const std::vector<std::string>& parameters,
    cmGeneratorExpressionContext* /*context*/,
    const GeneratorExpressionContent* /*content*/,
    cmGeneratorExpressionDAGChecker* /*dagChecker*/) const override
  {
    return cmSystemTools::VersionCompare(Op, parameters.front(), parameters[1])
      ? "1"
      : "0";
  }
};

static const VersionNode<cmSystemTools::OP_GREATER> versionGreaterNode;
static const VersionNode<cmSystemTools::OP_GREATER_EQUAL> versionGreaterEqNode;
static const VersionNode<cmSystemTools::OP_LESS> versionLessNode;
static const VersionNode<cmSystemTools::OP_LESS_EQUAL> versionLessEqNode;
static const VersionNode<cmSystemTools::OP_EQUAL> versionEqualNode;

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

  int NumExpectedParameters() const override { return ZeroOrMoreParameters; }

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
    for (auto const& param : parameters) {
      if (context->Config.empty()) {
        if (param.empty()) {
          return "1";
        }
      } else if (cmsysString_strcasecmp(param.c_str(),
                                        context->Config.c_str()) == 0) {
        return "1";
      }
    }

    if (context->CurrentTarget && context->CurrentTarget->IsImported()) {
      cmValue loc = nullptr;
      cmValue imp = nullptr;
      std::string suffix;
      if (context->CurrentTarget->Target->GetMappedConfig(context->Config, loc,
                                                          imp, suffix)) {
        // This imported target has an appropriate location
        // for this (possibly mapped) config.
        // Check if there is a proper config mapping for the tested config.
        std::vector<std::string> mappedConfigs;
        std::string mapProp = cmStrCat(
          "MAP_IMPORTED_CONFIG_", cmSystemTools::UpperCase(context->Config));
        if (cmValue mapValue = context->CurrentTarget->GetProperty(mapProp)) {
          cmExpandList(cmSystemTools::UpperCase(*mapValue), mappedConfigs);

          for (auto const& param : parameters) {
            if (cm::contains(mappedConfigs, cmSystemTools::UpperCase(param))) {
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
    const std::vector<std::string>& parameters,
    cmGeneratorExpressionContext* /*context*/,
    const GeneratorExpressionContent* /*content*/,
    cmGeneratorExpressionDAGChecker* /*dagChecker*/) const override
  {
    std::vector<std::string> list = cmExpandedList(parameters.front());
    return cmJoin(list, parameters[1]);
  }
} joinNode;

static const struct CompileLanguageNode : public cmGeneratorExpressionNode
{
  CompileLanguageNode() {} // NOLINT(modernize-use-equals-default)

  int NumExpectedParameters() const override { return ZeroOrMoreParameters; }

  std::string Evaluate(
    const std::vector<std::string>& parameters,
    cmGeneratorExpressionContext* context,
    const GeneratorExpressionContent* content,
    cmGeneratorExpressionDAGChecker* dagChecker) const override
  {
    if (context->Language.empty() &&
        (!dagChecker || !dagChecker->EvaluatingCompileExpression())) {
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

    for (auto const& param : parameters) {
      if (context->Language == param) {
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
    const std::vector<std::string>& parameters,
    cmGeneratorExpressionContext* context,
    const GeneratorExpressionContent* content,
    cmGeneratorExpressionDAGChecker* dagChecker) const override
  {
    if (!context->HeadTarget ||
        (context->Language.empty() &&
         (!dagChecker || !dagChecker->EvaluatingCompileExpression()))) {
      // reportError(context, content->GetOriginalExpression(), "");
      reportError(
        context, content->GetOriginalExpression(),
        "$<COMPILE_LANG_AND_ID:lang,id> may only be used with binary targets "
        "to specify include directories, compile definitions, and compile "
        "options.  It may not be used with the add_custom_command, "
        "add_custom_target, or file(GENERATE) commands.");
      return std::string();
    }
    cmGlobalGenerator* gg = context->LG->GetGlobalGenerator();
    std::string genName = gg->GetName();
    if (genName.find("Makefiles") == std::string::npos &&
        genName.find("Ninja") == std::string::npos &&
        genName.find("Visual Studio") == std::string::npos &&
        genName.find("Xcode") == std::string::npos &&
        genName.find("Watcom WMake") == std::string::npos) {
      reportError(
        context, content->GetOriginalExpression(),
        "$<COMPILE_LANG_AND_ID:lang,id> not supported for this generator.");
      return std::string();
    }

    const std::string& lang = context->Language;
    if (lang == parameters.front()) {
      std::vector<std::string> idParameter((parameters.cbegin() + 1),
                                           parameters.cend());
      return CompilerIdNode{ lang.c_str() }.EvaluateWithLanguage(
        idParameter, context, content, dagChecker, lang);
    }
    return "0";
  }
} languageAndIdNode;

static const struct LinkLanguageNode : public cmGeneratorExpressionNode
{
  LinkLanguageNode() {} // NOLINT(modernize-use-equals-default)

  int NumExpectedParameters() const override { return ZeroOrMoreParameters; }

  std::string Evaluate(
    const std::vector<std::string>& parameters,
    cmGeneratorExpressionContext* context,
    const GeneratorExpressionContent* content,
    cmGeneratorExpressionDAGChecker* dagChecker) const override
  {
    if (!context->HeadTarget || !dagChecker ||
        !(dagChecker->EvaluatingLinkExpression() ||
          dagChecker->EvaluatingLinkLibraries())) {
      reportError(context, content->GetOriginalExpression(),
                  "$<LINK_LANGUAGE:...> may only be used with binary targets "
                  "to specify link libraries, link directories, link options "
                  "and link depends.");
      return std::string();
    }
    if (dagChecker->EvaluatingLinkLibraries() && parameters.empty()) {
      reportError(
        context, content->GetOriginalExpression(),
        "$<LINK_LANGUAGE> is not supported in link libraries expression.");
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
                  "$<LINK_LANGUAGE:...> not supported for this generator.");
      return std::string();
    }

    if (dagChecker->EvaluatingLinkLibraries()) {
      context->HadHeadSensitiveCondition = true;
      context->HadLinkLanguageSensitiveCondition = true;
    }

    if (parameters.empty()) {
      return context->Language;
    }

    for (auto const& param : parameters) {
      if (context->Language == param) {
        return "1";
      }
    }
    return "0";
  }
} linkLanguageNode;

namespace {
struct LinkerId
{
  static std::string Evaluate(const std::vector<std::string>& parameters,
                              cmGeneratorExpressionContext* context,
                              const GeneratorExpressionContent* content,
                              const std::string& lang)
  {
    std::string const& linkerId =
      context->LG->GetMakefile()->GetSafeDefinition("CMAKE_" + lang +
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
        reportError(context, content->GetOriginalExpression(),
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
    const std::vector<std::string>& parameters,
    cmGeneratorExpressionContext* context,
    const GeneratorExpressionContent* content,
    cmGeneratorExpressionDAGChecker* dagChecker) const override
  {
    if (!context->HeadTarget || !dagChecker ||
        !(dagChecker->EvaluatingLinkExpression() ||
          dagChecker->EvaluatingLinkLibraries())) {
      reportError(
        context, content->GetOriginalExpression(),
        "$<LINK_LANG_AND_ID:lang,id> may only be used with binary targets "
        "to specify link libraries, link directories, link options, and link "
        "depends.");
      return std::string();
    }

    cmGlobalGenerator* gg = context->LG->GetGlobalGenerator();
    std::string genName = gg->GetName();
    if (genName.find("Makefiles") == std::string::npos &&
        genName.find("Ninja") == std::string::npos &&
        genName.find("Visual Studio") == std::string::npos &&
        genName.find("Xcode") == std::string::npos &&
        genName.find("Watcom WMake") == std::string::npos) {
      reportError(
        context, content->GetOriginalExpression(),
        "$<LINK_LANG_AND_ID:lang,id> not supported for this generator.");
      return std::string();
    }

    if (dagChecker->EvaluatingLinkLibraries()) {
      context->HadHeadSensitiveCondition = true;
      context->HadLinkLanguageSensitiveCondition = true;
    }

    const std::string& lang = context->Language;
    if (lang == parameters.front()) {
      std::vector<std::string> idParameter((parameters.cbegin() + 1),
                                           parameters.cend());
      return LinkerId::Evaluate(idParameter, context, content, lang);
    }
    return "0";
  }
} linkLanguageAndIdNode;

static const struct LinkLibraryNode : public cmGeneratorExpressionNode
{
  LinkLibraryNode() {} // NOLINT(modernize-use-equals-default)

  int NumExpectedParameters() const override { return OneOrMoreParameters; }

  std::string Evaluate(
    const std::vector<std::string>& parameters,
    cmGeneratorExpressionContext* context,
    const GeneratorExpressionContent* content,
    cmGeneratorExpressionDAGChecker* dagChecker) const override
  {
    using ForGenex = cmGeneratorExpressionDAGChecker::ForGenex;

    if (!context->HeadTarget || !dagChecker ||
        !dagChecker->EvaluatingLinkLibraries(nullptr,
                                             ForGenex::LINK_LIBRARY)) {
      reportError(context, content->GetOriginalExpression(),
                  "$<LINK_LIBRARY:...> may only be used with binary targets "
                  "to specify link libraries through 'LINK_LIBRARIES', "
                  "'INTERFACE_LINK_LIBRARIES', and "
                  "'INTERFACE_LINK_LIBRARIES_DIRECT' properties.");
      return std::string();
    }

    std::vector<std::string> list;
    cmExpandLists(parameters.begin(), parameters.end(), list);
    if (list.empty()) {
      reportError(
        context, content->GetOriginalExpression(),
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
      reportError(context, content->GetOriginalExpression(),
                  cmStrCat("The feature name '", feature,
                           "' contains invalid characters."));
      return std::string();
    }

    const auto LL_BEGIN = cmStrCat("<LINK_LIBRARY:", feature, '>');
    const auto LL_END = cmStrCat("</LINK_LIBRARY:", feature, '>');

    // filter out $<LINK_LIBRARY:..> tags with same feature
    // and raise an error for any different feature
    cm::erase_if(list, [&](const std::string& item) -> bool {
      return item == LL_BEGIN || item == LL_END;
    });
    auto it =
      std::find_if(list.cbegin() + 1, list.cend(),
                   [&feature](const std::string& item) -> bool {
                     return cmHasPrefix(item, "<LINK_LIBRARY:"_s) &&
                       item.substr(14, item.find('>', 14) - 14) != feature;
                   });
    if (it != list.cend()) {
      reportError(
        context, content->GetOriginalExpression(),
        "$<LINK_LIBRARY:...> with different features cannot be nested.");
      return std::string();
    }
    // $<LINK_GROUP:...> must not appear as part of $<LINK_LIBRARY:...>
    it = std::find_if(list.cbegin() + 1, list.cend(),
                      [](const std::string& item) -> bool {
                        return cmHasPrefix(item, "<LINK_GROUP:"_s);
                      });
    if (it != list.cend()) {
      reportError(context, content->GetOriginalExpression(),
                  "$<LINK_GROUP:...> cannot be nested inside a "
                  "$<LINK_LIBRARY:...> expression.");
      return std::string();
    }

    list.front() = LL_BEGIN;
    list.push_back(LL_END);

    return cmJoin(list, ";"_s);
  }
} linkLibraryNode;

static const struct LinkGroupNode : public cmGeneratorExpressionNode
{
  LinkGroupNode() {} // NOLINT(modernize-use-equals-default)

  int NumExpectedParameters() const override { return OneOrMoreParameters; }

  std::string Evaluate(
    const std::vector<std::string>& parameters,
    cmGeneratorExpressionContext* context,
    const GeneratorExpressionContent* content,
    cmGeneratorExpressionDAGChecker* dagChecker) const override
  {
    using ForGenex = cmGeneratorExpressionDAGChecker::ForGenex;

    if (!context->HeadTarget || !dagChecker ||
        !dagChecker->EvaluatingLinkLibraries(nullptr, ForGenex::LINK_GROUP)) {
      reportError(
        context, content->GetOriginalExpression(),
        "$<LINK_GROUP:...> may only be used with binary targets "
        "to specify group of link libraries through 'LINK_LIBRARIES', "
        "'INTERFACE_LINK_LIBRARIES', and "
        "'INTERFACE_LINK_LIBRARIES_DIRECT' properties.");
      return std::string();
    }

    std::vector<std::string> list;
    cmExpandLists(parameters.begin(), parameters.end(), list);
    if (list.empty()) {
      reportError(
        context, content->GetOriginalExpression(),
        "$<LINK_GROUP:...> expects a feature name as first argument.");
      return std::string();
    }
    // $<LINK_GROUP:..> cannot be nested
    if (std::find_if(list.cbegin(), list.cend(),
                     [](const std::string& item) -> bool {
                       return cmHasPrefix(item, "<LINK_GROUP"_s);
                     }) != list.cend()) {
      reportError(context, content->GetOriginalExpression(),
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
      reportError(context, content->GetOriginalExpression(),
                  cmStrCat("The feature name '", feature,
                           "' contains invalid characters."));
      return std::string();
    }

    const auto LG_BEGIN = cmStrCat(
      "<LINK_GROUP:", feature, ':',
      cmJoin(cmRange<decltype(list.cbegin())>(list.cbegin() + 1, list.cend()),
             "|"_s),
      '>');
    const auto LG_END = cmStrCat("</LINK_GROUP:", feature, '>');

    list.front() = LG_BEGIN;
    list.push_back(LG_END);

    return cmJoin(list, ";"_s);
  }
} linkGroupNode;

static const struct HostLinkNode : public cmGeneratorExpressionNode
{
  HostLinkNode() {} // NOLINT(modernize-use-equals-default)

  int NumExpectedParameters() const override { return ZeroOrMoreParameters; }

  std::string Evaluate(
    const std::vector<std::string>& parameters,
    cmGeneratorExpressionContext* context,
    const GeneratorExpressionContent* content,
    cmGeneratorExpressionDAGChecker* dagChecker) const override
  {
    if (!context->HeadTarget || !dagChecker ||
        !dagChecker->EvaluatingLinkOptionsExpression()) {
      reportError(context, content->GetOriginalExpression(),
                  "$<HOST_LINK:...> may only be used with binary targets "
                  "to specify link options.");
      return std::string();
    }

    return context->HeadTarget->IsDeviceLink() ? std::string()
                                               : cmJoin(parameters, ";");
  }
} hostLinkNode;

static const struct DeviceLinkNode : public cmGeneratorExpressionNode
{
  DeviceLinkNode() {} // NOLINT(modernize-use-equals-default)

  int NumExpectedParameters() const override { return ZeroOrMoreParameters; }

  std::string Evaluate(
    const std::vector<std::string>& parameters,
    cmGeneratorExpressionContext* context,
    const GeneratorExpressionContent* content,
    cmGeneratorExpressionDAGChecker* dagChecker) const override
  {
    if (!context->HeadTarget || !dagChecker ||
        !dagChecker->EvaluatingLinkOptionsExpression()) {
      reportError(context, content->GetOriginalExpression(),
                  "$<DEVICE_LINK:...> may only be used with binary targets "
                  "to specify link options.");
      return std::string();
    }

    if (context->HeadTarget->IsDeviceLink()) {
      std::vector<std::string> list;
      cmExpandLists(parameters.begin(), parameters.end(), list);
      const auto DL_BEGIN = "<DEVICE_LINK>"_s;
      const auto DL_END = "</DEVICE_LINK>"_s;
      cm::erase_if(list, [&](const std::string& item) {
        return item == DL_BEGIN || item == DL_END;
      });

      list.insert(list.begin(), static_cast<std::string>(DL_BEGIN));
      list.push_back(static_cast<std::string>(DL_END));

      return cmJoin(list, ";");
    }

    return std::string();
  }
} deviceLinkNode;

static std::string getLinkedTargetsContent(
  cmGeneratorTarget const* target, std::string const& prop,
  cmGeneratorExpressionContext* context,
  cmGeneratorExpressionDAGChecker* dagChecker)
{
  std::string result;
  if (cmLinkImplementationLibraries const* impl =
        target->GetLinkImplementationLibraries(
          context->Config, cmGeneratorTarget::LinkInterfaceFor::Usage)) {
    for (cmLinkImplItem const& lib : impl->Libraries) {
      if (lib.Target) {
        // Pretend $<TARGET_PROPERTY:lib.Target,prop> appeared in our
        // caller's property and hand-evaluate it as if it were compiled.
        // Create a context as cmCompiledGeneratorExpression::Evaluate does.
        cmGeneratorExpressionContext libContext(
          target->GetLocalGenerator(), context->Config, context->Quiet, target,
          target, context->EvaluateForBuildsystem, lib.Backtrace,
          context->Language);
        std::string libResult =
          lib.Target->EvaluateInterfaceProperty(prop, &libContext, dagChecker);
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

  static const char* GetErrorText(std::string const& targetName,
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
    const std::vector<std::string>& parameters,
    cmGeneratorExpressionContext* context,
    const GeneratorExpressionContent* content,
    cmGeneratorExpressionDAGChecker* dagCheckerParent) const override
  {
    static cmsys::RegularExpression propertyNameValidator("^[A-Za-z0-9_]+$");

    cmGeneratorTarget const* target = nullptr;
    std::string targetName;
    std::string propertyName;

    if (parameters.size() == 2) {
      targetName = parameters[0];
      propertyName = parameters[1];

      if (const char* e = GetErrorText(targetName, propertyName)) {
        reportError(context, content->GetOriginalExpression(), e);
        return std::string();
      }
      if (propertyName == "ALIASED_TARGET"_s) {
        if (context->LG->GetMakefile()->IsAlias(targetName)) {
          if (cmGeneratorTarget* tgt =
                context->LG->FindGeneratorTargetToUse(targetName)) {
            return tgt->GetName();
          }
        }
        return std::string();
      }
      if (propertyName == "ALIAS_GLOBAL"_s) {
        if (context->LG->GetMakefile()->IsAlias(targetName)) {
          return context->LG->GetGlobalGenerator()->IsAlias(targetName)
            ? "TRUE"
            : "FALSE";
        }
        return std::string();
      }
      target = context->LG->FindGeneratorTargetToUse(targetName);

      if (!target) {
        std::ostringstream e;
        e << "Target \"" << targetName << "\" not found.";
        reportError(context, content->GetOriginalExpression(), e.str());
        return std::string();
      }
      context->AllTargets.insert(target);

    } else if (parameters.size() == 1) {
      target = context->HeadTarget;
      propertyName = parameters[0];

      // Keep track of the properties seen while processing.
      // The evaluation of the LINK_LIBRARIES generator expressions
      // will check this to ensure that properties have one consistent
      // value for all evaluations.
      context->SeenTargetProperties.insert(propertyName);

      context->HadHeadSensitiveCondition = true;
      if (!target) {
        reportError(
          context, content->GetOriginalExpression(),
          "$<TARGET_PROPERTY:prop>  may only be used with binary targets.  "
          "It may not be used with add_custom_command or add_custom_target.  "
          " "
          "Specify the target to read a property from using the "
          "$<TARGET_PROPERTY:tgt,prop> signature instead.");
        return std::string();
      }
    } else {
      reportError(
        context, content->GetOriginalExpression(),
        "$<TARGET_PROPERTY:...> expression requires one or two parameters");
      return std::string();
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

    bool evaluatingLinkLibraries = false;

    if (dagCheckerParent) {
      if (dagCheckerParent->EvaluatingGenexExpression() ||
          dagCheckerParent->EvaluatingPICExpression()) {
        // No check required.
      } else if (dagCheckerParent->EvaluatingLinkLibraries()) {
        evaluatingLinkLibraries = true;
        if (!interfacePropertyName.empty()) {
          reportError(
            context, content->GetOriginalExpression(),
            "$<TARGET_PROPERTY:...> expression in link libraries "
            "evaluation depends on target property which is transitive "
            "over the link libraries, creating a recursion.");
          return std::string();
        }
      } else {
#define ASSERT_TRANSITIVE_PROPERTY_METHOD(METHOD) dagCheckerParent->METHOD() ||
        assert(CM_FOR_EACH_TRANSITIVE_PROPERTY_METHOD(
          ASSERT_TRANSITIVE_PROPERTY_METHOD) false); // NOLINT(clang-tidy)
#undef ASSERT_TRANSITIVE_PROPERTY_METHOD
      }
    }

    if (isInterfaceProperty) {
      return cmGeneratorExpression::StripEmptyListElements(
        target->EvaluateInterfaceProperty(propertyName, context,
                                          dagCheckerParent));
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

    if (!haveProp && !target->IsImported() &&
        target->GetType() != cmStateEnums::INTERFACE_LIBRARY) {
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
      result = cmGeneratorExpression::StripEmptyListElements(
        this->EvaluateDependentExpression(result, context->LG, context, target,
                                          &dagChecker, target));
      std::string linkedTargetsContent = getLinkedTargetsContent(
        target, interfacePropertyName, context, &dagChecker);
      if (!linkedTargetsContent.empty()) {
        result += (result.empty() ? "" : ";") + linkedTargetsContent;
      }
    }
    return result;
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
    std::string const& tgtName = parameters.front();
    cmGeneratorTarget* gt = context->LG->FindGeneratorTargetToUse(tgtName);
    if (!gt) {
      std::ostringstream e;
      e << "Objects of target \"" << tgtName
        << "\" referenced but no such target exists.";
      reportError(context, content->GetOriginalExpression(), e.str());
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
      reportError(context, content->GetOriginalExpression(), e.str());
      return std::string();
    }
    cmGlobalGenerator* gg = context->LG->GetGlobalGenerator();
    {
      std::string reason;
      if (!context->EvaluateForBuildsystem &&
          !gt->Target->HasKnownObjectFileLocation(&reason)) {
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
      cmValue loc = nullptr;
      cmValue imp = nullptr;
      std::string suffix;
      if (gt->Target->GetMappedConfig(context->Config, loc, imp, suffix)) {
        cmExpandList(*loc, objects);
      }
      context->HadContextSensitiveCondition = true;
    } else {
      gt->GetTargetObjectNames(context->Config, objects);

      std::string obj_dir;
      if (context->EvaluateForBuildsystem && !gg->SupportsCrossConfigs()) {
        // Use object file directory with buildsystem placeholder.
        obj_dir = gt->ObjectDirectory;
        context->HadContextSensitiveCondition =
          gt->HasContextDependentSources();
      } else {
        // Use object file directory with per-config location.
        obj_dir = gt->GetObjectDirectory(context->Config);
        context->HadContextSensitiveCondition = true;
      }

      for (std::string& o : objects) {
        o = cmStrCat(obj_dir, o);
      }
    }

    // Create the cmSourceFile instances in the referencing directory.
    cmMakefile* mf = context->LG->GetMakefile();
    for (std::string const& o : objects) {
      mf->AddTargetObject(tgtName, o);
    }

    return cmJoin(objects, ";");
  }
} targetObjectsNode;

static const struct TargetRuntimeDllsNode : public cmGeneratorExpressionNode
{
  TargetRuntimeDllsNode() {} // NOLINT(modernize-use-equals-default)

  std::string Evaluate(
    const std::vector<std::string>& parameters,
    cmGeneratorExpressionContext* context,
    const GeneratorExpressionContent* content,
    cmGeneratorExpressionDAGChecker* /*dagChecker*/) const override
  {
    std::string const& tgtName = parameters.front();
    cmGeneratorTarget* gt = context->LG->FindGeneratorTargetToUse(tgtName);
    if (!gt) {
      std::ostringstream e;
      e << "Objects of target \"" << tgtName
        << "\" referenced but no such target exists.";
      reportError(context, content->GetOriginalExpression(), e.str());
      return std::string();
    }
    cmStateEnums::TargetType type = gt->GetType();
    if (type != cmStateEnums::EXECUTABLE &&
        type != cmStateEnums::SHARED_LIBRARY &&
        type != cmStateEnums::MODULE_LIBRARY) {
      std::ostringstream e;
      e << "Objects of target \"" << tgtName
        << "\" referenced but is not one of the allowed target types "
        << "(EXECUTABLE, SHARED, MODULE).";
      reportError(context, content->GetOriginalExpression(), e.str());
      return std::string();
    }

    if (auto* cli = gt->GetLinkInformation(context->Config)) {
      std::vector<std::string> dllPaths;
      auto const& dlls = cli->GetRuntimeDLLs();

      for (auto const& dll : dlls) {
        if (auto loc = dll->MaybeGetLocation(context->Config)) {
          dllPaths.emplace_back(*loc);
        }
      }

      return cmJoin(dllPaths, ";");
    }

    return "";
  }
} targetRuntimeDllsNode;

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

    using LangMap = std::map<std::string, std::vector<std::string>>;
    static LangMap availableFeatures;

    LangMap testedFeatures;
    cmStandardLevelResolver standardResolver(context->LG->GetMakefile());
    for (std::string const& p : parameters) {
      std::string error;
      std::string lang;
      if (!standardResolver.CompileFeatureKnown(
            context->HeadTarget->Target->GetName(), p, lang, &error)) {
        reportError(context, content->GetOriginalExpression(), error);
        return std::string();
      }
      testedFeatures[lang].push_back(p);

      if (availableFeatures.find(lang) == availableFeatures.end()) {
        cmValue featuresKnown =
          standardResolver.CompileFeaturesAvailable(lang, &error);
        if (!featuresKnown) {
          reportError(context, content->GetOriginalExpression(), error);
          return std::string();
        }
        cmExpandList(featuresKnown, availableFeatures[lang]);
      }
    }

    bool evalLL = dagChecker && dagChecker->EvaluatingLinkLibraries();

    for (auto const& lit : testedFeatures) {
      std::vector<std::string> const& langAvailable =
        availableFeatures[lit.first];
      cmValue standardDefault = context->LG->GetMakefile()->GetDefinition(
        "CMAKE_" + lit.first + "_STANDARD_DEFAULT");
      for (std::string const& it : lit.second) {
        if (!cm::contains(langAvailable, it)) {
          return "0";
        }
        if (standardDefault && standardDefault->empty()) {
          // This compiler has no notion of language standard levels.
          // All features known for the language are always available.
          continue;
        }
        if (!standardResolver.HaveStandardAvailable(target, lit.first,
                                                    context->Config, it)) {
          if (evalLL) {
            cmValue l =
              target->GetLanguageStandard(lit.first, context->Config);
            if (!l) {
              l = standardDefault;
            }
            assert(l);
            context->MaxLanguageStandard[target][lit.first] = *l;
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

static cmPolicies::PolicyStatus statusForTarget(cmGeneratorTarget const* tgt,
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

static cmPolicies::PolicyID policyForString(const char* policy_id)
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
class ArtifactBundleDirNameTag;
class ArtifactBundleContentDirTag;

template <typename ArtifactT, typename ComponentT>
struct TargetFilesystemArtifactDependency
{
  static void AddDependency(cmGeneratorTarget* target,
                            cmGeneratorExpressionContext* context)
  {
    context->DependTargets.insert(target);
    context->AllTargets.insert(target);
  }
};

struct TargetFilesystemArtifactDependencyCMP0112
{
  static void AddDependency(cmGeneratorTarget* target,
                            cmGeneratorExpressionContext* context)
  {
    context->AllTargets.insert(target);
    cmLocalGenerator* lg = context->LG;
    switch (target->GetPolicyStatusCMP0112()) {
      case cmPolicies::WARN:
        if (lg->GetMakefile()->PolicyOptionalWarningEnabled(
              "CMAKE_POLICY_WARNING_CMP0112")) {
          std::string err =
            cmStrCat(cmPolicies::GetPolicyWarning(cmPolicies::CMP0112),
                     "\nDependency being added to target:\n  \"",
                     target->GetName(), "\"\n");
          lg->GetCMakeInstance()->IssueMessage(MessageType ::AUTHOR_WARNING,
                                               err, context->Backtrace);
        }
        CM_FALLTHROUGH;
      case cmPolicies::OLD:
        context->DependTargets.insert(target);
        break;
      case cmPolicies::REQUIRED_IF_USED:
      case cmPolicies::REQUIRED_ALWAYS:
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
    std::string result = cmStrCat(target->GetDirectory(context->Config), '/',
                                  target->GetSOName(context->Config));
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

    std::string result = cmStrCat(target->GetPDBDirectory(context->Config),
                                  '/', target->GetPDBName(context->Config));
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
struct TargetFilesystemArtifactResultCreator<ArtifactBundleDirNameTag>
{
  static std::string Create(cmGeneratorTarget* target,
                            cmGeneratorExpressionContext* context,
                            const GeneratorExpressionContent* content)
  {
    if (target->IsImported()) {
      ::reportError(
        context, content->GetOriginalExpression(),
        "TARGET_BUNDLE_DIR_NAME not allowed for IMPORTED targets.");
      return std::string();
    }
    if (!target->IsBundleOnApple()) {
      ::reportError(
        context, content->GetOriginalExpression(),
        "TARGET_BUNDLE_DIR_NAME is allowed only for Bundle targets.");
      return std::string();
    }

    auto level = cmGeneratorTarget::BundleDirLevel;
    auto config = context->Config;
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

struct TargetArtifactBase : public cmGeneratorExpressionNode
{
  TargetArtifactBase() {} // NOLINT(modernize-use-equals-default)

protected:
  cmGeneratorTarget* GetTarget(
    const std::vector<std::string>& parameters,
    cmGeneratorExpressionContext* context,
    const GeneratorExpressionContent* content,
    cmGeneratorExpressionDAGChecker* dagChecker) const
  {
    // Lookup the referenced target.
    std::string const& name = parameters.front();

    if (!cmGeneratorExpression::IsValidTargetName(name)) {
      ::reportError(context, content->GetOriginalExpression(),
                    "Expression syntax not recognized.");
      return nullptr;
    }
    cmGeneratorTarget* target = context->LG->FindGeneratorTargetToUse(name);
    if (!target) {
      ::reportError(context, content->GetOriginalExpression(),
                    "No target \"" + name + "\"");
      return nullptr;
    }
    if (target->GetType() >= cmStateEnums::OBJECT_LIBRARY &&
        target->GetType() != cmStateEnums::UNKNOWN_LIBRARY) {
      ::reportError(context, content->GetOriginalExpression(),
                    "Target \"" + name +
                      "\" is not an executable or library.");
      return nullptr;
    }
    if (dagChecker &&
        (dagChecker->EvaluatingLinkLibraries(target) ||
         (dagChecker->EvaluatingSources() &&
          target == dagChecker->TopTarget()))) {
      ::reportError(context, content->GetOriginalExpression(),
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
    const std::vector<std::string>& parameters,
    cmGeneratorExpressionContext* context,
    const GeneratorExpressionContent* content,
    cmGeneratorExpressionDAGChecker* dagChecker) const override
  {
    cmGeneratorTarget* target =
      this->GetTarget(parameters, context, content, dagChecker);
    if (!target) {
      return std::string();
    }
    // Not a dependent target if we are querying for ArtifactDirTag,
    // ArtifactNameTag, ArtifactBundleDirTag, ArtifactBundleDirNameTag,
    // and ArtifactBundleContentDirTag
    TargetFilesystemArtifactDependency<ArtifactT, ComponentT>::AddDependency(
      target, context);

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

static const TargetFilesystemArtifact<ArtifactBundleDirNameTag,
                                      ArtifactNameTag>
  targetBundleDirNameNode;

static const TargetFilesystemArtifact<ArtifactBundleContentDirTag,
                                      ArtifactPathTag>
  targetBundleContentDirNode;

//
// To retrieve base name for various artifacts
//
template <typename ArtifactT>
struct TargetOutputNameArtifactResultGetter
{
  static std::string Get(cmGeneratorTarget* target,
                         cmGeneratorExpressionContext* context,
                         const GeneratorExpressionContent* content);
};

template <>
struct TargetOutputNameArtifactResultGetter<ArtifactNameTag>
{
  static std::string Get(cmGeneratorTarget* target,
                         cmGeneratorExpressionContext* context,
                         const GeneratorExpressionContent* /*unused*/)
  {
    return target->GetOutputName(context->Config,
                                 cmStateEnums::RuntimeBinaryArtifact) +
      target->GetFilePostfix(context->Config);
  }
};

template <>
struct TargetOutputNameArtifactResultGetter<ArtifactLinkerTag>
{
  static std::string Get(cmGeneratorTarget* target,
                         cmGeneratorExpressionContext* context,
                         const GeneratorExpressionContent* content)
  {
    // The file used to link to the target (.so, .lib, .a).
    if (!target->IsLinkable()) {
      ::reportError(context, content->GetOriginalExpression(),
                    "TARGET_LINKER_FILE_BASE_NAME is allowed only for "
                    "libraries and executables with ENABLE_EXPORTS.");
      return std::string();
    }
    cmStateEnums::ArtifactType artifact =
      target->HasImportLibrary(context->Config)
      ? cmStateEnums::ImportLibraryArtifact
      : cmStateEnums::RuntimeBinaryArtifact;
    return target->GetOutputName(context->Config, artifact) +
      target->GetFilePostfix(context->Config);
  }
};

template <>
struct TargetOutputNameArtifactResultGetter<ArtifactPdbTag>
{
  static std::string Get(cmGeneratorTarget* target,
                         cmGeneratorExpressionContext* context,
                         const GeneratorExpressionContent* content)
  {
    if (target->IsImported()) {
      ::reportError(
        context, content->GetOriginalExpression(),
        "TARGET_PDB_FILE_BASE_NAME not allowed for IMPORTED targets.");
      return std::string();
    }

    std::string language = target->GetLinkerLanguage(context->Config);

    std::string pdbSupportVar = "CMAKE_" + language + "_LINKER_SUPPORTS_PDB";

    if (!context->LG->GetMakefile()->IsOn(pdbSupportVar)) {
      ::reportError(
        context, content->GetOriginalExpression(),
        "TARGET_PDB_FILE_BASE_NAME is not supported by the target linker.");
      return std::string();
    }

    cmStateEnums::TargetType targetType = target->GetType();

    if (targetType != cmStateEnums::SHARED_LIBRARY &&
        targetType != cmStateEnums::MODULE_LIBRARY &&
        targetType != cmStateEnums::EXECUTABLE) {
      ::reportError(context, content->GetOriginalExpression(),
                    "TARGET_PDB_FILE_BASE_NAME is allowed only for "
                    "targets with linker created artifacts.");
      return std::string();
    }

    return target->GetPDBOutputName(context->Config) +
      target->GetFilePostfix(context->Config);
  }
};

template <typename ArtifactT>
struct TargetFileBaseNameArtifact : public TargetArtifactBase
{
  TargetFileBaseNameArtifact() {} // NOLINT(modernize-use-equals-default)

  int NumExpectedParameters() const override { return 1; }

  std::string Evaluate(
    const std::vector<std::string>& parameters,
    cmGeneratorExpressionContext* context,
    const GeneratorExpressionContent* content,
    cmGeneratorExpressionDAGChecker* dagChecker) const override
  {
    cmGeneratorTarget* target =
      this->GetTarget(parameters, context, content, dagChecker);
    if (!target) {
      return std::string();
    }

    std::string result = TargetOutputNameArtifactResultGetter<ArtifactT>::Get(
      target, context, content);
    if (context->HadError) {
      return std::string();
    }
    return result;
  }
};

static const TargetFileBaseNameArtifact<ArtifactNameTag>
  targetFileBaseNameNode;
static const TargetFileBaseNameArtifact<ArtifactLinkerTag>
  targetLinkerFileBaseNameNode;
static const TargetFileBaseNameArtifact<ArtifactPdbTag>
  targetPdbFileBaseNameNode;

class ArtifactFilePrefixTag;
class ArtifactLinkerFilePrefixTag;
class ArtifactFileSuffixTag;
class ArtifactLinkerFileSuffixTag;

template <typename ArtifactT>
struct TargetFileArtifactResultGetter
{
  static std::string Get(cmGeneratorTarget* target,
                         cmGeneratorExpressionContext* context,
                         const GeneratorExpressionContent* content);
};

template <>
struct TargetFileArtifactResultGetter<ArtifactFilePrefixTag>
{
  static std::string Get(cmGeneratorTarget* target,
                         cmGeneratorExpressionContext* context,
                         const GeneratorExpressionContent*)
  {
    return target->GetFilePrefix(context->Config);
  }
};
template <>
struct TargetFileArtifactResultGetter<ArtifactLinkerFilePrefixTag>
{
  static std::string Get(cmGeneratorTarget* target,
                         cmGeneratorExpressionContext* context,
                         const GeneratorExpressionContent* content)
  {
    if (!target->IsLinkable()) {
      ::reportError(context, content->GetOriginalExpression(),
                    "TARGET_LINKER_PREFIX is allowed only for libraries and "
                    "executables with ENABLE_EXPORTS.");
      return std::string();
    }

    cmStateEnums::ArtifactType artifact =
      target->HasImportLibrary(context->Config)
      ? cmStateEnums::ImportLibraryArtifact
      : cmStateEnums::RuntimeBinaryArtifact;

    return target->GetFilePrefix(context->Config, artifact);
  }
};
template <>
struct TargetFileArtifactResultGetter<ArtifactFileSuffixTag>
{
  static std::string Get(cmGeneratorTarget* target,
                         cmGeneratorExpressionContext* context,
                         const GeneratorExpressionContent*)
  {
    return target->GetFileSuffix(context->Config);
  }
};
template <>
struct TargetFileArtifactResultGetter<ArtifactLinkerFileSuffixTag>
{
  static std::string Get(cmGeneratorTarget* target,
                         cmGeneratorExpressionContext* context,
                         const GeneratorExpressionContent* content)
  {
    if (!target->IsLinkable()) {
      ::reportError(context, content->GetOriginalExpression(),
                    "TARGET_LINKER_SUFFIX is allowed only for libraries and "
                    "executables with ENABLE_EXPORTS.");
      return std::string();
    }

    cmStateEnums::ArtifactType artifact =
      target->HasImportLibrary(context->Config)
      ? cmStateEnums::ImportLibraryArtifact
      : cmStateEnums::RuntimeBinaryArtifact;

    return target->GetFileSuffix(context->Config, artifact);
  }
};

template <typename ArtifactT>
struct TargetFileArtifact : public TargetArtifactBase
{
  TargetFileArtifact() {} // NOLINT(modernize-use-equals-default)

  int NumExpectedParameters() const override { return 1; }

  std::string Evaluate(
    const std::vector<std::string>& parameters,
    cmGeneratorExpressionContext* context,
    const GeneratorExpressionContent* content,
    cmGeneratorExpressionDAGChecker* dagChecker) const override
  {
    cmGeneratorTarget* target =
      this->GetTarget(parameters, context, content, dagChecker);
    if (!target) {
      return std::string();
    }

    std::string result =
      TargetFileArtifactResultGetter<ArtifactT>::Get(target, context, content);
    if (context->HadError) {
      return std::string();
    }
    return result;
  }
};

static const TargetFileArtifact<ArtifactFilePrefixTag> targetFilePrefixNode;
static const TargetFileArtifact<ArtifactLinkerFilePrefixTag>
  targetLinkerFilePrefixNode;
static const TargetFileArtifact<ArtifactFileSuffixTag> targetFileSuffixNode;
static const TargetFileArtifact<ArtifactLinkerFileSuffixTag>
  targetLinkerFileSuffixNode;

static const struct ShellPathNode : public cmGeneratorExpressionNode
{
  ShellPathNode() {} // NOLINT(modernize-use-equals-default)

  std::string Evaluate(
    const std::vector<std::string>& parameters,
    cmGeneratorExpressionContext* context,
    const GeneratorExpressionContent* content,
    cmGeneratorExpressionDAGChecker* /*dagChecker*/) const override
  {
    std::vector<std::string> listIn = cmExpandedList(parameters.front());
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
    { "PLATFORM_ID", &platformIdNode },
    { "COMPILE_FEATURES", &compileFeaturesNode },
    { "CONFIGURATION", &configurationNode },
    { "CONFIG", &configurationTestNode },
    { "TARGET_FILE", &targetNodeGroup.File },
    { "TARGET_LINKER_FILE", &targetLinkerNodeGroup.File },
    { "TARGET_SONAME_FILE", &targetSoNameNodeGroup.File },
    { "TARGET_PDB_FILE", &targetPdbNodeGroup.File },
    { "TARGET_FILE_BASE_NAME", &targetFileBaseNameNode },
    { "TARGET_LINKER_FILE_BASE_NAME", &targetLinkerFileBaseNameNode },
    { "TARGET_PDB_FILE_BASE_NAME", &targetPdbFileBaseNameNode },
    { "TARGET_FILE_PREFIX", &targetFilePrefixNode },
    { "TARGET_LINKER_FILE_PREFIX", &targetLinkerFilePrefixNode },
    { "TARGET_FILE_SUFFIX", &targetFileSuffixNode },
    { "TARGET_LINKER_FILE_SUFFIX", &targetLinkerFileSuffixNode },
    { "TARGET_FILE_NAME", &targetNodeGroup.FileName },
    { "TARGET_LINKER_FILE_NAME", &targetLinkerNodeGroup.FileName },
    { "TARGET_SONAME_FILE_NAME", &targetSoNameNodeGroup.FileName },
    { "TARGET_PDB_FILE_NAME", &targetPdbNodeGroup.FileName },
    { "TARGET_FILE_DIR", &targetNodeGroup.FileDir },
    { "TARGET_LINKER_FILE_DIR", &targetLinkerNodeGroup.FileDir },
    { "TARGET_SONAME_FILE_DIR", &targetSoNameNodeGroup.FileDir },
    { "TARGET_PDB_FILE_DIR", &targetPdbNodeGroup.FileDir },
    { "TARGET_BUNDLE_DIR", &targetBundleDirNode },
    { "TARGET_BUNDLE_DIR_NAME", &targetBundleDirNameNode },
    { "TARGET_BUNDLE_CONTENT_DIR", &targetBundleContentDirNode },
    { "STREQUAL", &strEqualNode },
    { "EQUAL", &equalNode },
    { "IN_LIST", &inListNode },
    { "FILTER", &filterNode },
    { "REMOVE_DUPLICATES", &removeDuplicatesNode },
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
    { "TARGET_PROPERTY", &targetPropertyNode },
    { "TARGET_NAME", &targetNameNode },
    { "TARGET_OBJECTS", &targetObjectsNode },
    { "TARGET_POLICY", &targetPolicyNode },
    { "TARGET_EXISTS", &targetExistsNode },
    { "TARGET_NAME_IF_EXISTS", &targetNameIfExistsNode },
    { "TARGET_GENEX_EVAL", &targetGenexEvalNode },
    { "TARGET_RUNTIME_DLLS", &targetRuntimeDllsNode },
    { "GENEX_EVAL", &genexEvalNode },
    { "BUILD_INTERFACE", &buildInterfaceNode },
    { "INSTALL_INTERFACE", &installInterfaceNode },
    { "INSTALL_PREFIX", &installPrefixNode },
    { "JOIN", &joinNode },
    { "LINK_ONLY", &linkOnlyNode },
    { "COMPILE_LANG_AND_ID", &languageAndIdNode },
    { "COMPILE_LANGUAGE", &languageNode },
    { "LINK_LANG_AND_ID", &linkLanguageAndIdNode },
    { "LINK_LANGUAGE", &linkLanguageNode },
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
