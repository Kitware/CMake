/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmInstallPackageInfoExportGenerator.h"

#include <utility>

#include <cm/memory>

#include "cmDiagnosticContext.h"
#include "cmExportInstallFileGenerator.h"
#include "cmExportInstallPackageInfoGenerator.h"
#include "cmPackageInfoArguments.h"

class cmExportSet;

cmInstallPackageInfoExportGenerator::cmInstallPackageInfoExportGenerator(
  cmExportSet* exportSet, std::string destination, std::string filePermissions,
  std::vector<std::string> const& configurations, std::string component,
  MessageLevel message, bool excludeFromAll, cmPackageInfoArguments arguments,
  std::string cxxModulesDirectory, cmDiagnosticContext context)
  : cmInstallExportGenerator(
      exportSet, std::move(destination), std::move(filePermissions),
      configurations, std::move(component), message, excludeFromAll,
      arguments.GetPackageFileName(), arguments.GetNamespace(),
      std::move(cxxModulesDirectory), std::move(context))
{
  this->EFGen = cm::make_unique<cmExportInstallPackageInfoGenerator>(
    this, std::move(arguments));
}

cmInstallPackageInfoExportGenerator::~cmInstallPackageInfoExportGenerator() =
  default;
