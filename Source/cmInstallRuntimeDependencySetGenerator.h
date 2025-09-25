/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include <iosfwd>
#include <string>
#include <vector>

#include "cmInstallGenerator.h"

class cmInstallRuntimeDependencySet;
class cmListFileBacktrace;
class cmLocalGenerator;

class cmInstallRuntimeDependencySetGenerator : public cmInstallGenerator
{
public:
  enum class DependencyType
  {
    Library,
    Framework,
  };

  cmInstallRuntimeDependencySetGenerator(
    DependencyType type, cmInstallRuntimeDependencySet* dependencySet,
    std::vector<std::string> installRPaths, bool noInstallRPath,
    std::string installNameDir, bool noInstallName, char const* depsVar,
    char const* rpathPrefix, char const* tmpVarPrefix, std::string destination,
    std::vector<std::string> const& configurations, std::string component,
    std::string permissions, MessageLevel message, bool exclude_from_all,
    cmListFileBacktrace backtrace);

  bool Compute(cmLocalGenerator* lg) override;

  DependencyType GetDependencyType() const { return this->Type; }

  cmInstallRuntimeDependencySet* GetRuntimeDependencySet() const
  {
    return this->DependencySet;
  }

  std::string GetDestination(std::string const& config) const;

protected:
  void GenerateScriptForConfig(std::ostream& os, std::string const& config,
                               Indent indent) override;

private:
  DependencyType Type;
  cmInstallRuntimeDependencySet* DependencySet;
  std::vector<std::string> InstallRPaths;
  bool NoInstallRPath;
  std::string InstallNameDir;
  bool NoInstallName;
  std::string Permissions;
  char const* DepsVar;
  char const* RPathPrefix;
  char const* TmpVarPrefix;
  cmLocalGenerator* LocalGenerator = nullptr;

  void GenerateAppleLibraryScript(
    std::ostream& os, std::string const& config,
    std::vector<std::string> const& evaluatedRPaths, Indent indent);
  void GenerateAppleFrameworkScript(
    std::ostream& os, std::string const& config,
    std::vector<std::string> const& evaluatedRPaths, Indent indent);
  void GenerateInstallNameFixup(
    std::ostream& os, std::string const& config,
    std::vector<std::string> const& evaluatedRPaths,
    std::string const& filename, std::string const& depName, Indent indent);
  void GenerateStripFixup(std::ostream& os, std::string const& config,
                          std::string const& depName, Indent indent);
};
