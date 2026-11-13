enable_language(C)
enable_language(CXX)

file(WRITE ${CMAKE_BINARY_DIR}/lib.c "int lib_c(void) { return 0; }\n")
file(WRITE ${CMAKE_BINARY_DIR}/lib.cpp "int lib_cxx() { return 0; }\n")
file(WRITE ${CMAKE_BINARY_DIR}/main.c "int main(void) { return 0; }\n")
file(WRITE ${CMAKE_BINARY_DIR}/main.cpp "int main() { return 0; }\n")

# A C library has no C++ PCH.
add_library(pch_c STATIC ${CMAKE_BINARY_DIR}/lib.c)
target_precompile_headers(pch_c PRIVATE <stddef.h>)
add_executable(main_cxx ${CMAKE_BINARY_DIR}/main.cpp)
target_precompile_headers(main_cxx REUSE_FROM pch_c)

# This one compiles C, but its headers are for C++ only.
add_library(pch_cxx STATIC ${CMAKE_BINARY_DIR}/lib.c ${CMAKE_BINARY_DIR}/lib.cpp)
target_precompile_headers(pch_cxx PRIVATE
  "$<$<COMPILE_LANGUAGE:CXX>:<stddef.h$<ANGLE-R>>")
add_executable(main_c ${CMAKE_BINARY_DIR}/main.c)
target_precompile_headers(main_c REUSE_FROM pch_cxx)
