/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmLocalGhsMultiGenerator.h"

#include <map>
#include <utility>
#include <vector>

#include <cm/optional>

#include "cmGeneratorTarget.h"
#include "cmGhsMultiTargetGenerator.h"
#include "cmGlobalGenerator.h"
#include "cmObjectLocation.h"
#include "cmSourceFile.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

cmLocalGhsMultiGenerator::cmLocalGhsMultiGenerator(cmGlobalGenerator* gg,
                                                   cmMakefile* mf)
  : cmLocalGenerator(gg, mf)
{
}

cmLocalGhsMultiGenerator::~cmLocalGhsMultiGenerator() = default;

std::string cmLocalGhsMultiGenerator::GetTargetDirectory(
  cmGeneratorTarget const* target,
  cmStateEnums::IntermediateDirKind /*kind*/) const
{
  std::string dir = cmStrCat(target->GetName(), ".dir");
  return dir;
}

void cmLocalGhsMultiGenerator::Generate()
{
  for (cmGeneratorTarget* gt :
       this->GlobalGenerator->GetLocalGeneratorTargetsInOrder(this)) {
    if (!gt->IsInBuildSystem()) {
      continue;
    }

    cmGhsMultiTargetGenerator tg(gt);
    tg.Generate();
  }
}

void cmLocalGhsMultiGenerator::ComputeObjectFilenames(
  std::map<cmSourceFile const*, cmObjectLocations>& mapping,
  std::string const& config, cmGeneratorTarget const* gt)
{
  std::string dir_max = cmStrCat(gt->GetSupportDirectory(), '/');

  // Count the number of object files with each name.  Note that
  // filesystem may not be case sensitive.
  std::map<std::string, int> counts;

  for (auto const& si : mapping) {
    cmSourceFile const* sf = si.first;
    std::string objectName;
    auto customObjectName = this->GetCustomObjectFileName(*sf);
    if (customObjectName.empty()) {
      objectName =
        cmSystemTools::GetFilenameWithoutLastExtension(sf->GetFullPath());
    } else {
      objectName = std::move(customObjectName);
    }
    objectName += this->GlobalGenerator->GetLanguageOutputExtension(*sf);
    std::string objectNameLower = cmSystemTools::LowerCase(objectName);
    counts[objectNameLower] += 1;
  }

  // For all source files producing duplicate names we need unique
  // object name computation.
  for (auto& si : mapping) {
    cmSourceFile const* sf = si.first;
    bool forceShortObjectName = true;
    std::string shortObjectName = this->GetObjectFileNameWithoutTarget(
      *sf, dir_max, nullptr, nullptr, &forceShortObjectName);
    std::string longObjectName;
    auto customObjectName = this->GetCustomObjectFileName(*sf);
    if (customObjectName.empty()) {
      longObjectName =
        cmSystemTools::GetFilenameWithoutLastExtension(sf->GetFullPath());
    } else {
      longObjectName = std::move(customObjectName);
      const_cast<cmGeneratorTarget*>(gt)->AddExplicitObjectName(sf);
    }
    longObjectName += this->GlobalGenerator->GetLanguageOutputExtension(*sf);

    if (counts[cmSystemTools::LowerCase(longObjectName)] > 1) {
      const_cast<cmGeneratorTarget*>(gt)->AddExplicitObjectName(sf);
      forceShortObjectName = false;
      longObjectName = this->GetObjectFileNameWithoutTarget(
        *sf, dir_max, nullptr, nullptr, &forceShortObjectName);
      cmsys::SystemTools::ReplaceString(longObjectName, "/", "_");
    }
    si.second.ShortLoc.emplace(shortObjectName);
    si.second.LongLoc.Update(longObjectName);
    this->FillCustomInstallObjectLocations(*sf, config, nullptr,
                                           si.second.InstallLongLoc);
  }
}
