/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCommands.h"

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
#include "cmConfigureFileCommand.h"
#include "cmContinueCommand.h"
#include "cmCreateTestSourceList.h"
#include "cmDefinePropertyCommand.h"
#include "cmEnableLanguageCommand.h"
#include "cmEnableTestingCommand.h"
#include "cmExecProgramCommand.h"
#include "cmExecuteProcessCommand.h"
#include "cmFileCommand.h"
#include "cmFindFileCommand.h"
#include "cmFindLibraryCommand.h"
#include "cmFindPackageCommand.h"
#include "cmFindPathCommand.h"
#include "cmFindProgramCommand.h"
#include "cmForEachCommand.h"
#include "cmFunctionCommand.h"
#include "cmGetCMakePropertyCommand.h"
#include "cmGetDirectoryPropertyCommand.h"
#include "cmGetFilenameComponentCommand.h"
#include "cmGetPropertyCommand.h"
#include "cmGetSourceFilePropertyCommand.h"
#include "cmGetTargetPropertyCommand.h"
#include "cmGetTestPropertyCommand.h"
#include "cmIfCommand.h"
#include "cmIncludeCommand.h"
#include "cmIncludeDirectoryCommand.h"
#include "cmIncludeRegularExpressionCommand.h"
#include "cmInstallCommand.h"
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
#include "cmParseArgumentsCommand.h"
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
#include "cmTryCompileCommand.h"
#include "cmTryRunCommand.h"
#include "cmUnexpectedCommand.h"
#include "cmUnsetCommand.h"
#include "cmWhileCommand.h"

#if defined(CMAKE_BUILD_WITH_CMAKE)
#include "cmAddCompileOptionsCommand.h"
#include "cmAuxSourceDirectoryCommand.h"
#include "cmBuildNameCommand.h"
#include "cmCMakeHostSystemInformationCommand.h"
#include "cmExportCommand.h"
#include "cmExportLibraryDependenciesCommand.h"
#include "cmFLTKWrapUICommand.h"
#include "cmIncludeExternalMSProjectCommand.h"
#include "cmInstallProgramsCommand.h"
#include "cmLinkLibrariesCommand.h"
#include "cmLoadCacheCommand.h"
#include "cmLoadCommandCommand.h"
#include "cmOutputRequiredFilesCommand.h"
#include "cmQTWrapCPPCommand.h"
#include "cmQTWrapUICommand.h"
#include "cmRemoveCommand.h"
#include "cmRemoveDefinitionsCommand.h"
#include "cmSourceGroupCommand.h"
#include "cmSubdirDependsCommand.h"
#include "cmTargetCompileDefinitionsCommand.h"
#include "cmTargetCompileFeaturesCommand.h"
#include "cmTargetCompileOptionsCommand.h"
#include "cmTargetIncludeDirectoriesCommand.h"
#include "cmTargetSourcesCommand.h"
#include "cmUseMangledMesaCommand.h"
#include "cmUtilitySourceCommand.h"
#include "cmVariableRequiresCommand.h"
#include "cmVariableWatchCommand.h"
#include "cmWriteFileCommand.h"
#endif

