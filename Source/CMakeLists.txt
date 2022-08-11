# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

# To ensure maximum portability across various compilers and platforms
# deactivate any compiler extensions.  Skip this for QNX, where additional
# work is needed to build without compiler extensions.
if (NOT CMAKE_SYSTEM_NAME STREQUAL "QNX")
  set(CMAKE_C_EXTENSIONS FALSE)
  set(CMAKE_CXX_EXTENSIONS FALSE)
endif()

include(CheckIncludeFile)

if(NOT CMake_DEFAULT_RECURSION_LIMIT)
  if(DEFINED ENV{DASHBOARD_TEST_FROM_CTEST})
    set(CMake_DEFAULT_RECURSION_LIMIT 100)
  elseif(MINGW OR MSYS)
    set(CMake_DEFAULT_RECURSION_LIMIT 400)
  elseif(WIN32 AND CMAKE_C_COMPILER_ID STREQUAL "IntelLLVM")
    set(CMake_DEFAULT_RECURSION_LIMIT 600)
  else()
    set(CMake_DEFAULT_RECURSION_LIMIT 1000)
  endif()
endif()

if(APPLE)
  set(CMake_USE_MACH_PARSER 1)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "AIX")
  set(CMake_USE_XCOFF_PARSER 1)
endif()

set(EXECUTABLE_OUTPUT_PATH ${CMake_BIN_DIR})

if(WIN32)
  # ensure Unicode friendly APIs are used on Windows
  add_definitions(-DUNICODE -D_UNICODE)

  # minimize windows.h content
  add_definitions(-DWIN32_LEAN_AND_MEAN)
endif()

# configure the .dox.in file
if(CMake_BUILD_DEVELOPER_REFERENCE)
  configure_file(
    "${CMake_SOURCE_DIR}/Source/dir.dox.in"
    "${CMake_BINARY_DIR}/Source/dir.dox"
    @ONLY
    )
endif()

# configure the .h file
configure_file(
  "${CMake_SOURCE_DIR}/Source/cmConfigure.cmake.h.in"
  "${CMake_BINARY_DIR}/Source/cmConfigure.h"
  )
configure_file(
  "${CMake_SOURCE_DIR}/Source/cmVersionConfig.h.in"
  "${CMake_BINARY_DIR}/Source/cmVersionConfig.h"
  )
configure_file(
  "${CMake_SOURCE_DIR}/Source/CPack/cmCPackConfigure.h.in"
  "${CMake_BINARY_DIR}/Source/CPack/cmCPackConfigure.h"
  )

# Tell CMake executable in the build tree where to find the source tree.
configure_file(
  "${CMake_SOURCE_DIR}/Source/CMakeSourceDir.txt.in"
  "${CMake_BINARY_DIR}/CMakeFiles/CMakeSourceDir.txt" @ONLY
  )

# add the include path to find the .h
include_directories(
  "${CMake_BINARY_DIR}/Source"
  "${CMake_SOURCE_DIR}/Source"
  "${CMake_SOURCE_DIR}/Source/LexerParser"
  ${CMAKE_ZLIB_INCLUDES}
  ${CMAKE_EXPAT_INCLUDES}
  ${CMAKE_TAR_INCLUDES}
  ${CMake_HAIKU_INCLUDE_DIRS}
  )

# Check if we can build the Mach-O parser.
if(CMake_USE_MACH_PARSER)
  set(MACH_SRCS cmMachO.h cmMachO.cxx)
endif()

# Check if we can build the XCOFF parser.
if(CMake_USE_XCOFF_PARSER)
  set(XCOFF_SRCS cmXCOFF.h cmXCOFF.cxx)
endif()

