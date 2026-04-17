include(CMakePrintHelpers)

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/stub.c" "void stub(void) {}\n")
add_library(mylib STATIC "${CMAKE_CURRENT_BINARY_DIR}/stub.c")

# Reserved keyword as positional argument
cmake_print_properties(
  DEFERRED
  TARGETS mylib
  PROPERTIES MY_PROP
)
