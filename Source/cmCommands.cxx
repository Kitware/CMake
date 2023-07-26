/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#include "cmCommands.h"

#include <cm/memory>

#include "cmAddCompileDefinitionsCommand.h"
#include "cmAddCustomCommandCommand.h"
#include "cmAddCustomTargetCommand.h"
#include "cmAddDefinitionsCommand.h"
#include "cmAddDependenciesCommand.h"
#include "cmAddExecutableCommand.h"
#include "cmAddLibraryCommand.h"
#include "cmAddSubDirectoryCommand.h"
#include "cmAddTestCommand.h"
#include "cmBlockCommand.h"
#include "cmBreakCommand.h"
#include "cmBuildCommand.h"
#include "cmCMakeLanguageCommand.h"
#include "cmCMakeMinimumRequired.h"
#include "cmCMakePathCommand.h"
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
#include "cmIncludeGuardCommand.h"
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
#include "cmPolicies.h"
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
#include "cmState.h"
#include "cmStringCommand.h"
#include "cmSubdirCommand.h"
#include "cmTargetCompileDefinitionsCommand.h"
#include "cmTargetCompileFeaturesCommand.h"
#include "cmTargetCompileOptionsCommand.h"
#include "cmTargetIncludeDirectoriesCommand.h"
#include "cmTargetLinkLibrariesCommand.h"
#include "cmTargetLinkOptionsCommand.h"
#include "cmTargetPrecompileHeadersCommand.h"
#include "cmTargetSourcesCommand.h"
#include "cmTryCompileCommand.h"
#include "cmTryRunCommand.h"
#include "cmUnsetCommand.h"
#include "cmWhileCommand.h"

#if !defined(CMAKE_BOOTSTRAP)
#  include "cmAddCompileOptionsCommand.h"
#  include "cmAddLinkOptionsCommand.h"
#  include "cmAuxSourceDirectoryCommand.h"
#  include "cmBuildNameCommand.h"
#  include "cmCMakeHostSystemInformationCommand.h"
#  include "cmExportCommand.h"
#  include "cmExportLibraryDependenciesCommand.h"
#  include "cmFLTKWrapUICommand.h"
#  include "cmFileAPICommand.h"
#  include "cmIncludeExternalMSProjectCommand.h"
#  include "cmInstallProgramsCommand.h"
#  include "cmLinkLibrariesCommand.h"
#  include "cmLoadCacheCommand.h"
#  include "cmLoadCommandCommand.h"
#  include "cmOutputRequiredFilesCommand.h"
#  include "cmQTWrapCPPCommand.h"
#  include "cmQTWrapUICommand.h"
#  include "cmRemoveCommand.h"
#  include "cmRemoveDefinitionsCommand.h"
#  include "cmSourceGroupCommand.h"
#  include "cmSubdirDependsCommand.h"
#  include "cmTargetLinkDirectoriesCommand.h"
#  include "cmUseMangledMesaCommand.h"
#  include "cmUtilitySourceCommand.h"
#  include "cmVariableRequiresCommand.h"
#  include "cmVariableWatchCommand.h"
#  include "cmWriteFileCommand.h"
#endif

