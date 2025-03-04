/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmGeneratorExpression.h"

#include <algorithm>
#include <cassert>
#include <memory>
#include <utility>

#include <cm/string_view>

#include "cmsys/RegularExpression.hxx"

#include "cmGeneratorExpressionContext.h"
#include "cmGeneratorExpressionDAGChecker.h"
#include "cmGeneratorExpressionEvaluator.h"
#include "cmGeneratorExpressionLexer.h"
#include "cmGeneratorExpressionParser.h"
#include "cmList.h"
#include "cmLocalGenerator.h"
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
  std::string input, cmLocalGenerator* lg, std::string const& config,
  cmGeneratorTarget const* headTarget,
  cmGeneratorExpressionDAGChecker* dagChecker,
  cmGeneratorTarget const* currentTarget, std::string const& language)
{
  if (Find(input) != std::string::npos) {
#ifndef CMAKE_BOOTSTRAP
    auto profilingRAII = lg->GetCMakeInstance()->CreateProfilingEntry(
      "genex_compile_eval", input);
#endif

    cmCompiledGeneratorExpression cge(*lg->GetCMakeInstance(),
                                      cmListFileBacktrace(), std::move(input));
    return cge.Evaluate(lg, config, headTarget, dagChecker, currentTarget,
                        language);
  }
  return input;
}

std::string const& cmCompiledGeneratorExpression::Evaluate(
  cmLocalGenerator* lg, std::string const& config,
  cmGeneratorTarget const* headTarget,
  cmGeneratorExpressionDAGChecker* dagChecker,
  cmGeneratorTarget const* currentTarget, std::string const& language) const
{
  cmGeneratorExpressionContext context(
    lg, config, this->Quiet, headTarget,
    currentTarget ? currentTarget : headTarget, this->EvaluateForBuildsystem,
    this->Backtrace, language);

  if (!this->NeedsEvaluation) {
    return this->Input;
  }

  this->Output.clear();

  for (auto const& it : this->Evaluators) {
    this->Output += it->Evaluate(&context, dagChecker);

    this->SeenTargetProperties.insert(context.SeenTargetProperties.cbegin(),
                                      context.SeenTargetProperties.cend());
    if (context.HadError) {
      this->Output.clear();
      break;
    }
  }

  this->MaxLanguageStandard = context.MaxLanguageStandard;

  if (!context.HadError) {
    this->HadContextSensitiveCondition = context.HadContextSensitiveCondition;
    this->HadHeadSensitiveCondition = context.HadHeadSensitiveCondition;
    this->HadLinkLanguageSensitiveCondition =
      context.HadLinkLanguageSensitiveCondition;
    this->SourceSensitiveTargets = context.SourceSensitiveTargets;
  }

  this->DependTargets = context.DependTargets;
  this->AllTargetsSeen = context.AllTargets;
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

static std::string stripAllGeneratorExpressions(std::string const& input)
{
  std::string result;
  std::string::size_type pos = 0;
  std::string::size_type lastPos = pos;
  int nestingLevel = 0;
  while ((pos = input.find("$<", lastPos)) != std::string::npos) {
    result += input.substr(lastPos, pos - lastPos);
    pos += 2;
    nestingLevel = 1;
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
    std::string::size_type const traversed = (c - cStart) + 1;
    if (!*c) {
      result += "$<" + input.substr(pos, traversed);
    }
    pos += traversed;
    lastPos = pos;
  }
  if (nestingLevel == 0) {
    result += input.substr(lastPos);
  }
  return cmGeneratorExpression::StripEmptyListElements(result);
}

static void prefixItems(std::string const& content, std::string& result,
                        cm::string_view const& prefix)
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
  std::string const& input, cmGeneratorExpression::PreprocessContext context,
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
        if (nestingLevel != 0) {
          continue;
        }
        if (context == cmGeneratorExpression::BuildInterface &&
            foundGenex == FoundGenex::BuildInterface) {
          result += input.substr(pos, c - cStart);
        } else if (context == cmGeneratorExpression::InstallInterface &&
                   foundGenex == FoundGenex::InstallInterface) {
          std::string const content = input.substr(pos, c - cStart);
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
    output.push_back(preGenex + "$<" + input.substr(pos, traversed));
    pos += traversed;
    lastPos = pos;
  }
  if (lastPos < input.size()) {
    cmExpandList(input.substr(lastPos), output);
  }
}

std::string cmGeneratorExpression::Preprocess(std::string const& input,
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

cm::string_view::size_type cmGeneratorExpression::Find(
  cm::string_view const& input)
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

  // Specify COMPILE_OPTIONS to DAGchecker, same semantic as COMPILE_FLAGS
  cmGeneratorExpressionDAGChecker dagChecker{
    this->HeadTarget,
    property == "COMPILE_FLAGS" ? "COMPILE_OPTIONS" : property,
    nullptr,
    nullptr,
    this->LocalGenerator,
    this->Config,
  };

  return this->CompiledGeneratorExpression->Evaluate(
    this->LocalGenerator, this->Config, this->HeadTarget, &dagChecker, nullptr,
    this->Language);
}
