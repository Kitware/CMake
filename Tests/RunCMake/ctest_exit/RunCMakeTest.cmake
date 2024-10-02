include(RunCMake)

message(STATUS "Multiple -S options:")

run_cmake_command(3 ${CMAKE_CTEST_COMMAND} -V
  -S ${RunCMake_SOURCE_DIR}/exit1.cmake
  -S ${RunCMake_SOURCE_DIR}/exit2.cmake
  )

run_cmake_command(7 ${CMAKE_CTEST_COMMAND} -V
  -S ${RunCMake_SOURCE_DIR}/exit4.cmake
  -S ${RunCMake_SOURCE_DIR}/exit1.cmake
  -S ${RunCMake_SOURCE_DIR}/exit2.cmake
  )

message(STATUS "Multiple -SP options:")

run_cmake_command(3 ${CMAKE_CTEST_COMMAND} -V
  -SP ${RunCMake_SOURCE_DIR}/exit1.cmake
  -SP ${RunCMake_SOURCE_DIR}/exit2.cmake
  )

run_cmake_command(7 ${CMAKE_CTEST_COMMAND} -V
  -SP ${RunCMake_SOURCE_DIR}/exit4.cmake
  -SP ${RunCMake_SOURCE_DIR}/exit1.cmake
  -SP ${RunCMake_SOURCE_DIR}/exit2.cmake
  )

message(STATUS "Mixed -S and -SP options:")

run_cmake_command(7 ${CMAKE_CTEST_COMMAND} -V
  -S ${RunCMake_SOURCE_DIR}/exit4.cmake
  -SP ${RunCMake_SOURCE_DIR}/exit1.cmake
  -S ${RunCMake_SOURCE_DIR}/exit2.cmake
  )

message(STATUS "ctest_run_script:")

configure_file(
  ${RunCMake_SOURCE_DIR}/test.cmake.in
  ${RunCMake_BINARY_DIR}/test.cmake @ONLY)

run_cmake_command(Script ${CMAKE_CTEST_COMMAND} -V
  -S ${RunCMake_BINARY_DIR}/test.cmake
  )
