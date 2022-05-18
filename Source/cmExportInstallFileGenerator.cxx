/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmExportInstallFileGenerator.h"

#include <algorithm>
#include <memory>
#include <sstream>
#include <utility>

#include "cmExportSet.h"
#include "cmFileSet.h"
#include "cmGeneratedFileStream.h"
#include "cmGeneratorExpression.h"
#include "cmGeneratorTarget.h"
#include "cmGlobalGenerator.h"
#include "cmInstallExportGenerator.h"
#include "cmInstallFileSetGenerator.h"
#include "cmInstallTargetGenerator.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmOutputConverter.h"
#include "cmPolicies.h"
#include "cmStateTypes.h"
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

std::string cmExportInstallFileGenerator::GetConfigImportFileGlob()
{
  std::string glob = cmStrCat(this->FileBase, "-*", this->FileExt);
  return glob;
}

bool cmExportInstallFileGenerator::GenerateMainFile(std::ostream& os)
{
  std::vector<cmTargetExport*> allTargets;
  {
    std::string expectedTargets;
    std::string sep;
    for (std::unique_ptr<cmTargetExport> const& te :
         this->IEGen->GetExportSet()->GetTargetExports()) {
      if (te->NamelinkOnly) {
        continue;
      }
      expectedTargets += sep + this->Namespace + te->Target->GetExportName();
      sep = " ";
      if (this->ExportedTargets.insert(te->Target).second) {
        allTargets.push_back(te.get());
      } else {
        std::ostringstream e;
        e << "install(EXPORT \"" << this->IEGen->GetExportSet()->GetName()
          << "\" ...) "
          << "includes target \"" << te->Target->GetName()
          << "\" more than once in the export set.";
        cmSystemTools::Error(e.str());
        return false;
      }
    }

    this->GenerateExpectedTargetsCode(os, expectedTargets);
  }

  // Compute the relative import prefix for the file
  this->GenerateImportPrefix(os);

  std::vector<std::string> missingTargets;

  bool require2_8_12 = false;
  bool require3_0_0 = false;
  bool require3_1_0 = false;
  bool requiresConfigFiles = false;
  // Create all the imported targets.
  for (cmTargetExport* te : allTargets) {
    cmGeneratorTarget* gt = te->Target;
    cmStateEnums::TargetType targetType = this->GetExportTargetType(te);

    requiresConfigFiles =
      requiresConfigFiles || targetType != cmStateEnums::INTERFACE_LIBRARY;

    this->GenerateImportTargetCode(os, gt, targetType);

    ImportPropertyMap properties;

    this->PopulateIncludeDirectoriesInterface(
      gt, cmGeneratorExpression::InstallInterface, properties, missingTargets,
      *te);
    this->PopulateSourcesInterface(gt, cmGeneratorExpression::InstallInterface,
                                   properties, missingTargets);
    this->PopulateInterfaceProperty("INTERFACE_SYSTEM_INCLUDE_DIRECTORIES", gt,
                                    cmGeneratorExpression::InstallInterface,
                                    properties, missingTargets);
    this->PopulateInterfaceProperty("INTERFACE_COMPILE_DEFINITIONS", gt,
                                    cmGeneratorExpression::InstallInterface,
                                    properties, missingTargets);
    this->PopulateInterfaceProperty("INTERFACE_COMPILE_OPTIONS", gt,
                                    cmGeneratorExpression::InstallInterface,
                                    properties, missingTargets);
    this->PopulateInterfaceProperty("INTERFACE_PRECOMPILE_HEADERS", gt,
                                    cmGeneratorExpression::InstallInterface,
                                    properties, missingTargets);
    this->PopulateInterfaceProperty("INTERFACE_AUTOUIC_OPTIONS", gt,
                                    cmGeneratorExpression::InstallInterface,
                                    properties, missingTargets);
    this->PopulateInterfaceProperty("INTERFACE_COMPILE_FEATURES", gt,
                                    cmGeneratorExpression::InstallInterface,
                                    properties, missingTargets);
    this->PopulateInterfaceProperty("INTERFACE_LINK_OPTIONS", gt,
                                    cmGeneratorExpression::InstallInterface,
                                    properties, missingTargets);
    this->PopulateLinkDirectoriesInterface(
      gt, cmGeneratorExpression::InstallInterface, properties, missingTargets);
    this->PopulateLinkDependsInterface(
      gt, cmGeneratorExpression::InstallInterface, properties, missingTargets);

    std::string errorMessage;
    if (!this->PopulateExportProperties(gt, properties, errorMessage)) {
      cmSystemTools::Error(errorMessage);
      return false;
    }

    const bool newCMP0022Behavior =
      gt->GetPolicyStatusCMP0022() != cmPolicies::WARN &&
      gt->GetPolicyStatusCMP0022() != cmPolicies::OLD;
    if (newCMP0022Behavior) {
      if (this->PopulateInterfaceLinkLibrariesProperty(
            gt, cmGeneratorExpression::InstallInterface, properties,
            missingTargets) &&
          !this->ExportOld) {
        require2_8_12 = true;
      }
    }
    if (targetType == cmStateEnums::INTERFACE_LIBRARY) {
      require3_0_0 = true;
    }
    if (gt->GetProperty("INTERFACE_SOURCES")) {
      // We can only generate INTERFACE_SOURCES in CMake 3.3, but CMake 3.1
      // can consume them.
      require3_1_0 = true;
    }

    this->PopulateInterfaceProperty("INTERFACE_POSITION_INDEPENDENT_CODE", gt,
                                    properties);

    this->PopulateCompatibleInterfaceProperties(gt, properties);

    this->GenerateInterfaceProperties(gt, os, properties);

    this->GenerateTargetFileSets(gt, os, te);
  }

  if (require3_1_0) {
    this->GenerateRequiredCMakeVersion(os, "3.1.0");
  } else if (require3_0_0) {
    this->GenerateRequiredCMakeVersion(os, "3.0.0");
  } else if (require2_8_12) {
    this->GenerateRequiredCMakeVersion(os, "2.8.12");
  }

  this->LoadConfigFiles(os);

  this->CleanupTemporaryVariables(os);
  this->GenerateImportedFileCheckLoop(os);

  bool result = true;
  // Generate an import file for each configuration.
  // Don't do this if we only export INTERFACE_LIBRARY targets.
  if (requiresConfigFiles) {
    for (std::string const& c : this->Configurations) {
      if (!this->GenerateImportFileConfig(c, missingTargets)) {
        result = false;
      }
    }
  }

  this->GenerateMissingTargetsCheckCode(os, missingTargets);

  return result;
}

