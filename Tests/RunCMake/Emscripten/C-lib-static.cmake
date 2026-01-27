enable_language(C)
add_library(emscripten-test-lib libmod.c)

add_executable(exec-lib-c module.c)
target_compile_definitions(exec-lib-c PRIVATE __USE_LIBFUN)
target_link_libraries(exec-lib-c PRIVATE emscripten-test-lib)