#
# Sources for CMakeLib
#
set(SRCS
  # Lexers/Parsers
  LexerParser/cmCommandArgumentLexer.cxx
  LexerParser/cmCommandArgumentLexer.h
  LexerParser/cmCommandArgumentLexer.in.l
  LexerParser/cmCommandArgumentParser.cxx
  LexerParser/cmCommandArgumentParserTokens.h
  LexerParser/cmCommandArgumentParser.y
  LexerParser/cmDependsJavaLexer.cxx
  LexerParser/cmDependsJavaLexer.h
  LexerParser/cmDependsJavaLexer.in.l
  LexerParser/cmDependsJavaParser.cxx
  LexerParser/cmDependsJavaParserTokens.h
  LexerParser/cmDependsJavaParser.y
  LexerParser/cmExprLexer.cxx
  LexerParser/cmExprLexer.h
  LexerParser/cmExprLexer.in.l
  LexerParser/cmExprParser.cxx
  LexerParser/cmExprParserTokens.h
  LexerParser/cmExprParser.y
  LexerParser/cmFortranLexer.cxx
  LexerParser/cmFortranLexer.h
  LexerParser/cmFortranLexer.in.l
  LexerParser/cmFortranParser.cxx
  LexerParser/cmFortranParserTokens.h
  LexerParser/cmFortranParser.y
  LexerParser/cmGccDepfileLexer.cxx
  LexerParser/cmGccDepfileLexer.h
  LexerParser/cmGccDepfileLexer.in.l
  LexerParser/cmListFileLexer.c
  LexerParser/cmListFileLexer.in.l

  cmAffinity.cxx
  cmAffinity.h
  cmAlgorithms.h
  cmArchiveWrite.cxx
  cmArgumentParser.cxx
  cmArgumentParser.h
  cmBase32.cxx
  cmBinUtilsLinker.cxx
  cmBinUtilsLinker.h
  cmBinUtilsLinuxELFGetRuntimeDependenciesTool.cxx
  cmBinUtilsLinuxELFGetRuntimeDependenciesTool.h
  cmBinUtilsLinuxELFLinker.cxx
  cmBinUtilsLinuxELFLinker.h
  cmBinUtilsLinuxELFObjdumpGetRuntimeDependenciesTool.cxx
  cmBinUtilsLinuxELFObjdumpGetRuntimeDependenciesTool.h
  cmBinUtilsMacOSMachOGetRuntimeDependenciesTool.cxx
  cmBinUtilsMacOSMachOGetRuntimeDependenciesTool.h
  cmBinUtilsMacOSMachOLinker.cxx
  cmBinUtilsMacOSMachOLinker.h
  cmBinUtilsMacOSMachOOToolGetRuntimeDependenciesTool.cxx
  cmBinUtilsMacOSMachOOToolGetRuntimeDependenciesTool.h
  cmBinUtilsWindowsPEDumpbinGetRuntimeDependenciesTool.cxx
  cmBinUtilsWindowsPEDumpbinGetRuntimeDependenciesTool.h
  cmBinUtilsWindowsPEGetRuntimeDependenciesTool.cxx
  cmBinUtilsWindowsPEGetRuntimeDependenciesTool.h
  cmBinUtilsWindowsPELinker.cxx
  cmBinUtilsWindowsPELinker.h
  cmBinUtilsWindowsPEObjdumpGetRuntimeDependenciesTool.cxx
  cmBinUtilsWindowsPEObjdumpGetRuntimeDependenciesTool.h
  cmBuildOptions.h
  cmCacheManager.cxx
  cmCacheManager.h
  cmCLocaleEnvironmentScope.h
  cmCLocaleEnvironmentScope.cxx
  cmCMakePath.h
  cmCMakePath.cxx
  cmCMakePresetsGraph.cxx
  cmCMakePresetsGraph.h
  cmCMakePresetsGraphInternal.h
  cmCMakePresetsGraphReadJSON.cxx
  cmCMakePresetsGraphReadJSONBuildPresets.cxx
  cmCMakePresetsGraphReadJSONConfigurePresets.cxx
  cmCMakePresetsGraphReadJSONTestPresets.cxx
  cmCommandArgumentParserHelper.cxx
  cmCommonTargetGenerator.cxx
  cmCommonTargetGenerator.h
  cmComputeComponentGraph.cxx
  cmComputeComponentGraph.h
  cmComputeLinkDepends.cxx
  cmComputeLinkDepends.h
  cmComputeLinkInformation.cxx
  cmComputeLinkInformation.h
  cmComputeTargetDepends.h
  cmComputeTargetDepends.cxx
  cmConsoleBuf.h
  cmConsoleBuf.cxx
  cmConstStack.h
  cmConstStack.tcc
  cmCPackPropertiesGenerator.h
  cmCPackPropertiesGenerator.cxx
  cmCryptoHash.cxx
  cmCryptoHash.h
  cmCurl.cxx
  cmCurl.h
  cmCustomCommand.cxx
  cmCustomCommand.h
  cmCustomCommandGenerator.cxx
  cmCustomCommandGenerator.h
  cmCustomCommandLines.cxx
  cmCustomCommandLines.h
  cmCustomCommandTypes.h
  cmDefinitions.cxx
  cmDefinitions.h
  cmDependencyProvider.h
  cmDepends.cxx
  cmDepends.h
  cmDependsC.cxx
  cmDependsC.h
  cmDependsFortran.cxx
  cmDependsFortran.h
  cmDependsJava.cxx
  cmDependsJava.h
  cmDependsJavaParserHelper.cxx
  cmDependsJavaParserHelper.h
  cmDependsCompiler.cxx
  cmDependsCompiler.h
  cmDocumentation.cxx
  cmDocumentationFormatter.cxx
  cmDocumentationSection.cxx
  cmDynamicLoader.cxx
  cmDynamicLoader.h
  cmELF.h
  cmELF.cxx
  cmExprParserHelper.cxx
  cmExportBuildAndroidMKGenerator.h
  cmExportBuildAndroidMKGenerator.cxx
  cmExportBuildFileGenerator.h
  cmExportBuildFileGenerator.cxx
  cmExportFileGenerator.h
  cmExportFileGenerator.cxx
  cmExportInstallAndroidMKGenerator.h
  cmExportInstallAndroidMKGenerator.cxx
  cmExportInstallFileGenerator.h
  cmExportInstallFileGenerator.cxx
  cmExportTryCompileFileGenerator.h
  cmExportTryCompileFileGenerator.cxx
  cmExportSet.h
  cmExportSet.cxx
  cmExternalMakefileProjectGenerator.cxx
  cmExternalMakefileProjectGenerator.h
  cmExtraCodeBlocksGenerator.cxx
  cmExtraCodeBlocksGenerator.h
  cmExtraCodeLiteGenerator.cxx
  cmExtraCodeLiteGenerator.h
  cmExtraEclipseCDT4Generator.cxx
  cmExtraEclipseCDT4Generator.h
  cmExtraKateGenerator.cxx
  cmExtraKateGenerator.h
  cmExtraSublimeTextGenerator.cxx
  cmExtraSublimeTextGenerator.h
  cmFileAPI.cxx
  cmFileAPI.h
  cmFileAPICache.cxx
  cmFileAPICache.h
  cmFileAPICodemodel.cxx
  cmFileAPICodemodel.h
  cmFileAPICMakeFiles.cxx
  cmFileAPICMakeFiles.h
  cmFileAPIToolchains.cxx
  cmFileAPIToolchains.h
  cmFileCopier.cxx
  cmFileCopier.h
  cmFileInstaller.cxx
  cmFileInstaller.h
  cmFileLock.cxx
  cmFileLock.h
  cmFileLockPool.cxx
  cmFileLockPool.h
  cmFileLockResult.cxx
  cmFileLockResult.h
  cmFilePathChecksum.cxx
  cmFilePathChecksum.h
  cmFileSet.cxx
  cmFileSet.h
  cmFileTime.cxx
  cmFileTime.h
  cmFileTimeCache.cxx
  cmFileTimeCache.h
  cmFileTimes.cxx
  cmFileTimes.h
  cmFortranParserImpl.cxx
  cmFSPermissions.cxx
  cmFSPermissions.h
  cmGccDepfileLexerHelper.cxx
  cmGccDepfileLexerHelper.h
  cmGccDepfileReader.cxx
  cmGccDepfileReader.h
  cmGeneratedFileStream.cxx
  cmGeneratorExpressionContext.cxx
  cmGeneratorExpressionContext.h
  cmGeneratorExpressionDAGChecker.cxx
  cmGeneratorExpressionDAGChecker.h
  cmGeneratorExpressionEvaluationFile.cxx
  cmGeneratorExpressionEvaluationFile.h
  cmGeneratorExpressionEvaluator.cxx
  cmGeneratorExpressionEvaluator.h
  cmGeneratorExpressionLexer.cxx
  cmGeneratorExpressionLexer.h
  cmGeneratorExpressionNode.cxx
  cmGeneratorExpressionNode.h
  cmGeneratorExpressionParser.cxx
  cmGeneratorExpressionParser.h
  cmGeneratorExpression.cxx
  cmGeneratorExpression.h
  cmGeneratorTarget.cxx
  cmGeneratorTarget.h
  cmLinkItemGraphVisitor.cxx
  cmLinkItemGraphVisitor.h
  cmGetPipes.cxx
  cmGetPipes.h
  cmGlobalCommonGenerator.cxx
  cmGlobalCommonGenerator.h
  cmGlobalGenerator.cxx
  cmGlobalGenerator.h
  cmGlobalGeneratorFactory.h
  cmGlobalUnixMakefileGenerator3.cxx
  cmGlobalUnixMakefileGenerator3.h
  cmGlobVerificationManager.cxx
  cmGlobVerificationManager.h
  cmGraphAdjacencyList.h
  cmGraphVizWriter.cxx
  cmGraphVizWriter.h
  cmInstallGenerator.h
  cmInstallGenerator.cxx
  cmInstallGetRuntimeDependenciesGenerator.h
  cmInstallGetRuntimeDependenciesGenerator.cxx
  cmInstallExportGenerator.cxx
  cmInstalledFile.h
  cmInstalledFile.cxx
  cmInstallFileSetGenerator.h
  cmInstallFileSetGenerator.cxx
  cmInstallFilesGenerator.h
  cmInstallFilesGenerator.cxx
  cmInstallImportedRuntimeArtifactsGenerator.h
  cmInstallImportedRuntimeArtifactsGenerator.cxx
  cmInstallRuntimeDependencySet.h
  cmInstallRuntimeDependencySet.cxx
  cmInstallRuntimeDependencySetGenerator.h
  cmInstallRuntimeDependencySetGenerator.cxx
  cmInstallScriptGenerator.h
  cmInstallScriptGenerator.cxx
  cmInstallSubdirectoryGenerator.h
  cmInstallSubdirectoryGenerator.cxx
  cmInstallTargetGenerator.h
  cmInstallTargetGenerator.cxx
  cmInstallDirectoryGenerator.h
  cmInstallDirectoryGenerator.cxx
  cmJSONHelpers.h
  cmLDConfigLDConfigTool.cxx
  cmLDConfigLDConfigTool.h
  cmLDConfigTool.cxx
  cmLDConfigTool.h
  cmLinkedTree.h
  cmLinkItem.cxx
  cmLinkItem.h
  cmLinkLineComputer.cxx
  cmLinkLineComputer.h
  cmLinkLineDeviceComputer.cxx
  cmLinkLineDeviceComputer.h
  cmListFileCache.cxx
  cmListFileCache.h
  cmLocalCommonGenerator.cxx
  cmLocalCommonGenerator.h
  cmLocalGenerator.cxx
  cmLocalGenerator.h
  cmPlaceholderExpander.cxx
  cmPlaceholderExpander.h
  cmRulePlaceholderExpander.cxx
  cmRulePlaceholderExpander.h
  cmLocalUnixMakefileGenerator3.cxx
  cmLocale.h
  ${MACH_SRCS}
  cmMakefile.cxx
  cmMakefile.h
  cmMakefileTargetGenerator.cxx
  cmMakefileExecutableTargetGenerator.cxx
  cmMakefileLibraryTargetGenerator.cxx
  cmMakefileProfilingData.cxx
  cmMakefileUtilityTargetGenerator.cxx
  cmMessageType.h
  cmMessenger.cxx
  cmMessenger.h
  cmMSVC60LinkLineComputer.cxx
  cmMSVC60LinkLineComputer.h
  cmOSXBundleGenerator.cxx
  cmOSXBundleGenerator.h
  cmOutputConverter.cxx
  cmOutputConverter.h
  cmNewLineStyle.h
  cmNewLineStyle.cxx
  cmOrderDirectories.cxx
  cmOrderDirectories.h
  cmPolicies.h
  cmPolicies.cxx
  cmProcessOutput.cxx
  cmProcessOutput.h
  cmProcessTools.cxx
  cmProcessTools.h
  cmValue.cxx
  cmValue.h
  cmProperty.h
  cmPropertyDefinition.cxx
  cmPropertyDefinition.h
  cmPropertyMap.cxx
  cmPropertyMap.h
  cmQtAutoGen.cxx
  cmQtAutoGen.h
  cmQtAutoGenerator.cxx
  cmQtAutoGenerator.h
  cmQtAutoGenGlobalInitializer.cxx
  cmQtAutoGenGlobalInitializer.h
  cmQtAutoGenInitializer.cxx
  cmQtAutoGenInitializer.h
  cmQtAutoMocUic.cxx
  cmQtAutoMocUic.h
  cmQtAutoRcc.cxx
  cmQtAutoRcc.h
  cmRST.cxx
  cmRST.h
  cmRuntimeDependencyArchive.cxx
  cmRuntimeDependencyArchive.h
  cmScriptGenerator.h
  cmScriptGenerator.cxx
  cmSourceFile.cxx
  cmSourceFile.h
  cmSourceFileLocation.cxx
  cmSourceFileLocation.h
  cmSourceFileLocationKind.h
  cmSourceGroup.cxx
  cmSourceGroup.h
  cmStandardLevelResolver.cxx
  cmStandardLevelResolver.h
  cmState.cxx
  cmState.h
  cmStateDirectory.cxx
  cmStateDirectory.h
  cmStateSnapshot.cxx
  cmStateSnapshot.h
  cmStateTypes.h
  cmStringAlgorithms.cxx
  cmStringAlgorithms.h
  cmSystemTools.cxx
  cmSystemTools.h
  cmTarget.cxx
  cmTarget.h
  cmTargetPropertyComputer.cxx
  cmTargetPropertyComputer.h
  cmTargetExport.h
  cmTest.cxx
  cmTest.h
  cmTestGenerator.cxx
  cmTestGenerator.h
  cmTransformDepfile.cxx
  cmTransformDepfile.h
  cmUuid.cxx
  cmUVHandlePtr.cxx
  cmUVHandlePtr.h
  cmUVProcessChain.cxx
  cmUVProcessChain.h
  cmUVStreambuf.h
  cmUVSignalHackRAII.h
  cmVariableWatch.cxx
  cmVariableWatch.h
  cmVersion.cxx
  cmVersion.h
  cmWindowsRegistry.cxx
  cmWindowsRegistry.h
  cmWorkerPool.cxx
  cmWorkerPool.h
  cmWorkingDirectory.cxx
  cmWorkingDirectory.h
  ${XCOFF_SRCS}
  cmXMLParser.cxx
  cmXMLParser.h
  cmXMLSafe.cxx
  cmXMLSafe.h
  cmXMLWriter.cxx
  cmXMLWriter.h
  cmake.cxx
  cmake.h

  cmCommand.cxx
  cmCommand.h
  cmCommands.cxx
  cmCommands.h
  cmAddCompileDefinitionsCommand.cxx
  cmAddCompileDefinitionsCommand.h
  cmAddCompileOptionsCommand.cxx
  cmAddCompileOptionsCommand.h
  cmAddLinkOptionsCommand.cxx
  cmAddLinkOptionsCommand.h
  cmAddCustomCommandCommand.cxx
  cmAddCustomCommandCommand.h
  cmAddCustomTargetCommand.cxx
  cmAddCustomTargetCommand.h
  cmAddDefinitionsCommand.cxx
  cmAddDefinitionsCommand.h
  cmAddDependenciesCommand.cxx
  cmAddDependenciesCommand.h
  cmAddExecutableCommand.cxx
  cmAddExecutableCommand.h
  cmAddLibraryCommand.cxx
  cmAddLibraryCommand.h
  cmAddSubDirectoryCommand.cxx
  cmAddSubDirectoryCommand.h
  cmAddTestCommand.cxx
  cmAddTestCommand.h
  cmAuxSourceDirectoryCommand.cxx
  cmAuxSourceDirectoryCommand.h
  cmBreakCommand.cxx
  cmBreakCommand.h
  cmBuildCommand.cxx
  cmBuildCommand.h
  cmBuildNameCommand.cxx
  cmBuildNameCommand.h
  cmCMakeHostSystemInformationCommand.cxx
  cmCMakeHostSystemInformationCommand.h
  cmCMakeLanguageCommand.cxx
  cmCMakeLanguageCommand.h
  cmCMakeMinimumRequired.cxx
  cmCMakeMinimumRequired.h
  cmCMakePathCommand.h
  cmCMakePathCommand.cxx
  cmCMakePolicyCommand.cxx
  cmCMakePolicyCommand.h
  cmConditionEvaluator.cxx
  cmConditionEvaluator.h
  cmConfigureFileCommand.cxx
  cmConfigureFileCommand.h
  cmContinueCommand.cxx
  cmContinueCommand.h
  cmCoreTryCompile.cxx
  cmCoreTryCompile.h
  cmCreateTestSourceList.cxx
  cmCreateTestSourceList.h
  cmDefinePropertyCommand.cxx
  cmDefinePropertyCommand.h
  cmEnableLanguageCommand.cxx
  cmEnableLanguageCommand.h
  cmEnableTestingCommand.cxx
  cmEnableTestingCommand.h
  cmExecProgramCommand.cxx
  cmExecProgramCommand.h
  cmExecuteProcessCommand.cxx
  cmExecuteProcessCommand.h
  cmExpandedCommandArgument.cxx
  cmExpandedCommandArgument.h
  cmExportCommand.cxx
  cmExportCommand.h
  cmExportLibraryDependenciesCommand.cxx
  cmExportLibraryDependenciesCommand.h
  cmFLTKWrapUICommand.cxx
  cmFLTKWrapUICommand.h
  cmFileCommand.cxx
  cmFileCommand.h
  cmFindBase.cxx
  cmFindBase.h
  cmFindCommon.cxx
  cmFindCommon.h
  cmFindFileCommand.cxx
  cmFindFileCommand.h
  cmFindLibraryCommand.cxx
  cmFindLibraryCommand.h
  cmFindPackageCommand.cxx
  cmFindPackageCommand.h
  cmFindPathCommand.cxx
  cmFindPathCommand.h
  cmFindProgramCommand.cxx
  cmFindProgramCommand.h
  cmForEachCommand.cxx
  cmForEachCommand.h
  cmFunctionBlocker.cxx
  cmFunctionBlocker.h
  cmFunctionCommand.cxx
  cmFunctionCommand.h
  cmGetCMakePropertyCommand.cxx
  cmGetCMakePropertyCommand.h
  cmGetDirectoryPropertyCommand.cxx
  cmGetDirectoryPropertyCommand.h
  cmGetFilenameComponentCommand.cxx
  cmGetFilenameComponentCommand.h
  cmGetPropertyCommand.cxx
  cmGetPropertyCommand.h
  cmGetSourceFilePropertyCommand.cxx
  cmGetSourceFilePropertyCommand.h
  cmGetTargetPropertyCommand.cxx
  cmGetTargetPropertyCommand.h
  cmGetTestPropertyCommand.cxx
  cmGetTestPropertyCommand.h
  cmHexFileConverter.cxx
  cmHexFileConverter.h
  cmIfCommand.cxx
  cmIfCommand.h
  cmIncludeCommand.cxx
  cmIncludeCommand.h
  cmIncludeDirectoryCommand.cxx
  cmIncludeDirectoryCommand.h
  cmIncludeExternalMSProjectCommand.cxx
  cmIncludeExternalMSProjectCommand.h
  cmIncludeGuardCommand.cxx
  cmIncludeGuardCommand.h
  cmIncludeRegularExpressionCommand.cxx
  cmIncludeRegularExpressionCommand.h
  cmInstallCommand.cxx
  cmInstallCommand.h
  cmInstallCommandArguments.cxx
  cmInstallCommandArguments.h
  cmInstallFilesCommand.cxx
  cmInstallFilesCommand.h
  cmInstallProgramsCommand.cxx
  cmInstallProgramsCommand.h
  cmInstallTargetsCommand.cxx
  cmInstallTargetsCommand.h
  cmLinkDirectoriesCommand.cxx
  cmLinkDirectoriesCommand.h
  cmLinkLibrariesCommand.cxx
  cmLinkLibrariesCommand.h
  cmListCommand.cxx
  cmListCommand.h
  cmLoadCacheCommand.cxx
  cmLoadCacheCommand.h
  cmLoadCommandCommand.cxx
  cmLoadCommandCommand.h
  cmMacroCommand.cxx
  cmMacroCommand.h
  cmMakeDirectoryCommand.cxx
  cmMakeDirectoryCommand.h
  cmMarkAsAdvancedCommand.cxx
  cmMarkAsAdvancedCommand.h
  cmMathCommand.cxx
  cmMathCommand.h
  cmMessageCommand.cxx
  cmMessageCommand.h
  cmMessageMetadata.h
  cmOptionCommand.cxx
  cmOptionCommand.h
  cmOutputRequiredFilesCommand.cxx
  cmOutputRequiredFilesCommand.h
  cmParseArgumentsCommand.cxx
  cmParseArgumentsCommand.h
  cmPathLabel.cxx
  cmPathLabel.h
  cmProjectCommand.cxx
  cmProjectCommand.h
  cmQTWrapCPPCommand.cxx
  cmQTWrapCPPCommand.h
  cmQTWrapUICommand.cxx
  cmQTWrapUICommand.h
  cmRemoveCommand.cxx
  cmRemoveCommand.h
  cmRemoveDefinitionsCommand.cxx
  cmRemoveDefinitionsCommand.h
  cmReturnCommand.cxx
  cmReturnCommand.h
  cmSearchPath.cxx
  cmSearchPath.h
  cmSeparateArgumentsCommand.cxx
  cmSeparateArgumentsCommand.h
  cmSetCommand.cxx
  cmSetCommand.h
  cmSetDirectoryPropertiesCommand.cxx
  cmSetDirectoryPropertiesCommand.h
  cmSetPropertyCommand.cxx
  cmSetPropertyCommand.h
  cmSetSourceFilesPropertiesCommand.cxx
  cmSetSourceFilesPropertiesCommand.h
  cmSetTargetPropertiesCommand.cxx
  cmSetTargetPropertiesCommand.h
  cmSetTestsPropertiesCommand.cxx
  cmSetTestsPropertiesCommand.h
  cmSiteNameCommand.cxx
  cmSiteNameCommand.h
  cmSourceGroupCommand.cxx
  cmSourceGroupCommand.h
  cmString.cxx
  cmString.hxx
  cmStringReplaceHelper.cxx
  cmStringCommand.cxx
  cmStringCommand.h
  cmSubcommandTable.cxx
  cmSubcommandTable.h
  cmSubdirCommand.cxx
  cmSubdirCommand.h
  cmSubdirDependsCommand.cxx
  cmSubdirDependsCommand.h
  cmTargetCompileDefinitionsCommand.cxx
  cmTargetCompileDefinitionsCommand.h
  cmTargetCompileFeaturesCommand.cxx
  cmTargetCompileFeaturesCommand.h
  cmTargetCompileOptionsCommand.cxx
  cmTargetCompileOptionsCommand.h
  cmTargetIncludeDirectoriesCommand.cxx
  cmTargetIncludeDirectoriesCommand.h
  cmTargetLinkOptionsCommand.cxx
  cmTargetLinkOptionsCommand.h
  cmTargetLinkDirectoriesCommand.cxx
  cmTargetLinkDirectoriesCommand.h
  cmTargetLinkLibrariesCommand.cxx
  cmTargetLinkLibrariesCommand.h
  cmTargetPrecompileHeadersCommand.cxx
  cmTargetPrecompileHeadersCommand.h
  cmTargetPropCommandBase.cxx
  cmTargetPropCommandBase.h
  cmTargetSourcesCommand.cxx
  cmTargetSourcesCommand.h
  cmTimestamp.cxx
  cmTimestamp.h
  cmTryCompileCommand.cxx
  cmTryCompileCommand.h
  cmTryRunCommand.cxx
  cmTryRunCommand.h
  cmUnsetCommand.cxx
  cmUnsetCommand.h
  cmUseMangledMesaCommand.cxx
  cmUseMangledMesaCommand.h
  cmUtilitySourceCommand.cxx
  cmUtilitySourceCommand.h
  cmVariableRequiresCommand.cxx
  cmVariableRequiresCommand.h
  cmVariableWatchCommand.cxx
  cmVariableWatchCommand.h
  cmWhileCommand.cxx
  cmWhileCommand.h
  cmWriteFileCommand.cxx
  cmWriteFileCommand.h

  cm_get_date.h
  cm_get_date.c
  cm_utf8.h
  cm_utf8.c
  cm_codecvt.hxx
  cm_codecvt.cxx

  cmDuration.h
  cmDuration.cxx

  bindexplib.cxx
  )

