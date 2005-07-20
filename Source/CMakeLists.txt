CMAKE_MINIMUM_REQUIRED(VERSION 1.5)

# configure the .h file
CONFIGURE_FILE(
  "${CMake_SOURCE_DIR}/Source/cmConfigure.cmake.h.in"
  "${CMake_BINARY_DIR}/Source/cmConfigure.h"
  )

# add the include path to find the .h 
INCLUDE_DIRECTORIES(
  "${CMake_BINARY_DIR}/Source"
  "${CMake_SOURCE_DIR}/Source"
  ${CMAKE_ZLIB_INCLUDES}
  ${CMAKE_EXPAT_INCLUDES}
  )

# let cmake know it is supposed to use it
ADD_DEFINITIONS(-DCMAKE_BUILD_WITH_CMAKE)

OPTION(CMAKE_REGENERATE_YACCLEX
  "Regenerate YACC and LEXX files" OFF)
MARK_AS_ADVANCED(CMAKE_REGENERATE_YACCLEX)
IF(CMAKE_REGENERATE_YACCLEX)
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
    FOREACH(name cmCommandArgument)
      SET(src "${CMAKE_CURRENT_SOURCE_DIR}/${name}Parser.y")
      SET(dst "${CMAKE_CURRENT_BINARY_DIR}/${name}Parser.cxx")
      SET(hdr "${CMAKE_CURRENT_BINARY_DIR}/${name}ParserTokens.h")
      ADD_CUSTOM_COMMAND(
        OUTPUT "${dst}"
        DEPENDS "${src}"
        COMMAND ${YACC_EXECUTABLE}
        ARGS --name-prefix=${name}_yy --defines="${hdr}" -o"${dst}" "${src}")
      SET(yacc_files ${yacc_files} "${dst}")
    ENDFOREACH(name)
    ADD_CUSTOM_TARGET(RerunYacc DEPENDS ${yacc_files})
  ENDIF(YACC_EXECUTABLE)
  IF(FLEX_EXECUTABLE)
    SET(lex_files)
    FOREACH(name cmCommandArgument)
      SET(src "${CMAKE_CURRENT_SOURCE_DIR}/${name}Lexer.in.l")
      SET(dst "${CMAKE_CURRENT_BINARY_DIR}/${name}Lexer.cxx")
      SET(hdr "${CMAKE_CURRENT_BINARY_DIR}/${name}Lexer.h")
      ADD_CUSTOM_COMMAND(
        OUTPUT "${dst}"
        DEPENDS "${src}"
        COMMAND ${FLEX_EXECUTABLE}
        ARGS --prefix=${name}_yy --header-file="${hdr}" -o"${dst}" "${src}")
      SET(lex_files ${lex_files} "${dst}")
    ENDFOREACH(name)
    ADD_CUSTOM_TARGET(RerunLex DEPENDS ${lex_files})
  ENDIF(FLEX_EXECUTABLE)

ENDIF(CMAKE_REGENERATE_YACCLEX)
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
  cmDynamicLoader.cxx
  cmDynamicLoader.h
  cmGeneratedFileStream.cxx
  cmGlob.cxx
  cmGlobalGenerator.cxx
  cmGlobalGenerator.h
  cmGlobalUnixMakefileGenerator3.cxx
  cmGlobalUnixMakefileGenerator3.h
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
  cmOrderLinkDirectories.cxx
  cmSourceFile.cxx
  cmSourceFile.h
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
  SET(SRCS ${SRCS}
    cmGlobalKdevelopGenerator.cxx
    cmLocalKdevelopGenerator.cxx)
ENDIF(UNIX)
# XCode only works on apple
IF(APPLE)
  SET(SRCS ${SRCS}
    cmXCodeObject.cxx
    cmGlobalXCodeGenerator.cxx
    cmGlobalXCodeGenerator.h
    cmGlobalXCode21Generator.cxx
    cmGlobalXCode21Generator.h
    cmLocalXCodeGenerator.cxx
    cmLocalXCodeGenerator.h)
ENDIF(APPLE)


IF (WIN32)
  IF(NOT UNIX)
    SET(SRCS ${SRCS}
      cmGlobalBorlandMakefileGenerator.cxx
      cmGlobalNMakeMakefileGenerator.cxx
      cmGlobalVisualStudio6Generator.cxx
      cmLocalVisualStudio6Generator.cxx
      cmGlobalBorlandMakefileGenerator.h
      cmGlobalNMakeMakefileGenerator.h
      cmGlobalVisualStudio6Generator.h
      cmLocalVisualStudio6Generator.h
      cmWin32ProcessExecution.cxx
      cmWin32ProcessExecution.h
      )
    IF(NOT MINGW)
      SET(SRCS ${SRCS}
        cmGlobalVisualStudio71Generator.h
        cmGlobalVisualStudio8Generator.h
        cmGlobalVisualStudio7Generator.h
        cmLocalVisualStudio7Generator.h
        cmGlobalVisualStudio8Generator.cxx
        cmGlobalVisualStudio71Generator.cxx
        cmGlobalVisualStudio7Generator.cxx
        cmLocalVisualStudio7Generator.cxx)
    ENDIF(NOT MINGW)
  ENDIF(NOT UNIX)
