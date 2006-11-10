#
# A more complex test case
#
SET(CMAKE_BACKWARDS_COMPATIBILITY 1.4)
PROJECT (Complex)

# Choose whether to test CMakeLib.
SET(COMPLEX_TEST_CMAKELIB 1)
IF(CMAKE_TEST_DIFFERENT_GENERATOR)
  SET(COMPLEX_TEST_CMAKELIB 0)
ENDIF(CMAKE_TEST_DIFFERENT_GENERATOR)
IF(CMAKE_TEST_SYSTEM_LIBRARIES)
  SET(COMPLEX_TEST_CMAKELIB 0)
ENDIF(CMAKE_TEST_SYSTEM_LIBRARIES)

#
# Define a macro
#
MACRO(ASSERT value msg)
  IF (NOT ${value})
    MESSAGE ("Assertion failure:" ${msg} )
  ENDIF (NOT ${value})
ENDMACRO(ASSERT)
 
# invoke the macro
ASSERT(Complex_BINARY_DIR "The PROJECT command is broken")

#
# Define a var args macro, it must take two or four args
#
MACRO(TEST_ARGC value1 value2)
  ADD_DEFINITIONS(${value1} ${value2})
  IF (${ARGC} MATCHES 4)
    ADD_DEFINITIONS(${ARGV2} ${ARGV3})
  ENDIF (${ARGC} MATCHES 4)
ENDMACRO(TEST_ARGC)
 
# invoke the macro
TEST_ARGC(-DCMAKE_ARGV1 -DCMAKE_ARGV2 -DCMAKE_ARGV3 -DCMAKE_ARGV4)

MACRO(TEST_VAR_ARG fa)
  IF("${ARGV}" MATCHES "^1;2;3$")
    MESSAGE(STATUS "ARGV works")
  ELSE("${ARGV}" MATCHES "^1;2;3$")
    MESSAGE(FATAL_ERROR "ARGV does not work; got \"${ARGV}\" instead of \"1;2;3\"")
  ENDIF("${ARGV}" MATCHES "^1;2;3$")
  IF("${ARGN}" MATCHES "^2;3$")
    MESSAGE(STATUS "ARGN works")
  ELSE("${ARGN}" MATCHES "^2;3$")
    MESSAGE(FATAL_ERROR "ARGV does not work; got \"${ARGN}\" instead of \"2;3\"")
  ENDIF("${ARGN}" MATCHES "^2;3$")
ENDMACRO(TEST_VAR_ARG)

TEST_VAR_ARG(1 2 3)

# Floating-point comparison test.
IF(2.4 LESS 2.4)
  MESSAGE(FATAL_ERROR "Failed: 2.4 LESS 2.4")
ENDIF(2.4 LESS 2.4)
IF(2.4 GREATER 2.4)
  MESSAGE(FATAL_ERROR "Failed: 2.4 GREATER 2.4")
ENDIF(2.4 GREATER 2.4)
IF(NOT 2.4 EQUAL 2.4)
  MESSAGE(FATAL_ERROR "Failed: NOT 2.4 EQUAL 2.4")
ENDIF(NOT 2.4 EQUAL 2.4)

#
# Use the ansi CXX compile flag for building cmake
#
IF (CMAKE_ANSI_CXXFLAGS)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_ANSI_CXXFLAGS}")  
ENDIF (CMAKE_ANSI_CXXFLAGS)

IF(CMAKE_SYSTEM MATCHES "OSF1-V.*")
  IF(NOT CMAKE_COMPILER_IS_GNUCXX)
     SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -timplicit_local -no_implicit_include ")
  ENDIF(NOT CMAKE_COMPILER_IS_GNUCXX)
ENDIF(CMAKE_SYSTEM MATCHES "OSF1-V.*")


ADD_DEFINITIONS(-DCMAKE_IS_FUN)
ADD_DEFINITIONS(-DCMAKE_IS_REALLY_FUN)

SET(TEST_SEP "a b c")
SEPARATE_ARGUMENTS(TEST_SEP)


#
# Include vars from a file and from a cache
#
IF (EXISTS ${Complex_SOURCE_DIR}/VarTests.cmake)
  INCLUDE(${Complex_SOURCE_DIR}/VarTests.cmake)
ENDIF (EXISTS ${Complex_SOURCE_DIR}/VarTests.cmake)
INCLUDE(fileshouldnotbehere OPTIONAL)
LOAD_CACHE(${Complex_SOURCE_DIR}/Cache
           EXCLUDE
             CACHE_TEST_VAR_EXCLUDED
           INCLUDE_INTERNALS
             CACHE_TEST_VAR_INTERNAL)

LOAD_CACHE(${Complex_SOURCE_DIR}/Cache READ_WITH_PREFIX foo CACHE_TEST_VAR2)
IF(${fooCACHE_TEST_VAR2} MATCHES bar)
  MESSAGE("Load cache worked: ${fooCACHE_TEST_VAR2}")
