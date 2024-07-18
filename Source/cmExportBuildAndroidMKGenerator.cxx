/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmExportBuildAndroidMKGenerator.h"

#include <sstream>
#include <vector>

#include "cmGeneratorExpression.h"
#include "cmGeneratorTarget.h"
#include "cmPolicies.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmTarget.h"

cmExportBuildAndroidMKGenerator::cmExportBuildAndroidMKGenerator() = default;

bool cmExportBuildAndroidMKGenerator::GenerateMainFile(std::ostream& os)
{
  if (!this->CollectExports([&](cmGeneratorTarget const*) {})) {
    return false;
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
  }

  return true;
}

void cmExportBuildAndroidMKGenerator::GenerateImportHeaderCode(
  std::ostream& os, std::string const&)
{
  os << "LOCAL_PATH := $(call my-dir)\n\n";
}

void cmExportBuildAndroidMKGenerator::GenerateImportTargetCode(
  std::ostream& os, cmGeneratorTarget const* target,
  cmStateEnums::TargetType /*targetType*/)
{
  std::string targetName = cmStrCat(this->Namespace, target->GetExportName());
  os << "include $(CLEAR_VARS)\n";
  os << "LOCAL_MODULE := ";
  os << targetName << "\n";
  os << "LOCAL_SRC_FILES := ";
  std::string const noConfig; // FIXME: What config to use here?
  std::string path =
    cmSystemTools::ConvertToOutputPath(target->GetFullPath(noConfig));
  os << path << "\n";
}
