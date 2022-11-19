enable_language(C)

# Include a --checks option to confirm that we don't match options that start
# with --, only a standalone --
set(CMAKE_C_CLANG_TIDY "${PSEUDO_TIDY}" -p ${CMAKE_BINARY_DIR} --checks=*)

add_executable(main main.c)