ELSE(${fooCACHE_TEST_VAR2} MATCHES bar)
  MESSAGE(FATAL_ERROR "Load cache with prefix failed: ${fooCACHE_TEST_VAR2}")
ENDIF(${fooCACHE_TEST_VAR2} MATCHES bar)



#
# Specify include and lib dirs 
# (BEFORE is for coverage)
#
INCLUDE_DIRECTORIES(
  Library
  ${Complex_SOURCE_DIR}/../../Source
  ${Complex_BINARY_DIR}/../../Source
)

INCLUDE_DIRECTORIES(BEFORE
  ${Complex_BINARY_DIR}
)
INCLUDE_DIRECTORIES(SYSTEM Library/SystemDir)

INCLUDE_REGULAR_EXPRESSION("^(cmTest|file|sharedFile|test).*$" "^cmMissing")

LINK_DIRECTORIES(
  ${Complex_BINARY_DIR}/Library
)

#
# check for SET CACHE FORCE
#
SET(FORCE_TEST 1 CACHE STRING "a test")
SET(FORCE_TEST 0 CACHE STRING "a test" FORCE)

#
# Lib and exe path
#
SET (LIBRARY_OUTPUT_PATH 
     ${Complex_BINARY_DIR}/bin/ CACHE INTERNAL
     "Single output directory for building all libraries.")

SET (EXECUTABLE_OUTPUT_PATH 
     ${Complex_BINARY_DIR}/bin/ CACHE INTERNAL
     "Single output directory for building all executables.")

MESSAGE (Test " " escape " " semi-colon " " \; \;)
#
# Exec program (TODO: test a result)
# Increase coverage.
#
MESSAGE("\nIgnore this message")
OPTION(NO_EXEC_PROGRAM "Do not test EXEC_PROGRAM" 0)
IF (NOT NO_EXEC_PROGRAM)
  EXEC_PROGRAM(${CMAKE_COMMAND} ARGS -E echo NO_EXEC_PROGRAM "${Complex_BINARY_DIR}")
ELSE (NOT NO_EXEC_PROGRAM) 
  MESSAGE("Set this option ON")
ENDIF (NOT NO_EXEC_PROGRAM) 

MARK_AS_ADVANCED(NO_EXEC_PROGRAM)
MARK_AS_ADVANCED(CLEAR NO_EXEC_PROGRAM)

# Execute a process.  Add coverage for this command.
EXECUTE_PROCESS(
  COMMAND ${CMAKE_COMMAND} -E echo "ABCDEFG"
  OUTPUT_VARIABLE TEST_OUT
  )
IF("${TEST_OUT}" MATCHES "^ABCDEFG\n$")
ELSE("${TEST_OUT}" MATCHES "^ABCDEFG\n$")
  MESSAGE(SEND_ERROR "EXECUTE_PROCESS output test failed: [${TEST_OUT}]")
ENDIF("${TEST_OUT}" MATCHES "^ABCDEFG\n$")

# This test has some problems on UNIX systems.  Disabling for now.
#
# EXECUTE_PROCESS(
#   COMMAND ${CMAKE_COMMAND} -E echo "ABCDEFG"
#   COMMAND /process/does/not/exist
#   OUTPUT_QUIET
#   ERROR_QUIET
#   RESULT_VARIABLE RESULT
#   )
# IF("${RESULT}" MATCHES "^0$")
#   MESSAGE(SEND_ERROR
#     "EXECUTE_PROCESS result test failed with RESULT=[${RESULT}]")
# ELSE("${RESULT}" MATCHES "^0$")
#   MESSAGE(STATUS "EXECUTE_PROCESS result test passed with RESULT=[${RESULT}]")
# ENDIF("${RESULT}" MATCHES "^0$")

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
FOREACH (ext ${CMAKE_SHLIB_SUFFIX};.so;.a;.sl)
  CONFIGURE_FILE(
    ${Complex_SOURCE_DIR}/Library/dummy
    ${Complex_BINARY_DIR}/Library/libdummylib${ext}
    COPYONLY IMMEDIATE)
ENDFOREACH (ext)

FIND_LIBRARY(FIND_DUMMY_LIB
             dummylib
             PATHS
             ${Complex_BINARY_DIR}/Library DOC "find dummy lib")

FIND_LIBRARY(FIND_DUMMY_LIB
             NAMES dummylib dummylib2
             PATHS
             ${Complex_BINARY_DIR}/Library DOC "find dummy lib")

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

SET_SOURCE_FILES_PROPERTIES(nonexisting_file3 PROPERTIES
  GENERATED 1
  ABSTRACT 1
  WRAP_EXCLUDE 1
  COMPILE_FLAGS "-foo -bar")
