/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmExportInstallPackageInfoGenerator.h"

#include <memory>
#include <set>
#include <utility>
#include <vector>

#include <cm3p/json/value.h>

#include "cmExportSet.h"
#include "cmGeneratorExpression.h"
#include "cmGeneratorTarget.h"
#include "cmInstallExportGenerator.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmTarget.h"
#include "cmTargetExport.h"

cmExportInstallPackageInfoGenerator::cmExportInstallPackageInfoGenerator(
  cmInstallExportGenerator* iegen, std::string packageName,
  std::string version, std::string versionCompat, std::string versionSchema,
  std::vector<std::string> defaultTargets,
  std::vector<std::string> defaultConfigurations)
  : cmExportPackageInfoGenerator(
      std::move(packageName), std::move(version), std::move(versionCompat),
      std::move(versionSchema), std::move(defaultTargets),
      std::move(defaultConfigurations))
  , cmExportInstallFileGenerator(iegen)
{
}

std::string cmExportInstallPackageInfoGenerator::GetConfigImportFileGlob()
  const
{
  std::string glob = cmStrCat(this->FileBase, "@*", this->FileExt);
  return glob;
}

std::string const& cmExportInstallPackageInfoGenerator::GetExportName() const
{
  return this->GetPackageName();
}

bool cmExportInstallPackageInfoGenerator::GenerateMainFile(std::ostream& os)
{
  std::vector<cmTargetExport const*> allTargets;
  {
    auto visitor = [&](cmTargetExport const* te) { allTargets.push_back(te); };

    if (!this->CollectExports(visitor)) {
      return false;
    }
  }

  if (!this->CheckDefaultTargets()) {
    return false;
  }

  Json::Value root = this->GeneratePackageInfo();
  Json::Value& components = root["components"];

  // Compute the relative import prefix for the file
  std::string const& packagePath = this->GenerateImportPrefix();
  if (packagePath.empty()) {
    return false;
  }
  root["cps_path"] = packagePath;

  bool requiresConfigFiles = false;
  // Create all the imported targets.
  for (cmTargetExport const* te : allTargets) {
    cmGeneratorTarget* gt = te->Target;
    cmStateEnums::TargetType targetType = this->GetExportTargetType(te);

    Json::Value* const component =
      this->GenerateImportTarget(components, gt, targetType);
    if (!component) {
      return false;
    }

    ImportPropertyMap properties;
    if (!this->PopulateInterfaceProperties(te, properties)) {
      return false;
    }
    this->PopulateInterfaceLinkLibrariesProperty(
      gt, cmGeneratorExpression::InstallInterface, properties);

    if (targetType != cmStateEnums::INTERFACE_LIBRARY) {
      requiresConfigFiles = true;
    }

    // Set configuration-agnostic properties for component.
    this->GenerateInterfaceProperties(*component, gt, properties);
  }

  this->GeneratePackageRequires(root);

  // Write the primary packing information file.
  this->WritePackageInfo(root, os);

  bool result = true;

  // Generate an import file for each configuration.
  if (requiresConfigFiles) {
    for (std::string const& c : this->Configurations) {
      if (!this->GenerateImportFileConfig(c)) {
        result = false;
      }
    }
  }

  return result;
}

void cmExportInstallPackageInfoGenerator::GenerateImportTargetsConfig(
  std::ostream& os, std::string const& config, std::string const& suffix)
{
  Json::Value root;
  root["name"] = this->GetPackageName();
  root["configuration"] = config;

  Json::Value& components = root["components"];

  for (auto const& te : this->GetExportSet()->GetTargetExports()) {
    // Collect import properties for this target.
    if (this->GetExportTargetType(te.get()) ==
        cmStateEnums::INTERFACE_LIBRARY) {
      continue;
    }

    ImportPropertyMap properties;
    std::set<std::string> importedLocations;

    this->PopulateImportProperties(config, suffix, te.get(), properties,
                                   importedLocations);

    this->GenerateInterfaceConfigProperties(components, te->Target, suffix,
                                            properties);
  }

  this->WritePackageInfo(root, os);
}

std::string cmExportInstallPackageInfoGenerator::GenerateImportPrefix() const
{
  std::string expDest = this->IEGen->GetDestination();
  if (cmSystemTools::FileIsFullPath(expDest)) {
    std::string const& installPrefix =
      this->IEGen->GetLocalGenerator()->GetMakefile()->GetSafeDefinition(
        "CMAKE_INSTALL_PREFIX");
    if (cmHasPrefix(expDest, installPrefix)) {
      auto n = installPrefix.length();
      while (n < expDest.length() && expDest[n] == '/') {
        ++n;
      }
      expDest = expDest.substr(n);
    } else {
      this->ReportError(
        cmStrCat("install(PACKAGE_INFO \"", this->GetExportName(),
                 "\" ...) specifies DESTINATION \"", expDest,
                 "\" which is not a subdirectory of the install prefix."));
      return {};
    }
  }

  if (expDest.empty()) {
    return this->GetInstallPrefix();
  }
  return cmStrCat(this->GetImportPrefixWithSlash(), expDest);
}

std::string cmExportInstallPackageInfoGenerator::InstallNameDir(
  cmGeneratorTarget const* target, std::string const& config)
{
  std::string install_name_dir;

  cmMakefile* mf = target->Target->GetMakefile();
  if (mf->IsOn("CMAKE_PLATFORM_HAS_INSTALLNAME")) {
    install_name_dir =
      target->GetInstallNameDirForInstallTree(config, "@prefix@");
  }

  return install_name_dir;
}

std::string cmExportInstallPackageInfoGenerator::GetCxxModulesDirectory() const
{
  // TODO: Implement a not-CMake-specific mechanism for providing module
  // information.
  // return IEGen->GetCxxModuleDirectory();
  return {};
}
