/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <cstddef>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#include "cmAlgorithms.h"
#include "cmListFileCache.h"
#include "cmPropertyMap.h"
#include "cmValue.h"

class cmMakefile;
class cmTarget;
class cmFileSet;
class cmSourceFile;

namespace cm {

enum class RuleScope
{
  Local,
  Global
};

template <typename E>
struct enum_hash
{
  typename std::enable_if<std::is_enum<E>::value, std::size_t>::type
  operator()(E const key) const
  {
    return static_cast<std::size_t>(key);
  }
};
}

//
// cmRule: base class for various rule types
//
class cmRule
{
public:
  using CustomCommand = std::vector<std::string>;
  using CustomCommands = std::vector<CustomCommand>;

  virtual ~cmRule() = default;

  cmRule& operator=(cmRule const&) = delete;

  cmMakefile& GetMakefile() const { return *this->Makefile; }

  /** Get the name of the rule */
  std::string const& GetName() const { return this->Name; }
  virtual std::string const& GetParentName() const;

  /** Get the command lines.  */
  virtual CustomCommands const& GetCommands() const = 0;
  /** Get the output files.  */
  virtual std::vector<std::string> const& GetOutputs() const = 0;

  /** Get the extra files produced by the command.  */
  virtual std::vector<std::string> const& GetByproducts() const = 0;

  /** Get the vector that holds the list of dependencies. */
  virtual std::vector<std::string> const& GetDepends() const = 0;
  /** Get the file that holds the list of dependencies. */
  virtual std::string const& GetDepfile() const = 0;

  enum class ConfiguratorType
  {
    FileSet,
    Source
  };
  enum class ChainConfigurators
  {
    No,
    Yes
  };
  using ConfiguratorSet =
    std::vector<std::pair<std::string const*, std::string const*>>;
  void SetConfigurator(ConfiguratorType type, std::string configurator,
                       ChainConfigurators chain = ChainConfigurators::No);
  ConfiguratorSet const& GetConfigurators(ConfiguratorType type) const;

  bool HasConfigurators(ConfiguratorType type) const;

  bool IsGloballyVisible() const
  {
    return this->Scope == cm::RuleScope::Global;
  }

  cmFileSet* GetOutputFileSet(cmTarget* target,
                              cmFileSet const* fileSet) const;

  // Special properties
  cmBTStringRange GetIncludeDirectories() const;

  cmBTStringRange GetCompileOptions() const;

  cmBTStringRange GetCompileDefinitions() const;

  //! Set/Get a property of this rule
  void SetProperty(std::string const& prop, cmValue value);
  void SetProperty(std::string const& prop, std::nullptr_t)
  {
    this->SetProperty(prop, cmValue{ nullptr });
  }
  void RemoveProperty(std::string const& prop)
  {
    this->SetProperty(prop, cmValue{ nullptr });
  }
  void SetProperty(std::string const& prop, std::string const& value)
  {
    this->SetProperty(prop, cmValue{ value });
  }
  void AppendProperty(std::string const& prop, std::string const& value,
                      bool asString = false);
  cmValue GetProperty(std::string const& prop) const;

  void CheckProperty(std::string const& prop, cmMakefile& context) const;

  bool InGeneration() const { return this->Generation; }

  struct Pattern
  {
    Pattern(std::string name, std::string value)
      : Name(std::move(name))
      , Value(std::move(value))
    {
    }

    std::string Name;
    std::string Value;
  };
  using PatternSet = std::vector<Pattern>;
  bool Instantiate(cmTarget const* target, cmFileSet const* fileSet,
                   cmFileSet const* outputFileSet, PatternSet& patterns) const;
  bool Instantiate(cmTarget const* target, cmFileSet const* fileSet,
                   cmFileSet const* outputFileSet, cmSourceFile* source,
                   PatternSet& patterns) const;

protected:
  cmRule(cmMakefile& makefile, std::string name, cm::RuleScope scope);
  cmRule(cmRule const& parent, cmMakefile& makefile, std::string name,
         cm::RuleScope scope);

private:
  // switch context cmRule usage
  void SwitchMode() const { this->Generation = true; }

  cmMakefile* Makefile;
  std::string Name;
  cm::RuleScope Scope = cm::RuleScope::Local;
  std::unordered_map<ConfiguratorType, std::string,
                     cm::enum_hash<ConfiguratorType>>
    Configurators;
  std::unordered_map<ConfiguratorType, ConfiguratorSet,
                     cm::enum_hash<ConfiguratorType>>
    ConfiguratorsChain;
  cmPropertyMap Properties;
  std::vector<BT<std::string>> IncludeDirectories;
  std::vector<BT<std::string>> CompileOptions;
  std::vector<BT<std::string>> CompileDefinitions;
  // cmRule properties should not be changed during generation phase
  // Use mutable field to track current phase independently of state of object
  // (const or not)
  mutable bool Generation = false;
};

//
// cmCustomRule: define a template used to generate custom commands
//
class cmCustomRule : public cmRule
{
public:
  cmCustomRule(cmMakefile& makefile, std::string name, CustomCommands commands,
               std::vector<std::string> outputs,
               cm::RuleScope scope = cm::RuleScope::Local);

  cmCustomRule& operator=(cmCustomRule const&) = delete;

  /** Get the command lines.  */
  CustomCommands const& GetCommands() const override;
  /** Get the output files.  */
  std::vector<std::string> const& GetOutputs() const override;

  /** Set/Get the extra files produced by the command.  */
  void SetByproducts(std::vector<std::string> byproducts);
  std::vector<std::string> const& GetByproducts() const override;

  /** Set/Get the vector that holds the list of dependencies. */
  void SetDepends(std::vector<std::string> depends);
  std::vector<std::string> const& GetDepends() const override;
  /** Set/Get the file that holds the list of dependencies. */
  void SetDepfile(std::string depfile);
  std::string const& GetDepfile() const override;

private:
  CustomCommands Commands;
  std::vector<std::string> Outputs;
  std::vector<std::string> Byproducts;
  std::vector<std::string> Depends;
  std::string Depfile;
};

//
// cmSpecializedRule: rule embedding another rule and enabling properties
// customization.
//
class cmSpecializedRule : public cmRule
{
public:
  cmSpecializedRule(cmMakefile& makefile, std::string name,
                    cmRule const& parent,
                    cm::RuleScope scope = cm::RuleScope::Local);

  cmSpecializedRule& operator=(cmSpecializedRule const&) = delete;

  std::string const& GetParentName() const override;

  /** Get the command lines.  */
  CustomCommands const& GetCommands() const override;
  /** Get the output files.  */
  std::vector<std::string> const& GetOutputs() const override;

  /** Get the extra files produced by the command.  */
  std::vector<std::string> const& GetByproducts() const override;

  /** Get the vector that holds the list of dependencies. */
  std::vector<std::string> const& GetDepends() const override;
  /** Get the file that holds the list of dependencies. */
  std::string const& GetDepfile() const override;

private:
  cmRule const& ParentRule;
};
