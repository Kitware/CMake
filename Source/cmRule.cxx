/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmRule.h"

#include <algorithm>
#include <iterator>
#include <memory>
#include <unordered_map>
#include <utility>

#include <cm/string_view>
#include <cmext/algorithm>
#include <cmext/string_view>

#include "cmsys/RegularExpression.hxx"

#include "cmExecutionStatus.h"
#include "cmFileSet.h"
#include "cmFileSetMetadata.h"
#include "cmGlobalGenerator.h"
#include "cmList.h"
#include "cmListFileCache.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmPlaceholderExpander.h"
#include "cmRange.h"
#include "cmSourceFile.h"
#include "cmStringAlgorithms.h"
#include "cmTarget.h"
#include "cmValue.h"

namespace {
cm::string_view const NAME = "NAME"_s;
cm::string_view const OUTPUT = "OUTPUT"_s;
cm::string_view const COMMAND = "COMMAND"_s;
cm::string_view const COMMAND_COUNT = "COMMAND_COUNT"_s;
cm::string_view const COMMAND_EXPAND_LISTS = "COMMAND_EXPAND_LISTS"_s;
cm::string_view const COMMENT = "COMMENT"_s;
cm::string_view const COMPILE_DEFINITIONS = "COMPILE_DEFINITIONS"_s;
cm::string_view const COMPILE_OPTIONS = "COMPILE_OPTIONS"_s;
cm::string_view const DEPENDS = "DEPENDS"_s;
cm::string_view const DEPENDS_EXPLICIT_ONLY = "DEPENDS_EXPLICIT_ONLY"_s;
cm::string_view const BYPRODUCTS = "BYPRODUCTS"_s;
cm::string_view const DEPFILE = "DEPFILE"_s;
cm::string_view const GLOBAL = "GLOBAL"_s;
cm::string_view const INCLUDE_DIRECTORIES = "INCLUDE_DIRECTORIES"_s;
cm::string_view const JOB_POOL_COMPILE = "JOB_POOL_COMPILE"_s;
cm::string_view const JOB_SERVER_AWARE = "JOB_SERVER_AWARE"_s;
cm::string_view const OUTPUT_FILE_SET = "OUTPUT_FILE_SET"_s;
cm::string_view const PARENT_RULE = "PARENT_RULE"_s;
cm::string_view const FILE_SET_CONFIGURATORS = "FILE_SET_CONFIGURATORS"_s;
cm::string_view const SOURCE_CONFIGURATORS = "SOURCE_CONFIGURATORS"_s;
cm::string_view const USES_TERMINAL = "USES_TERMINAL"_s;
cm::string_view const VERBATIM = "VERBATIM"_s;
cm::string_view const WORKING_DIRECTORY = "WORKING_DIRECTORY"_s;

cmsys::RegularExpression commandIndex("^COMMAND_[0-9]+$");

enum class ReadOnlyCondition
{
  All,
  Configuration,
  Generation,
};

struct ReadOnlyProperty
{
  ReadOnlyProperty(ReadOnlyCondition cond)
    : Condition{ cond }
  {
  }

  ReadOnlyCondition Condition;

  std::string Message(cm::string_view prop, cmRule const* rule) const
  {
    std::string msg;
    switch (this->Condition) {
      case ReadOnlyCondition::All:
        msg = " property is read-only for rules (\"";
        break;
      case ReadOnlyCondition::Configuration:
        msg = " property can't be set during configuration for rules (\"";
        break;
      case ReadOnlyCondition::Generation:
        msg = " property can't be set during generation for rules (\"";
        break;
    }
    return cmStrCat('"', prop, "\" ", msg, rule->GetName(), "\")\n");
  }