void cmExportInstallFileGenerator::GenerateImportPrefix(std::ostream& os)
{
  // Set an _IMPORT_PREFIX variable for import location properties
  // to reference if they are relative to the install prefix.
  std::string installPrefix =
    this->IEGen->GetLocalGenerator()->GetMakefile()->GetSafeDefinition(
      "CMAKE_INSTALL_PREFIX");
  std::string const& expDest = this->IEGen->GetDestination();
  if (cmSystemTools::FileIsFullPath(expDest)) {
    // The export file is being installed to an absolute path so the
    // package is not relocatable.  Use the configured install prefix.
    /* clang-format off */
    os <<
      "# The installation prefix configured by this project.\n"
      "set(_IMPORT_PREFIX \"" << installPrefix << "\")\n"
      "\n";
    /* clang-format on */
  } else {
    // Add code to compute the installation prefix relative to the
    // import file location.
    std::string absDest = installPrefix + "/" + expDest;
    std::string absDestS = absDest + "/";
    os << "# Compute the installation prefix relative to this file.\n"
       << "get_filename_component(_IMPORT_PREFIX"
       << " \"${CMAKE_CURRENT_LIST_FILE}\" PATH)\n";
    if (cmHasLiteralPrefix(absDestS, "/lib/") ||
        cmHasLiteralPrefix(absDestS, "/lib64/") ||
        cmHasLiteralPrefix(absDestS, "/libx32/") ||
        cmHasLiteralPrefix(absDestS, "/usr/lib/") ||
        cmHasLiteralPrefix(absDestS, "/usr/lib64/") ||
        cmHasLiteralPrefix(absDestS, "/usr/libx32/")) {
      // Handle "/usr move" symlinks created by some Linux distros.
      /* clang-format off */
      os <<
        "# Use original install prefix when loaded through a\n"
        "# cross-prefix symbolic link such as /lib -> /usr/lib.\n"
        "get_filename_component(_realCurr \"${_IMPORT_PREFIX}\" REALPATH)\n"
        "get_filename_component(_realOrig \"" << absDest << "\" REALPATH)\n"
        "if(_realCurr STREQUAL _realOrig)\n"
        "  set(_IMPORT_PREFIX \"" << absDest << "\")\n"
        "endif()\n"
        "unset(_realOrig)\n"
        "unset(_realCurr)\n";
      /* clang-format on */
    }
    std::string dest = expDest;
    while (!dest.empty()) {
      os << "get_filename_component(_IMPORT_PREFIX \"${_IMPORT_PREFIX}\" "
            "PATH)\n";
      dest = cmSystemTools::GetFilenamePath(dest);
    }
    os << "if(_IMPORT_PREFIX STREQUAL \"/\")\n"
       << "  set(_IMPORT_PREFIX \"\")\n"
       << "endif()\n"
       << "\n";
  }
}

