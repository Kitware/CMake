/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#include "cm_memory.hxx"

#include "cmCommands.h"
#include "cmPolicies.h"
#include "cmState.h"

#include "cmAddCompileDefinitionsCommand.h"
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
#include "cmCommand.h"
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
#include "cmTargetCompileDefinitionsCommand.h"
#include "cmTargetCompileFeaturesCommand.h"
#include "cmTargetCompileOptionsCommand.h"
#include "cmTargetIncludeDirectoriesCommand.h"
#include "cmTargetLinkLibrariesCommand.h"
#include "cmTargetSourcesCommand.h"
#include "cmTryCompileCommand.h"
#include "cmTryRunCommand.h"
#include "cmUnsetCommand.h"
#include "cmWhileCommand.h"

#if defined(CMAKE_BUILD_WITH_CMAKE)
#  include "cmAddCompileOptionsCommand.h"
#  include "cmAddLinkOptionsCommand.h"
#  include "cmAuxSourceDirectoryCommand.h"
#  include "cmBuildNameCommand.h"
#  include "cmCMakeHostSystemInformationCommand.h"
#  include "cmExportCommand.h"
#  include "cmExportLibraryDependenciesCommand.h"
#  include "cmFLTKWrapUICommand.h"
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
#  include "cmTargetLinkOptionsCommand.h"
#  include "cmUseMangledMesaCommand.h"
#  include "cmUtilitySourceCommand.h"
#  include "cmVariableRequiresCommand.h"
#  include "cmVariableWatchCommand.h"
#  include "cmWriteFileCommand.h"
#endif

void GetScriptingCommands(cmState* state)
{
  state->AddBuiltinCommand("break", cm::make_unique<cmBreakCommand>());
  state->AddBuiltinCommand("cmake_minimum_required",
                           cm::make_unique<cmCMakeMinimumRequired>());
  state->AddBuiltinCommand("cmake_policy",
                           cm::make_unique<cmCMakePolicyCommand>());
  state->AddBuiltinCommand("configure_file",
                           cm::make_unique<cmConfigureFileCommand>());
  state->AddBuiltinCommand("continue", cm::make_unique<cmContinueCommand>());
  state->AddBuiltinCommand("exec_program",
                           cm::make_unique<cmExecProgramCommand>());
  state->AddBuiltinCommand("execute_process",
                           cm::make_unique<cmExecuteProcessCommand>());
  state->AddBuiltinCommand("file", cm::make_unique<cmFileCommand>());
  state->AddBuiltinCommand("find_file", cm::make_unique<cmFindFileCommand>());
  state->AddBuiltinCommand("find_library",
                           cm::make_unique<cmFindLibraryCommand>());
  state->AddBuiltinCommand("find_package",
                           cm::make_unique<cmFindPackageCommand>());
  state->AddBuiltinCommand("find_path", cm::make_unique<cmFindPathCommand>());
  state->AddBuiltinCommand("find_program",
                           cm::make_unique<cmFindProgramCommand>());
  state->AddBuiltinCommand("foreach", cm::make_unique<cmForEachCommand>());
  state->AddBuiltinCommand("function", cm::make_unique<cmFunctionCommand>());
  state->AddBuiltinCommand("get_cmake_property",
                           cm::make_unique<cmGetCMakePropertyCommand>());
  state->AddBuiltinCommand("get_directory_property",
                           cm::make_unique<cmGetDirectoryPropertyCommand>());
  state->AddBuiltinCommand("get_filename_component",
                           cm::make_unique<cmGetFilenameComponentCommand>());
  state->AddBuiltinCommand("get_property",
                           cm::make_unique<cmGetPropertyCommand>());
  state->AddBuiltinCommand("if", cmIfCommand);
  state->AddBuiltinCommand("include", cm::make_unique<cmIncludeCommand>());
  state->AddBuiltinCommand("include_guard",
                           cm::make_unique<cmIncludeGuardCommand>());
  state->AddBuiltinCommand("list", cm::make_unique<cmListCommand>());
  state->AddBuiltinCommand("macro", cm::make_unique<cmMacroCommand>());
  state->AddBuiltinCommand("make_directory",
                           cm::make_unique<cmMakeDirectoryCommand>());
  state->AddBuiltinCommand("mark_as_advanced",
                           cm::make_unique<cmMarkAsAdvancedCommand>());
  state->AddBuiltinCommand("math", cm::make_unique<cmMathCommand>());
  state->AddBuiltinCommand("message", cm::make_unique<cmMessageCommand>());
  state->AddBuiltinCommand("option", cm::make_unique<cmOptionCommand>());
  state->AddBuiltinCommand("cmake_parse_arguments",
                           cm::make_unique<cmParseArgumentsCommand>());
  state->AddBuiltinCommand("return", cmReturnCommand);
  state->AddBuiltinCommand("separate_arguments",
                           cm::make_unique<cmSeparateArgumentsCommand>());
  state->AddBuiltinCommand("set", cm::make_unique<cmSetCommand>());
  state->AddBuiltinCommand("set_directory_properties",
                           cm::make_unique<cmSetDirectoryPropertiesCommand>());
  state->AddBuiltinCommand("set_property",
                           cm::make_unique<cmSetPropertyCommand>());
  state->AddBuiltinCommand("site_name", cm::make_unique<cmSiteNameCommand>());
  state->AddBuiltinCommand("string", cm::make_unique<cmStringCommand>());
  state->AddBuiltinCommand("unset", cm::make_unique<cmUnsetCommand>());
  state->AddBuiltinCommand("while", cmWhileCommand);

  state->AddUnexpectedCommand(
    "else",
    "An ELSE command was found outside of a proper "
    "IF ENDIF structure. Or its arguments did not match "
    "the opening IF command.");
  state->AddUnexpectedCommand(
    "elseif",
    "An ELSEIF command was found outside of a proper "
    "IF ENDIF structure.");
  state->AddUnexpectedCommand(
    "endforeach",
    "An ENDFOREACH command was found outside of a proper "
    "FOREACH ENDFOREACH structure. Or its arguments did "
    "not match the opening FOREACH command.");
  state->AddUnexpectedCommand(
    "endfunction",
    "An ENDFUNCTION command was found outside of a proper "
    "FUNCTION ENDFUNCTION structure. Or its arguments did not "
    "match the opening FUNCTION command.");
  state->AddUnexpectedCommand(
    "endif",
    "An ENDIF command was found outside of a proper "
    "IF ENDIF structure. Or its arguments did not match "
    "the opening IF command.");
  state->AddUnexpectedCommand(
    "endmacro",
    "An ENDMACRO command was found outside of a proper "
    "MACRO ENDMACRO structure. Or its arguments did not "
    "match the opening MACRO command.");
  state->AddUnexpectedCommand(
    "endwhile",
    "An ENDWHILE command was found outside of a proper "
    "WHILE ENDWHILE structure. Or its arguments did not "
    "match the opening WHILE command.");

#if defined(CMAKE_BUILD_WITH_CMAKE)
  state->AddBuiltinCommand(
    "cmake_host_system_information",
    cm::make_unique<cmCMakeHostSystemInformationCommand>());
  state->AddBuiltinCommand("remove", cm::make_unique<cmRemoveCommand>());
  state->AddBuiltinCommand("variable_watch",
                           cm::make_unique<cmVariableWatchCommand>());
  state->AddBuiltinCommand("write_file",
                           cm::make_unique<cmWriteFileCommand>());

  state->AddDisallowedCommand(
    "build_name", cm::make_unique<cmBuildNameCommand>(), cmPolicies::CMP0036,
    "The build_name command should not be called; see CMP0036.");
  state->AddDisallowedCommand(
    "use_mangled_mesa", cm::make_unique<cmUseMangledMesaCommand>(),
    cmPolicies::CMP0030,
    "The use_mangled_mesa command should not be called; see CMP0030.");

#endif
}

