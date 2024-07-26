/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmInstallPackageInfoExportGenerator.h"

#include <utility>

#include <cm/memory>

#include "cmExportInstallFileGenerator.h"
#include "cmExportInstallPackageInfoGenerator.h"
#include "cmListFileCache.h"

class cmExportSet;

cmInstallPackageInfoExportGenerator::cmInstallPackageInfoExportGenerator(
  cmExportSet* exportSet, std::string destination, std::string filePermissions,
  std::vector<std::string> const& configurations, std::string component,
  MessageLevel message, bool excludeFromAll, std::string filename,
  std::string packageName, std::string version, std::string versionCompat,
  std::string versionSchema, std::vector<std::string> defaultTargets,
  std::vector<std::string> defaultConfigurations,
  std::string cxxModulesDirectory, cmListFileBacktrace backtrace)
  : cmInstallExportGenerator(
      exportSet, std::move(destination), std::move(filePermissions),
      configurations, std::move(component), message, excludeFromAll,
      std::move(filename), packageName + "::", std::move(cxxModulesDirectory),
      std::move(backtrace))
{
  this->EFGen = cm::make_unique<cmExportInstallPackageInfoGenerator>(
    this, std::move(packageName), std::move(version), std::move(versionCompat),
    std::move(versionSchema), std::move(defaultTargets),
    std::move(defaultConfigurations));
}

cmInstallPackageInfoExportGenerator::~cmInstallPackageInfoExportGenerator() =
  default;
