INCLUDE(CheckIncludeFile)
# Check if we can build support for ELF parsing.
CHECK_INCLUDE_FILE("elf.h" HAVE_ELF_H)
IF(HAVE_ELF_H)
  SET(CMAKE_USE_ELF_PARSER 1)
ELSE(HAVE_ELF_H)
  SET(CMAKE_USE_ELF_PARSER)
ENDIF(HAVE_ELF_H)

# configure the .h file
CONFIGURE_FILE(
  "${CMake_SOURCE_DIR}/Source/cmConfigure.cmake.h.in"
  "${CMake_BINARY_DIR}/Source/cmConfigure.h"
  )
CONFIGURE_FILE(
  "${CMake_SOURCE_DIR}/Source/CPack/cmCPackConfigure.h.in"
  "${CMake_BINARY_DIR}/Source/CPack/cmCPackConfigure.h"
  )

# add the include path to find the .h
INCLUDE_DIRECTORIES(
  "${CMake_BINARY_DIR}/Source"
  "${CMake_SOURCE_DIR}/Source"
  ${CMAKE_ZLIB_INCLUDES}
  ${CMAKE_EXPAT_INCLUDES}
  ${CMAKE_TAR_INCLUDES}
  ${CMAKE_COMPRESS_INCLUDES}
  )

# let cmake know it is supposed to use it
ADD_DEFINITIONS(-DCMAKE_BUILD_WITH_CMAKE)

OPTION(CMAKE_REGENERATE_YACCLEX
  "Regenerate YACC and LEXX files" OFF)
MARK_AS_ADVANCED(CMAKE_REGENERATE_YACCLEX)
IF(CMAKE_REGENERATE_YACCLEX)
  SET(parsersLexers cmDependsFortran cmCommandArgument cmExpr)
  FIND_PROGRAM(YACC_EXECUTABLE
    NAMES yacc bison
    PATHS /usr/bin
    DOC "Yacc or Bison executable")
  FIND_PROGRAM(FLEX_EXECUTABLE
    NAMES flex
    PATHS /usr/bin
    DOC "Flex executable")
  MARK_AS_ADVANCED(YACC_EXECUTABLE FLEX_EXECUTABLE)
  IF(YACC_EXECUTABLE)
    SET(BISON_FLAGS)
    IF(YACC_EXECUTABLE MATCHES "bison")
      SET(BISON_FLAGS "--yacc")
    ENDIF(YACC_EXECUTABLE MATCHES "bison")
    SET(yacc_files)
    FOREACH(name ${parsersLexers})
      SET(src "${CMAKE_CURRENT_SOURCE_DIR}/${name}Parser.y")
      SET(dst "${CMAKE_CURRENT_BINARY_DIR}/${name}Parser.cxx")
      SET(hdr "${CMAKE_CURRENT_BINARY_DIR}/${name}ParserTokens.h")
      ADD_CUSTOM_COMMAND(
        OUTPUT "${dst}"
        DEPENDS "${src}"
        COMMAND
        ${YACC_EXECUTABLE}
        --name-prefix=${name}_yy --defines=${hdr} -o${dst} ${src}
        )
      SET(yacc_files ${yacc_files} "${dst}")
    ENDFOREACH(name)
    ADD_CUSTOM_TARGET(RerunYacc DEPENDS ${yacc_files})
  ENDIF(YACC_EXECUTABLE)
  IF(FLEX_EXECUTABLE)
    SET(lex_files)
    FOREACH(name ${parsersLexers})
      SET(src "${CMAKE_CURRENT_SOURCE_DIR}/${name}Lexer.in.l")
      SET(dst "${CMAKE_CURRENT_BINARY_DIR}/${name}Lexer.cxx")
      SET(hdr "${CMAKE_CURRENT_BINARY_DIR}/${name}Lexer.h")
      ADD_CUSTOM_COMMAND(
        OUTPUT "${dst}"
        DEPENDS "${src}"
        COMMAND
        ${FLEX_EXECUTABLE}
        --prefix=${name}_yy --header-file=${hdr} -o${dst} ${src}
        )
      SET(lex_files ${lex_files} "${dst}")
    ENDFOREACH(name)
    ADD_CUSTOM_TARGET(RerunLex DEPENDS ${lex_files})
  ENDIF(FLEX_EXECUTABLE)

