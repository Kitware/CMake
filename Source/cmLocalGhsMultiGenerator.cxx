/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmLocalGhsMultiGenerator.h"

#include "cmGeneratorTarget.h"
#include "cmGhsMultiTargetGenerator.h"
#include "cmGlobalGenerator.h"
#include "cmSourceFile.h"
#include "cmStateTypes.h"
#include "cmSystemTools.h"

#include <algorithm>
#include <utility>

cmLocalGhsMultiGenerator::cmLocalGhsMultiGenerator(cmGlobalGenerator* gg,
                                                   cmMakefile* mf)
  : cmLocalGenerator(gg, mf)
{
}

cmLocalGhsMultiGenerator::~cmLocalGhsMultiGenerator() = default;

std::string cmLocalGhsMultiGenerator::GetTargetDirectory(
  cmGeneratorTarget const* target) const
{
  std::string dir;
  dir += target->GetName();
  dir += ".dir";
  return dir;
}

void cmLocalGhsMultiGenerator::GenerateTargetsDepthFirst(
  cmGeneratorTarget* target, std::vector<cmGeneratorTarget*>& remaining)
{
  if (target->GetType() == cmStateEnums::INTERFACE_LIBRARY) {
    return;
  }
  // Find this target in the list of remaining targets.
  auto it = std::find(remaining.begin(), remaining.end(), target);
  if (it == remaining.end()) {
    // This target was already handled.
    return;
  }
  // Remove this target from the list of remaining targets because
  // we are handling it now.
  *it = nullptr;

  cmGhsMultiTargetGenerator tg(target);
  tg.Generate();
}

void cmLocalGhsMultiGenerator::Generate()
{
  std::vector<cmGeneratorTarget*> remaining = this->GetGeneratorTargets();
  for (auto& t : remaining) {
    if (t) {
      GenerateTargetsDepthFirst(t, remaining);
    }
  }
}

void cmLocalGhsMultiGenerator::ComputeObjectFilenames(
  std::map<cmSourceFile const*, std::string>& mapping,
  cmGeneratorTarget const* gt)
{
  std::string dir_max;
  dir_max += this->GetCurrentBinaryDirectory();
  dir_max += "/";
  dir_max += this->GetTargetDirectory(gt);
  dir_max += "/";

  // Count the number of object files with each name.  Note that
  // filesystem may not be case sensitive.
  std::map<std::string, int> counts;

  for (auto const& si : mapping) {
    cmSourceFile const* sf = si.first;
    std::string objectNameLower = cmSystemTools::LowerCase(
      cmSystemTools::GetFilenameWithoutLastExtension(sf->GetFullPath()));
    objectNameLower += this->GlobalGenerator->GetLanguageOutputExtension(*sf);
    counts[objectNameLower] += 1;
  }

  // For all source files producing duplicate names we need unique
  // object name computation.
  for (auto& si : mapping) {
    cmSourceFile const* sf = si.first;
    std::string objectName =
      cmSystemTools::GetFilenameWithoutLastExtension(sf->GetFullPath());
    objectName += this->GlobalGenerator->GetLanguageOutputExtension(*sf);

    if (counts[cmSystemTools::LowerCase(objectName)] > 1) {
      const_cast<cmGeneratorTarget*>(gt)->AddExplicitObjectName(sf);
      bool keptSourceExtension;
      objectName = this->GetObjectFileNameWithoutTarget(*sf, dir_max,
                                                        &keptSourceExtension);
      cmsys::SystemTools::ReplaceString(objectName, "/", "_");
    }
    si.second = objectName;
  }
}
