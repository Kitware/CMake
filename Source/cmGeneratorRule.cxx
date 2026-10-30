
/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmGeneratorRule.h"

#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <vector>

#include <cm/memory>
#include <cm/optional>
#include <cm/string_view>
#include <cmext/algorithm>
#include <cmext/string_view>

#include "cmCMakePath.h"
#include "cmCustomCommand.h"
#include "cmCustomCommandLines.h"
#include "cmDiagnostics.h"
#include "cmFileSet.h"
#include "cmGeneratorExpression.h"
#include "cmList.h"
#include "cmListFileCache.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmSourceFile.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmTarget.h"

namespace {
class rule_pattern_error : public std::runtime_error
{
public:
  rule_pattern_error(std::string const& msg)
    : std::runtime_error(msg)
  {
  }
};

std::vector<cm::string_view> ReservedPatterns{ "RULE"_s,
                                               "TARGET"_s,
                                               "FILE_SET"_s,
                                               "SOURCE_DIR"_s,
                                               "BINARY_DIR"_s,
                                               "CURRENT_SOURCE_DIR"_s,
                                               "CURRENT_BINARY_DIR"_s,
                                               "OUTPUT_CURRENT_SOURCE_DIR"_s,
                                               "OUTPUT_CURRENT_BINARY_DIR"_s,
                                               "SOURCE"_s,
                                               "INPUT_DIR"_s,
                                               "FILE_NAME"_s,
                                               "BASE_NAME"_s,
                                               "INCLUDE_DIRECTORIES"_s,
                                               "COMPILE_OPTIONS"_s,
                                               "COMPILE_DEFINITIONS"_s };
}

cm::RulePatternSet const cmGeneratorRule::DefaultProperties{
  { "INCLUDE_DIRECTORIES",
    "$<LIST:FILTER,$<FILE_SET_PROPERTY:<FILE_SET>,TARGET:<TARGET>,INCLUDE_"
    "DIRECTORIES>;$<SOURCE_PROPERTY:<SOURCE>,TARGET_DIRECTORY:<TARGET>,"
    "INCLUDE_DIRECTORIES>;$<RULE_PROPERTY:<RULE>,INCLUDE_DIRECTORIES>,"
    "EXCLUDE,^$>" },
  { "COMPILE_OPTIONS",
    "$<LIST:FILTER,$<RULE_PROPERTY:<RULE>,COMPILE_OPTIONS>;$<SOURCE_"
    "PROPERTY:<SOURCE>,"
    "TARGET_DIRECTORY:<TARGET>,COMPILE_OPTIONS>;$<FILE_SET_PROPERTY:<FILE_"
    "SET>,TARGET:<TARGET>,COMPILE_OPTIONS>,EXCLUDE,^$>" },
  { "COMPILE_DEFINITIONS",
    "$<LIST:FILTER,$<RULE_PROPERTY:<RULE>,COMPILE_DEFINITIONS>;$<SOURCE_"
    "PROPERTY:<"
    "SOURCE>,"
    "TARGET_DIRECTORY:<TARGET>,COMPILE_DEFINITIONS>;$<FILE_SET_PROPERTY:<"
    "FILE_SET>,TARGET:<TARGET>,COMPILE_DEFINITIONS>,EXCLUDE,^$>" }
};

