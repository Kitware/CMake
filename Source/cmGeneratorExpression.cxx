/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmGeneratorExpression.h"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <memory>
#include <stack>
#include <utility>

#include <cm/string_view>

#include "cmsys/RegularExpression.hxx"

#include "cmGenExContext.h"
#include "cmGenExEvaluation.h"
#include "cmGeneratorExpressionDAGChecker.h"
#include "cmGeneratorExpressionEvaluator.h"
#include "cmGeneratorExpressionLexer.h"
#include "cmGeneratorExpressionParser.h"
#include "cmGeneratorTarget.h"
#include "cmList.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmake.h"

cmGeneratorExpression::cmGeneratorExpression(cmake& cmakeInstance,
                                             cmListFileBacktrace backtrace)
  : CMakeInstance(cmakeInstance)
  , Backtrace(std::move(backtrace))
{
}

cmCompiledGeneratorExpression::~cmCompiledGeneratorExpression() = default;

cmGeneratorExpression::~cmGeneratorExpression() = default;

std::unique_ptr<cmCompiledGeneratorExpression> cmGeneratorExpression::Parse(
  std::string input) const
{
  return std::unique_ptr<cmCompiledGeneratorExpression>(
    new cmCompiledGeneratorExpression(this->CMakeInstance, this->Backtrace,
                                      std::move(input)));
}

std::string cmGeneratorExpression::Evaluate(
  std::string input, cmLocalGenerator const* lg, std::string const& config,
  cmGeneratorTarget const* headTarget,
  cmGeneratorExpressionDAGChecker* dagChecker,
  cmGeneratorTarget const* currentTarget, std::string const& language)
{
  if (Find(input) != std::string::npos) {
#ifndef CMAKE_BOOTSTRAP
    auto profilingRAII = lg->GetCMakeInstance()->CreateProfilingEntry(
      "genex_compile_eval", input);
#endif

    cm::GenEx::Context context(lg, config, language);
    cmCompiledGeneratorExpression cge(*lg->GetCMakeInstance(),
                                      cmListFileBacktrace(), std::move(input));
    return cge.Evaluate(context, dagChecker, headTarget, currentTarget);
  }
  return input;
}

std::string const& cmCompiledGeneratorExpression::Evaluate(
  cmLocalGenerator const* lg, std::string const& config,
  cmGeneratorTarget const* headTarget) const
{
  cm::GenEx::Context context(lg, config);
  return this->Evaluate(context, nullptr, headTarget);
}

std::string const& cmCompiledGeneratorExpression::Evaluate(
  cm::GenEx::Context const& context,
  cmGeneratorExpressionDAGChecker* dagChecker,
  cmGeneratorTarget const* headTarget,
  cmGeneratorTarget const* currentTarget) const
{
  cm::GenEx::Evaluation eval(context, this->Quiet, headTarget,
                             currentTarget ? currentTarget : headTarget,
                             this->EvaluateForBuildsystem, this->Backtrace);

  if (!this->NeedsEvaluation) {
    return this->Input;
  }

  this->Output.clear();

  for (auto const& it : this->Evaluators) {
    this->Output += it->Evaluate(&eval, dagChecker);

    this->SeenTargetProperties.insert(eval.SeenTargetProperties.cbegin(),
                                      eval.SeenTargetProperties.cend());
    if (eval.HadError) {
      this->Output.clear();
      break;
    }
  }

  this->MaxLanguageStandard = eval.MaxLanguageStandard;

  if (!eval.HadError) {
    this->HadContextSensitiveCondition = eval.HadContextSensitiveCondition;
    this->HadHeadSensitiveCondition = eval.HadHeadSensitiveCondition;
    this->HadLinkLanguageSensitiveCondition =
      eval.HadLinkLanguageSensitiveCondition;
    this->SourceSensitiveTargets = eval.SourceSensitiveTargets;
  }

  this->DependTargets = eval.DependTargets;
  this->AllTargetsSeen = eval.AllTargets;
  return this->Output;
}