ENDIF(CMAKE_REGENERATE_YACCLEX)

# Check if we can build the ELF parser.
IF(CMAKE_USE_ELF_PARSER)
  SET(ELF_SRCS cmELF.h cmELF.cxx)
ENDIF(CMAKE_USE_ELF_PARSER)

#
# Sources for CMakeLib
#
SET(SRCS
  cmBootstrapCommands.cxx
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
  cmCustomCommand.cxx
  cmCustomCommand.h
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
  cmExtraEclipseCDT4Generator.cxx
  cmExtraEclipseCDT4Generator.h
  cmFileTimeComparison.cxx
  cmFileTimeComparison.h
  cmGeneratedFileStream.cxx
  cmGlobalGenerator.cxx
  cmGlobalGenerator.h
  cmGlobalUnixMakefileGenerator3.cxx
  cmGlobalUnixMakefileGenerator3.h
  cmGraphAdjacencyList.h
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
  cmOrderDirectories.cxx
  cmOrderDirectories.h
  cmPolicies.h
  cmPolicies.cxx
  cmProperty.cxx
  cmProperty.h
  cmPropertyDefinition.cxx
  cmPropertyDefinition.h
  cmPropertyDefinitionMap.cxx
  cmPropertyDefinitionMap.h
  cmPropertyMap.cxx
  cmPropertyMap.h
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
  cmTest.cxx
  cmTest.h
  cmVariableWatch.cxx
  cmVariableWatch.h
  cmVersion.cxx
  cmVersion.h
  cmXMLParser.cxx
  cmXMLParser.h
  cmake.cxx
  cmake.h
  cmakewizard.cxx
  cmakewizard.h
  )

# Kdevelop only works on UNIX and not windows
IF(UNIX)
  SET(SRCS ${SRCS} cmGlobalKdevelopGenerator.cxx)
ENDIF(UNIX)

IF(NOT CYGWIN)
  SET(SRCS ${SRCS} cmExtraCodeBlocksGenerator.cxx)
ENDIF(NOT CYGWIN)

# XCode only works on apple
IF(APPLE)
  SET(SRCS ${SRCS}
    cmXCodeObject.cxx
    cmXCode21Object.cxx
    cmGlobalXCodeGenerator.cxx
    cmGlobalXCodeGenerator.h
    cmGlobalXCode21Generator.cxx
    cmGlobalXCode21Generator.h
    cmLocalXCodeGenerator.cxx
    cmLocalXCodeGenerator.h)
ENDIF(APPLE)


IF (WIN32)
  SET(SRCS ${SRCS}
    cmCallVisualStudioMacro.cxx
    cmCallVisualStudioMacro.h
    )

  IF(NOT UNIX)
    SET(SRCS ${SRCS}
      cmGlobalBorlandMakefileGenerator.cxx
      cmGlobalBorlandMakefileGenerator.h
      cmGlobalMSYSMakefileGenerator.cxx
      cmGlobalMinGWMakefileGenerator.cxx
      cmGlobalNMakeMakefileGenerator.cxx
      cmGlobalNMakeMakefileGenerator.h
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
      cmGlobalVisualStudio8Win64Generator.cxx
      cmGlobalVisualStudio8Win64Generator.h
      cmGlobalVisualStudio9Win64Generator.cxx
      cmGlobalVisualStudio9Win64Generator.h
      cmGlobalVisualStudioGenerator.cxx
      cmGlobalVisualStudioGenerator.h
      cmGlobalWatcomWMakeGenerator.cxx
      cmLocalVisualStudio6Generator.cxx
      cmLocalVisualStudio6Generator.h
      cmLocalVisualStudio7Generator.cxx
      cmLocalVisualStudio7Generator.h
      cmLocalVisualStudioGenerator.cxx
      cmLocalVisualStudioGenerator.h
      cmWin32ProcessExecution.cxx
      cmWin32ProcessExecution.h
      )
  ENDIF(NOT UNIX)
