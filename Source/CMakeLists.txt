CMAKE_MINIMUM_REQUIRED(VERSION 1.5)

SET(SRCS
cmake.cxx
cmakewizard.cxx
cmMakeDepend.cxx
cmMakefile.cxx
cmSourceFile.cxx
cmSystemTools.cxx
cmDocumentation.cxx
cmDynamicLoader.cxx
cmCommands.cxx
cmTarget.cxx
cmCustomCommand.cxx
cmCacheManager.cxx
cmSourceGroup.cxx
cmListFileCache.cxx
cmGlob.cxx
cmGlobalGenerator.cxx
cmGlobalUnixMakefileGenerator.cxx
cmLocalGenerator.cxx
cmLocalUnixMakefileGenerator.cxx
cmVariableWatch.cxx
cmake.h
cmakewizard.h
cmMakeDepend.h
cmMakefile.h
cmSourceFile.h
cmSystemTools.h
cmDynamicLoader.h
cmCommands.h
cmTarget.h
cmCustomCommand.h
cmCacheManager.h
cmSourceGroup.h
cmListFileCache.h
cmGlobalGenerator.h
cmGlobalUnixMakefileGenerator.h
cmLocalGenerator.h
cmLocalUnixMakefileGenerator.h
cmVariableWatch.h
)

# configure the .h file
CONFIGURE_FILE(
${CMake_SOURCE_DIR}/Source/cmConfigure.cmake.h.in 
${CMake_BINARY_DIR}/Source/cmConfigure.h )

# add the include path to find the .h 
INCLUDE_DIRECTORIES(${CMake_BINARY_DIR}/Source)
INCLUDE_DIRECTORIES(${CMake_SOURCE_DIR}/Source)

# let cmake know it is supposed to use it
ADD_DEFINITIONS(-DCMAKE_BUILD_WITH_CMAKE)

IF (APPLE)
  SET(SRCS ${SRCS}
    cmGlobalCodeWarriorGenerator.cxx
    cmLocalCodeWarriorGenerator.cxx
    cmGlobalCodeWarriorGenerator.h
    cmLocalCodeWarriorGenerator.h
    )
ENDIF (APPLE)

IF (WIN32)
  IF(NOT UNIX)
    SET(SRCS ${SRCS}
                    cmGlobalBorlandMakefileGenerator.cxx
                    cmGlobalNMakeMakefileGenerator.cxx
                    cmGlobalVisualStudio6Generator.cxx
                    cmLocalVisualStudio6Generator.cxx
                    cmGlobalVisualStudio71Generator.cxx
                    cmGlobalVisualStudio7Generator.cxx
                    cmLocalVisualStudio7Generator.cxx
                    cmGlobalBorlandMakefileGenerator.h
                    cmGlobalNMakeMakefileGenerator.h
                    cmGlobalVisualStudio6Generator.h
                    cmLocalVisualStudio6Generator.h
                    cmGlobalVisualStudio7Generator.h
                    cmLocalVisualStudio7Generator.h
		    cmWin32ProcessExecution.cxx
		    cmWin32ProcessExecution.h
                    )
    IF(  NOT BORLAND )
      LINK_LIBRARIES( rpcrt4.lib )
      ADD_EXECUTABLE(cmw9xcom cmw9xcom.cxx)
      TARGET_LINK_LIBRARIES(cmw9xcom CMakeLib)
      SUBDIRS(MFCDialog)
    ENDIF( NOT BORLAND )
  ENDIF(NOT UNIX)
ENDIF (WIN32)

# create a library used by the command line and the GUI
ADD_LIBRARY(CMakeLib ${SRCS})

TARGET_LINK_LIBRARIES(CMakeLib cmsys)

# always link in the library
# the library is found here
LINK_DIRECTORIES(${CMake_BINARY_DIR}/Source)

ADD_EXECUTABLE(cmake cmakemain.cxx)
ADD_EXECUTABLE(DumpDocumentation cmDumpDocumentation)

SET(CMTEST_SRCS ctest.cxx cmCTest.cxx)

IF (UNIX)
  TARGET_LINK_LIBRARIES(CMakeLib ${CMAKE_DL_LIBS})
ENDIF (UNIX)

TARGET_LINK_LIBRARIES(cmake CMakeLib)
TARGET_LINK_LIBRARIES(DumpDocumentation CMakeLib)

SET(CMAKE_BUILD_WITH_CURL 1)
IF(CMAKE_BUILD_WITH_CURL)
  SUBDIRS(CTest)
  SET(CMTEST_SRCS ${CMTEST_SRCS} CTest/cmCTestSubmit.cxx)
  ADD_DEFINITIONS(-DHAVE_CURL)
  INCLUDE_DIRECTORIES(
    ${CMAKE_SOURCE_DIR}/Source/CTest/Curl
    ${CMAKE_SOURCE_DIR}/Source/CTest
    )
