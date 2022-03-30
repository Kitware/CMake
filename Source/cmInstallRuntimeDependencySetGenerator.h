/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include <iosfwd>
#include <string>
#include <vector>

#include "cmInstallGenerator.h"
#include "cmScriptGenerator.h"

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
    std::string installNameDir, bool noInstallName, const char* depsVar,
    const char* rpathPrefix, const char* tmpVarPrefix, std::string destination,
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
  void GenerateScriptForConfig(std::ostream& os, const std::string& config,
                               Indent indent) override;

private:
  DependencyType Type;
  cmInstallRuntimeDependencySet* DependencySet;
  std::vector<std::string> InstallRPaths;
  bool NoInstallRPath;
  std::string InstallNameDir;
  bool NoInstallName;
  std::string Permissions;
  const char* DepsVar;
  const char* RPathPrefix;
  const char* TmpVarPrefix;
  cmLocalGenerator* LocalGenerator = nullptr;

  void GenerateAppleLibraryScript(
    std::ostream& os, const std::string& config,
    const std::vector<std::string>& evaluatedRPaths, Indent indent);
  void GenerateAppleFrameworkScript(
    std::ostream& os, const std::string& config,
    const std::vector<std::string>& evaluatedRPaths, Indent indent);
  void GenerateInstallNameFixup(
    std::ostream& os, const std::string& config,
    const std::vector<std::string>& evaluatedRPaths,
    const std::string& filename, const std::string& depName, Indent indent);
  void GenerateStripFixup(std::ostream& os, const std::string& config,
                          const std::string& depName, Indent indent);
};