ENDIF (WIN32)

# create a library used by the command line and the GUI
ADD_LIBRARY(CMakeLib ${SRCS})
TARGET_LINK_LIBRARIES(CMakeLib cmsys
  ${CMAKE_EXPAT_LIBRARIES} ${CMAKE_ZLIB_LIBRARIES}
  ${CMAKE_TAR_LIBRARIES} ${CMAKE_COMPRESS_LIBRARIES}
  ${CMAKE_CURL_LIBRARIES})

# On Apple we need Carbon
IF(APPLE)
  TARGET_LINK_LIBRARIES(CMakeLib "-framework CoreFoundation")
ENDIF(APPLE)

# On some platforms we need the rpcrt4 library for the VS 7 generators.
IF(CMAKE_BUILD_ON_VISUAL_STUDIO OR MINGW)
  TARGET_LINK_LIBRARIES(CMakeLib rpcrt4)
ENDIF(CMAKE_BUILD_ON_VISUAL_STUDIO OR MINGW)

#
# CTestLib
#
INCLUDE_DIRECTORIES(
  "${CMAKE_SOURCE_DIR}/Source/CTest"
  ${CMAKE_XMLRPC_INCLUDES}
  ${CMAKE_CURL_INCLUDES}
  )
#
# Sources for CTestLib
#
SET(CTEST_SRCS cmCTest.cxx
  CTest/cmCTestBuildAndTestHandler.cxx
  CTest/cmCTestBuildCommand.cxx
  CTest/cmCTestBuildHandler.cxx
  CTest/cmCTestConfigureCommand.cxx
  CTest/cmCTestConfigureHandler.cxx
  CTest/cmCTestCoverageCommand.cxx
  CTest/cmCTestCoverageHandler.cxx
  CTest/cmCTestEmptyBinaryDirectoryCommand.cxx
  CTest/cmCTestGenericHandler.cxx
  CTest/cmCTestHandlerCommand.cxx
  CTest/cmCTestMemCheckCommand.cxx
  CTest/cmCTestMemCheckHandler.cxx
  CTest/cmCTestReadCustomFilesCommand.cxx
  CTest/cmCTestRunScriptCommand.cxx
  CTest/cmCTestScriptHandler.cxx
  CTest/cmCTestSleepCommand.cxx
  CTest/cmCTestStartCommand.cxx
  CTest/cmCTestSubmitCommand.cxx
  CTest/cmCTestSubmitHandler.cxx
  CTest/cmCTestTestCommand.cxx
  CTest/cmCTestTestHandler.cxx
  CTest/cmCTestUpdateCommand.cxx
  CTest/cmCTestUpdateHandler.cxx
  )

# Build CTestLib
ADD_LIBRARY(CTestLib ${CTEST_SRCS})
TARGET_LINK_LIBRARIES(CTestLib CMakeLib ${CMAKE_CURL_LIBRARIES} ${CMAKE_XMLRPC_LIBRARIES})

#
# Sources for CPack
#
SET(CPACK_SRCS
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
  )

IF(WIN32 AND UNIX)
  SET(CPACK_SRCS ${CPACK_SRCS}
    CPack/cmCPackCygwinBinaryGenerator.cxx
    CPack/cmCPackCygwinSourceGenerator.cxx
    )
ENDIF(WIN32 AND UNIX)
  
IF(UNIX)
  SET(CPACK_SRCS ${CPACK_SRCS}
    CPack/cmCPackDebGenerator.cxx
    CPack/cmCPackRPMGenerator.cxx
    )
ENDIF(UNIX)

