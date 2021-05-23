include(RunCMake)

run_cmake(CMP0078-WARN)
run_cmake(CMP0078-OLD)
run_cmake(CMP0078-NEW)

run_cmake(CMP0086-WARN)

if (CMake_TEST_FindPython)

  macro(run_cmake_target test subtest target)
    set(RunCMake_TEST_BINARY_DIR ${RunCMake_BINARY_DIR}/${test}-build)
    set(RunCMake_TEST_NO_CLEAN 1)
    run_cmake_command(${test}-${subtest} ${CMAKE_COMMAND} --build . --target ${target} ${ARGN})

    unset(RunCMake_TEST_BINARY_DIR)
    unset(RunCMake_TEST_NO_CLEAN)
  endmacro()

  run_cmake(CMP0086-OLD)
  run_cmake_target(CMP0086-OLD build example)
  run_cmake(CMP0086-NEW)
  run_cmake_target(CMP0086-NEW build example)

endif()

run_cmake(CMP0122-WARN)
run_cmake(CMP0122-OLD)
run_cmake(CMP0122-NEW)