std::vector<cmCommand*> GetPredefinedCommands()
{
  std::vector<cmCommand*> commands;

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
  commands.push_back(new cmEnableLanguageCommand);
  commands.push_back(new cmEnableTestingCommand);
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
  commands.push_back(new cmGetCMakePropertyCommand);
  commands.push_back(new cmGetDirectoryPropertyCommand);
  commands.push_back(new cmGetFilenameComponentCommand);
  commands.push_back(new cmGetPropertyCommand);
  commands.push_back(new cmGetSourceFilePropertyCommand);
  commands.push_back(new cmGetTargetPropertyCommand);
  commands.push_back(new cmGetTestPropertyCommand);
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
  commands.push_back(new cmParseArgumentsCommand);
  commands.push_back(new cmProjectCommand);
  commands.push_back(new cmReturnCommand);
  commands.push_back(new cmSeparateArgumentsCommand);
  commands.push_back(new cmSetCommand);
  commands.push_back(new cmSetDirectoryPropertiesCommand);
  commands.push_back(new cmSetPropertyCommand);
  commands.push_back(new cmSetSourceFilesPropertiesCommand);
  commands.push_back(new cmSetTargetPropertiesCommand);
  commands.push_back(new cmSetTestsPropertiesCommand);
  commands.push_back(new cmSiteNameCommand);
  commands.push_back(new cmStringCommand);
  commands.push_back(new cmSubdirCommand);
  commands.push_back(new cmTargetLinkLibrariesCommand);
  commands.push_back(new cmTryCompileCommand);
  commands.push_back(new cmTryRunCommand);
  commands.push_back(new cmUnsetCommand);
  commands.push_back(new cmWhileCommand);

  commands.push_back(new cmUnexpectedCommand(
    "else", "An ELSE command was found outside of a proper "
            "IF ENDIF structure. Or its arguments did not match "
            "the opening IF command."));
  commands.push_back(new cmUnexpectedCommand(
    "elseif", "An ELSEIF command was found outside of a proper "
              "IF ENDIF structure."));
  commands.push_back(new cmUnexpectedCommand(
    "endforeach", "An ENDFOREACH command was found outside of a proper "
                  "FOREACH ENDFOREACH structure. Or its arguments did "
                  "not match the opening FOREACH command."));
  commands.push_back(new cmUnexpectedCommand(
    "endfunction", "An ENDFUNCTION command was found outside of a proper "
                   "FUNCTION ENDFUNCTION structure. Or its arguments did not "
                   "match the opening FUNCTION command."));
  commands.push_back(new cmUnexpectedCommand(
    "endif", "An ENDIF command was found outside of a proper "
             "IF ENDIF structure. Or its arguments did not match "
             "the opening IF command."));
  commands.push_back(new cmUnexpectedCommand(
    "endmacro", "An ENDMACRO command was found outside of a proper "
                "MACRO ENDMACRO structure. Or its arguments did not "
                "match the opening MACRO command."));
  commands.push_back(new cmUnexpectedCommand(
    "endwhile", "An ENDWHILE command was found outside of a proper "
                "WHILE ENDWHILE structure. Or its arguments did not "
                "match the opening WHILE command."));

#if defined(CMAKE_BUILD_WITH_CMAKE)
  commands.push_back(new cmAddCompileOptionsCommand);
  commands.push_back(new cmAuxSourceDirectoryCommand);
  commands.push_back(new cmBuildNameCommand);
  commands.push_back(new cmCMakeHostSystemInformationCommand);
  commands.push_back(new cmExportCommand);
  commands.push_back(new cmExportLibraryDependenciesCommand);
  commands.push_back(new cmFLTKWrapUICommand);
  commands.push_back(new cmIncludeExternalMSProjectCommand);
  commands.push_back(new cmInstallProgramsCommand);
  commands.push_back(new cmLinkLibrariesCommand);
  commands.push_back(new cmLoadCacheCommand);
  commands.push_back(new cmLoadCommandCommand);
  commands.push_back(new cmOutputRequiredFilesCommand);
  commands.push_back(new cmQTWrapCPPCommand);
  commands.push_back(new cmQTWrapUICommand);
  commands.push_back(new cmRemoveCommand);
  commands.push_back(new cmRemoveDefinitionsCommand);
  commands.push_back(new cmSourceGroupCommand);
  commands.push_back(new cmSubdirDependsCommand);
  commands.push_back(new cmTargetCompileDefinitionsCommand);
  commands.push_back(new cmTargetCompileFeaturesCommand);
  commands.push_back(new cmTargetCompileOptionsCommand);
  commands.push_back(new cmTargetIncludeDirectoriesCommand);
  commands.push_back(new cmTargetSourcesCommand);
  commands.push_back(new cmUseMangledMesaCommand);
  commands.push_back(new cmUtilitySourceCommand);
  commands.push_back(new cmVariableRequiresCommand);
  commands.push_back(new cmVariableWatchCommand);
  commands.push_back(new cmWriteFileCommand);
#endif

  return commands;
}