cmGeneratorRule::cmGeneratorRule(cmRule const* rule, cmTarget const* target,
                                 cmFileSet const* fileSet,
                                 cmFileSet const* outputFileSet,
                                 cmSourceFile const* source,
                                 cm::RulePatternSet const& patterns)
  : Rule(rule)
  , Target(target)
  , FileSet(fileSet)
  , OutputFileSet(outputFileSet)
  , Source(source)
  , Makefile(*fileSet->GetMakefile())
  , RuleExpander(this->GetMakefile(), this->Rule->GetName(), this->Patterns)
{
  this->Name = cmStrCat(rule->GetName(), '_',
                        std::hash<std::string>{}(cmStrCat(
                          rule->GetName(), '-', target->GetName(), '-',
                          fileSet->GetName(), '-', source->GetFullPath())));

  cmCMakePath file = cmCMakePath{ this->Source->GetFullPath() }.Normal();
  cmCMakePath sourceDir = file.IsAbsolute()
    ? file.GetParentPath()
    : this->GetMakefile().GetCurrentSourceDirectory();

  /* clang-format off */
  this->Patterns.insert({ "RULE", this->GetName() });
  this->Patterns.insert({ "TARGET", this->Target->GetName() });
  this->Patterns.insert({ "FILE_SET", this->FileSet->GetName() });
  this->Patterns.insert({ "SOURCE_DIR", this->GetMakefile().GetHomeDirectory() });
  this->Patterns.insert({ "BINARY_DIR", this->GetMakefile().GetHomeOutputDirectory() });
  this->Patterns.insert({ "CURRENT_SOURCE_DIR", this->GetMakefile().GetCurrentSourceDirectory() });
  this->Patterns.insert({ "CURRENT_BINARY_DIR", this->GetMakefile().GetCurrentBinaryDirectory() });
  this->Patterns.insert({ "OUTPUT_CURRENT_SOURCE_DIR", outputFileSet->GetMakefile()->GetCurrentSourceDirectory() });
  this->Patterns.insert({ "OUTPUT_CURRENT_BINARY_DIR", outputFileSet->GetMakefile()->GetCurrentBinaryDirectory() });
  this->Patterns.insert({ "SOURCE", file.GenericString() });
  this->Patterns.insert({ "INPUT_DIR", file.GetParentPath().GenericString() });
  this->Patterns.insert({ "FILE_NAME", file.GetFileName().GenericString() });
  this->Patterns.insert({ "BASE_NAME", file.GetFileName().RemoveWideExtension().GenericString() });
  /* clang-format on */

  // instantiate default properties pattern
  for (auto const& item : DefaultProperties) {
    this->Patterns.insert(
      { item.first, this->RuleExpander.ExpandVariables(item.second) });
  }

  this->UpdateRuleExpander(patterns);
}

void cmGeneratorRule::UpdateRuleExpander(cm::RulePatternSet const& patterns)
{
  if (patterns.empty()) {
    return;
  }

  for (auto const& pattern : patterns) {
    if (cm::contains(ReservedPatterns, pattern.first)) {
      // overwriting a reserved pattern is not allowed
      continue;
    }

    this->Patterns[pattern.first] = pattern.second;
  }
}

cmValue cmGeneratorRule::GetProperty(std::string const& property) const
{
  cmValue value = this->Properties.GetPropertyValue(property);
  if (value) {
    return value;
  }

  if (property == "OUTPUT_FILE_SET"_s) {
    this->Properties.SetProperty(
      property,
      cmList{ this->OutputFileSet->GetName(), this->OutputFileSet->GetType() }
        .to_string());
    return this->Properties.GetPropertyValue(property);
  }

  if (property == "WORKING_DIRECTORY") {
    // property was not set by the user, so use the default
    this->Properties.SetProperty(
      property, this->GetMakefile().GetCurrentBinaryDirectory());
    return this->Properties.GetPropertyValue(property);
  }

  // property not yet instantiated, retrieve it from cmRule
  value = this->Rule->GetProperty(property);
  if (value) {
    std::string expandedValue{ *value };
    this->Properties.SetProperty(
      property, this->RuleExpander.ExpandVariables(expandedValue));
    return this->Properties.GetPropertyValue(property);
  }

  return value;
}

