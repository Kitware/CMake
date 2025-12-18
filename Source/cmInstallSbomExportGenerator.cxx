/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmInstallSbomExportGenerator.h"

#include <utility>

#include <cm/memory>

#include "cmExportInstallFileGenerator.h"
#include "cmExportInstallSbomGenerator.h"
#include "cmListFileCache.h"
#include "cmSbomArguments.h"

class cmExportSet;

cmInstallSbomExportGenerator::cmInstallSbomExportGenerator(
  cmExportSet* exportSet, std::string destination, std::string filePermissions,
  std::vector<std::string> const& configurations, std::string component,
  MessageLevel message, bool excludeFromAll, cmSbomArguments args,
  std::string cxxModulesDirectory, cmListFileBacktrace backtrace)
  : cmInstallExportGenerator(
      exportSet, std::move(destination), std::move(filePermissions),
      configurations, std::move(component), message, excludeFromAll,
      args.GetPackageFileName(), args.GetNamespace(),
      std::move(cxxModulesDirectory), std::move(backtrace))
{
  this->EFGen = cm::make_unique<cmExportInstallSbomGenerator>(this, args);
}

cmInstallSbomExportGenerator::~cmInstallSbomExportGenerator() = default;