void GetProjectCommands(cmState* state)
{
  state->AddBuiltinCommand("add_custom_command",
                           cm::make_unique<cmAddCustomCommandCommand>());
  state->AddBuiltinCommand("add_custom_target",
                           cm::make_unique<cmAddCustomTargetCommand>());
  state->AddBuiltinCommand("add_definitions",
                           cm::make_unique<cmAddDefinitionsCommand>());
  state->AddBuiltinCommand("add_dependencies",
                           cm::make_unique<cmAddDependenciesCommand>());
  state->AddBuiltinCommand("add_executable",
                           cm::make_unique<cmAddExecutableCommand>());
  state->AddBuiltinCommand("add_library",
                           cm::make_unique<cmAddLibraryCommand>());
  state->AddBuiltinCommand("add_subdirectory",
                           cm::make_unique<cmAddSubDirectoryCommand>());
  state->AddBuiltinCommand("add_test", cm::make_unique<cmAddTestCommand>());
  state->AddBuiltinCommand("build_command", cm::make_unique<cmBuildCommand>());
  state->AddBuiltinCommand("create_test_sourcelist",
                           cm::make_unique<cmCreateTestSourceList>());
  state->AddBuiltinCommand("define_property",
                           cm::make_unique<cmDefinePropertyCommand>());
  state->AddBuiltinCommand("enable_language",
                           cm::make_unique<cmEnableLanguageCommand>());
  state->AddBuiltinCommand("enable_testing", cmEnableTestingCommand);
  state->AddBuiltinCommand("get_source_file_property",
                           cm::make_unique<cmGetSourceFilePropertyCommand>());
  state->AddBuiltinCommand("get_target_property",
                           cm::make_unique<cmGetTargetPropertyCommand>());
  state->AddBuiltinCommand("get_test_property",
                           cm::make_unique<cmGetTestPropertyCommand>());
  state->AddBuiltinCommand("include_directories",
                           cm::make_unique<cmIncludeDirectoryCommand>());
  state->AddBuiltinCommand(
    "include_regular_expression",
    cm::make_unique<cmIncludeRegularExpressionCommand>());
  state->AddBuiltinCommand("install", cm::make_unique<cmInstallCommand>());
  state->AddBuiltinCommand("install_files",
                           cm::make_unique<cmInstallFilesCommand>());
  state->AddBuiltinCommand("install_targets",
                           cm::make_unique<cmInstallTargetsCommand>());
  state->AddBuiltinCommand("link_directories",
                           cm::make_unique<cmLinkDirectoriesCommand>());
  state->AddBuiltinCommand("project", cm::make_unique<cmProjectCommand>());
  state->AddBuiltinCommand(
    "set_source_files_properties",
    cm::make_unique<cmSetSourceFilesPropertiesCommand>());
  state->AddBuiltinCommand("set_target_properties",
                           cm::make_unique<cmSetTargetPropertiesCommand>());
  state->AddBuiltinCommand("set_tests_properties",
                           cm::make_unique<cmSetTestsPropertiesCommand>());
  state->AddBuiltinCommand("subdirs", cm::make_unique<cmSubdirCommand>());
  state->AddBuiltinCommand(
    "target_compile_definitions",
    cm::make_unique<cmTargetCompileDefinitionsCommand>());
  state->AddBuiltinCommand("target_compile_features",
                           cm::make_unique<cmTargetCompileFeaturesCommand>());
  state->AddBuiltinCommand("target_compile_options",
                           cm::make_unique<cmTargetCompileOptionsCommand>());
  state->AddBuiltinCommand(
    "target_include_directories",
    cm::make_unique<cmTargetIncludeDirectoriesCommand>());
  state->AddBuiltinCommand("target_link_libraries",
                           cm::make_unique<cmTargetLinkLibrariesCommand>());
  state->AddBuiltinCommand("target_sources",
                           cm::make_unique<cmTargetSourcesCommand>());
  state->AddBuiltinCommand("try_compile",
                           cm::make_unique<cmTryCompileCommand>());
  state->AddBuiltinCommand("try_run", cm::make_unique<cmTryRunCommand>());

#if defined(CMAKE_BUILD_WITH_CMAKE)
  state->AddBuiltinCommand("add_compile_definitions",
                           cm::make_unique<cmAddCompileDefinitionsCommand>());
  state->AddBuiltinCommand("add_compile_options",
                           cm::make_unique<cmAddCompileOptionsCommand>());
  state->AddBuiltinCommand("aux_source_directory",
                           cm::make_unique<cmAuxSourceDirectoryCommand>());
  state->AddBuiltinCommand("export", cm::make_unique<cmExportCommand>());
  state->AddBuiltinCommand("fltk_wrap_ui",
                           cm::make_unique<cmFLTKWrapUICommand>());
  state->AddBuiltinCommand(
    "include_external_msproject",
    cm::make_unique<cmIncludeExternalMSProjectCommand>());
  state->AddBuiltinCommand("install_programs",
                           cm::make_unique<cmInstallProgramsCommand>());
  state->AddBuiltinCommand("add_link_options",
                           cm::make_unique<cmAddLinkOptionsCommand>());
  state->AddBuiltinCommand("link_libraries",
                           cm::make_unique<cmLinkLibrariesCommand>());
  state->AddBuiltinCommand("target_link_options",
                           cm::make_unique<cmTargetLinkOptionsCommand>());
  state->AddBuiltinCommand("target_link_directories",
                           cm::make_unique<cmTargetLinkDirectoriesCommand>());
  state->AddBuiltinCommand("load_cache",
                           cm::make_unique<cmLoadCacheCommand>());
  state->AddBuiltinCommand("qt_wrap_cpp",
                           cm::make_unique<cmQTWrapCPPCommand>());
  state->AddBuiltinCommand("qt_wrap_ui", cm::make_unique<cmQTWrapUICommand>());
  state->AddBuiltinCommand("remove_definitions",
                           cm::make_unique<cmRemoveDefinitionsCommand>());
  state->AddBuiltinCommand("source_group",
                           cm::make_unique<cmSourceGroupCommand>());

  state->AddDisallowedCommand(
    "export_library_dependencies",
    cm::make_unique<cmExportLibraryDependenciesCommand>(), cmPolicies::CMP0033,
    "The export_library_dependencies command should not be called; "
    "see CMP0033.");
  state->AddDisallowedCommand(
    "load_command", cm::make_unique<cmLoadCommandCommand>(),
    cmPolicies::CMP0031,
    "The load_command command should not be called; see CMP0031.");
  state->AddDisallowedCommand(
    "output_required_files", cm::make_unique<cmOutputRequiredFilesCommand>(),
    cmPolicies::CMP0032,
    "The output_required_files command should not be called; see CMP0032.");
  state->AddDisallowedCommand(
    "subdir_depends", cm::make_unique<cmSubdirDependsCommand>(),
    cmPolicies::CMP0029,
    "The subdir_depends command should not be called; see CMP0029.");
  state->AddDisallowedCommand(
    "utility_source", cm::make_unique<cmUtilitySourceCommand>(),
    cmPolicies::CMP0034,
    "The utility_source command should not be called; see CMP0034.");
  state->AddDisallowedCommand(
    "variable_requires", cm::make_unique<cmVariableRequiresCommand>(),
    cmPolicies::CMP0035,
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
  CM_UNEXPECTED_PROJECT_COMMAND("load_cache");
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
