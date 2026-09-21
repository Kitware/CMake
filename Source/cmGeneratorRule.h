/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <memory>
#include <string>
#include <unordered_map>

#include "cmPlaceholderExpander.h"
#include "cmPropertyMap.h"
#include "cmRule.h"
#include "cmValue.h"

class cmMakefile;
class cmTarget;
class cmFileSet;
class cmSourceFile;
class cmCustomCommand;

class cmGeneratorRule
{
public:
  cmGeneratorRule(cmRule const* rule, cmTarget const* target,
                  cmFileSet const* fileSet, cmFileSet const* outputFileSet,
                  cmSourceFile const* source,
                  cmRule::PatternSet const& patterns);

  cmGeneratorRule(cmGeneratorRule const&) = delete;
  cmGeneratorRule& operator=(cmGeneratorRule const&) = delete;

  cmMakefile& GetMakefile() const { return this->Makefile; }

  /** Get the name of the rule */
  std::string const& GetName() const { return this->Name; }

  bool IsGloballyVisible() const { return this->Rule->IsGloballyVisible(); }

  cmValue GetProperty(std::string const& property) const;

  std::unique_ptr<cmCustomCommand> Generate(cmFileSet* outFileSet) const;

private:
  void UpdateRuleExpander(cmRule::PatternSet const& patterns);

  std::unique_ptr<cmCustomCommand> CreateCustomCommand() const;

  std::string Name;
  cmRule const* Rule;
  cmTarget const* Target;
  cmFileSet const* FileSet;
  cmFileSet const* OutputFileSet;
  cmSourceFile const* Source;
  cmMakefile& Makefile;
  mutable cmPropertyMap Properties;

  class RulePlaceholderExpander : public cmPlaceholderExpander
  {
  public:
    using VariableMap = std::unordered_map<std::string, std::string>;
    VariableMap Values;

    std::string ExpandVariables(std::string const& string)
    {
      std::string value{ string };
      return cmPlaceholderExpander::ExpandVariables(
        value, cmPlaceholderExpander::HandleGenex::Yes);
    }

  private:
    std::string ExpandVariable(std::string const& variable) override;
  };

  static RulePlaceholderExpander::VariableMap const DefaultProperties;

  mutable RulePlaceholderExpander RuleExpander;
};
