/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

#include "cmFastbuildUtilityTargetGenerator.h"

#include <set>
#include <string>
#include <utility>
#include <vector>

#include <cm/memory>

#include "cmFastbuildTargetGenerator.h"
#include "cmGeneratorTarget.h"
#include "cmGlobalFastbuildGenerator.h"
#include "cmListFileCache.h"
#include "cmMakefile.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmTarget.h"
#include "cmTargetDepend.h"

cmFastbuildUtilityTargetGenerator::cmFastbuildUtilityTargetGenerator(
  cmGeneratorTarget* gt, std::string configParam)
  : cmFastbuildTargetGenerator(gt, std::move(configParam))
{
}

void cmFastbuildUtilityTargetGenerator::Generate()
{
  std::string targetName = GeneratorTarget->GetName();

  if (this->GeneratorTarget->GetType() == cmStateEnums::GLOBAL_TARGET) {
    targetName = GetGlobalGenerator()->GetTargetName(GeneratorTarget);
  }

  FastbuildAliasNode fastbuildTarget;
  fastbuildTarget.Name = targetName;

  LogMessage("<-------------->");
  LogMessage("Generate Utility target: " + targetName);
  LogMessage("Config: " + Config);
  for (auto const& dep : TargetDirectDependencies) {
    LogMessage("Dep: " + dep->GetName());
  }

  std::vector<std::string> nonImportedUtils;
  for (BT<std::pair<std::string, bool>> const& util :
       this->GeneratorTarget->GetUtilities()) {
    if (util.Value.first == targetName) {
      continue;
    }
    auto const& utilTargetName =
      this->ConvertToFastbuildPath(util.Value.first);
    LogMessage("Util: " + utilTargetName);
    auto* const target = this->Makefile->FindTargetToUse(utilTargetName);
    if (target && target->IsImported()) {
      LogMessage("Skipping imported util target: " + utilTargetName);
      continue;
    }
    // Since interface target don't appear in the generated build files,
    // transitively propagate their deps (if any).
    // Tested in "ExternalProjectSubdir" test.
    if (target && target->GetType() == cmStateEnums::INTERFACE_LIBRARY) {
      for (auto const& dep : target->GetUtilities()) {
        auto const& depName = this->ConvertToFastbuildPath(dep.Value.first);
        LogMessage("Transitively propagating iface dep: " + depName +
                   ", is cross: " + std::to_string(dep.Value.second));
        nonImportedUtils.emplace_back(depName);
        fastbuildTarget.PreBuildDependencies.emplace(
          this->ConvertToFastbuildPath(depName));
      }
    } else {
      nonImportedUtils.emplace_back(utilTargetName);
      fastbuildTarget.PreBuildDependencies.emplace(utilTargetName);
    }
  }
  if (this->GetGlobalGenerator()->IsExcluded(this->GetGeneratorTarget())) {
    LogMessage(cmStrCat("Excluding ", targetName, " from ALL"));
    fastbuildTarget.ExcludeFromAll = true;
  }
  auto preBuild = GenerateCommands(FastbuildBuildStep::PRE_BUILD);

  // Tested in "RunCMake.CPack*" tests.
  // Utility target "package" has packaging steps as "POST_BUILD".
  for (auto& exec : GenerateCommands(FastbuildBuildStep::POST_BUILD).Nodes) {
    fastbuildTarget.PreBuildDependencies.emplace(exec.Name);
    for (std::string const& util : nonImportedUtils) {
      LogMessage("Adding: util " + util);
      exec.PreBuildDependencies.emplace(util);
    }
    // So POST_BUILD is executed AFTER PRE_BUILD (tested in "CustomCommand"
    // test).
    for (auto const& pre : preBuild.Nodes) {
      LogMessage("Adding: " + pre.Name);
      exec.PreBuildDependencies.emplace(pre.Name);
    }
    this->GetGlobalGenerator()->AddTarget(std::move(exec));
  }

  for (auto& exec : preBuild.Nodes) {
    LogMessage("Adding exec " + exec.Name);
    fastbuildTarget.PreBuildDependencies.emplace(exec.Name);
    this->GetGlobalGenerator()->AddTarget(std::move(exec));
  }

  for (auto& exec : GenerateCommands(FastbuildBuildStep::REST).Nodes) {
    fastbuildTarget.PreBuildDependencies.emplace(exec.Name);
    for (auto const& dep : TargetDirectDependencies) {
      LogMessage("Direct dep " + dep->GetName() +
                 "-all propagating to CC: " + exec.Name);
      // All custom commands from within the target must be executed AFTER all
      // the target's deps.
      exec.PreBuildDependencies.emplace(dep->GetName());
    }
    this->GetGlobalGenerator()->AddTarget(std::move(exec));
  }
  if (fastbuildTarget.PreBuildDependencies.empty()) {
    if (fastbuildTarget.ExcludeFromAll) {
      return;
    }
    fastbuildTarget.PreBuildDependencies.emplace(FASTBUILD_NOOP_FILE_NAME);
  }
  fastbuildTarget.Hidden = false;
  this->AdditionalCleanFiles();
  this->GetGlobalGenerator()->AddTarget(std::move(fastbuildTarget));
}
