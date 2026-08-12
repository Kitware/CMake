# DEFERRED is only meaningful with TARGETS - pairing it with a non-TARGETS
# scope must produce a fatal error.
cmake_language(
  PRINT_PROPERTIES
  SOURCES nothing.c
  DEFERRED
  NAMED LANGUAGE
)