ENDIF (WIN32)

# create a library used by the command line and the GUI
ADD_LIBRARY(CMakeLib ${SRCS})
TARGET_LINK_LIBRARIES(CMakeLib cmsys ${CMAKE_EXPAT_LIBRARIES} ${CMAKE_ZLIB_LIBRARIES})
IF (UNIX)
  TARGET_LINK_LIBRARIES(CMakeLib ${CMAKE_DL_LIBS})
ENDIF (UNIX)
IF(CMAKE_BUILD_ON_VISUAL_STUDIO)
  # On Visual Studio we need rpcrt4.lib for cmVisualStudio7 generator.
  TARGET_LINK_LIBRARIES(CMakeLib rpcrt4.lib)
ENDIF(CMAKE_BUILD_ON_VISUAL_STUDIO)

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
SET(CMTEST_SRCS cmCTest.cxx 
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
ADD_LIBRARY(CTestLib ${CMTEST_SRCS})
TARGET_LINK_LIBRARIES(CTestLib CMakeLib ${CMAKE_CURL_LIBRARIES} ${CMAKE_XMLRPC_LIBRARIES})

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
  ENDIF(NOT UNIX)
ENDIF(WIN32)

# Build CTest executable
ADD_EXECUTABLE(ctest ctest.cxx)
TARGET_LINK_LIBRARIES(ctest CTestLib)

# Curses GUI
IF (UNIX)
  INCLUDE (${CMake_SOURCE_DIR}/Modules/FindCurses.cmake OPTIONAL)
  IF (CURSES_LIBRARY)
    INCLUDE(${CMake_SOURCE_DIR}/Source/CursesDialog/CMakeLists.txt)
  ENDIF (CURSES_LIBRARY)
ENDIF (UNIX)

# MFC GUI
# MFC libraries are only available on Visual Studio
IF(CMAKE_BUILD_ON_VISUAL_STUDIO)
  SUBDIRS(MFCDialog)
ENDIF(CMAKE_BUILD_ON_VISUAL_STUDIO)

# WX Widgets GUI
OPTION(CMAKE_BUILD_WX_DIALOG "Build wxWidgets dialog for CMake" FALSE)
MARK_AS_ADVANCED(CMAKE_BUILD_WX_DIALOG)
IF(CMAKE_BUILD_WX_DIALOG)
  SUBDIRS(WXDialog)
ENDIF(CMAKE_BUILD_WX_DIALOG)

# Testing
IF (NOT DART_ROOT)
  SET(MAKEPROGRAM ${CMAKE_MAKE_PROGRAM})
ENDIF (NOT DART_ROOT)

IF(BUILD_TESTING)
  # This variable is set by cmake, however to
  # test cmake we want to make sure that 
  # the ctest from this cmake is used for testing
  # and not the ctest from the cmake building and testing
  # cmake.
  SET(CMAKE_CTEST_COMMAND "${EXECUTABLE_OUTPUT_PATH}/ctest")

  # Should the long tests be run?
  OPTION(CMAKE_RUN_LONG_TESTS "Should the long tests be run (such as Bootstrap)." ON)
  MARK_AS_ADVANCED(CMAKE_RUN_LONG_TESTS)

  ADD_TEST(CommandLineTest ${CMAKE_CTEST_COMMAND}
    --build-and-test 
    "${CMake_SOURCE_DIR}/Tests/CommandLineTest"
    "${CMake_BINARY_DIR}/Tests/CommandLineTest"
    --build-two-config 
    --build-generator ${CMAKE_GENERATOR}
    --build-makeprogram ${MAKEPROGRAM}
    --build-project CommandLineTest
    --test-command CommandLineTest)

  ADD_TEST(CustomCommand  ${CMAKE_CTEST_COMMAND}
    --build-and-test 
    "${CMake_SOURCE_DIR}/Tests/CustomCommand"
    "${CMake_BINARY_DIR}/Tests/CustomCommand"
    --build-two-config
    --build-generator ${CMAKE_GENERATOR}
    --build-project CustomCommand
    --build-makeprogram ${MAKEPROGRAM}
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/CustomCommand/bin"
    --test-command CustomCommand
    )

  ADD_TEST(FindPackageTest ${CMAKE_CTEST_COMMAND}
    --build-and-test 
    "${CMake_SOURCE_DIR}/Tests/FindPackageTest"
    "${CMake_BINARY_DIR}/Tests/FindPackageTest"
    --build-two-config
    --build-generator ${CMAKE_GENERATOR}
    --build-project FindPackageTest
    --build-makeprogram ${MAKEPROGRAM}
    --test-command FindPackageTest)

  ADD_TEST(SystemInformation ${CMAKE_CTEST_COMMAND}
    --build-and-test 
    "${CMake_SOURCE_DIR}/Tests/SystemInformation"
    "${CMake_BINARY_DIR}/Tests/SystemInformation"
    --build-two-config
    --build-generator ${CMAKE_GENERATOR}
    --build-project DumpInformation
    --build-makeprogram ${MAKEPROGRAM}
    --test-command DumpInformation)

  ADD_TEST(StringFileTest ${CMAKE_CTEST_COMMAND}
    --build-and-test 
    "${CMake_SOURCE_DIR}/Tests/StringFileTest"
    "${CMake_BINARY_DIR}/Tests/StringFileTest"
    --build-two-config
    --build-generator ${CMAKE_GENERATOR}
    --build-project StringFileTest
    --build-makeprogram ${MAKEPROGRAM}
    --test-command StringFileTest)

  ADD_TEST(TryCompile ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/TryCompile"
    "${CMake_BINARY_DIR}/Tests/TryCompile"
    --build-generator ${CMAKE_GENERATOR}
    --build-project TryCompile
    --build-makeprogram ${MAKEPROGRAM}
    --build-two-config
    --test-command TryCompile)

  ADD_TEST(simple ${CMAKE_CTEST_COMMAND}
    --build-and-test 
    "${CMake_SOURCE_DIR}/Tests/Simple"
    "${CMake_BINARY_DIR}/Tests/Simple"
    --build-generator ${CMAKE_GENERATOR}
    --build-project Simple
    --build-makeprogram ${MAKEPROGRAM}
    --build-two-config
    --test-command simple)

#  ADD_TEST(SameName  ${CMAKE_CTEST_COMMAND}
#    --build-and-test 
#    "${CMake_SOURCE_DIR}/Tests/SameName"
#    "${CMake_BINARY_DIR}/Tests/SameName"
#    --build-generator ${CMAKE_GENERATOR}
#    --build-project SameName
#    --build-makeprogram ${MAKEPROGRAM}
#    --build-two-config
#    --test-command   
#    "${CMake_BINARY_DIR}/Tests/SameName/Exe1/mytest2")

  ADD_TEST(OutOfSource ${CMAKE_CTEST_COMMAND}
    --build-and-test 
    "${CMake_SOURCE_DIR}/Tests/OutOfSource"
    "${CMake_BINARY_DIR}/Tests/OutOfSource"
    --build-generator ${CMAKE_GENERATOR}
    --build-project OutOfSource
    --build-makeprogram ${MAKEPROGRAM}
    --build-two-config
    --test-command 
    "${CMake_BINARY_DIR}/Tests/OutOfSource/SubDir/OutOfSourceSubdir/simple")

  ADD_TEST(PreOrder ${CMAKE_CTEST_COMMAND}
    --build-and-test 
    "${CMake_SOURCE_DIR}/Tests/PreOrder"
    "${CMake_BINARY_DIR}/Tests/PreOrder"
    --build-generator ${CMAKE_GENERATOR}
    --build-project PreOrder
    --build-makeprogram ${MAKEPROGRAM}
    --build-two-config
    --test-command simple)

  ADD_TEST(conly  ${CMAKE_CTEST_COMMAND}
    --build-and-test 
    "${CMake_SOURCE_DIR}/Tests/COnly"
    "${CMake_BINARY_DIR}/Tests/COnly"
    --build-generator ${CMAKE_GENERATOR}
    --build-project conly
    --build-makeprogram ${MAKEPROGRAM}
    --build-two-config
    --test-command   conly)

  ADD_TEST(MacroTest ${CMAKE_CTEST_COMMAND}
    --build-and-test 
    "${CMake_SOURCE_DIR}/Tests/MacroTest"
    "${CMake_BINARY_DIR}/Tests/MacroTest"
    --build-generator ${CMAKE_GENERATOR}
    --build-project littleMacroTest 
    --build-makeprogram ${MAKEPROGRAM}
    --build-two-config
    --test-command   miniMacroTest)

  SET(SimpleInstallInstallDir 
    "${CMake_BINARY_DIR}/Tests/SimpleInstall/InstallDirectory")
  ADD_TEST(SimpleInstall ${CMAKE_CTEST_COMMAND}
    --build-and-test 
    "${CMake_SOURCE_DIR}/Tests/SimpleInstall"
    "${CMake_BINARY_DIR}/Tests/SimpleInstall"
    --build-generator ${CMAKE_GENERATOR}
    --build-project TestSimpleInstall
    --build-makeprogram ${MAKEPROGRAM}
    --build-two-config
    --build-options
    "-DCMAKE_INSTALL_PREFIX:PATH=${SimpleInstallInstallDir}"
    --test-command   ${SimpleInstallInstallDir}/bin/SimpleInstall)
  ADD_TEST(SimpleInstall-Stage2 ${CMAKE_CTEST_COMMAND}
    --build-and-test 
    "${CMake_SOURCE_DIR}/Tests/SimpleInstallS2"
    "${CMake_BINARY_DIR}/Tests/SimpleInstallS2"
    --build-generator ${CMAKE_GENERATOR}
    --build-project TestSimpleInstall
    --build-makeprogram ${MAKEPROGRAM}
    --build-two-config
    --build-options
    "-DCMAKE_INSTALL_PREFIX:PATH=${SimpleInstallInstallDir}"
    "-DSTAGE2:BOOL=1"
    --test-command   ${SimpleInstallInstallDir}/bin/SimpleInstallS2)

  ADD_TEST(X11  ${CMAKE_CTEST_COMMAND}
    --build-and-test 
    "${CMake_SOURCE_DIR}/Tests/X11"
    "${CMake_BINARY_DIR}/Tests/X11"
    --build-generator ${CMAKE_GENERATOR}
    --build-project UseX11
    --build-makeprogram ${MAKEPROGRAM}
    --build-two-config
    --test-command  UseX11)

  ADD_TEST(LoadedCommand  ${CMAKE_CTEST_COMMAND}
    --build-and-test 
    "${CMake_SOURCE_DIR}/Tests/LoadCommand"
    "${CMake_BINARY_DIR}/Tests/LoadCommand"
    --build-generator ${CMAKE_GENERATOR}
    --build-project LoadedCommand
    --build-makeprogram ${MAKEPROGRAM}
    --build-two-config
    --test-command  LoadedCommand)

  ADD_TEST(LoadedCommandOneConfig  ${CMAKE_CTEST_COMMAND}
    --build-and-test 
    "${CMake_SOURCE_DIR}/Tests/LoadCommandOneConfig"
    "${CMake_BINARY_DIR}/Tests/LoadCommandOneConfig"
    --build-generator ${CMAKE_GENERATOR}
    --build-project LoadedCommand
    --build-makeprogram ${MAKEPROGRAM}
    --test-command  LoadedCommand 
    )

  # Como does not seem to support shared libraries.
  GET_FILENAME_COMPONENT(CMAKE_BASE_NAME ${CMAKE_CXX_COMPILER} NAME_WE)
  IF(CMAKE_BASE_NAME MATCHES "^como$")
    SET(COMPILER_IS_COMO 1)
  ENDIF(CMAKE_BASE_NAME MATCHES "^como$")
  IF(NOT COMPILER_IS_COMO)
    ADD_TEST(complex   ${CMAKE_CTEST_COMMAND}
      --build-and-test 
      "${CMake_SOURCE_DIR}/Tests/Complex"
      "${CMake_BINARY_DIR}/Tests/Complex"
      --build-two-config
      --build-generator ${CMAKE_GENERATOR}
      --build-project complex
      --build-makeprogram ${MAKEPROGRAM}
      --build-exe-dir "${CMake_BINARY_DIR}/Tests/Complex/bin"
      --test-command complex 
      )

    ADD_TEST(complexOneConfig  ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/ComplexOneConfig"
      "${CMake_BINARY_DIR}/Tests/ComplexOneConfig"
      --build-generator ${CMAKE_GENERATOR}
      --build-project complex
      --build-makeprogram ${MAKEPROGRAM}
      --build-exe-dir "${CMake_BINARY_DIR}/Tests/ComplexOneConfig/bin"
      --test-command complex)

#    ADD_TEST(complexRelativePaths  ${CMAKE_CTEST_COMMAND}
#      --build-and-test
#      "${CMake_SOURCE_DIR}/Tests/ComplexRelativePaths"
#      "${CMake_BINARY_DIR}/Tests/ComplexRelativePaths"
#      --build-generator ${CMAKE_GENERATOR}
#      --build-project complex
#      --build-makeprogram ${MAKEPROGRAM}
#      --build-exe-dir "${CMake_BINARY_DIR}/Tests/ComplexRelativePaths/bin" 
#      --build-options -DCMAKE_USE_RELATIVE_PATHS:BOOL=ON
#      --test-command complex)

  ENDIF(NOT COMPILER_IS_COMO)

  ADD_TEST(Example ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Example"
    "${CMake_BINARY_DIR}/Example"
    --build-generator ${CMAKE_GENERATOR}
    --build-project HELLO
    --build-makeprogram ${MAKEPROGRAM}
    --build-exe-dir "${CMake_BINARY_DIR}/Example/Demo"
    --test-command helloDemo
    )

  ADD_TEST(testing ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/Testing"
    "${CMake_BINARY_DIR}/Tests/Testing"
    --build-generator ${CMAKE_GENERATOR}
    --build-project testing
    --build-makeprogram ${MAKEPROGRAM}
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/Testing/bin"
    --test-command testing
    )

  ADD_TEST(wrapping  ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/Wrapping"
    "${CMake_BINARY_DIR}/Tests/Wrapping"
    --build-generator ${CMAKE_GENERATOR}
    --build-project wrapping
    --build-makeprogram ${MAKEPROGRAM}
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/Wrapping/bin"
    --test-command wrapping
    )

  INCLUDE ( ${CMAKE_ROOT}/Modules/FindQt.cmake )
  MARK_AS_ADVANCED(
    QT_INCLUDE_PATH
    QT_MOC_EXE
    QT_QT_LIBRARY
    QT_UIC_EXE)

  IF (QT_FOUND AND QT_UIC_EXECUTABLE)
    ADD_TEST(qtwrapping  ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/Wrapping"
      "${CMake_BINARY_DIR}/Tests/Wrapping"
      --build-generator ${CMAKE_GENERATOR}
      --build-project Wrapping
      --build-makeprogram ${MAKEPROGRAM}
      --build-exe-dir "${CMake_BINARY_DIR}/Tests/Wrapping/bin"
      --test-command qtwrapping
      )
  ENDIF (QT_FOUND AND QT_UIC_EXECUTABLE)

  ADD_TEST(testdriver1 ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/TestDriver"
    "${CMake_BINARY_DIR}/Tests/TestDriver"
    --build-generator ${CMAKE_GENERATOR}
    --build-makeprogram ${MAKEPROGRAM}
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/Wrapping/bin"
    --build-project TestDriverTest
    --test-command TestDriverTest test1
    )

  ADD_TEST(testdriver2 ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/TestDriver"
    "${CMake_BINARY_DIR}/Tests/TestDriver"
    --build-generator ${CMAKE_GENERATOR}
    --build-makeprogram ${MAKEPROGRAM}
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/Wrapping/bin"
    --build-project TestDriverTest
    --test-command TestDriverTest test2
    )

  ADD_TEST(testdriver3  ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/TestDriver"
    "${CMake_BINARY_DIR}/Tests/TestDriver"
    --build-generator ${CMAKE_GENERATOR}
    --build-makeprogram ${MAKEPROGRAM}
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/Wrapping/bin"
    --build-project TestDriverTest
    --test-command TestDriverTest subdir/test3
    )

  ADD_TEST(dependency_w_libout  ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/Dependency"
    "${CMake_BINARY_DIR}/Tests/Dependency/WithLibOut"
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/Dependency/WithLibOut/Exec"
    --build-project Dependency 
    --build-generator ${CMAKE_GENERATOR}
    --build-makeprogram ${MAKEPROGRAM}
    --build-options
    -DLIBRARY_OUTPUT_PATH:PATH=${CMake_BINARY_DIR}/Tests/Dependency/WithLibOut/Lib
    --test-command exec
    )


  ADD_TEST(dependency_wo_lib_out   ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/Dependency"
    "${CMake_BINARY_DIR}/Tests/Dependency/WOLibOut"
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/Dependency/WOLibOut/Exec"
    --build-generator ${CMAKE_GENERATOR}
    --build-makeprogram ${MAKEPROGRAM}
    --build-project Dependency 
    --test-command exec
    )

  ADD_TEST(dependency2  ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/Dependency"
    "${CMake_BINARY_DIR}/Tests/Dependency/WithLibOut" 
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/Dependency/WithLibOut/Exec2"
    --build-generator ${CMAKE_GENERATOR}
    --build-makeprogram ${MAKEPROGRAM}
    --build-project Dependency 
    --build-options
    -DLIBRARY_OUTPUT_PATH:PATH=${CMake_BINARY_DIR}/Tests/Dependency/WithLibOut/Lib
    --test-command exec2
    )

  ADD_TEST(dependency3  ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/Dependency"
    "${CMake_BINARY_DIR}/Tests/Dependency/WithLibOut"
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/Dependency/WithLibOut/Exec3"
    --build-generator ${CMAKE_GENERATOR}
    --build-makeprogram ${MAKEPROGRAM}
    --build-project Dependency 
    --build-options
    -DLIBRARY_OUTPUT_PATH:PATH=${CMake_BINARY_DIR}/Tests/Dependency/WithLibOut/Lib
    --test-command exec3)

  ADD_TEST(dependency4   ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/Dependency"
    "${CMake_BINARY_DIR}/Tests/Dependency/WithLibOut"
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/Dependency/WithLibOut/Exec4"
    --build-generator ${CMAKE_GENERATOR}
    --build-makeprogram ${MAKEPROGRAM}
    --build-project Dependency 
    --build-options
    -DLIBRARY_OUTPUT_PATH:PATH=${CMake_BINARY_DIR}/Tests/Dependency/WithLibOut/Lib
    --test-command exec4
    )

  ADD_TEST(JumpWithLibOut  ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/Jump"
    "${CMake_BINARY_DIR}/Tests/Jump/WithLibOut"
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/Jump/WithLibOut/Executable"
    --build-project Jump
    --build-generator ${CMAKE_GENERATOR}
    --build-makeprogram ${MAKEPROGRAM}
    --build-options
    -DLIBRARY_OUTPUT_PATH:PATH=${CMake_BINARY_DIR}/Tests/Jump/WithLibOut/Lib
    --test-command jumpExecutable
    )

  ADD_TEST(JumpNoLibOut  ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/Jump"
    "${CMake_BINARY_DIR}/Tests/Jump/NoLibOut"
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/Jump/NoLibOut/Executable"
    --build-run-dir "${CMake_BINARY_DIR}/Tests/Jump/NoLibOut/Executable"
    --build-project Jump 
    --build-generator ${CMAKE_GENERATOR}
    --build-makeprogram ${MAKEPROGRAM}
    --test-command jumpExecutable
    )

  ADD_TEST(linkline   ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/LinkLine"
    "${CMake_BINARY_DIR}/Tests/LinkLine"
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/LinkLine"
    --build-generator ${CMAKE_GENERATOR}
    --build-makeprogram ${MAKEPROGRAM}
    --build-project Linkline
    --test-command Exec
    )

  ADD_TEST(linkorder1 ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/LinkLineOrder"
    "${CMake_BINARY_DIR}/Tests/LinkLineOrder" 
    --build-generator ${CMAKE_GENERATOR}
    --build-makeprogram ${MAKEPROGRAM}
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/LinkLineOrder"
    --build-project LinkLineOrder
    --test-command Exec1
    )

  ADD_TEST(linkorder2  ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/LinkLineOrder"
    "${CMake_BINARY_DIR}/Tests/LinkLineOrder"
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/LinkLineOrder"
    --build-generator ${CMAKE_GENERATOR}
    --build-makeprogram ${MAKEPROGRAM}
    --build-project LinkLineOrder
    --test-command Exec2
    )

  ADD_TEST(ExternalOBJ ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/ExternalOBJ"
    "${CMake_BINARY_DIR}/Tests/ExternalOBJ"
    --build-generator ${CMAKE_GENERATOR}
    --build-project ExternalOBJ
    --build-makeprogram ${MAKEPROGRAM}
    --build-exe-dir "${CMake_BINARY_DIR}/ExternalOBJ"
    --test-command executable
    )

  ADD_TEST(kwsys ${CMAKE_CTEST_COMMAND}
    --build-and-test
    "${CMake_SOURCE_DIR}/Source/kwsys"
    "${CMake_BINARY_DIR}/Tests/kwsys"
    --build-exe-dir "${CMake_BINARY_DIR}/Tests/kwsys"
    --build-generator ${CMAKE_GENERATOR}
    --build-makeprogram ${MAKEPROGRAM}
    --build-project kwsys
    --test-command  testIOS
    )

  IF (WIN32)
    ADD_TEST(SubDir ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/SubDir"
      "${CMake_BINARY_DIR}/Tests/SubDir"
      --build-exe-dir "${CMake_BINARY_DIR}/Tests/SubDir/Executable"
      --build-generator ${CMAKE_GENERATOR}
      --build-makeprogram ${MAKEPROGRAM}
      --build-project SUBDIR
      --test-command test 
      "${CMake_BINARY_DIR}/Tests/SubDir/ShouldBeHere"
      "${CMake_BINARY_DIR}/Tests/SubDir/testfromsubdir.obj"
      )
  ELSE (WIN32)
    ADD_TEST(SubDir ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/SubDir"
      "${CMake_BINARY_DIR}/Tests/SubDir"
      --build-exe-dir "${CMake_BINARY_DIR}/Tests/SubDir/Executable"
      --build-generator ${CMAKE_GENERATOR}
      --build-makeprogram ${MAKEPROGRAM}
      --build-project SUBDIR
      --test-command test 
      "${CMake_BINARY_DIR}/Tests/SubDir/ShouldBeHere"
      "${CMake_BINARY_DIR}/Tests/SubDir/testfromsubdir.o"
      )
  ENDIF (WIN32)    

  IF("${CMAKE_GENERATOR}" MATCHES "Makefile")
    ADD_TEST(MakeClean ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/MakeClean"
      "${CMake_BINARY_DIR}/Tests/MakeClean"
      --build-generator ${CMAKE_GENERATOR}
      --build-project MakeClean
      --build-makeprogram ${MAKEPROGRAM}
      --build-exe-dir "${CMake_BINARY_DIR}/MakeClean"
      --test-command check_clean
      )
  ENDIF("${CMAKE_GENERATOR}" MATCHES "Makefile")

  IF(${CMAKE_GENERATOR} MATCHES "Visual Studio")
    ADD_TEST(VSExternalInclude ${CMAKE_CTEST_COMMAND}
      --build-and-test 
      "${CMake_SOURCE_DIR}/Tests/VSExternalInclude"
      "${CMake_BINARY_DIR}/Tests/VSExternalInclude"
      --build-two-config
      --build-generator ${CMAKE_GENERATOR}
      --build-project VSExternalInclude
      --build-makeprogram ${MAKEPROGRAM}
      --test-command VSExternalInclude)
  ENDIF(${CMAKE_GENERATOR} MATCHES "Visual Studio")

  IF (APPLE AND CMAKE_COMPILER_IS_GNUCXX)
    ADD_TEST(BundleTest ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/BundleTest"
      "${CMake_BINARY_DIR}/Tests/BundleTest"
      --build-two-config
      --build-generator ${CMAKE_GENERATOR}
      --build-makeprogram ${MAKEPROGRAM}
      --build-project BundleTest
      --test-command  BundleTest.app/Contents/MacOS/BundleTest)
    
    ADD_TEST(objc++ ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/Objc++"
      "${CMake_BINARY_DIR}/Tests/Objc++"
      --build-generator ${CMAKE_GENERATOR}
      --build-makeprogram ${MAKEPROGRAM}
      --build-project  objc++
      --test-command  objc++
      )
  ENDIF (APPLE AND CMAKE_COMPILER_IS_GNUCXX)

  IF (CTEST_TEST_CTEST AND CMAKE_RUN_LONG_TESTS)
    CONFIGURE_FILE("${CMake_SOURCE_DIR}/Tests/CTestTest/test.cmake.in"
      "${CMake_BINARY_DIR}/Tests/CTestTest/test.cmake" @ONLY ESCAPE_QUOTES)
    ADD_TEST(CTestTest ${CMAKE_CTEST_COMMAND}
      -S "${CMake_BINARY_DIR}/Tests/CTestTest/test.cmake" -V
      --output-log "${CMake_BINARY_DIR}/Tests/CTestTest/testOutput.log"
      )
    CONFIGURE_FILE("${CMake_SOURCE_DIR}/Tests/CTestTest2/test.cmake.in"
      "${CMake_BINARY_DIR}/Tests/CTestTest2/test.cmake" @ONLY ESCAPE_QUOTES)
    ADD_TEST(CTestTest2 ${CMAKE_CTEST_COMMAND}
      -S "${CMake_BINARY_DIR}/Tests/CTestTest2/test.cmake" -V
      --output-log "${CMake_BINARY_DIR}/Tests/CTestTest2/testOutput.log"
      )
    CONFIGURE_FILE("${CMake_SOURCE_DIR}/Tests/CTestTest3/test.cmake.in"
      "${CMake_BINARY_DIR}/Tests/CTestTest3/test.cmake" @ONLY ESCAPE_QUOTES)
    ADD_TEST(CTestTest3 ${CMAKE_CTEST_COMMAND}
      -S "${CMake_BINARY_DIR}/Tests/CTestTest3/test.cmake" -V
      --output-log "${CMake_BINARY_DIR}/Tests/CTestTest3/testOutput.log"
      )
  ENDIF (CTEST_TEST_CTEST AND CMAKE_RUN_LONG_TESTS)

  IF("${CMAKE_GENERATOR}" MATCHES XCode)
    SET(CMAKE_SKIP_BOOTSTRAP_TEST 1)
  ENDIF("${CMAKE_GENERATOR}" MATCHES XCode)
  IF(EXISTS "${CMAKE_BINARY_DIR}/CMakeLists.txt")
    # If there is CMakeLists.txt in the binary tree, assume in-source build
    SET(CMAKE_SKIP_BOOTSTRAP_TEST 1)
  ENDIF(EXISTS "${CMAKE_BINARY_DIR}/CMakeLists.txt")
  IF(UNIX AND CMAKE_RUN_LONG_TESTS AND NOT CMAKE_SKIP_BOOTSTRAP_TEST)
    ADD_TEST(BootstrapTest ${CMAKE_CTEST_COMMAND}
      --build-and-test
      ${CMake_SOURCE_DIR}
      ${CMake_BINARY_DIR}/Tests/BootstrapTest
      --build-nocmake 
      --build-noclean
      --build-makeprogram ${CMake_SOURCE_DIR}/bootstrap
      --build-generator "${CMAKE_GENERATOR}"
      --test-command ${CMake_BINARY_DIR}/Tests/BootstrapTest/Bootstrap.cmk/cmake)
  ENDIF(UNIX AND CMAKE_RUN_LONG_TESTS AND NOT CMAKE_SKIP_BOOTSTRAP_TEST)

  IF(CMAKE_GENERATOR MATCHES "Makefiles")
    # see if we can find a fortran compiler on the machine
    # if so, add the fortran test and see if it works.
    SET(CMAKE_Fortran_COMPILER_LIST ifort ifc efc f95 pgf95 
      lf95 xlf95 fort gfortran f90  pgf90   xlf90  
      epcf90 f77  fort77 frt pgf77  xlf  fl32 af77 g77  )
    FIND_PROGRAM(CMAKE_Fortran_COMPILER_FULLPATH NAMES ${CMAKE_Fortran_COMPILER_LIST} )
    MARK_AS_ADVANCED(CMAKE_Fortran_COMPILER_FULLPATH)
    IF(CMAKE_Fortran_COMPILER_FULLPATH)
      ADD_TEST(Fortran ${CMAKE_CTEST_COMMAND}
        --build-and-test 
        "${CMake_SOURCE_DIR}/Tests/Fortran"
        "${CMake_BINARY_DIR}/Tests/Fortran"
        --build-generator ${CMAKE_GENERATOR}
        --build-project testf
        --build-makeprogram ${MAKEPROGRAM}
        --build-two-config
        --test-command testf)
    ENDIF(CMAKE_Fortran_COMPILER_FULLPATH)

    INCLUDE(FindJava)
    IF(JAVA_COMPILE AND JAVA_RUNTIME AND JAVA_ARCHIVE AND NOT MINGW)
      GET_FILENAME_COMPONENT(JNIPATH ${JAVA_COMPILE} PATH)
      FIND_FILE(JNI_H jni.h
        "${JNIPATH}/../include"
        "${JNIPATH}/../java/include")
      IF(JNI_H)
        FILE(READ "${JNI_H}" JNI_FILE)
        IF("${JNI_FILE}" MATCHES "JDK1_2")
          ADD_TEST(Java ${CMAKE_CTEST_COMMAND}
            --build-and-test 
            "${CMake_SOURCE_DIR}/Tests/Java"
            "${CMake_BINARY_DIR}/Tests/Java"
            --build-generator ${CMAKE_GENERATOR}
            --build-project hello
            --build-makeprogram ${MAKEPROGRAM}
            --build-two-config 
            --build-run-dir "${CMake_BINARY_DIR}/Tests/Java/"
            --test-command ${JAVA_RUNTIME} -classpath hello.jar HelloWorld)
        ENDIF("${JNI_FILE}" MATCHES "JDK1_2")
      ENDIF(JNI_H) 
    ENDIF(JAVA_COMPILE AND JAVA_RUNTIME AND JAVA_ARCHIVE AND NOT MINGW)
  ENDIF(CMAKE_GENERATOR MATCHES "Makefiles")
    
  IF (CMAKE_WXWINDOWS_WXCONFIG_EXECUTABLE OR WXWINDOWS_INCLUDE_DIR)
    # Will be set if the wxwindows gui is on
    ADD_TEST(UseWX ${CMAKE_CTEST_COMMAND}
      --build-and-test
      "${CMake_SOURCE_DIR}/Tests/UseWX"
      "${CMake_BINARY_DIR}/Tests/UseWX"
      --build-generator ${CMAKE_GENERATOR}
      --build-makeprogram ${MAKEPROGRAM}
      --build-exe-dir ${CMake_BINARY_DIR}/Tests/UseWX
      --build-project UsewxWindows
      --build-options -DCMAKE_WXWINDOWS_WXCONFIG_EXECUTABLE:FILEPATH=${CMAKE_WXWINDOWS_WXCONFIG_EXECUTABLE}
      --test-command UseWX
      )
  ENDIF (CMAKE_WXWINDOWS_WXCONFIG_EXECUTABLE OR WXWINDOWS_INCLUDE_DIR)

  IF(UNIX)
    STRING(COMPARE EQUAL "${CMAKE_INSTALL_PREFIX}" "${CMake_BINARY_DIR}/Tests/TestShellInstall/Prefix"
      PREFIX_IS_FOR_TEST)
    IF(PREFIX_IS_FOR_TEST)
      CONFIGURE_FILE(
        ${CMake_SOURCE_DIR}/Tests/TestInstall.sh.in
        ${CMake_BINARY_DIR}/Tests/TestShellInstall/TestInstall.sh
        @ONLY IMMEDIATE
        )
      ADD_TEST(ShellInstall /bin/sh ${CMake_BINARY_DIR}/Tests/TestShellInstall/TestShellInstall.sh)
    ENDIF(PREFIX_IS_FOR_TEST)
  ENDIF(UNIX)

ENDIF(BUILD_TESTING)

INCLUDE (${CMAKE_BINARY_DIR}/Source/LocalUserOptions.cmake OPTIONAL)
INCLUDE (${CMAKE_SOURCE_DIR}/Source/LocalUserOptions.cmake OPTIONAL)

INSTALL_TARGETS(/bin cmake)
INSTALL_TARGETS(/bin ctest)
INSTALL_FILES(${CMAKE_DATA_DIR}/include cmCPluginAPI.h)
