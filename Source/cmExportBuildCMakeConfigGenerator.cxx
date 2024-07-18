/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmExportBuildCMakeConfigGenerator.h"

#include <algorithm>
#include <cstddef>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <utility>
#include <vector>

#include <cm/string_view>
#include <cmext/string_view>

#include "cmCryptoHash.h"
#include "cmExportSet.h"
#include "cmFileSet.h"
#include "cmGeneratedFileStream.h"
#include "cmGeneratorExpression.h"
#include "cmGeneratorTarget.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmOutputConverter.h"
#include "cmPolicies.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmTarget.h"

cmExportBuildCMakeConfigGenerator::cmExportBuildCMakeConfigGenerator()
{
  this->LG = nullptr;
  this->ExportSet = nullptr;
}

bool cmExportBuildCMakeConfigGenerator::GenerateMainFile(std::ostream& os)
{
  {
    std::string expectedTargets;
    std::string sep;
    bool generatedInterfaceRequired = false;
    auto visitor = [&](cmGeneratorTarget const* te) {
      expectedTargets += sep + this->Namespace + te->GetExportName();
      sep = " ";

      generatedInterfaceRequired |=
        this->GetExportTargetType(te) == cmStateEnums::INTERFACE_LIBRARY;
    };

    if (!this->CollectExports(visitor)) {
      return false;
    }

    if (generatedInterfaceRequired) {
      this->SetRequiredCMakeVersion(3, 0, 0);
    }
    this->GenerateExpectedTargetsCode(os, expectedTargets);
  }

  // Create all the imported targets.
  for (auto const& exp : this->Exports) {
    cmGeneratorTarget* gte = exp.Target;
    this->GenerateImportTargetCode(os, gte, this->GetExportTargetType(gte));

    gte->Target->AppendBuildInterfaceIncludes();

    ImportPropertyMap properties;
    if (!this->PopulateInterfaceProperties(gte, properties)) {
      return false;
    }

    bool const newCMP0022Behavior =
      gte->GetPolicyStatusCMP0022() != cmPolicies::WARN &&
      gte->GetPolicyStatusCMP0022() != cmPolicies::OLD;
    if (newCMP0022Behavior) {
      this->PopulateInterfaceLinkLibrariesProperty(
        gte, cmGeneratorExpression::BuildInterface, properties);
    }

    this->GenerateInterfaceProperties(gte, os, properties);

    this->GenerateTargetFileSets(gte, os);
  }

  std::string cxx_modules_name;
  if (this->ExportSet) {
    cxx_modules_name = this->ExportSet->GetName();
  } else {
    cmCryptoHash hasher(cmCryptoHash::AlgoSHA3_512);
    constexpr std::size_t HASH_TRUNCATION = 12;
    for (auto const& target : this->Targets) {
      hasher.Append(target.Name);
    }
    cxx_modules_name = hasher.FinalizeHex().substr(0, HASH_TRUNCATION);
  }

  this->GenerateCxxModuleInformation(cxx_modules_name, os);

  // Generate import file content for each configuration.
  for (std::string const& c : this->Configurations) {
    this->GenerateImportConfig(os, c);
  }

  // Generate import file content for each configuration.
  for (std::string const& c : this->Configurations) {
    this->GenerateImportCxxModuleConfigTargetInclusion(cxx_modules_name, c);
  }

  this->GenerateMissingTargetsCheckCode(os);

  return true;
}

