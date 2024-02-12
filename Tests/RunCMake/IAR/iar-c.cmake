enable_language(C)

add_executable(exec-c module.c)
target_compile_options(exec-c PRIVATE -e)
target_link_options(exec-c PRIVATE ${LINKER_OPTS})