void GetScriptingCommands(cmState* state)
{
  state->AddFlowControlCommand("break", cmBreakCommand);
  state->AddFlowControlCommand("continue", cmContinueCommand);
  state->AddFlowControlCommand("foreach", cmForEachCommand);
  state->AddFlowControlCommand("function", cmFunctionCommand);
  state->AddFlowControlCommand("if", cmIfCommand);
  state->AddFlowControlCommand("macro", cmMacroCommand);
  state->AddFlowControlCommand("return", cmReturnCommand);
  state->AddFlowControlCommand("while", cmWhileCommand);
  state->AddFlowControlCommand("block", cmBlockCommand);

  state->AddBuiltinCommand("cmake_language", cmCMakeLanguageCommand);
  state->AddBuiltinCommand("cmake_minimum_required", cmCMakeMinimumRequired);
  state->AddBuiltinCommand("cmake_path", cmCMakePathCommand);
  state->AddBuiltinCommand("cmake_policy", cmCMakePolicyCommand);
  state->AddBuiltinCommand("configure_file", cmConfigureFileCommand);
  state->AddBuiltinCommand("execute_process", cmExecuteProcessCommand);
  state->AddBuiltinCommand("file", cmFileCommand);
  state->AddBuiltinCommand("find_file", cmFindFile);
  state->AddBuiltinCommand("find_library", cmFindLibrary);
  state->AddBuiltinCommand("find_package", cmFindPackage);
  state->AddBuiltinCommand("find_path", cmFindPath);
  state->AddBuiltinCommand("find_program", cmFindProgram);
  state->AddBuiltinCommand("get_cmake_property", cmGetCMakePropertyCommand);
  state->AddBuiltinCommand("get_directory_property",
                           cmGetDirectoryPropertyCommand);
  state->AddBuiltinCommand("get_filename_component",
                           cmGetFilenameComponentCommand);
  state->AddBuiltinCommand("get_property", cmGetPropertyCommand);
  state->AddBuiltinCommand("include", cmIncludeCommand);
  state->AddBuiltinCommand("include_guard", cmIncludeGuardCommand);
  state->AddBuiltinCommand("list", cmListCommand);
  state->AddBuiltinCommand("make_directory", cmMakeDirectoryCommand);
  state->AddBuiltinCommand("mark_as_advanced", cmMarkAsAdvancedCommand);
  state->AddBuiltinCommand("math", cmMathCommand);
  state->AddBuiltinCommand("message", cmMessageCommand);
  state->AddBuiltinCommand("option", cmOptionCommand);
  state->AddBuiltinCommand("cmake_parse_arguments", cmParseArgumentsCommand);
  state->AddBuiltinCommand("separate_arguments", cmSeparateArgumentsCommand);
  state->AddBuiltinCommand("set", cmSetCommand);
  state->AddBuiltinCommand("set_directory_properties",
                           cmSetDirectoryPropertiesCommand);
  state->AddBuiltinCommand("set_property", cmSetPropertyCommand);
  state->AddBuiltinCommand("site_name", cmSiteNameCommand);
  state->AddBuiltinCommand("string", cmStringCommand);
  state->AddBuiltinCommand("unset", cmUnsetCommand);

  state->AddUnexpectedFlowControlCommand(
    "else",
    "An ELSE command was found outside of a proper "
    "IF ENDIF structure. Or its arguments did not match "
    "the opening IF command.");
  state->AddUnexpectedFlowControlCommand(
    "elseif",
    "An ELSEIF command was found outside of a proper "
    "IF ENDIF structure.");
  state->AddUnexpectedFlowControlCommand(
    "endforeach",
    "An ENDFOREACH command was found outside of a proper "
    "FOREACH ENDFOREACH structure. Or its arguments did "
    "not match the opening FOREACH command.");
  state->AddUnexpectedFlowControlCommand(
    "endfunction",
    "An ENDFUNCTION command was found outside of a proper "
    "FUNCTION ENDFUNCTION structure. Or its arguments did not "
    "match the opening FUNCTION command.");
  state->AddUnexpectedFlowControlCommand(
    "endif",
    "An ENDIF command was found outside of a proper "
    "IF ENDIF structure. Or its arguments did not match "
    "the opening IF command.");
  state->AddUnexpectedFlowControlCommand(
    "endmacro",
    "An ENDMACRO command was found outside of a proper "
    "MACRO ENDMACRO structure. Or its arguments did not "
    "match the opening MACRO command.");
  state->AddUnexpectedFlowControlCommand(
    "endwhile",
    "An ENDWHILE command was found outside of a proper "
    "WHILE ENDWHILE structure. Or its arguments did not "
    "match the opening WHILE command.");
  state->AddUnexpectedFlowControlCommand(
    "endblock",
    "An ENDBLOCK command was found outside of a proper "
    "BLOCK ENDBLOCK structure.");

#if !defined(CMAKE_BOOTSTRAP)
  state->AddBuiltinCommand("cmake_host_system_information",
                           cmCMakeHostSystemInformationCommand);
  state->AddBuiltinCommand("load_cache", cmLoadCacheCommand);
  state->AddBuiltinCommand("remove", cmRemoveCommand);
  state->AddBuiltinCommand("variable_watch", cmVariableWatchCommand);
  state->AddBuiltinCommand("write_file", cmWriteFileCommand);

  state->AddDisallowedCommand(
    "build_name", cmBuildNameCommand, cmPolicies::CMP0036,
    "The build_name command should not be called; see CMP0036.");
  state->AddDisallowedCommand(
    "use_mangled_mesa", cmUseMangledMesaCommand, cmPolicies::CMP0030,
    "The use_mangled_mesa command should not be called; see CMP0030.");
  state->AddDisallowedCommand(
    "exec_program", cmExecProgramCommand, cmPolicies::CMP0153,
    "The exec_program command should not be called; see CMP0153.");

#endif
}

