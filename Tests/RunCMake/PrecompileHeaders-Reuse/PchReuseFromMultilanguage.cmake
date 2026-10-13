set(CMAKE_INTERMEDIATE_DIR_STRATEGY FULL CACHE STRING "" FORCE)

enable_language(C)
enable_language(CXX)

file(WRITE ${CMAKE_BINARY_DIR}/lib.c "int lib_c(void) { return 0; }\n")
file(WRITE ${CMAKE_BINARY_DIR}/lib.cpp "int lib_cxx() { return 0; }\n")
file(WRITE ${CMAKE_BINARY_DIR}/main.c "int main(void) { return 0; }\n")

# With Visual Studio, the C and C++ PCH objects of this target need
# disambiguated names.
add_library(mixed STATIC ${CMAKE_BINARY_DIR}/lib.c ${CMAKE_BINARY_DIR}/lib.cpp)
target_precompile_headers(mixed PRIVATE <stddef.h>)

# This links the C PCH object of 'mixed', whose name must match in every
# configuration.
add_executable(main_c ${CMAKE_BINARY_DIR}/main.c)
target_precompile_headers(main_c REUSE_FROM mixed)

enable_testing()
add_test(NAME main_c COMMAND main_c)
