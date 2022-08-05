cmake_minimum_required (VERSION 2.8.12)
if(POLICY CMP0129)
  cmake_policy(SET CMP0129 NEW)
endif()
project(TryCompile)

macro(TEST_ASSERT value msg)
  if (NOT ${value})
    message (SEND_ERROR "Assertion failure:" ${msg} )
  endif ()
endmacro()

macro(TEST_FAIL value msg)
  if (${value})
    message (SEND_ERROR "Failing test succeeded:" ${msg} )
  endif ()
endmacro()

macro(TEST_EXPECT_EXACT command expected)
  if(NOT "x${result}" STREQUAL "x${expected}")
    message(SEND_ERROR "${CMAKE_CURRENT_LIST_LINE}: TEST \"${command}\" failed: \"${result}\" expected: \"${expected}\"")
  endif()
endmacro()

macro(TEST_EXPECT_CONTAINS command expected)
  if(NOT "${result}" MATCHES "${expected}")
    message(SEND_ERROR "${CMAKE_CURRENT_LIST_LINE}: TEST \"${command}\" failed: \"${result}\" expected: \"${expected}\"")
  endif()
endmacro()


# try to compile a file that should compile
# also check that COPY_FILE works
try_compile(SHOULD_PASS
    ${TryCompile_BINARY_DIR}
    ${TryCompile_SOURCE_DIR}/pass.c
    OUTPUT_VARIABLE TRY_OUT
    COPY_FILE ${TryCompile_BINARY_DIR}/CopyOfPass
    )

if(NOT SHOULD_PASS)
  message(SEND_ERROR "should pass failed ${TRY_OUT}")
endif()
if(NOT EXISTS "${TryCompile_BINARY_DIR}/CopyOfPass")
   message(SEND_ERROR "COPY_FILE to \"${TryCompile_BINARY_DIR}/CopyOfPass\" failed")
else()
   file(REMOVE "${TryCompile_BINARY_DIR}/CopyOfPass")
endif()

# try to compile a file that should compile
# also check that COPY_FILE_ERROR works
file(WRITE ${TryCompile_BINARY_DIR}/invalid "")
try_compile(SHOULD_PASS
    ${TryCompile_BINARY_DIR}
    ${TryCompile_SOURCE_DIR}/pass.c
    OUTPUT_VARIABLE TRY_OUT
    COPY_FILE ${TryCompile_BINARY_DIR}/invalid/path
    COPY_FILE_ERROR _captured
    )
if(NOT SHOULD_PASS)
  message(SEND_ERROR "should pass failed ${TRY_OUT}")
endif()
if(NOT _captured MATCHES "Cannot copy output executable.*/invalid/path")
  message(SEND_ERROR "COPY_FILE_ERROR did not capture expected message")
endif()

# try to compile a file that should not compile
try_compile(SHOULD_FAIL
    ${TryCompile_BINARY_DIR}
    ${TryCompile_SOURCE_DIR}/fail.c
    OUTPUT_VARIABLE TRY_OUT)
if(SHOULD_FAIL)
   message(SEND_ERROR "Should fail passed ${TRY_OUT}")
endif()

# try to compile a file that should compile
try_compile(SHOULD_PASS
    ${TryCompile_BINARY_DIR}
    ${TryCompile_SOURCE_DIR}/pass.c
    OUTPUT_VARIABLE TRY_OUT)
if(NOT SHOULD_PASS)
  message(SEND_ERROR "should pass failed ${TRY_OUT}")
endif()

# try to compile a file that should not compile
try_compile(SHOULD_FAIL
    ${TryCompile_BINARY_DIR}
    ${TryCompile_SOURCE_DIR}/fail.c
    OUTPUT_VARIABLE TRY_OUT)
if(SHOULD_FAIL)
   message(SEND_ERROR "Should fail passed ${TRY_OUT}")
endif()

