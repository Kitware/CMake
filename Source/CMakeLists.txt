#=============================================================================
# CMake - Cross Platform Makefile Generator
# Copyright 2000-2009 Kitware, Inc., Insight Software Consortium
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
include(CheckIncludeFile)
# Check if we can build support for ELF parsing.
CHECK_INCLUDE_FILE("elf.h" HAVE_ELF_H)
if(HAVE_ELF_H)
  set(CMAKE_USE_ELF_PARSER 1)
else()
  set(CMAKE_USE_ELF_PARSER)
endif()

set(EXECUTABLE_OUTPUT_PATH ${CMake_BIN_DIR})

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

# add the include path to find the .h
include_directories(
  "${CMake_BINARY_DIR}/Source"
  "${CMake_SOURCE_DIR}/Source"
  ${CMAKE_ZLIB_INCLUDES}
  ${CMAKE_EXPAT_INCLUDES}
  ${CMAKE_TAR_INCLUDES}
  ${CMAKE_COMPRESS_INCLUDES}
  )

# let cmake know it is supposed to use it
add_definitions(-DCMAKE_BUILD_WITH_CMAKE)

option(CMAKE_REGENERATE_YACCLEX
  "Regenerate YACC and LEXX files" OFF)
mark_as_advanced(CMAKE_REGENERATE_YACCLEX)
if(CMAKE_REGENERATE_YACCLEX)
  set(parsersLexers cmDependsFortran cmCommandArgument cmExpr)
  find_program(YACC_EXECUTABLE
    NAMES yacc bison
    PATHS /usr/bin
    DOC "Yacc or Bison executable")
  find_program(FLEX_EXECUTABLE
    NAMES flex
    PATHS /usr/bin
    DOC "Flex executable")
  mark_as_advanced(YACC_EXECUTABLE FLEX_EXECUTABLE)
  if(YACC_EXECUTABLE)
    set(BISON_FLAGS)
    if(YACC_EXECUTABLE MATCHES "bison")
      set(BISON_FLAGS "--yacc")
    endif()
    set(yacc_files)
    foreach(name ${parsersLexers})
      set(src "${CMAKE_CURRENT_SOURCE_DIR}/${name}Parser.y")
      set(dst "${CMAKE_CURRENT_BINARY_DIR}/${name}Parser.cxx")
      set(hdr "${CMAKE_CURRENT_BINARY_DIR}/${name}ParserTokens.h")
      add_custom_command(
        OUTPUT "${dst}"
        DEPENDS "${src}"
        COMMAND
        ${YACC_EXECUTABLE}
        --name-prefix=${name}_yy --defines=${hdr} -o${dst} ${src}
        )
      set(yacc_files ${yacc_files} "${dst}")
    endforeach()
    add_custom_target(RerunYacc DEPENDS ${yacc_files})
  endif()
  if(FLEX_EXECUTABLE)
    set(lex_files)
    foreach(name ${parsersLexers})
      set(src "${CMAKE_CURRENT_SOURCE_DIR}/${name}Lexer.in.l")
      set(dst "${CMAKE_CURRENT_BINARY_DIR}/${name}Lexer.cxx")
      set(hdr "${CMAKE_CURRENT_BINARY_DIR}/${name}Lexer.h")
      add_custom_command(
        OUTPUT "${dst}"
        DEPENDS "${src}"
        COMMAND
        ${FLEX_EXECUTABLE}
        --prefix=${name}_yy --header-file=${hdr} -o${dst} ${src}
        )
      set(lex_files ${lex_files} "${dst}")
    endforeach()
    add_custom_target(RerunLex DEPENDS ${lex_files})
  endif()

endif()

# Check if we can build the ELF parser.
if(CMAKE_USE_ELF_PARSER)
  set(ELF_SRCS cmELF.h cmELF.cxx)
endif()