  bool IsReadOnly(cm::string_view prop, cmRule const* rule) const
  {
    if ((rule->InGeneration() &&
         this->Condition == ReadOnlyCondition::Configuration) ||
        (!rule->InGeneration() &&
         this->Condition == ReadOnlyCondition::Generation)) {
      return false;
    }

    rule->GetMakefile().IssueMessage(MessageType::FATAL_ERROR,
                                     this->Message(prop, rule));

    return true;
  }
};

bool IsSettableProperty(cm::string_view prop, cmRule const* rule)
{
  using ROC = ReadOnlyCondition;
  static std::unordered_map<cm::string_view, ReadOnlyProperty> const
    readOnlyProps{ { NAME, { ROC::All } },
                   { OUTPUT, { ROC::All } },
                   { COMMAND, { ROC::All } },
                   { COMMAND_COUNT, { ROC::All } },
                   { COMMAND_EXPAND_LISTS, { ROC::Generation } },
                   { COMMENT, { ROC::Generation } },
                   { COMPILE_DEFINITIONS, { ROC::Generation } },
                   { COMPILE_OPTIONS, { ROC::Generation } },
                   { DEPENDS, { ROC::All } },
                   { DEPENDS_EXPLICIT_ONLY, { ROC::Generation } },
                   { BYPRODUCTS, { ROC::All } },
                   { DEPFILE, { ROC::All } },
                   { GLOBAL, { ROC::Generation } },
                   { INCLUDE_DIRECTORIES, { ROC::Generation } },
                   { JOB_POOL_COMPILE, { ROC::Generation } },
                   { JOB_SERVER_AWARE, { ROC::Generation } },
                   { OUTPUT_FILE_SET, { ROC::Generation } },
                   { PARENT_RULE, { ROC::All } },
                   { FILE_SET_CONFIGURATORS, { ROC::All } },
                   { SOURCE_CONFIGURATORS, { ROC::All } },
                   { USES_TERMINAL, { ROC::Generation } },
                   { VERBATIM, { ROC::Generation } },
                   { WORKING_DIRECTORY, { ROC::Generation } } };

  auto it =
    readOnlyProps.find(commandIndex.find(prop.data()) ? COMMAND : prop);

  if (it != readOnlyProps.end()) {
    return !(it->second.IsReadOnly(prop, rule));
  }

  if (rule->InGeneration()) {
    rule->GetMakefile().IssueMessage(
      MessageType::FATAL_ERROR,
      cmStrCat("rule properties cannot be changed during generation (\"",
               rule->GetName(), "\")\n"));
    return false;
  }

  return true;
}

class FileSetNamePlaceholderExpander : public cmPlaceholderExpander
{
public:
  using VariableMap = std::unordered_map<std::string, std::string>;
  VariableMap Values;

  bool InError = false;

private:
  std::string ExpandVariable(std::string const& variable) override
  {
    if (cm::contains(this->Values, variable)) {
      return this->Values[variable];
    }
    this->InError = true;
    // If there is no variable defined, mark unresolved variable by '<' and '>'
    return cmStrCat('<', variable, '>');
  }
};
}

cmRule::cmRule(cmMakefile& makefile, std::string name, cm::RuleScope scope)
  : Makefile(&makefile)
  , Name(std::move(name))
  , Scope(scope)
{
  // set some useful properties
  this->SetProperty("VERBATIM", cmValue::True);
  this->SetProperty("COMMAND_EXPAND_LISTS", cmValue::True);
  this->SetProperty("OUTPUT_FILE_SET",
                    "__cmake_rule_<RULE>_<TARGET>_<FILE_SET>_outputs;SOURCES");
}

cmRule::cmRule(cmRule const& parent, cmMakefile& makefile, std::string name,
               cm::RuleScope scope)
  : Makefile(&makefile)
  , Name(std::move(name))
  , Scope(scope)
  , ConfiguratorsChain(parent.ConfiguratorsChain)
  , Properties(parent.Properties)
  , IncludeDirectories(parent.IncludeDirectories)
  , CompileOptions(parent.CompileOptions)
  , CompileDefinitions(parent.CompileDefinitions)
  , Generation(parent.Generation)
{
}
std::string const& cmRule::GetParentName() const
{
  static std::string empty;

  return empty;
}

void cmRule::SetConfigurator(ConfiguratorType type, std::string configurator,
                             ChainConfigurators chain)
{
  this->Configurators[type] = std::move(configurator);
  if (chain == ChainConfigurators::Yes) {
    this->ConfiguratorsChain[type].emplace_back(&this->GetName(),
                                                &this->Configurators[type]);
  } else {
    this->ConfiguratorsChain[type].assign(
      1,
      ConfiguratorSet::value_type{ &this->GetName(),
                                   &this->Configurators[type] });
  }
}
cmRule::ConfiguratorSet const& cmRule::GetConfigurators(
  ConfiguratorType type) const
{
  static ConfiguratorSet emptySet;

  if (this->HasConfigurators(type)) {
    return this->ConfiguratorsChain.at(type);
  }
  return emptySet;
}

bool cmRule::HasConfigurators(ConfiguratorType type) const
{
  return cm::contains(this->ConfiguratorsChain, type);
}

