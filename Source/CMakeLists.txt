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
cmake
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
  IF(NOT UNIX)
    SUBDIRS(MFCDialog)
  ENDIF(NOT UNIX)
ENDIF (WIN32)

SOURCE_FILES(SRCS cmUnixMakefileGenerator.cxx)


# create a library used by the command line and the GUI
ADD_LIBRARY(CMakeLib SRCS)
# always link in the library
LINK_LIBRARIES(CMakeLib)
# the library is found here
LINK_DIRECTORIES(${CMake_BINARY_DIR}/Sources)

ADD_EXECUTABLE(cmake cmakemain)
ADD_EXECUTABLE(cmaketest cmaketest.cxx cmSystemTools.cxx)

CONFIGURE_FILE(
${CMake_SOURCE_DIR}/Source/cmaketest.h.in 
${CMake_BINARY_DIR}/Source/cmaketest.h )
ADD_TEST(simple ${CMake_BINARY_DIR}/Source/cmaketest 
  ${CMake_SOURCE_DIR}/Tests/Simple 
  ${CMake_BINARY_DIR}/Tests/Simple 
  ${CMake_BINARY_DIR}/Tests/Simple/simple
)

INSTALL_TARGETS(/bin cmake)

