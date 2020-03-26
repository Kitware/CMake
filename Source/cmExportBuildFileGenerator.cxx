/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmExportBuildFileGenerator.h"

#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <utility>

#include "cmAlgorithms.h"
#include "cmExportSet.h"
#include "cmGeneratorExpression.h"
#include "cmGeneratorTarget.h"
#include "cmGlobalGenerator.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmPolicies.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmTarget.h"
#include "cmTargetExport.h"
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

bool cmExportBuildFileGenerator::GenerateMainFile(std::ostream& os)
{
  {
    std::string expectedTargets;
    std::string sep;
    std::vector<std::string> targets;
    bool generatedInterfaceRequired = false;
    this->GetTargets(targets);
    for (std::string const& tei : targets) {
      cmGeneratorTarget* te = this->LG->FindGeneratorTargetToUse(tei);
      expectedTargets += sep + this->Namespace + te->GetExportName();
      sep = " ";
      if (this->ExportedTargets.insert(te).second) {
        this->Exports.push_back(te);
      } else {
        std::ostringstream e;
        e << "given target \"" << te->GetName() << "\" more than once.";
        this->LG->GetGlobalGenerator()->GetCMakeInstance()->IssueMessage(
          MessageType::FATAL_ERROR, e.str(),
          this->LG->GetMakefile()->GetBacktrace());
        return false;
      }
      generatedInterfaceRequired |=
        this->GetExportTargetType(te) == cmStateEnums::INTERFACE_LIBRARY;
    }

    if (generatedInterfaceRequired) {
      this->GenerateRequiredCMakeVersion(os, "3.0.0");
    }
    this->GenerateExpectedTargetsCode(os, expectedTargets);
  }

  std::vector<std::string> missingTargets;

  // Create all the imported targets.
  for (cmGeneratorTarget* gte : this->Exports) {
    this->GenerateImportTargetCode(os, gte, this->GetExportTargetType(gte));

    gte->Target->AppendBuildInterfaceIncludes();

    ImportPropertyMap properties;

    this->PopulateInterfaceProperty("INTERFACE_INCLUDE_DIRECTORIES", gte,
                                    cmGeneratorExpression::BuildInterface,
                                    properties, missingTargets);
    this->PopulateInterfaceProperty("INTERFACE_SOURCES", gte,
                                    cmGeneratorExpression::BuildInterface,
                                    properties, missingTargets);
    this->PopulateInterfaceProperty("INTERFACE_COMPILE_DEFINITIONS", gte,
                                    cmGeneratorExpression::BuildInterface,
                                    properties, missingTargets);
    this->PopulateInterfaceProperty("INTERFACE_COMPILE_OPTIONS", gte,
                                    cmGeneratorExpression::BuildInterface,
                                    properties, missingTargets);
    this->PopulateInterfaceProperty("INTERFACE_PRECOMPILE_HEADERS", gte,
                                    cmGeneratorExpression::BuildInterface,
                                    properties, missingTargets);
    this->PopulateInterfaceProperty("INTERFACE_AUTOUIC_OPTIONS", gte,
                                    cmGeneratorExpression::BuildInterface,
                                    properties, missingTargets);
    this->PopulateInterfaceProperty("INTERFACE_COMPILE_FEATURES", gte,
                                    cmGeneratorExpression::BuildInterface,
                                    properties, missingTargets);
    this->PopulateInterfaceProperty("INTERFACE_LINK_OPTIONS", gte,
                                    cmGeneratorExpression::BuildInterface,
                                    properties, missingTargets);
    this->PopulateInterfaceProperty("INTERFACE_LINK_DIRECTORIES", gte,
                                    cmGeneratorExpression::BuildInterface,
                                    properties, missingTargets);
    this->PopulateInterfaceProperty("INTERFACE_LINK_DEPENDS", gte,
                                    cmGeneratorExpression::BuildInterface,
                                    properties, missingTargets);
    this->PopulateInterfaceProperty("INTERFACE_POSITION_INDEPENDENT_CODE", gte,
                                    properties);

    std::string errorMessage;
    if (!this->PopulateExportProperties(gte, properties, errorMessage)) {
      this->LG->GetGlobalGenerator()->GetCMakeInstance()->IssueMessage(
        MessageType::FATAL_ERROR, errorMessage,
        this->LG->GetMakefile()->GetBacktrace());
      return false;
    }

    const bool newCMP0022Behavior =
      gte->GetPolicyStatusCMP0022() != cmPolicies::WARN &&
      gte->GetPolicyStatusCMP0022() != cmPolicies::OLD;
    if (newCMP0022Behavior) {
      this->PopulateInterfaceLinkLibrariesProperty(
        gte, cmGeneratorExpression::BuildInterface, properties,
        missingTargets);
    }
    this->PopulateCompatibleInterfaceProperties(gte, properties);

    this->GenerateInterfaceProperties(gte, os, properties);
  }

  // Generate import file content for each configuration.
  for (std::string const& c : this->Configurations) {
    this->GenerateImportConfig(os, c, missingTargets);
  }

  this->GenerateMissingTargetsCheckCode(os, missingTargets);

  return true;
}

