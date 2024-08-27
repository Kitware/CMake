/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmExportInstallFileGenerator.h"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <memory>
#include <set>
#include <sstream>
#include <utility>

#include "cmExportSet.h"
#include "cmGeneratedFileStream.h"
#include "cmGeneratorTarget.h"
#include "cmGlobalGenerator.h"
#include "cmInstallTargetGenerator.h"
#include "cmList.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmPolicies.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmTarget.h"
#include "cmTargetExport.h"
#include "cmValue.h"

cmExportInstallFileGenerator::cmExportInstallFileGenerator(
  cmInstallExportGenerator* iegen)
  : IEGen(iegen)
{
}

void cmExportInstallFileGenerator::ReplaceInstallPrefix(
  std::string& input) const
{
  cmGeneratorExpression::ReplaceInstallPrefix(input, this->GetInstallPrefix());
}

void cmExportInstallFileGenerator::PopulateImportProperties(
  std::string const& config, std::string const& suffix,
  cmTargetExport const* targetExport, ImportPropertyMap& properties,
  std::set<std::string>& importedLocations)
{
  this->SetImportLocationProperty(config, suffix,
                                  targetExport->ArchiveGenerator, properties,
                                  importedLocations);
  this->SetImportLocationProperty(config, suffix,
                                  targetExport->LibraryGenerator, properties,
                                  importedLocations);
  this->SetImportLocationProperty(config, suffix,
                                  targetExport->RuntimeGenerator, properties,
                                  importedLocations);
  this->SetImportLocationProperty(config, suffix,
                                  targetExport->ObjectsGenerator, properties,
                                  importedLocations);
  this->SetImportLocationProperty(config, suffix,
                                  targetExport->FrameworkGenerator, properties,
                                  importedLocations);
  this->SetImportLocationProperty(config, suffix,
                                  targetExport->BundleGenerator, properties,
                                  importedLocations);

  // If any file location was set for the target add it to the
  // import file.
  if (!properties.empty()) {
    // Get the rest of the target details.
    cmGeneratorTarget const* const gtgt = targetExport->Target;
    this->SetImportDetailProperties(config, suffix, gtgt, properties);

    // TODO: PUBLIC_HEADER_LOCATION
    // This should wait until the build feature propagation stuff is done.
    // Then this can be a propagated include directory.
    // this->GenerateImportProperty(config, te->HeaderGenerator, properties);
  }
}

std::string cmExportInstallFileGenerator::GetImportXcFrameworkLocation(
  std::string const& config, cmTargetExport const* targetExport) const
{
  std::string importedXcFrameworkLocation = targetExport->XcFrameworkLocation;
  if (!importedXcFrameworkLocation.empty()) {
    importedXcFrameworkLocation = cmGeneratorExpression::Preprocess(
      importedXcFrameworkLocation,
      cmGeneratorExpression::PreprocessContext::InstallInterface,
      this->GetImportPrefixWithSlash());
    importedXcFrameworkLocation = cmGeneratorExpression::Evaluate(
      importedXcFrameworkLocation, targetExport->Target->GetLocalGenerator(),
      config, targetExport->Target, nullptr, targetExport->Target);
    if (!importedXcFrameworkLocation.empty() &&
        !cmSystemTools::FileIsFullPath(importedXcFrameworkLocation) &&
        !cmHasPrefix(importedXcFrameworkLocation,
                     this->GetImportPrefixWithSlash())) {
      return cmStrCat(this->GetImportPrefixWithSlash(),
                      importedXcFrameworkLocation);
    }
  }

  return importedXcFrameworkLocation;
}

