# try to compile a file that should compile
try_compile(SHOULD_PASS
  ${try_compile_bindir_or_SOURCES}
  ${TryCompile_SOURCE_DIR}/pass.c
  OUTPUT_VARIABLE TRY_OUT)
EXPECT_PASS(SHOULD_PASS "${TRY_OUT}")

# try to compile a file that should compile
# also check that COPY_FILE works
try_compile(SHOULD_PASS
  ${try_compile_bindir_or_SOURCES}
  ${TryCompile_SOURCE_DIR}/pass.c
  OUTPUT_VARIABLE TRY_OUT
  COPY_FILE ${TryCompile_BINARY_DIR}/CopyOfPass
  )
EXPECT_PASS(SHOULD_PASS "${TRY_OUT}")

if(NOT EXISTS "${TryCompile_BINARY_DIR}/CopyOfPass")
  message(SEND_ERROR "COPY_FILE to \"${TryCompile_BINARY_DIR}/CopyOfPass\" failed")
else()
  file(REMOVE "${TryCompile_BINARY_DIR}/CopyOfPass")
endif()

# try to compile a file that should compile
# also check that COPY_FILE_ERROR works
file(WRITE ${TryCompile_BINARY_DIR}/invalid "")
try_compile(SHOULD_PASS
  ${try_compile_bindir_or_SOURCES}
  ${TryCompile_SOURCE_DIR}/pass.c
  OUTPUT_VARIABLE TRY_OUT
  COPY_FILE ${TryCompile_BINARY_DIR}/invalid/path
  COPY_FILE_ERROR _captured
  )
EXPECT_PASS(SHOULD_PASS "${TRY_OUT}")

if(NOT _captured MATCHES "Cannot copy output executable.*/invalid/path")
  message(SEND_ERROR "COPY_FILE_ERROR did not capture expected message")
endif()

# try to compile a file that should not compile
try_compile(SHOULD_FAIL
  ${try_compile_bindir_or_SOURCES}
  ${TryCompile_SOURCE_DIR}/fail.c
  OUTPUT_VARIABLE TRY_OUT)
EXPECT_FAIL(SHOULD_FAIL "${TRY_OUT}")

# try to compile two files that should compile
try_compile(SHOULD_PASS
  ${try_compile_bindir_or_SOURCES}
  ${try_compile_redundant_SOURCES}
  ${TryCompile_SOURCE_DIR}/pass2a.c
  ${TryCompile_SOURCE_DIR}/pass2b.cxx
  OUTPUT_VARIABLE TRY_OUT)
EXPECT_PASS(SHOULD_PASS "${TRY_OUT}")

# try to compile two files that should not compile
try_compile(SHOULD_FAIL
  ${try_compile_bindir_or_SOURCES}
  ${try_compile_redundant_SOURCES}
  ${TryCompile_SOURCE_DIR}/fail2a.c
  ${TryCompile_SOURCE_DIR}/fail2b.c
  OUTPUT_VARIABLE TRY_OUT)
EXPECT_FAIL(SHOULD_FAIL "${TRY_OUT}")

# try to compile a file that should compile
set(_c_flags "${CMAKE_C_FLAGS}")
if(WATCOM)
  string(APPEND CMAKE_C_FLAGS " -dTESTDEF")
else()
  string(APPEND CMAKE_C_FLAGS " \"-DTESTDEF\"")
endif()
try_compile(SHOULD_PASS
  ${try_compile_bindir_or_SOURCES}
  ${TryCompile_SOURCE_DIR}/testdef.c
  OUTPUT_VARIABLE TRY_OUT)
EXPECT_PASS(SHOULD_PASS "${TRY_OUT}")
set(CMAKE_C_FLAGS "${_c_flags}")

try_compile(CMAKE_ANSI_FOR_SCOPE
  ${try_compile_bindir_or_SOURCES}
  ${CMAKE_ROOT}/Modules/TestForAnsiForScope.cxx OUTPUT_VARIABLE OUT)
if(CMAKE_ANSI_FOR_SCOPE)
  message("Compiler supports ansi for")
else()
  message("Compiler does not support ansi for scope")
endif()

message("use the module now")
include(${CMAKE_ROOT}/Modules/TestForANSIForScope.cmake)
if(CMAKE_ANSI_FOR_SCOPE)
  message("Compiler supports ansi for")
else()
  message("Compiler does not support ansi for scope")
endif()

