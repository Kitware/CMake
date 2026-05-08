/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include <iosfwd>
#include <map>
#include <string>
#include <vector>

#include "cmInstallGenerator.h"

class cmDiagnosticContext;
class cmExportInstallCMakeConfigGenerator;
class cmGeneratorFileSet;
class cmGeneratorTarget;
class cmLocalGenerator;

class cmInstallFileSetGenerator : public cmInstallGenerator
{
public:
  cmInstallFileSetGenerator(std::string targetName, std::string fileSetName,
                            std::string destination,
                            std::string filePermissions,
                            std::vector<std::string> const& configurations,
                            std::string const& component, MessageLevel message,
                            bool excludeFromAll, bool optional,
                            cmDiagnosticContext context);
  ~cmInstallFileSetGenerator() override;

  bool Compute(cmLocalGenerator* lg) override;

  struct DestinationContext
  {
    std::string UnescapedDestination;
    bool HadContextSensitiveCondition;
  };
  std::string GetDestination(std::string const& config) const;
  DestinationContext GetDestination(cmGeneratorTarget* gt,
                                    std::string const& config) const;
  bool GetOptional() const { return this->Optional; }
  std::string GetFileSetName() const { return this->FileSetName; }
  cmGeneratorFileSet const* GetFileSet() const { return this->FileSet; };
  cmGeneratorTarget* GetTarget() const { return this->Target; }

protected:
  friend cmExportInstallCMakeConfigGenerator;
  std::string GetDestination() const;

  void GenerateScriptForConfig(std::ostream& os, std::string const& config,
                               Indent indent) override;

private:
  std::string TargetName;
  cmLocalGenerator* LocalGenerator;
  cmGeneratorFileSet const* FileSet;
  std::string const FileSetName;
  std::string const FilePermissions;
  bool const Optional;
  cmGeneratorTarget* Target;

  std::map<std::string, std::vector<std::string>> CalculateFilesPerDir(
    std::string const& config) const;
};
