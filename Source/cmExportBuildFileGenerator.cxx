/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmExportBuildFileGenerator.h"

#include <algorithm>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <utility>

#include "cmExportSet.h"
#include "cmGeneratorExpression.h"
#include "cmGeneratorTarget.h"
#include "cmGlobalGenerator.h"
#include "cmList.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmTarget.h"
#include "cmTargetExport.h"
#include "cmValue.h"
#include "cmake.h"

class cmSourceFile;

cmExportBuildFileGenerator::cmExportBuildFileGenerator()
{
  this->LG = nullptr;
  this->ExportSet = nullptr;
}

void cmExportBuildFileGenerator::Compute(cmLocalGenerator* lg)
{
  this->LG = lg;
  if (this->ExportSet) {
    this->ExportSet->Compute(lg);
  }
}

cmStateEnums::TargetType cmExportBuildFileGenerator::GetExportTargetType(
  cmGeneratorTarget const* target) const
{
  cmStateEnums::TargetType targetType = target->GetType();
  // An object library exports as an interface library if we cannot
  // tell clients where to find the objects.  This is sufficient
  // to support transitive usage requirements on other targets that
  // use the object library.
  if (targetType == cmStateEnums::OBJECT_LIBRARY &&
      !target->Target->HasKnownObjectFileLocation(nullptr)) {
    targetType = cmStateEnums::INTERFACE_LIBRARY;
  }
  return targetType;
}

void cmExportBuildFileGenerator::SetExportSet(cmExportSet* exportSet)
{
  this->ExportSet = exportSet;
}

void cmExportBuildFileGenerator::SetImportLocationProperty(
  std::string const& config, std::string const& suffix,
  cmGeneratorTarget* target, ImportPropertyMap& properties)
{
  // Get the makefile in which to lookup target information.
  cmMakefile* mf = target->Makefile;

  if (target->GetType() == cmStateEnums::OBJECT_LIBRARY) {
    std::string prop = cmStrCat("IMPORTED_OBJECTS", suffix);

    // Compute all the object files inside this target and setup
    // IMPORTED_OBJECTS as a list of object files
    std::vector<cmSourceFile const*> objectSources;
    target->GetObjectSources(objectSources, config);
    std::string const obj_dir = target->GetObjectDirectory(config);
    std::vector<std::string> objects;
    for (cmSourceFile const* sf : objectSources) {
      std::string const& obj = target->GetObjectName(sf);
      objects.push_back(obj_dir + obj);
    }

    // Store the property.
    properties[prop] = cmList::to_string(objects);
  } else {
    // Add the main target file.
    {
      std::string prop = cmStrCat("IMPORTED_LOCATION", suffix);
      std::string value;
      if (target->IsAppBundleOnApple()) {
        value =
          target->GetFullPath(config, cmStateEnums::RuntimeBinaryArtifact);
      } else {
        value = target->GetFullPath(config,
                                    cmStateEnums::RuntimeBinaryArtifact, true);
      }
      properties[prop] = value;
    }

    // Add the import library for windows DLLs.
    if (target->HasImportLibrary(config)) {
      std::string prop = cmStrCat("IMPORTED_IMPLIB", suffix);
      std::string value =
        target->GetFullPath(config, cmStateEnums::ImportLibraryArtifact, true);
      if (mf->GetDefinition("CMAKE_IMPORT_LIBRARY_SUFFIX")) {
        target->GetImplibGNUtoMS(config, value, value,
                                 "${CMAKE_IMPORT_LIBRARY_SUFFIX}");
      }
      properties[prop] = value;
    }
  }
}

bool cmExportBuildFileGenerator::CollectExports(
  std::function<void(cmGeneratorTarget const*)> visitor)
{
  auto pred = [&](cmExportBuildFileGenerator::TargetExport& tei) -> bool {
    cmGeneratorTarget* te = this->LG->FindGeneratorTargetToUse(tei.Name);
    if (this->ExportedTargets.insert(te).second) {
      this->Exports.emplace_back(te, tei.XcFrameworkLocation);
      visitor(te);
      return true;
    }

    this->ComplainAboutDuplicateTarget(te->GetName());
    return false;
  };

  std::vector<TargetExport> targets;
  this->GetTargets(targets);
  return std::all_of(targets.begin(), targets.end(), pred);
}