cmFileSet* cmRule::GetOutputFileSet(cmTarget* target,
                                    cmFileSet const* fileSet) const
{
  cmList outputFileSet{ this->GetProperty(std::string{ OUTPUT_FILE_SET }) };
  FileSetNamePlaceholderExpander expander;
  expander.Values["RULE"] = this->GetName();
  expander.Values["TARGET"] = target->GetName();
  expander.Values["FILE_SET"] = fileSet->GetName();

  std::string fsName{ outputFileSet[0] };
  expander.ExpandVariables(fsName);
  if (expander.InError) {
    this->GetMakefile().IssueMessage(
      MessageType::FATAL_ERROR,
      cmStrCat("Output File Set name, for rule \"", this->GetName(),
               "\", did not expand correctly:\n  \"", fsName, "\"."));
    return nullptr;
  }

  auto result = target->GetOrCreateFileSet(fsName, outputFileSet[1],
                                           fileSet->GetVisibility());
  if (!result.second && result.first->GetType() != outputFileSet[1]) {
    this->GetMakefile().IssueMessage(
      MessageType::FATAL_ERROR,
      cmStrCat("The output file set \"", fsName, "\", for the target \"",
               target->GetName(), "\", has the type \"",
               result.first->GetType(), "\" instead of \"", outputFileSet[1],
               "\", as specified by the rule \"", this->GetName(), "\"."));
    return nullptr;
  }

  return result.first;
}

cmBTStringRange cmRule::GetIncludeDirectories() const
{
  return cmMakeRange(this->IncludeDirectories);
}

cmBTStringRange cmRule::GetCompileOptions() const
{
  return cmMakeRange(this->CompileOptions);
}

cmBTStringRange cmRule::GetCompileDefinitions() const
{
  return cmMakeRange(this->CompileDefinitions);
}

void cmRule::SetProperty(std::string const& prop, cmValue value)
{
  if (!IsSettableProperty(prop, this)) {
    return;
  }

  if (prop == GLOBAL) {
    if (!value.IsOn()) {
      this->GetMakefile().IssueMessage(
        MessageType::FATAL_ERROR,
        cmStrCat("GLOBAL property can't be set to FALSE on rules (\"",
                 this->GetName(), "\")"));
      return;
    }
    /* no need to change anything if value does not change */
    if (!this->IsGloballyVisible()) {
      this->Scope = cm::RuleScope::Global;
      this->GetMakefile().GetGlobalGenerator()->IndexRule(this);
    }
  } else if (prop == OUTPUT_FILE_SET) {
    cmList fileSet{ value };
    if (fileSet.size() != 2) {
      this->GetMakefile().IssueMessage(
        MessageType::FATAL_ERROR,
        "OUTPUT_FILE_SET property require a list of 2 elements:\n  "
        "\"name;type\"");
      return;
    }
    if (!cm::FileSetMetadata::IsKnownType(fileSet[1])) {
      this->GetMakefile().IssueMessage(
        MessageType::FATAL_ERROR,
        cmStrCat(
          "specified file set type is erroneous. The supported types are: ",
          cmJoin(cm::FileSetMetadata::GetKnownTypes(), ", "), '.'));
      return;
    }
    this->Properties.SetProperty(prop, value);
  } else if (prop == INCLUDE_DIRECTORIES) {
    this->IncludeDirectories.clear();
    if (value) {
      cmListFileBacktrace lfbt = this->GetMakefile().GetBacktrace();
      this->IncludeDirectories.emplace_back(value, lfbt);
    }
  } else if (prop == COMPILE_OPTIONS) {
    this->CompileOptions.clear();
    if (value) {
      cmListFileBacktrace lfbt = this->GetMakefile().GetBacktrace();
      this->CompileOptions.emplace_back(value, lfbt);
    }
  } else if (prop == COMPILE_DEFINITIONS) {
    this->CompileDefinitions.clear();
    if (value) {
      cmListFileBacktrace lfbt = this->GetMakefile().GetBacktrace();
      this->CompileDefinitions.emplace_back(value, lfbt);
    }
  } else {
    this->Properties.SetProperty(prop, value);
  }
}

