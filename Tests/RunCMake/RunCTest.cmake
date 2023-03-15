include(RunCMake)

# Isolate our ctest runs from external environment.
unset(ENV{CTEST_PARALLEL_LEVEL})
unset(ENV{CTEST_OUTPUT_ON_FAILURE})
unset(ENV{CTEST_NO_TESTS_ACTION})

function(run_ctest CASE_NAME)
  configure_file(${RunCMake_SOURCE_DIR}/test.cmake.in
                 ${RunCMake_BINARY_DIR}/${CASE_NAME}/test.cmake @ONLY)
  if(EXISTS "${RunCMake_SOURCE_DIR}/CTestConfig.cmake.in")
    configure_file(${RunCMake_SOURCE_DIR}/CTestConfig.cmake.in
                   ${RunCMake_BINARY_DIR}/${CASE_NAME}/CTestConfig.cmake @ONLY)
  else()
    file(REMOVE ${RunCMake_BINARY_DIR}/${CASE_NAME}/CTestConfig.cmake)
  endif()
  configure_file(${RunCMake_SOURCE_DIR}/CMakeLists.txt.in
                 ${RunCMake_BINARY_DIR}/${CASE_NAME}/CMakeLists.txt @ONLY)
  if(NOT DEFINED RunCTest_VERBOSE_FLAG)
    set(RunCTest_VERBOSE_FLAG "-V")
  endif()
  run_cmake_command(${CASE_NAME} ${CMAKE_CTEST_COMMAND}
    -C Debug
    -S ${RunCMake_BINARY_DIR}/${CASE_NAME}/test.cmake
    ${RunCTest_VERBOSE_FLAG}
    --output-log ${RunCMake_BINARY_DIR}/${CASE_NAME}-build/testOutput.log
    --no-compress-output
    ${ARGN}
    )
endfunction()
