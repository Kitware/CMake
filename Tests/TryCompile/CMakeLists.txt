cmake_minimum_required (VERSION 2.8.12)
if(POLICY CMP0129)
  cmake_policy(SET CMP0129 NEW)
endif()
project(TryCompile)

macro(EXPECT_PASS var out)
  if(NOT ${var})
    message(SEND_ERROR "Should pass failed ${out}")
  endif()
endmacro()

macro(EXPECT_FAIL var out)
  if(${var})
    message(SEND_ERROR "Should fail passed ${out}")
  endif()
endmacro()

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

if (APPLE)
  enable_language(OBJC)
  enable_language(OBJCXX)
endif()


# run old signature tests
set(try_compile_bindir_or_SOURCES ${TryCompile_BINARY_DIR})
set(try_compile_redundant_SOURCES SOURCES)
set(try_compile_output_vars OUTPUT_VARIABLE TRY_OUT)
set(try_compile_compile_output_var TRY_OUT)
set(try_compile_run_output_var TRY_OUT)
include(old_and_new_signature_tests.cmake)

# run new signature tests
set(try_compile_bindir_or_SOURCES SOURCES)
set(try_compile_redundant_SOURCES "")
set(try_compile_output_vars
  COMPILE_OUTPUT_VARIABLE COMPILE_OUT
  RUN_OUTPUT_VARIABLE RUN_OUTPUT)
set(try_compile_compile_output_var COMPILE_OUT)
set(try_compile_run_output_var RUN_OUTPUT)
include(old_and_new_signature_tests.cmake)

# try to compile a project (old signature)
message("Testing try_compile project mode (old signature)")
try_compile(TEST_INNER
  ${TryCompile_BINARY_DIR}/CMakeFiles/Inner
  ${TryCompile_SOURCE_DIR}/Inner
  TryCompileInner innerexe
  OUTPUT_VARIABLE output)
TEST_ASSERT(TEST_INNER "try_compile project mode failed:\n${output}")

# try to compile a project (new signature)
message("Testing try_compile project mode (new signature)")
try_compile(TEST_INNER
  PROJECT TryCompileInner
  SOURCE_DIR ${TryCompile_SOURCE_DIR}/Inner
  TARGET innerexe
  OUTPUT_VARIABLE output)
TEST_ASSERT(TEST_INNER "try_compile project mode failed:\n${output}")

add_executable(TryCompile pass.c)

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