ENDIF(CMAKE_BUILD_WITH_CURL)

ADD_EXECUTABLE(ctest ${CMTEST_SRCS})
TARGET_LINK_LIBRARIES(ctest CMakeLib)
IF(CMAKE_BUILD_WITH_CURL)
  TARGET_LINK_LIBRARIES(ctest Curl)
ENDIF(CMAKE_BUILD_WITH_CURL)

IF (UNIX)
  INCLUDE (${CMake_SOURCE_DIR}/Modules/FindCurses.cmake OPTIONAL)
  IF (CURSES_LIBRARY)
    SUBDIRS(CursesDialog/form)
    INCLUDE(${CMake_SOURCE_DIR}/Source/CursesDialog/CMakeLists.txt)
  ENDIF (CURSES_LIBRARY)
ENDIF (UNIX)

OPTION(CMAKE_BUILD_WX_DIALOG "Build wxWindows dialog for CMake" FALSE)
MARK_AS_ADVANCED(CMAKE_BUILD_WX_DIALOG)
IF(CMAKE_BUILD_WX_DIALOG)
  SUBDIRS(WXDialog)
ENDIF(CMAKE_BUILD_WX_DIALOG)

IF (NOT DART_ROOT)
SET(MAKEPROGRAM ${CMAKE_MAKE_PROGRAM})
ENDIF (NOT DART_ROOT)

CONFIGURE_FILE(
  ${CMake_SOURCE_DIR}/Source/cmaketest.h.in 
  ${CMake_BINARY_DIR}/Source/cmaketest.h ESCAPE_QUOTES)

ADD_EXECUTABLE(cmaketest cmaketest.cxx)
TARGET_LINK_LIBRARIES(cmaketest CMakeLib)

