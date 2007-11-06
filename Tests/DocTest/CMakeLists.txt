project (DocTest)

add_executable (DocTest DocTest.cxx)

set_properties(GLOBAL PROPERTIES REPORT_UNDEFINED_PROPERTIES
  "${CMAKE_CURRENT_BINARY_DIR}/UndefinedProperties.txt")
