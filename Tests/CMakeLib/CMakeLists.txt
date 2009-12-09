include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMake_BINARY_DIR}/Source
  ${CMake_SOURCE_DIR}/Source
  )

set(CMakeLib_TESTS
  testUTF8
  testXMLParser
  testXMLSafe
  )

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/testXMLParser.h.in
               ${CMAKE_CURRENT_BINARY_DIR}/testXMLParser.h @ONLY)

create_test_sourcelist(CMakeLib_TEST_SRCS CMakeLibTests.cxx ${CMakeLib_TESTS})
add_executable(CMakeLibTests ${CMakeLib_TEST_SRCS})
target_link_libraries(CMakeLibTests CMakeLib)

# Xcode 2.x forgets to create the output directory before linking
# the individual architectures.
if(CMAKE_OSX_ARCHITECTURES AND XCODE
    AND NOT "${XCODE_VERSION}" MATCHES "^[^12]")
  add_custom_command(
    TARGET CMakeLibTests
    PRE_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CFG_INTDIR}"
    )
endif()

foreach(test ${CMakeLib_TESTS})
  add_test(CMakeLib.${test} CMakeLibTests ${test})
endforeach()
