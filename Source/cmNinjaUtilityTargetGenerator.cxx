/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmNinjaUtilityTargetGenerator.h"

#include <algorithm>
#include <array>
#include <iterator>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "cmCustomCommand.h"
#include "cmCustomCommandGenerator.h"
#include "cmGeneratedFileStream.h"
#include "cmGeneratorTarget.h"
#include "cmGlobalNinjaGenerator.h"
#include "cmLocalNinjaGenerator.h"
#include "cmNinjaTypes.h"
#include "cmOutputConverter.h"
#include "cmSourceFile.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmTarget.h"
#include "cmValue.h"

cmNinjaUtilityTargetGenerator::cmNinjaUtilityTargetGenerator(
  cmGeneratorTarget* target)
  : cmNinjaTargetGenerator(target)
{
}

cmNinjaUtilityTargetGenerator::~cmNinjaUtilityTargetGenerator() = default;

void cmNinjaUtilityTargetGenerator::Generate(const std::string& config)
{
  if (!this->GetGeneratorTarget()->Target->IsPerConfig()) {
    this->WriteUtilBuildStatements(config, config);
    return;
  }

  for (auto const& fileConfig : this->GetConfigNames()) {
    if (!this->GetGlobalGenerator()
           ->GetCrossConfigs(fileConfig)
           .count(config)) {
      continue;
    }
    if (fileConfig != config &&
        this->GetGeneratorTarget()->GetType() == cmStateEnums::GLOBAL_TARGET) {
      continue;
    }
    this->WriteUtilBuildStatements(config, fileConfig);
  }
}

void cmNinjaUtilityTargetGenerator::WriteUtilBuildStatements(
  std::string const& config, std::string const& fileConfig)
{
  cmGlobalNinjaGenerator* gg = this->GetGlobalGenerator();
  cmLocalNinjaGenerator* lg = this->GetLocalGenerator();
  cmGeneratorTarget* genTarget = this->GetGeneratorTarget();

  std::string configDir;
  if (genTarget->Target->IsPerConfig()) {
    configDir = gg->ConfigDirectory(fileConfig);
  }
  std::string utilCommandName =
    cmStrCat(lg->GetCurrentBinaryDirectory(), "/CMakeFiles", configDir, "/",
             this->GetTargetName(), ".util");
  utilCommandName = this->ConvertToNinjaPath(utilCommandName);

  cmNinjaBuild phonyBuild("phony");
  std::vector<std::string> commands;
  cmNinjaDeps deps;
  cmGlobalNinjaGenerator::CCOutputs util_outputs(gg);
  util_outputs.ExplicitOuts.emplace_back(utilCommandName);

  bool uses_terminal = false;
  {
    std::array<std::vector<cmCustomCommand> const*, 2> const cmdLists = {
      { &genTarget->GetPreBuildCommands(), &genTarget->GetPostBuildCommands() }
    };

    for (std::vector<cmCustomCommand> const* cmdList : cmdLists) {
      for (cmCustomCommand const& ci : *cmdList) {
        cmCustomCommandGenerator ccg(ci, fileConfig, lg);
        lg->AppendCustomCommandDeps(ccg, deps, fileConfig);
        lg->AppendCustomCommandLines(ccg, commands);
        util_outputs.Add(ccg.GetByproducts());
        if (ci.GetUsesTerminal()) {
          uses_terminal = true;
        }
      }
    }
  }

  {
    std::vector<cmSourceFile*> sources;
    genTarget->GetSourceFiles(sources, config);
    for (cmSourceFile const* source : sources) {
      if (cmCustomCommand const* cc = source->GetCustomCommand()) {
        cmCustomCommandGenerator ccg(*cc, config, lg);
        lg->AddCustomCommandTarget(cc, genTarget);

        // Depend on all custom command outputs.
        const std::vector<std::string>& ccOutputs = ccg.GetOutputs();
        const std::vector<std::string>& ccByproducts = ccg.GetByproducts();
        std::transform(ccOutputs.begin(), ccOutputs.end(),
                       std::back_inserter(deps), this->MapToNinjaPath());
        std::transform(ccByproducts.begin(), ccByproducts.end(),
                       std::back_inserter(deps), this->MapToNinjaPath());
      }
    }
  }

  std::string outputConfig;
  if (genTarget->Target->IsPerConfig()) {
    outputConfig = config;
  }
  lg->AppendTargetOutputs(genTarget, phonyBuild.Outputs, outputConfig);
  if (genTarget->Target->GetType() != cmStateEnums::GLOBAL_TARGET) {
    lg->AppendTargetOutputs(genTarget, gg->GetByproductsForCleanTarget(),
                            config);
    std::copy(util_outputs.ExplicitOuts.begin(),
              util_outputs.ExplicitOuts.end(),
              std::back_inserter(gg->GetByproductsForCleanTarget()));
  }
  lg->AppendTargetDepends(genTarget, deps, config, fileConfig,
                          DependOnTargetArtifact);

  if (commands.empty()) {
    phonyBuild.Comment = "Utility command for " + this->GetTargetName();
    phonyBuild.ExplicitDeps = std::move(deps);
    if (genTarget->GetType() != cmStateEnums::GLOBAL_TARGET) {
      gg->WriteBuild(this->GetImplFileStream(fileConfig), phonyBuild);
    } else {
      gg->WriteBuild(this->GetCommonFileStream(), phonyBuild);
    }
  } else {
    std::string command = lg->BuildCommandLine(
      commands, config, fileConfig, "utility", this->GeneratorTarget);
    std::string desc;
    cmValue echoStr = genTarget->GetProperty("EchoString");
    if (echoStr) {
      desc = *echoStr;
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
    command = gg->ExpandCFGIntDir(command, config);

    if (command.find('$') != std::string::npos) {
      return;
    }

    std::string ccConfig;
    if (genTarget->Target->IsPerConfig() &&
        genTarget->GetType() != cmStateEnums::GLOBAL_TARGET) {
      ccConfig = config;
    }
    if (config == fileConfig ||
        gg->GetPerConfigUtilityTargets().count(genTarget->GetName())) {
      gg->WriteCustomCommandBuild(
        command, desc, "Utility command for " + this->GetTargetName(),
        /*depfile*/ "", /*job_pool*/ "", uses_terminal,
        /*restat*/ true, ccConfig, std::move(util_outputs), std::move(deps));
    }

    phonyBuild.ExplicitDeps.push_back(utilCommandName);
    if (genTarget->GetType() != cmStateEnums::GLOBAL_TARGET) {
      gg->WriteBuild(this->GetImplFileStream(fileConfig), phonyBuild);
    } else {
      gg->WriteBuild(this->GetCommonFileStream(), phonyBuild);
    }
  }

  // Find ADDITIONAL_CLEAN_FILES
  this->AdditionalCleanFiles(config);

  // Add an alias for the logical target name regardless of what directory
  // contains it.  Skip this for GLOBAL_TARGET because they are meant to
  // be per-directory and have one at the top-level anyway.
  if (genTarget->GetType() != cmStateEnums::GLOBAL_TARGET) {
    gg->AddTargetAlias(this->GetTargetName(), genTarget, config);
  }
}
