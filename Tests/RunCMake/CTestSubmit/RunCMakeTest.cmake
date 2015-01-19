include(RunCMake)

# Default case parameters.
set(CASE_DROP_METHOD "http")
set(CASE_CTEST_SUBMIT_ARGS "")

function(run_ctest CASE_NAME)
  configure_file(${RunCMake_SOURCE_DIR}/test.cmake.in
                 ${RunCMake_BINARY_DIR}/${CASE_NAME}/test.cmake @ONLY)
  configure_file(${RunCMake_SOURCE_DIR}/CTestConfig.cmake.in
                 ${RunCMake_BINARY_DIR}/${CASE_NAME}/CTestConfig.cmake @ONLY)
  configure_file(${RunCMake_SOURCE_DIR}/CMakeLists.txt.in
                 ${RunCMake_BINARY_DIR}/${CASE_NAME}/CMakeLists.txt @ONLY)
  run_cmake_command(${CASE_NAME} ${CMAKE_CTEST_COMMAND}
    -C Debug
    -S ${RunCMake_BINARY_DIR}/${CASE_NAME}/test.cmake
    -V
    --output-log ${RunCMake_BINARY_DIR}/${CASE_NAME}-build/testOutput.log
    ${ARGN}
    )
endfunction()

#-----------------------------------------------------------------------------
# Test bad argument combinations.

function(run_ctest_submit CASE_NAME)
  set(CASE_CTEST_SUBMIT_ARGS "${ARGN}")
  run_ctest(${CASE_NAME})
endfunction()

run_ctest_submit(BadArg bad-arg)
run_ctest_submit(BadPARTS PARTS bad-part)
run_ctest_submit(BadFILES FILES bad-file)
run_ctest_submit(RepeatRETURN_VALUE RETURN_VALUE res RETURN_VALUE res)

#-----------------------------------------------------------------------------
# Test failed drops by various protocols

function(run_ctest_submit_FailDrop CASE_DROP_METHOD)
  run_ctest(FailDrop-${CASE_DROP_METHOD})
endfunction()

# TODO: call run_ctest_submit_FailDrop() for each submission protocol
