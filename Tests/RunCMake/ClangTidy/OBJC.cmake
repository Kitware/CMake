enable_language(OBJC)
set(CMAKE_OBJC_CLANG_TIDY "${PSEUDO_TIDY}" -some -args)
add_executable(main main.m)