bool cmExportInstallFileGenerator::GenerateImportFileConfig(
  std::string const& config)
{
  // Skip configurations not enabled for this export.
  if (!this->IEGen->InstallsForConfig(config)) {
    return true;
  }

  // Construct the name of the file to generate.
  std::string fileName = cmStrCat(this->FileDir, '/', this->FileBase,
                                  this->GetConfigFileNameSeparator());
  if (!config.empty()) {
    fileName += cmSystemTools::LowerCase(config);
  } else {
    fileName += "noconfig";
  }
  fileName += this->FileExt;

  // Open the output file to generate it.
  cmGeneratedFileStream exportFileStream(fileName, true);
  if (!exportFileStream) {
    std::string se = cmSystemTools::GetLastSystemError();
    std::ostringstream e;
    e << "cannot write to file \"" << fileName << "\": " << se;
    cmSystemTools::Error(e.str());
    return false;
  }
  exportFileStream.SetCopyIfDifferent(true);
  std::ostream& os = exportFileStream;

  // Generate the per-config target information.
  this->GenerateImportConfig(os, config);

  // Record this per-config import file.
  this->ConfigImportFiles[config] = fileName;

  return true;
}

void cmExportInstallFileGenerator::SetImportLocationProperty(
  std::string const& config, std::string const& suffix,
  cmInstallTargetGenerator* itgen, ImportPropertyMap& properties,
  std::set<std::string>& importedLocations)
{
  // Skip rules that do not match this configuration.
  if (!(itgen && itgen->InstallsForConfig(config))) {
    return;
  }

  // Get the target to be installed.
  cmGeneratorTarget* target = itgen->GetTarget();

  // Construct the installed location of the target.
  std::string dest = itgen->GetDestination(config);
  std::string value;
  if (!cmSystemTools::FileIsFullPath(dest)) {
    // The target is installed relative to the installation prefix.
    value = std::string{ this->GetImportPrefixWithSlash() };
  }
  value += dest;
  value += "/";

  if (itgen->IsImportLibrary()) {
    // Construct the property name.
    std::string prop = cmStrCat("IMPORTED_IMPLIB", suffix);

    // Append the installed file name.
    value += cmInstallTargetGenerator::GetInstallFilename(
      target, config, cmInstallTargetGenerator::NameImplibReal);

    // Store the property.
    properties[prop] = value;
    importedLocations.insert(prop);
  } else if (itgen->GetTarget()->GetType() == cmStateEnums::OBJECT_LIBRARY) {
    // Construct the property name.
    std::string prop = cmStrCat("IMPORTED_OBJECTS", suffix);

    // Compute all the object files inside this target and setup
    // IMPORTED_OBJECTS as a list of object files
    std::vector<std::string> objects;
    itgen->GetInstallObjectNames(config, objects);
    for (std::string& obj : objects) {
      obj = cmStrCat(value, obj);
    }

    // Store the property.
    properties[prop] = cmList::to_string(objects);
    importedLocations.insert(prop);
  } else {
    if (target->IsFrameworkOnApple() && target->HasImportLibrary(config)) {
      // store as well IMPLIB value
      auto importProp = cmStrCat("IMPORTED_IMPLIB", suffix);
      auto importValue =
        cmStrCat(value,
                 cmInstallTargetGenerator::GetInstallFilename(
                   target, config, cmInstallTargetGenerator::NameImplibReal));

      // Store the property.
      properties[importProp] = importValue;
      importedLocations.insert(importProp);
    }

    // Construct the property name.
    std::string prop = cmStrCat("IMPORTED_LOCATION", suffix);

    // Append the installed file name.
    if (target->IsAppBundleOnApple()) {
      value += cmInstallTargetGenerator::GetInstallFilename(target, config);
      value += ".app/";
      if (!target->Makefile->PlatformIsAppleEmbedded()) {
        value += "Contents/MacOS/";
      }
      value += cmInstallTargetGenerator::GetInstallFilename(target, config);
    } else {
      value += cmInstallTargetGenerator::GetInstallFilename(
        target, config, cmInstallTargetGenerator::NameReal);
    }

    // Store the property.
    properties[prop] = value;
    importedLocations.insert(prop);
  }
}