#
# Sources for CMakeLib
#
set(SRCS
  cmStandardIncludes.cxx
  cmArchiveWrite.cxx
  cmBootstrapCommands1.cxx
  cmBootstrapCommands2.cxx
  cmCacheManager.cxx
  cmCacheManager.h
  cmCommands.cxx
  cmCommands.h
  cmCommandArgumentLexer.cxx
  cmCommandArgumentParser.cxx
  cmCommandArgumentParserHelper.cxx
  cmComputeComponentGraph.cxx
  cmComputeComponentGraph.h
  cmComputeLinkDepends.cxx
  cmComputeLinkDepends.h
  cmComputeLinkInformation.cxx
  cmComputeLinkInformation.h
  cmComputeTargetDepends.h
  cmComputeTargetDepends.cxx
  cmCryptoHash.cxx
  cmCryptoHash.h
  cmCustomCommand.cxx
  cmCustomCommand.h
  cmCustomCommandGenerator.cxx
  cmCustomCommandGenerator.h
  cmDefinitions.cxx
  cmDefinitions.h
  cmDepends.cxx
  cmDepends.h
  cmDependsC.cxx
  cmDependsC.h
  cmDependsFortran.cxx
  cmDependsFortran.h
  cmDependsFortranLexer.cxx
  cmDependsFortranParser.cxx
  cmDependsFortranParser.h
  cmDependsJava.cxx
  cmDependsJava.h
  cmDependsJavaLexer.cxx
  cmDependsJavaParser.cxx
  cmDependsJavaParserHelper.cxx
  cmDependsJavaParserHelper.h
  cmDocumentation.cxx
  cmDocumentationFormatter.cxx
  cmDocumentationFormatterHTML.cxx
  cmDocumentationFormatterDocbook.cxx
  cmDocumentationFormatterMan.cxx
  cmDocumentationFormatterText.cxx
  cmDocumentationFormatterUsage.cxx
  cmDocumentationSection.cxx
  cmDocumentCompileDefinitions.h
  cmDocumentGeneratorExpressions.h
  cmDocumentLocationUndefined.h
  cmDocumentVariables.cxx
  cmDynamicLoader.cxx
  cmDynamicLoader.h
  ${ELF_SRCS}
  cmExprLexer.cxx
  cmExprParser.cxx
  cmExprParserHelper.cxx
  cmExportBuildFileGenerator.h
  cmExportBuildFileGenerator.cxx
  cmExportFileGenerator.h
  cmExportFileGenerator.cxx
  cmExportInstallFileGenerator.h
  cmExportInstallFileGenerator.cxx
  cmExportTryCompileFileGenerator.h
  cmExportTryCompileFileGenerator.cxx
  cmExportSet.h
  cmExportSet.cxx
  cmExportSetMap.h
  cmExportSetMap.cxx
  cmExtraCodeBlocksGenerator.cxx
  cmExtraCodeBlocksGenerator.h
  cmExtraEclipseCDT4Generator.cxx
  cmExtraEclipseCDT4Generator.h
  cmExtraSublimeTextGenerator.cxx
  cmExtraSublimeTextGenerator.h
  cmFileTimeComparison.cxx
  cmFileTimeComparison.h
  cmGeneratedFileStream.cxx
  cmGeneratorExpressionDAGChecker.cxx
  cmGeneratorExpressionDAGChecker.h
  cmGeneratorExpressionEvaluator.cxx
  cmGeneratorExpressionEvaluator.h
  cmGeneratorExpressionLexer.cxx
  cmGeneratorExpressionLexer.h
  cmGeneratorExpressionParser.cxx
  cmGeneratorExpressionParser.h
  cmGeneratorExpression.cxx
  cmGeneratorExpression.h
  cmGeneratorTarget.cxx
  cmGeneratorTarget.h
  cmGlobalGenerator.cxx
  cmGlobalGenerator.h
  cmGlobalGeneratorFactory.h
  cmGlobalUnixMakefileGenerator3.cxx
  cmGlobalUnixMakefileGenerator3.h
  cmGraphAdjacencyList.h
  cmGraphVizWriter.cxx
  cmGraphVizWriter.h
  cmInstallGenerator.h
  cmInstallGenerator.cxx
  cmInstallExportGenerator.cxx
  cmInstallFilesGenerator.h
  cmInstallFilesGenerator.cxx
  cmInstallScriptGenerator.h
  cmInstallScriptGenerator.cxx
  cmInstallTargetGenerator.h
  cmInstallTargetGenerator.cxx
  cmInstallDirectoryGenerator.h
  cmInstallDirectoryGenerator.cxx
  cmListFileCache.cxx
  cmListFileCache.h
  cmListFileLexer.c
  cmLocalGenerator.cxx
  cmLocalGenerator.h
  cmLocalUnixMakefileGenerator3.cxx
  cmMakeDepend.cxx
  cmMakeDepend.h
  cmMakefile.cxx
  cmMakefile.h
  cmMakefileTargetGenerator.cxx
  cmMakefileExecutableTargetGenerator.cxx
  cmMakefileLibraryTargetGenerator.cxx
  cmMakefileUtilityTargetGenerator.cxx
  cmOSXBundleGenerator.cxx
  cmOSXBundleGenerator.h
  cmNewLineStyle.h
  cmNewLineStyle.cxx
  cmOrderDirectories.cxx
  cmOrderDirectories.h
  cmPolicies.h
  cmPolicies.cxx
  cmProcessTools.cxx
  cmProcessTools.h
  cmProperty.cxx
  cmProperty.h
  cmPropertyDefinition.cxx
  cmPropertyDefinition.h
  cmPropertyDefinitionMap.cxx
  cmPropertyDefinitionMap.h
  cmPropertyMap.cxx
  cmPropertyMap.h
  cmQtAutomoc.cxx
  cmQtAutomoc.h
  cmScriptGenerator.h
  cmScriptGenerator.cxx
  cmSourceFile.cxx
  cmSourceFile.h
  cmSourceFileLocation.cxx
  cmSourceFileLocation.h
  cmSourceGroup.cxx
  cmSourceGroup.h
  cmSystemTools.cxx
  cmSystemTools.h
  cmTarget.cxx
  cmTarget.h
  cmTargetExport.h
  cmTest.cxx
  cmTest.h
  cmTestGenerator.cxx
  cmTestGenerator.h
  cmVariableWatch.cxx
  cmVariableWatch.h
  cmVersion.cxx
  cmVersion.h
  cmXMLParser.cxx
  cmXMLParser.h
  cmXMLSafe.cxx
  cmXMLSafe.h
  cmake.cxx
  cmake.h
  cmakewizard.cxx
  cmakewizard.h

  cm_sha2.h
  cm_sha2.c
  cm_utf8.h
  cm_utf8.c
  )

