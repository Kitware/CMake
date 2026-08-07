include(CMakePrintHelpers)

# ALL is a reserved cmake_language(PRINT_VARIABLES) keyword; the wrapper must
# reject it up front rather than forward it.
cmake_print_variables(ALL)