cmStateEnums::TargetType cmExportInstallFileGenerator::GetExportTargetType(
  cmTargetExport const* targetExport) const
{
  cmStateEnums::TargetType targetType = targetExport->Target->GetType();
  // An OBJECT library installed with no OBJECTS DESTINATION
  // is transformed to an INTERFACE library.
  if (targetType == cmStateEnums::OBJECT_LIBRARY &&
      !targetExport->ObjectsGenerator) {
    targetType = cmStateEnums::INTERFACE_LIBRARY;
  }
  return targetType;
}

std::string const& cmExportInstallFileGenerator::GetExportName() const
{
  return this->GetExportSet()->GetName();
}

void cmExportInstallFileGenerator::HandleMissingTarget(
  std::string& link_libs, cmGeneratorTarget const* depender,
  cmGeneratorTarget* dependee)
{
  auto const& exportInfo = this->FindExportInfo(dependee);
  auto const& exportFiles = exportInfo.first;

  if (exportFiles.size() == 1) {
    std::string missingTarget = exportInfo.second;

    missingTarget += dependee->GetExportName();
    link_libs += missingTarget;
    this->MissingTargets.emplace_back(std::move(missingTarget));
  } else {
    // All exported targets should be known here and should be unique.
    // This is probably user-error.
    this->ComplainAboutMissingTarget(depender, dependee, exportFiles);
  }
}

cmExportFileGenerator::ExportInfo cmExportInstallFileGenerator::FindExportInfo(
  cmGeneratorTarget const* target) const
{
  std::vector<std::string> exportFiles;
  std::string ns;

  auto const& name = target->GetName();
  auto& exportSets =
    target->GetLocalGenerator()->GetGlobalGenerator()->GetExportSets();

  for (auto const& exp : exportSets) {
    auto const& exportSet = exp.second;
    auto const& targets = exportSet.GetTargetExports();

    if (std::any_of(targets.begin(), targets.end(),
                    [&name](std::unique_ptr<cmTargetExport> const& te) {
                      return te->TargetName == name;
                    })) {
      std::vector<cmInstallExportGenerator const*> const* installs =
        exportSet.GetInstallations();
      for (cmInstallExportGenerator const* install : *installs) {
        exportFiles.push_back(install->GetDestinationFile());
        ns = install->GetNamespace();
      }
    }
  }

  return { exportFiles, exportFiles.size() == 1 ? ns : std::string{} };
}