# Kdevelop only works on UNIX and not windows
if(UNIX)
  set(SRCS ${SRCS} cmGlobalKdevelopGenerator.cxx)
endif()

# Xcode only works on Apple
if(APPLE)
  set(SRCS ${SRCS}
    cmXCodeObject.cxx
    cmXCode21Object.cxx
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
      cmGlobalVisualStudio6Generator.cxx
      cmGlobalVisualStudio6Generator.h
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
      cmVisualStudio10TargetGenerator.h
      cmVisualStudio10TargetGenerator.cxx
      cmLocalVisualStudio10Generator.cxx
      cmLocalVisualStudio10Generator.h
      cmGlobalVisualStudio10Generator.h
      cmGlobalVisualStudio10Generator.cxx
      cmGlobalVisualStudio11Generator.h
      cmGlobalVisualStudio11Generator.cxx
      cmGlobalVisualStudioGenerator.cxx
      cmGlobalVisualStudioGenerator.h
      cmGlobalWatcomWMakeGenerator.cxx
      cmIDEFlagTable.h
      cmIDEOptions.cxx
      cmIDEOptions.h
      cmLocalVisualStudio6Generator.cxx
      cmLocalVisualStudio6Generator.h
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
      cmWin32ProcessExecution.cxx
      cmWin32ProcessExecution.h
      )
  endif()
