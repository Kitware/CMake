/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include <iosfwd>
#include <set>
#include <string>
#include <vector>

#include <cm/optional>

#include "cmGlobalVisualStudio71Generator.h"

class cmGeneratorTarget;
class cmMakefile;
class cmake;
struct cmIDEFlagTable;

/** \class cmGlobalVisualStudio8Generator
 * \brief Write a Unix makefiles.
 *
 * cmGlobalVisualStudio8Generator manages UNIX build process for a tree
 */
class cmGlobalVisualStudio8Generator : public cmGlobalVisualStudio71Generator
{
public:
  //! Get the name for the generator.
  std::string GetName() const override { return this->Name; }

  /** Get the name of the main stamp list file. */
  static std::string GetGenerateStampList();

  void EnableLanguage(std::vector<std::string> const& languages, cmMakefile*,
                      bool optional) override;
  virtual void AddPlatformDefinitions(cmMakefile* mf);

  bool SetGeneratorPlatform(std::string const& p, cmMakefile* mf) override;

  cm::optional<std::string> const& GetTargetFrameworkVersion() const;
  cm::optional<std::string> const& GetTargetFrameworkIdentifier() const;
  cm::optional<std::string> const& GetTargetFrameworkTargetsVersion() const;

  /** Return true if the target project file should have the option
      LinkLibraryDependencies and link to .sln dependencies. */
  bool NeedLinkLibraryDependencies(cmGeneratorTarget* target) override;

  /** Return true if building for Windows CE */
  bool TargetsWindowsCE() const override
  {
    return !this->WindowsCEVersion.empty();
  }

protected:
  cmGlobalVisualStudio8Generator(cmake* cm, std::string const& name);

  virtual bool ProcessGeneratorPlatformField(std::string const& key,
                                             std::string const& value);

  void AddExtraIDETargets() override;

  std::string FindDevEnvCommand() override;

  bool VSLinksDependencies() const override { return false; }

  bool AddCheckTarget();

  /** Return true if the configuration needs to be deployed */
  virtual bool NeedsDeploy(cmGeneratorTarget const& target,
                           char const* config) const;

  /** Returns true if the target system support debugging deployment. */
  virtual bool TargetSystemSupportsDeployment() const;

  static cmIDEFlagTable const* GetExtraFlagTableVS8();
  void WriteSolutionConfigurations(
    std::ostream& fout, std::vector<std::string> const& configs) override;
  void WriteProjectConfigurations(
    std::ostream& fout, std::string const& name,
    cmGeneratorTarget const& target, std::vector<std::string> const& configs,
    std::set<std::string> const& configsPartOfDefaultBuild,
    std::string const& platformMapping = "") override;
  bool ComputeTargetDepends() override;
  void WriteProjectDepends(std::ostream& fout, std::string const& name,
                           std::string const& path,
                           cmGeneratorTarget const* t) override;

  bool UseFolderProperty() const override;

  std::string Name;
  std::string WindowsCEVersion;

  cm::optional<std::string> DefaultTargetFrameworkVersion;
  cm::optional<std::string> DefaultTargetFrameworkIdentifier;
  cm::optional<std::string> DefaultTargetFrameworkTargetsVersion;

private:
  bool ParseGeneratorPlatform(std::string const& is, cmMakefile* mf);
};
