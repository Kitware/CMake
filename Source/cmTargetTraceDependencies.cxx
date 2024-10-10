/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmTargetTraceDependencies.h"

#include <sstream>
#include <utility>

#include <cmext/algorithm>

#include "cmCustomCommandGenerator.h"
#include "cmGeneratorTarget.h"
#include "cmGlobalGenerator.h"
#include "cmList.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmSourceFile.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmTarget.h"
#include "cmValue.h"

cmTargetTraceDependencies::cmTargetTraceDependencies(cmGeneratorTarget* target)
  : GeneratorTarget(target)
{
  // Convenience.
  this->Makefile = target->Target->GetMakefile();
  this->LocalGenerator = target->GetLocalGenerator();
  this->GlobalGenerator = this->LocalGenerator->GetGlobalGenerator();
  this->CurrentEntry = nullptr;

  // Queue all the source files already specified for the target.
  std::set<cmSourceFile*> emitted;
  std::vector<std::string> const& configs =
    this->Makefile->GetGeneratorConfigs(cmMakefile::IncludeEmptyConfig);
  for (std::string const& c : configs) {
    std::vector<cmSourceFile*> sources;
    this->GeneratorTarget->GetSourceFiles(sources, c);
    for (cmSourceFile* sf : sources) {
      const std::set<cmGeneratorTarget const*> tgts =
        this->GlobalGenerator->GetFilenameTargetDepends(sf);
      if (cm::contains(tgts, this->GeneratorTarget)) {
        std::ostringstream e;
        e << "Evaluation output file\n  \"" << sf->ResolveFullPath()
          << "\"\ndepends on the sources of a target it is used in.  This "
             "is a dependency loop and is not allowed.";
        this->GeneratorTarget->LocalGenerator->IssueMessage(
          MessageType::FATAL_ERROR, e.str());
        return;
      }
      if (emitted.insert(sf).second && this->SourcesQueued.insert(sf).second) {
        this->SourceQueue.push(sf);
      }
    }
  }

  // Queue pre-build, pre-link, and post-build rule dependencies.
  this->CheckCustomCommands(this->GeneratorTarget->GetPreBuildCommands());
  this->CheckCustomCommands(this->GeneratorTarget->GetPreLinkCommands());
  this->CheckCustomCommands(this->GeneratorTarget->GetPostBuildCommands());
}

void cmTargetTraceDependencies::Trace()
{
  // Process one dependency at a time until the queue is empty.
  while (!this->SourceQueue.empty()) {
    // Get the next source from the queue.
    cmSourceFile* sf = this->SourceQueue.front();
    this->SourceQueue.pop();
    this->CurrentEntry = &this->GeneratorTarget->SourceDepends[sf];

    // Queue dependencies added explicitly by the user.
    if (cmValue additionalDeps = sf->GetProperty("OBJECT_DEPENDS")) {
      cmList objDeps{ *additionalDeps };
      for (auto& objDep : objDeps) {
        if (cmSystemTools::FileIsFullPath(objDep)) {
          objDep = cmSystemTools::CollapseFullPath(objDep);
        }
      }
      this->FollowNames(objDeps);
    }

    // Queue the source needed to generate this file, if any.
    this->FollowName(sf->ResolveFullPath());

    // Queue dependencies added programmatically by commands.
    this->FollowNames(sf->GetDepends());

    // Queue custom command dependencies.
    if (cmCustomCommand const* cc = sf->GetCustomCommand()) {
      this->CheckCustomCommand(*cc);
    }
  }
  this->CurrentEntry = nullptr;

  this->GeneratorTarget->AddTracedSources(this->NewSources);
}

void cmTargetTraceDependencies::QueueSource(cmSourceFile* sf)
{
  if (this->SourcesQueued.insert(sf).second) {
    this->SourceQueue.push(sf);

    // Make sure this file is in the target at the end.
    this->NewSources.push_back(sf->ResolveFullPath());
  }
}