std::unique_ptr<cmCustomCommand> cmGeneratorRule::CreateCustomCommand() const
{
  auto expandVariables = [this](std::string const& item) -> std::string {
    auto result = this->RuleExpander.ExpandPatterns(item);
    if (this->RuleExpander.InError) {
      throw rule_pattern_error{ cmStrCat(
        "Pattern expression\n  ", item, "\nfrom rule \"",
        this->GetRule()->GetName(), "\", used by the fileset \"",
        this->FileSet->GetName(), "\" of the target \"",
        this->Target->GetName(), "\", did not expand correctly.") };
    }
    return result;
  };

  auto expandVector =
    [&expandVariables](
      std::vector<std::string> const& data) -> std::vector<std::string> {
    std::vector<std::string> result;
    result.reserve(data.size());
    std::transform(data.begin(), data.end(), std::back_inserter(result),
                   expandVariables);
    return result;
  };

  auto expandPath =
    [&expandVariables](std::string const& binaryDirectory,
                       std::string const& data) -> std::string {
    std::string path = expandVariables(data);
    if (cmGeneratorExpression::StartsWithGeneratorExpression(path)) {
      cmSystemTools::ConvertToUnixSlashes(path);
    } else {
      if (cmSystemTools::FileIsFullPath(path)) {
        path = cmSystemTools::CollapseFullPath(path);
      } else {
        path = cmSystemTools::CollapseFullPath(path, binaryDirectory);
      }
    }
    return path;
  };

  auto expandPaths =
    [&expandPath](
      std::string const& binaryDirectory,
      std::vector<std::string> const& data) -> std::vector<std::string> {
    std::vector<std::string> result;
    result.reserve(data.size());
    std::transform(
      data.begin(), data.end(), std::back_inserter(result),
      [&expandPath, &binaryDirectory](std::string const& item) -> std::string {
        return expandPath(binaryDirectory, item);
      });
    return result;
  };

  try {
    auto cc = cm::make_unique<cmCustomCommand>();

    cmCustomCommandLines commandLines;
    for (cmRule::CustomCommand const& line : this->Rule->GetCommands()) {
      cmCustomCommandLine command;
      command.reserve(line.size());
      std::transform(line.begin(), line.end(), std::back_inserter(command),
                     expandVariables);
      commandLines.push_back(command);
    }
    cc->SetCommandLines(std::move(commandLines));

    cc->SetOutputs(expandPaths(this->GetMakefile().GetCurrentBinaryDirectory(),
                               this->Rule->GetOutputs()));

    if (!this->Rule->GetByproducts().empty()) {
      cc->SetByproducts(expandVector(this->Rule->GetByproducts()));
    }

    if (!this->Rule->GetDepfile().empty()) {
      // store normalized path as property
      this->Properties.SetProperty(
        "DEPFILE",
        expandPath(this->GetMakefile().GetCurrentBinaryDirectory(),
                   this->Rule->GetDepfile()));
      cc->SetDepfile(expandVariables(this->Rule->GetDepfile()));
    }
    std::vector<std::string> depends{ 1, this->Source->GetFullPath() };
    if (!this->Rule->GetDepends().empty()) {
      depends.insert(depends.end(), this->Rule->GetDepends().begin(),
                     this->Rule->GetDepends().end());
    }
    cc->SetDepends(expandVector(depends));
    if (cmValue deps_explicit =
          this->Rule->GetProperty("DEPENDS_EXPLICIT_ONLY")) {
      cc->SetDependsExplicitOnly(deps_explicit.IsOn());
    } else {
      cc->SetDependsExplicitOnly(this->GetMakefile().IsOn(
        "CMAKE_ADD_CUSTOM_COMMAND_DEPENDS_EXPLICIT_ONLY"));
    }

    if (cmValue wd = this->Rule->GetProperty("WORKING_DIRECTORY")) {
      // store normalized path as property
      this->Properties.SetProperty(
        "WORKING_DIRECTORY",
        expandPath(this->GetMakefile().GetCurrentBinaryDirectory(), *wd));
      cc->SetWorkingDirectory(expandVariables(*wd));
    }

    if (this->Rule->GetProperty("USES_TERMINAL").IsOn() &&
        !this->Rule->GetProperty("JOB_POOL_COMPILE")->empty()) {
      this->GetMakefile().IssueDiagnostic(
        cmDiagnostics::CMD_AUTHOR,
        cmStrCat("RULE \"", this->GetName(), "\": JOB_POOL \"",
                 expandVariables(this->Rule->GetProperty("JOB_POOL_COMPILE")),
                 "\" is shadowed by USES_TERMINAL."));
    }
    if (!this->Rule->GetProperty("JOB_POOL_COMPILE")->empty()) {
      cc->SetJobPool(
        expandVariables(*this->Rule->GetProperty("JOB_POOL_COMPILE")));
    } else if (this->Rule->GetProperty("USES_TERMINAL").IsOn()) {
      cc->SetUsesTerminal(true);
    }

    cc->SetJobserverAware(this->Rule->GetProperty("JOB_SERVER_AWARE").IsOn());

    cc->SetEscapeOldStyle(!this->Rule->GetProperty("VERBATIM").IsOn());
    cc->SetCommandExpandLists(
      this->Rule->GetProperty("COMMAND_EXPAND_LISTS").IsOn());

    if (cmValue comment = this->Rule->GetProperty("COMMENT")) {
      cc->SetComment(expandVariables(*comment));
    }

    return cc;
  } catch (rule_pattern_error const& error) {
    this->GetMakefile().IssueMessage(MessageType::FATAL_ERROR, error.what());
    return nullptr;
  }
}

std::unique_ptr<cmCustomCommand> cmGeneratorRule::Generate(
  cmFileSet* outFileSet) const
{
  auto cc = this->CreateCustomCommand();
  if (!cc) {
    return nullptr;
  }

  // populate file set with outputs from custom command
  for (auto const& output : cc->GetOutputs()) {
    outFileSet->AddFileEntry(BT<std::string>{ output });
  }

  return cc;
}
