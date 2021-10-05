if (NOT CMAKE_C_COMPILER_ID MATCHES "GNU|LCC|Intel" OR
   (CMAKE_C_COMPILER_ID STREQUAL "Intel" AND CMAKE_HOST_WIN32))
  return()
endif()

file(STRINGS ${RunCMake_TEST_BINARY_DIR}/compile_commands.json empty_dir_commands
     REGEX "command.*-Winvalid-pch.*empty.dir/cmake_pch.h")
file(STRINGS ${RunCMake_TEST_BINARY_DIR}/compile_commands.json foo_dir_commands
     REGEX "command.*-Winvalid-pch.*foo.dir/cmake_pch.h")

list(LENGTH empty_dir_commands empty_dir_commands_size)
list(LENGTH foo_dir_commands foo_dir_commands_size)

if (empty_dir_commands_size EQUAL 0)
  set(RunCMake_TEST_FAILED "empty target should have -Winvalid-pch compile option present")
  return()
endif()

if (foo_dir_commands_size GREATER 0)
  set(RunCMake_TEST_FAILED "foo target should not have -Winvalid-pch compile option present")
  return()
endif()
