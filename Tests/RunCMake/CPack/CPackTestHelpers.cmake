cmake_policy(SET CMP0057 NEW)

function(run_cpack_test TEST_NAME types build)
  if(TEST_TYPE IN_LIST types)
    set(RunCMake_TEST_NO_CLEAN TRUE)
    set(RunCMake_TEST_BINARY_DIR "${RunCMake_BINARY_DIR}/${TEST_NAME}-build")

     # TODO this should be executed only once per ctest run (not per generator)
    file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
    file(MAKE_DIRECTORY "${RunCMake_TEST_BINARY_DIR}")

    # execute cmake
    set(RunCMake_TEST_OPTIONS "-DGENERATOR_TYPE=${TEST_TYPE}")
    run_cmake(${TEST_NAME})

    # execute optional build step
    if(build)
      run_cmake_command(${TEST_NAME}-Build "${CMAKE_COMMAND}" --build "${RunCMake_TEST_BINARY_DIR}")
    endif()

    # execute cpack
    execute_process(
      COMMAND "${CMAKE_CPACK_COMMAND}"
      WORKING_DIRECTORY "${RunCMake_TEST_BINARY_DIR}"
      OUTPUT_FILE "${RunCMake_TEST_BINARY_DIR}/test_output.txt"
      ERROR_FILE "${RunCMake_TEST_BINARY_DIR}/test_error.txt"
      )

    # verify result
    run_cmake_command(
      ${TEST_TYPE}/${TEST_NAME}
      "${CMAKE_COMMAND}"
        -DRunCMake_TEST=${TEST_NAME}
        -DGENERATOR_TYPE=${TEST_TYPE}
        "-Dsrc_dir=${RunCMake_SOURCE_DIR}"
        "-Dbin_dir=${RunCMake_TEST_BINARY_DIR}"
        "-Dconfig_file=${config_file}"
        -P "${RunCMake_SOURCE_DIR}/VerifyResult.cmake"
      )
  endif()
endfunction()