# try to compile two files that should compile
try_compile(SHOULD_PASS
    ${TryCompile_BINARY_DIR}
    SOURCES ${TryCompile_SOURCE_DIR}/pass2a.c ${TryCompile_SOURCE_DIR}/pass2b.cxx
    OUTPUT_VARIABLE TRY_OUT)
if(NOT SHOULD_PASS)
  message(SEND_ERROR "should pass failed ${TRY_OUT}")
endif()

# try to compile two files that should not compile
try_compile(SHOULD_FAIL
    ${TryCompile_BINARY_DIR}
    SOURCES ${TryCompile_SOURCE_DIR}/fail2a.c ${TryCompile_SOURCE_DIR}/fail2b.c
    OUTPUT_VARIABLE TRY_OUT)
if(SHOULD_FAIL)
   message(SEND_ERROR "Should fail passed ${TRY_OUT}")
endif()

# try to compile a file that should compile
set(_c_flags "${CMAKE_C_FLAGS}")
if(WATCOM)
  string(APPEND CMAKE_C_FLAGS " -dTESTDEF")
else()
  string(APPEND CMAKE_C_FLAGS " \"-DTESTDEF\"")
endif()
try_compile(SHOULD_PASS
    ${TryCompile_BINARY_DIR}
    ${TryCompile_SOURCE_DIR}/testdef.c
    OUTPUT_VARIABLE TRY_OUT)
if(NOT SHOULD_PASS)
  message(SEND_ERROR "should pass failed ${TRY_OUT}")
endif()
set(CMAKE_C_FLAGS "${_c_flags}")

if(NOT SHOULD_FAIL)
  if(SHOULD_PASS)
    message("All Tests passed, ignore all previous output.")
  else()
    message("Test failed")
  endif()
else()
  message("Test failed")
endif()
try_compile(CMAKE_ANSI_FOR_SCOPE
  ${TryCompile_BINARY_DIR}
    ${CMAKE_ROOT}/Modules/TestForAnsiForScope.cxx OUTPUT_VARIABLE OUT)
if (CMAKE_ANSI_FOR_SCOPE)
   message("Compiler supports ansi for")
else()
   message("Compiler does not support ansi for scope")
endif()

try_compile(CMAKE_ANSI_FOR_SCOPE
  ${TryCompile_BINARY_DIR}
    ${CMAKE_ROOT}/Modules/TestForAnsiForScope.cxx OUTPUT_VARIABLE OUT)
if (CMAKE_ANSI_FOR_SCOPE)
   message("Compiler supports ansi for")
else()
   message("Compiler does not support ansi for scope")
endif()

message("use the module now")
include(${CMAKE_ROOT}/Modules/TestForANSIForScope.cmake)
if (CMAKE_ANSI_FOR_SCOPE)
   message("Compiler supports ansi for")
else()
   message("Compiler does not support ansi for scope")
endif()

message("Testing try_compile project mode")
try_compile(TEST_INNER
  ${TryCompile_BINARY_DIR}/CMakeFiles/Inner
  ${TryCompile_SOURCE_DIR}/Inner
  TryCompileInner innerexe
  OUTPUT_VARIABLE output)
TEST_ASSERT(TEST_INNER "try_compile project mode failed:\n${output}")

try_compile(COMPILE_DEFINITIONS_LIST_EXPANDED
    ${TryCompile_BINARY_DIR}
    ${TryCompile_SOURCE_DIR}/check_a_b.c
    OUTPUT_VARIABLE output
    COMPILE_DEFINITIONS "-DDEF_A;-DDEF_B"
    )
if(COMPILE_DEFINITIONS_LIST_EXPANDED)
  message(STATUS "COMPILE_DEFINITIONS list expanded correctly")
else()
  string(REPLACE "\n" "\n  " output "  ${output}")
  message(SEND_ERROR "COMPILE_DEFINITIONS list did not expand correctly\n${output}")
endif()

try_compile(SHOULD_FAIL_DUE_TO_BAD_SOURCE
    ${TryCompile_BINARY_DIR}
    ${TryCompile_SOURCE_DIR}/pass.c
    OUTPUT_VARIABLE output
    COMPILE_DEFINITIONS "bad#source.c"
    )