void GetProjectCommands(cmState* state)
{
  state->AddBuiltinCommand("add_compile_definitions",
                           cmAddCompileDefinitionsCommand);
  state->AddBuiltinCommand("add_custom_command", cmAddCustomCommandCommand);
  state->AddBuiltinCommand("add_custom_target", cmAddCustomTargetCommand);
  state->AddBuiltinCommand("add_definitions", cmAddDefinitionsCommand);
  state->AddBuiltinCommand("add_dependencies", cmAddDependenciesCommand);
  state->AddBuiltinCommand("add_executable", cmAddExecutableCommand);
  state->AddBuiltinCommand("add_library", cmAddLibraryCommand);
  state->AddBuiltinCommand("add_subdirectory", cmAddSubDirectoryCommand);
  state->AddBuiltinCommand("add_test", cmAddTestCommand);
  state->AddBuiltinCommand("build_command", cmBuildCommand);
  state->AddBuiltinCommand("create_test_sourcelist", cmCreateTestSourceList);
  state->AddBuiltinCommand("define_property", cmDefinePropertyCommand);
  state->AddBuiltinCommand("enable_language", cmEnableLanguageCommand);
  state->AddBuiltinCommand("enable_testing", cmEnableTestingCommand);
  state->AddBuiltinCommand("get_source_file_property",
                           cmGetSourceFilePropertyCommand);
  state->AddBuiltinCommand("get_target_property", cmGetTargetPropertyCommand);
  state->AddBuiltinCommand("get_test_property", cmGetTestPropertyCommand);
  state->AddBuiltinCommand("include_directories", cmIncludeDirectoryCommand);
  state->AddBuiltinCommand("include_regular_expression",
                           cmIncludeRegularExpressionCommand);
  state->AddBuiltinCommand("install", cmInstallCommand);
  state->AddBuiltinCommand("install_files", cmInstallFilesCommand);
  state->AddBuiltinCommand("install_targets", cmInstallTargetsCommand);
  state->AddBuiltinCommand("link_directories", cmLinkDirectoriesCommand);
  state->AddBuiltinCommand("project", cmProjectCommand);
  state->AddBuiltinCommand("set_source_files_properties",
                           cmSetSourceFilesPropertiesCommand);
  state->AddBuiltinCommand("set_target_properties",
                           cmSetTargetPropertiesCommand);
  state->AddBuiltinCommand("set_tests_properties",
                           cmSetTestsPropertiesCommand);
  state->AddBuiltinCommand("subdirs", cmSubdirCommand);
  state->AddBuiltinCommand("target_compile_definitions",
                           cmTargetCompileDefinitionsCommand);
  state->AddBuiltinCommand("target_compile_features",
                           cmTargetCompileFeaturesCommand);
  state->AddBuiltinCommand("target_compile_options",
                           cmTargetCompileOptionsCommand);
  state->AddBuiltinCommand("target_include_directories",
                           cmTargetIncludeDirectoriesCommand);
  state->AddBuiltinCommand("target_link_libraries",
                           cmTargetLinkLibrariesCommand);
  state->AddBuiltinCommand("target_link_options", cmTargetLinkOptionsCommand);
  state->AddBuiltinCommand("target_sources", cmTargetSourcesCommand);
  state->AddBuiltinCommand("try_compile", cmTryCompileCommand);
  state->AddBuiltinCommand("try_run", cmTryRunCommand);
  state->AddBuiltinCommand("target_precompile_headers",
                           cmTargetPrecompileHeadersCommand);

#if !defined(CMAKE_BOOTSTRAP)
  state->AddBuiltinCommand("add_compile_options", cmAddCompileOptionsCommand);
  state->AddBuiltinCommand("aux_source_directory",
                           cmAuxSourceDirectoryCommand);
  state->AddBuiltinCommand("export", cmExportCommand);
  state->AddBuiltinCommand("fltk_wrap_ui", cmFLTKWrapUICommand);
  state->AddBuiltinCommand("include_external_msproject",
                           cmIncludeExternalMSProjectCommand);
  state->AddBuiltinCommand("install_programs", cmInstallProgramsCommand);
  state->AddBuiltinCommand("add_link_options", cmAddLinkOptionsCommand);
  state->AddBuiltinCommand("link_libraries", cmLinkLibrariesCommand);
  state->AddBuiltinCommand("target_link_directories",
                           cmTargetLinkDirectoriesCommand);
  state->AddBuiltinCommand("qt_wrap_cpp", cmQTWrapCPPCommand);
  state->AddBuiltinCommand("qt_wrap_ui", cmQTWrapUICommand);
  state->AddBuiltinCommand("remove_definitions", cmRemoveDefinitionsCommand);
  state->AddBuiltinCommand("source_group", cmSourceGroupCommand);
  state->AddBuiltinCommand("cmake_file_api", cmFileAPICommand);

  state->AddDisallowedCommand(
    "export_library_dependencies", cmExportLibraryDependenciesCommand,
    cmPolicies::CMP0033,
    "The export_library_dependencies command should not be called; "
    "see CMP0033.");
  state->AddDisallowedCommand(
    "load_command", cmLoadCommandCommand, cmPolicies::CMP0031,
    "The load_command command should not be called; see CMP0031.");
  state->AddDisallowedCommand(
    "output_required_files", cmOutputRequiredFilesCommand, cmPolicies::CMP0032,
    "The output_required_files command should not be called; see CMP0032.");
  state->AddDisallowedCommand(
    "subdir_depends", cmSubdirDependsCommand, cmPolicies::CMP0029,
    "The subdir_depends command should not be called; see CMP0029.");
  state->AddDisallowedCommand(
    "utility_source", cmUtilitySourceCommand, cmPolicies::CMP0034,
    "The utility_source command should not be called; see CMP0034.");
  state->AddDisallowedCommand(
    "variable_requires", cmVariableRequiresCommand, cmPolicies::CMP0035,
    "The variable_requires command should not be called; see CMP0035.");
#endif
}