void cmExportBuildCMakeConfigGenerator::GenerateImportTargetsConfig(
  std::ostream& os, std::string const& config, std::string const& suffix)
{
  for (auto const& exp : this->Exports) {
    cmGeneratorTarget* target = exp.Target;

    // Collect import properties for this target.
    ImportPropertyMap properties;

    if (this->GetExportTargetType(target) != cmStateEnums::INTERFACE_LIBRARY) {
      this->SetImportLocationProperty(config, suffix, target, properties);
    }
    if (!properties.empty()) {
      // Get the rest of the target details.
      if (this->GetExportTargetType(target) !=
          cmStateEnums::INTERFACE_LIBRARY) {
        this->SetImportDetailProperties(config, suffix, target, properties);
        this->SetImportLinkInterface(config, suffix,
                                     cmGeneratorExpression::BuildInterface,
                                     target, properties);
      }

      // TODO: PUBLIC_HEADER_LOCATION
      // This should wait until the build feature propagation stuff
      // is done.  Then this can be a propagated include directory.
      // this->GenerateImportProperty(config, te->HeaderGenerator,
      //                              properties);

      // Generate code in the export file.
      std::string importedXcFrameworkLocation = exp.XcFrameworkLocation;
      if (!importedXcFrameworkLocation.empty()) {
        importedXcFrameworkLocation = cmGeneratorExpression::Preprocess(
          importedXcFrameworkLocation,
          cmGeneratorExpression::PreprocessContext::BuildInterface);
        importedXcFrameworkLocation = cmGeneratorExpression::Evaluate(
          importedXcFrameworkLocation, exp.Target->GetLocalGenerator(), config,
          exp.Target, nullptr, exp.Target);
        if (!importedXcFrameworkLocation.empty() &&
            !cmSystemTools::FileIsFullPath(importedXcFrameworkLocation)) {
          importedXcFrameworkLocation =
            cmStrCat(this->LG->GetCurrentBinaryDirectory(), '/',
                     importedXcFrameworkLocation);
        }
      }
      this->GenerateImportPropertyCode(os, config, suffix, target, properties,
                                       importedXcFrameworkLocation);
    }
  }
}

namespace {
bool EntryIsContextSensitive(
  std::unique_ptr<cmCompiledGeneratorExpression> const& cge)
{
  return cge->GetHadContextSensitiveCondition();
}
}

std::string cmExportBuildCMakeConfigGenerator::GetFileSetDirectories(
  cmGeneratorTarget* gte, cmFileSet* fileSet, cmTargetExport const* /*te*/)
{
  std::vector<std::string> resultVector;

  auto configs =
    gte->Makefile->GetGeneratorConfigs(cmMakefile::IncludeEmptyConfig);
  auto directoryEntries = fileSet->CompileDirectoryEntries();

  for (auto const& config : configs) {
    auto directories = fileSet->EvaluateDirectoryEntries(
      directoryEntries, gte->LocalGenerator, config, gte);

    bool const contextSensitive =
      std::any_of(directoryEntries.begin(), directoryEntries.end(),
                  EntryIsContextSensitive);

    auto const& type = fileSet->GetType();
    // C++ modules do not support interface file sets which are dependent upon
    // the configuration.
    if (contextSensitive && type == "CXX_MODULES"_s) {
      auto* mf = this->LG->GetMakefile();
      std::ostringstream e;
      e << "The \"" << gte->GetName() << "\" target's interface file set \""
        << fileSet->GetName() << "\" of type \"" << type
        << "\" contains context-sensitive base directory entries which is not "
           "supported.";
      mf->IssueMessage(MessageType::FATAL_ERROR, e.str());
      return std::string{};
    }

    for (auto const& directory : directories) {
      auto dest = cmOutputConverter::EscapeForCMake(
        directory, cmOutputConverter::WrapQuotes::NoWrap);

      if (contextSensitive && configs.size() != 1) {
        resultVector.push_back(
          cmStrCat("\"$<$<CONFIG:", config, ">:", dest, ">\""));
      } else {
        resultVector.emplace_back(cmStrCat('"', dest, '"'));
        break;
      }
    }
  }

  return cmJoin(resultVector, " ");
}

