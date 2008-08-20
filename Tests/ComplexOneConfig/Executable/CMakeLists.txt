CMAKE_MINIMUM_REQUIRED(VERSION 1.3)
#
# Create exe. 
#
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DTEST_CXX_FLAGS")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DTEST_C_FLAGS")
SET(CMAKE_C_FLAGS "${CMAKE_ANSI_CFLAGS} ${CMAKE_C_FLAGS}")

IF(COMPLEX_TEST_CMAKELIB)
  # Link to CMake lib
  LINK_DIRECTORIES(${Complex_BINARY_DIR}/../../Source)
  LINK_DIRECTORIES(${Complex_BINARY_DIR}/../../Source/kwsys)
  LINK_DIRECTORIES(${Complex_BINARY_DIR}/../../Utilities/cmexpat)
  LINK_DIRECTORIES(${Complex_BINARY_DIR}/../../Utilities/cmzlib)
  LINK_DIRECTORIES(${Complex_BINARY_DIR}/../../Utilities/cmcurl)
  LINK_DIRECTORIES(${Complex_BINARY_DIR}/../../Utilities/cmtar)
ENDIF(COMPLEX_TEST_CMAKELIB)

# Create an imported target for if(TARGET) test below.
ADD_LIBRARY(ExeImportedTarget UNKNOWN IMPORTED)

# Test if(TARGET) command.
IF(NOT TARGET CMakeTestLibrary)
  MESSAGE(FATAL_ERROR "IF(NOT TARGET CMakeTestLibrary) returned true!")
ENDIF(NOT TARGET CMakeTestLibrary)
IF(NOT TARGET ExeImportedTarget)
  MESSAGE(FATAL_ERROR "IF(NOT TARGET ExeImportedTarget) returned true!")
ENDIF(NOT TARGET ExeImportedTarget)
IF(TARGET LibImportedTarget)
  MESSAGE(FATAL_ERROR "IF(TARGET LibImportedTarget) returned true!")
ENDIF(TARGET LibImportedTarget)
IF(TARGET NotATarget)
  MESSAGE(FATAL_ERROR "IF(TARGET NotATarget) returned true!")
ENDIF(TARGET NotATarget)

  # Use LINK_LIBRARIES instead of TARGET_LINK_LIBRARIES to 
SET(COMPLEX_LIBS CMakeTestLibrary;CMakeTestLibraryShared;CMakeTestCLibraryShared)
LINK_LIBRARIES(${COMPLEX_LIBS})

# Test forcing a .cxx file to not build.
SET_SOURCE_FILES_PROPERTIES(complex_nobuild.cxx PROPERTIES
  HEADER_FILE_ONLY 1)

ADD_EXECUTABLE(A A.cxx A.hh A.h A.txt)
ADD_EXECUTABLE(complex complex testcflags.c )
#  Sub1/NameConflictTest.c Sub2/NameConflictTest.c)
ADD_EXECUTABLE(complex.file complex.file.cxx complex_nobuild.cxx)
IF(COMPLEX_TEST_CMAKELIB)
  TARGET_LINK_LIBRARIES(complex CMakeLib cmsys cmexpat cmzlib cmtar cmcurl)
ENDIF(COMPLEX_TEST_CMAKELIB)

IF (UNIX)
  TARGET_LINK_LIBRARIES(complex ${CMAKE_DL_LIBS})
ELSE(UNIX)
  IF (NOT BORLAND)
    IF(NOT MINGW)
      TARGET_LINK_LIBRARIES(complex rpcrt4.lib)
    ENDIF(NOT MINGW)
  ENDIF(NOT BORLAND)
ENDIF (UNIX)

# Test linking to static lib when a shared lib has the same name.
IF(CMAKE_EXE_LINK_STATIC_CXX_FLAGS)
  ADD_DEFINITIONS(-DCOMPLEX_TEST_LINK_STATIC)
  TARGET_LINK_LIBRARIES(complex CMakeTestLinkStatic)
ENDIF(CMAKE_EXE_LINK_STATIC_CXX_FLAGS)

# can we get the path to a source file
GET_SOURCE_FILE_PROPERTY(A_LOCATION A.cxx LOCATION)
IF ("${A_LOCATION}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}/A.cxx")
  ADD_DEFINITIONS(-DCMAKE_FOUND_ACXX)
ENDIF ("${A_LOCATION}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}/A.cxx")

# get the directory parent
GET_DIRECTORY_PROPERTY(P_VALUE PARENT_DIRECTORY)
IF ("${P_VALUE}" STREQUAL "${CMAKE_SOURCE_DIR}")
  ADD_DEFINITIONS(-DCMAKE_FOUND_PARENT)
ENDIF ("${P_VALUE}" STREQUAL "${CMAKE_SOURCE_DIR}")

# get the stack of listfiles
INCLUDE(Included.cmake)
IF ("${LF_VALUE}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt;${CMAKE_CURRENT_SOURCE_DIR}/Included.cmake")
  ADD_DEFINITIONS(-DCMAKE_FOUND_LISTFILE_STACK)
