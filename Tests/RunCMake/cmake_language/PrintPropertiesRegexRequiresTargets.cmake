# Regex filters are only valid with the TARGETS scope and ALL.
cmake_language(
  PRINT_PROPERTIES
  SOURCES nothing.c
  NAMED LANGUAGE
  PROPERTY_NAME_REGEX "LANG"
)