void cmExportInstallFileGenerator::CleanupTemporaryVariables(std::ostream& os)
{
  /* clang-format off */
  os << "# Cleanup temporary variables.\n"
     << "set(_IMPORT_PREFIX)\n"
     << "\n";
  /* clang-format on */
}

void cmExportInstallFileGenerator::LoadConfigFiles(std::ostream& os)
{
  // Now load per-configuration properties for them.
  /* clang-format off */
  os << "# Load information for each installed configuration.\n"
     << "get_filename_component(_DIR \"${CMAKE_CURRENT_LIST_FILE}\" PATH)\n"
     << "file(GLOB CONFIG_FILES \"${_DIR}/"
     << this->GetConfigImportFileGlob() << "\")\n"
     << "foreach(f ${CONFIG_FILES})\n"
     << "  include(${f})\n"
     << "endforeach()\n"
     << "\n";
  /* clang-format on */
}

void cmExportInstallFileGenerator::ReplaceInstallPrefix(std::string& input)
{
  cmGeneratorExpression::ReplaceInstallPrefix(input, "${_IMPORT_PREFIX}");
}

bool cmExportInstallFileGenerator::GenerateImportFileConfig(
  const std::string& config, std::vector<std::string>& missingTargets)
{
  // Skip configurations not enabled for this export.
  if (!this->IEGen->InstallsForConfig(config)) {
    return true;
  }

  // Construct the name of the file to generate.
  std::string fileName = cmStrCat(this->FileDir, '/', this->FileBase, '-');
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

  // Start with the import file header.
  this->GenerateImportHeaderCode(os, config);

  // Generate the per-config target information.
  this->GenerateImportConfig(os, config, missingTargets);

  // End with the import file footer.
  this->GenerateImportFooterCode(os);

  // Record this per-config import file.
  this->ConfigImportFiles[config] = fileName;

  return true;
}

void cmExportInstallFileGenerator::GenerateImportTargetsConfig(
  std::ostream& os, const std::string& config, std::string const& suffix,
  std::vector<std::string>& missingTargets)
{
  // Add each target in the set to the export.
  for (std::unique_ptr<cmTargetExport> const& te :
       this->IEGen->GetExportSet()->GetTargetExports()) {
    // Collect import properties for this target.
    if (this->GetExportTargetType(te.get()) ==
        cmStateEnums::INTERFACE_LIBRARY) {
      continue;
    }

    ImportPropertyMap properties;
    std::set<std::string> importedLocations;

    this->SetImportLocationProperty(config, suffix, te->ArchiveGenerator,
                                    properties, importedLocations);
    this->SetImportLocationProperty(config, suffix, te->LibraryGenerator,
                                    properties, importedLocations);
    this->SetImportLocationProperty(config, suffix, te->RuntimeGenerator,
                                    properties, importedLocations);
    this->SetImportLocationProperty(config, suffix, te->ObjectsGenerator,
                                    properties, importedLocations);
    this->SetImportLocationProperty(config, suffix, te->FrameworkGenerator,
                                    properties, importedLocations);
    this->SetImportLocationProperty(config, suffix, te->BundleGenerator,
                                    properties, importedLocations);

    // If any file location was set for the target add it to the
    // import file.
    if (!properties.empty()) {
      // Get the rest of the target details.
      cmGeneratorTarget* gtgt = te->Target;
      this->SetImportDetailProperties(config, suffix, gtgt, properties,
                                      missingTargets);

      this->SetImportLinkInterface(config, suffix,
                                   cmGeneratorExpression::InstallInterface,
                                   gtgt, properties, missingTargets);

      // TODO: PUBLIC_HEADER_LOCATION
      // This should wait until the build feature propagation stuff
      // is done.  Then this can be a propagated include directory.
      // this->GenerateImportProperty(config, te->HeaderGenerator,
      //                              properties);

      // Generate code in the export file.
      this->GenerateImportPropertyCode(os, config, gtgt, properties);
      this->GenerateImportedFileChecksCode(os, gtgt, properties,
                                           importedLocations);
    }
  }
}

