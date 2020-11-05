enable_language(OBJCXX)
set(CMAKE_OBJCXX_CLANG_TIDY "${PSEUDO_TIDY}" -some -args)
add_executable(main main.mm)