IF(APPLE)
  SET(CPACK_SRCS ${CPACK_SRCS}
    CPack/cmCPackBundleGenerator.cxx
    CPack/cmCPackOSXX11Generator.cxx
    CPack/cmCPackPackageMakerGenerator.cxx
    )
ENDIF(APPLE)

# Build CPackLib
ADD_LIBRARY(CPackLib ${CPACK_SRCS})
TARGET_LINK_LIBRARIES(CPackLib CMakeLib)

IF(APPLE)
  ADD_EXECUTABLE(cmakexbuild cmakexbuild.cxx)
  TARGET_LINK_LIBRARIES(cmakexbuild CMakeLib)
  ADD_EXECUTABLE(OSXScriptLauncher
    CPack/OSXScriptLauncher.cxx)
  TARGET_LINK_LIBRARIES(OSXScriptLauncher cmsys)
  TARGET_LINK_LIBRARIES(OSXScriptLauncher "-framework Carbon")
ENDIF(APPLE)

# Build CMake executable
ADD_EXECUTABLE(cmake cmakemain.cxx)
TARGET_LINK_LIBRARIES(cmake CMakeLib)
ADD_EXECUTABLE(DumpDocumentation cmDumpDocumentation)
TARGET_LINK_LIBRARIES(DumpDocumentation CMakeLib)

# Build special executable for running programs on Windows 98
IF(WIN32)
  IF(NOT UNIX)
    ADD_EXECUTABLE(cmw9xcom cmw9xcom.cxx)
    TARGET_LINK_LIBRARIES(cmw9xcom CMakeLib)
    INSTALL_TARGETS(/bin cmw9xcom)
  ENDIF(NOT UNIX)
ENDIF(WIN32)

# Build CTest executable
ADD_EXECUTABLE(ctest ctest.cxx)
TARGET_LINK_LIBRARIES(ctest CTestLib)

# Build CPack executable
ADD_EXECUTABLE(cpack CPack/cpack.cxx)
TARGET_LINK_LIBRARIES(cpack CPackLib)

# Curses GUI
IF(BUILD_CursesDialog)
  INCLUDE(${CMake_SOURCE_DIR}/Source/CursesDialog/CMakeLists.txt)
ENDIF(BUILD_CursesDialog)

# MFC GUI
# MFC libraries are only available on Visual Studio
IF(BUILD_MFCDialog)
  SUBDIRS(MFCDialog)
ENDIF(BUILD_MFCDialog)

# Qt GUI
OPTION(BUILD_QtDialog "Build Qt dialog for CMake" FALSE)
IF(BUILD_QtDialog)
  SUBDIRS(QtDialog)
ENDIF(BUILD_QtDialog)

# WX Widgets GUI
OPTION(BUILD_WXDialog "Build wxWidgets dialog for CMake" FALSE)
MARK_AS_ADVANCED(BUILD_WXDialog)
IF(BUILD_WXDialog)
  SUBDIRS(WXDialog)
ENDIF(BUILD_WXDialog)

# FLTK GUI, needs some work: generator selection, paths with spaces, ...
# OPTION(BUILD_FLTKDialog "Build FLTK dialog for CMake" FALSE)
# MARK_AS_ADVANCED(BUILD_FLTKDialog)
# IF(BUILD_FLTKDialog)
#  SUBDIRS(FLTKDialog)
# ENDIF(BUILD_FLTKDialog)

INCLUDE (${CMAKE_BINARY_DIR}/Source/LocalUserOptions.cmake OPTIONAL)
INCLUDE (${CMAKE_SOURCE_DIR}/Source/LocalUserOptions.cmake OPTIONAL)

INSTALL_TARGETS(/bin cmake)
INSTALL_TARGETS(/bin ctest)
INSTALL_TARGETS(/bin cpack)
IF(APPLE)
  INSTALL_TARGETS(/bin cmakexbuild)
ENDIF(APPLE)

INSTALL_FILES(${CMAKE_DATA_DIR}/include cmCPluginAPI.h)
