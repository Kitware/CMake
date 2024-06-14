enable_language(C)

add_library(iar-test-lib)
target_sources(iar-test-lib PRIVATE libmod.c)

add_executable(exec-lib-c)
target_sources(exec-lib-c PRIVATE module.c)
target_compile_definitions(exec-lib-c PRIVATE __USE_LIBFUN)
target_link_libraries(exec-lib-c LINK_PUBLIC iar-test-lib)
target_link_options(exec-lib-c PRIVATE ${LINKER_OPTS})
