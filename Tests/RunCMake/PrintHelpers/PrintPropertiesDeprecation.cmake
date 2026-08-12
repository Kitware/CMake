include(CMakePrintHelpers)

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/stub.c" "void stub(void) {}\n")
add_library(mylib STATIC "${CMAKE_CURRENT_BINARY_DIR}/stub.c")

cmake_print_properties(TARGETS mylib PROPERTIES TYPE)
