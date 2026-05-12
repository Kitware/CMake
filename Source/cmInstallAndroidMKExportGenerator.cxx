/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmInstallAndroidMKExportGenerator.h"

#include <utility>

#include <cm/memory>

#include "cmDiagnosticContext.h"
#include "cmExportInstallAndroidMKGenerator.h"
#include "cmExportInstallFileGenerator.h"

class cmExportSet;

cmInstallAndroidMKExportGenerator::cmInstallAndroidMKExportGenerator(
  cmExportSet* exportSet, std::string destination, std::string filePermissions,
  std::vector<std::string> const& configurations, std::string component,
  MessageLevel message, bool excludeFromAll, std::string filename,
  std::string targetNamespace, cmDiagnosticContext context)
  : cmInstallExportGenerator(exportSet, std::move(destination),
                             std::move(filePermissions), configurations,
                             std::move(component), message, excludeFromAll,
                             std::move(filename), std::move(targetNamespace),
                             std::string{}, std::move(context))
{
  this->EFGen = cm::make_unique<cmExportInstallAndroidMKGenerator>(this);
}

cmInstallAndroidMKExportGenerator::~cmInstallAndroidMKExportGenerator() =
  default;
