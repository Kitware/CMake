#
# A more complex test case
#
PROJECT (Complex)


  
#
# Use the ansi CXX compile flag for building cmake
#
IF (CMAKE_ANSI_CXXFLAGS)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_ANSI_CXXFLAGS}")  
ENDIF (CMAKE_ANSI_CXXFLAGS)

ADD_DEFINITIONS(-DCMAKE_IS_FUN)

SET(TEST_SEP "a b c")
SEPARATE_ARGUMENTS(TEST_SEP)


#
# Include vars from a file and from a cache
#
IF (EXISTS ${Complex_SOURCE_DIR}/VarTests.cmake)
  INCLUDE(${Complex_SOURCE_DIR}/VarTests.cmake)
ENDIF (EXISTS ${Complex_SOURCE_DIR}/VarTests.cmake)

LOAD_CACHE(${Complex_SOURCE_DIR}/Cache
           EXCLUDE
             CACHE_TEST_VAR_EXCLUDED
           INCLUDE_INTERNALS
             CACHE_TEST_VAR_INTERNAL)

#
# Specify include and lib dirs 
# (BEFORE is for coverage)
#
INCLUDE_DIRECTORIES(
  ${Complex_SOURCE_DIR}/Library
  ${Complex_SOURCE_DIR}/../../Source
)

INCLUDE_DIRECTORIES(BEFORE
  ${Complex_BINARY_DIR}
)

INCLUDE_REGULAR_EXPRESSION("^(cmTest|file|sharedFile).*$" ".*")

LINK_DIRECTORIES(
  ${Complex_BINARY_DIR}/Library
)

#
# Lib and exe path
#
SET (LIBRARY_OUTPUT_PATH 
     ${Complex_BINARY_DIR}/bin/ CACHE PATH 
     "Single output directory for building all libraries.")

SET (EXECUTABLE_OUTPUT_PATH 
     ${Complex_BINARY_DIR}/bin/ CACHE PATH 
     "Single output directory for building all executables.")

MESSAGE (Test " " escape " " semi-colon " " \; \;)
#
# Exec program (TODO: test a result)
# Increase coverage.
#
MESSAGE("\nIgnore this message")
OPTION(NO_EXEC_PROGRAM "Do not test EXEC_PROGRAM" 0)
IF (NOT NO_EXEC_PROGRAM)
  EXEC_PROGRAM("echo NO_EXEC_PROGRAM" "${Complex_BINARY_DIR}")
  EXEC_PROGRAM(echo ARGS NO_EXEC_PROGRAM "${Complex_BINARY_DIR}")
ELSE (NOT NO_EXEC_PROGRAM) 
  MESSAGE("Set this option ON")
ENDIF (NOT NO_EXEC_PROGRAM) 

MARK_AS_ADVANCED(NO_EXEC_PROGRAM)
MARK_AS_ADVANCED(CLEAR NO_EXEC_PROGRAM)

#
# Create directory.
# The 'complex' executable will then test if this dir exists,
# sadly it won't be able to remove it.
#
MAKE_DIRECTORY("${Complex_BINARY_DIR}/make_dir")

#
# Test FIND_LIBARY
# Create a dummy empty lib
#
CONFIGURE_FILE(
  ${Complex_SOURCE_DIR}/Library/dummy
  ${Complex_BINARY_DIR}/Library/dummylib.lib
  COPYONLY IMMEDIATE)
FOREACH (ext "${CMAKE_SHLIB_SUFFIX};.so;.a;.sl")
  CONFIGURE_FILE(
    ${Complex_SOURCE_DIR}/Library/dummy
    ${Complex_BINARY_DIR}/Library/libdummylib${ext}
    COPYONLY IMMEDIATE)
ENDFOREACH (ext)

FIND_LIBRARY(FIND_DUMMY_LIB
             dummylib
             PATHS
             ${Complex_BINARY_DIR}/Library)

#
# Test SET_SOURCE_FILES_PROPERTIES 
#
SET_SOURCE_FILES_PROPERTIES(nonexisting_file2 
	                    GENERATED 
	                    ABSTRACT 
                            WRAP_EXCLUDE 
                            COMPILE_FLAGS "-foo -bar")

GET_SOURCE_FILE_PROPERTY(FILE_HAS_ABSTRACT nonexisting_file2 ABSTRACT)
GET_SOURCE_FILE_PROPERTY(FILE_HAS_WRAP_EXCLUDE nonexisting_file2 WRAP_EXCLUDE)
GET_SOURCE_FILE_PROPERTY(FILE_COMPILE_FLAGS nonexisting_file2 COMPILE_FLAGS)

#
# Test registry (win32)
# Create a file, put its path in a registry key, try to find the file in that
# path using that registry key, then remove the file and the key
#
IF (WIN32)
  IF (NOT UNIX)
    SET(dir "${Complex_BINARY_DIR}/registry_dir")
    SET(file "registry_test_dummy")
    SET(hkey "HKEY_CURRENT_USER\\Software\\Kitware\\CMake\\Tests\\Complex;registry_test")
    CONFIGURE_FILE(
      ${Complex_SOURCE_DIR}/Library/dummy
      "${dir}/${file}"
      COPYONLY IMMEDIATE)
    EXEC_PROGRAM(${CCOMMAND_COMMAND} ARGS "write_regv \"${hkey}\" \"${dir}\"")
    FIND_PATH(REGISTRY_TEST_PATH
              ${file}
              "[${hkey}]")
    EXEC_PROGRAM(${CCOMMAND_COMMAND} ARGS "delete_regv \"${hkey}\"")
    EXEC_PROGRAM(${CCOMMAND_COMMAND} ARGS "remove \"${dir}/${file}\"")
  ENDIF (NOT UNIX)
ENDIF (WIN32)

#
# Test a set and a remove
#
SET(REMOVE_STRING a b c d e f)
SET(removeVar1 c e)
REMOVE(REMOVE_STRING ${removeVar1} f)

#
# Configure file 
# (plug vars to #define so that they can be tested)
#
CONFIGURE_FILE(
  ${Complex_SOURCE_DIR}/cmTestConfigure.h.in
  ${Complex_BINARY_DIR}/cmTestConfigure.h)

SET(STRING_WITH_QUOTES "\"hello world\"")
# test CONFIGURE_FILE with ESCAPE_QUOTES on
CONFIGURE_FILE(
  ${Complex_SOURCE_DIR}/cmTestConfigureEscape.h.in
  ${Complex_BINARY_DIR}/cmTestConfigureEscape.h ESCAPE_QUOTES)

#
# Create the libs and the main exe
#
SUBDIRS(Library Executable)
SUBDIR_DEPENDS(Executable Library)
