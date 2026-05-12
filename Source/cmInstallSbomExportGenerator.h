/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include <string>
#include <vector>

#include "cmInstallExportGenerator.h"

class cmDiagnosticContext;
class cmExportSet;
class cmSbomArguments;

class cmInstallSbomExportGenerator final : public cmInstallExportGenerator
{
public:
  cmInstallSbomExportGenerator(cmExportSet* exportSet, std::string destination,
                               std::string filePermissions,
                               std::vector<std::string> const& configurations,
                               std::string component, MessageLevel message,
                               bool excludeFromAll, cmSbomArguments arguments,
                               std::string cxxModulesDirectory,
                               cmDiagnosticContext context);
  cmInstallSbomExportGenerator(cmInstallSbomExportGenerator const&) = delete;
  ~cmInstallSbomExportGenerator() override;

  cmInstallSbomExportGenerator& operator=(
    cmInstallSbomExportGenerator const&) = delete;

  char const* InstallSubcommand() const override { return "SBOM"; }
};
