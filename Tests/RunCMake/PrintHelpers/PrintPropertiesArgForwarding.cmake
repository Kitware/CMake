include(CMakePrintHelpers)

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/stub.c" "void stub(void) {}\n")
add_library(mylib STATIC "${CMAKE_CURRENT_BINARY_DIR}/stub.c")

# TARGETS scope: forwards as TARGETS mylib NAMED TYPE NAME.
cmake_print_properties(TARGETS mylib PROPERTIES TYPE NAME)

# SOURCES scope: forwards as SOURCES stub.c NAMED LANGUAGE.  Confirms
# scope-keyword translation covers more than just TARGETS.
cmake_print_properties(
  SOURCES "${CMAKE_CURRENT_BINARY_DIR}/stub.c"
  PROPERTIES LANGUAGE)
