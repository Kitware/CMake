
/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmGeneratorRule.h"

#include <algorithm>
#include <iterator>
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
#include "cmSourceFile.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmTarget.h"

namespace {
std::vector<cm::string_view> ReservedPatterns{ "RULE"_s,
                                               "TARGET"_s,
                                               "FILE_SET"_s,
                                               "SOURCE_DIR"_s,
                                               "BINARY_DIR"_s,
                                               "CURRENT_SOURCE_DIR"_s,
                                               "CURRENT_BINARY_DIR"_s,
                                               "SOURCE"_s,
                                               "INPUT_DIR"_s,
                                               "FILE_NAME"_s,
                                               "BASE_NAME"_s,
                                               "INCLUDE_DIRECTORIES"_s,
                                               "COMPILE_OPTIONS"_s,
                                               "COMPILE_DEFINITIONS"_s };
}

std::string cmGeneratorRule::RulePlaceholderExpander::ExpandVariable(
  std::string const& variable)
{
  if (cm::contains(this->Values, variable)) {
    return this->Values[variable];
  }
  // If there is no variable defined, mark unresolved variable by '{' and '}'
  return cmStrCat('{', variable, '}');
}

cmGeneratorRule::RulePlaceholderExpander::VariableMap const
  cmGeneratorRule::DefaultProperties{
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
                                 cmRule::PatternSet const& patterns)
  : Rule(rule)
  , Target(target)
  , FileSet(fileSet)
  , OutputFileSet(outputFileSet)
  , Source(source)
  , Makefile(*fileSet->GetMakefile())
{
  this->Name = cmStrCat(rule->GetName(), '_',
                        std::hash<std::string>{}(cmStrCat(
                          rule->GetName(), '-', target->GetName(), '-',
                          fileSet->GetName(), '-', source->GetFullPath())));

  auto& values = this->RuleExpander.Values;

  cmCMakePath file = cmCMakePath{ this->Source->GetFullPath() }.Normal();
  cmCMakePath sourceDir = file.IsAbsolute()
    ? file.GetParentPath()
    : this->GetMakefile().GetCurrentSourceDirectory();

  /* clang-format off */
  values.insert({ "RULE", this->GetName() });
  values.insert({ "TARGET", this->Target->GetName() });
  values.insert({ "FILE_SET", this->FileSet->GetName() });
  values.insert({ "SOURCE_DIR", this->GetMakefile().GetHomeDirectory() });
  values.insert({ "BINARY_DIR", this->GetMakefile().GetHomeOutputDirectory() });
  values.insert({ "CURRENT_SOURCE_DIR", this->GetMakefile().GetCurrentSourceDirectory() });
  values.insert({ "CURRENT_BINARY_DIR", this->GetMakefile().GetCurrentBinaryDirectory() });
  values.insert({ "SOURCE", file.GenericString() });
  values.insert({ "INPUT_DIR", file.GetParentPath().GenericString() });
  values.insert({ "FILE_NAME", file.GetFileName().GenericString() });
  values.insert({ "BASE_NAME", file.GetFileName().RemoveWideExtension().GenericString() });
  /* clang-format on */

  // instantiate default properties pattern
  for (auto const& item : DefaultProperties) {
    values.insert(
      { item.first, this->RuleExpander.ExpandVariables(item.second) });
  }

  this->UpdateRuleExpander(patterns);
}

void cmGeneratorRule::UpdateRuleExpander(cmRule::PatternSet const& patterns)
{
  if (patterns.empty()) {
    return;
  }

  auto& values = this->RuleExpander.Values;

  for (cmRule::Pattern const& pattern : patterns) {
    if (cm::contains(ReservedPatterns, pattern.Name)) {
      // overwriting a reserved pattern is not allowed
      continue;
    }

    std::string data{ pattern.Value };
    values[pattern.Name] = this->RuleExpander.ExpandVariables(data);
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
    return this->RuleExpander.ExpandVariables(item);
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

  auto expandPaths =
    [&expandVariables](
      std::string const& binaryDirectory,
      std::vector<std::string> const& data) -> std::vector<std::string> {
    std::vector<std::string> result;
    result.reserve(data.size());
    std::transform(
      data.begin(), data.end(), std::back_inserter(result),
      [&expandVariables,
       &binaryDirectory](std::string const& item) -> std::string {
        std::string path = expandVariables(item);
        if (!cmSystemTools::FileIsFullPath(path) &&
            !cmGeneratorExpression::StartsWithGeneratorExpression(path)) {
          path = cmStrCat(binaryDirectory, '/', path);
        }
        cmSystemTools::ConvertToUnixSlashes(path);
        if (cmSystemTools::FileIsFullPath(path)) {
          path = cmSystemTools::CollapseFullPath(path);
        }
        return path;
      });
    return result;
  };

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
}

std::unique_ptr<cmCustomCommand> cmGeneratorRule::Generate(
  cmFileSet* outFileSet) const
{
  auto cc = this->CreateCustomCommand();

  // populate file set with outputs from custom command
  for (auto const& output : cc->GetOutputs()) {
    outFileSet->AddFileEntry(BT<std::string>{ output });
    // get the directory of the generated file
    outFileSet->AddDirectoryEntry(
      BT<std::string>{ cmStrCat("$<PATH:GET_PARENT_PATH,", output, '>') });
  }

  return cc;
}