SET_PROPERTY(SOURCE cmProcessOutput.cxx cmWindowsRegistry.cxx APPEND PROPERTY COMPILE_DEFINITIONS
  KWSYS_ENCODING_DEFAULT_CODEPAGE=${KWSYS_ENCODING_DEFAULT_CODEPAGE})

# Xcode only works on Apple
if(APPLE)
  set(SRCS ${SRCS}
    cmXCodeObject.cxx
    cmXCode21Object.cxx
    cmXCodeScheme.cxx
    cmGlobalXCodeGenerator.cxx
    cmGlobalXCodeGenerator.h
    cmLocalXCodeGenerator.cxx
    cmLocalXCodeGenerator.h)
endif()


if (WIN32)
  set(SRCS ${SRCS}
    cmCallVisualStudioMacro.cxx
    cmCallVisualStudioMacro.h
    )

  if(NOT UNIX)
    set(SRCS ${SRCS}
      cmGlobalBorlandMakefileGenerator.cxx
      cmGlobalBorlandMakefileGenerator.h
      cmGlobalMSYSMakefileGenerator.cxx
      cmGlobalMinGWMakefileGenerator.cxx
      cmGlobalNMakeMakefileGenerator.cxx
      cmGlobalNMakeMakefileGenerator.h
      cmGlobalJOMMakefileGenerator.cxx
      cmGlobalJOMMakefileGenerator.h
      cmGlobalVisualStudio71Generator.cxx
      cmGlobalVisualStudio71Generator.h
      cmGlobalVisualStudio7Generator.cxx
      cmGlobalVisualStudio7Generator.h
      cmGlobalVisualStudio8Generator.cxx
      cmGlobalVisualStudio8Generator.h
      cmGlobalVisualStudio9Generator.cxx
      cmGlobalVisualStudio9Generator.h
      cmVisualStudioGeneratorOptions.h
      cmVisualStudioGeneratorOptions.cxx
      cmVsProjectType.h
      cmVisualStudio10TargetGenerator.h
      cmVisualStudio10TargetGenerator.cxx
      cmLocalVisualStudio10Generator.cxx
      cmLocalVisualStudio10Generator.h
      cmGlobalVisualStudio10Generator.h
      cmGlobalVisualStudio10Generator.cxx
      cmGlobalVisualStudio11Generator.h
      cmGlobalVisualStudio11Generator.cxx
      cmGlobalVisualStudio12Generator.h
      cmGlobalVisualStudio12Generator.cxx
      cmGlobalVisualStudio14Generator.h
      cmGlobalVisualStudio14Generator.cxx
      cmGlobalVisualStudioGenerator.cxx
      cmGlobalVisualStudioGenerator.h
      cmGlobalVisualStudioVersionedGenerator.h
      cmGlobalVisualStudioVersionedGenerator.cxx
      cmIDEFlagTable.h
      cmIDEOptions.cxx
      cmIDEOptions.h
      cmLocalVisualStudio7Generator.cxx
      cmLocalVisualStudio7Generator.h
      cmLocalVisualStudioGenerator.cxx
      cmLocalVisualStudioGenerator.h
      cmVisualStudioSlnData.h
      cmVisualStudioSlnData.cxx
      cmVisualStudioSlnParser.h
      cmVisualStudioSlnParser.cxx
      cmVisualStudioWCEPlatformParser.h
      cmVisualStudioWCEPlatformParser.cxx
      cmVSSetupHelper.cxx
      cmVSSetupHelper.h
      )

    # Add a manifest file to executables on Windows to allow for
    # GetVersion to work properly on Windows 8 and above.
    set(MANIFEST_FILE ${CMAKE_CURRENT_SOURCE_DIR}/cmake.version.manifest)
  endif()
