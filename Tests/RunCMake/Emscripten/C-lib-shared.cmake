enable_language(C)
add_library(emscripten-test-lib SHARED libmod.c)
target_link_options(emscripten-test-lib PRIVATE -sSIDE_MODULE)

add_executable(exec-lib-c module.c)
target_compile_definitions(exec-lib-c PRIVATE __USE_LIBFUN)
target_link_libraries(exec-lib-c PRIVATE emscripten-test-lib)
target_link_options(exec-lib-c PRIVATE -sMAIN_MODULE)
