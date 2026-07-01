/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmExportInstallPackageInfoGenerator.h"

#include <functional>
#include <map>
#include <memory>
#include <set>
#include <utility>
#include <vector>

#include <cm/optional>
#include <cm/string_view>
#include <cmext/algorithm>
#include <cmext/string_view>

#include <cm3p/json/value.h>

#include "cmAlgorithms.h"
#include "cmDiagnostics.h"
#include "cmExportSet.h"
#include "cmFileSetMetadata.h"
#include "cmGenExContext.h"
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
    this->GenerateTargetFileSets(*component, gt, te);
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
  cmGeneratorTarget const* target, cmTargetExport const* targetExport,
  cmGeneratorFileSet const* fileSet, cm::optional<std::string> const& config)
{
  cmInstallFileSetGenerator const* const fsg =
    targetExport->FileSetGenerators.at(fileSet->GetName());
  cmInstallFileSetGenerator::DestinationContext const result =
    fsg->GetDestination(target, config.value_or(""));

  if (!config == result.HadContextSensitiveCondition) {
    return {};
  }

  // Use cm::optional here to enable NRVO.
  cm::optional<std::string> dest = cmOutputConverter::EscapeForCMake(
    result.UnescapedDestination, cmOutputConverter::WrapQuotes::NoWrap);

  if (!cmSystemTools::FileIsFullPath(result.UnescapedDestination)) {
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
      this->IssueMessage(MessageType::FATAL_ERROR,
                         cmStrCat("File set \"", name,
                                  "\" is listed in interface file sets of ",
                                  gte->GetName(),
                                  " but has not been created"));
      return false;
    }

    cm::optional<std::string> const& fileSetDirectory =
      this->GetFileSetDirectory(gte, te, fileSet, config);

    if (fileSet->GetType() == cm::FileSetMetadata::HEADERS) {
      if (!fileSetDirectory) {
        if (!config) {
          this->RequiresConfigFiles = true;
        }
      } else if (!cm::contains(seenIncludeDirectories, *fileSetDirectory)) {
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

void cmExportInstallPackageInfoGenerator::GenerateTargetFileSets(
  Json::Value& fileSets, cmGeneratorTarget const* target,
  cmGeneratorFileSet const* fileSet, cmTargetExport const* targetExport,
  std::string const& type) const
{
  cm::optional<std::string> dest =
    this->GetFileSetDirectory(target, targetExport, fileSet);
  if (!dest) {
    this->IssueDiagnostic(
      cmDiagnostics::CMD_AUTHOR,
      cmStrCat("The \""_s, target->GetName(),
               "\" target's interface file set \""_s, fileSet->GetName(),
               "\" of type \""_s, fileSet->GetType(),
               "\" has a context-sensitive destination, which is not "
               "supported.  The file set will not be exported."_s));
    return;
  }

  cm::GenEx::Context context{ target->LocalGenerator, {} };

  std::vector<std::string> files;
  auto eval = [&files](std::string&& /*baseDir*/, std::string&& relPath,
                       std::string&& /*file*/) {
    files.emplace_back(std::move(relPath));
  };

  if (fileSet->EvaluateFiles(context, target, eval)) {
    this->IssueDiagnostic(
      cmDiagnostics::CMD_AUTHOR,
      cmStrCat("The \""_s, target->GetName(),
               "\" target's interface file set \""_s, fileSet->GetName(),
               "\" of type \""_s, fileSet->GetType(),
               "\" contains context-sensitive information, which is not "
               "supported.  The file set will not be exported."_s));
    return;
  }

  this->GenerateTargetFileSet(fileSets, fileSet, type, *dest, files);
}
