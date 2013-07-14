/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2011 Peter Collingbourne <peter@pcc.me.uk>
  Copyright 2011 Nicolas Despres <nicolas.despres@gmail.com>

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmNinjaUtilityTargetGenerator.h"
#include "cmCustomCommand.h"
#include "cmGeneratedFileStream.h"
#include "cmGlobalNinjaGenerator.h"
#include "cmMakefile.h"
#include "cmSourceFile.h"
#include "cmTarget.h"

cmNinjaUtilityTargetGenerator::cmNinjaUtilityTargetGenerator(
    cmGeneratorTarget *target)
  : cmNinjaTargetGenerator(target->Target) {}

cmNinjaUtilityTargetGenerator::~cmNinjaUtilityTargetGenerator() {}

void cmNinjaUtilityTargetGenerator::Generate()
{
  std::vector<std::string> commands;
  cmNinjaDeps deps, outputs;

  const std::vector<cmCustomCommand> *cmdLists[2] = {
    &this->GetTarget()->GetPreBuildCommands(),
    &this->GetTarget()->GetPostBuildCommands()
  };

  for (unsigned i = 0; i != 2; ++i) {
    for (std::vector<cmCustomCommand>::const_iterator
         ci = cmdLists[i]->begin(); ci != cmdLists[i]->end(); ++ci) {
      this->GetLocalGenerator()->AppendCustomCommandDeps(&*ci, deps);
      this->GetLocalGenerator()->AppendCustomCommandLines(&*ci, commands);
    }
  }

  std::vector<cmSourceFile*> sources;
  this->GetTarget()->GetSourceFiles(sources);
  for(std::vector<cmSourceFile*>::const_iterator source = sources.begin();
      source != sources.end(); ++source)
    {
    if(cmCustomCommand* cc = (*source)->GetCustomCommand())
      {
      this->GetLocalGenerator()->AddCustomCommandTarget(cc, this->GetTarget());

      // Depend on all custom command outputs.
      const std::vector<std::string>& ccOutputs = cc->GetOutputs();
      std::transform(ccOutputs.begin(), ccOutputs.end(),
                     std::back_inserter(deps), MapToNinjaPath());
      }
    }

  this->GetLocalGenerator()->AppendTargetOutputs(this->GetTarget(), outputs);
  this->GetLocalGenerator()->AppendTargetDepends(this->GetTarget(), deps);

  if (commands.empty()) {
    this->GetGlobalGenerator()->WritePhonyBuild(this->GetBuildFileStream(),
                                                "Utility command for "
                                                  + this->GetTargetName(),
                                                outputs,
                                                deps);
  } else {
    std::string command =
      this->GetLocalGenerator()->BuildCommandLine(commands);
    const char *echoStr = this->GetTarget()->GetProperty("EchoString");
    std::string desc;
    if (echoStr)
      desc = echoStr;
    else
      desc = "Running utility command for " + this->GetTargetName();

    // TODO: fix problematic global targets.  For now, search and replace the
    // makefile vars.
    cmSystemTools::ReplaceString(
      command,
      "$(CMAKE_SOURCE_DIR)",
      this->GetLocalGenerator()->ConvertToOutputFormat(
        this->GetTarget()->GetMakefile()->GetHomeDirectory(),
        cmLocalGenerator::SHELL).c_str());
    cmSystemTools::ReplaceString(
      command,
      "$(CMAKE_BINARY_DIR)",
      this->GetLocalGenerator()->ConvertToOutputFormat(
        this->GetTarget()->GetMakefile()->GetHomeOutputDirectory(),
        cmLocalGenerator::SHELL).c_str());
    cmSystemTools::ReplaceString(command, "$(ARGS)", "");

    if (command.find('$') != std::string::npos)
      return;

    std::string utilCommandName = cmake::GetCMakeFilesDirectoryPostSlash();
    utilCommandName += this->GetTargetName() + ".util";

    this->GetGlobalGenerator()->WriteCustomCommandBuild(
      command,
      desc,
      "Utility command for " + this->GetTargetName(),
      cmNinjaDeps(1, utilCommandName),
      deps);

    this->GetGlobalGenerator()->WritePhonyBuild(this->GetBuildFileStream(),
                                                "",
                                                outputs,
                                                cmNinjaDeps(1, utilCommandName)
                                                );
  }

  this->GetGlobalGenerator()->AddTargetAlias(this->GetTargetName(),
                                             this->GetTarget());
}
