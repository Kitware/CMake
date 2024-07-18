/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmInstallCMakeConfigExportGenerator.h"

#include <utility>

#include <cm/memory>

#include "cmExportInstallCMakeConfigGenerator.h"
#include "cmExportInstallFileGenerator.h"
#include "cmListFileCache.h"

class cmExportSet;

cmInstallCMakeConfigExportGenerator::cmInstallCMakeConfigExportGenerator(
  cmExportSet* exportSet, std::string destination, std::string filePermissions,
  std::vector<std::string> const& configurations, std::string component,
  MessageLevel message, bool excludeFromAll, std::string filename,
  std::string targetNamespace, std::string cxxModulesDirectory, bool exportOld,
  bool exportPackageDependencies, cmListFileBacktrace backtrace)
  : cmInstallExportGenerator(
      exportSet, std::move(destination), std::move(filePermissions),
      configurations, std::move(component), message, excludeFromAll,
      std::move(filename), std::move(targetNamespace),
      std::move(cxxModulesDirectory), std::move(backtrace))
  , ExportOld(exportOld)
  , ExportPackageDependencies(exportPackageDependencies)
{
  this->EFGen = cm::make_unique<cmExportInstallCMakeConfigGenerator>(this);
}

cmInstallCMakeConfigExportGenerator::~cmInstallCMakeConfigExportGenerator() =
  default;

void cmInstallCMakeConfigExportGenerator::GenerateScript(std::ostream& os)
{
  auto* const efgen =
    static_cast<cmExportInstallCMakeConfigGenerator*>(this->EFGen.get());
  efgen->SetExportOld(this->ExportOld);
  efgen->SetExportPackageDependencies(this->ExportPackageDependencies);

  this->cmInstallExportGenerator::GenerateScript(os);
}