ENDIF ("${LF_VALUE}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt;${CMAKE_CURRENT_SOURCE_DIR}/Included.cmake")

# Test add/remove definitions.
ADD_DEFINITIONS(
  -DCOMPLEX_DEFINED_PRE
  -DCOMPLEX_DEFINED
  -DCOMPLEX_DEFINED_POST
  -DCOMPLEX_DEFINED
  )
REMOVE_DEFINITIONS(-DCOMPLEX_DEFINED)

# Test pre-build/pre-link/post-build rules for an executable.
ADD_CUSTOM_COMMAND(TARGET complex PRE_BUILD
                   COMMAND ${CREATE_FILE_EXE}
                   ARGS "${Complex_BINARY_DIR}/Executable/prebuild.txt")
ADD_CUSTOM_COMMAND(TARGET complex PRE_BUILD
                   COMMAND ${CREATE_FILE_EXE}
                   ARGS "${Complex_BINARY_DIR}/Executable/prelink.txt")
ADD_CUSTOM_COMMAND(TARGET complex POST_BUILD
                   COMMAND ${CREATE_FILE_EXE}
                   ARGS "${Complex_BINARY_DIR}/Executable/postbuild.txt")
ADD_CUSTOM_COMMAND(TARGET complex POST_BUILD
                   COMMAND ${CMAKE_COMMAND}
                   ARGS -E copy
                        "${Complex_BINARY_DIR}/Executable/postbuild.txt"
                        "${Complex_BINARY_DIR}/Executable/postbuild2.txt")

SET_SOURCE_FILES_PROPERTIES(complex
  COMPILE_FLAGS
  "-DFILE_HAS_EXTRA_COMPILE_FLAGS"
  #" -DFILE_DEFINE_STRING=\\\"hello\\\""
  OBJECT_DEPENDS ${Complex_BINARY_DIR}/cmTestGeneratedHeader.h
)
SET_TARGET_PROPERTIES(complex PROPERTIES COMPILE_FLAGS "-DCOMPLEX_TARGET_FLAG")
ADD_CUSTOM_COMMAND(
  TARGET complex
  SOURCE ${Complex_SOURCE_DIR}/cmTestGeneratedHeader.h.in
  COMMAND ${CMAKE_COMMAND}
  ARGS -E copy ${Complex_SOURCE_DIR}/cmTestGeneratedHeader.h.in
               ${Complex_BINARY_DIR}/cmTestGeneratedHeader.h
  OUTPUTS ${Complex_BINARY_DIR}/cmTestGeneratedHeader.h
  DEPENDS ${CMAKE_COMMAND}
)

# Test creating an executable that is not built by default.
ADD_EXECUTABLE(notInAllExe EXCLUDE_FROM_ALL notInAllExe.cxx)
TARGET_LINK_LIBRARIES(notInAllExe notInAllLib)

# Test user-value flag mapping for the VS IDE.
IF(MSVC)
  SET_TARGET_PROPERTIES(notInAllExe PROPERTIES
    LINK_FLAGS "/NODEFAULTLIB:LIBC;LIBCMT;MSVCRT")
ENDIF(MSVC)

# Test creating a custom target that builds not-in-all targets.
ADD_CUSTOM_TARGET(notInAllCustom)
ADD_DEPENDENCIES(notInAllCustom notInAllExe)

#
# Output the files required by 'complex' to a file.
#
# This test has been moved to the 'required' subdir so that it
# has no side-effects on the current Makefile (duplicated source file
# due to source list expansion done twice).
#
ADD_SUBDIRECTORY(Temp)

IF(CMAKE_COMPILER_IS_GNUCXX AND CMAKE_INCLUDE_SYSTEM_FLAG_CXX)
  ADD_EXECUTABLE(testSystemDir testSystemDir.cxx)
  SET_TARGET_PROPERTIES(testSystemDir PROPERTIES COMPILE_FLAGS "-Werror")
ENDIF(CMAKE_COMPILER_IS_GNUCXX AND CMAKE_INCLUDE_SYSTEM_FLAG_CXX)

#
# Extra coverage.Not used.
#
INSTALL_TARGETS(/tmp complex)
INSTALL_PROGRAMS(/tmp complex)

CONFIGURE_FILE(
  ${Complex_SOURCE_DIR}/Executable/cmVersion.h.in
  ${Complex_BINARY_DIR}/cmVersion.h)

SOURCE_GROUP(A_GROUP ".cxx")
SOURCE_GROUP(B_GROUP REGULAR_EXPRESSION "cxx")
SOURCE_GROUP(C_GROUP FILES complex.cxx)

FILE(WRITE ${Complex_BINARY_DIR}/A/libA.a "test")
FILE(WRITE ${Complex_BINARY_DIR}/A/libC.a "test")
FILE(WRITE ${Complex_BINARY_DIR}/B/libB.a "test")
FILE(WRITE ${Complex_BINARY_DIR}/B/libA.a "test")
FILE(WRITE ${Complex_BINARY_DIR}/C/libC.a "test")
FILE(WRITE ${Complex_BINARY_DIR}/C/libB.a "test")
