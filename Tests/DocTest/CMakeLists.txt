cmake_minimum_required (VERSION 2.6)
project (DocTest)

add_executable (DocTest DocTest.cxx)

set_property(GLOBAL PROPERTY REPORT_UNDEFINED_PROPERTIES
  "${CMAKE_CURRENT_BINARY_DIR}/UndefinedProperties.txt")
