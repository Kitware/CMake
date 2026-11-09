include(${CMAKE_CURRENT_LIST_DIR}/check.cmake)
find_package(FLEX REQUIRED)

flex_target(Scanner scanner.l scanner.c DEFINES_FILE scanner.h)

# Simulate the result of a bison_target(Parser ...) call.
set(BISON_Parser_OUTPUT_HEADER "${CMAKE_CURRENT_BINARY_DIR}/parser.h")

add_flex_bison_dependency(Scanner Parser)
foreach(f IN LISTS FLEX_Scanner_OUTPUTS)
  get_source_file_property(deps "${f}" OBJECT_DEPENDS)
  check_var(deps "${BISON_Parser_OUTPUT_HEADER}")
endforeach()