endif ()

# Ninja support
set(SRCS ${SRCS}
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
  )
if(WIN32 AND NOT CYGWIN AND NOT BORLAND)
  set_source_files_properties(cmcldeps.cxx PROPERTIES COMPILE_DEFINITIONS _WIN32_WINNT=0x0501)
  add_executable(cmcldeps cmcldeps.cxx)
  target_link_libraries(cmcldeps CMakeLib)
  install_targets(/bin cmcldeps)
endif()

# create a library used by the command line and the GUI
add_library(CMakeLib ${SRCS})
target_link_libraries(CMakeLib cmsys
  ${CMAKE_EXPAT_LIBRARIES} ${CMAKE_ZLIB_LIBRARIES}
  ${CMAKE_TAR_LIBRARIES} ${CMAKE_COMPRESS_LIBRARIES}
  ${CMAKE_CURL_LIBRARIES} )

# On Apple we need CoreFoundation
if(APPLE)
  target_link_libraries(CMakeLib "-framework CoreFoundation")
endif()

# On some platforms we need the rpcrt4 library for the VS 7 generators.
if(CMAKE_BUILD_ON_VISUAL_STUDIO OR MINGW)
  target_link_libraries(CMakeLib rpcrt4)
endif()

#
# CTestLib
#
include_directories(
  "${CMake_SOURCE_DIR}/Source/CTest"
  ${CMAKE_XMLRPC_INCLUDES}
  ${CMAKE_CURL_INCLUDES}
  )
#
# Sources for CTestLib
#
set(CTEST_SRCS cmCTest.cxx
  CTest/cmProcess.cxx
  CTest/cmCTestBatchTestHandler.cxx
  CTest/cmCTestBuildAndTestHandler.cxx
  CTest/cmCTestBuildCommand.cxx
  CTest/cmCTestBuildHandler.cxx
  CTest/cmCTestConfigureCommand.cxx
  CTest/cmCTestConfigureHandler.cxx
  CTest/cmCTestCoverageCommand.cxx
  CTest/cmCTestCoverageHandler.cxx
  CTest/cmParseMumpsCoverage.cxx
  CTest/cmParseCacheCoverage.cxx
  CTest/cmParseGTMCoverage.cxx
  CTest/cmParsePHPCoverage.cxx
  CTest/cmCTestEmptyBinaryDirectoryCommand.cxx
  CTest/cmCTestGenericHandler.cxx
  CTest/cmCTestHandlerCommand.cxx
  CTest/cmCTestLaunch.cxx
  CTest/cmCTestMemCheckCommand.cxx
  CTest/cmCTestMemCheckHandler.cxx
  CTest/cmCTestMultiProcessHandler.cxx
  CTest/cmCTestReadCustomFilesCommand.cxx
  CTest/cmCTestRunScriptCommand.cxx
  CTest/cmCTestRunTest.cxx
  CTest/cmCTestScriptHandler.cxx
  CTest/cmCTestSleepCommand.cxx
  CTest/cmCTestStartCommand.cxx
  CTest/cmCTestSubmitCommand.cxx
  CTest/cmCTestSubmitHandler.cxx
  CTest/cmCTestTestCommand.cxx
  CTest/cmCTestTestHandler.cxx
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
  )

# Build CTestLib
add_library(CTestLib ${CTEST_SRCS})
target_link_libraries(CTestLib CMakeLib ${CMAKE_CURL_LIBRARIES} ${CMAKE_XMLRPC_LIBRARIES})

