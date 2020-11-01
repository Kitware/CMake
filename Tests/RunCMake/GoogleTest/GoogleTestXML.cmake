project(test_include_dirs LANGUAGES CXX)
include(GoogleTest)

enable_testing()

# This creates the folder structure for the paramterized tests
# to avoid handling missing folders in C++
#
# This must match the match the name defined in xml_output.cpp
# for every instance of tests with GetParam.
#
# The folder name is created fom the test name (output of the line
# without leading spaces: "GoogleTestXMLSpecial/cases.") and
# the parts until the last slash ("case/"). These parts are concatenated.
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/GoogleTestXMLSpecial/cases.case")

add_executable(xml_output xml_output.cpp)
gtest_discover_tests(
  xml_output
  XML_OUTPUT_DIR ${CMAKE_BINARY_DIR}
)
