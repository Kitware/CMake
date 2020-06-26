project(test_include_dirs LANGUAGES CXX)
include(GoogleTest)

enable_testing()

add_executable(xml_output xml_output.cpp)
gtest_discover_tests(
  xml_output
  XML_OUTPUT_DIR ${CMAKE_BINARY_DIR}
)
