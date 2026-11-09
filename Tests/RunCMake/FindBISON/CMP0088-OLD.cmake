cmake_policy(SET CMP0088 OLD)
include(${CMAKE_CURRENT_LIST_DIR}/check.cmake)
find_package(BISON REQUIRED)

bison_target(Old parser.y parser.c VERBOSE REPORT_FILE report.txt)
check_var(BISON_Old_INPUT "parser.y")
check_var(BISON_Old_OUTPUT_SOURCE "parser.c")
check_var(BISON_Old_OUTPUT_HEADER "parser.h")
check_var(BISON_Old_OUTPUTS
  "parser.c;${CMAKE_CURRENT_SOURCE_DIR}/report.txt;parser.h")
