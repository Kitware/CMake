/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include <iosfwd>
#include <string>
#include <vector>

#include "cmInstallGenerator.h"
#include "cmListFileCache.h"
#include "cmScriptGenerator.h"

class cmGeneratorTarget;
class cmLocalGenerator;

class cmInstallImportedRuntimeArtifactsGenerator : public cmInstallGenerator
{
public:
  cmInstallImportedRuntimeArtifactsGenerator(
    std::string targetName, std::string const& dest,
    std::string file_permissions,
    std::vector<std::string> const& configurations,
    std::string const& component, MessageLevel message, bool exclude_from_all,
    bool optional, cmListFileBacktrace backtrace = cmListFileBacktrace());
  ~cmInstallImportedRuntimeArtifactsGenerator() override = default;

  bool Compute(cmLocalGenerator* lg) override;

  cmGeneratorTarget* GetTarget() const { return this->Target; }

  bool GetOptional() const { return this->Optional; }

  std::string GetDestination(std::string const& config) const;

protected:
  void GenerateScriptForConfig(std::ostream& os, const std::string& config,
                               Indent indent) override;

private:
  std::string const TargetName;
  cmGeneratorTarget* Target;
  std::string const FilePermissions;
  bool const Optional;
};
