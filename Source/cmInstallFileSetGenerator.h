/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include <iosfwd>
#include <map>
#include <string>
#include <vector>

#include "cmInstallGenerator.h"

class cmGeneratorTarget;
class cmFileSet;
class cmListFileBacktrace;
class cmLocalGenerator;

struct cmFileSetDestinations
{
  std::string Headers;
  std::string CXXModules;
};

class cmInstallFileSetGenerator : public cmInstallGenerator
{
public:
  cmInstallFileSetGenerator(std::string targetName, std::string fileSetName,
                            cmFileSetDestinations dests,
                            std::string file_permissions,
                            std::vector<std::string> const& configurations,
                            std::string const& component, MessageLevel message,
                            bool exclude_from_all, bool optional,
                            cmListFileBacktrace backtrace);
  ~cmInstallFileSetGenerator() override;

  bool Compute(cmLocalGenerator* lg) override;

  std::string GetDestination(std::string const& config) const;
  std::string GetDestination() const { return this->Destination; }
  bool GetOptional() const { return this->Optional; }
  std::string GetFileSetName() const { return this->FileSetName; }
  cmFileSet const* GetFileSet() const { return this->FileSet; };
  cmGeneratorTarget* GetTarget() const { return this->Target; }

protected:
  void GenerateScriptForConfig(std::ostream& os, std::string const& config,
                               Indent indent) override;

private:
  std::string TargetName;
  cmLocalGenerator* LocalGenerator;
  cmFileSet const* FileSet;
  std::string const FileSetName;
  std::string const FilePermissions;
  cmFileSetDestinations FileSetDestinations;
  bool const Optional;
  cmGeneratorTarget* Target;

  std::map<std::string, std::vector<std::string>> CalculateFilesPerDir(
    std::string const& config) const;
};
