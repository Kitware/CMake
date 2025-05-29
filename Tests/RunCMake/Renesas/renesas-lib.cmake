enable_language(C)
add_library(renesas-test-lib libmod.c)

add_executable(exec-lib-c module.c)
target_compile_options(exec-lib-c)
target_compile_definitions(exec-lib-c PRIVATE __USE_LIBFUN)
target_link_libraries(exec-lib-c PRIVATE renesas-test-lib)
