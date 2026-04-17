add_library(myimp SHARED IMPORTED)
set_target_properties(myimp PROPERTIES IMPORTED_LOCATION "/imported/libmyimp.so")

# Reading LOCATION from an imported target succeeds.
cmake_language(PRINT_PROPERTIES TARGETS myimp NAMED LOCATION)
