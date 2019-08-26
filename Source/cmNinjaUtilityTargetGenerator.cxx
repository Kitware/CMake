/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmNinjaUtilityTargetGenerator.h"

#include "cmCustomCommand.h"
#include "cmCustomCommandGenerator.h"
#include "cmGeneratedFileStream.h"
#include "cmGeneratorTarget.h"
#include "cmGlobalNinjaGenerator.h"
#include "cmLocalNinjaGenerator.h"
#include "cmMakefile.h"
#include "cmNinjaTypes.h"
#include "cmOutputConverter.h"
#include "cmSourceFile.h"
#include "cmStateTypes.h"
#include "cmSystemTools.h"

#include <algorithm>
#include <array>
#include <iterator>
#include <string>
#include <utility>
#include <vector>

cmNinjaUtilityTargetGenerator::cmNinjaUtilityTargetGenerator(
  cmGeneratorTarget* target)
  : cmNinjaTargetGenerator(target)
{
}

cmNinjaUtilityTargetGenerator::~cmNinjaUtilityTargetGenerator() = default;

void cmNinjaUtilityTargetGenerator::Generate()
{
  cmGlobalNinjaGenerator* gg = this->GetGlobalGenerator();
  cmLocalNinjaGenerator* lg = this->GetLocalGenerator();
  cmGeneratorTarget* genTarget = this->GetGeneratorTarget();

  std::string utilCommandName = lg->GetCurrentBinaryDirectory();
  utilCommandName += "/CMakeFiles";
  utilCommandName += "/";
  utilCommandName += this->GetTargetName() + ".util";
  utilCommandName = this->ConvertToNinjaPath(utilCommandName);

  cmNinjaBuild phonyBuild("phony");
  std::vector<std::string> commands;
  cmNinjaDeps deps, util_outputs(1, utilCommandName);

  bool uses_terminal = false;
  {
    std::array<std::vector<cmCustomCommand> const*, 2> const cmdLists = {
      { &genTarget->GetPreBuildCommands(), &genTarget->GetPostBuildCommands() }
    };

    for (std::vector<cmCustomCommand> const* cmdList : cmdLists) {
      for (cmCustomCommand const& ci : *cmdList) {
        cmCustomCommandGenerator ccg(ci, this->GetConfigName(), lg);
        lg->AppendCustomCommandDeps(ccg, deps);
        lg->AppendCustomCommandLines(ccg, commands);
        std::vector<std::string> const& ccByproducts = ccg.GetByproducts();
        std::transform(ccByproducts.begin(), ccByproducts.end(),
                       std::back_inserter(util_outputs), MapToNinjaPath());
        if (ci.GetUsesTerminal()) {
          uses_terminal = true;
        }
      }
    }
  }

  {
    std::string const& config =
      this->GetMakefile()->GetSafeDefinition("CMAKE_BUILD_TYPE");
    std::vector<cmSourceFile*> sources;
    genTarget->GetSourceFiles(sources, config);
    for (cmSourceFile const* source : sources) {
      if (cmCustomCommand const* cc = source->GetCustomCommand()) {
        cmCustomCommandGenerator ccg(*cc, this->GetConfigName(), lg);
        lg->AddCustomCommandTarget(cc, genTarget);

        // Depend on all custom command outputs.
        const std::vector<std::string>& ccOutputs = ccg.GetOutputs();
        const std::vector<std::string>& ccByproducts = ccg.GetByproducts();
        std::transform(ccOutputs.begin(), ccOutputs.end(),
                       std::back_inserter(deps), MapToNinjaPath());
        std::transform(ccByproducts.begin(), ccByproducts.end(),
                       std::back_inserter(deps), MapToNinjaPath());
      }
    }
  }

  lg->AppendTargetOutputs(genTarget, phonyBuild.Outputs);
  lg->AppendTargetDepends(genTarget, deps);

  if (commands.empty()) {
    phonyBuild.Comment = "Utility command for " + this->GetTargetName();
    phonyBuild.ExplicitDeps = std::move(deps);
    gg->WriteBuild(this->GetBuildFileStream(), phonyBuild);
  } else {
    std::string command =
      lg->BuildCommandLine(commands, "utility", this->GeneratorTarget);
    std::string desc;
    const char* echoStr = genTarget->GetProperty("EchoString");
    if (echoStr) {
      desc = echoStr;
    } else {
      desc = "Running utility command for " + this->GetTargetName();
    }

    // TODO: fix problematic global targets.  For now, search and replace the
    // makefile vars.
    cmSystemTools::ReplaceString(
      command, "$(CMAKE_SOURCE_DIR)",
      lg->ConvertToOutputFormat(lg->GetSourceDirectory(),
                                cmOutputConverter::SHELL));
    cmSystemTools::ReplaceString(
      command, "$(CMAKE_BINARY_DIR)",
      lg->ConvertToOutputFormat(lg->GetBinaryDirectory(),
                                cmOutputConverter::SHELL));
    cmSystemTools::ReplaceString(command, "$(ARGS)", "");

    if (command.find('$') != std::string::npos) {
      return;
    }

    for (std::string const& util_output : util_outputs) {
      gg->SeenCustomCommandOutput(util_output);
    }

    gg->WriteCustomCommandBuild(command, desc,
                                "Utility command for " + this->GetTargetName(),
                                /*depfile*/ "", /*job_pool*/ "", uses_terminal,
                                /*restat*/ true, util_outputs, deps);

    phonyBuild.ExplicitDeps.push_back(utilCommandName);
    gg->WriteBuild(this->GetBuildFileStream(), phonyBuild);
  }

  // Find ADDITIONAL_CLEAN_FILES
  this->AdditionalCleanFiles();

  // Add an alias for the logical target name regardless of what directory
  // contains it.  Skip this for GLOBAL_TARGET because they are meant to
  // be per-directory and have one at the top-level anyway.
  if (genTarget->GetType() != cmStateEnums::GLOBAL_TARGET) {
    gg->AddTargetAlias(this->GetTargetName(), genTarget);
  }
}