void cmTargetTraceDependencies::FollowName(std::string const& name)
{
  // Use lower bound with key comparison to not repeat the search for the
  // insert position if the name could not be found (which is the common case).
  auto i = this->NameMap.lower_bound(name);
  if (i == this->NameMap.end() || i->first != name) {
    // Check if we know how to generate this file.
    cmSourcesWithOutput sources =
      this->LocalGenerator->GetSourcesWithOutput(name);
    // If we failed to find a target or source and we have a relative path, it
    // might be a valid source if made relative to the current binary
    // directory.
    if (!sources.Target && !sources.Source &&
        !cmSystemTools::FileIsFullPath(name)) {
      auto fullname =
        cmStrCat(this->Makefile->GetCurrentBinaryDirectory(), '/', name);
      fullname = cmSystemTools::CollapseFullPath(
        fullname, this->Makefile->GetHomeOutputDirectory());
      sources = this->LocalGenerator->GetSourcesWithOutput(fullname);
    }
    i = this->NameMap.emplace_hint(i, name, sources);
  }
  if (cmTarget* t = i->second.Target) {
    // The name is a byproduct of a utility target or a PRE_BUILD, PRE_LINK, or
    // POST_BUILD command.
    this->GeneratorTarget->Target->AddUtility(t->GetName(), false);
  }
  if (cmSourceFile* sf = i->second.Source) {
    // For now only follow the dependency if the source file is not a
    // byproduct.  Semantics of byproducts in a non-Ninja context will have to
    // be defined first.
    if (!i->second.SourceIsByproduct) {
      // Record the dependency we just followed.
      if (this->CurrentEntry) {
        this->CurrentEntry->Depends.push_back(sf);
      }
      this->QueueSource(sf);
    }
  }
}

void cmTargetTraceDependencies::FollowNames(
  std::vector<std::string> const& names)
{
  for (std::string const& name : names) {
    this->FollowName(name);
  }
}

bool cmTargetTraceDependencies::IsUtility(std::string const& dep)
{
  // Dependencies on targets (utilities) are supposed to be named by
  // just the target name.  However for compatibility we support
  // naming the output file generated by the target (assuming there is
  // no output-name property which old code would not have set).  In
  // that case the target name will be the file basename of the
  // dependency.
  std::string util = cmSystemTools::GetFilenameName(dep);
  if (cmSystemTools::GetFilenameLastExtension(util) == ".exe") {
    util = cmSystemTools::GetFilenameWithoutLastExtension(util);
  }

  // Check for a target with this name.
  if (cmGeneratorTarget* t =
        this->GeneratorTarget->GetLocalGenerator()->FindGeneratorTargetToUse(
          util)) {
    // If we find the target and the dep was given as a full path,
    // then make sure it was not a full path to something else, and
    // the fact that the name matched a target was just a coincidence.
    if (cmSystemTools::FileIsFullPath(dep)) {
      if (t->GetType() >= cmStateEnums::EXECUTABLE &&
          t->GetType() <= cmStateEnums::MODULE_LIBRARY) {
        // This is really only for compatibility so we do not need to
        // worry about configuration names and output names.
        std::string tLocation = t->GetLocationForBuild();
        tLocation = cmSystemTools::GetFilenamePath(tLocation);
        std::string depLocation = cmSystemTools::GetFilenamePath(dep);
        depLocation = cmSystemTools::CollapseFullPath(depLocation);
        tLocation = cmSystemTools::CollapseFullPath(tLocation);
        if (depLocation == tLocation) {
          this->GeneratorTarget->Target->AddUtility(util, false);
          return true;
        }
      }
    } else {
      // The original name of the dependency was not a full path.  It
      // must name a target, so add the target-level dependency.
      this->GeneratorTarget->Target->AddUtility(util, true);
      return true;
    }
  }

  // The dependency does not name a target built in this project.
  return false;
}

void cmTargetTraceDependencies::CheckCustomCommand(cmCustomCommand const& cc)
{
  // Collect dependencies referenced by all configurations.
  std::set<std::string> depends;
  for (std::string const& config :
       this->Makefile->GetGeneratorConfigs(cmMakefile::IncludeEmptyConfig)) {
    for (cmCustomCommandGenerator const& ccg :
         this->LocalGenerator->MakeCustomCommandGenerators(cc, config)) {
      // Collect target-level dependencies referenced in command lines.
      for (auto const& util : ccg.GetUtilities()) {
        this->GeneratorTarget->Target->AddUtility(util);
      }

      // Collect file-level dependencies referenced in DEPENDS.
      depends.insert(ccg.GetDepends().begin(), ccg.GetDepends().end());
    }
  }

  // Queue file-level dependencies.
  for (std::string const& dep : depends) {
    if (!this->IsUtility(dep)) {
      // The dependency does not name a target and may be a file we
      // know how to generate.  Queue it.
      this->FollowName(dep);
    }
  }
}

void cmTargetTraceDependencies::CheckCustomCommands(
  const std::vector<cmCustomCommand>& commands)
{
  for (cmCustomCommand const& command : commands) {
    this->CheckCustomCommand(command);
  }
}
