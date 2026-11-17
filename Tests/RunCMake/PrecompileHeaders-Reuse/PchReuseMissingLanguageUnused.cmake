enable_language(C)
enable_language(CXX)

file(WRITE ${CMAKE_BINARY_DIR}/lib.c "int lib_c(void) { return 0; }\n")
file(WRITE ${CMAKE_BINARY_DIR}/skip.cpp "int main() { return 0; }\n")
file(WRITE ${CMAKE_BINARY_DIR}/disabled.cpp "int main() { return 0; }\n")

add_library(pch_c STATIC ${CMAKE_BINARY_DIR}/lib.c)
target_precompile_headers(pch_c PRIVATE <stddef.h>)

# Neither of these compiles C++ with a PCH, so pch_c needs no C++ PCH.
add_executable(skip ${CMAKE_BINARY_DIR}/skip.cpp)
target_precompile_headers(skip REUSE_FROM pch_c)
set_property(SOURCE ${CMAKE_BINARY_DIR}/skip.cpp
  PROPERTY SKIP_PRECOMPILE_HEADERS ON)

add_executable(disabled ${CMAKE_BINARY_DIR}/disabled.cpp)
target_precompile_headers(disabled REUSE_FROM pch_c)
set_property(TARGET disabled PROPERTY DISABLE_PRECOMPILE_HEADERS ON)