void cmExportBuildFileGenerator::GenerateImportTargetsConfig(
  std::ostream& os, const std::string& config, std::string const& suffix,
  std::vector<std::string>& missingTargets)
{
  for (cmGeneratorTarget* target : this->Exports) {
    // Collect import properties for this target.
    ImportPropertyMap properties;

    if (this->GetExportTargetType(target) != cmStateEnums::INTERFACE_LIBRARY) {
      this->SetImportLocationProperty(config, suffix, target, properties);
    }
    if (!properties.empty()) {
      // Get the rest of the target details.
      if (this->GetExportTargetType(target) !=
          cmStateEnums::INTERFACE_LIBRARY) {
        this->SetImportDetailProperties(config, suffix, target, properties,
                                        missingTargets);
        this->SetImportLinkInterface(config, suffix,
                                     cmGeneratorExpression::BuildInterface,
                                     target, properties, missingTargets);
      }

      // TODO: PUBLIC_HEADER_LOCATION
      // This should wait until the build feature propagation stuff
      // is done.  Then this can be a propagated include directory.
      // this->GenerateImportProperty(config, te->HeaderGenerator,
      //                              properties);

      // Generate code in the export file.
      this->GenerateImportPropertyCode(os, config, target, properties);
    }
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
      !this->LG->GetGlobalGenerator()->HasKnownObjectFileLocation(nullptr)) {
    targetType = cmStateEnums::INTERFACE_LIBRARY;
  }
  return targetType;
}

void cmExportBuildFileGenerator::SetExportSet(cmExportSet* exportSet)
{
  this->ExportSet = exportSet;
}

void cmExportBuildFileGenerator::SetImportLocationProperty(
  const std::string& config, std::string const& suffix,
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
      const std::string& obj = target->GetObjectName(sf);
      objects.push_back(obj_dir + obj);
    }

    // Store the property.
    properties[prop] = cmJoin(objects, ";");
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
        target->GetFullPath(config, cmStateEnums::ImportLibraryArtifact);
      if (mf->GetDefinition("CMAKE_IMPORT_LIBRARY_SUFFIX")) {
        target->GetImplibGNUtoMS(config, value, value,
                                 "${CMAKE_IMPORT_LIBRARY_SUFFIX}");
      }
      properties[prop] = value;
    }
  }
}

void cmExportBuildFileGenerator::HandleMissingTarget(
  std::string& link_libs, std::vector<std::string>& missingTargets,
  cmGeneratorTarget* depender, cmGeneratorTarget* dependee)
{
  // The target is not in the export.
  if (!this->AppendMode) {
    const std::string name = dependee->GetName();
    cmGlobalGenerator* gg =
      dependee->GetLocalGenerator()->GetGlobalGenerator();
    auto exportInfo = this->FindBuildExportInfo(gg, name);
    std::vector<std::string> const& exportFiles = exportInfo.first;

    if (exportFiles.size() == 1) {
      std::string missingTarget = exportInfo.second;

      missingTarget += dependee->GetExportName();
      link_libs += missingTarget;
      missingTargets.push_back(std::move(missingTarget));
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
  std::vector<std::string>& targets) const
{
  if (this->ExportSet) {
    for (std::unique_ptr<cmTargetExport> const& te :
         this->ExportSet->GetTargetExports()) {
      targets.push_back(te->TargetName);
    }
    return;
  }
  targets = this->Targets;
}

std::pair<std::vector<std::string>, std::string>
cmExportBuildFileGenerator::FindBuildExportInfo(cmGlobalGenerator* gg,
                                                const std::string& name)
{
  std::vector<std::string> exportFiles;
  std::string ns;

  auto& exportSets = gg->GetBuildExportSets();

  for (auto const& exp : exportSets) {
    const auto& exportSet = exp.second;
    std::vector<std::string> targets;
    exportSet->GetTargets(targets);
    if (cmContains(targets, name)) {
      exportFiles.push_back(exp.first);
      ns = exportSet->GetNamespace();
    }
  }

  return { exportFiles, ns };
}

void cmExportBuildFileGenerator::ComplainAboutMissingTarget(
  cmGeneratorTarget* depender, cmGeneratorTarget* dependee,
  std::vector<std::string> const& exportFiles)
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

  this->LG->GetGlobalGenerator()->GetCMakeInstance()->IssueMessage(
    MessageType::FATAL_ERROR, e.str(),
    this->LG->GetMakefile()->GetBacktrace());
}

std::string cmExportBuildFileGenerator::InstallNameDir(
  cmGeneratorTarget* target, const std::string& config)
{
  std::string install_name_dir;

  cmMakefile* mf = target->Target->GetMakefile();
  if (mf->IsOn("CMAKE_PLATFORM_HAS_INSTALLNAME")) {
    install_name_dir = target->GetInstallNameDirForBuildTree(config);
  }

  return install_name_dir;
}
