/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include <string>
#include <vector>

#include "cmInstallExportGenerator.h"

class cmExportSet;
class cmListFileBacktrace;
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
                               cmListFileBacktrace backtrace);
  cmInstallSbomExportGenerator(cmInstallSbomExportGenerator const&) = delete;
  ~cmInstallSbomExportGenerator() override;

  cmInstallSbomExportGenerator& operator=(
    cmInstallSbomExportGenerator const&) = delete;

  char const* InstallSubcommand() const override { return "SBOM"; }
};
