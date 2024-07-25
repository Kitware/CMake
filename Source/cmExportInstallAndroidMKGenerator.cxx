/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmExportInstallAndroidMKGenerator.h"

#include <cstddef>
#include <memory>
#include <sstream>
#include <vector>

#include "cmExportSet.h"
#include "cmGeneratorExpression.h"
#include "cmGeneratorTarget.h"
#include "cmInstallExportGenerator.h"
#include "cmInstallTargetGenerator.h"
#include "cmPolicies.h"
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

void cmExportInstallAndroidMKGenerator::ReportDuplicateTarget(
  std::string const& targetName) const
{
  std::ostringstream e;
  e << "install(EXPORT_ANDROID_MK \"" << this->GetExportSet()->GetName()
    << "\" ...) "
    << "includes target \"" << targetName
    << "\" more than once in the export set.";
  this->ReportError(e.str());
}

bool cmExportInstallAndroidMKGenerator::GenerateMainFile(std::ostream& os)
{
  std::vector<cmTargetExport const*> allTargets;
  {
    auto visitor = [&](cmTargetExport const* te) { allTargets.push_back(te); };

    if (!this->CollectExports(visitor)) {
      return false;
    }
  }

  // Create all the imported targets.
  for (cmTargetExport const* te : allTargets) {
    cmGeneratorTarget const* gt = te->Target;

    this->GenerateImportTargetCode(os, gt, this->GetExportTargetType(te));

    ImportPropertyMap properties;
    if (!this->PopulateInterfaceProperties(te, properties)) {
      return false;
    }

    bool const newCMP0022Behavior =
      gt->GetPolicyStatusCMP0022() != cmPolicies::WARN &&
      gt->GetPolicyStatusCMP0022() != cmPolicies::OLD;
    if (newCMP0022Behavior) {
      this->PopulateInterfaceLinkLibrariesProperty(
        gt, cmGeneratorExpression::InstallInterface, properties);
    }

    this->GenerateInterfaceProperties(gt, os, properties);
  }

  return true;
}

void cmExportInstallAndroidMKGenerator::GenerateImportHeaderCode(
  std::ostream& os, std::string const&)
{
  std::string installDir = this->IEGen->GetDestination();
  os << "LOCAL_PATH := $(call my-dir)\n";
  size_t numDotDot = cmSystemTools::CountChar(installDir.c_str(), '/');
  numDotDot += installDir.empty() ? 0 : 1;
  std::string path;
  for (size_t n = 0; n < numDotDot; n++) {
    path += "/..";
  }
  os << "_IMPORT_PREFIX := $(LOCAL_PATH)" << path << "\n\n";
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
