project(test_include_dirs)
include(CTest)
include(GoogleTest)

enable_testing()

add_executable(xml_output xml_output.cpp)
gtest_discover_tests(
  xml_output
  XML_OUTPUT_DIR ${CMAKE_BINARY_DIR}
)