#
# Sources for CPack
#
set(CPACK_SRCS
  CPack/cmCPackArchiveGenerator.cxx
  CPack/cmCPackComponentGroup.cxx
  CPack/cmCPackGeneratorFactory.cxx
  CPack/cmCPackGenerator.cxx
  CPack/cmCPackLog.cxx
  CPack/cmCPackNSISGenerator.cxx
  CPack/cmCPackSTGZGenerator.cxx
  CPack/cmCPackTGZGenerator.cxx
  CPack/cmCPackTarBZip2Generator.cxx
  CPack/cmCPackTarCompressGenerator.cxx
  CPack/cmCPackZIPGenerator.cxx
  CPack/cmCPackDocumentVariables.cxx
  CPack/cmCPackDocumentMacros.cxx
  )

if(CYGWIN)
  set(CPACK_SRCS ${CPACK_SRCS}
    CPack/cmCPackCygwinBinaryGenerator.cxx
    CPack/cmCPackCygwinSourceGenerator.cxx
    )
endif()

if(UNIX)
  set(CPACK_SRCS ${CPACK_SRCS}
    CPack/cmCPackDebGenerator.cxx
    CPack/cmCPackRPMGenerator.cxx
    )
endif()

if(WIN32)
  set(CPACK_SRCS ${CPACK_SRCS}
    CPack/WiX/cmCPackWIXGenerator.cxx
    CPack/WiX/cmWIXSourceWriter.cxx
    CPack/WiX/cmWIXRichTextFormatWriter.cxx
  )
endif()

if(APPLE)
  set(CPACK_SRCS ${CPACK_SRCS}
    CPack/cmCPackBundleGenerator.cxx
    CPack/cmCPackDragNDropGenerator.cxx
    CPack/cmCPackOSXX11Generator.cxx
    CPack/cmCPackPackageMakerGenerator.cxx
    )
endif()

# Build CPackLib
add_library(CPackLib ${CPACK_SRCS})
target_link_libraries(CPackLib CMakeLib)

if(APPLE)
  add_executable(cmakexbuild cmakexbuild.cxx)
  target_link_libraries(cmakexbuild CMakeLib)
  add_executable(OSXScriptLauncher
    CPack/OSXScriptLauncher.cxx)
  target_link_libraries(OSXScriptLauncher cmsys)
  target_link_libraries(OSXScriptLauncher "-framework CoreFoundation")
endif()

# Build CMake executable
add_executable(cmake cmakemain.cxx)
target_link_libraries(cmake CMakeLib)

# Build special executable for running programs on Windows 98.
# Included on any Windows (unconditional packaging required!).
if(WIN32)
  if(NOT UNIX)
    add_executable(cmw9xcom cmw9xcom.cxx)
    target_link_libraries(cmw9xcom CMakeLib)
    install_targets(/bin cmw9xcom)
  endif()
endif()

# Build CTest executable
add_executable(ctest ctest.cxx)
target_link_libraries(ctest CTestLib)

# Build CPack executable
add_executable(cpack CPack/cpack.cxx)
target_link_libraries(cpack CPackLib)

# Curses GUI
if(BUILD_CursesDialog)
  include(${CMake_SOURCE_DIR}/Source/CursesDialog/CMakeLists.txt)
endif()

# Qt GUI
option(BUILD_QtDialog "Build Qt dialog for CMake" FALSE)
if(BUILD_QtDialog)
  add_subdirectory(QtDialog)
endif()

include (${CMake_BINARY_DIR}/Source/LocalUserOptions.cmake OPTIONAL)
include (${CMake_SOURCE_DIR}/Source/LocalUserOptions.cmake OPTIONAL)

install_targets(/bin cmake)
install_targets(/bin ctest)
install_targets(/bin cpack)
if(APPLE)
  install_targets(/bin cmakexbuild)
endif()

install_files(${CMAKE_DATA_DIR}/include cmCPluginAPI.h)