std::string cmExportBuildCMakeConfigGenerator::GetFileSetFiles(
  cmGeneratorTarget* gte, cmFileSet* fileSet, cmTargetExport const* /*te*/)
{
  std::vector<std::string> resultVector;

  auto configs =
    gte->Makefile->GetGeneratorConfigs(cmMakefile::IncludeEmptyConfig);

  auto fileEntries = fileSet->CompileFileEntries();
  auto directoryEntries = fileSet->CompileDirectoryEntries();

  for (auto const& config : configs) {
    auto directories = fileSet->EvaluateDirectoryEntries(
      directoryEntries, gte->LocalGenerator, config, gte);

    std::map<std::string, std::vector<std::string>> files;
    for (auto const& entry : fileEntries) {
      fileSet->EvaluateFileEntry(directories, files, entry,
                                 gte->LocalGenerator, config, gte);
    }

    bool const contextSensitive =
      std::any_of(directoryEntries.begin(), directoryEntries.end(),
                  EntryIsContextSensitive) ||
      std::any_of(fileEntries.begin(), fileEntries.end(),
                  EntryIsContextSensitive);

    auto const& type = fileSet->GetType();
    // C++ modules do not support interface file sets which are dependent upon
    // the configuration.
    if (contextSensitive && type == "CXX_MODULES"_s) {
      auto* mf = this->LG->GetMakefile();
      std::ostringstream e;
      e << "The \"" << gte->GetName() << "\" target's interface file set \""
        << fileSet->GetName() << "\" of type \"" << type
        << "\" contains context-sensitive file entries which is not "
           "supported.";
      mf->IssueMessage(MessageType::FATAL_ERROR, e.str());
      return std::string{};
    }

    for (auto const& it : files) {
      for (auto const& filename : it.second) {
        auto escapedFile = cmOutputConverter::EscapeForCMake(
          filename, cmOutputConverter::WrapQuotes::NoWrap);
        if (contextSensitive && configs.size() != 1) {
          resultVector.push_back(
            cmStrCat("\"$<$<CONFIG:", config, ">:", escapedFile, ">\""));
        } else {
          resultVector.emplace_back(cmStrCat('"', escapedFile, '"'));
        }
      }
    }

    if (!(contextSensitive && configs.size() != 1)) {
      break;
    }
  }

  return cmJoin(resultVector, " ");
}

void cmExportBuildCMakeConfigGenerator::GenerateCxxModuleConfigInformation(
  std::string const& name, std::ostream& os) const
{
  char const* opt = "";
  if (this->Configurations.size() > 1) {
    // With more than one configuration, each individual file is optional.
    opt = " OPTIONAL";
  }

  // Generate import file content for each configuration.
  for (std::string c : this->Configurations) {
    if (c.empty()) {
      c = "noconfig";
    }
    os << "include(\"${CMAKE_CURRENT_LIST_DIR}/cxx-modules-" << name << '-'
       << c << ".cmake\"" << opt << ")\n";
  }
}

bool cmExportBuildCMakeConfigGenerator::
  GenerateImportCxxModuleConfigTargetInclusion(std::string const& name,
                                               std::string config) const
{
  auto cxx_modules_dirname = this->GetCxxModulesDirectory();
  if (cxx_modules_dirname.empty()) {
    return true;
  }

  if (config.empty()) {
    config = "noconfig";
  }

  std::string fileName =
    cmStrCat(this->FileDir, '/', cxx_modules_dirname, "/cxx-modules-", name,
             '-', config, ".cmake");

  cmGeneratedFileStream os(fileName, true);
  if (!os) {
    std::string se = cmSystemTools::GetLastSystemError();
    std::ostringstream e;
    e << "cannot write to file \"" << fileName << "\": " << se;
    cmSystemTools::Error(e.str());
    return false;
  }
  os.SetCopyIfDifferent(true);

  for (auto const* tgt : this->ExportedTargets) {
    // Only targets with C++ module sources will have a
    // collator-generated install script.
    if (!tgt->HaveCxx20ModuleSources()) {
      continue;
    }

    os << "include(\"${CMAKE_CURRENT_LIST_DIR}/target-"
       << tgt->GetFilesystemExportName() << '-' << config << ".cmake\")\n";
  }

  return true;
}
