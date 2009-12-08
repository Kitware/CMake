include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMake_BINARY_DIR}/Source
  ${CMake_SOURCE_DIR}/Source
  )

set(CMakeLib_TESTS
  testXMLParser
  )

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/testXMLParser.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/testXMLParser.h @ONLY)

create_test_sourcelist(CMakeLib_TEST_SRCS CMakeLibTests.cxx ${CMakeLib_TESTS})
add_executable(CMakeLibTests ${CMakeLib_TEST_SRCS})
target_link_libraries(CMakeLibTests CMakeLib)

foreach(test ${CMakeLib_TESTS})
  add_test(CMakeLib.${test} CMakeLibTests ${test})
endforeach()
