include(RunCMake)

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