cmCompiledGeneratorExpression::cmCompiledGeneratorExpression(
  cmake& cmakeInstance, cmListFileBacktrace backtrace, std::string input)
  : Backtrace(std::move(backtrace))
  , Input(std::move(input))
{
#ifndef CMAKE_BOOTSTRAP
  auto profilingRAII =
    cmakeInstance.CreateProfilingEntry("genex_compile", this->Input);
#endif

  cmGeneratorExpressionLexer l;
  std::vector<cmGeneratorExpressionToken> tokens = l.Tokenize(this->Input);
  this->NeedsEvaluation = l.GetSawGeneratorExpression();

  if (this->NeedsEvaluation) {
    cmGeneratorExpressionParser p(tokens);
    p.Parse(this->Evaluators);
  }
}

std::string cmGeneratorExpression::StripEmptyListElements(
  std::string const& input)
{
  if (input.find(';') == std::string::npos) {
    return input;
  }
  std::string result;
  result.reserve(input.size());

  char const* c = input.c_str();
  char const* last = c;
  bool skipSemiColons = true;
  for (; *c; ++c) {
    if (*c == ';') {
      if (skipSemiColons) {
        result.append(last, c - last);
        last = c + 1;
      }
      skipSemiColons = true;
    } else {
      skipSemiColons = false;
    }
  }
  result.append(last);

  if (!result.empty() && *(result.end() - 1) == ';') {
    result.resize(result.size() - 1);
  }

  return result;
}

static std::string extractAllGeneratorExpressions(
  cm::string_view input,
  std::map<std::string, std::vector<std::string>>* collected)
{
  std::string result;
  std::string::size_type pos = 0;
  std::string::size_type lastPos = pos;
  std::stack<char const*> starts; // indices of "$<"
  std::stack<char const*> colons; // indices of ":"
  while ((pos = input.find("$<", lastPos)) != std::string::npos) {
    result += input.substr(lastPos, pos - lastPos);
    starts.push(input.data() + pos);
    pos += 2;
    char const* c = input.data() + pos;
    char const* const cStart = c;
    for (; *c; ++c) {
      if (cmGeneratorExpression::StartsWithGeneratorExpression(c)) {
        starts.push(c);
        ++c;
        continue;
      }
      if (c[0] == ':') {
        if (colons.size() < starts.size()) {
          colons.push(c);
        }
      } else if (c[0] == '>') {
        if (!colons.empty() && !starts.empty() &&
            starts.top() < colons.top()) {
          if (collected) {
            (*collected)[std::string(starts.top() + 2, colons.top())]
              .push_back(std::string(colons.top() + 1, c));
          }
          colons.pop();
        }
        if (!starts.empty()) {
          starts.pop();
        }
        if (starts.empty()) {
          break;
        }
      }
    }
    std::string::size_type const traversed = (c - cStart) + 1;
    if (!*c) {
      result += cmStrCat("$<", input.substr(pos, traversed));
    }
    pos += traversed;
    lastPos = pos;
  }
  if (starts.empty()) {
    result += input.substr(lastPos);
  }
  return cmGeneratorExpression::StripEmptyListElements(result);
}

static std::string stripAllGeneratorExpressions(cm::string_view input)
{
  return extractAllGeneratorExpressions(input, nullptr);
}

static void prefixItems(std::string const& content, std::string& result,
                        cm::string_view prefix)
{
  std::vector<std::string> entries;
  cmGeneratorExpression::Split(content, entries);
  char const* sep = "";
  for (std::string const& e : entries) {
    result += sep;
    sep = ";";
    if (!cmSystemTools::FileIsFullPath(e) &&
        cmGeneratorExpression::Find(e) != 0) {
      result += prefix;
    }
    result += e;
  }
}

