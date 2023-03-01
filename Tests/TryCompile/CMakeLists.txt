cmake_minimum_required (VERSION 3.5)
if(POLICY CMP0129)
  cmake_policy(SET CMP0129 NEW)
endif()
project(TryCompile)

macro(EXPECT_PASS var out)
  if(NOT ${var})
    message(SEND_ERROR "Should pass failed:\n${out}")
  endif()
endmacro()

macro(EXPECT_FAIL var out)
  if(${var})
    message(SEND_ERROR "Should fail passed:\n${out}")
  endif()
endmacro()

macro(EXPECT_COMPILED name var out)
  if(NOT ${var})
    message(SEND_ERROR "${name} failed compiling:\n${out}")
  endif()
endmacro()

macro(EXPECT_RUN_RESULT name var expected)
  if(NOT ${var} EQUAL ${expected})
    message(SEND_ERROR " ${name} gave unexpected run result: ${${var}} expected: ${expected}")
  endif()
endmacro()

macro(TEST_ASSERT value msg)
  if (NOT ${value})
    message (SEND_ERROR "Assertion failure:" ${msg} )
  endif ()
endmacro()

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

# try to compile an empty source specified directly
try_compile(SHOULD_FAIL_DUE_TO_EMPTY_SOURCE
  SOURCE_FROM_CONTENT empty.c "")
if(SHOULD_FAIL_DUE_TO_EMPTY_SOURCE)
  message(SEND_ERROR "Trying to compile an empty source succeeded?")
endif()

try_compile(SHOULD_FAIL_DUE_TO_EMPTY_SOURCE
  SOURCE_FROM_VAR empty.c NAME_OF_A_VAR_THAT_IS_NOT_SET)
if(SHOULD_FAIL_DUE_TO_EMPTY_SOURCE)
  message(SEND_ERROR "Trying to compile an empty source succeeded?")
endif()

# try to compile a copied source
try_compile(SHOULD_PASS
  SOURCE_FROM_FILE pass.c ${TryCompile_SOURCE_DIR}/pass.c
  OUTPUT_VARIABLE TRY_OUT)
EXPECT_COMPILED("SOURCE_FROM_FILE" SHOULD_PASS "${TRY_OUT}")

# try to run a source specified directly
set(TRY_RUN_MAIN_CODE
  "extern int answer(); \n"
  "int main() { return answer(); }\n")
set(TRY_RUN_EXT_CODE
  "int answer() { return 42; }\n")

try_run(SHOULD_EXIT_WITH_ERROR SHOULD_COMPILE
  SOURCE_FROM_CONTENT main.c "${TRY_RUN_MAIN_CODE}"
  SOURCE_FROM_CONTENT answer.c "${TRY_RUN_EXT_CODE}"
  COMPILE_OUTPUT_VARIABLE COMPILE_OUTPUT)
EXPECT_COMPILED("SOURCE_FROM_CONTENT" SHOULD_COMPILE "${COMPILE_OUTPUT}")
EXPECT_RUN_RESULT("SOURCE_FROM_CONTENT" SHOULD_EXIT_WITH_ERROR 42)

try_run(SHOULD_EXIT_WITH_ERROR SHOULD_COMPILE
  SOURCE_FROM_VAR main.c TRY_RUN_MAIN_CODE
  SOURCE_FROM_VAR answer.c TRY_RUN_EXT_CODE
  COMPILE_OUTPUT_VARIABLE COMPILE_OUTPUT)
EXPECT_COMPILED("SOURCE_FROM_VAR" SHOULD_COMPILE "${COMPILE_OUTPUT}")
EXPECT_RUN_RESULT("SOURCE_FROM_VAR" SHOULD_EXIT_WITH_ERROR 42)

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
# also test that the check_prototype_definition macro works

include(CheckPrototypeDefinition)

check_prototype_definition(remove
  "int remove(const char *pathname)"
  "0"
  "stdio.h"
  TEST_REMOVE_PROTO)
test_assert(TEST_REMOVE_PROTO "check_prototype_definition for remove() failed")
