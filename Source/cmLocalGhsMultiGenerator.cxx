/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmLocalGhsMultiGenerator.h"

#include "cmGeneratedFileStream.h"
#include "cmGeneratorTarget.h"
#include "cmGhsMultiTargetGenerator.h"
#include "cmGlobalGhsMultiGenerator.h"
#include "cmMakefile.h"

cmLocalGhsMultiGenerator::cmLocalGhsMultiGenerator(cmGlobalGenerator* gg,
                                                   cmMakefile* mf)
  : cmLocalGenerator(gg, mf)
{
}

cmLocalGhsMultiGenerator::~cmLocalGhsMultiGenerator()
{
}

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