# test that COMPILE_DEFINITIONS are correctly expanded
try_compile(COMPILE_DEFINITIONS_LIST_EXPANDED
    ${try_compile_bindir_or_SOURCES}
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

# try to compile a file that doesn't exist
try_compile(SHOULD_FAIL_DUE_TO_BAD_SOURCE
    ${try_compile_bindir_or_SOURCES}
    ${TryCompile_SOURCE_DIR}/pass.c
    OUTPUT_VARIABLE output
    COMPILE_DEFINITIONS "bad#source.c"
    )
if(SHOULD_FAIL_DUE_TO_BAD_SOURCE AND NOT CMAKE_GENERATOR MATCHES "Watcom WMake|NMake Makefiles")
  string(REPLACE "\n" "\n  " output "  ${output}")
  message(SEND_ERROR "try_compile with bad#source.c did not fail:\n${output}")
elseif(NOT output MATCHES [[(bad#source\.c|bad\.c|bad[':])]])
  string(REPLACE "\n" "\n  " output "  ${output}")
  message(SEND_ERROR "try_compile with bad#source.c failed without mentioning bad source:\n${output}")
else()
  message(STATUS "try_compile with bad#source.c correctly failed")
endif()

if(APPLE)
  # try to compile a file that should compile
  try_compile(SHOULD_PASS
    ${try_compile_bindir_or_SOURCES}
    ${TryCompile_SOURCE_DIR}/pass.m
    OUTPUT_VARIABLE TRY_OUT)
  EXPECT_PASS(SHOULD_PASS "${TRY_OUT}")

  # try to compile a file that should not compile
  try_compile(SHOULD_FAIL
    ${try_compile_bindir_or_SOURCES}
    ${TryCompile_SOURCE_DIR}/fail.m
    OUTPUT_VARIABLE TRY_OUT)
  EXPECT_FAIL(SHOULD_FAIL "${TRY_OUT}")
endif()

# check that try_compile honors NO_CACHE
function(try_compile_scope_test)
  try_compile(
    CACHED_RESULT
    ${try_compile_bindir_or_SOURCES}
    ${TryCompile_SOURCE_DIR}/pass.c)
  try_compile(
    SHOULD_NOT_ESCAPE_SCOPE_RESULT
    ${try_compile_bindir_or_SOURCES}
    ${TryCompile_SOURCE_DIR}/pass.c
    NO_CACHE)
endfunction()

try_compile_scope_test()

if(NOT DEFINED CACHE{CACHED_RESULT})
  message(SEND_ERROR " Result from try_compile was not cached")
endif()
if(DEFINED SHOULD_NOT_ESCAPE_SCOPE_RESULT)
  message(SEND_ERROR " Result from try_compile(NO_CACHE) leaked")
endif()

######################################

# now test try_run()

# try to run a file that should compile and run without error
# also check that OUTPUT_VARIABLE contains both the compile output
# and the run output
try_run(SHOULD_RUN SHOULD_COMPILE
    ${try_compile_bindir_or_SOURCES}
    ${TryCompile_SOURCE_DIR}/exit_success.c
    ${try_compile_output_vars})
EXPECT_COMPILED("exit_success" SHOULD_COMPILE "${${try_compile_compile_output_var}}")
EXPECT_RUN_RESULT("exit_success" SHOULD_RUN 0)

# check the compile output for the filename
if(NOT "${${try_compile_compile_output_var}}" MATCHES "exit_success")
  message(SEND_ERROR
    " ${try_compile_compile_output_var} didn't contain \"exit_success\":"
    " \"${${try_compile_compile_output_var}}\"")
endif()
# check the run output
if(NOT "${${try_compile_run_output_var}}" MATCHES "hello world")
  message(SEND_ERROR
    " ${try_compile_run_output_var} didn't contain \"hello world\":"
    " \"${${try_compile_run_output_var}}\"")
endif()

try_run(ARG_TEST_RUN ARG_TEST_COMPILE
    ${try_compile_bindir_or_SOURCES}
    ${TryCompile_SOURCE_DIR}/expect_arg.c
    COMPILE_OUTPUT_VARIABLE TRY_OUT
    ARGS arg1 arg2)
EXPECT_COMPILED("expect_arg" ARG_TEST_COMPILE "${TRY_OUT}")
EXPECT_RUN_RESULT("expect_arg" ARG_TEST_RUN 0)

# try to run a file that should compile and run, but return an error
try_run(SHOULD_EXIT_WITH_ERROR SHOULD_COMPILE
    ${try_compile_bindir_or_SOURCES}
    ${TryCompile_SOURCE_DIR}/exit_with_error.c
    COMPILE_OUTPUT_VARIABLE COMPILE_OUTPUT
    RUN_OUTPUT_VARIABLE RUN_OUTPUT)
EXPECT_COMPILED("exit_with_error" SHOULD_COMPILE "${COMPILE_OUTPUT}")
EXPECT_RUN_RESULT("exit_with_error" SHOULD_EXIT_WITH_ERROR 1)

# check the compile output, it should contain the filename
if(NOT "${COMPILE_OUTPUT}" MATCHES "exit_with_error")
  message(SEND_ERROR " COMPILE_OUT didn't contain \"exit_with_error\": \"${COMPILE_OUTPUT}\"")
endif()
#... but not the run time output
if("${COMPILE_OUTPUT}" MATCHES "hello world")
  message(SEND_ERROR " COMPILE_OUT contains the run output: \"${COMPILE_OUTPUT}\"")
endif()
# check the run output, it should contain stdout
if(NOT "${RUN_OUTPUT}" MATCHES "hello world")
  message(SEND_ERROR " RUN_OUTPUT didn't contain \"hello world\": \"${RUN_OUTPUT}\"")
endif()

# try to run a file and parse stdout and stderr separately
# also check that COPY_FILE works
try_run(SHOULD_EXIT_WITH_ERROR SHOULD_COMPILE
  ${try_compile_bindir_or_SOURCES}
  ${TryCompile_SOURCE_DIR}/stdout_and_stderr.c
  COPY_FILE ${TryCompile_BINARY_DIR}/CopyOfRun
  COMPILE_OUTPUT_VARIABLE COMPILE_OUTPUT
  RUN_OUTPUT_STDOUT_VARIABLE RUN_OUTPUT_STDOUT
  RUN_OUTPUT_STDERR_VARIABLE RUN_OUTPUT_STDERR)
EXPECT_PASS(SHOULD_COMPILE "${COMPILE_OUTPUT}")

if(NOT EXISTS "${TryCompile_BINARY_DIR}/CopyOfRun")
  message(SEND_ERROR "COPY_FILE to \"${TryCompile_BINARY_DIR}/CopyOfRun\" failed")
else()
  file(REMOVE "${TryCompile_BINARY_DIR}/CopyOfRun")
endif()

# check the run stdout output
if(NOT "${RUN_OUTPUT_STDOUT}" MATCHES "hello world")
  message(SEND_ERROR " RUN_OUTPUT_STDOUT didn't contain \"hello world\": \"${RUN_OUTPUT_STDOUT}\"")
endif()
# check the run stderr output
if(NOT "${RUN_OUTPUT_STDERR}" MATCHES "error")
  message(SEND_ERROR " RUN_OUTPUT_STDERR didn't contain \"error\": \"${RUN_OUTPUT_STDERR}\"")
endif()

# check that try_run honors NO_CACHE
function(try_run_scope_test)
  try_run(
    CACHED_RUN_RESULT
    CACHED_COMPILE_RESULT
    ${try_compile_bindir_or_SOURCES}
    ${TryCompile_SOURCE_DIR}/exit_success.c)
  try_run(
    SHOULD_NOT_ESCAPE_SCOPE_RUN_RESULT
    SHOULD_NOT_ESCAPE_SCOPE_COMPILE_RESULT
    ${try_compile_bindir_or_SOURCES}
    ${TryCompile_SOURCE_DIR}/exit_success.c
    NO_CACHE)
endfunction()

try_run_scope_test()

if(NOT DEFINED CACHE{CACHED_COMPILE_RESULT})
  message(SEND_ERROR " Compile result from try_run was not cached")
endif()
if(NOT DEFINED CACHE{CACHED_RUN_RESULT})
  message(SEND_ERROR " Run result from try_run was not cached")
endif()
if(DEFINED SHOULD_NOT_ESCAPE_SCOPE_COMPILE_RESULT)
  message(SEND_ERROR " Compile result from try_run(NO_CACHE) leaked")
endif()
if(DEFINED SHOULD_NOT_ESCAPE_SCOPE_RUN_RESULT)
  message(SEND_ERROR " Run result from try_run(NO_CACHE) leaked")
endif()