void cmExportInstallFileGenerator::SetImportLocationProperty(
  const std::string& config, std::string const& suffix,
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
    value = "${_IMPORT_PREFIX}/";
  }
  value += dest;
  value += "/";

  if (itgen->IsImportLibrary()) {
    // Construct the property name.
    std::string prop = cmStrCat("IMPORTED_IMPLIB", suffix);

    // Append the installed file name.
    value += cmInstallTargetGenerator::GetInstallFilename(
      target, config, cmInstallTargetGenerator::NameImplib);

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
    properties[prop] = cmJoin(objects, ";");
    importedLocations.insert(prop);
  } else {
    // Construct the property name.
    std::string prop = cmStrCat("IMPORTED_LOCATION", suffix);

    // Append the installed file name.
    if (target->IsAppBundleOnApple()) {
      value += cmInstallTargetGenerator::GetInstallFilename(target, config);
      value += ".app/Contents/MacOS/";
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
      targetExport->ObjectsGenerator == nullptr) {
    targetType = cmStateEnums::INTERFACE_LIBRARY;
  }
  return targetType;
}

void cmExportInstallFileGenerator::HandleMissingTarget(
  std::string& link_libs, std::vector<std::string>& missingTargets,
  cmGeneratorTarget const* depender, cmGeneratorTarget* dependee)
{
  const std::string name = dependee->GetName();
  cmGlobalGenerator* gg = dependee->GetLocalGenerator()->GetGlobalGenerator();
  auto exportInfo = this->FindNamespaces(gg, name);
  std::vector<std::string> const& exportFiles = exportInfo.first;
  if (exportFiles.size() == 1) {
    std::string missingTarget = exportInfo.second;

    missingTarget += dependee->GetExportName();
    link_libs += missingTarget;
    missingTargets.push_back(std::move(missingTarget));
  } else {
    // All exported targets should be known here and should be unique.
    // This is probably user-error.
    this->ComplainAboutMissingTarget(depender, dependee, exportFiles);
  }
}

std::pair<std::vector<std::string>, std::string>
cmExportInstallFileGenerator::FindNamespaces(cmGlobalGenerator* gg,
                                             const std::string& name)
{
  std::vector<std::string> exportFiles;
  std::string ns;
  const cmExportSetMap& exportSets = gg->GetExportSets();

  for (auto const& expIt : exportSets) {
    const cmExportSet& exportSet = expIt.second;

    bool containsTarget = false;
    for (auto const& target : exportSet.GetTargetExports()) {
      if (name == target->TargetName) {
        containsTarget = true;
        break;
      }
    }

    if (containsTarget) {
      std::vector<cmInstallExportGenerator const*> const* installs =
        exportSet.GetInstallations();
      for (cmInstallExportGenerator const* install : *installs) {
        exportFiles.push_back(install->GetDestinationFile());
        ns = install->GetNamespace();
      }
    }
  }

  return { exportFiles, ns };
}

void cmExportInstallFileGenerator::ComplainAboutMissingTarget(
  cmGeneratorTarget const* depender, cmGeneratorTarget const* dependee,
  std::vector<std::string> const& exportFiles)
{
  std::ostringstream e;
  e << "install(EXPORT \"" << this->IEGen->GetExportSet()->GetName()
    << "\" ...) "
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
  cmSystemTools::Error(e.str());
}

