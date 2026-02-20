/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmExportInstallPackageInfoGenerator.h"

#include <functional>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <utility>
#include <vector>

#include <cm/optional>
#include <cm/string_view>
#include <cmext/algorithm>
#include <cmext/string_view>

#include <cm3p/json/value.h>

#include "cmAlgorithms.h"
#include "cmExportSet.h"
#include "cmFileSetMetadata.h"
#include "cmGeneratorExpression.h"
#include "cmGeneratorFileSet.h"
#include "cmGeneratorTarget.h"
#include "cmInstallExportGenerator.h"
#include "cmInstallFileSetGenerator.h"
#include "cmList.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmOutputConverter.h"
#include "cmPackageInfoArguments.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmTarget.h"
#include "cmTargetExport.h"

cmExportInstallPackageInfoGenerator::cmExportInstallPackageInfoGenerator(
  cmInstallExportGenerator* iegen, cmPackageInfoArguments arguments)
  : cmExportPackageInfoGenerator(std::move(arguments))
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

  if (!this->CheckPackage()) {
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
      this->RequiresConfigFiles = true;
    }

    // De-duplicate include directories prior to generation.
    auto it = properties.find("INTERFACE_INCLUDE_DIRECTORIES");
    if (it != properties.end()) {
      auto list = cmList{ it->second };
      list = cmList{ list.begin(), cmRemoveDuplicates(list) };
      properties["INTERFACE_INCLUDE_DIRECTORIES"] = list.to_string();
    }

    // Set configuration-agnostic properties for component.
    this->GenerateInterfaceProperties(*component, gt, properties);
    if (!this->GenerateFileSetProperties(*component, gt, te, packagePath)) {
      return false;
    }
  }

  this->GeneratePackageRequires(root);

  // Write the primary packing information file.
  this->WritePackageInfo(root, os);

  bool result = true;

  // Generate an import file for each configuration.
  if (this->RequiresConfigFiles) {
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
  root["configuration"] = (config.empty() ? "noconfig" : config);

  std::string const& packagePath = this->GenerateImportPrefix();

  Json::Value& components = root["components"];

  for (auto const& te : this->GetExportSet()->GetTargetExports()) {
    // Collect import properties for this target.
    ImportPropertyMap properties;
    std::set<std::string> importedLocations;

    if (this->GetExportTargetType(te.get()) !=
        cmStateEnums::INTERFACE_LIBRARY) {
      this->PopulateImportProperties(config, suffix, te.get(), properties,
                                     importedLocations);
    }

    Json::Value component =
      this->GenerateInterfaceConfigProperties(suffix, properties);
    this->GenerateFileSetProperties(component, te->Target, te.get(),
                                    packagePath, config);

    if (!component.empty()) {
      components[te->Target->GetExportName()] = std::move(component);
    }
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
  return IEGen->GetCxxModuleDirectory();
}

cm::optional<std::string>
cmExportInstallPackageInfoGenerator::GetFileSetDirectory(
  cmGeneratorTarget* gte, cmTargetExport const* te,
  cmGeneratorFileSet const* fileSet, cm::optional<std::string> const& config)
{
  cmGeneratorExpression ge(*gte->Makefile->GetCMakeInstance());
  auto cge =
    ge.Parse(te->FileSetGenerators.at(fileSet->GetName())->GetDestination());

  std::string const unescapedDest =
    cge->Evaluate(gte->LocalGenerator, config.value_or(""), gte);
  bool const isConfigDependent = cge->GetHadContextSensitiveCondition();

  if (config && !isConfigDependent) {
    return {};
  }
  if (!config && isConfigDependent) {
    this->RequiresConfigFiles = true;
    return {};
  }

  std::string const& type = fileSet->GetType();
  if (config && (type == cm::FileSetMetadata::CXX_MODULES)) {
    // C++ modules do not support interface file sets which are dependent
    // upon the configuration.
    cmMakefile* mf = gte->LocalGenerator->GetMakefile();
    std::ostringstream e;
    e << "The \"" << gte->GetName() << "\" target's interface file set \""
      << fileSet->GetName() << "\" of type \"" << type
      << "\" contains context-sensitive base file entries which is not "
         "supported.";
    mf->IssueMessage(MessageType::FATAL_ERROR, e.str());
    return {};
  }

  cm::optional<std::string> dest = cmOutputConverter::EscapeForCMake(
    unescapedDest, cmOutputConverter::WrapQuotes::NoWrap);

  if (!cmSystemTools::FileIsFullPath(unescapedDest)) {
    dest = cmStrCat("@prefix@/"_s, *dest);
  }

  return dest;
}

bool cmExportInstallPackageInfoGenerator::GenerateFileSetProperties(
  Json::Value& component, cmGeneratorTarget* gte, cmTargetExport const* te,
  std::string const& packagePath, cm::optional<std::string> config)
{
  bool hasModules = false;
  std::set<std::string> seenIncludeDirectories;
  for (auto const& name : gte->Target->GetAllInterfaceFileSets()) {
    cmGeneratorFileSet const* fileSet = gte->GetFileSet(name);

    if (!fileSet) {
      gte->Makefile->IssueMessage(
        MessageType::FATAL_ERROR,
        cmStrCat("File set \"", name,
                 "\" is listed in interface file sets of ", gte->GetName(),
                 " but has not been created"));
      return false;
    }

    cm::optional<std::string> const& fileSetDirectory =
      this->GetFileSetDirectory(gte, te, fileSet, config);

    if (fileSet->GetType() == cm::FileSetMetadata::HEADERS) {
      if (fileSetDirectory &&
          !cm::contains(seenIncludeDirectories, *fileSetDirectory)) {
        component["includes"].append(*fileSetDirectory);
        seenIncludeDirectories.insert(*fileSetDirectory);
      }
    } else if (fileSet->GetType() == cm::FileSetMetadata::CXX_MODULES) {
      hasModules = true;
      this->RequiresConfigFiles = true;
    }
  }

  if (hasModules && config) {
    std::string const manifestPath =
      this->GenerateCxxModules(component, gte, packagePath, *config);
    if (!manifestPath.empty()) {
      this->ConfigCxxModuleFiles[*config] =
        cmStrCat(this->FileDir, '/', manifestPath);
    }
  }
  return true;
}