void cmRule::AppendProperty(std::string const& prop, std::string const& value,
                            bool asString)
{
  if (!IsSettableProperty(prop, this)) {
    return;
  }

  if (prop == GLOBAL) {
    this->GetMakefile().IssueMessage(
      MessageType::FATAL_ERROR,
      cmStrCat("GLOBAL property can't be appended, only set on rules (\"",
               this->GetName(), "\")\n"));
    return;
  }

  if (prop == INCLUDE_DIRECTORIES) {
    if (!value.empty()) {
      cmListFileBacktrace lfbt = this->GetMakefile().GetBacktrace();
      this->IncludeDirectories.emplace_back(value, lfbt);
    }
  } else if (prop == COMPILE_OPTIONS) {
    if (!value.empty()) {
      cmListFileBacktrace lfbt = this->GetMakefile().GetBacktrace();
      this->CompileOptions.emplace_back(value, lfbt);
    }
  } else if (prop == COMPILE_DEFINITIONS) {
    if (!value.empty()) {
      cmListFileBacktrace lfbt = this->GetMakefile().GetBacktrace();
      this->CompileDefinitions.emplace_back(value, lfbt);
    }
  } else {
    this->Properties.AppendProperty(prop, value, asString);
  }
}

cmValue cmRule::GetProperty(std::string const& prop) const
{
  static std::string value;

  if (prop == NAME) {
    return cmValue{ this->GetName() };
  }
  if (prop == OUTPUT) {
    value = cmList::to_string(this->GetOutputs());
    return cmValue{ value };
  }
  if (prop == COMMAND) {
    value = cmList::to_string(this->GetCommands()[0]);
    return cmValue{ value };
  }
  if (prop == COMMAND_COUNT) {
    value = std::to_string(this->GetCommands().size());
    return cmValue{ value };
  }
  if (commandIndex.find(prop)) {
    auto index = std::stoul(prop.substr(8));
    value = index >= this->GetCommands().size()
      ? "NOTFOUND"
      : cmList::to_string(this->GetCommands()[index]);
    return cmValue{ value };
  }
  if (prop == DEPENDS) {
    value = cmList::to_string(this->GetDepends());
    return cmValue{ value };
  }
  if (prop == BYPRODUCTS) {
    value = cmList::to_string(this->GetByproducts());
    return cmValue{ value };
  }
  if (prop == DEPFILE) {
    return cmValue{ this->GetDepfile() };
  }
  if (prop == GLOBAL) {
    return this->IsGloballyVisible() ? cmValue::True : cmValue::False;
  }
  if (prop == PARENT_RULE) {
    return cmValue{ this->GetParentName() };
  }
  if (prop == FILE_SET_CONFIGURATORS || prop == SOURCE_CONFIGURATORS) {
    ConfiguratorType type = prop == FILE_SET_CONFIGURATORS
      ? ConfiguratorType::FileSet
      : ConfiguratorType::Source;

    if (this->HasConfigurators(type)) {
      auto cfgs_set = this->GetConfigurators(type);
      std::vector<std::string> cfgs;
      cfgs.reserve(cfgs_set.size());

      std::transform(cfgs_set.cbegin(), cfgs_set.cend(),
                     std::back_inserter(cfgs),
                     [](ConfiguratorSet::value_type cfg) -> std::string {
                       return *cfg.second;
                     });

      value = cmList::to_string(cfgs);
      return cmValue{ value };
    }

    return nullptr;
  }

  // Check for the properties with backtraces.
  if (prop == INCLUDE_DIRECTORIES) {
    if (this->IncludeDirectories.empty()) {
      return nullptr;
    }

    value = cmList::to_string(this->IncludeDirectories);
    return cmValue{ value };
  }

  if (prop == COMPILE_OPTIONS) {
    if (this->CompileOptions.empty()) {
      return nullptr;
    }

    value = cmList::to_string(this->CompileOptions);
    return cmValue{ value };
  }

  if (prop == COMPILE_DEFINITIONS) {
    if (this->CompileDefinitions.empty()) {
      return nullptr;
    }

    value = cmList::to_string(this->CompileDefinitions);
    return cmValue{ value };
  }

  return this->Properties.GetPropertyValue(prop);
}

void cmRule::CheckProperty(std::string const& prop, cmMakefile& context) const
{
  // Certain properties need checking.
  if (prop == GLOBAL) {
    auto const& rules = context.GetOwnedRules();
    auto it = std::find_if(rules.begin(), rules.end(),
                           [&](std::unique_ptr<cmRule> const& rule) -> bool {
                             return this == rule.get();
                           });
    if (it == rules.end()) {
      context.IssueMessage(
        MessageType::FATAL_ERROR,
        cmStrCat("Attempt to promote rule \"", this->GetName(),
                 "\" to global scope (by setting GLOBAL) "
                 "which is not created in this directory."));
    }
  }
}

