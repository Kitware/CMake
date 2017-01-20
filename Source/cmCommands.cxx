/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCommands.h"
#include "cmState.h"

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
#include "cmDisallowedCommand.h"
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

void GetScriptingCommands(cmState* state)
{
  state->AddCommand(new cmBreakCommand);
  state->AddCommand(new cmCMakeMinimumRequired);
  state->AddCommand(new cmCMakePolicyCommand);
  state->AddCommand(new cmConfigureFileCommand);
  state->AddCommand(new cmContinueCommand);
  state->AddCommand(new cmExecProgramCommand);
  state->AddCommand(new cmExecuteProcessCommand);
  state->AddCommand(new cmFileCommand);
  state->AddCommand(new cmFindFileCommand);
  state->AddCommand(new cmFindLibraryCommand);
  state->AddCommand(new cmFindPackageCommand);
  state->AddCommand(new cmFindPathCommand);
  state->AddCommand(new cmFindProgramCommand);
  state->AddCommand(new cmForEachCommand);
  state->AddCommand(new cmFunctionCommand);
  state->AddCommand(new cmGetCMakePropertyCommand);
  state->AddCommand(new cmGetDirectoryPropertyCommand);
  state->AddCommand(new cmGetFilenameComponentCommand);
  state->AddCommand(new cmGetPropertyCommand);
  state->AddCommand(new cmIfCommand);
  state->AddCommand(new cmIncludeCommand);
  state->AddCommand(new cmListCommand);
  state->AddCommand(new cmMacroCommand);
  state->AddCommand(new cmMakeDirectoryCommand);
  state->AddCommand(new cmMarkAsAdvancedCommand);
  state->AddCommand(new cmMathCommand);
  state->AddCommand(new cmMessageCommand);
  state->AddCommand(new cmOptionCommand);
  state->AddCommand(new cmParseArgumentsCommand);
  state->AddCommand(new cmReturnCommand);
  state->AddCommand(new cmSeparateArgumentsCommand);
  state->AddCommand(new cmSetCommand);
  state->AddCommand(new cmSetDirectoryPropertiesCommand);
  state->AddCommand(new cmSetPropertyCommand);
  state->AddCommand(new cmSiteNameCommand);
  state->AddCommand(new cmStringCommand);
  state->AddCommand(new cmUnsetCommand);
  state->AddCommand(new cmWhileCommand);

  state->AddCommand(new cmUnexpectedCommand(
    "else", "An ELSE command was found outside of a proper "
            "IF ENDIF structure. Or its arguments did not match "
            "the opening IF command."));
  state->AddCommand(new cmUnexpectedCommand(
    "elseif", "An ELSEIF command was found outside of a proper "
              "IF ENDIF structure."));
  state->AddCommand(new cmUnexpectedCommand(
    "endforeach", "An ENDFOREACH command was found outside of a proper "
                  "FOREACH ENDFOREACH structure. Or its arguments did "
                  "not match the opening FOREACH command."));
  state->AddCommand(new cmUnexpectedCommand(
    "endfunction", "An ENDFUNCTION command was found outside of a proper "
                   "FUNCTION ENDFUNCTION structure. Or its arguments did not "
                   "match the opening FUNCTION command."));
  state->AddCommand(new cmUnexpectedCommand(
    "endif", "An ENDIF command was found outside of a proper "
             "IF ENDIF structure. Or its arguments did not match "
             "the opening IF command."));
  state->AddCommand(new cmUnexpectedCommand(
    "endmacro", "An ENDMACRO command was found outside of a proper "
                "MACRO ENDMACRO structure. Or its arguments did not "
                "match the opening MACRO command."));
  state->AddCommand(new cmUnexpectedCommand(
    "endwhile", "An ENDWHILE command was found outside of a proper "
                "WHILE ENDWHILE structure. Or its arguments did not "
                "match the opening WHILE command."));

#if defined(CMAKE_BUILD_WITH_CMAKE)
  state->AddCommand(new cmCMakeHostSystemInformationCommand);
  state->AddCommand(new cmRemoveCommand);
  state->AddCommand(new cmVariableWatchCommand);
  state->AddCommand(new cmWriteFileCommand);

  state->AddCommand(new cmDisallowedCommand(
    new cmBuildNameCommand, cmPolicies::CMP0036,
    "The build_name command should not be called; see CMP0036."));
  state->AddCommand(new cmDisallowedCommand(
    new cmUseMangledMesaCommand, cmPolicies::CMP0030,
    "The use_mangled_mesa command should not be called; see CMP0030."));

#endif
}

