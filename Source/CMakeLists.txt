CMAKE_MINIMUM_REQUIRED(VERSION 1.3)

IF(WIN32)
  SET(BUILD_SHARED_LIBS 1)
ENDIF(WIN32)

SET(SRCS
cmake.cxx
cmakewizard.cxx
cmMakeDepend.cxx
cmMakefile.cxx
cmMakefileGenerator.cxx
cmRegularExpression.cxx
cmSourceFile.cxx
cmSystemTools.cxx
cmDirectory.cxx
cmDynamicLoader.cxx
cmCPluginAPI.cxx
cmCommands.cxx
cmTarget.cxx
cmCustomCommand.cxx
cmCacheManager.cxx
cmSourceGroup.cxx
cmListFileCache.cxx
cmake.h
cmakewizard.h
cmMakeDepend.h
cmMakefile.h
cmMakefileGenerator.h
cmRegularExpression.h
cmSourceFile.h
cmSystemTools.h
cmDynamicLoader.h
cmDirectory.h
cmCommands.h
cmTarget.h
cmCustomCommand.h
cmCacheManager.h
cmSourceGroup.h
cmListFileCache.h
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

IF (WIN32)
  SET(SRCS ${SRCS}
                    cmDSWWriter.cxx 
                    cmDSPWriter.cxx 
                    cmMSProjectGenerator.cxx
                    cmBorlandMakefileGenerator.cxx
                    cmNMakeMakefileGenerator.cxx
                    cmMSDotNETGenerator.cxx
                    cmDSWWriter.h 
                    cmDSPWriter.h 
                    cmMSProjectGenerator.h
                    cmBorlandMakefileGenerator.h
                    cmNMakeMakefileGenerator.h
                    cmMSDotNETGenerator.h
                    )
  IF(NOT UNIX)
    IF(  NOT BORLAND )
      LINK_LIBRARIES( rpcrt4.lib )
      SUBDIRS(MFCDialog)
    ENDIF( NOT BORLAND )
  ENDIF(NOT UNIX)
ENDIF (WIN32)

SET(SRCS ${SRCS} cmUnixMakefileGenerator.cxx cmUnixMakefileGenerator.h)

# create a library used by the command line and the GUI
ADD_LIBRARY(CMakeLib ${SRCS})


# always link in the library
# the library is found here
LINK_DIRECTORIES(${CMake_BINARY_DIR}/Source)

ADD_EXECUTABLE(cmake cmakemain.cxx)
ADD_EXECUTABLE(DumpDocumentation cmDumpDocumentation)
ADD_EXECUTABLE(ctest ctest.cxx)

TARGET_LINK_LIBRARIES(cmake CMakeLib)
TARGET_LINK_LIBRARIES(DumpDocumentation CMakeLib)
TARGET_LINK_LIBRARIES(ctest CMakeLib)

IF (UNIX)
  INCLUDE (${CMake_SOURCE_DIR}/Modules/FindCurses.cmake OPTIONAL)
  IF (CURSES_LIBRARY)
    SUBDIRS(CursesDialog/form)
    INCLUDE(${CMake_SOURCE_DIR}/Source/CursesDialog/CMakeLists.txt)
  ENDIF (CURSES_LIBRARY)
ENDIF (UNIX)

IF (NOT DART_ROOT)
SET(MAKEPROGRAM ${CMAKE_MAKE_PROGRAM})
ENDIF (NOT DART_ROOT)

CONFIGURE_FILE(
  ${CMake_SOURCE_DIR}/Source/cmaketest.h.in 
  ${CMake_BINARY_DIR}/Source/cmaketest.h ESCAPE_QUOTES)

ADD_EXECUTABLE(cmaketest cmaketest.cxx)
TARGET_LINK_LIBRARIES(cmaketest CMakeLib)

#ADD_LIBRARY(TEST_PLUGIN SHARED cmSimpleCommandPlugin.c)
#TARGET_LINK_LIBRARIES(TEST_PLUGIN CMakeLib)

IF(BUILD_TESTING)
    ADD_TEST(DumpDocumentation ${CMake_BINARY_DIR}/Source/DumpDocumentation
      ${CMake_BINARY_DIR}/CMakeDoc.html)

    ADD_TEST(simple ${CMake_BINARY_DIR}/Source/cmaketest 
      ${CMake_SOURCE_DIR}/Tests/Simple 
      ${CMake_BINARY_DIR}/Tests/Simple 
      simple)

    ADD_TEST(conly ${CMake_BINARY_DIR}/Source/cmaketest 
      ${CMake_SOURCE_DIR}/Tests/COnly 
      ${CMake_BINARY_DIR}/Tests/COnly 
      conly)

    ADD_TEST(complex ${CMake_BINARY_DIR}/Source/cmaketest 
      ${CMake_SOURCE_DIR}/Tests/Complex 
      ${CMake_BINARY_DIR}/Tests/Complex 
      complex 
      ${CMake_BINARY_DIR}/Tests/Complex/bin)

    ADD_TEST(Example ${CMake_BINARY_DIR}/Source/cmaketest 
      ${CMake_SOURCE_DIR}/Example
      ${CMake_BINARY_DIR}/Example
      helloDemo
      ${CMake_BINARY_DIR}/Example/Demo
      HELLO)

    ADD_TEST(testing ${CMake_BINARY_DIR}/Source/cmaketest 
      ${CMake_SOURCE_DIR}/Tests/Testing 
      ${CMake_BINARY_DIR}/Tests/Testing
      testing
      ${CMake_BINARY_DIR}/Tests/Testing/bin)

    ADD_TEST(wrapping ${CMake_BINARY_DIR}/Source/cmaketest 
      ${CMake_SOURCE_DIR}/Tests/Wrapping 
      ${CMake_BINARY_DIR}/Tests/Wrapping
      wrapping
      ${CMake_BINARY_DIR}/Tests/Wrapping/bin)

    ADD_TEST(testdriver1 ${CMake_BINARY_DIR}/Source/cmaketest 
      ${CMake_SOURCE_DIR}/Tests/TestDriver 
      ${CMake_BINARY_DIR}/Tests/TestDriver
      TestDriverTest
      ${CMake_BINARY_DIR}/Tests/Wrapping/bin
      TestDriverTest test1)

    ADD_TEST(testdriver2 ${CMake_BINARY_DIR}/Source/cmaketest 
      ${CMake_SOURCE_DIR}/Tests/TestDriver 
      ${CMake_BINARY_DIR}/Tests/TestDriver
      TestDriverTest
      ${CMake_BINARY_DIR}/Tests/Wrapping/bin
      TestDriverTest test2)

    ADD_TEST(testdriver3 ${CMake_BINARY_DIR}/Source/cmaketest 
      ${CMake_SOURCE_DIR}/Tests/TestDriver 
      ${CMake_BINARY_DIR}/Tests/TestDriver
      TestDriverTest
      ${CMake_BINARY_DIR}/Tests/Wrapping/bin
      TestDriverTest subdir/test3)

    ADD_TEST(dependency_w_libout ${CMake_BINARY_DIR}/Source/cmaketest 
      ${CMake_SOURCE_DIR}/Tests/Dependency 
      ${CMake_BINARY_DIR}/Tests/Dependency/WithLibOut
      exec
      ${CMake_BINARY_DIR}/Tests/Dependency/WithLibOut/Exec
      Dependency CMAKE_ARGS -DLIBRARY_OUTPUT_PATH:PATH=${CMake_BINARY_DIR}/Tests/Dependency/WithLibOut/Lib)

    ADD_TEST(dependency_wo_lib_out ${CMake_BINARY_DIR}/Source/cmaketest 
      ${CMake_SOURCE_DIR}/Tests/Dependency 
      ${CMake_BINARY_DIR}/Tests/Dependency/WOLibOut
      exec
      ${CMake_BINARY_DIR}/Tests/Dependency/WOLibOut/Exec
      Dependency)

    ADD_TEST(dependency2 ${CMake_BINARY_DIR}/Source/cmaketest 
      ${CMake_SOURCE_DIR}/Tests/Dependency 
      ${CMake_BINARY_DIR}/Tests/Dependency/WithLibOut
      exec2
      ${CMake_BINARY_DIR}/Tests/Dependency/WithLibOut/Exec2
      Dependency CMAKE_ARGS -DLIBRARY_OUTPUT_PATH:PATH=${CMake_BINARY_DIR}/Tests/Dependency/WithLibOut/Lib)

    ADD_TEST(dependency3 ${CMake_BINARY_DIR}/Source/cmaketest 
      ${CMake_SOURCE_DIR}/Tests/Dependency 
      ${CMake_BINARY_DIR}/Tests/Dependency/WithLibOut
      exec3
      ${CMake_BINARY_DIR}/Tests/Dependency/WithLibOut/Exec3
      Dependency CMAKE_ARGS -DLIBRARY_OUTPUT_PATH:PATH=${CMake_BINARY_DIR}/Tests/Dependency/WithLibOut/Lib)

    ADD_TEST(dependency4 ${CMake_BINARY_DIR}/Source/cmaketest 
      ${CMake_SOURCE_DIR}/Tests/Dependency 
      ${CMake_BINARY_DIR}/Tests/Dependency/WithLibOut
      exec4
      ${CMake_BINARY_DIR}/Tests/Dependency/WithLibOut/Exec4
      Dependency CMAKE_ARGS -DLIBRARY_OUTPUT_PATH:PATH=${CMake_BINARY_DIR}/Tests/Dependency/WithLibOut/Lib)

    ADD_TEST(linkline ${CMake_BINARY_DIR}/Source/cmaketest 
      ${CMake_SOURCE_DIR}/Tests/LinkLine
      ${CMake_BINARY_DIR}/Tests/LinkLine
      Exec
      ${CMake_BINARY_DIR}/Tests/LinkLine
      LinkLine)

  ENDIF (DART_ROOT)
ENDIF(BUILD_TESTING)


INCLUDE (${CMAKE_BINARY_DIR}/Source/LocalUserOptions.cmake OPTIONAL)
INCLUDE (${CMAKE_SOURCE_DIR}/Source/LocalUserOptions.cmake OPTIONAL)

INSTALL_TARGETS(/bin cmake)
INSTALL_TARGETS(/bin ctest)
INSTALL_TARGETS(/bin cmaketest)

