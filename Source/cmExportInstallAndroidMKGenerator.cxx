/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmExportInstallAndroidMKGenerator.h"

#include <cstddef>
#include <memory>
#include <ostream>

#include "cmExportBuildAndroidMKGenerator.h"
#include "cmExportSet.h"
#include "cmGeneratorTarget.h"
#include "cmInstallExportGenerator.h"
#include "cmInstallTargetGenerator.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmTarget.h"
#include "cmTargetExport.h"

cmExportInstallAndroidMKGenerator::cmExportInstallAndroidMKGenerator(
  cmInstallExportGenerator* iegen)
  : cmExportInstallFileGenerator(iegen)
{
}

void cmExportInstallAndroidMKGenerator::GenerateImportHeaderCode(
  std::ostream& os, const std::string&)
{
  std::string installDir = this->IEGen->GetDestination();
  os << "LOCAL_PATH := $(call my-dir)\n";
  size_t numDotDot = cmSystemTools::CountChar(installDir.c_str(), '/');
  numDotDot += installDir.empty() ? 0 : 1;
  std::string path;
  for (size_t n = 0; n < numDotDot; n++) {
    path += "/..";
  }
  os << "_IMPORT_PREFIX := "
     << "$(LOCAL_PATH)" << path << "\n\n";
  for (std::unique_ptr<cmTargetExport> const& te :
       this->IEGen->GetExportSet()->GetTargetExports()) {
    // Collect import properties for this target.
    if (te->Target->GetType() == cmStateEnums::INTERFACE_LIBRARY) {
      continue;
    }
    std::string dest;
    if (te->LibraryGenerator) {
      dest = te->LibraryGenerator->GetDestination("");
    }
    if (te->ArchiveGenerator) {
      dest = te->ArchiveGenerator->GetDestination("");
    }
    te->Target->Target->SetProperty("__dest", dest);
  }
}

void cmExportInstallAndroidMKGenerator::GenerateImportFooterCode(std::ostream&)
{
}

void cmExportInstallAndroidMKGenerator::GenerateImportTargetCode(
  std::ostream& os, cmGeneratorTarget const* target,
  cmStateEnums::TargetType /*targetType*/)
{
  std::string targetName = cmStrCat(this->Namespace, target->GetExportName());
  os << "include $(CLEAR_VARS)\n";
  os << "LOCAL_MODULE := ";
  os << targetName << "\n";
  os << "LOCAL_SRC_FILES := $(_IMPORT_PREFIX)/";
  os << target->Target->GetSafeProperty("__dest") << "/";
  std::string config;
  if (!this->Configurations.empty()) {
    config = this->Configurations[0];
  }
  os << target->GetFullName(config) << "\n";
}

void cmExportInstallAndroidMKGenerator::GenerateExpectedTargetsCode(
  std::ostream&, const std::string&)
{
}

void cmExportInstallAndroidMKGenerator::GenerateImportPropertyCode(
  std::ostream&, const std::string&, cmGeneratorTarget const*,
  ImportPropertyMap const&)
{
}

void cmExportInstallAndroidMKGenerator::GenerateMissingTargetsCheckCode(
  std::ostream&, const std::vector<std::string>&)
{
}

void cmExportInstallAndroidMKGenerator::GenerateInterfaceProperties(
  cmGeneratorTarget const* target, std::ostream& os,
  const ImportPropertyMap& properties)
{
  std::string config;
  if (!this->Configurations.empty()) {
    config = this->Configurations[0];
  }
  cmExportBuildAndroidMKGenerator::GenerateInterfaceProperties(
    target, os, properties, cmExportBuildAndroidMKGenerator::INSTALL, config);
}

void cmExportInstallAndroidMKGenerator::LoadConfigFiles(std::ostream&)
{
}

void cmExportInstallAndroidMKGenerator::GenerateImportPrefix(std::ostream&)
{
}

void cmExportInstallAndroidMKGenerator::GenerateRequiredCMakeVersion(
  std::ostream&, const char*)
{
}

void cmExportInstallAndroidMKGenerator::CleanupTemporaryVariables(
  std::ostream&)
{
}

void cmExportInstallAndroidMKGenerator::GenerateImportedFileCheckLoop(
  std::ostream&)
{
}

void cmExportInstallAndroidMKGenerator::GenerateImportedFileChecksCode(
  std::ostream&, cmGeneratorTarget*, ImportPropertyMap const&,
  const std::set<std::string>&)
{
}

bool cmExportInstallAndroidMKGenerator::GenerateImportFileConfig(
  const std::string&, std::vector<std::string>&)
{
  return true;
}
