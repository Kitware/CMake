include(RunCMake)

run_cmake_command(build-no-cache
  ${CMAKE_COMMAND} --build ${RunCMake_SOURCE_DIR})
run_cmake_command(build-no-generator
  ${CMAKE_COMMAND} --build ${RunCMake_SOURCE_DIR}/cache-no-generator)
run_cmake_command(build-bad-generator
  ${CMAKE_COMMAND} --build ${RunCMake_SOURCE_DIR}/cache-bad-generator)

if(UNIX)
  run_cmake_command(E_create_symlink-missing-dir
    ${CMAKE_COMMAND} -E create_symlink T missing-dir/L
    )

  # Use a single build tree for a few tests without cleaning.
  set(RunCMake_TEST_BINARY_DIR
    ${RunCMake_BINARY_DIR}/E_create_symlink-broken-build)
  set(RunCMake_TEST_NO_CLEAN 1)
  file(REMOVE_RECURSE "${RunCMake_TEST_BINARY_DIR}")
  run_cmake_command(E_create_symlink-broken-create
    ${CMAKE_COMMAND} -E create_symlink T L
    )
  run_cmake_command(E_create_symlink-broken-replace
    ${CMAKE_COMMAND} -E create_symlink . L
    )
  unset(RunCMake_TEST_BINARY_DIR)
  unset(RunCMake_TEST_NO_CLEAN)

  run_cmake_command(E_create_symlink-no-replace-dir
    ${CMAKE_COMMAND} -E create_symlink T .
    )
endif()

run_cmake_command(E_sleep-no-args ${CMAKE_COMMAND} -E sleep)
run_cmake_command(E_sleep-bad-arg1 ${CMAKE_COMMAND} -E sleep x)
run_cmake_command(E_sleep-bad-arg2 ${CMAKE_COMMAND} -E sleep 1 -1)
run_cmake_command(E_sleep-one-tenth ${CMAKE_COMMAND} -E sleep 0.1)