void GetProjectCommandsInScriptMode(cmState* state)
{
#define CM_UNEXPECTED_PROJECT_COMMAND(NAME)                                   \
  state->AddUnexpectedCommand(NAME, "command is not scriptable")

  CM_UNEXPECTED_PROJECT_COMMAND("add_compile_options");
  CM_UNEXPECTED_PROJECT_COMMAND("add_custom_command");
  CM_UNEXPECTED_PROJECT_COMMAND("add_custom_target");
  CM_UNEXPECTED_PROJECT_COMMAND("add_definitions");
  CM_UNEXPECTED_PROJECT_COMMAND("add_dependencies");
  CM_UNEXPECTED_PROJECT_COMMAND("add_executable");
  CM_UNEXPECTED_PROJECT_COMMAND("add_library");
  CM_UNEXPECTED_PROJECT_COMMAND("add_subdirectory");
  CM_UNEXPECTED_PROJECT_COMMAND("add_test");
  CM_UNEXPECTED_PROJECT_COMMAND("aux_source_directory");
  CM_UNEXPECTED_PROJECT_COMMAND("build_command");
  CM_UNEXPECTED_PROJECT_COMMAND("cmake_file_api");
  CM_UNEXPECTED_PROJECT_COMMAND("create_test_sourcelist");
  CM_UNEXPECTED_PROJECT_COMMAND("define_property");
  CM_UNEXPECTED_PROJECT_COMMAND("enable_language");
  CM_UNEXPECTED_PROJECT_COMMAND("enable_testing");
  CM_UNEXPECTED_PROJECT_COMMAND("export");
  CM_UNEXPECTED_PROJECT_COMMAND("fltk_wrap_ui");
  CM_UNEXPECTED_PROJECT_COMMAND("get_source_file_property");
  CM_UNEXPECTED_PROJECT_COMMAND("get_target_property");
  CM_UNEXPECTED_PROJECT_COMMAND("get_test_property");
  CM_UNEXPECTED_PROJECT_COMMAND("include_directories");
  CM_UNEXPECTED_PROJECT_COMMAND("include_external_msproject");
  CM_UNEXPECTED_PROJECT_COMMAND("include_regular_expression");
  CM_UNEXPECTED_PROJECT_COMMAND("install");
  CM_UNEXPECTED_PROJECT_COMMAND("link_directories");
  CM_UNEXPECTED_PROJECT_COMMAND("link_libraries");
  CM_UNEXPECTED_PROJECT_COMMAND("project");
  CM_UNEXPECTED_PROJECT_COMMAND("qt_wrap_cpp");
  CM_UNEXPECTED_PROJECT_COMMAND("qt_wrap_ui");
  CM_UNEXPECTED_PROJECT_COMMAND("remove_definitions");
  CM_UNEXPECTED_PROJECT_COMMAND("set_source_files_properties");
  CM_UNEXPECTED_PROJECT_COMMAND("set_target_properties");
  CM_UNEXPECTED_PROJECT_COMMAND("set_tests_properties");
  CM_UNEXPECTED_PROJECT_COMMAND("source_group");
  CM_UNEXPECTED_PROJECT_COMMAND("target_compile_definitions");
  CM_UNEXPECTED_PROJECT_COMMAND("target_compile_features");
  CM_UNEXPECTED_PROJECT_COMMAND("target_compile_options");
  CM_UNEXPECTED_PROJECT_COMMAND("target_include_directories");
  CM_UNEXPECTED_PROJECT_COMMAND("target_link_libraries");
  CM_UNEXPECTED_PROJECT_COMMAND("target_sources");
  CM_UNEXPECTED_PROJECT_COMMAND("try_compile");
  CM_UNEXPECTED_PROJECT_COMMAND("try_run");

  // deprecated commands
  CM_UNEXPECTED_PROJECT_COMMAND("export_library_dependencies");
  CM_UNEXPECTED_PROJECT_COMMAND("load_command");
  CM_UNEXPECTED_PROJECT_COMMAND("output_required_files");
  CM_UNEXPECTED_PROJECT_COMMAND("subdir_depends");
  CM_UNEXPECTED_PROJECT_COMMAND("utility_source");
  CM_UNEXPECTED_PROJECT_COMMAND("variable_requires");

#undef CM_UNEXPECTED_PROJECT_COMMAND
}