endif ()

# Watcom support
if(WIN32 OR CMAKE_SYSTEM_NAME STREQUAL "Linux" OR CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  set_property(SOURCE cmake.cxx APPEND PROPERTY COMPILE_DEFINITIONS CMAKE_USE_WMAKE)
  list(APPEND SRCS
    cmGlobalWatcomWMakeGenerator.cxx
    cmGlobalWatcomWMakeGenerator.h
    )
endif()

# GHS support
# Works only for windows and linux
if(WIN32 OR CMAKE_SYSTEM_NAME STREQUAL "Linux")
  set(SRCS ${SRCS}
      cmGlobalGhsMultiGenerator.cxx
      cmGlobalGhsMultiGenerator.h
      cmLocalGhsMultiGenerator.cxx
      cmLocalGhsMultiGenerator.h
      cmGhsMultiTargetGenerator.cxx
      cmGhsMultiTargetGenerator.h
      cmGhsMultiGpj.cxx
      cmGhsMultiGpj.h
    )
endif()


# Ninja support
set(SRCS ${SRCS}
  cmScanDepFormat.cxx
  cmGlobalNinjaGenerator.cxx
  cmGlobalNinjaGenerator.h
  cmNinjaTypes.h
  cmLocalNinjaGenerator.cxx
  cmLocalNinjaGenerator.h
  cmNinjaTargetGenerator.cxx
  cmNinjaTargetGenerator.h
  cmNinjaNormalTargetGenerator.cxx
  cmNinjaNormalTargetGenerator.h
  cmNinjaUtilityTargetGenerator.cxx
  cmNinjaUtilityTargetGenerator.h
  cmNinjaLinkLineComputer.cxx
  cmNinjaLinkLineComputer.h
  cmNinjaLinkLineDeviceComputer.cxx
  cmNinjaLinkLineDeviceComputer.h
  )

# Temporary variable for tools targets
set(_tools)

if(WIN32 AND NOT CYGWIN)
  set_source_files_properties(cmcldeps.cxx PROPERTIES COMPILE_DEFINITIONS _WIN32_WINNT=0x0501)
  add_executable(cmcldeps cmcldeps.cxx ${MANIFEST_FILE})
  list(APPEND _tools cmcldeps)
  target_link_libraries(cmcldeps CMakeLib)
endif()

foreach(v CURL_CA_BUNDLE CURL_CA_PATH)
  if(${v})
    set_property(SOURCE cmCurl.cxx APPEND PROPERTY COMPILE_DEFINITIONS ${v}="${${v}}")
  endif()
endforeach()

foreach(check
    STAT_HAS_ST_MTIM
    STAT_HAS_ST_MTIMESPEC
    )
  if(KWSYS_CXX_${check}_COMPILED) # abuse KWSys check cache entry
    set(CMake_${check} 1)
  else()
    set(CMake_${check} 0)
  endif()
  set_property(SOURCE cmFileTime.cxx APPEND PROPERTY
    COMPILE_DEFINITIONS CMake_${check}=${CMake_${check}})
endforeach()

# create a library used by the command line and the GUI
add_library(CMakeLib ${SRCS})
target_link_libraries(CMakeLib cmsys
  ${CMAKE_STD_LIBRARY}
  ${CMAKE_EXPAT_LIBRARIES} ${CMAKE_ZLIB_LIBRARIES}
  ${CMAKE_TAR_LIBRARIES}
  ${CMAKE_CURL_LIBRARIES}
  ${CMAKE_JSONCPP_LIBRARIES}
  ${CMAKE_LIBUV_LIBRARIES}
  ${CMAKE_LIBRHASH_LIBRARIES}
  ${CMake_KWIML_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT}
  )

if(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND CMAKE_SYSTEM_PROCESSOR MATCHES "sparc")
  # the atomic instructions are implemented using libatomic on some platforms,
  # so linking to that may be required
  check_library_exists(atomic __atomic_fetch_add_4 "" LIBATOMIC_NEEDED)
  if(LIBATOMIC_NEEDED)
    target_link_libraries(CMakeLib atomic)
  endif()
endif()

# On Apple we need CoreFoundation and CoreServices
if(APPLE)
  target_link_libraries(CMakeLib "-framework CoreFoundation")
  target_link_libraries(CMakeLib "-framework CoreServices")
endif()

if(WIN32 AND NOT UNIX)
  # We need the rpcrt4 library on Windows.
  # We need the crypt32 library on Windows for crypto/cert APIs.
  target_link_libraries(CMakeLib rpcrt4 crypt32)
endif()

target_compile_definitions(CMakeLib PUBLIC ${CLANG_TIDY_DEFINITIONS})

#
# CTestLib
#
include_directories(
  "${CMake_SOURCE_DIR}/Source/CTest"
  ${CMAKE_CURL_INCLUDES}
  )
#
# Sources for CTestLib
#
set(CTEST_SRCS cmCTest.cxx
  CTest/cmProcess.cxx
  CTest/cmCTestBinPacker.cxx
  CTest/cmCTestBuildAndTestHandler.cxx
  CTest/cmCTestBuildCommand.cxx
  CTest/cmCTestBuildHandler.cxx
  CTest/cmCTestConfigureCommand.cxx
  CTest/cmCTestConfigureHandler.cxx
  CTest/cmCTestCoverageCommand.cxx
  CTest/cmCTestCoverageHandler.cxx
  CTest/cmCTestCurl.cxx
  CTest/cmParseMumpsCoverage.cxx
  CTest/cmParseCacheCoverage.cxx
  CTest/cmParseGTMCoverage.cxx
  CTest/cmParseJacocoCoverage.cxx
  CTest/cmParseBlanketJSCoverage.cxx
  CTest/cmParsePHPCoverage.cxx
  CTest/cmParseCoberturaCoverage.cxx
  CTest/cmParseDelphiCoverage.cxx
  CTest/cmCTestEmptyBinaryDirectoryCommand.cxx
  CTest/cmCTestGenericHandler.cxx
  CTest/cmCTestHandlerCommand.cxx
  CTest/cmCTestResourceAllocator.cxx
  CTest/cmCTestResourceSpec.cxx
  CTest/cmCTestLaunch.cxx
  CTest/cmCTestLaunchReporter.cxx
  CTest/cmCTestMemCheckCommand.cxx
  CTest/cmCTestMemCheckHandler.cxx
  CTest/cmCTestMultiProcessHandler.cxx
  CTest/cmCTestReadCustomFilesCommand.cxx
  CTest/cmCTestResourceGroupsLexerHelper.cxx
  CTest/cmCTestRunScriptCommand.cxx
  CTest/cmCTestRunTest.cxx
  CTest/cmCTestScriptHandler.cxx
  CTest/cmCTestSleepCommand.cxx
  CTest/cmCTestStartCommand.cxx
  CTest/cmCTestSubmitCommand.cxx
  CTest/cmCTestSubmitHandler.cxx
  CTest/cmCTestTestCommand.cxx
  CTest/cmCTestTestHandler.cxx
  CTest/cmCTestTestMeasurementXMLParser.cxx
  CTest/cmCTestUpdateCommand.cxx
  CTest/cmCTestUpdateHandler.cxx
  CTest/cmCTestUploadCommand.cxx
  CTest/cmCTestUploadHandler.cxx

  CTest/cmCTestVC.cxx
  CTest/cmCTestVC.h
  CTest/cmCTestGlobalVC.cxx
  CTest/cmCTestGlobalVC.h
  CTest/cmCTestCVS.cxx
  CTest/cmCTestCVS.h
  CTest/cmCTestSVN.cxx
  CTest/cmCTestSVN.h
  CTest/cmCTestBZR.cxx
  CTest/cmCTestBZR.h
  CTest/cmCTestGIT.cxx
  CTest/cmCTestGIT.h
  CTest/cmCTestHG.cxx
  CTest/cmCTestHG.h
  CTest/cmCTestP4.cxx
  CTest/cmCTestP4.h

  LexerParser/cmCTestResourceGroupsLexer.cxx
  LexerParser/cmCTestResourceGroupsLexer.h
  LexerParser/cmCTestResourceGroupsLexer.in.l
  )

# Build CTestLib
add_library(CTestLib ${CTEST_SRCS})
target_link_libraries(CTestLib CMakeLib ${CMAKE_CURL_LIBRARIES})

#
# CPack
#
include_directories(
  "${CMake_SOURCE_DIR}/Source/CPack"
  )
#
# Sources for CPack
#
set(CPACK_SRCS
  CPack/cmCPackArchiveGenerator.cxx
  CPack/cmCPackComponentGroup.cxx
  CPack/cmCPackDebGenerator.cxx
  CPack/cmCPackExternalGenerator.cxx
  CPack/cmCPackGeneratorFactory.cxx
  CPack/cmCPackGenerator.cxx
  CPack/cmCPackLog.cxx
  CPack/cmCPackNSISGenerator.cxx
  CPack/cmCPackNuGetGenerator.cxx
  CPack/cmCPackSTGZGenerator.cxx
  )
# CPack IFW generator
set(CPACK_SRCS ${CPACK_SRCS}
  CPack/IFW/cmCPackIFWCommon.cxx
  CPack/IFW/cmCPackIFWCommon.h
  CPack/IFW/cmCPackIFWGenerator.cxx
  CPack/IFW/cmCPackIFWGenerator.h
  CPack/IFW/cmCPackIFWInstaller.cxx
  CPack/IFW/cmCPackIFWInstaller.h
  CPack/IFW/cmCPackIFWPackage.cxx
  CPack/IFW/cmCPackIFWPackage.h
  CPack/IFW/cmCPackIFWRepository.cxx
  CPack/IFW/cmCPackIFWRepository.h
  )

if(CYGWIN)
  set(CPACK_SRCS ${CPACK_SRCS}
    CPack/cmCPackCygwinBinaryGenerator.cxx
    CPack/cmCPackCygwinSourceGenerator.cxx
    )
endif()

option(CPACK_ENABLE_FREEBSD_PKG "Add FreeBSD pkg(8) generator to CPack." OFF)

if(UNIX)
  set(CPACK_SRCS ${CPACK_SRCS}
    CPack/cmCPackRPMGenerator.cxx
    )

  # Optionally, try to use pkg(8)
  if(CPACK_ENABLE_FREEBSD_PKG)
    # On UNIX, you may find FreeBSD's pkg(8) and attendant
    # library -- it can be used on FreeBSD, Dragonfly, NetBSD,
    # OpenBSD and also Linux and OSX. Look for the header and
    # the library; it's a warning on FreeBSD if they're not
    # found, and informational on other platforms.
    find_path(FREEBSD_PKG_INCLUDE_DIRS "pkg.h")
    if(FREEBSD_PKG_INCLUDE_DIRS)
      find_library(FREEBSD_PKG_LIBRARIES
        pkg
        DOC "FreeBSD pkg(8) library")
      if(FREEBSD_PKG_LIBRARIES)
        set(CPACK_SRCS ${CPACK_SRCS}
          CPack/cmCPackFreeBSDGenerator.cxx
          )
      endif()
    endif()

    if (NOT FREEBSD_PKG_INCLUDE_DIRS OR NOT FREEBSD_PKG_LIBRARIES)
      message(FATAL_ERROR "CPack needs libpkg(3) to produce FreeBSD packages natively.")
    endif()
  else()
    set(FREEBSD_PKG_INCLUDE_DIRS NOTFOUND)
    set(FREEBSD_PKG_LIBRARIES NOTFOUND)
  endif()
endif()

if(CYGWIN)
  find_package(LibUUID)
endif()
if(WIN32 OR (CYGWIN AND LibUUID_FOUND))
  set(CPACK_SRCS ${CPACK_SRCS}
    CPack/WiX/cmCMakeToWixPath.cxx
    CPack/WiX/cmCMakeToWixPath.h
    CPack/WiX/cmCPackWIXGenerator.cxx
    CPack/WiX/cmCPackWIXGenerator.h
    CPack/WiX/cmWIXAccessControlList.cxx
    CPack/WiX/cmWIXAccessControlList.h
    CPack/WiX/cmWIXDirectoriesSourceWriter.cxx
    CPack/WiX/cmWIXDirectoriesSourceWriter.h
    CPack/WiX/cmWIXFeaturesSourceWriter.cxx
    CPack/WiX/cmWIXFeaturesSourceWriter.h
    CPack/WiX/cmWIXFilesSourceWriter.cxx
    CPack/WiX/cmWIXFilesSourceWriter.h
    CPack/WiX/cmWIXPatch.cxx
    CPack/WiX/cmWIXPatch.h
    CPack/WiX/cmWIXPatchParser.cxx
    CPack/WiX/cmWIXPatchParser.h
    CPack/WiX/cmWIXRichTextFormatWriter.cxx
    CPack/WiX/cmWIXRichTextFormatWriter.h
    CPack/WiX/cmWIXShortcut.cxx
    CPack/WiX/cmWIXShortcut.h
    CPack/WiX/cmWIXSourceWriter.cxx
    CPack/WiX/cmWIXSourceWriter.h
    )
endif()

if(APPLE)
  set(CPACK_SRCS ${CPACK_SRCS}
    CPack/cmCPackBundleGenerator.cxx
    CPack/cmCPackDragNDropGenerator.cxx
    CPack/cmCPackPKGGenerator.cxx
    CPack/cmCPackProductBuildGenerator.cxx
    )
endif()

# Build CPackLib
add_library(CPackLib ${CPACK_SRCS})
target_link_libraries(CPackLib CMakeLib)
if(APPLE)
  # Some compilers produce errors in the CoreServices framework headers.
  # Ideally such errors should be fixed by either the compiler vendor
  # or the framework source, but we try to workaround it and build anyway.
  # If it does not work, build with reduced functionality and warn.
  check_include_file("CoreServices/CoreServices.h" HAVE_CoreServices)
  if(HAVE_CoreServices)
    set_property(SOURCE CPack/cmCPackDragNDropGenerator.cxx PROPERTY COMPILE_DEFINITIONS HAVE_CoreServices)
    target_link_libraries(CPackLib "-framework CoreServices")
  else()
    message(WARNING "This compiler does not appear to support\n"
      "  #include <CoreServices/CoreServices.h>\n"
      "Some CPack functionality may be limited.\n"
      "See CMakeFiles/CMakeError.log for details of the failure.")
  endif()
endif()
if(CYGWIN AND LibUUID_FOUND)
  target_link_libraries(CPackLib ${LibUUID_LIBRARIES})
  include_directories(CPackLib ${LibUUID_INCLUDE_DIRS})
  set_property(SOURCE CPack/cmCPackGeneratorFactory.cxx PROPERTY COMPILE_DEFINITIONS HAVE_LIBUUID)
endif()
if(CPACK_ENABLE_FREEBSD_PKG AND FREEBSD_PKG_INCLUDE_DIRS AND FREEBSD_PKG_LIBRARIES)
  target_link_libraries(CPackLib ${FREEBSD_PKG_LIBRARIES})
  include_directories(${FREEBSD_PKG_INCLUDE_DIRS})
  add_definitions(-DHAVE_FREEBSD_PKG)
endif()

# Build CMake executable
add_executable(cmake cmakemain.cxx cmcmd.cxx cmcmd.h ${MANIFEST_FILE})
list(APPEND _tools cmake)
target_link_libraries(cmake CMakeLib)

# Build CTest executable
add_executable(ctest ctest.cxx ${MANIFEST_FILE})
list(APPEND _tools ctest)
target_link_libraries(ctest CTestLib)

# Build CPack executable
add_executable(cpack CPack/cpack.cxx ${MANIFEST_FILE})
list(APPEND _tools cpack)
target_link_libraries(cpack CPackLib)

# Curses GUI
if(BUILD_CursesDialog)
  add_subdirectory(CursesDialog)
endif()

# Qt GUI
option(BUILD_QtDialog "Build Qt dialog for CMake" FALSE)
if(BUILD_QtDialog)
  add_subdirectory(QtDialog)
endif()

include (${CMake_BINARY_DIR}/Source/LocalUserOptions.cmake OPTIONAL)
include (${CMake_SOURCE_DIR}/Source/LocalUserOptions.cmake OPTIONAL)

if(WIN32)
  # Compute the binary version that appears in the RC file. Version
  # components in the RC file are 16-bit integers so we may have to
  # split the patch component.
  if(CMake_VERSION_PATCH MATCHES "^([0-9]+)([0-9][0-9][0-9][0-9])$")
    set(CMake_RCVERSION_YEAR "${CMAKE_MATCH_1}")
    set(CMake_RCVERSION_MONTH_DAY "${CMAKE_MATCH_2}")
    string(REGEX REPLACE "^0+" "" CMake_RCVERSION_MONTH_DAY "${CMake_RCVERSION_MONTH_DAY}")
    set(CMake_RCVERSION ${CMake_VERSION_MAJOR},${CMake_VERSION_MINOR},${CMake_RCVERSION_YEAR},${CMake_RCVERSION_MONTH_DAY})
    unset(CMake_RCVERSION_MONTH_DAY)
    unset(CMake_RCVERSION_YEAR)
  else()
    set(CMake_RCVERSION ${CMake_VERSION_MAJOR},${CMake_VERSION_MINOR},${CMake_VERSION_PATCH},0)
  endif()
  set(CMake_RCVERSION_STR ${CMake_VERSION})

  # Add Windows executable version information.
  configure_file("CMakeVersion.rc.in" "CMakeVersion.rc" @ONLY)

  # We use a separate object library for this to work around a limitation of
  # MinGW's windres tool with spaces in the path to the include directories.
  add_library(CMakeVersion OBJECT "${CMAKE_CURRENT_BINARY_DIR}/CMakeVersion.rc")
  set_property(TARGET CMakeVersion PROPERTY INCLUDE_DIRECTORIES "")
  foreach(_tool ${_tools})
    target_sources(${_tool} PRIVATE $<TARGET_OBJECTS:CMakeVersion>)
  endforeach()
endif()

if(CMake_JOB_POOL_LINK_BIN)
  set_property(TARGET ${_tools} PROPERTY JOB_POOL_LINK "link-bin")
  set_property(GLOBAL APPEND PROPERTY JOB_POOLS "link-bin=${CMake_JOB_POOL_LINK_BIN}")
endif()

# Install tools

foreach(_tool ${_tools})
  CMake_OPTIONAL_COMPONENT(${_tool})
  install(TARGETS ${_tool} DESTINATION ${CMAKE_BIN_DIR} ${COMPONENT})
endforeach()

install(FILES cmCPluginAPI.h DESTINATION ${CMAKE_DATA_DIR}/include)

# Unset temporary variables
unset(_tools)
