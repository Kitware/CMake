set(log "${RunCMake_BINARY_DIR}/CustomCommandDepfile-build/build.ninja")
file(READ "${log}" build_file)

set(RunCMake_TEST_FAILED)
if(NOT "${build_file}" MATCHES "depfile = test\\.d")
  string(CONCAT no_test_d "Log file:\n ${log}\n" "does not have expected line: depfile = test.d")
  list(APPEND RunCMake_TEST_FAILED "${no_test_d}")
endif()
if(NOT "${build_file}" MATCHES "depfile = test_Debug\\.d")
  string(CONCAT no_test_Debug_d "\nLog file:\n ${log}\n" "does not have expected line: depfile = test_Debug.d")
  list(APPEND RunCMake_TEST_FAILED "${no_test_Debug_d}")
endif()

function(_run_ninja dir)
  execute_process(
    COMMAND "${RunCMake_MAKE_PROGRAM}" ${ARGN}
    WORKING_DIRECTORY "${dir}"
    OUTPUT_VARIABLE ninja_stdout
    ERROR_VARIABLE ninja_stderr
    RESULT_VARIABLE ninja_result
    )
  if(NOT ninja_result EQUAL 0)
    message(STATUS "
============ beginning of ninja's stdout ============
${ninja_stdout}
=============== end of ninja's stdout ===============
")
    message(STATUS "
============ beginning of ninja's stderr ============
${ninja_stderr}
=============== end of ninja's stderr ===============
")
    message(FATAL_ERROR
      "top ninja build failed exited with status ${ninja_result}")
  endif()
  set(ninja_stdout "${ninja_stdout}" PARENT_SCOPE)
endfunction()

_run_ninja("${RunCMake_BINARY_DIR}/CustomCommandDepfile-build")
_run_ninja("${RunCMake_BINARY_DIR}/CustomCommandDepfile-build" -t deps hello.copy.c)
if (ninja_stdout MATCHES "deps not found")
  list(APPEND RunCMake_TEST_FAILED "Ninja did not track the deps of hello.copy.c in the database")
endif ()
_run_ninja("${RunCMake_BINARY_DIR}/CustomCommandDepfile-build" -t deps hello.copy2.c)
if (ninja_stdout MATCHES "deps not found")
  list(APPEND RunCMake_TEST_FAILED "Ninja did not track the deps of hello.copy2.c in the database")
endif ()
