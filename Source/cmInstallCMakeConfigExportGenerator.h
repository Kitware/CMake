/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include <iosfwd>
#include <string>
#include <vector>

#include "cmInstallExportGenerator.h"

class cmExportSet;
class cmListFileBacktrace;

/** \class cmInstallCMakeConfigExportGenerator
 * \brief Generate rules for creating CMake export files.
 */
class cmInstallCMakeConfigExportGenerator : public cmInstallExportGenerator
{
public:
  cmInstallCMakeConfigExportGenerator(
    cmExportSet* exportSet, std::string destination,
    std::string filePermissions,
    std::vector<std::string> const& configurations, std::string component,
    MessageLevel message, bool excludeFromAll, std::string filename,
    std::string targetNamespace, std::string cxxModulesDirectory,
    bool exportOld, bool exportPackageDependencies,
    cmListFileBacktrace backtrace);
  cmInstallCMakeConfigExportGenerator(
    cmInstallCMakeConfigExportGenerator const&) = delete;
  ~cmInstallCMakeConfigExportGenerator() override;

  cmInstallCMakeConfigExportGenerator& operator=(
    cmInstallCMakeConfigExportGenerator const&) = delete;

  char const* InstallSubcommand() const override { return "EXPORT"; }

protected:
  void GenerateScript(std::ostream& os) override;

  bool const ExportOld;
  bool const ExportPackageDependencies;
};
