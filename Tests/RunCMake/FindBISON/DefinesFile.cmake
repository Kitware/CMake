include(${CMAKE_CURRENT_LIST_DIR}/check.cmake)
find_package(BISON REQUIRED)

bison_target(Relative parser.y parser.c DEFINES_FILE custom.h)
check_var(BISON_Relative_OUTPUT_HEADER "custom.h")
check_var(BISON_Relative_OUTPUTS "parser.c;custom.h")
check_var(BISON_Relative_OPTIONS "--defines=custom.h")

bison_target(Subdir parser.y parser2.c DEFINES_FILE include/custom.h)
check_var(BISON_Subdir_OUTPUT_HEADER "include/custom.h")
check_var(BISON_Subdir_OPTIONS "--defines=include/custom.h")

set(header "${CMAKE_CURRENT_BINARY_DIR}/include/absolute.h")
bison_target(Absolute parser.y parser3.c DEFINES_FILE "${header}")
check_var(BISON_Absolute_OUTPUT_HEADER "${header}")
check_var(BISON_Absolute_OUTPUTS "parser3.c;${header}")
check_var(BISON_Absolute_OPTIONS "--defines=${header}")