std::string cmExportInstallFileGenerator::InstallNameDir(
  cmGeneratorTarget const* target, const std::string& config)
{
  std::string install_name_dir;

  cmMakefile* mf = target->Target->GetMakefile();
  if (mf->IsOn("CMAKE_PLATFORM_HAS_INSTALLNAME")) {
    install_name_dir =
      target->GetInstallNameDirForInstallTree(config, "${_IMPORT_PREFIX}");
  }

  return install_name_dir;
}

namespace {
bool EntryIsContextSensitive(
  const std::unique_ptr<cmCompiledGeneratorExpression>& cge)
{
  return cge->GetHadContextSensitiveCondition();
}
}

std::string cmExportInstallFileGenerator::GetFileSetDirectories(
  cmGeneratorTarget* gte, cmFileSet* fileSet, cmTargetExport* te)
{
  std::vector<std::string> resultVector;

  auto configs =
    gte->Makefile->GetGeneratorConfigs(cmMakefile::IncludeEmptyConfig);

  cmGeneratorExpression ge;
  auto cge = ge.Parse(te->FileSetGenerators.at(fileSet)->GetDestination());

  for (auto const& config : configs) {
    auto dest = cmStrCat("${_IMPORT_PREFIX}/",
                         cmOutputConverter::EscapeForCMake(
                           cge->Evaluate(gte->LocalGenerator, config, gte),
                           cmOutputConverter::WrapQuotes::NoWrap));

    if (cge->GetHadContextSensitiveCondition() && configs.size() != 1) {
      resultVector.push_back(
        cmStrCat("\"$<$<CONFIG:", config, ">:", dest, ">\""));
    } else {
      resultVector.push_back(cmStrCat('"', dest, '"'));
      break;
    }
  }

  return cmJoin(resultVector, " ");
}

std::string cmExportInstallFileGenerator::GetFileSetFiles(
  cmGeneratorTarget* gte, cmFileSet* fileSet, cmTargetExport* te)
{
  std::vector<std::string> resultVector;

  auto configs =
    gte->Makefile->GetGeneratorConfigs(cmMakefile::IncludeEmptyConfig);

  auto fileEntries = fileSet->CompileFileEntries();
  auto directoryEntries = fileSet->CompileDirectoryEntries();

  cmGeneratorExpression destGe;
  auto destCge =
    destGe.Parse(te->FileSetGenerators.at(fileSet)->GetDestination());

  for (auto const& config : configs) {
    auto directories = fileSet->EvaluateDirectoryEntries(
      directoryEntries, gte->LocalGenerator, config, gte);

    std::map<std::string, std::vector<std::string>> files;
    for (auto const& entry : fileEntries) {
      fileSet->EvaluateFileEntry(directories, files, entry,
                                 gte->LocalGenerator, config, gte);
    }
    auto dest = cmStrCat("${_IMPORT_PREFIX}/",
                         cmOutputConverter::EscapeForCMake(
                           destCge->Evaluate(gte->LocalGenerator, config, gte),
                           cmOutputConverter::WrapQuotes::NoWrap),
                         '/');

    bool const contextSensitive = destCge->GetHadContextSensitiveCondition() ||
      std::any_of(directoryEntries.begin(), directoryEntries.end(),
                  EntryIsContextSensitive) ||
      std::any_of(fileEntries.begin(), fileEntries.end(),
                  EntryIsContextSensitive);

    for (auto const& it : files) {
      auto prefix = it.first.empty() ? "" : cmStrCat(it.first, '/');
      for (auto const& filename : it.second) {
        auto relFile =
          cmStrCat(prefix, cmSystemTools::GetFilenameName(filename));
        auto escapedFile =
          cmStrCat(dest,
                   cmOutputConverter::EscapeForCMake(
                     relFile, cmOutputConverter::WrapQuotes::NoWrap));
        if (contextSensitive && configs.size() != 1) {
          resultVector.push_back(
            cmStrCat("\"$<$<CONFIG:", config, ">:", escapedFile, ">\""));
        } else {
          resultVector.push_back(cmStrCat('"', escapedFile, '"'));
        }
      }
    }

    if (!(contextSensitive && configs.size() != 1)) {
      break;
    }
  }

  return cmJoin(resultVector, " ");
}