void GetProjectCommands(cmState* state)
{
  state->AddCommand(new cmAddCustomCommandCommand);
  state->AddCommand(new cmAddCustomTargetCommand);
  state->AddCommand(new cmAddDefinitionsCommand);
  state->AddCommand(new cmAddDependenciesCommand);
  state->AddCommand(new cmAddExecutableCommand);
  state->AddCommand(new cmAddLibraryCommand);
  state->AddCommand(new cmAddSubDirectoryCommand);
  state->AddCommand(new cmAddTestCommand);
  state->AddCommand(new cmBuildCommand);
  state->AddCommand(new cmCreateTestSourceList);
  state->AddCommand(new cmDefinePropertyCommand);
  state->AddCommand(new cmEnableLanguageCommand);
  state->AddCommand(new cmEnableTestingCommand);
  state->AddCommand(new cmGetSourceFilePropertyCommand);
  state->AddCommand(new cmGetTargetPropertyCommand);
  state->AddCommand(new cmGetTestPropertyCommand);
  state->AddCommand(new cmIncludeDirectoryCommand);
  state->AddCommand(new cmIncludeRegularExpressionCommand);
  state->AddCommand(new cmInstallCommand);
  state->AddCommand(new cmInstallFilesCommand);
  state->AddCommand(new cmInstallTargetsCommand);
  state->AddCommand(new cmLinkDirectoriesCommand);
  state->AddCommand(new cmProjectCommand);
  state->AddCommand(new cmSetSourceFilesPropertiesCommand);
  state->AddCommand(new cmSetTargetPropertiesCommand);
  state->AddCommand(new cmSetTestsPropertiesCommand);
  state->AddCommand(new cmSubdirCommand);
  state->AddCommand(new cmTargetLinkLibrariesCommand);
  state->AddCommand(new cmTryCompileCommand);
  state->AddCommand(new cmTryRunCommand);

#if defined(CMAKE_BUILD_WITH_CMAKE)
  state->AddCommand(new cmAddCompileOptionsCommand);
  state->AddCommand(new cmAuxSourceDirectoryCommand);
  state->AddCommand(new cmExportCommand);
  state->AddCommand(new cmFLTKWrapUICommand);
  state->AddCommand(new cmIncludeExternalMSProjectCommand);
  state->AddCommand(new cmInstallProgramsCommand);
  state->AddCommand(new cmLinkLibrariesCommand);
  state->AddCommand(new cmLoadCacheCommand);
  state->AddCommand(new cmQTWrapCPPCommand);
  state->AddCommand(new cmQTWrapUICommand);
  state->AddCommand(new cmRemoveDefinitionsCommand);
  state->AddCommand(new cmSourceGroupCommand);
  state->AddCommand(new cmTargetCompileDefinitionsCommand);
  state->AddCommand(new cmTargetCompileFeaturesCommand);
  state->AddCommand(new cmTargetCompileOptionsCommand);
  state->AddCommand(new cmTargetIncludeDirectoriesCommand);
  state->AddCommand(new cmTargetSourcesCommand);

  state->AddCommand(new cmDisallowedCommand(
    new cmExportLibraryDependenciesCommand, cmPolicies::CMP0033,
    "The export_library_dependencies command should not be called; "
    "see CMP0033."));
  state->AddCommand(new cmDisallowedCommand(
    new cmLoadCommandCommand, cmPolicies::CMP0031,
    "The load_command command should not be called; see CMP0031."));
  state->AddCommand(new cmDisallowedCommand(
    new cmOutputRequiredFilesCommand, cmPolicies::CMP0032,
    "The output_required_files command should not be called; "
    "see CMP0032."));
  state->AddCommand(new cmDisallowedCommand(
    new cmSubdirDependsCommand, cmPolicies::CMP0029,
    "The subdir_depends command should not be called; see CMP0029."));
  state->AddCommand(new cmDisallowedCommand(
    new cmUtilitySourceCommand, cmPolicies::CMP0034,
    "The utility_source command should not be called; see CMP0034."));
  state->AddCommand(new cmDisallowedCommand(
    new cmVariableRequiresCommand, cmPolicies::CMP0035,
    "The variable_requires command should not be called; see CMP0035."));
#endif
}
