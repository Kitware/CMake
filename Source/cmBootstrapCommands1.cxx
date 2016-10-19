/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
// This file is used to compile all the commands
// that CMake knows about at compile time.
// This is sort of a boot strapping approach since you would
// like to have CMake to build CMake.
#include "cmAddCustomCommandCommand.h"
#include "cmAddCustomTargetCommand.h"
#include "cmAddDefinitionsCommand.h"
#include "cmAddDependenciesCommand.h"
#include "cmAddExecutableCommand.h"
#include "cmAddLibraryCommand.h"
#include "cmAddSubDirectoryCommand.h"
#include "cmAddTestCommand.h"
#include "cmBreakCommand.h"
#include "cmBuildCommand.h"
#include "cmCMakeMinimumRequired.h"
#include "cmCMakePolicyCommand.h"
#include "cmCommandArgumentsHelper.h"
#include "cmCommands.h"
#include "cmConfigureFileCommand.h"
#include "cmContinueCommand.h"
#include "cmCoreTryCompile.h"
#include "cmCreateTestSourceList.h"
#include "cmDefinePropertyCommand.h"
#include "cmElseCommand.h"
#include "cmEnableLanguageCommand.h"
#include "cmEnableTestingCommand.h"
#include "cmEndForEachCommand.h"
#include "cmEndFunctionCommand.h"
#include "cmEndIfCommand.h"
#include "cmEndMacroCommand.h"
#include "cmEndWhileCommand.h"
#include "cmExecProgramCommand.h"
#include "cmExecuteProcessCommand.h"
#include "cmFileCommand.h"
#include "cmFindBase.h"
#include "cmFindCommon.h"
#include "cmFindFileCommand.h"
#include "cmFindLibraryCommand.h"
#include "cmFindPackageCommand.h"
#include "cmFindPathCommand.h"
#include "cmFindProgramCommand.h"
#include "cmForEachCommand.h"
#include "cmFunctionCommand.h"
#include "cmParseArgumentsCommand.h"
#include "cmPathLabel.h"
#include "cmSearchPath.h"

void GetBootstrapCommands1(std::vector<cmCommand*>& commands)
{
  commands.push_back(new cmAddCustomCommandCommand);
  commands.push_back(new cmAddCustomTargetCommand);
  commands.push_back(new cmAddDefinitionsCommand);
  commands.push_back(new cmAddDependenciesCommand);
  commands.push_back(new cmAddExecutableCommand);
  commands.push_back(new cmAddLibraryCommand);
  commands.push_back(new cmAddSubDirectoryCommand);
  commands.push_back(new cmAddTestCommand);
  commands.push_back(new cmBreakCommand);
  commands.push_back(new cmBuildCommand);
  commands.push_back(new cmCMakeMinimumRequired);
  commands.push_back(new cmCMakePolicyCommand);
  commands.push_back(new cmConfigureFileCommand);
  commands.push_back(new cmContinueCommand);
  commands.push_back(new cmCreateTestSourceList);
  commands.push_back(new cmDefinePropertyCommand);
  commands.push_back(new cmElseCommand);
  commands.push_back(new cmEnableLanguageCommand);
  commands.push_back(new cmEnableTestingCommand);
  commands.push_back(new cmEndForEachCommand);
  commands.push_back(new cmEndFunctionCommand);
  commands.push_back(new cmEndIfCommand);
  commands.push_back(new cmEndMacroCommand);
  commands.push_back(new cmEndWhileCommand);
  commands.push_back(new cmExecProgramCommand);
  commands.push_back(new cmExecuteProcessCommand);
  commands.push_back(new cmFileCommand);
  commands.push_back(new cmFindFileCommand);
  commands.push_back(new cmFindLibraryCommand);
  commands.push_back(new cmFindPackageCommand);
  commands.push_back(new cmFindPathCommand);
  commands.push_back(new cmFindProgramCommand);
  commands.push_back(new cmForEachCommand);
  commands.push_back(new cmFunctionCommand);
  commands.push_back(new cmParseArgumentsCommand);
}
