cmake_policy(SET CMP0098 OLD)
include(${CMAKE_CURRENT_LIST_DIR}/check.cmake)
find_package(FLEX REQUIRED)

# With the OLD behavior, flex runs in the current source directory.
set(src "${CMAKE_CURRENT_SOURCE_DIR}")
flex_target(Old scanner.l scanner.c DEFINES_FILE scanner.h)
check_var(FLEX_Old_INPUT "scanner.l")
check_var(FLEX_Old_OUTPUT_HEADER "scanner.h")
check_var(FLEX_Old_OUTPUTS "${src}/scanner.c;${src}/scanner.h")
check_var(FLEX_Old_OPTIONS "--header-file=${src}/scanner.h")
