/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
// This file is used to compile all the commands
// that CMake knows about at compile time.
// This is sort of a boot strapping approach since you would
// like to have CMake to build CMake.
#include "cmCommands.h"
#include "cmConditionEvaluator.h"
#include "cmExpandedCommandArgument.h"
#include "cmGetCMakePropertyCommand.h"
#include "cmGetDirectoryPropertyCommand.h"
#include "cmGetFilenameComponentCommand.h"
#include "cmGetPropertyCommand.h"
#include "cmGetSourceFilePropertyCommand.h"
#include "cmGetTargetPropertyCommand.h"
#include "cmGetTestPropertyCommand.h"
#include "cmHexFileConverter.h"
#include "cmIfCommand.h"
#include "cmIncludeCommand.h"
#include "cmIncludeDirectoryCommand.h"
#include "cmIncludeRegularExpressionCommand.h"
#include "cmInstallCommand.h"
#include "cmInstallCommandArguments.h"
#include "cmInstallFilesCommand.h"
#include "cmInstallTargetsCommand.h"
#include "cmLinkDirectoriesCommand.h"
#include "cmListCommand.h"
#include "cmMacroCommand.h"
#include "cmMakeDirectoryCommand.h"
#include "cmMarkAsAdvancedCommand.h"
#include "cmMathCommand.h"
#include "cmMessageCommand.h"
#include "cmOptionCommand.h"
#include "cmProjectCommand.h"
#include "cmReturnCommand.h"
#include "cmSeparateArgumentsCommand.h"
#include "cmSetCommand.h"
#include "cmSetDirectoryPropertiesCommand.h"
#include "cmSetPropertyCommand.h"
#include "cmSetSourceFilesPropertiesCommand.h"
#include "cmSetTargetPropertiesCommand.h"
#include "cmSetTestsPropertiesCommand.h"
#include "cmSiteNameCommand.h"
#include "cmStringCommand.h"
#include "cmSubdirCommand.h"
#include "cmTargetLinkLibrariesCommand.h"
#include "cmTimestamp.h"
#include "cmTryCompileCommand.h"
#include "cmTryRunCommand.h"
#include "cmUnsetCommand.h"
#include "cmWhileCommand.h"

void GetBootstrapCommands2(std::vector<cmCommand*>& commands)
{
  commands.push_back(new cmGetCMakePropertyCommand);
  commands.push_back(new cmGetDirectoryPropertyCommand);
  commands.push_back(new cmGetFilenameComponentCommand);
  commands.push_back(new cmGetPropertyCommand);
  commands.push_back(new cmGetSourceFilePropertyCommand);
  commands.push_back(new cmGetTargetPropertyCommand);
  commands.push_back(new cmIfCommand);
  commands.push_back(new cmIncludeCommand);
  commands.push_back(new cmIncludeDirectoryCommand);
  commands.push_back(new cmIncludeRegularExpressionCommand);
  commands.push_back(new cmInstallCommand);
  commands.push_back(new cmInstallFilesCommand);
  commands.push_back(new cmInstallTargetsCommand);
  commands.push_back(new cmLinkDirectoriesCommand);
  commands.push_back(new cmListCommand);
  commands.push_back(new cmMacroCommand);
  commands.push_back(new cmMakeDirectoryCommand);
  commands.push_back(new cmMarkAsAdvancedCommand);
  commands.push_back(new cmMathCommand);
  commands.push_back(new cmMessageCommand);
  commands.push_back(new cmOptionCommand);
  commands.push_back(new cmProjectCommand);
  commands.push_back(new cmReturnCommand);
  commands.push_back(new cmSeparateArgumentsCommand);
  commands.push_back(new cmSetCommand);
  commands.push_back(new cmSetDirectoryPropertiesCommand);
  commands.push_back(new cmSetPropertyCommand);
  commands.push_back(new cmSetSourceFilesPropertiesCommand);
  commands.push_back(new cmSetTargetPropertiesCommand);
  commands.push_back(new cmGetTestPropertyCommand);
  commands.push_back(new cmSetTestsPropertiesCommand);
  commands.push_back(new cmSiteNameCommand);
  commands.push_back(new cmStringCommand);
  commands.push_back(new cmSubdirCommand);
  commands.push_back(new cmTargetLinkLibrariesCommand);
  commands.push_back(new cmTryCompileCommand);
  commands.push_back(new cmTryRunCommand);
  commands.push_back(new cmUnsetCommand);
  commands.push_back(new cmWhileCommand);
}