namespace {
cmsys::RegularExpression PatternRegex{ "(^[A-Z][A-Z0-9_]+)=(.*)$" };

void UpdatePatterns(cmRule::PatternSet& patterns, cmList const& newPatterns)
{
  for (auto const& pattern : newPatterns) {
    if (PatternRegex.find(pattern)) {
      auto it = std::find_if(patterns.begin(), patterns.end(),
                             [](cmRule::Pattern const& item) {
                               return item.Name == PatternRegex.match(1);
                             });
      if (it == patterns.end()) {
        patterns.emplace_back(PatternRegex.match(1), PatternRegex.match(2));
      } else {
        it->Value = PatternRegex.match(2);
      }
    }
  }
}

bool ConfigureRule(cmRule::ConfiguratorSet const& configurators,
                   cmMakefile* mf, cmRule::PatternSet& patterns,
                   std::string const& rule, std::string const& target,
                   std::string const& fileSet,
                   std::string const& outputFileSet,
                   std::string const& source = {})
{
  cm::string_view patternsVariable{ "_CMAKE_RULE_PATTERNS"_s };

  for (auto const& item : configurators) {
    // The validator command will be executed in an isolated scope.
    cmMakefile::ScopePushPop varScope(mf);
    cmMakefile::PolicyPushPop polScope(mf);
    static_cast<void>(varScope);
    static_cast<void>(polScope);

    std::vector<cmListFileArgument> args{
      cmListFileArgument{ rule, cmListFileArgument::Unquoted, 0 },
      cmListFileArgument{ target, cmListFileArgument::Unquoted, 0 },
      cmListFileArgument{ fileSet, cmListFileArgument::Unquoted, 0 },
      cmListFileArgument{ outputFileSet, cmListFileArgument::Unquoted, 0 }
    };
    if (!source.empty()) {
      args.emplace_back(source, cmListFileArgument::Quoted, 0);
    }
    args.emplace_back(patternsVariable, cmListFileArgument::Unquoted, 0);

    cmListFileFunction command(*item.second, 0, 0, args);
    cmExecutionStatus status(*mf);
    if (!mf->ExecuteCommand(command, status)) {
      mf->IssueMessage(MessageType::FATAL_ERROR,
                       cmStrCat("Erroneous execution of the CONFIGURATOR \"",
                                *item.second, "\", from the RULE \"",
                                *item.first, "\", for the FILE_SET \"",
                                fileSet, "\" of TARGET \"", target, "\"."));
      return false;
    }
    UpdatePatterns(
      patterns, cmList{ mf->GetDefinition(std::string{ patternsVariable }) });
  }

  return true;
}
}

bool cmRule::Instantiate(cmTarget const* target, cmFileSet const* fileSet,
                         cmFileSet const* outputFileSet,
                         PatternSet& patterns) const
{
  this->SwitchMode();

  // First, take patterns from RULE_PATTERNS file set property, if any
  if (cmValue fsPatterns = fileSet->GetProperty("RULE_PATTERNS")) {
    UpdatePatterns(patterns, cmList{ fsPatterns, cmList::EmptyElements::No });
  }

  if (this->HasConfigurators(ConfiguratorType::FileSet)) {
    // Finalize file set level configuration by calling user's commands
    return ConfigureRule(this->GetConfigurators(ConfiguratorType::FileSet),
                         fileSet->GetMakefile(), patterns, this->GetName(),
                         target->GetName(), fileSet->GetName(),
                         outputFileSet->GetName());
  }

  return true;
}

bool cmRule::Instantiate(cmTarget const* target, cmFileSet const* fileSet,
                         cmFileSet const* outputFileSet, cmSourceFile* source,
                         PatternSet& patterns) const
{
  // First, take patterns from <RULE>_PATTERNS source file property, if any
  if (cmValue sfPatterns =
        source->GetProperty(cmStrCat(this->GetName(), "_PATTERNS"))) {
    UpdatePatterns(patterns, cmList{ sfPatterns, cmList::EmptyElements::No });
  }

  if (this->HasConfigurators(ConfiguratorType::Source)) {
    // Before custom commands generation, finalize source level
    // configuration by calling user's commands
    return ConfigureRule(this->GetConfigurators(ConfiguratorType::Source),
                         fileSet->GetMakefile(), patterns, this->GetName(),
                         target->GetName(), fileSet->GetName(),
                         outputFileSet->GetName(), source->GetFullPath());
  }

  return true;
}