static std::string stripExportInterface(
  cm::string_view input, cmGeneratorExpression::PreprocessContext context,
  cm::string_view importPrefix)
{
  std::string result;

  int nestingLevel = 0;
  std::string::size_type pos = 0;
  std::string::size_type lastPos = pos;
  while (true) {
    std::string::size_type bPos = input.find("$<BUILD_INTERFACE:", lastPos);
    std::string::size_type iPos = input.find("$<INSTALL_INTERFACE:", lastPos);
    std::string::size_type lPos =
      input.find("$<BUILD_LOCAL_INTERFACE:", lastPos);

    pos = std::min({ bPos, iPos, lPos });
    if (pos == std::string::npos) {
      break;
    }

    result += input.substr(lastPos, pos - lastPos);
    enum class FoundGenex
    {
      BuildInterface,
      InstallInterface,
      BuildLocalInterface,
    } foundGenex = FoundGenex::BuildInterface;
    if (pos == bPos) {
      foundGenex = FoundGenex::BuildInterface;
      pos += cmStrLen("$<BUILD_INTERFACE:");
    } else if (pos == iPos) {
      foundGenex = FoundGenex::InstallInterface;
      pos += cmStrLen("$<INSTALL_INTERFACE:");
    } else if (pos == lPos) {
      foundGenex = FoundGenex::BuildLocalInterface;
      pos += cmStrLen("$<BUILD_LOCAL_INTERFACE:");
    } else {
      assert(false && "Invalid position found");
    }
    nestingLevel = 1;
    char const* c = input.data() + pos;
    char const* const cStart = c;
    for (; *c; ++c) {
      if (cmGeneratorExpression::StartsWithGeneratorExpression(c)) {
        ++nestingLevel;
        ++c;
        continue;
      }
      if (c[0] == '>') {
        --nestingLevel;
        if (nestingLevel != 0) {
          continue;
        }
        if (context == cmGeneratorExpression::BuildInterface &&
            foundGenex == FoundGenex::BuildInterface) {
          result += input.substr(pos, c - cStart);
        } else if (context == cmGeneratorExpression::InstallInterface &&
                   foundGenex == FoundGenex::InstallInterface) {
          std::string const content =
            static_cast<std::string>(input.substr(pos, c - cStart));
          if (!importPrefix.empty()) {
            prefixItems(content, result, importPrefix);
          } else {
            result += content;
          }
        }
        break;
      }
    }
    std::string::size_type const traversed = (c - cStart) + 1;
    if (!*c) {
      auto remaining = input.substr(pos, traversed);
      switch (foundGenex) {
        case FoundGenex::BuildInterface:
          result = cmStrCat(result, "$<BUILD_INTERFACE:", remaining);
          break;
        case FoundGenex::InstallInterface:
          result = cmStrCat(result, "$<INSTALL_INTERFACE:", remaining);
          break;
        case FoundGenex::BuildLocalInterface:
          result = cmStrCat(result, "$<BUILD_LOCAL_INTERFACE:", remaining);
          break;
      }
    }
    pos += traversed;
    lastPos = pos;
  }
  if (nestingLevel == 0) {
    result += input.substr(lastPos);
  }

  return cmGeneratorExpression::StripEmptyListElements(result);
}

void cmGeneratorExpression::Split(std::string const& input,
                                  std::vector<std::string>& output)
{
  std::string::size_type pos = 0;
  std::string::size_type lastPos = pos;
  while ((pos = input.find("$<", lastPos)) != std::string::npos) {
    std::string part = input.substr(lastPos, pos - lastPos);
    std::string preGenex;
    if (!part.empty()) {
      std::string::size_type startPos = input.rfind(';', pos);
      if (startPos == std::string::npos) {
        preGenex = part;
        part.clear();
      } else if (startPos != pos - 1 && startPos >= lastPos) {
        part = input.substr(lastPos, startPos - lastPos);
        preGenex = input.substr(startPos + 1, pos - startPos - 1);
      }
      if (!part.empty()) {
        cmExpandList(part, output);
      }
    }
    pos += 2;
    int nestingLevel = 1;
    char const* c = input.c_str() + pos;
    char const* const cStart = c;
    for (; *c; ++c) {
      if (cmGeneratorExpression::StartsWithGeneratorExpression(c)) {
        ++nestingLevel;
        ++c;
        continue;
      }
      if (c[0] == '>') {
        --nestingLevel;
        if (nestingLevel == 0) {
          break;
        }
      }
    }
    for (; *c; ++c) {
      // Capture the part after the genex and before the next ';'
      if (c[0] == ';') {
        --c;
        break;
      }
    }
    std::string::size_type const traversed = (c - cStart) + 1;
    output.push_back(cmStrCat(preGenex, "$<", input.substr(pos, traversed)));
    pos += traversed;
    lastPos = pos;
  }
  if (lastPos < input.size()) {
    cmExpandList(input.substr(lastPos), output);
  }
}

std::string cmGeneratorExpression::Preprocess(cm::string_view input,
                                              PreprocessContext context,
                                              cm::string_view importPrefix)
{
  if (context == StripAllGeneratorExpressions) {
    return stripAllGeneratorExpressions(input);
  }
  if (context == BuildInterface || context == InstallInterface) {
    return stripExportInterface(input, context, importPrefix);
  }

  assert(false &&
         "cmGeneratorExpression::Preprocess called with invalid args");
  return std::string();
}

