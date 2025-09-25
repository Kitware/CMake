/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include <string>
#include <vector>

#include "cmInstallExportGenerator.h"

class cmExportSet;
class cmListFileBacktrace;
class cmPackageInfoArguments;

/** \class cmInstallPackageInfoGenerator
 * \brief Generate rules for creating CPS package info files.
 */
class cmInstallPackageInfoExportGenerator : public cmInstallExportGenerator
{
public:
  cmInstallPackageInfoExportGenerator(
    cmExportSet* exportSet, std::string destination,
    std::string filePermissions,
    std::vector<std::string> const& configurations, std::string component,
    MessageLevel message, bool excludeFromAll,
    cmPackageInfoArguments arguments, std::string cxxModulesDirectory,
    cmListFileBacktrace backtrace);
  cmInstallPackageInfoExportGenerator(
    cmInstallPackageInfoExportGenerator const&) = delete;
  ~cmInstallPackageInfoExportGenerator() override;

  cmInstallPackageInfoExportGenerator& operator=(
    cmInstallPackageInfoExportGenerator const&) = delete;

  char const* InstallSubcommand() const override { return "PACKAGE_INFO"; }
};
