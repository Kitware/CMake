/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmGlobalCommonGenerator.h"

#include <memory>
#include <utility>

#include "cmGeneratorTarget.h"
#include "cmLocalGenerator.h"
#include "cmStateDirectory.h"
#include "cmStateSnapshot.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"

class cmake;

cmGlobalCommonGenerator::cmGlobalCommonGenerator(cmake* cm)
  : cmGlobalGenerator(cm)
{
}

cmGlobalCommonGenerator::~cmGlobalCommonGenerator() = default;

std::map<std::string, cmGlobalCommonGenerator::DirectoryTarget>
cmGlobalCommonGenerator::ComputeDirectoryTargets() const
{
  std::map<std::string, DirectoryTarget> dirTargets;
  for (const auto& lg : this->LocalGenerators) {
    std::string const& currentBinaryDir(
      lg->GetStateSnapshot().GetDirectory().GetCurrentBinary());
    DirectoryTarget& dirTarget = dirTargets[currentBinaryDir];
    dirTarget.LG = lg.get();

    // The directory-level rule should depend on the target-level rules
    // for all targets in the directory.
    for (const auto& gt : lg->GetGeneratorTargets()) {
      cmStateEnums::TargetType const type = gt->GetType();
      if (type != cmStateEnums::EXECUTABLE &&
          type != cmStateEnums::STATIC_LIBRARY &&
          type != cmStateEnums::SHARED_LIBRARY &&
          type != cmStateEnums::MODULE_LIBRARY &&
          type != cmStateEnums::OBJECT_LIBRARY &&
          type != cmStateEnums::UTILITY) {
        continue;
      }
      DirectoryTarget::Target t;
      t.GT = gt.get();
      if (const char* exclude = gt->GetProperty("EXCLUDE_FROM_ALL")) {
        if (cmIsOn(exclude)) {
          // This target has been explicitly excluded.
          t.ExcludeFromAll = true;
        } else {
          // This target has been explicitly un-excluded.  The directory-level
          // rule for every directory between this and the root should depend
          // on the target-level rule for this target.
          for (cmStateSnapshot dir =
                 lg->GetStateSnapshot().GetBuildsystemDirectoryParent();
               dir.IsValid(); dir = dir.GetBuildsystemDirectoryParent()) {
            std::string const& d = dir.GetDirectory().GetCurrentBinary();
            dirTargets[d].Targets.emplace_back(t);
          }
        }
      }
      dirTarget.Targets.emplace_back(t);
    }

    // The directory-level rule should depend on the directory-level
    // rules of the subdirectories.
    for (cmStateSnapshot const& state : lg->GetStateSnapshot().GetChildren()) {
      DirectoryTarget::Dir d;
      d.Path = state.GetDirectory().GetCurrentBinary();
      d.ExcludeFromAll =
        state.GetDirectory().GetPropertyAsBool("EXCLUDE_FROM_ALL");
      dirTarget.Children.emplace_back(std::move(d));
    }
  }

  return dirTargets;
}