void cmExportInstallFileGenerator::ComplainAboutMissingTarget(
  cmGeneratorTarget const* depender, cmGeneratorTarget const* dependee,
  std::vector<std::string> const& exportFiles) const
{
  std::ostringstream e;
  e << "install(" << this->IEGen->InstallSubcommand() << " \""
    << this->GetExportName() << "\" ...) "
    << "includes target \"" << depender->GetName()
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

void cmExportInstallFileGenerator::ComplainAboutDuplicateTarget(
  std::string const& targetName) const
{
  std::ostringstream e;
  e << "install(" << this->IEGen->InstallSubcommand() << " \""
    << this->GetExportName() << "\" ...) "
    << "includes target \"" << targetName
    << "\" more than once in the export set.";
  this->ReportError(e.str());
}

void cmExportInstallFileGenerator::ReportError(
  std::string const& errorMessage) const
{
  cmSystemTools::Error(errorMessage);
}

std::string cmExportInstallFileGenerator::InstallNameDir(
  cmGeneratorTarget const* target, std::string const& config)
{
  std::string install_name_dir;

  cmMakefile* mf = target->Target->GetMakefile();
  if (mf->IsOn("CMAKE_PLATFORM_HAS_INSTALLNAME")) {
    auto const& prefix = this->GetInstallPrefix();
    install_name_dir = target->GetInstallNameDirForInstallTree(config, prefix);
  }

  return install_name_dir;
}

std::string cmExportInstallFileGenerator::GetCxxModuleFile() const
{
  return this->GetCxxModuleFile(this->GetExportSet()->GetName());
}

bool cmExportInstallFileGenerator::CollectExports(
  std::function<void(cmTargetExport const*)> const& visitor)
{
  auto pred = [&](std::unique_ptr<cmTargetExport> const& te) -> bool {
    if (te->NamelinkOnly) {
      return true;
    }
    if (this->ExportedTargets.insert(te->Target).second) {
      visitor(te.get());
      return true;
    }

    this->ComplainAboutDuplicateTarget(te->Target->GetName());
    return false;
  };

  auto const& targets = this->GetExportSet()->GetTargetExports();
  return std::all_of(targets.begin(), targets.end(), pred);
}

bool cmExportInstallFileGenerator::PopulateInterfaceProperties(
  cmTargetExport const* targetExport, ImportPropertyMap& properties)
{
  cmGeneratorTarget const* const gt = targetExport->Target;

  std::string includesDestinationDirs;
  this->PopulateInterfaceProperty("INTERFACE_SYSTEM_INCLUDE_DIRECTORIES", gt,
                                  cmGeneratorExpression::InstallInterface,
                                  properties);
  this->PopulateIncludeDirectoriesInterface(
    gt, cmGeneratorExpression::InstallInterface, properties, *targetExport,
    includesDestinationDirs);
  this->PopulateLinkDirectoriesInterface(
    gt, cmGeneratorExpression::InstallInterface, properties);
  this->PopulateLinkDependsInterface(
    gt, cmGeneratorExpression::InstallInterface, properties);
  this->PopulateSourcesInterface(gt, cmGeneratorExpression::InstallInterface,
                                 properties);

  return this->PopulateInterfaceProperties(
    gt, includesDestinationDirs, cmGeneratorExpression::InstallInterface,
    properties);
}

namespace {
bool isSubDirectory(std::string const& a, std::string const& b)
{
  return (cmSystemTools::ComparePath(a, b) ||
          cmSystemTools::IsSubDirectory(a, b));
}
}

bool cmExportInstallFileGenerator::CheckInterfaceDirs(
  std::string const& prepro, cmGeneratorTarget const* target,
  std::string const& prop) const
{
  std::string const& installDir =
    target->Makefile->GetSafeDefinition("CMAKE_INSTALL_PREFIX");
  std::string const& topSourceDir =
    target->GetLocalGenerator()->GetSourceDirectory();
  std::string const& topBinaryDir =
    target->GetLocalGenerator()->GetBinaryDirectory();

  std::vector<std::string> parts;
  cmGeneratorExpression::Split(prepro, parts);

  bool const inSourceBuild = topSourceDir == topBinaryDir;

  bool hadFatalError = false;

  for (std::string const& li : parts) {
    size_t genexPos = cmGeneratorExpression::Find(li);
    if (genexPos == 0) {
      continue;
    }
    if (cmHasPrefix(li, this->GetImportPrefixWithSlash())) {
      continue;
    }
    MessageType messageType = MessageType::FATAL_ERROR;
    std::ostringstream e;
    if (genexPos != std::string::npos) {
      if (prop == "INTERFACE_INCLUDE_DIRECTORIES") {
        switch (target->GetPolicyStatusCMP0041()) {
          case cmPolicies::WARN:
            messageType = MessageType::WARNING;
            e << cmPolicies::GetPolicyWarning(cmPolicies::CMP0041) << "\n";
            break;
          case cmPolicies::OLD:
            continue;
          case cmPolicies::REQUIRED_IF_USED:
          case cmPolicies::REQUIRED_ALWAYS:
          case cmPolicies::NEW:
            hadFatalError = true;
            break; // Issue fatal message.
        }
      } else {
        hadFatalError = true;
      }
    }
    if (!cmSystemTools::FileIsFullPath(li)) {
      /* clang-format off */
      e << "Target \"" << target->GetName() << "\" " << prop <<
           " property contains relative path:\n"
           "  \"" << li << "\"";
      /* clang-format on */
      target->GetLocalGenerator()->IssueMessage(messageType, e.str());
    }
    bool inBinary = isSubDirectory(li, topBinaryDir);
    bool inSource = isSubDirectory(li, topSourceDir);
    if (isSubDirectory(li, installDir)) {
      // The include directory is inside the install tree.  If the
      // install tree is not inside the source tree or build tree then
      // fall through to the checks below that the include directory is not
      // also inside the source tree or build tree.
      bool shouldContinue =
        (!inBinary || isSubDirectory(installDir, topBinaryDir)) &&
        (!inSource || isSubDirectory(installDir, topSourceDir));

      if (prop == "INTERFACE_INCLUDE_DIRECTORIES") {
        if (!shouldContinue) {
          switch (target->GetPolicyStatusCMP0052()) {
            case cmPolicies::WARN: {
              std::ostringstream s;
              s << cmPolicies::GetPolicyWarning(cmPolicies::CMP0052) << "\n";
              s << "Directory:\n    \"" << li
                << "\"\nin "
                   "INTERFACE_INCLUDE_DIRECTORIES of target \""
                << target->GetName()
                << "\" is a subdirectory of the install "
                   "directory:\n    \""
                << installDir
                << "\"\nhowever it is also "
                   "a subdirectory of the "
                << (inBinary ? "build" : "source") << " tree:\n    \""
                << (inBinary ? topBinaryDir : topSourceDir) << "\"\n";
              target->GetLocalGenerator()->IssueMessage(
                MessageType::AUTHOR_WARNING, s.str());
              CM_FALLTHROUGH;
            }
            case cmPolicies::OLD:
              shouldContinue = true;
              break;
            case cmPolicies::REQUIRED_ALWAYS:
            case cmPolicies::REQUIRED_IF_USED:
            case cmPolicies::NEW:
              break;
          }
        }
      }
      if (shouldContinue) {
        continue;
      }
    }
    if (inBinary) {
      /* clang-format off */
      e << "Target \"" << target->GetName() << "\" " << prop <<
           " property contains path:\n"
           "  \"" << li << "\"\nwhich is prefixed in the build directory.";
      /* clang-format on */
      target->GetLocalGenerator()->IssueMessage(messageType, e.str());
    }
    if (!inSourceBuild) {
      if (inSource) {
        e << "Target \"" << target->GetName() << "\" " << prop
          << " property contains path:\n"
             "  \""
          << li << "\"\nwhich is prefixed in the source directory.";
        target->GetLocalGenerator()->IssueMessage(messageType, e.str());
      }
    }
  }
  return !hadFatalError;
}

void cmExportInstallFileGenerator::PopulateSourcesInterface(
  cmGeneratorTarget const* gt,
  cmGeneratorExpression::PreprocessContext preprocessRule,
  ImportPropertyMap& properties)
{
  assert(preprocessRule == cmGeneratorExpression::InstallInterface);

  char const* const propName = "INTERFACE_SOURCES";
  cmValue input = gt->GetProperty(propName);

  if (!input) {
    return;
  }

  if (input->empty()) {
    properties[propName].clear();
    return;
  }

  std::string prepro = cmGeneratorExpression::Preprocess(
    *input, preprocessRule, this->GetImportPrefixWithSlash());
  if (!prepro.empty()) {
    this->ResolveTargetsInGeneratorExpressions(prepro, gt);

    if (!this->CheckInterfaceDirs(prepro, gt, propName)) {
      return;
    }
    properties[propName] = prepro;
  }
}

void cmExportInstallFileGenerator::PopulateIncludeDirectoriesInterface(
  cmGeneratorTarget const* target,
  cmGeneratorExpression::PreprocessContext preprocessRule,
  ImportPropertyMap& properties, cmTargetExport const& te,
  std::string& includesDestinationDirs)
{
  assert(preprocessRule == cmGeneratorExpression::InstallInterface);

  includesDestinationDirs.clear();

  char const* const propName = "INTERFACE_INCLUDE_DIRECTORIES";
  cmValue input = target->GetProperty(propName);

  cmGeneratorExpression ge(*target->Makefile->GetCMakeInstance());

  std::string dirs = cmGeneratorExpression::Preprocess(
    cmList::to_string(target->Target->GetInstallIncludeDirectoriesEntries(te)),
    preprocessRule, this->GetImportPrefixWithSlash());
  this->ReplaceInstallPrefix(dirs);
  std::unique_ptr<cmCompiledGeneratorExpression> cge = ge.Parse(dirs);
  std::string exportDirs =
    cge->Evaluate(target->GetLocalGenerator(), "", target);

  if (cge->GetHadContextSensitiveCondition()) {
    cmLocalGenerator* lg = target->GetLocalGenerator();
    std::ostringstream e;
    e << "Target \"" << target->GetName()
      << "\" is installed with "
         "INCLUDES DESTINATION set to a context sensitive path.  Paths which "
         "depend on the configuration, policy values or the link interface "
         "are "
         "not supported.  Consider using target_include_directories instead.";
    lg->IssueMessage(MessageType::FATAL_ERROR, e.str());
    return;
  }

  if (!input && exportDirs.empty()) {
    return;
  }
  if ((input && input->empty()) && exportDirs.empty()) {
    // Set to empty
    properties[propName].clear();
    return;
  }

  this->AddImportPrefix(exportDirs);
  includesDestinationDirs = exportDirs;

  std::string includes = (input ? *input : "");
  char const* const sep = input ? ";" : "";
  includes += sep + exportDirs;
  std::string prepro = cmGeneratorExpression::Preprocess(
    includes, preprocessRule, this->GetImportPrefixWithSlash());
  if (!prepro.empty()) {
    this->ResolveTargetsInGeneratorExpressions(prepro, target);

    if (!this->CheckInterfaceDirs(prepro, target, propName)) {
      return;
    }
    properties[propName] = prepro;
  }
}

void cmExportInstallFileGenerator::PopulateLinkDependsInterface(
  cmGeneratorTarget const* gt,
  cmGeneratorExpression::PreprocessContext preprocessRule,
  ImportPropertyMap& properties)
{
  assert(preprocessRule == cmGeneratorExpression::InstallInterface);

  char const* const propName = "INTERFACE_LINK_DEPENDS";
  cmValue input = gt->GetProperty(propName);

  if (!input) {
    return;
  }

  if (input->empty()) {
    properties[propName].clear();
    return;
  }

  std::string prepro = cmGeneratorExpression::Preprocess(
    *input, preprocessRule, this->GetImportPrefixWithSlash());
  if (!prepro.empty()) {
    this->ResolveTargetsInGeneratorExpressions(prepro, gt);

    if (!this->CheckInterfaceDirs(prepro, gt, propName)) {
      return;
    }
    properties[propName] = prepro;
  }
}

void cmExportInstallFileGenerator::PopulateLinkDirectoriesInterface(
  cmGeneratorTarget const* gt,
  cmGeneratorExpression::PreprocessContext preprocessRule,
  ImportPropertyMap& properties)
{
  assert(preprocessRule == cmGeneratorExpression::InstallInterface);

  char const* const propName = "INTERFACE_LINK_DIRECTORIES";
  cmValue input = gt->GetProperty(propName);

  if (!input) {
    return;
  }

  if (input->empty()) {
    properties[propName].clear();
    return;
  }

  std::string prepro = cmGeneratorExpression::Preprocess(
    *input, preprocessRule, this->GetImportPrefixWithSlash());
  if (!prepro.empty()) {
    this->ResolveTargetsInGeneratorExpressions(prepro, gt);

    if (!this->CheckInterfaceDirs(prepro, gt, propName)) {
      return;
    }
    properties[propName] = prepro;
  }
}