IF(BUILD_TESTING)
    ADD_TEST(CommandLineTest ${EXECUTABLE_OUTPUT_PATH}/cmaketest 
      ${CMake_SOURCE_DIR}/Tests/CommandLineTest
      ${CMake_BINARY_DIR}/Tests/CommandLineTest
      CommandLineTest)

    ADD_TEST(CustomCommand ${EXECUTABLE_OUTPUT_PATH}/cmaketest 
      ${CMake_SOURCE_DIR}/Tests/CustomCommand
      ${CMake_BINARY_DIR}/Tests/CustomCommand 
      CustomCommand
      ${CMake_BINARY_DIR}/Tests/CustomCommand/bin)

    ADD_TEST(FindPackageTest ${EXECUTABLE_OUTPUT_PATH}/cmaketest 
      ${CMake_SOURCE_DIR}/Tests/FindPackageTest
      ${CMake_BINARY_DIR}/Tests/FindPackageTest
      FindPackageTest)

    ADD_TEST(SystemInformation ${EXECUTABLE_OUTPUT_PATH}/cmaketest 
      ${CMake_SOURCE_DIR}/Tests/SystemInformation
      ${CMake_BINARY_DIR}/Tests/SystemInformation 
      DumpInformation)

    ADD_TEST(StringFileTest ${EXECUTABLE_OUTPUT_PATH}/cmaketest 
      ${CMake_SOURCE_DIR}/Tests/StringFileTest
      ${CMake_BINARY_DIR}/Tests/StringFileTest
      StringFileTest)

    ADD_TEST(TryCompile ${EXECUTABLE_OUTPUT_PATH}/cmaketest 
      ${CMake_SOURCE_DIR}/Tests/TryCompile
      ${CMake_BINARY_DIR}/Tests/TryCompile 
      TryCompile)

    ADD_TEST(simple ${EXECUTABLE_OUTPUT_PATH}/cmaketest 
      ${CMake_SOURCE_DIR}/Tests/Simple 
      ${CMake_BINARY_DIR}/Tests/Simple 
      simple)

    ADD_TEST(conly ${EXECUTABLE_OUTPUT_PATH}/cmaketest 
      ${CMake_SOURCE_DIR}/Tests/COnly 
      ${CMake_BINARY_DIR}/Tests/COnly 
      conly)

    ADD_TEST(X11 ${EXECUTABLE_OUTPUT_PATH}/cmaketest 
      ${CMake_SOURCE_DIR}/Tests/X11
      ${CMake_BINARY_DIR}/Tests/X11 
      UseX11)

    ADD_TEST(LoadedCommand ${EXECUTABLE_OUTPUT_PATH}/cmaketest 
      ${CMake_SOURCE_DIR}/Tests/LoadCommand 
      ${CMake_BINARY_DIR}/Tests/LoadCommand 
      LoadedCommand)

    ADD_TEST(LoadedCommandOneConfig ${EXECUTABLE_OUTPUT_PATH}/cmaketest 
      ${CMake_SOURCE_DIR}/Tests/LoadCommand 
      ${CMake_BINARY_DIR}/Tests/LoadCommandOneConfig
      LoadedCommand ONLY_ONE_CONFIG)

    # Como does not seem to support shared libraries.
    GET_FILENAME_COMPONENT(CMAKE_BASE_NAME ${CMAKE_CXX_COMPILER} NAME_WE)
    IF(CMAKE_BASE_NAME MATCHES "^como$")
      SET(COMPILER_IS_COMO 1)
    ENDIF(CMAKE_BASE_NAME MATCHES "^como$")
    IF(NOT COMPILER_IS_COMO)
      ADD_TEST(complex ${EXECUTABLE_OUTPUT_PATH}/cmaketest 
        ${CMake_SOURCE_DIR}/Tests/Complex 
        ${CMake_BINARY_DIR}/Tests/Complex 
        complex 
        ${CMake_BINARY_DIR}/Tests/Complex/bin)

      ADD_TEST(complexOneConfig ${EXECUTABLE_OUTPUT_PATH}/cmaketest 
        ${CMake_SOURCE_DIR}/Tests/Complex 
        ${CMake_BINARY_DIR}/Tests/ComplexOneConfig 
        complex 
        ${CMake_BINARY_DIR}/Tests/ComplexOneConfig/bin ONLY_ONE_CONFIG)
    ENDIF(NOT COMPILER_IS_COMO)

    ADD_TEST(Example ${EXECUTABLE_OUTPUT_PATH}/cmaketest 
      ${CMake_SOURCE_DIR}/Example
      ${CMake_BINARY_DIR}/Example
      helloDemo
      ${CMake_BINARY_DIR}/Example/Demo
      HELLO)

    ADD_TEST(testing ${EXECUTABLE_OUTPUT_PATH}/cmaketest 
      ${CMake_SOURCE_DIR}/Tests/Testing 
      ${CMake_BINARY_DIR}/Tests/Testing
      testing
      ${CMake_BINARY_DIR}/Tests/Testing/bin)

    ADD_TEST(wrapping ${EXECUTABLE_OUTPUT_PATH}/cmaketest 
      ${CMake_SOURCE_DIR}/Tests/Wrapping 
      ${CMake_BINARY_DIR}/Tests/Wrapping
      wrapping
      ${CMake_BINARY_DIR}/Tests/Wrapping/bin)

    INCLUDE ( ${CMAKE_ROOT}/Modules/FindQt.cmake )
    MARK_AS_ADVANCED(
     QT_INCLUDE_PATH
     QT_MOC_EXE
     QT_QT_LIBRARY
     QT_UIC_EXE)

    IF (QT_FOUND)
      ADD_TEST(qtwrapping ${EXECUTABLE_OUTPUT_PATH}/cmaketest 
        ${CMake_SOURCE_DIR}/Tests/Wrapping 
        ${CMake_BINARY_DIR}/Tests/Wrapping
        qtwrapping
        ${CMake_BINARY_DIR}/Tests/Wrapping/bin
        wrapping)
    ENDIF (QT_FOUND)

    ADD_TEST(testdriver1 ${EXECUTABLE_OUTPUT_PATH}/cmaketest 
      ${CMake_SOURCE_DIR}/Tests/TestDriver 
      ${CMake_BINARY_DIR}/Tests/TestDriver
      TestDriverTest
      ${CMake_BINARY_DIR}/Tests/Wrapping/bin
      TestDriverTest test1)

    ADD_TEST(testdriver2 ${EXECUTABLE_OUTPUT_PATH}/cmaketest 
      ${CMake_SOURCE_DIR}/Tests/TestDriver 
      ${CMake_BINARY_DIR}/Tests/TestDriver
      TestDriverTest
      ${CMake_BINARY_DIR}/Tests/Wrapping/bin
      TestDriverTest test2)

    ADD_TEST(testdriver3 ${EXECUTABLE_OUTPUT_PATH}/cmaketest 
      ${CMake_SOURCE_DIR}/Tests/TestDriver 
      ${CMake_BINARY_DIR}/Tests/TestDriver
      TestDriverTest
      ${CMake_BINARY_DIR}/Tests/Wrapping/bin
      TestDriverTest subdir/test3)

    ADD_TEST(dependency_w_libout ${EXECUTABLE_OUTPUT_PATH}/cmaketest 
      ${CMake_SOURCE_DIR}/Tests/Dependency 
      ${CMake_BINARY_DIR}/Tests/Dependency/WithLibOut
      exec
      ${CMake_BINARY_DIR}/Tests/Dependency/WithLibOut/Exec
      Dependency CMAKE_ARGS -DLIBRARY_OUTPUT_PATH:PATH=${CMake_BINARY_DIR}/Tests/Dependency/WithLibOut/Lib)

    ADD_TEST(dependency_wo_lib_out ${EXECUTABLE_OUTPUT_PATH}/cmaketest 
      ${CMake_SOURCE_DIR}/Tests/Dependency 
      ${CMake_BINARY_DIR}/Tests/Dependency/WOLibOut
      exec
      ${CMake_BINARY_DIR}/Tests/Dependency/WOLibOut/Exec
      Dependency)

    ADD_TEST(dependency2 ${EXECUTABLE_OUTPUT_PATH}/cmaketest 
      ${CMake_SOURCE_DIR}/Tests/Dependency 
      ${CMake_BINARY_DIR}/Tests/Dependency/WithLibOut
      exec2
      ${CMake_BINARY_DIR}/Tests/Dependency/WithLibOut/Exec2
      Dependency CMAKE_ARGS -DLIBRARY_OUTPUT_PATH:PATH=${CMake_BINARY_DIR}/Tests/Dependency/WithLibOut/Lib)

    ADD_TEST(dependency3 ${EXECUTABLE_OUTPUT_PATH}/cmaketest 
      ${CMake_SOURCE_DIR}/Tests/Dependency 
      ${CMake_BINARY_DIR}/Tests/Dependency/WithLibOut
      exec3
      ${CMake_BINARY_DIR}/Tests/Dependency/WithLibOut/Exec3
      Dependency CMAKE_ARGS -DLIBRARY_OUTPUT_PATH:PATH=${CMake_BINARY_DIR}/Tests/Dependency/WithLibOut/Lib)

    ADD_TEST(dependency4 ${EXECUTABLE_OUTPUT_PATH}/cmaketest 
      ${CMake_SOURCE_DIR}/Tests/Dependency 
      ${CMake_BINARY_DIR}/Tests/Dependency/WithLibOut
      exec4
      ${CMake_BINARY_DIR}/Tests/Dependency/WithLibOut/Exec4
      Dependency CMAKE_ARGS -DLIBRARY_OUTPUT_PATH:PATH=${CMake_BINARY_DIR}/Tests/Dependency/WithLibOut/Lib)

    ADD_TEST(linkline ${EXECUTABLE_OUTPUT_PATH}/cmaketest 
      ${CMake_SOURCE_DIR}/Tests/LinkLine
      ${CMake_BINARY_DIR}/Tests/LinkLine
      Exec
      ${CMake_BINARY_DIR}/Tests/LinkLine
      LinkLine)

    ADD_TEST(linkorder1 ${EXECUTABLE_OUTPUT_PATH}/cmaketest 
      ${CMake_SOURCE_DIR}/Tests/LinkLineOrder
      ${CMake_BINARY_DIR}/Tests/LinkLineOrder
      Exec1
      ${CMake_BINARY_DIR}/Tests/LinkLineOrder
      LinkLineOrder)

    ADD_TEST(linkorder2 ${EXECUTABLE_OUTPUT_PATH}/cmaketest 
      ${CMake_SOURCE_DIR}/Tests/LinkLineOrder
      ${CMake_BINARY_DIR}/Tests/LinkLineOrder
      Exec2
      ${CMake_BINARY_DIR}/Tests/LinkLineOrder
      LinkLineOrder)

    ADD_TEST(curl ${EXECUTABLE_OUTPUT_PATH}/cmaketest 
             ${CMake_SOURCE_DIR}/Source/CTest/Curl
             ${CMake_BINARY_DIR}/Tests/Curl
             LIBCURL)

    ADD_TEST(kwsys ${EXECUTABLE_OUTPUT_PATH}/cmaketest
             ${CMake_SOURCE_DIR}/Source/kwsys
             ${CMake_BINARY_DIR}/Tests/kwsys
             test1
             ${CMake_BINARY_DIR}/Tests/kwsys
             kwsys)

    IF (APPLE)
      ADD_TEST(objc++ ${EXECUTABLE_OUTPUT_PATH}/cmaketest
		        ${CMake_SOURCE_DIR}/Tests/Objc++
			${CMake_BINARY_DIR}/Tests/Objc++
			objc++)
    ENDIF (APPLE)
ENDIF(BUILD_TESTING)

INCLUDE (${CMAKE_BINARY_DIR}/Source/LocalUserOptions.cmake OPTIONAL)
INCLUDE (${CMAKE_SOURCE_DIR}/Source/LocalUserOptions.cmake OPTIONAL)

INSTALL_TARGETS(/bin cmake)
INSTALL_TARGETS(/bin ctest)
INSTALL_TARGETS(/bin cmaketest)
INSTALL_FILES(${CMAKE_DATA_DIR}/include cmCPluginAPI.h)
