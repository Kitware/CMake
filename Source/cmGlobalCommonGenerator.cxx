/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmGlobalCommonGenerator.h"

#include <algorithm>
#include <memory>
#include <utility>

#include <cmext/algorithm>

#include <cmsys/Glob.hxx>

#include "cmGeneratorExpression.h"
#include "cmGeneratorTarget.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmStateDirectory.h"
#include "cmStateSnapshot.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmValue.h"
#include "cmake.h"

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
    std::string currentBinaryDir =
      lg->GetStateSnapshot().GetDirectory().GetCurrentBinary();
    DirectoryTarget& dirTarget = dirTargets[currentBinaryDir];
    dirTarget.LG = lg.get();
    const std::vector<std::string>& configs =
      lg->GetMakefile()->GetGeneratorConfigs(cmMakefile::IncludeEmptyConfig);

    // The directory-level rule should depend on the target-level rules
    // for all targets in the directory.
    for (const auto& gt : lg->GetGeneratorTargets()) {
      cmStateEnums::TargetType const type = gt->GetType();
      if (type == cmStateEnums::GLOBAL_TARGET || !gt->IsInBuildSystem()) {
        continue;
      }
      DirectoryTarget::Target t;
      t.GT = gt.get();
      const std::string EXCLUDE_FROM_ALL("EXCLUDE_FROM_ALL");
      if (cmValue exclude = gt->GetProperty(EXCLUDE_FROM_ALL)) {
        for (const std::string& config : configs) {
          cmGeneratorExpressionInterpreter genexInterpreter(lg.get(), config,
                                                            gt.get());
          if (cmIsOn(genexInterpreter.Evaluate(*exclude, EXCLUDE_FROM_ALL))) {
            // This target has been explicitly excluded.
            t.ExcludedFromAllInConfigs.push_back(config);
          }
        }

        if (t.ExcludedFromAllInConfigs.empty()) {
          // This target has been explicitly un-excluded.  The directory-level
          // rule for every directory between this and the root should depend
          // on the target-level rule for this target.
          for (cmStateSnapshot dir =
                 lg->GetStateSnapshot().GetBuildsystemDirectoryParent();
               dir.IsValid(); dir = dir.GetBuildsystemDirectoryParent()) {
            std::string d = dir.GetDirectory().GetCurrentBinary();
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

bool cmGlobalCommonGenerator::IsExcludedFromAllInConfig(
  const DirectoryTarget::Target& t, const std::string& config)
{
  if (this->IsMultiConfig()) {
    return cm::contains(t.ExcludedFromAllInConfigs, config);
  }
  return !t.ExcludedFromAllInConfigs.empty();
}

std::string cmGlobalCommonGenerator::GetEditCacheCommand() const
{
  // If generating for an extra IDE, the edit_cache target cannot
  // launch a terminal-interactive tool, so always use cmake-gui.
  if (!this->GetExtraGeneratorName().empty()) {
    return cmSystemTools::GetCMakeGUICommand();
  }

  // Use an internal cache entry to track the latest dialog used
  // to edit the cache, and use that for the edit_cache target.
  cmake* cm = this->GetCMakeInstance();
  std::string editCacheCommand = cm->GetCMakeEditCommand();
  if (!cm->GetCacheDefinition("CMAKE_EDIT_COMMAND") ||
      !editCacheCommand.empty()) {
    if (this->SupportsDirectConsole() && editCacheCommand.empty()) {
      editCacheCommand = cmSystemTools::GetCMakeCursesCommand();
    }
    if (editCacheCommand.empty()) {
      editCacheCommand = cmSystemTools::GetCMakeGUICommand();
    }
    if (!editCacheCommand.empty()) {
      cm->AddCacheEntry("CMAKE_EDIT_COMMAND", editCacheCommand,
                        "Path to cache edit program executable.",
                        cmStateEnums::INTERNAL);
    }
  }
  cmValue edit_cmd = cm->GetCacheDefinition("CMAKE_EDIT_COMMAND");
  return edit_cmd ? *edit_cmd : std::string();
}

void cmGlobalCommonGenerator::RemoveUnknownClangTidyExportFixesFiles() const
{
  for (auto const& dir : this->ClangTidyExportFixesDirs) {
    cmsys::Glob g;
    g.SetRecurse(true);
    g.SetListDirs(false);
    g.FindFiles(cmStrCat(dir, "/*.yaml"));
    for (auto const& file : g.GetFiles()) {
      if (!this->ClangTidyExportFixesFiles.count(file) &&
          !std::any_of(this->ClangTidyExportFixesFiles.begin(),
                       this->ClangTidyExportFixesFiles.end(),
                       [&file](const std::string& knownFile) -> bool {
                         return cmSystemTools::SameFile(file, knownFile);
                       })) {
        cmSystemTools::RemoveFile(file);
      }
    }
  }
}
