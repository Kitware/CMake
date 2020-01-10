include(RunCMake)

run_cmake(empty_keyword_args)

if (CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
  macro(run_cmake_target test subtest target)
    set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${test}-build)
    set(RunCMake_TEST_OUTPUT_MERGE 1)
    set(RunCMake_TEST_NO_CLEAN 1)
    run_cmake_command(${test}-${subtest} ${CMAKE_COMMAND} --build . --target ${target} ${ARGN})

    unset(RunCMake_TEST_BINARY_DIR)
    unset(RunCMake_TEST_OUTPUT_MERGE)
    unset(RunCMake_TEST_NO_CLEAN)
  endmacro()

  run_cmake(CMP0101-BEFORE_keyword)

  run_cmake_target(CMP0101-BEFORE_keyword OLD CMP0101_OLD)
  run_cmake_target(CMP0101-BEFORE_keyword NEW CMP0101_NEW)
endif()
