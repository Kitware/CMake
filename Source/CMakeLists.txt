CMAKE_MINIMUM_REQUIRED(VERSION 1.3)

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

IF (UNIX)
  INCLUDE (${CMake_SOURCE_DIR}/Modules/FindCurses.cmake OPTIONAL)
  IF (CURSES_LIBRARY)
    SUBDIRS(CursesDialog/form)
    INCLUDE(${CMake_SOURCE_DIR}/Source/CursesDialog/CMakeLists.txt)
  ENDIF (CURSES_LIBRARY)
ENDIF (UNIX)

SET(SRCS ${SRCS} cmUnixMakefileGenerator.cxx cmUnixMakefileGenerator.h)


# create a library used by the command line and the GUI
ADD_LIBRARY(CMakeLib ${SRCS})
# always link in the library
LINK_LIBRARIES(CMakeLib)
# the library is found here
LINK_DIRECTORIES(${CMake_BINARY_DIR}/Source)

ADD_EXECUTABLE(cmake cmakemain.cxx)
ADD_EXECUTABLE(DumpDocumentation cmDumpDocumentation)

ADD_EXECUTABLE(ctest ctest.cxx cmSystemTools.cxx cmRegularExpression.cxx)
ADD_EXECUTABLE(ccommand ccommand.cxx cmSystemTools.cxx cmMakefile.cxx)

CONFIGURE_FILE(
  ${CMake_SOURCE_DIR}/Source/cmaketest.h.in 
  ${CMake_BINARY_DIR}/Source/cmaketest.h ESCAPE_QUOTES)

ADD_EXECUTABLE(cmaketest cmaketest.cxx cmSystemTools.cxx)

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
ENDIF(BUILD_TESTING)


INCLUDE (${CMAKE_BINARY_DIR}/Source/LocalUserOptions.cmake OPTIONAL)
INCLUDE (${CMAKE_SOURCE_DIR}/Source/LocalUserOptions.cmake OPTIONAL)

INSTALL_TARGETS(/bin cmake)
INSTALL_TARGETS(/bin ctest)
INSTALL_TARGETS(/bin ccommand)
INSTALL_TARGETS(/bin cmaketest)