GET_SOURCE_FILE_PROPERTY(FILE_HAS_ABSTRACT nonexisting_file3 ABSTRACT)
GET_SOURCE_FILE_PROPERTY(FILE_HAS_WRAP_EXCLUDE nonexisting_file3 WRAP_EXCLUDE)
GET_SOURCE_FILE_PROPERTY(FILE_COMPILE_FLAGS nonexisting_file3 COMPILE_FLAGS)

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
    EXEC_PROGRAM(${CMAKE_COMMAND} ARGS "-E write_regv \"${hkey}\" \"${dir}\"")
    FIND_PATH(REGISTRY_TEST_PATH
      ${file}
      "[${hkey}]" DOC "Registry_Test_Path")
    EXEC_PROGRAM(${CMAKE_COMMAND} ARGS "-E delete_regv \"${hkey}\"")
    EXEC_PROGRAM(${CMAKE_COMMAND} ARGS "-E remove \"${dir}/${file}\"")
  ENDIF (NOT UNIX)
ENDIF (WIN32)

#
# Test a set and a remove
#
SET(REMOVE_STRING a b c d e f)
SET(removeVar1 c e)
REMOVE(REMOVE_STRING ${removeVar1} f)

#
# Test an IF inside a FOREACH.
#
FOREACH(x "a")
  IF(${x} MATCHES "a")
    # Should always execute.
    SET(IF_INSIDE_FOREACH_THEN_EXECUTED 1)
  ELSE(${x} MATCHES "a")
    # Should never execute.
    SET(IF_INSIDE_FOREACH_ELSE_EXECUTED 1)
  ENDIF(${x} MATCHES "a")
ENDFOREACH(x)

# test WHILE command
SET (while_var 1)
WHILE (while_var LESS 1000)
  SET(while_var ${while_var}0)
ENDWHILE(while_var LESS 1000)

SET(SHOULD_BE_ZERO )
SET(SHOULD_BE_ONE 1)

# test elseif functionality, the mess below tries to catch problem 
# of clauses being executed early or late etc
set (RESULT 3)
if (RESULT EQUAL 1)
  if (RESULT EQUAL 2)
    set (ELSEIF_RESULT 1)
  elseif (RESULT EQUAL 3)
    set (ELSEIF_RESULT 1)
  endif (RESULT EQUAL 2)  
elseif (RESULT EQUAL 2)
  set (ELSEIF_RESULT 1)
elseif (RESULT EQUAL 3)
  if (RESULT EQUAL 2)
    set (ELSEIF_RESULT 1)
  elseif (RESULT EQUAL 3)
    if (NOT ELSEIF_RESULT EQUAL 1)
      set (ELSEIF_RESULT 2)
    endif (NOT ELSEIF_RESULT EQUAL 1)      
  endif (RESULT EQUAL 2)  
elseif (RESULT EQUAL 4)
  if (RESULT EQUAL 2)
    set (ELSEIF_RESULT 1)
  elseif (RESULT EQUAL 3)
    set (ELSEIF_RESULT 1)
  endif (RESULT EQUAL 2)  
else (RESULT  EQUAL 1)
  if (RESULT EQUAL 2)
    set (ELSEIF_RESULT 1)
  elseif (RESULT EQUAL 3)
    set (ELSEIF_RESULT 1)
  endif (RESULT EQUAL 2)  
endif (RESULT EQUAL 1)

if (NOT ELSEIF_RESULT EQUAL 2)
  set (ELSEIF_RESULT 0)
endif (NOT ELSEIF_RESULT EQUAL 2)

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

# Test regular expression commands.
STRING(REGEX MATCH "b" RESULT "abc")
IF(NOT RESULT)
  MESSAGE(SEND_ERROR "STRING(REGEX MATCH ... ) test failed.")
ENDIF(NOT RESULT)
STRING(REGEX MATCHALL "b" RESULT "abcb")
IF(NOT RESULT)
  MESSAGE(SEND_ERROR "STRING(REGEX MATCHALL ... ) test failed.")
ENDIF(NOT RESULT)
STRING(REGEX REPLACE ".([bd])." "[\\1]" RESULT "a(b)c(d)e")
IF("x${RESULT}" MATCHES "^xa\\[b\\]c\\[d\\]e$")
  SET(STRING_REGEX_PASSED 1)
ENDIF("x${RESULT}" MATCHES "^xa\\[b\\]c\\[d\\]e$")
IF(NOT STRING_REGEX_PASSED)
  MESSAGE(SEND_ERROR
    "STRING(REGEX REPLACE ... ) test failed (\"${RESULT}\" v. \"a[b]c[d]e\")")
ENDIF(NOT STRING_REGEX_PASSED)


#
# Create the libs and the main exe
#
ADD_SUBDIRECTORY(Library)
ADD_SUBDIRECTORY(Executable)
SUBDIR_DEPENDS(Executable Library)
EXPORT_LIBRARY_DEPENDENCIES(${Complex_BINARY_DIR}/ComplexLibraryDepends.cmake)
INCLUDE(${Complex_BINARY_DIR}/ComplexLibraryDepends.cmake OPTIONAL)
