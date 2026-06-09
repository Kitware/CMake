# __CMAKE_PRINT_VARIABLES selects the legacy NAMED output, so it is only valid
# with NAMED; here no mode keyword is given (implicit ALL), which rejects it.
cmake_language(PRINT_VARIABLES __CMAKE_PRINT_VARIABLES)
