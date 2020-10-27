file(STRINGS ${RunCMake_TEST_BINARY_DIR}/compile_commands.json empty_dir_commands
     REGEX "command.*-fpch-instantiate-templates.*empty.dir/cmake_pch[A-Za-z0-9_.]*.h")
file(STRINGS ${RunCMake_TEST_BINARY_DIR}/compile_commands.json foo_dir_commands
     REGEX "command.*-fpch-instantiate-templates.*foo.dir/cmake_pch[A-Za-z0-9_.]*.h")

list(LENGTH empty_dir_commands empty_dir_commands_size)
list(LENGTH foo_dir_commands foo_dir_commands_size)

if (empty_dir_commands_size EQUAL 0)
  set(RunCMake_TEST_FAILED "empty target should have -fpch-instantiate-templates compile option present")
  return()
endif()

if (foo_dir_commands_size GREATER 0)
  set(RunCMake_TEST_FAILED "foo target should not have -fpch-instantiate-templates compile option present")
  return()
endif()
