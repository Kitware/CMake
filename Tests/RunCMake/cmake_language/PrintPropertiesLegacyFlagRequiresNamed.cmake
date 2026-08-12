# __CMAKE_PRINT_PROPERTIES marks the legacy cmake_print_properties() call, which
# is always NAMED; here no NAMED is given (implicit ALL), which rejects it.
cmake_language(PRINT_PROPERTIES TARGETS some_target __CMAKE_PRINT_PROPERTIES)