if(SHOULD_FAIL_DUE_TO_BAD_SOURCE AND NOT CMAKE_GENERATOR MATCHES "Watcom WMake|NMake Makefiles")
  string(REPLACE "\n" "\n  " output "  ${output}")
  message(SEND_ERROR "try_compile with bad#source.c did not fail:\n${output}")
elseif(NOT output MATCHES [[(bad#source\.c|bad\.c|bad')]])
  string(REPLACE "\n" "\n  " output "  ${output}")
  message(SEND_ERROR "try_compile with bad#source.c failed without mentioning bad source:\n${output}")
else()
  message(STATUS "try_compile with bad#source.c correctly failed")
endif()

add_executable(TryCompile pass.c)

######################################

# now two tests for try_run()

# try to run a file that should compile and run without error
# also check that OUTPUT_VARIABLE contains both the compile output
# and the run output
try_run(SHOULD_RUN SHOULD_COMPILE
    ${TryCompile_BINARY_DIR}
    ${TryCompile_SOURCE_DIR}/exit_success.c
    OUTPUT_VARIABLE TRY_OUT)
if(NOT SHOULD_COMPILE)
  message(SEND_ERROR "exit_success failed compiling: ${TRY_OUT}")
endif()
if(NOT "${SHOULD_RUN}" STREQUAL "0")
  message(SEND_ERROR "exit_success failed running with exit code ${SHOULD_RUN}")
endif()
# check the compile output for the filename
if(NOT "${TRY_OUT}" MATCHES "exit_success")
  message(SEND_ERROR " TRY_OUT didn't contain \"exit_success\": \"${TRY_OUT}\"")
endif()
# check the run output
if(NOT "${TRY_OUT}" MATCHES "hello world")
  message(SEND_ERROR " TRY_OUT didn't contain \"hello world\": \"${TRY_OUT}\"")
endif()

try_run(ARG_TEST_RUN ARG_TEST_COMPILE
    ${TryCompile_BINARY_DIR}
    ${TryCompile_SOURCE_DIR}/expect_arg.c
    OUTPUT_VARIABLE TRY_OUT
    ARGS arg1 arg2)
if(NOT ARG_TEST_COMPILE)
  message(SEND_ERROR "expect_arg failed compiling: ${TRY_OUT}")
endif()
if(NOT "${ARG_TEST_RUN}" STREQUAL "0")
  message(SEND_ERROR "expect_arg failed running with exit code ${ARG_TEST_RUN} ${TRY_OUT}")
endif()

# try to run a file that should compile and run, but return an error
try_run(SHOULD_EXIT_WITH_ERROR SHOULD_COMPILE
    ${TryCompile_BINARY_DIR}
    ${TryCompile_SOURCE_DIR}/exit_with_error.c
    COMPILE_OUTPUT_VARIABLE COMPILE_OUTPUT
    RUN_OUTPUT_VARIABLE RUN_OUTPUT)

if(NOT SHOULD_COMPILE)
  message(STATUS " exit_with_error failed compiling: ${COMPILE_OUTPUT}")
endif()
if("${SHOULD_EXIT_WITH_ERROR}" STREQUAL "0")
  message(SEND_ERROR " exit_with_error passed with exit code ${SHOULD_EXIT_WITH_ERROR}")
endif()

# check the compile output, it should contain the filename
if(NOT "${COMPILE_OUTPUT}" MATCHES "exit_with_error")
  message(SEND_ERROR " COMPILE_OUT didn't contain \"exit_with_error\": \"${COMPILE_OUTPUT}\"")
endif()
#... but not the run time output
if("${COMPILE_OUTPUT}" MATCHES "hello world")
  message(SEND_ERROR " COMPILE_OUT contains the run output: \"${COMPILE_OUTPUT}\"")
endif()
# check the run output, it should stdout
if(NOT "${RUN_OUTPUT}" MATCHES "hello world")
  message(SEND_ERROR " RUN_OUTPUT didn't contain \"hello world\": \"${RUN_OUTPUT}\"")
endif()

#######################################################################
#
# also test that the CHECK_C_SOURCE_COMPILES, CHECK_CXX_SOURCE_COMPILES
# CHECK_C_SOURCE_RUNS and CHECK_CXX_SOURCE_RUNS macros work

include(CheckCSourceCompiles)
include(CheckCXXSourceCompiles)
include(CheckCSourceRuns)
include(CheckCXXSourceRuns)

CHECK_C_SOURCE_COMPILES("I don't build" C_BUILD_SHOULD_FAIL)
CHECK_C_SOURCE_COMPILES("int main() {return 0;}" C_BUILD_SHOULD_WORK)
CHECK_C_SOURCE_RUNS("int main() {return 1;}" C_RUN_SHOULD_FAIL)
CHECK_C_SOURCE_RUNS("int main() {return 0;}" C_RUN_SHOULD_WORK)

TEST_FAIL(C_BUILD_SHOULD_FAIL "CHECK_C_SOURCE_COMPILES() succeeded, but should have failed")
TEST_ASSERT(C_BUILD_SHOULD_WORK "CHECK_C_SOURCE_COMPILES() failed")
TEST_FAIL(C_RUN_SHOULD_FAIL "CHECK_C_SOURCE_RUNS() succeeded, but should have failed")
TEST_ASSERT(C_RUN_SHOULD_WORK "CHECK_C_SOURCE_RUNS() failed")

CHECK_CXX_SOURCE_COMPILES("I don't build" CXX_BUILD_SHOULD_FAIL)
CHECK_CXX_SOURCE_COMPILES("int main() {return 0;}" CXX_BUILD_SHOULD_WORK)
CHECK_CXX_SOURCE_COMPILES("void l(char const (&x)[2]){}; int main() { l(\"\\\\n\"); return 0;}"
  CXX_BUILD_SHOULD_WORK_COMPLEX)

CHECK_CXX_SOURCE_RUNS("int main() {return 2;}" CXX_RUN_SHOULD_FAIL)
CHECK_CXX_SOURCE_RUNS("int main() {return 0;}" CXX_RUN_SHOULD_WORK)

TEST_FAIL(CXX_BUILD_SHOULD_FAIL "CHECK_CXX_SOURCE_COMPILES() succeeded, but should have failed")
TEST_ASSERT(CXX_BUILD_SHOULD_WORK "CHECK_CXX_SOURCE_COMPILES() failed")
TEST_ASSERT(CXX_BUILD_SHOULD_WORK_COMPLEX "CHECK_CXX_SOURCE_COMPILES() failed")
TEST_FAIL(CXX_RUN_SHOULD_FAIL "CHECK_CXX_SOURCE_RUNS() succeeded, but should have failed")
TEST_ASSERT(CXX_RUN_SHOULD_WORK "CHECK_CXX_SOURCE_RUNS() failed")

foreach(lang C CXX)
  if(NOT CMAKE_${lang}_COMPILER_ID STREQUAL "PathScale")
    set(${lang}_DD --)
  endif()
endforeach()

unset(C_BOGUS_FLAG CACHE)
include(CheckCCompilerFlag)
CHECK_C_COMPILER_FLAG(${C_DD}-_this_is_not_a_flag_ C_BOGUS_FLAG)
TEST_FAIL(C_BOGUS_FLAG "CHECK_C_COMPILER_FLAG() succeeded, but should have failed")
unset(C_BOGUS_FLAG CACHE)
if(DEFINED C_BOGUS_FLAG)
  # Verify that CHECK_C_COMPILER_FLAG didn't construct a normal variable
  message(SEND_ERROR "CHECK_C_COMPILER_FLAG shouldn't construct C_BOGUS_FLAG as a normal variable")
endif()

unset(CXX_BOGUS_FLAG CACHE)
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG(${CXX_DD}-_this_is_not_a_flag_ CXX_BOGUS_FLAG)
TEST_FAIL(CXX_BOGUS_FLAG "CHECK_CXX_COMPILER_FLAG() succeeded, but should have failed")
unset(CXX_BOGUS_FLAG CACHE)
if(DEFINED CXX_BOGUS_FLAG)
  # Verify that CHECK_C_COMPILER_FLAG didn't construct a normal variable
  message(SEND_ERROR "CHECK_CXX_COMPILER_FLAG shouldn't construct CXX_BOGUS_FLAG as a normal variable")
endif()

if(CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_C_COMPILER_ID STREQUAL "LCC")
  unset(C_STRICT_PROTOTYPES CACHE)
  CHECK_C_COMPILER_FLAG("-Werror;-Wstrict-prototypes" C_STRICT_PROTOTYPES)
  TEST_ASSERT(C_STRICT_PROTOTYPES "CHECK_C_COMPILER_FLAG failed -Werror -Wstrict-prototypes")
endif()

#########################################################################
#
# Test that the CHECK_OBJCC_SOURCE_COMPILES, CHECK_OBJCXX_SOURCE_COMPILES
# CHECK_OBJC_SOURCE_RUNS and CHECK_OBJCXX_SOURCE_RUNS macros work

if (APPLE)
    enable_language(OBJC)
    enable_language(OBJCXX)

    include(CheckOBJCSourceCompiles)
    include(CheckOBJCXXSourceCompiles)
    include(CheckOBJCSourceRuns)
    include(CheckOBJCXXSourceRuns)

    CHECK_OBJC_SOURCE_COMPILES("I don't build in Objective-C" OBJC_BUILD_SHOULD_FAIL)
    CHECK_OBJC_SOURCE_COMPILES("int main() { return 0; }" SIMPLE_OBJC_BUILD_SHOULD_WORK)

    TEST_FAIL(OBJC_BUILD_SHOULD_FAIL "CHECK_OBJC_SOURCE_COMPILES() succeeded, but should have failed")
    TEST_ASSERT(SIMPLE_OBJC_BUILD_SHOULD_WORK "CHECK_OBJC_SOURCE_COMPILES() failed, but should have succeeded")

    set(CMAKE_REQUIRED_LIBRARIES "-framework Foundation")

    CHECK_OBJC_SOURCE_COMPILES("#import <Foundation/Foundation.h>\nint main()\n{\nNSObject *foo;\nreturn 0;\n}\n" OBJC_BUILD_SHOULD_WORK)
    CHECK_OBJC_SOURCE_RUNS("int main() { return 2; }" SIMPLE_OBJC_RUN_SHOULD_FAIL)
    CHECK_OBJC_SOURCE_RUNS("int main() { return 0; }" SIMPLE_OBJC_RUN_SHOULD_WORK)
    CHECK_OBJC_SOURCE_RUNS("#import <Foundation/Foundation.h>\nint main()\n{\nNSObject *foo;\nreturn 2;\n}\n" OBJC_RUN_SHOULD_FAIL)
    CHECK_OBJC_SOURCE_RUNS("#import <Foundation/Foundation.h>\nint main()\n{\nNSObject *foo;\nreturn 0;\n}\n" OBJC_RUN_SHOULD_WORK)

    TEST_ASSERT(OBJC_BUILD_SHOULD_WORK "CHECK_OBJC_SOURCE_COMPILES() failed, but should have succeeded")
    TEST_FAIL(SIMPLE_OBJC_RUN_SHOULD_FAIL "CHECK_OBJC_SOURC_RUNS() succeeds, but should have failed")
    TEST_ASSERT(SIMPLE_OBJC_RUN_SHOULD_WORK "CHECK_OBJC_SOURCE_RUNS() failed, but should have succeeded")
    TEST_FAIL(OBJC_RUN_SHOULD_FAIL "CHECK_OBJC_SOURCE_RUNS() succeeds, but should have failed")
    TEST_ASSERT(OBJC_RUN_SHOULD_WORK "CHECK_OBJC_SOURCE_RUNS() failed, but should have succeeded")


    CHECK_OBJCXX_SOURCE_COMPILES("I don't build in Objective-C++" OBJCXX_BUILD_SHOULD_FAIL)
    CHECK_OBJCXX_SOURCE_COMPILES("int main() { return 0; }" SIMPLE_OBJCXX_BUILD_SHOULD_WORK)

    TEST_FAIL(OBJCXX_BUILD_SHOULD_FAIL "CHECK_OBJCXX_SOURCE_COMPILES() succeeded, but should have failed")
    TEST_ASSERT(SIMPLE_OBJCXX_BUILD_SHOULD_WORK "CHECK_OBJCXX_SOURCE_COMPILES() failed, but should have succeeded")

    CHECK_OBJCXX_SOURCE_COMPILES("#import <Foundation/Foundation.h>\n#include <iostream>\nint main()\n{\nNSObject *foo;\nstd::cout << \"Hello\" << std::endl;\nreturn 0;\n}\n" OBJCXX_BUILD_SHOULD_WORK)
    CHECK_OBJCXX_SOURCE_RUNS("int main() { return 2; }" SIMPLE_OBJCXX_RUN_SHOULD_FAIL)
    CHECK_OBJCXX_SOURCE_RUNS("int main() { return 0; }" SIMPLE_OBJCXX_RUN_SHOULD_WORK)
    CHECK_OBJCXX_SOURCE_RUNS("#import <Foundation/Foundation.h>\n#include <vector>\nint main()\n{\nNSObject *foo;\nstd::vector<int> bar;\nreturn 2;\n}\n" OBJCXX_RUN_SHOULD_FAIL)
    CHECK_OBJCXX_SOURCE_RUNS("#import <Foundation/Foundation.h>\n#include <vector>\nint main()\n{\nNSObject *foo;\nstd::vector<int> bar;\nreturn 0;\n}\n" OBJCXX_RUN_SHOULD_WORK)

    TEST_ASSERT(OBJCXX_BUILD_SHOULD_WORK "CHECK_OBJCXX_SOURCE_COMPILES() failed, but should have succeeded")
    TEST_FAIL(SIMPLE_OBJCXX_RUN_SHOULD_FAIL "CHECK_OBJCXX_SOURC_RUNS() succeeds, but should have failed")
    TEST_ASSERT(SIMPLE_OBJCXX_RUN_SHOULD_WORK "CHECK_OBJCXX_SOURCE_RUNS() failed, but should have succeeded")
    TEST_FAIL(OBJCXX_RUN_SHOULD_FAIL "CHECK_OBJCXX_SOURCE_RUNS() succeeds, but should have failed")
    TEST_ASSERT(OBJCXX_RUN_SHOULD_WORK "CHECK_OBJCXX_SOURCE_RUNS() failed, but should have succeeded")

    # try to compile a file that should compile
    try_compile(SHOULD_PASS
      ${TryCompile_BINARY_DIR}
      ${TryCompile_SOURCE_DIR}/pass.m
      OUTPUT_VARIABLE TRY_OUT)
    if(NOT SHOULD_PASS)
      message(SEND_ERROR "should pass failed ${TRY_OUT}")
    endif()

    # try to compile a file that should not compile
    try_compile(SHOULD_FAIL
      ${TryCompile_BINARY_DIR}
      ${TryCompile_SOURCE_DIR}/fail.m
      OUTPUT_VARIABLE TRY_OUT)
    if(SHOULD_FAIL)
      message(SEND_ERROR "Should fail passed ${TRY_OUT}")
    endif()

endif()

#######################################################################
#
# also test that the check_prototype_definition macro works

include(CheckPrototypeDefinition)

check_prototype_definition(remove
  "int remove(const char *pathname)"
  "0"
  "stdio.h"
  TEST_REMOVE_PROTO)
test_assert(TEST_REMOVE_PROTO "check_prototype_definition for remove() failed")