std::string cmGeneratorExpression::Collect(
  std::string const& input,
  std::map<std::string, std::vector<std::string>>& collected)
{
  return extractAllGeneratorExpressions(input, &collected);
}

bool cmGeneratorExpression::ForbidGeneratorExpressions(
  cmGeneratorTarget const* target, std::string const& propertyName,
  std::string const& propertyValue)
{
  std::map<std::string, std::vector<std::string>> allowList;
  std::string evaluatedValue;
  return ForbidGeneratorExpressions(target, propertyName, propertyValue,
                                    evaluatedValue, allowList);
}

bool cmGeneratorExpression::ForbidGeneratorExpressions(
  cmGeneratorTarget const* target, std::string const& propertyName,
  std::string const& propertyValue, std::string& evaluatedValue,
  std::map<std::string, std::vector<std::string>>& allowList)
{
  size_t const initialAllowedGenExps = allowList.size();
  evaluatedValue = Collect(propertyValue, allowList);
  if (evaluatedValue != propertyValue &&
      allowList.size() > initialAllowedGenExps) {
    target->Makefile->IssueMessage(
      MessageType::FATAL_ERROR,
      cmStrCat("Property \"", propertyName, "\" of target \"",
               target->GetName(),
               "\" contains a generator expression. This is not allowed."));
    return false;
  }

  // Check for nested generator expressions (e.g., $<LINK_ONLY:$<...>>).
  for (auto const& genexp : allowList) {
    for (auto const& value : genexp.second) {
      if (value.find("$<") != std::string::npos) {
        target->Makefile->IssueMessage(
          MessageType::FATAL_ERROR,
          cmStrCat("$<", genexp.first, ":...> expression in \"", propertyName,
                   "\" of target \"", target->GetName(),
                   "\" contains a generator expression. This is not "
                   "allowed."));
        return false;
      }
    }
  }
  return true;
}

cm::string_view::size_type cmGeneratorExpression::Find(cm::string_view input)
{
  cm::string_view::size_type const openpos = input.find("$<");
  if (openpos != cm::string_view::npos &&
      input.find('>', openpos) != cm::string_view::npos) {
    return openpos;
  }
  return cm::string_view::npos;
}

bool cmGeneratorExpression::IsValidTargetName(std::string const& input)
{
  // The ':' is supported to allow use with IMPORTED targets. At least
  // Qt 4 and 5 IMPORTED targets use ':' as the namespace delimiter.
  static cmsys::RegularExpression targetNameValidator("^[A-Za-z0-9_.:+-]+$");

  return targetNameValidator.find(input);
}

void cmGeneratorExpression::ReplaceInstallPrefix(
  std::string& input, std::string const& replacement)
{
  std::string::size_type pos = 0;
  std::string::size_type lastPos = pos;

  while ((pos = input.find("$<INSTALL_PREFIX>", lastPos)) !=
         std::string::npos) {
    std::string::size_type endPos = pos + cmStrLen("$<INSTALL_PREFIX>");
    input.replace(pos, endPos - pos, replacement);
    lastPos = endPos;
  }
}

void cmCompiledGeneratorExpression::GetMaxLanguageStandard(
  cmGeneratorTarget const* tgt, std::map<std::string, std::string>& mapping)
{
  auto it = this->MaxLanguageStandard.find(tgt);
  if (it != this->MaxLanguageStandard.end()) {
    mapping = it->second;
  }
}

std::string const& cmGeneratorExpressionInterpreter::Evaluate(
  std::string expression, std::string const& property)
{
  this->CompiledGeneratorExpression =
    this->GeneratorExpression.Parse(std::move(expression));

  cm::GenEx::Context context(this->LocalGenerator, this->Config,
                             this->Language);

  // Specify COMPILE_OPTIONS to DAGchecker, same semantic as COMPILE_FLAGS
  cmGeneratorExpressionDAGChecker dagChecker{
    this->HeadTarget,
    property == "COMPILE_FLAGS" ? "COMPILE_OPTIONS" : property,
    nullptr,
    nullptr,
    context,
  };

  return this->CompiledGeneratorExpression->Evaluate(context, &dagChecker,
                                                     this->HeadTarget);
}
