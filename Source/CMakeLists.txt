INCLUDE (${CMAKE_ROOT}/Modules/FindFLTK.cmake)
IF(FLTK_LIBRARY)
  IF(FLTK_INCLUDE_PATH)
    OPTION(BUILD_FLTK_GUI "Build FLTK CMake GUI." "ON")
    IF(BUILD_FLTK_GUI)
      SUBDIRS(FLTKDialog)
    ENDIF(BUILD_FLTK_GUI)
  ENDIF(FLTK_INCLUDE_PATH)
ENDIF(FLTK_LIBRARY)

SOURCE_FILES(SRCS
cmake.cxx
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
cmCableClassSet.cxx
cmSourceGroup.cxx
cmListFileCache.cxx
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
  SOURCE_FILES(SRCS cmDSWWriter.cxx cmDSPWriter.cxx cmMSProjectGenerator.cxx)
  SOURCE_FILES(SRCS cmBorlandMakefileGenerator.cpp)
  IF(NOT UNIX)
    IF(  CMAKE_CXX_COMPILER MATCHES Borland )
    ELSE (CMAKE_CXX_COMPILER MATCHES Borland )
      SUBDIRS(MFCDialog)
    ENDIF( CMAKE_CXX_COMPILER MATCHES Borland)
  ENDIF(NOT UNIX)
ENDIF (WIN32)

IF (UNIX)
  INCLUDE (${CMAKE_ROOT}/Modules/FindCurses.cmake)
  IF (CURSES_LIBRARY)
    SUBDIRS(CursesDialog/form)
    SUBDIRS(CursesDialog)
    SUBDIR_DEPENDS(CursesDialog CursesDialog/form)
  ENDIF (CURSES_LIBRARY)
ENDIF (UNIX)

SOURCE_FILES(SRCS cmUnixMakefileGenerator.cxx)


# create a library used by the command line and the GUI
ADD_LIBRARY(CMakeLib SRCS)
# always link in the library
LINK_LIBRARIES(CMakeLib)
# the library is found here
LINK_DIRECTORIES(${CMake_BINARY_DIR}/Source)

ADD_EXECUTABLE(cmake cmakemain)
ADD_EXECUTABLE(DumpDocumentation cmDumpDocumentation)

ADD_EXECUTABLE(ctest ctest.cxx cmSystemTools.cxx cmRegularExpression.cxx)

IF(BUILD_TESTING)
  IF (DART_ROOT)
    ADD_EXECUTABLE(cmaketest cmaketest.cxx cmSystemTools.cxx)
    ADD_DEPENDENCIES(cmaketest cmake)
    ADD_DEPENDENCIES(Nightly cmaketest cmake)
    ADD_DEPENDENCIES(Experimental cmaketest cmake)

    CONFIGURE_FILE(
      ${CMake_SOURCE_DIR}/Source/cmaketest.h.in 
      ${CMake_BINARY_DIR}/Source/cmaketest.h ESCAPE_QUOTES)
    ADD_TEST(DumpDocumentation ${CMake_BINARY_DIR}/Source/DumpDocumentation
      ${CMake_BINARY_DIR}/CMakeDoc.html )
    ADD_TEST(simple ${CMake_BINARY_DIR}/Source/cmaketest 
      ${CMake_SOURCE_DIR}/Tests/Simple 
      ${CMake_BINARY_DIR}/Tests/Simple simple )
    ADD_TEST(complex ${CMake_BINARY_DIR}/Source/cmaketest 
      ${CMake_SOURCE_DIR}/Tests/Complex 
      ${CMake_BINARY_DIR}/Tests/Complex 
      complex 
      ${CMake_BINARY_DIR}/Tests/Complex/bin )

  ENDIF (DART_ROOT)
ENDIF(BUILD_TESTING)



INSTALL_TARGETS(/bin cmake)
INSTALL_TARGETS(/bin ctest)

