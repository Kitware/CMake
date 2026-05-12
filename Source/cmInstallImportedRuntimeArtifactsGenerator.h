/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include <iosfwd>
#include <string>
#include <vector>

#include "cmDiagnosticContext.h"
#include "cmInstallGenerator.h"

class cmGeneratorTarget;

class cmInstallImportedRuntimeArtifactsGenerator : public cmInstallGenerator
{
public:
  cmInstallImportedRuntimeArtifactsGenerator(
    std::string targetName, std::string const& dest,
    std::string filePermissions,
    std::vector<std::string> const& configurations,
    std::string const& component, MessageLevel message, bool excludeFromAll,
    bool optional, cmDiagnosticContext context = {});
  ~cmInstallImportedRuntimeArtifactsGenerator() override = default;

  bool Compute(cmLocalGenerator* lg) override;

  cmGeneratorTarget* GetTarget() const { return this->Target; }

  bool GetOptional() const { return this->Optional; }

  std::string GetDestination(std::string const& config) const;

protected:
  void GenerateScriptForConfig(std::ostream& os, std::string const& config,
                               Indent indent) override;

private:
  std::string const TargetName;
  cmGeneratorTarget* Target;
  std::string const FilePermissions;
  bool const Optional;
};
