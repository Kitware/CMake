include(RunCMake)

# Function called in *-check.cmake scripts to check api files.
function(check_api expect)
  file(GLOB_RECURSE actual
    LIST_DIRECTORIES TRUE
    RELATIVE ${RunCMake_TEST_BINARY_DIR}/.cmake/api/v1
    ${RunCMake_TEST_BINARY_DIR}/.cmake/api/v1/*
    )
  if(NOT "${actual}" MATCHES "${expect}")
    set(RunCMake_TEST_FAILED "API files:
  ${actual}
do not match what we expected:
  ${expect}
in directory:
  ${RunCMake_TEST_BINARY_DIR}/.cmake/api/v1" PARENT_SCOPE)
  endif()
endfunction()

function(check_python case)
  if(RunCMake_TEST_FAILED OR NOT PYTHON_EXECUTABLE)
    return()
  endif()
  file(GLOB index ${RunCMake_TEST_BINARY_DIR}/.cmake/api/v1/reply/index-*.json)
  execute_process(
    COMMAND ${PYTHON_EXECUTABLE} "${RunCMake_SOURCE_DIR}/${case}-check.py" "${index}"
    RESULT_VARIABLE result
    OUTPUT_VARIABLE output
    ERROR_VARIABLE output
    )
  if(NOT result EQUAL 0)
    string(REPLACE "\n" "\n  " output "  ${output}")
    set(RunCMake_TEST_FAILED "Unexpected index:\n${output}" PARENT_SCOPE)
  endif()
endfunction()

if(RunCMake_GENERATOR_IS_MULTI_CONFIG)
  set(RunCMake_TEST_OPTIONS "-DCMAKE_CONFIGURATION_TYPES=Debug\\;Release\\;MinSizeRel\\;RelWithDebInfo")
endif()

run_cmake(Nothing)
run_cmake(Empty)
run_cmake(EmptyClient)
run_cmake(Stale)
run_cmake(SharedStateless)
run_cmake(ClientStateless)
run_cmake(MixedStateless)
run_cmake(DuplicateStateless)
run_cmake(ClientStateful)

function(run_object object)
  set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${object}-build)
  run_cmake(${object})
  set(RunCMake_TEST_NO_CLEAN 1)
  run_cmake_command(${object}-SharedStateless ${CMAKE_COMMAND} .)
  run_cmake_command(${object}-ClientStateless ${CMAKE_COMMAND} .)
  run_cmake_command(${object}-ClientStateful ${CMAKE_COMMAND} .)
endfunction()

run_object(codemodel-v2)
run_object(cache-v2)
run_object(cmakeFiles-v1)