void cmExportBuildFileGenerator::HandleMissingTarget(
  std::string& link_libs, cmGeneratorTarget const* depender,
  cmGeneratorTarget* dependee)
{
  // The target is not in the export.
  if (!this->AppendMode) {
    auto const& exportInfo = this->FindExportInfo(dependee);
    auto const& exportFiles = exportInfo.first;

    if (exportFiles.size() == 1) {
      std::string missingTarget = exportInfo.second;

      missingTarget += dependee->GetExportName();
      link_libs += missingTarget;
      this->MissingTargets.emplace_back(std::move(missingTarget));
      return;
    }
    // We are not appending, so all exported targets should be
    // known here.  This is probably user-error.
    this->ComplainAboutMissingTarget(depender, dependee, exportFiles);
  }
  // Assume the target will be exported by another command.
  // Append it with the export namespace.
  link_libs += this->Namespace;
  link_libs += dependee->GetExportName();
}

void cmExportBuildFileGenerator::GetTargets(
  std::vector<TargetExport>& targets) const
{
  if (this->ExportSet) {
    for (std::unique_ptr<cmTargetExport> const& te :
         this->ExportSet->GetTargetExports()) {
      if (te->NamelinkOnly) {
        continue;
      }
      targets.emplace_back(te->TargetName, te->XcFrameworkLocation);
    }
    return;
  }
  targets = this->Targets;
}

cmExportFileGenerator::ExportInfo cmExportBuildFileGenerator::FindExportInfo(
  cmGeneratorTarget const* target) const
{
  std::vector<std::string> exportFiles;
  std::string ns;

  auto const& name = target->GetName();
  auto& exportSets =
    target->GetLocalGenerator()->GetGlobalGenerator()->GetBuildExportSets();

  for (auto const& exp : exportSets) {
    auto const& exportSet = exp.second;
    std::vector<TargetExport> targets;
    exportSet->GetTargets(targets);
    if (std::any_of(
          targets.begin(), targets.end(),
          [&name](TargetExport const& te) { return te.Name == name; })) {
      exportFiles.push_back(exp.first);
      ns = exportSet->GetNamespace();
    }
  }

  return { exportFiles, exportFiles.size() == 1 ? ns : std::string{} };
}

void cmExportBuildFileGenerator::ComplainAboutMissingTarget(
  cmGeneratorTarget const* depender, cmGeneratorTarget const* dependee,
  std::vector<std::string> const& exportFiles) const
{
  std::ostringstream e;
  e << "export called with target \"" << depender->GetName()
    << "\" which requires target \"" << dependee->GetName() << "\" ";
  if (exportFiles.empty()) {
    e << "that is not in any export set.";
  } else {
    e << "that is not in this export set, but in multiple other export sets: "
      << cmJoin(exportFiles, ", ") << ".\n";
    e << "An exported target cannot depend upon another target which is "
         "exported multiple times. Consider consolidating the exports of the "
         "\""
      << dependee->GetName() << "\" target to a single export.";
  }

  this->ReportError(e.str());
}

void cmExportBuildFileGenerator::ComplainAboutDuplicateTarget(
  std::string const& targetName) const
{
  std::ostringstream e;
  e << "given target \"" << targetName << "\" more than once.";
  this->ReportError(e.str());
}

void cmExportBuildFileGenerator::ReportError(
  std::string const& errorMessage) const
{
  this->LG->GetGlobalGenerator()->GetCMakeInstance()->IssueMessage(
    MessageType::FATAL_ERROR, errorMessage,
    this->LG->GetMakefile()->GetBacktrace());
}

std::string cmExportBuildFileGenerator::InstallNameDir(
  cmGeneratorTarget const* target, std::string const& config)
{
  std::string install_name_dir;

  cmMakefile* mf = target->Target->GetMakefile();
  if (mf->IsOn("CMAKE_PLATFORM_HAS_INSTALLNAME")) {
    install_name_dir = target->GetInstallNameDirForBuildTree(config);
  }

  return install_name_dir;
}

bool cmExportBuildFileGenerator::PopulateInterfaceProperties(
  cmGeneratorTarget const* target, ImportPropertyMap& properties)
{
  this->PopulateInterfaceProperty("INTERFACE_INCLUDE_DIRECTORIES", target,
                                  cmGeneratorExpression::BuildInterface,
                                  properties);
  this->PopulateInterfaceProperty("INTERFACE_LINK_DIRECTORIES", target,
                                  cmGeneratorExpression::BuildInterface,
                                  properties);
  this->PopulateInterfaceProperty("INTERFACE_LINK_DEPENDS", target,
                                  cmGeneratorExpression::BuildInterface,
                                  properties);
  this->PopulateInterfaceProperty("INTERFACE_SOURCES", target,
                                  cmGeneratorExpression::BuildInterface,
                                  properties);

  return this->PopulateInterfaceProperties(
    target, {}, cmGeneratorExpression::BuildInterface, properties);
}
