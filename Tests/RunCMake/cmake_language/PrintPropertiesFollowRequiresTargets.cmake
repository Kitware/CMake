# FOLLOW_DEPENDENCIES is only valid with the TARGETS scope.
cmake_language(
  PRINT_PROPERTIES
  SOURCES nothing.c
  FOLLOW_DEPENDENCIES
  NAMED LANGUAGE
)
