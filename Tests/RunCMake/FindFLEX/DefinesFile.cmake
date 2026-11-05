include(${CMAKE_CURRENT_LIST_DIR}/check.cmake)
find_package(FLEX REQUIRED)

set(bin "${CMAKE_CURRENT_BINARY_DIR}")

# The header is reported as given, but passed to flex as an absolute path.
flex_target(Relative scanner.l scanner1.c DEFINES_FILE custom.h)
check_var(FLEX_Relative_OUTPUT_HEADER "custom.h")
check_var(FLEX_Relative_OUTPUTS "${bin}/scanner1.c;${bin}/custom.h")
check_var(FLEX_Relative_OPTIONS "--header-file=${bin}/custom.h")

flex_target(Subdir scanner.l scanner2.c DEFINES_FILE include/custom.h)
check_var(FLEX_Subdir_OUTPUT_HEADER "include/custom.h")
check_var(FLEX_Subdir_OUTPUTS "${bin}/scanner2.c;${bin}/include/custom.h")
check_var(FLEX_Subdir_OPTIONS "--header-file=${bin}/include/custom.h")

set(header "${bin}/include/absolute.h")
flex_target(Absolute scanner.l scanner3.c DEFINES_FILE "${header}")
check_var(FLEX_Absolute_OUTPUT_HEADER "${header}")
check_var(FLEX_Absolute_OUTPUTS "${bin}/scanner3.c;${header}")
check_var(FLEX_Absolute_OPTIONS "--header-file=${header}")
