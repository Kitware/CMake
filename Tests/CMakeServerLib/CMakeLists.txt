include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMake_BINARY_DIR}/Source
  ${CMake_SOURCE_DIR}/Source
  )

set(CMakeServerLib_TESTS
  testServerBuffering
  )

create_test_sourcelist(CMakeLib_TEST_SRCS CMakeServerLibTests.cxx ${CMakeServerLib_TESTS})
add_executable(CMakeServerLibTests ${CMakeLib_TEST_SRCS})
target_link_libraries(CMakeServerLibTests CMakeLib CMakeServerLib)

foreach(test ${CMakeServerLib_TESTS})
  add_test(CMakeServerLib.${test} CMakeServerLibTests ${test} ${${test}_ARGS})
endforeach()
