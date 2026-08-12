enable_language(C)
add_library(mylib STATIC nothing.c)

# If we only read LOCATION based properties on a non-imported target, print just
# the warning and suppress the header.
cmake_language(PRINT_PROPERTIES TARGETS mylib
  NAMED LOCATION LOCATION_Debug Debug_LOCATION)
