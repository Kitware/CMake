if(NOT DEFINED CMake_SOURCE_DIR)
  message(FATAL_ERROR "CMake_SOURCE_DIR not defined")
endif()

if(NOT DEFINED dir)
  message(FATAL_ERROR "dir not defined")
endif()

if(NOT DEFINED gen)
  message(FATAL_ERROR "gen not defined")
endif()

message(STATUS "CTEST_FULL_OUTPUT (Avoid ctest truncation of output)")

# Run cmake:
#
function(run_cmake build_dir extra_args expected_result expected_output expected_error)
  message(STATUS "run_cmake build_dir='${build_dir}' extra_args='${extra_args}'")

  # Ensure build_dir exists:
  #
  execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${build_dir})

  # Run cmake:
  #
  execute_process(COMMAND ${CMAKE_COMMAND}
    ${extra_args}
    -G ${gen} ${CMake_SOURCE_DIR}/Tests/CMakeCommands/build_command
    RESULT_VARIABLE result
    OUTPUT_VARIABLE stdout
    ERROR_VARIABLE stderr
    WORKING_DIRECTORY ${build_dir}
    )

  message(STATUS "result='${result}'")
  message(STATUS "stdout='${stdout}'")
  message(STATUS "stderr='${stderr}'")
  message(STATUS "")

  # Verify result and output match expectations:
  #
  if("0" STREQUAL "${expected_result}")
    if(NOT "${result}" STREQUAL "0")
      message(FATAL_ERROR
        "error: result='${result}' is non-zero and different than expected_result='${expected_result}'")
    endif()
  else()
    if("${result}" STREQUAL "0")
      message(FATAL_ERROR
        "error: result='${result}' is zero and different than expected_result='${expected_result}'")
    endif()
  endif()

  foreach(e ${expected_output})
    if(NOT stdout MATCHES "${e}")
      message(FATAL_ERROR
        "error: stdout does not match expected_output item e='${e}'")
    else()
      message(STATUS "info: stdout matches '${e}'")
    endif()
  endforeach()

  foreach(e ${expected_error})
    if(NOT stderr MATCHES "${e}")
      message(FATAL_ERROR
        "error: stderr does not match expected_error item e='${e}'")
    else()
      message(STATUS "info: stderr matches '${e}'")
    endif()
  endforeach()

  message(STATUS "result, stdout and stderr match all expectations: test passes")
  message(STATUS "")
endfunction()


# Expect this case to succeed:
run_cmake("${dir}/b1" "" 0
  "Build files have been written to:"
  "skipping cases 1, 2 and 3 because TEST_ERROR_CONDITIONS is OFF")


# Expect this one to fail:
run_cmake("${dir}/b2" "-DTEST_ERROR_CONDITIONS:BOOL=ON" 1
  "Configuring incomplete, errors occurred!"
  "build_command requires at least one argument naming a CMake variable;build_command unknown argument ")
